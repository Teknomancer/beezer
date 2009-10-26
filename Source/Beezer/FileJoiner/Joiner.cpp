/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <Messenger.h>
#include <String.h>
#include <File.h>
#include <Entry.h>
#include <Path.h>
#include <Node.h>
#include <Directory.h>
#include <Debug.h>

#include <sys/stat.h>
#include <fs_attr.h>

#include <stdlib.h>
#include <stdio.h>

#include "Shared.h"
#include "Joiner.h"

typedef struct stat StatStruct;

//=============================================================================================================//

status_t JoinFile (const char *firstChunkPathStr, const char *outputDir, const char *separator,
            BMessenger *progress, volatile bool *cancel)
{
    BString firstChunkPath = firstChunkPathStr;
    int32 index = firstChunkPath.FindLast ('/');
    BString dirString;
    firstChunkPath.CopyInto (dirString, 0, index);
    
    int32 index2 = firstChunkPath.FindLast (separator);
    if (index2 <= 0 || index2 < index)
        return BZR_ERROR;
    
    BString baseName;
    firstChunkPath.CopyInto (baseName, index + 1, index2 - 1 - index);
    
    BString numberString;
    firstChunkPath.CopyInto (numberString, index2 + strlen(separator), firstChunkPath.Length() - index2);
    
    int8 width = numberString.Length();
    
    BString curFileName = baseName;
    curFileName << separator << numberString;
    
    // Initialize output file
    BString outputFilePath = outputDir;
    outputFilePath << "/" << baseName;
    BFile destFile;
    status_t err = destFile.SetTo (outputFilePath.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
    BDirectory destDir (dirString.String());
    if (err != B_OK)
        return BZR_ERROR;

    // Determine buffer size used for copying file -- this is not inside the loop because all
    // files being joined are of the same size
    BEntry chunkEntry (firstChunkPathStr, false);
    StatStruct srcStat;
    BFile srcFile (firstChunkPathStr, B_READ_ONLY);
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
        bufSize = freeSize / 4;                        // take 1/4 of RAM max
        bufSize -= bufSize % (16 * 1024);            // Round to 16 KB boundaries
        if (bufSize < kMinBufferSize)                // at least kMinBufferSize
            bufSize = kMinBufferSize;
        else if (bufSize > kMaxBufferSize)            // no more than kMaxBufferSize
            bufSize = kMaxBufferSize; 
    }

    BDirectory dir (dirString.String());
    uint16 start = atoi (numberString.String());    // start from the number they choose eg 2 or 3
    off_t writePosition = 0;
    char *buffer = new char[bufSize];
    while (dir.FindEntry (curFileName.String(), &chunkEntry, false) == B_OK)
    {
        PRINT (("%s\n", curFileName.String()));

        // Do the copying stuff!!
        for (;;)
        {
            if (cancel && *cancel == true)
            {
                destFile.Unset();

                BEntry destEntry;
                if (dir.FindEntry (baseName.String(), &destEntry) == B_OK)
                    destEntry.Remove();

                return BZR_CANCEL;
            }
            
            ssize_t bytes = srcFile.Read (buffer, bufSize);
            if (bytes > 0)
            {
                ssize_t updateBytes = 0;
                if (bytes > 32 * 1024)
                    updateBytes = bytes / 2;
                
                ssize_t result = destFile.WriteAt (writePosition, buffer, (size_t)bytes);

                if (progress)
                {
                    BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
                    updateMessage.AddString ("text", curFileName.String());
                    updateMessage.AddFloat ("delta", (float)result);
                    progress->SendMessage (&updateMessage, &reply);
                }

                if (result != bytes)
                    return B_ERROR;
                    
                writePosition += bytes;
            }
            else
                break;
        }
        
        start++;
        char buf[B_PATH_NAME_LENGTH];
        sprintf (buf, "%s%s%0*d", baseName.String(), separator, width, start);
        curFileName = buf;
        
        srcFile.Unset();
        BString nextFilePath = outputDir;
        nextFilePath << "/" << curFileName;
        srcFile.SetTo (nextFilePath.String(), B_READ_ONLY);
    }
    
    // Copy attributes from first chunk to joint file
    BFile firstChunkFile (firstChunkPathStr, B_READ_ONLY);
    JoinCopyAttributes (&firstChunkFile, &destFile, buffer, bufSize);
    
    destFile.Unset();
    return BZR_DONE;
}

//=============================================================================================================//

void JoinCopyAttributes (BNode *srcNode, BNode *destNode, void *buffer, size_t bufSize)
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

void FindChunks (const char *firstChunkPathStr, const char *separator, int32 &fileCount,
            off_t &totalSize, volatile bool *cancel)
{
    BString firstChunkPath = firstChunkPathStr;
    int32 index = firstChunkPath.FindLast ('/');
    BString dirString;
    firstChunkPath.CopyInto (dirString, 0, index);
    
    int32 index2 = firstChunkPath.FindLast (separator);
    if (index2 <= 0 || index2 < index)
        return;
    
    BString baseName;
    firstChunkPath.CopyInto (baseName, index + 1, index2 - 1 - index);
    
    BString numberString;
    firstChunkPath.CopyInto (numberString, index2 + strlen(separator), firstChunkPath.Length() - index2);
    
    int8 width = numberString.Length();
    
    BString curFileName = baseName;
    curFileName << separator << numberString;
    
    BDirectory dir (dirString.String());
    BEntry chunkEntry;
    off_t size;
    uint16 start = atoi (numberString.String());    // start from the number they choose eg 2 or 3
    // uint16 start = 1;                            // always start with 1 as the first file number
    // Determine what is to be done, either 1 or the number user chooses
    while (dir.FindEntry (curFileName.String(), &chunkEntry, false) == B_OK)
    {
        if (cancel && *cancel == true)
            break;
        
        fileCount++;
        chunkEntry.GetSize (&size);
        totalSize += size;
        
        start++;
        char buf[B_PATH_NAME_LENGTH];
        sprintf (buf, "%s%s%0*d", baseName.String(), separator, width, start);
        curFileName = buf;
    }
}

//=============================================================================================================//
