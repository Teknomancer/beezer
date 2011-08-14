/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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
 * -> Neither the name of the author nor the names of its contributors may
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

#include <Debug.h>
#include <Entry.h>
#include <Message.h>
#include <Path.h>
#include <String.h>
#include <File.h>
#include <NodeInfo.h>
#include <Messenger.h>
#include <Directory.h>
#include <Menu.h>
#include <MenuItem.h>

#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <fstream>

#include "RarArchiver.h"
#include "RarStrings.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

//=============================================================================================================//

Archiver *load_archiver (const char *binPath)
{
    return new RarArchiver (binPath);
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

RarArchiver::RarArchiver (const char *binPath)
{
    // The list of supported mimetypes by this add-on, note the first (index 0) mime-type
    // in the list will be the one that will be used while creating files using this add-on
    m_passwordRequired = false;
    m_mimeList.AddItem (strdup ("application/x-rar"));
    m_mimeList.AddItem (strdup ("application/x-rar-compressed"));
    m_mimeList.AddItem (strdup ("application/rar"));
    SetArchiveType ("rar");
    SetArchiveExtension (".rar");

    m_error = BZR_DONE;

    // Detect rar binary
    if (IsBinaryFound (m_rarPath, binPath, BZR_ARK) == false
        || IsBinaryFound (m_unrarPath, binPath, BZR_UNARK) == false)
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}

//=============================================================================================================//

status_t RarArchiver::ReadOpen (FILE *fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
           sizeStr[25], methodStr[25], packedStr[25], ratioStr[15], dayStr[5], permStr[50],
           monthStr[5], yearStr[8], hourStr[5], minuteStr[5], dateStr[90], crcStr[25], versionStr[25],
           pathStr[B_PATH_NAME_LENGTH + 1];
    
    do
    {
        fgets (lineString, len, fp);
    } while (!feof (fp) && (strstr (lineString, "--------" ) == NULL));
    
    fgets (lineString, len, fp);
    
    bool odd = true;
    while (!feof (fp) && (strstr (lineString, "--------" ) == NULL))
    {
        lineString[strlen (lineString) - 1] = '\0';
        
        // Rar reports in 2 lines, first line for just the path, the next line has columnar info
        // So odd has paths and even lines have info about those paths - so read accordingly
        if (odd == true)
        {
           sscanf (lineString,    "%[^\n]", pathStr);
           odd = false;
           fgets (lineString, len, fp);
           continue;
        }
        else
        {
           sscanf (lineString,
               " %[0-9] %[0-9] %[0-9%] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9] %[^ ] %[^ ] %[^ ] %[^\n]",
               sizeStr, packedStr, ratioStr, dayStr, monthStr, yearStr, hourStr, minuteStr, permStr,
               crcStr, methodStr, versionStr);
            odd = true;
        }
        
        struct tm timeStruct; time_t timeValue;
        MakeTime (&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");
        FormatDate (dateStr, 90, &timeStruct);
               
        BString pathString = pathStr;
        if (pathString.Length() > 1)
        {
           if (pathString.ByteAt(0) == '*')
               m_passwordRequired = true;
    
           pathString.Remove (0, 1);
        }
        
        // Check to see if pathStr is as folder, else add it as a file
        if (strstr (permStr, "D") != NULL || permStr[0] == 'd')
        {
           // Critical we add '/' for empty folders as rar doesn't report folder names with '/'
           pathString << '/';
           m_entriesList.AddItem (new ArchiveEntry (true, pathString.String(), sizeStr, packedStr, dateStr,
                                    timeValue, methodStr, crcStr));
        }
        else
        {
           m_entriesList.AddItem (new ArchiveEntry (false, pathString.String(), sizeStr, packedStr, dateStr,
                                    timeValue, methodStr,crcStr));
        }

        fgets (lineString, len, fp);
    }

    return BZR_DONE;
}

//=============================================================================================================//

status_t RarArchiver::Open (entry_ref *ref, BMessage *fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo (ref);
    
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "v" << "-c-" << m_archivePath.Path();
    
    FILE *out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe (outdes, errdes);
    
    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle rar unloadable error here

    status_t exitCode;
    resume_thread (tid);
    
    close (errdes[1]);
    close (outdes[1]);

    out = fdopen (outdes[0], "r");
    exitCode = ReadOpen (out);
    
    close (outdes[0]);
    fclose (out);
    
    err = fdopen (errdes[0], "r");
    exitCode = Archiver::ReadErrStream (err, "- the file header is corrupt");
    close (errdes[0]);
    fclose (err);

    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::Extract (entry_ref *refToDir, BMessage *message, BMessenger *progress,
                      volatile bool *cancel)
{
    BEntry dirEntry;
    entry_ref dirRef;
    
    dirEntry.SetTo (refToDir);
    status_t exitCode = BZR_DONE;
    if (progress)        // Perform output directory checking only when a messenger is passed
    {
        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false)
           return BZR_EXTRACT_DIR_INIT_ERROR;
    }

    BPath dirPath (refToDir);
    BEntry archiveEntry (&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
        return BZR_ARCHIVE_PATH_INIT_ERROR;

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo (kPath, &type, &count);
        if (type != B_STRING_TYPE)
           return BZR_UNKNOWN;
    }
    
    // Setup argv, fill with selection names if needed
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "x" << "-idp" << "-c-";
    
    if (progress)        // Only enable extract options when user is NOT viewing
    {
        if (m_settingsMenu->FindItem(kNoOverwrite)->IsMarked())
           m_pipeMgr << "-o-";
        else if (m_settingsMenu->FindItem(kUpdate)->IsMarked())
           m_pipeMgr << "-u";
        else if (m_settingsMenu->FindItem(kFreshen)->IsMarked())
           m_pipeMgr << "-f";
        else
           m_pipeMgr << "-o+";
    }
    else
        m_pipeMgr << "-o+";
        
    if (m_settingsMenu->FindItem(kKeepBroken)->IsMarked())
        m_pipeMgr << "-kb";
    
    if (m_settingsMenu->FindItem(kProcessAttributes)->IsMarked() == false)
        m_pipeMgr << "-ee";
    
    if (Password().Length() > 0)
        m_pipeMgr << ((BString)"-p" << Password());
    else
        m_pipeMgr << "-p-";
    
    m_pipeMgr << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char *pathString = NULL;
        if (message->FindString (kPath, i, &pathString) == B_OK)
           m_pipeMgr << SupressWildcards (pathString);
    }

    BString dirStr = dirPath.Path();
    dirStr << '/';
    m_pipeMgr << dirStr.String();

    FILE *out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe (outdes, errdes);
    
    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;           // Handle rar unloadable error here

    if (progress)
        resume_thread (tid);
    else
    {
        status_t exitCode;
        wait_for_thread (tid, &exitCode);
    }
    
    close (errdes[1]);
    close (outdes[1]);

    if (progress)
    {
        out = fdopen (outdes[0], "r");
        exitCode = ReadExtract (out, progress, cancel);
        fclose (out);
    }

    err = fdopen (errdes[0], "r");
    BString errStreamOutput;
    Archiver::ReadStream (err, errStreamOutput);
    if (errStreamOutput.FindFirst ("Encrypted file:  CRC failed in ") > 0)
        exitCode = BZR_PASSWORD_ERROR;
        
    fclose (err);
    
    close (outdes[0]);
    close (errdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread (tid);
    
    m_pipeMgr.FlushArgs();
    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::ReadExtract (FILE *fp, BMessenger *progress, volatile bool *cancel)
{
    // Reads output of rar while extracting files and updates progress window (thru messenger)
    char lineString[728];

    // Prepare message to update the progress bar
    BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
    updateMessage.AddFloat ("delta", 1.0f);

    while (fgets (lineString, 727, fp))
    {
        if (cancel && *cancel == true)
           return BZR_CANCEL_ARCHIVER;

        lineString[strlen (lineString) - 1] = '\0';
        if (strncmp (lineString, "Extracting  ", 12) == 0)
        {
           BString lineStr = lineString;
           int32 found = lineStr.FindLast ("OK");
           lineStr.Remove (found, lineStr.Length() - found);
           
           const char *fileName = LeafFromPath (lineStr.String());
           if (fileName && strlen (fileName) > 0)
           {
               updateMessage.RemoveName ("text");
               updateMessage.AddString ("text", fileName);

               progress->SendMessage (&updateMessage, &reply);
           }
        }
    }

    return BZR_DONE;
}

//=============================================================================================================//

status_t RarArchiver::Test (char *&outputStr, BMessenger *progress, volatile bool *cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry (&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unrarPath << "t";;
    
    if (Password().Length() > 0)
        m_pipeMgr << ((BString)"-p" << Password());
    else
        m_pipeMgr << "-p-";
    
    m_pipeMgr << "-idp" << m_archivePath.Path();
    
    FILE *out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe (outdes, errdes);
    
    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle lha unloadable error here
        return B_ERROR;
    }

    resume_thread (tid);
    
    close (outdes[1]);
    out = fdopen (outdes[0], "r");
    status_t exitCode = ReadTest (out, outputStr, progress, cancel);
    close (errdes[1]);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen (errdes[0], "r");
        BString errStreamContent;
        Archiver::ReadStream(err, errStreamContent);
        if (errStreamContent.Length() > 0)
           exitCode = BZR_ERRSTREAM_FOUND;

        close (errdes[0]);
        close (outdes[0]);
        fclose (out);
        fclose (err);
        
        if (exitCode == BZR_ERRSTREAM_FOUND)
        {
           BString outStr = outputStr;
           delete[] outputStr;
           outStr << "\n" << errStreamContent << "\n";
           outputStr = new char[strlen (outStr.String()) + 1];
           strcpy (outputStr, outStr.String());
        }
    }
        
    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread (tid);
    
    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::ReadTest (FILE *fp, char *&outputStr, BMessenger *progress, volatile bool *cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of arj -t
    status_t exitCode = BZR_DONE;
    char lineString[999];
    int32 lineCount = -1;
    BString fullOutputStr;

    BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
    updateMessage.AddFloat ("delta", 1.0f);
    
    while (fgets (lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
           exitCode = BZR_CANCEL_ARCHIVER;
           break;
        }
        
        lineString[strlen (lineString) - 1] = '\0';
        fullOutputStr << lineString << "\n";
        lineCount++;
        
        // Skip first 4 line which contains Archive: <path of archive> | We don't need this here
        if (lineCount > 3)
        {
           char *testingStr = lineString;
           testingStr += CountCharsInFront (testingStr, ' ');
           
           if (strncmp (testingStr, "Testing     ", 12) == 0)
           {
               // The format of testingStr is now "Testing   path-here    OK"
               // Number of spaces between path and OK is not constant
               BString pathStr = testingStr;
               if (pathStr.FindLast ("OK") < 0)
                  exitCode = BZR_ERRSTREAM_FOUND;
               
               pathStr.RemoveLast ("OK");
               while (pathStr[pathStr.Length() - 1] == ' ')     // Trim right hand side spaces
                  pathStr.RemoveLast (" ");

               if (pathStr.ByteAt (pathStr.Length() - 1) != '/')
               {
                  updateMessage.RemoveName ("text");
                  updateMessage.AddString ("text", FinalPathComponent (pathStr.String() + 12));
                  progress->SendMessage (&updateMessage, &reply);
               }
           }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy (outputStr, fullOutputStr.String());
    
    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::GetComment (char *&commentStr)
{
    // Setup the comment retreiving process
    BString commentString;
    BEntry archiveEntry (&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        commentStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    BString tempFilePath = TempDirectoryPath();    // rar creates the path if its not there so we needn't bother
                                           // if it did not we would have to create it
    
    tempFilePath << "/" << m_archivePath.Leaf() << "_bzr_.txt";
    
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_rarPath << "cw" << "-y" << m_archivePath.Path() << tempFilePath.String();
    m_pipeMgr.Pipe();
    
    BFile tempFile (tempFilePath.String(), B_READ_ONLY);
    if (tempFile.InitCheck() == B_OK)
    {
        off_t fileSize;
        tempFile.GetSize (&fileSize);
        commentStr = new char [fileSize + 1];
        ssize_t bytesRead = tempFile.Read ((void*)commentStr, fileSize);
        if (bytesRead < 0)
           bytesRead = 0;

        commentStr[bytesRead] = '\0';
        return BZR_DONE;
    }
    else
        commentStr = NULL;
    
    return BZR_ERRSTREAM_FOUND;
}

//=============================================================================================================//

status_t RarArchiver::SetComment (char *commentStr, const char *tempDirPath)
{
    BEntry archiveEntry (&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        commentStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    BString outputString, tempFilePath = tempDirPath;
    tempFilePath << "/" << m_archivePath.Leaf() << "_bzr_.txt";
    
    BFile commentFile (tempFilePath.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
    if (commentStr)
        commentFile.Write (commentStr, strlen (commentStr));
    commentFile.Unset();
    
    BString param = "-z";
    param << tempFilePath;

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_rarPath << "c" << param.String() << "-y" << m_archivePath.Path();
    m_pipeMgr.Pipe ();

    return BZR_DONE;
}

//=============================================================================================================//

bool RarArchiver::SupportsComment () const
{
    return true;
}

//=============================================================================================================//

bool RarArchiver::CanPartiallyOpen() const
{
    // This is because we cannot determine the EXACT added-path because rar reports it with trailing spaces
    // ans we have no way of finding what spaces are added by rar and what belong to the end of a filename
    return false;
}

//=============================================================================================================//

status_t RarArchiver::Add (bool createMode, const char *relativePath, BMessage *message, BMessage *addedPaths,
                      BMessenger *progress, volatile bool *cancel)
{
    // Don't check if archive exists in createMode, otherwise check
    if (createMode == false)
    {
        BEntry archiveEntry (&m_archiveRef, true);
        if (archiveEntry.Exists() == false)
           return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    char level[12];
    BMenu *ratioMenu = m_settingsMenu->FindItem(kLevel0)->Menu();
    
    sprintf (level, "-m%ld", ratioMenu->IndexOf(ratioMenu->FindMarked()));
    m_pipeMgr << m_rarPath << "a" << "-ol" << "-c-" << "-ow" << "-idp" << "-o+" << level;

    if (m_settingsMenu->FindItem(kDisableSolidArk)->IsMarked())
        m_pipeMgr << "-s-";
    else
        m_pipeMgr << "-s";
    
    if (m_settingsMenu->FindItem(kMultimediaCompress)->IsMarked() == false)
        m_pipeMgr << "-mm";
    
    if (m_settingsMenu->FindItem(kRecoveryRecord)->IsMarked())
        m_pipeMgr << "-rr";

    if (m_settingsMenu->FindItem(kRecurse)->IsMarked())
        m_pipeMgr << "-r";

    if (Password().Length() > 0)
        m_pipeMgr << ((BString)"-p" << Password());
    
    m_pipeMgr << m_archivePath.Path();
    
    int32 count = 0L;
    uint32 type;
    message->GetInfo (kPath, &type, &count);
    if (type != B_STRING_TYPE)
        return BZR_UNKNOWN;

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char *pathString = NULL;
        if (message->FindString (kPath, i, &pathString) == B_OK)
           m_pipeMgr << pathString;
    }

    FILE *out;
    int outdes[2];

    if (relativePath)
        chdir (relativePath);

    thread_id tid = m_pipeMgr.Pipe (outdes);
    
    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle rar unloadable error here

    resume_thread (tid);
    close (outdes[1]);

    BString outputStr;
    out = fdopen (outdes[0], "r");
    status_t exitCode = ReadAdd (out, addedPaths, progress, cancel);
    close (outdes[0]);
    fclose (out);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread (tid);
    
    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::ReadAdd (FILE *fp, BMessage *addedPaths, BMessenger *progress, volatile bool *cancel)
{
    // Read output while adding files to archive
    status_t exitCode = BZR_DONE;
    char lineString[999];
    BMessage updateMessage (BZR_UPDATE_PROGRESS), reply ('DUMB');
    updateMessage.AddFloat ("delta", 1.0f);
    
    while (fgets (lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
           exitCode = BZR_CANCEL_ARCHIVER;
           break;
        }

        lineString[strlen (lineString) - 1] = '\0';
        if (strncmp (lineString, "Adding    ", 10) == 0 || strncmp (lineString, "Updating  ", 10) == 0)
        {
           BString filePath = lineString + 10;
           int32 openBraceIndex = filePath.FindLast ("OK");
           if (openBraceIndex > 0)
               filePath.Remove (openBraceIndex - 1, filePath.Length() - openBraceIndex + 1);
           else
           {
               // Check if it's a warning (reported in the NEXT LINE by rar)
               fgets (lineString, 998, fp);
               lineString[strlen (lineString) - 1] = '\0';
               BString tmpBuf = lineString;
               if (tmpBuf.IFindFirst ("WARNING:") < 0)        // otherwise nothing to worry about, its only warning
                  exitCode = BZR_ERRSTREAM_FOUND;
               else
               {
                  // Read subsequent line to see if it is OK
                  // this is an example of what is happening
                  // Adding    ToonsAndComics/Tinyportal0_75.zip
                  // WARNING: Cannot get ToonsAndComics/Tinyportal0_75.zip owner and group
                  //  OK
                  // This is what we are HANDLING and OVERRIDING/HACKING whatever to ignore the warning, 
                  // we are now about to read the " OK" line and see if it is OK, otherwise flag error
                  fgets (lineString, 998, fp);
                  lineString[strlen (lineString) - 1] = '\0';
                  tmpBuf = lineString;
                  if (tmpBuf.IFindFirst ("OK") < 0)
                      exitCode = BZR_ERRSTREAM_FOUND;           // not OK, meaning something is wrong!
               }
           }
           
           const char *fileName = FinalPathComponent (filePath.String());

           // Don't update progress bar for folders (damn !! damn !! rar does NOT
           // report a trailing slash for dirs so this below check fails and reports
           // dirs too to the progress - its not a real big bug but looks ugly when
           // the progress bar reports like 43 of 30 files due to directories --- upto rar authors
           if (fileName[strlen(fileName)-1] != '/' && progress)
           {
               updateMessage.RemoveName ("text");
               updateMessage.AddString ("text", fileName);
               progress->SendMessage (&updateMessage, &reply);
           }
           
           addedPaths->AddString (kPath, filePath.String());
        }
        else if (strstr (lineString, "- the file header is corrupt"))
        {
           // This will occur when a user tries to add files to a 3.x archive using 2.x rar version 
           // When the 3.x rar file is supported by password
           exitCode = BZR_ERROR;
        }
    }
    
    return exitCode;
}

//=============================================================================================================//

status_t RarArchiver::Delete (char *&outputStr, BMessage *message, BMessenger *progress,
                      volatile bool *cancel)
{
    return BZR_NOT_SUPPORTED;
}

//=============================================================================================================//

status_t RarArchiver::Create (BPath *archivePath, const char *relPath, BMessage *fileList, BMessage *addedPaths,
                             BMessenger *progress, volatile bool *cancel)
{
    // true=>normalize path, which means everything otherthan the leaf must exist,
    // meaning we have everything ready and only need to create the leaf (by add)
    m_archivePath.SetTo (archivePath->Path(), NULL, true);

    status_t result = Add (true, relPath, fileList, addedPaths, progress, cancel);
    
    // Once creating is done, set m_archiveRef to pointed to the existing archive file
    if (result == BZR_DONE)
    {
        BEntry tempEntry (m_archivePath.Path(), true);
        if (tempEntry.Exists())
           tempEntry.GetRef (&m_archiveRef);
        
        SetMimeType();
    }

    return result;
}

//=============================================================================================================//

void RarArchiver::BuildDefaultMenu ()
{
    BMenu *ratioMenu, *addMenu, *extractMenu, *othersMenu;
    BMenuItem *item;

    m_settingsMenu = new BMenu (m_typeStr);
    
    // Build the compression-level sub-menu (sorry we can't avoid using english strings here)
    ratioMenu = new BMenu (kCompressionLevel);
    ratioMenu->SetRadioMode (true);
    
    ratioMenu->AddItem (new BMenuItem (kLevel0, NULL));
    ratioMenu->AddItem (new BMenuItem (kLevel1, NULL));
    ratioMenu->AddItem (new BMenuItem (kLevel2, NULL));
    ratioMenu->AddItem (new BMenuItem (kLevel3, NULL));
    ratioMenu->AddItem (new BMenuItem (kLevel4, NULL));
    ratioMenu->AddItem (new BMenuItem (kLevel5, NULL));
    
    ratioMenu->FindItem (kLevel3)->SetMarked (true);

    // Build the "While Adding" sub-menu
    addMenu = new BMenu (kAdding);
    addMenu->SetRadioMode (false);

    item = new BMenuItem (kDisableSolidArk, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (true);
    addMenu->AddItem (item);

    item = new BMenuItem (kRecoveryRecord, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (true);
    addMenu->AddItem (item);

    item = new BMenuItem (kRecurse, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (true);
    addMenu->AddItem (item);
    
    item = new BMenuItem (kMultimediaCompress, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (false);
    addMenu->AddItem (item);

    // Build the "While Extracting" sub-menu
    extractMenu = new BMenu (kExtracting);
    extractMenu->SetRadioMode (true);

    item = new BMenuItem (kAlwaysOverwrite, NULL);
    item->SetMarked (true);
    extractMenu->AddItem (item);
    
    item = new BMenuItem (kNoOverwrite, NULL);
    item->SetMarked (false);
    extractMenu->AddItem (item);

    item = new BMenuItem (kUpdate, NULL);
    item->SetMarked (false);
    extractMenu->AddItem (item);

    item = new BMenuItem (kFreshen, NULL);
    item->SetMarked (false);
    extractMenu->AddItem (item);

    // Build the "Other Options" sub-menu
    othersMenu = new BMenu (kOtherOptions);
    othersMenu->SetRadioMode (false);

    item = new BMenuItem (kProcessAttributes, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (true);
    othersMenu->AddItem (item);

    item = new BMenuItem (kKeepBroken, new BMessage (BZR_MENUITEM_SELECTED));
    item->SetMarked (false);
    othersMenu->AddItem (item);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem (ratioMenu);
    m_settingsMenu->AddItem (addMenu);
    m_settingsMenu->AddItem (extractMenu);
    m_settingsMenu->AddItem (othersMenu);
}

//=============================================================================================================//

void RarArchiver::SetMimeType ()
{
    // It seems the lha binary resets the mime-type of archives to "application/octet-stream", after
    // operations such as delete, add, create - this function sets it back to the correct type
    BNode node(m_archivePath.Path());
    if (node.InitCheck() == B_OK)
    {
        BNodeInfo nodeInfo (&node);
        nodeInfo.SetType ((const char*)m_mimeList.ItemAtFast(0L));
    }
}

//=============================================================================================================//

bool RarArchiver::NeedsTempDirectory () const
{
    // The comments need temp directory
    return true;
}

//=============================================================================================================//

bool RarArchiver::SupportsPassword () const
{
    return true;
}

//=============================================================================================================//

bool RarArchiver::CanDeleteFiles () const
{
    // Currently deleting is weird in Rar because of solid blocks (whole block gets blown off)
    // hence we currently don't allow delete operations -- later change if needed
    return false;
}

//=============================================================================================================//
