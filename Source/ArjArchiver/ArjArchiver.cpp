/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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

#include "ArjArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BZipArchiver"
#else
#define B_TRANSLATE(x) x
#endif


#define S_NONE "(none)"
#define S_FASTEST "(fastest)"
#define S_BEST_DEFAULT "(best,default)"
#define S_RECURSE_DIRS "Recurse into folders"
#define S_UPDATE_FILES "Update files (new and newer)"
#define S_FRESHEN_FILES "Freshen existing files"
#define S_MULTI_VOLUMES "Enable multiple volumes"



Archiver* load_archiver()
{
    return new ArjArchiver();
}



ArjArchiver::ArjArchiver()
{
    // The list of supported mimetypes by this add-on, note the first (index 0) mime-type
    // in the list will be the one that will be used while creating files using this add-on
    m_mimeList.AddItem(strdup("application/x-arj-compressed"));
    m_mimeList.AddItem(strdup("application/x-arj"));
    m_mimeList.AddItem(strdup("application/arj"));
    SetArchiveType("arj");
    SetArchiveExtension(".arj");

    m_error = BZR_DONE;
    // Detect arj binary
    if (IsBinaryFound(m_arjPath, BZR_ARK) == false)
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}



status_t ArjArchiver::ReadOpen(FILE* fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len], fileCount[20], revisionStr[20],
         sizeStr[25], packedStr[25], ratioStr[15], dayStr[5], permStr[50],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], secondStr[5], guaStr[25],
         bpmgsStr[20], osStr[30], osStr2[15], pathStr[B_PATH_NAME_LENGTH + 1];

    do
    {
        fgets(lineString, len, fp);
    }
    while (!feof(fp) && (strstr(lineString, "--------") == NULL));

    fgets(lineString, len, fp);

    while (!feof(fp) && (strstr(lineString, "--------") == NULL))
    {
        lineString[strlen(lineString) - 1] = '\0';

        // Arj reports in 4 lines, first line for just the path, the next line has columnar info
        sscanf(lineString,    "%[^ ] %[^\n]", fileCount, pathStr);
        fgets(lineString, len, fp);
        sscanf(lineString,
               " %[0-9] %[^ ] %[0-9] %[0-9] %[^ ] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]:%[0-9] %[^ ] %[^ ] %[^\n]",
               revisionStr, osStr,
               sizeStr, packedStr, ratioStr, yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr,
               permStr, guaStr, bpmgsStr);

        // Possible host OS strings from code of ARJ port from 3dEyes -- thanks to him for giving me this info!
        // static char *host_os_names[]={"MS-DOS", "PRIMOS", "UNIX", "AMIGA", "MAC-OS",
        //                  "OS/2", "APPLE GS", "ATARI ST", "NEXT", "VAX VMS", "WIN95", "WIN32", NULL};
        if (strcmp(osStr, "ATARI") == 0 || strcmp(osStr, "APPLE") == 0 ||
                strcmp(osStr, "VAX") == 0)         // include those with spaces
        {
            // We need to re-read the lineString to include an extra column because of the space (i.e. osStr2)
            sscanf(lineString,
                   " %[0-9] %[^ ] %[^ ] %[0-9] %[0-9] %[^ ] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]:%[0-9] %[^ ] %[^ ] %[^\n]",
                   revisionStr, osStr2, osStr2,
                   sizeStr, packedStr, ratioStr, yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr,
                   permStr, guaStr, bpmgsStr);

            strcat(osStr, " ");
            strcat(osStr, osStr2);
        }

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");

        BString pathString = pathStr;

        // Check to see if pathStr is as folder, else add it as a file
        if (strstr(permStr, "D") != NULL || permStr[0] == 'd')
        {
            // Critical we add '/' for empty folders as rar doesn't report folder names with '/'
            pathString << '/';
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, packedStr, timeValue, "-", "-"));
        }
        else
        {
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, packedStr, timeValue, "-", "-"));
        }

        fgets(lineString, len, fp);
        fgets(lineString, len, fp);
        fgets(lineString, len, fp);
    }

    return BZR_DONE;
}



status_t ArjArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_arjPath << "v" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle arj unloadable error here

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    exitCode = ReadOpen(out);

    close(outdes[0]);
    fclose(out);

    err = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(err);
    close(errdes[0]);
    fclose(err);

    return exitCode;
}



status_t ArjArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                              volatile bool* cancel)
{
    BEntry dirEntry;
    entry_ref dirRef;

    dirEntry.SetTo(refToDir);
    status_t exitCode = BZR_DONE;
    if (progress)        // Perform output directory checking only when a messenger is passed
    {
        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false)
            return BZR_EXTRACT_DIR_INIT_ERROR;
    }

    BPath dirPath(refToDir);
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
        return BZR_ARCHIVE_PATH_INIT_ERROR;

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo(kPath, &type, &count);
        if (type != B_STRING_TYPE)
            return BZR_UNKNOWN;
    }

    // Setup argv, fill with selection names if needed
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_arjPath << "x" << "-y" << "-i";

    // For quick-extraction (that is - viewing etc) don't process the below switches
    if (progress)
    {
        if (m_settingsMenu->FindItem(B_TRANSLATE(S_UPDATE_FILES))->IsMarked() == true)
            m_pipeMgr << "-u";
        else if (m_settingsMenu->FindItem(B_TRANSLATE(S_FRESHEN_FILES))->IsMarked() == true)
            m_pipeMgr << "-f";
    }

    if (m_settingsMenu->FindItem(B_TRANSLATE(S_MULTI_VOLUMES))->IsMarked() == true)
        m_pipeMgr << "-v";

    m_pipeMgr << m_archivePath.Path() << dirPath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcards(pathString);
    }

    FILE* out;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;           // Handle arj unloadable error here

    if (progress)
        resume_thread(tid);
    else
    {
        status_t exitCode;
        wait_for_thread(tid, &exitCode);
    }

    close(errdes[1]);
    close(outdes[1]);

    if (progress)
    {
        out = fdopen(outdes[0], "r");
        exitCode = ReadExtract(out, progress, cancel);
        fclose(out);
    }

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}



status_t ArjArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    // Reads output of arj while extracting files and updates progress window (thru messenger)
    char lineString[928];
    BString buf;

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 727, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';

        // Later must handle "error" and "file #no: error at offset" strings in unzip output
        // Line format is as follows:
        // Extracting pictures/Batio.jpg        to /boot/home/temp/ax/pictures/Batio.jpg  OK
        // Extracting pictures/ue3.jpg          to /boot/home/temp/ax/pictures/ue3.jpg  OK
        if (strncmp(lineString, "Extracting ", 11) == 0)
        {
            // Remove the OK first, then the "to /", because OK will always exists (except in case of error)
            // and the "to" part will exist when dest dir is specified
            BString lineStr = lineString;
            int32 index = lineStr.FindLast("OK");
            if (index > 0)
                lineStr.Remove(index, lineStr.Length() - index);

            index = lineStr.FindLast("to /");
            if (index > 0)
                lineStr.Remove(index, lineStr.Length() - index);

            while (lineStr[lineStr.Length() - 1] == ' ')     // Trim right hand side spaces
                lineStr.RemoveLast(" ");

            updateMessage.RemoveName("text");
            updateMessage.AddString("text", LeafFromPath(lineStr.String()));

            progress->SendMessage(&updateMessage, &reply);
        }
    }


    return BZR_DONE;
}



status_t ArjArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_arjPath << "t" << "-i" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle arj unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(outdes[1]);
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadTest(out, outputStr, progress, cancel);
    close(errdes[1]);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen(errdes[0], "r");
        BString errStreamContent;
        Archiver::ReadStream(err, errStreamContent);
        if (errStreamContent.Length() > 0)
            exitCode = BZR_ERRSTREAM_FOUND;

        close(errdes[0]);
        close(outdes[0]);
        fclose(out);
        fclose(err);

        if (exitCode == BZR_ERRSTREAM_FOUND)
        {
            BString outStr = outputStr;
            delete[] outputStr;
            outStr << "\n" << errStreamContent << "\n";
            outputStr = new char[strlen(outStr.String()) + 1];
            strcpy(outputStr, outStr.String());
        }
    }

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    return exitCode;
}



status_t ArjArchiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of arj -t
    status_t exitCode = BZR_DONE;
    char lineString[999];
    int32 lineCount = -1;
    BString fullOutputStr;

    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        lineString[strlen(lineString) - 1] = '\0';
        fullOutputStr << lineString << "\n";
        lineCount++;

        // Skip first 4 line which contains Archive: <path of archive> | We don't need this here
        if (lineCount > 3)
        {
            char* testingStr = lineString;
            testingStr += CountCharsInFront(testingStr, ' ');

            if (strncmp(testingStr, "Testing ", 8) == 0)
            {
                // The format of testingStr is now "Testing path-here    OK"
                // Number of spaces between path and OK is not constant
                BString pathStr = testingStr;
                if (pathStr.FindLast("OK") < 0)
                    exitCode = BZR_ERRSTREAM_FOUND;

                pathStr.RemoveLast("OK");
                while (pathStr[pathStr.Length() - 1] == ' ')     // Trim right hand side spaces
                    pathStr.RemoveLast(" ");

                if (pathStr.ByteAt(pathStr.Length() - 1) != '/')
                {
                    updateMessage.RemoveName("text");
                    updateMessage.AddString("text", FinalPathComponent(pathStr.String() + 9));
                    progress->SendMessage(&updateMessage, &reply);
                }
            }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());

    return exitCode;
}



bool ArjArchiver::SupportsFolderEntity() const
{
    return false;
}



bool ArjArchiver::CanPartiallyOpen() const
{
    // Here is why we cannot partially open - the arj binary does NOT report adding of folders (or its
    // content) when both files & folders are being added, hence we need to reload each time - oh well
    return false;
}



status_t ArjArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    // Don't EVER check if archive exist (FOR ARJ ONLY) this is because when all files from an open arj ark are
    // deleted, (arj binary) deletes the archive itself
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_arjPath << "a" << "-i";

    // Add addtime options
    BString noneMenuStr("0");
    noneMenuStr << " " << B_TRANSLATE(S_NONE);
    BString fastestMenuStr("4");
    fastestMenuStr << " " << B_TRANSLATE(S_FASTEST);
    BMenu* ratioMenu = m_settingsMenu->FindItem(noneMenuStr)->Menu();
    BString level = ratioMenu->FindMarked()->Label();
    if (level == noneMenuStr)
        level = "-m0";
    else if (level == "2")
        level = "-m4";
    else if (level == "3")
        level = "-m3";
    else if (level == fastestMenuStr)
        level = "-m2";
    else
        level = "-m1";

    m_pipeMgr << level.String();

    if (m_settingsMenu->FindItem(B_TRANSLATE(S_RECURSE_DIRS))->IsMarked())
        m_pipeMgr << "-r";

    m_pipeMgr << m_archivePath.Path();

    int32 count = 0L;
    uint32 type;
    message->GetInfo(kPath, &type, &count);
    if (type != B_STRING_TYPE)
        return BZR_UNKNOWN;

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << pathString;
    }

    FILE* out, *err;
    int outdes[2], errdes[2];

    if (relativePath)
        chdir(relativePath);

    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle arj unloadable error here

    resume_thread(tid);
    close(errdes[1]);
    close(outdes[1]);

    BString outputStr;
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadAdd(out, addedPaths, progress, cancel);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen(errdes[0], "r");
        exitCode = Archiver::ReadErrStream(err);
        close(errdes[0]);
        fclose(err);
    }
    close(outdes[0]);
    fclose(out);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}



status_t ArjArchiver::ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel)
{
    // Read output while adding files to archive
    status_t exitCode = BZR_DONE;
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);
    char lineString[999];

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        lineString[strlen(lineString) - 1] = '\0';
        if (strncmp(lineString, "Adding", 6) == 0 || strncmp(lineString, "Replacing", 9) == 0)
        {
            BString filePath = lineString + 10;
            int32 index = filePath.FindLast('%');
            if (index > 7)
                filePath.Remove(index - 5, 6);
            else
                exitCode = BZR_ERRSTREAM_FOUND;

            while (filePath[filePath.Length() - 1] == ' ')     // Trim right hand side spaces
                filePath.RemoveLast(" ");

            const char* fileName = FinalPathComponent(filePath.String());

            // Don't update progress bar for folders
            if (fileName[strlen(fileName) - 1] != '/' && progress)
            {
                updateMessage.RemoveName("text");
                updateMessage.AddString("text", fileName);
                progress->SendMessage(&updateMessage, &reply);
            }

            addedPaths->AddString(kPath, filePath.String());
        }
    }

    return exitCode;
}



status_t ArjArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
                             volatile bool* cancel)
{
    // Setup deleting process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo(kPath, &type, &count);
        if (type != B_STRING_TYPE)
            return BZR_UNKNOWN;
    }

    m_pipeMgr.FlushArgs();
    // Added -i, -y switches 0.06
    m_pipeMgr << m_arjPath << "d" << "-i" << "-y" << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcardSet(pathString);
        // Use SupressWildcardSet (which does not supress * character as arj needs * to delete folders fully)
    }

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle arj unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadDelete(out, outputStr, progress, cancel);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen(errdes[0], "r");
        exitCode = Archiver::ReadErrStream(err);
        close(errdes[0]);
        fclose(err);
    }
    close(outdes[0]);
    fclose(out);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}



status_t ArjArchiver::ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress,
                                 volatile bool* cancel)
{
    int32 len = B_PATH_NAME_LENGTH + strlen("Deleting ") + 2;
    char lineString[len];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, len - 1, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        if (strncmp(lineString, "Deleting ", 9) == 0)
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", FinalPathComponent(lineString + 9));
            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}



status_t ArjArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                             BMessenger* progress, volatile bool* cancel)
{
    // true=>normalize path, which means everything otherthan the leaf must exist,
    // meaning we have everything ready and only need to create the leaf (by add)
    m_archivePath.SetTo(archivePath->Path(), NULL, true);

    status_t result = Add(true, relPath, fileList, addedPaths, progress, cancel);

    // Once creating is done, set m_archiveRef to pointed to the existing archive file
    if (result == BZR_DONE)
    {
        BEntry tempEntry(m_archivePath.Path(), true);
        if (tempEntry.Exists())
            tempEntry.GetRef(&m_archiveRef);

        SetMimeType();
    }

    return result;
}



void ArjArchiver::BuildDefaultMenu()
{
    BMenu* extractMenu;
    BMenu* ratioMenu;
    BMenu* addMenu;
    BMenuItem* item;
    m_settingsMenu = new BMenu(m_typeStr);

    // Build the compression-level sub-menu (sorry we can't avoid using english strings here)
    ratioMenu = new BMenu(B_TRANSLATE("Compression level"));
    ratioMenu->SetRadioMode(true);

    BString menuStr("0");
    menuStr << " " << B_TRANSLATE(S_NONE);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    menuStr = "4";
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    ratioMenu->AddItem(new BMenuItem("3", NULL));
    ratioMenu->AddItem(new BMenuItem("2", NULL));
    menuStr = "1";
    menuStr << " " << B_TRANSLATE(S_BEST_DEFAULT);
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    ratioMenu->AddItem(defaultItem);

    defaultItem->SetMarked(true);

    // Build the "While adding" sub-menu
    addMenu = new BMenu(B_TRANSLATE("While adding"));
    addMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE(S_RECURSE_DIRS), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    addMenu->AddItem(item);

    // Build the extract sub-menu
    extractMenu = new BMenu(B_TRANSLATE("While extracting"));
    extractMenu->SetRadioMode(false);

    extractMenu->AddItem(new BMenuItem(B_TRANSLATE(S_UPDATE_FILES), new BMessage(BZR_MENUITEM_SELECTED)));
    extractMenu->AddItem(new BMenuItem(B_TRANSLATE(S_FRESHEN_FILES), new BMessage(BZR_MENUITEM_SELECTED)));
    item = new BMenuItem(B_TRANSLATE(S_MULTI_VOLUMES), new BMessage(BZR_MENUITEM_SELECTED));
    extractMenu->AddItem(item);
    item->SetMarked(true);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(ratioMenu);
    m_settingsMenu->AddItem(addMenu);
    m_settingsMenu->AddItem(extractMenu);
}



void ArjArchiver::SetMimeType()
{
    // It seems the arj binary resets the mime-type of archives to "application/octet-stream", after
    // operations such as delete, add, create - this function sets it back to the correct type
    BNode node(m_archivePath.Path());
    if (node.InitCheck() == B_OK)
    {
        BNodeInfo nodeInfo(&node);
        nodeInfo.SetType((const char*)m_mimeList.ItemAtFast(0L));
    }
}



bool ArjArchiver::CanAddEmptyFolders() const
{
    // :( this is courtesy of the arj format itself
    return false;
}



BList ArjArchiver::HiddenColumns(BList* columns) const
{
    // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
    // Tar.GZip or Tar don't have 2,3,6,7 -- Pure Gzip have all
    BList availList(*columns);
    availList.RemoveItems(0, 6);     // Remove 0..5

    // Now list has 0-method 1-crc <-- these columns are to be hidden
    return availList;
}


