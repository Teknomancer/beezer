/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Entry.h>
#include <Node.h>
#include <Message.h>
#include <Messenger.h>
#include <File.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <SymLink.h>
#include <Locker.h>
#include <Autolock.h>
#include <String.h>
#include <Path.h>
#include <Debug.h>

#include <fs_attr.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "FSUtils.h"
#include "Shared.h"

typedef struct stat StatStruct;

// Global object declares
BLocker _fs_utils_locker ("_fs_utils_lock", true);

//=============================================================================================================//

status_t CopyDirectory (BEntry *srcDir, BDirectory *destDir, BMessenger *progress, volatile bool *cancel)
{
    BAutolock autoLocker (&_fs_utils_locker);
    if (!autoLocker.IsLocked())
        return B_ERROR;

    // Create sub-directory (further on will copy into subDir -- see loop below)
    char subDirLeaf [B_FILE_NAME_LENGTH];
    srcDir->GetName (subDirLeaf);
    BDirectory subDir;
    
    // If there already exists a folder under the name of subDirLeaf, then use it
    destDir->CreateDirectory (subDirLeaf, &subDir);
    subDir.SetTo (destDir, subDirLeaf);
    if (subDir.InitCheck() != B_OK)        // a file with the subDirLeaf name exists, don't bother
        return B_ERROR;                  // clobbering it, just quit we have already overcome possibility of
                                    // a folder with the subDirLeaf name existing, don't bother anymore
        
    // Copies the attributes of source-directory (in-case of custom icon'd-folders etc)
    // This will copy all that to the destination directory as well
    BNode srcNode (srcDir);
    BEntry destEntry;
    destDir->GetEntry (&destEntry);
    BNode destNode (&destEntry);
    const size_t bufSize = 1024 * 128;
    char *buffer = new char[bufSize];
    CopyAttributes (&srcNode, &destNode, buffer, bufSize);
    delete[] buffer;

    BDirectory dir (srcDir);
    BEntry entry;
    status_t exitCode = BZR_DONE;
    while (dir.GetNextEntry (&entry, false) == B_OK)
    {
        if (*cancel == true)
           return BZR_CANCEL;
        
        if (entry.IsDirectory() == false)
           exitCode = CopyFile (&entry, &subDir, progress, cancel);
        else
           exitCode = CopyDirectory (&entry, &subDir, progress, cancel);

    }
    
    return exitCode;
}

//=============================================================================================================//

void GetDirectoryInfo (BEntry *srcDir, int32 &fileCount, int32 &folderCount, off_t &totalSize,
           volatile bool *cancel)
{
    BAutolock autoLocker (&_fs_utils_locker);
    if (!autoLocker.IsLocked())
        return;

    if (cancel && *cancel == true)     // check for cancel again for finer granularity
        return;
    
    // The fileCount, folderCount, totalSize must be initialized to zero before this function
    // is called (for the first time in its recursion - i.e. from the calling program)
    folderCount++;
    BDirectory dir (srcDir);
    BEntry entry;
    while (dir.GetNextEntry (&entry, false) == B_OK)
    {
        if (cancel && *cancel == true)
           return;
        
        if (entry.IsDirectory() == false)
        {
           off_t size;
           entry.GetSize (&size);
           fileCount ++;
           totalSize += size;
        }
        else
           GetDirectoryInfo (&entry, fileCount, folderCount, totalSize, cancel);
    }
}

//=============================================================================================================//

void RemoveDirectory (BDirectory *dir)
{
    BAutolock autoLocker (&_fs_utils_locker);
    if (!autoLocker.IsLocked())
        return;
    
    // Remove all entries in the given directory, (including all subdirs), and the dir itself
    BEntry entry;
    dir->Rewind();
    while (dir->GetNextEntry (&entry, false) == B_OK)
    {
        if (entry.IsDirectory() == true)
        {
           BDirectory subDir (&entry);
           RemoveDirectory (&subDir);
        }
        else
           entry.Remove();
    }
    
    dir->GetEntry(&entry);
    if (entry.Exists())
        entry.Remove();
}

//=============================================================================================================//

BString CreateTempDirectory (const char *prefix, BDirectory **createdDir, bool createNow)
{
    BAutolock autolocker (&_fs_utils_locker);
    if (!autolocker.IsLocked())
        return NULL;
    
    BString prefixStr = "bzr_";
    if (prefix)
        prefixStr << prefix << "_";
    
    BPath tmpPath;
    find_directory (B_COMMON_TEMP_DIRECTORY, &tmpPath, true);
    BString tempDirName = tempnam (const_cast<char*>(tmpPath.Path()), const_cast<char*>(prefixStr.String()));

    // Further random the dir name (the below number is a prime)
    tempDirName << "_" << rand() % 1646237;
    if (createNow)
    {
        create_directory (tempDirName.String(), 0777);
        if (createdDir)
           *createdDir = new BDirectory (tempDirName.String());
    }

    return tempDirName;
}

//=============================================================================================================//

status_t CopyFile (BEntry *srcEntry, BDirectory *destDir, BMessenger *progress,    volatile bool *cancel)
{
    BAutolock autoLocker (&_fs_utils_locker);
    if (!autoLocker.IsLocked())
        return B_ERROR;

    char destLeaf [B_FILE_NAME_LENGTH];
    srcEntry->GetName (destLeaf);

    if (progress)
    {
        BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
        updateMessage.AddString ("text", destLeaf);
        updateMessage.AddFloat ("delta", 1.0f);
        progress->SendMessage (&updateMessage, &reply);
    }
    
    if (srcEntry->IsSymLink())                  // Handle copying of symlink
    {
        BSymLink srcLink;
        BSymLink newLink;
        char linkPath [MAXPATHLEN];

        srcLink.SetTo(srcEntry);
        srcLink.ReadLink (linkPath, MAXPATHLEN-1);

        if (destDir->CreateSymLink (destLeaf, linkPath, &newLink) == B_OK)
           return BZR_DONE;
        else
           return B_ERROR;
           
    }
    else if (srcEntry->IsFile())               // Handle copying of file
    {
        StatStruct srcStat;
        BFile srcFile (srcEntry, B_READ_ONLY);
        srcFile.GetStat (&srcStat);
        
        const size_t kMinBufferSize = 1024 * 128; 
        const size_t kMaxBufferSize = 1024 * 1024; 
        
        size_t bufSize = kMinBufferSize;
        
        if (bufSize < srcStat.st_size)
        {
           // File is bigger than buffer size; find an optimal buffer size for copying
           system_info sysInfo;
           get_system_info (&sysInfo);

           size_t freeSize = static_cast<size_t>((sysInfo.max_pages - sysInfo.used_pages) * B_PAGE_SIZE);
           bufSize = freeSize / 4;                      // take 1/4 of RAM max 
           bufSize -= bufSize % (16 * 1024);           // Round to 16 KB boundaries 
           if (bufSize < kMinBufferSize)               // at least kMinBufferSize 
               bufSize = kMinBufferSize; 
           else if (bufSize > kMaxBufferSize)           // no more than kMaxBufferSize 
               bufSize = kMaxBufferSize; 
        }
        
        BFile destFile;
        status_t err = destFile.SetTo (destDir, destLeaf, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
        if (err != B_OK) return err;
        
        char *buffer = new char[bufSize];
        for (;;)
        {
           if (cancel && *cancel == true)
           {
               destFile.Unset();

               BEntry destEntry;
               if (destDir->FindEntry (destLeaf, &destEntry) == B_OK)
                  destEntry.Remove();

               return BZR_CANCEL;
           }
           
           ssize_t bytes = srcFile.Read (buffer, bufSize);
           if (bytes > 0)
           {
               ssize_t updateBytes = 0;
               if (bytes > 32 * 1024)
                  updateBytes = bytes / 2;
               
               ssize_t result = destFile.Write (buffer, (size_t)bytes);
               if (result != bytes)
                  return B_ERROR;
           }
           else
               break;
        }
        
        CopyAttributes (&srcFile, &destFile, buffer, bufSize);
        delete[] buffer;
        
        destFile.SetPermissions (srcStat.st_mode);
        destFile.SetOwner (srcStat.st_uid);
        destFile.SetGroup (srcStat.st_gid);
        destFile.SetModificationTime (srcStat.st_mtime);
        destFile.SetCreationTime (srcStat.st_crtime);
    }
    return BZR_DONE;
}

//=============================================================================================================//

void CopyAttributes (BNode *srcNode, BNode *destNode, void *buffer, size_t bufSize)
{
    srcNode->RewindAttrs();
    char name[256];
    while (srcNode->GetNextAttrName (name) == B_OK)
    {
        attr_info info;
        if (srcNode->GetAttrInfo (name, &info) != B_OK)
           continue;

        ssize_t bytes;
        ssize_t numToRead = (ssize_t)info.size;
        
        for (off_t offset = 0; numToRead > 0; offset += bytes)
        {
           size_t chunkSize = (size_t)numToRead;
           if (chunkSize > bufSize)
               chunkSize = bufSize;

           bytes = srcNode->ReadAttr (name, info.type, offset, buffer, chunkSize);
           if (bytes <= 0) 
               break;
           
           destNode->WriteAttr (name, info.type, offset, buffer, (size_t)bytes);
           numToRead -= bytes;
        }
    }
}

//=============================================================================================================//

status_t SplitFile (BEntry *srcEntry, BDirectory *destDir, BMessenger *progress, uint64 fragmentSize,
           uint16 fragmentCount, char *sepString, BString &firstChunkName, volatile bool *cancel)
{
    BAutolock autoLocker (&_fs_utils_locker);
    if (!autoLocker.IsLocked())
        return B_ERROR;
    
    off_t size;
    char destLeaf [B_FILE_NAME_LENGTH];
    srcEntry->GetName (destLeaf);
    srcEntry->GetSize (&size);
    
    BString outputName = destLeaf;

    if (srcEntry->IsFile() == false)               // Handle reading of file
        return BZR_ERROR;
    
    // Now determine the width of numbers needed for proper alphabetical sort
    int8 width = 0;
    int64 numerator = fragmentCount;
    while (numerator > 0)
    {
        numerator /= 10;
        width++;
    }

    if (width == 1)        // Minimum is 01, 02, 03... not 1, 2, 3 even when pieces are less than 10
        width++;
    
    StatStruct srcStat;
    BFile srcFile (srcEntry, B_READ_ONLY);
    srcFile.GetStat (&srcStat);
    
    const size_t kMinBufferSize = 1024 * 128;
    const size_t kMaxBufferSize = 1024 * 1024; 
    
    size_t bufSize = kMinBufferSize;
    
    if (bufSize < srcStat.st_size)
    {
        // File is bigger than buffer size; find an optimal buffer size for copying
        system_info sysInfo;
        get_system_info (&sysInfo);

        size_t freeSize = static_cast<size_t>((sysInfo.max_pages - sysInfo.used_pages) * B_PAGE_SIZE);
        bufSize = freeSize / 4;                      // take 1/4 of RAM max
        bufSize -= bufSize % (16 * 1024);           // Round to 16 KB boundaries
        if (bufSize < kMinBufferSize)               // at least kMinBufferSize
           bufSize = kMinBufferSize;
        else if (bufSize > kMaxBufferSize)           // no more than kMaxBufferSize
           bufSize = kMaxBufferSize; 
    }
    
    if (bufSize > fragmentSize)
        bufSize = fragmentSize;

    size_t bufSizeOriginal = bufSize;
    char bufFileName[B_FILE_NAME_LENGTH+1];
    for (uint16 i = 0; i < fragmentCount; i++)
    {
        BFile destFile;
        sprintf (bufFileName, "%s%s%0*d", destLeaf, sepString, width, i + 1);
        BString destFileName = bufFileName;
        
        status_t err = destFile.SetTo (destDir, destFileName.String(),
                         B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);

        if (err != B_OK)
           return BZR_ERROR;
        
        // Restore bufferSize to original buffer size!
        if (bufSize != bufSizeOriginal)
           bufSize = bufSizeOriginal;
        
        char *buffer = new char[bufSize];
        uint64 bytesWritten = 0;
        for (;;)
        {
           if (cancel && *cancel == true)
           {
               destFile.Unset();
    
               BEntry destEntry;
               if (destDir->FindEntry (destFileName.String(), &destEntry) == B_OK)
                  destEntry.Remove();
    
               return BZR_CANCEL;
           }
           
           
           // Example: Fragment size = 1.4 MB
           //         bufSize = 1 MB
           //         so it will write 1 MB, then another 1 MB, but no we must write only 0.4 MB
           //         -- the below check does that, i.e. sets buffersize as 0.4 MB
           // Thus change buffersize to write denominations less than buffersize :)
        
           if (fragmentSize - bytesWritten < bufSize)
               bufSize = fragmentSize - bytesWritten;
           
           ssize_t bytes = srcFile.Read (buffer, bufSize);
           if (bytes > 0)
           {
               ssize_t result = destFile.Write (buffer, (size_t)bytes);
               
               if (progress)
               {
                  BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
                  updateMessage.AddString ("text", destFileName.String());
                  updateMessage.AddFloat ("delta", (float)result);
                  progress->SendMessage (&updateMessage, &reply);
               }
               
               if (result != bytes)
                  return BZR_ERROR;
               else
                  bytesWritten += bytes;
           }
           else
               break;
           
           if (bytesWritten >= fragmentSize)
               break;
        }
        
        // Copy attributes only for the first file
        if (i == 0)
        {
           sprintf (bufFileName, "%s%s%0*d", destLeaf, sepString, width, 1);
           destFile.Unset();
           status_t err = destFile.SetTo (destDir, bufFileName, B_READ_WRITE);
           
           if (err == B_OK)
               CopyAttributes (&srcFile, &destFile, buffer, bufSize);

           // Pass this information back to the caller!!
           firstChunkName = bufFileName;
        }
               
        delete[] buffer;
        destFile.SetPermissions (srcStat.st_mode);
        destFile.SetOwner (srcStat.st_uid);
        destFile.SetGroup (srcStat.st_gid);
        destFile.SetModificationTime (srcStat.st_mtime);
        destFile.SetCreationTime (srcStat.st_crtime);
    }
    
    return BZR_DONE;
}

//=============================================================================================================//
