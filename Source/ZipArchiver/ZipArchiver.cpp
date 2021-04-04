// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

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

#include "ZipArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "XzArchiver"
#else
#define B_TRANSLATE(x) x
#endif


#define S_NONE "(none)"
#define S_FASTEST "(fastest)"
#define S_BEST_DEFAULT "(best,default)"
#define S_DIR_RECURSE "Recurse into folders"
#define S_DIR_EXTRACT "Extract folders"
#define S_EXTRACT_ATTRS "Extract attributes"
#define S_NO_OVERWRITE "Never overwrite existing files"
#define S_UPDATE_FILES "Update files, create if needed"
#define S_FRESHEN_FILES "Freshen existing files, create none"



Archiver* load_archiver(const char* addonImagePath)
{
    return new ZipArchiver(addonImagePath);
}



ZipArchiver::ZipArchiver(const char* addonImagePath)
    : Archiver(addonImagePath)
{

    if (GetBinaryPath(m_unzipPath, "unzip") == true
        && GetBinaryPath(m_zipPath, "zip") == true)
        m_error = BZR_DONE;
    else
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }

    // Detect zipnote binary (optional, for zip comments)
    if (GetBinaryPath(m_zipnotePath, "zipnote") == false)
    {
        m_error = BZR_OPTIONAL_BINARY_MISSING;
        m_zipnotePath[0] = '\0';
    }
}



status_t ZipArchiver::ReadOpen(FILE* fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
         sizeStr[25], methodStr[25], packedStr[25], ratioStr[15], dayStr[5],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], crcStr[25],
         pathStr[B_PATH_NAME_LENGTH + 1];

    do {
        fgets(lineString, len, fp);
    } while (!feof(fp) && (strstr(lineString, "--------") == NULL));

    fgets(lineString, len, fp);

    while (!feof(fp) && (strstr(lineString, "--------") == NULL))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString,
               " %[0-9]  %[^ ] %[0-9]  %[^ ]  %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]  %[^ ]%[^\n]",
               sizeStr, methodStr, packedStr, ratioStr, monthStr, dayStr, yearStr, hourStr, minuteStr, crcStr, pathStr);

        const char *pathString = &pathStr[2];

        struct tm timeStruct;
        time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");

        if (StrEndsWith(pathString, "/"))
            m_entriesList.AddItem(new ArchiveEntry(true, pathString, sizeStr, packedStr, timeValue, methodStr, crcStr));
        else
            m_entriesList.AddItem(new ArchiveEntry(false, pathString, sizeStr, packedStr, timeValue, methodStr, crcStr));
 
        fgets(lineString, len, fp);
    }

    return BZR_DONE;
}



status_t ZipArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unzipPath << "-v" << "-q" << m_archivePath.Path();

    if (fileList)
    {
        uint32 type;
        int32 count;
        const char* path;
        fileList->GetInfo(kPath, &type, &count);

        for (int32 i = --count; i >= 0; i--)
            if (fileList->FindString(kPath, i, &path) == B_OK)
                m_pipeMgr << SupressWildcards(path);
    }

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle unzip unloadable error here

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    exitCode = ReadOpen(out);

    close(outdes[0]);
    fclose(out);

    err = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(err, ":  zipfile is empty");
    close(errdes[0]);
    fclose(err);

    return exitCode;
}



status_t ZipArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
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
    m_pipeMgr << m_unzipPath;
    if (m_settingsMenu->FindItem(B_TRANSLATE(S_EXTRACT_ATTRS))->IsMarked() == true)
        m_pipeMgr << "-o";
    else
        m_pipeMgr << "-Jo";

    // For normal quick-extraction i.e. with no Progressbar don't junk directories
    if (m_settingsMenu->FindItem(B_TRANSLATE(S_DIR_EXTRACT))->IsMarked() == false && progress != NULL)
        m_pipeMgr << "-j";

    if (progress)    // Use extract options only when user is NOT viewing
    {
        if (m_settingsMenu->FindItem(B_TRANSLATE(S_NO_OVERWRITE))->IsMarked() == true)
            m_pipeMgr << "-n";
        else if (m_settingsMenu->FindItem(B_TRANSLATE(S_UPDATE_FILES))->IsMarked() == true)
            m_pipeMgr << "-u";
        else if (m_settingsMenu->FindItem(B_TRANSLATE(S_FRESHEN_FILES))->IsMarked() == true)
            m_pipeMgr << "-f";
    }

    m_pipeMgr << m_archivePath.Path() << "-d" << dirPath.Path();

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
        return B_ERROR;           // Handle unzip unloadable error here

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



status_t ZipArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    // Reads output of unzip while extracting files and updates progress window (thru messenger)
    char lineString[728];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 727, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        // The following -2 is because we somehow get 2 characters extra after filename
        lineString[strlen(lineString) - 3] = '\0';

        // Later must handle "error" and "file #no: error at offset" strings in unzip output
        if (strncmp(lineString, "  inflating:", 12) == 0 ||
                strncmp(lineString, " extracting:", 12) == 0 ||
                strncmp(lineString, "    linking:", 12) == 0)
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", LeafFromPath(lineString));

            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}



status_t ZipArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unzipPath << "-t" << m_archivePath.Path();

    FILE* out;
    int outdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle unzip unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(outdes[1]);
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadTest(out, outputStr, progress, cancel);
    close(outdes[0]);
    fclose(out);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    return exitCode;
}



status_t ZipArchiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of unzip -t
    status_t exitCode = BZR_ERRSTREAM_FOUND;
    char lineString[999];
    int32 lineCount = -1;
    BString fullOutputStr;

    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);
    bool errFlag = false;

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

        // Skip first line which contains Archive: <path of archive> | We don't need this here
        if (lineCount > 0)
        {
            char* testingStr = lineString;
            testingStr += CountCharsInFront(testingStr, ' ');

            if (strncmp(testingStr, "testing:", 8) == 0)
            {
                // The format of testingStr is now "testing: path-here    OK"
                // Number of spaces between path and OK is 3, so simply
                // remove "OK" and "testing: ", then parse out just the filename from path
                BString pathStr = testingStr;
                pathStr.RemoveLast("OK");
                pathStr.RemoveLast("   ");
                while (pathStr[pathStr.Length() - 1] == ' ')     // Trim right hand side spaces
                    pathStr.RemoveLast(" ");

                if (pathStr.ByteAt(pathStr.Length() - 1) != '/')
                {
                    updateMessage.RemoveName("text");
                    updateMessage.AddString("text", FinalPathComponent(pathStr.String() + 9));
                    progress->SendMessage(&updateMessage, &reply);
                }
            }
            else if (strncmp(testingStr, "No errors detected in", 21) == 0 && errFlag == false)
            {
                // Must be the last line if no error && no errors detected before (ie errFlag = false)
                exitCode = BZR_DONE;
            }
            else
            {
                // Special check for empty zip :)
                if (strstr(testingStr, "zipfile is empty"))
                {
                    exitCode = BZR_DONE;
                    continue;
                }
                else if (strstr(testingStr, "No errors detected in compressed data of"))
                {
                    // Special check (sometimes comments may cause BZR_ERRSTREAM_FOUND, but if in
                    // the end if it says "No errors" then the file is OKAY.
                    exitCode = BZR_DONE;
                    continue;
                }
                else
                {
                    // uh oh! something wrong - set error found flag to true
                    errFlag = true;
                    exitCode = BZR_ERRSTREAM_FOUND;
                }
            }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());

    return exitCode;
}



status_t ZipArchiver::GetComment(char*& commentStr)
{
    // Setup the comment retreiving process
    BString commentString;
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        commentStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unzipPath << "-qq" << "-z" << m_archivePath.Path();

    FILE* out;
    int outdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        commentStr = NULL;        // Handle unzip unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(outdes[1]);
    out = fdopen(outdes[0], "r");
    ReadStream(out, commentString);
    close(outdes[0]);
    fclose(out);

    commentStr = new char [commentString.Length() + 1];
    strcpy(commentStr, commentString.String());

    return BZR_DONE;
}



status_t ZipArchiver::SetComment(char* commentStr, const char* tempDirPath)
{
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        commentStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    // Make zipnote write the existing comments out to the temp file in temp dir
    BString outputString, tempFilePath = tempDirPath;
    tempFilePath << "/" << m_archivePath.Leaf() << "_bzr_.txt";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_zipnotePath << m_archivePath.Path();

    FILE* out;
    int outdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle unzip unloadable error here

    resume_thread(tid);
    close(outdes[1]);

    // Read the current comment (if any) into outputString
    out = fdopen(outdes[0], "r");
    ReadStream(out, outputString);
    close(outdes[0]);
    fclose(out);

    // Replace outputString (current comment) with new comment
    const char* uniqueIDLine = "@ (zip file comment below this line)";
    int32 index = outputString.FindLast(uniqueIDLine);
    if (index)
        outputString.Remove(index, outputString.Length() - index);

    outputString << uniqueIDLine << "\n" << commentStr;

    // Write the new comment file back to the temp file
    std::fstream fileStream(tempFilePath.String(), std::ios::out);
    if (fileStream.is_open())
        fileStream << outputString.String();

    fileStream.close();

    // Make zipnote write to the zipfile from the temp file -- we use system() function
    // because zipnote reads from stdin ie using "<" character thus piping won't work
    BString commandStr = m_zipnotePath;
    commandStr << " -w \"" << m_archivePath.Path() << "\"" << " < " << "\"" << tempFilePath.String() << "\"";
    system(commandStr.String());

    return BZR_DONE;
}



bool ZipArchiver::SupportsComment() const
{
    // First let us check if the system has "zipnote" binary - this is what we will use to WRITE comments
    // to a zipfile - reading of comments from an existing zip file will be done using unzip itself
    return strlen(m_zipnotePath) > 0 ? true : false;
}



bool ZipArchiver::SupportsFolderEntity() const
{
    // This means zip binary will not delete entire folders given the folder name, we need to be
    // passed either a wildcard like folder/* or each entry inside the folder
    return false;
}



status_t ZipArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    // Don't check if archive exists in createMode, otherwise check
    if (createMode == false)
    {
        BEntry archiveEntry(&m_archiveRef, true);
        if (archiveEntry.Exists() == false)
            return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    char level[10];
    BString menuStr("0");
    menuStr << " " << B_TRANSLATE(S_NONE);
    BMenu* ratioMenu = m_settingsMenu->FindItem(menuStr)->Menu();

    sprintf(level, "-%ld", ratioMenu->IndexOf(ratioMenu->FindMarked()));
    m_pipeMgr << m_zipPath << "-y" << level;
    if (m_settingsMenu->FindItem(B_TRANSLATE(S_DIR_RECURSE))->IsMarked() == true)
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
        return B_ERROR;        // Handle zip unloadable error here

    resume_thread(tid);
    close(errdes[1]);
    close(outdes[1]);

    BString outputStr;
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadAdd(out, addedPaths, progress, cancel);
    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        err = fdopen(errdes[0], "r");
        exitCode = Archiver::ReadErrStream(err, "zip warning: couldn't write complete file type");
        close(errdes[0]);
        fclose(err);
    }
    close(outdes[0]);
    fclose(out);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}



status_t ZipArchiver::ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel)
{
    // Read output while adding files to archive
    status_t exitCode = BZR_DONE;
    char lineString[999];
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
        if (strncmp(lineString, "  adding:", 9) == 0 || strncmp(lineString, "updating:", 9) == 0)
        {
            BString filePath = lineString + 10;
            int32 openBraceIndex = filePath.FindLast('(');
            if (openBraceIndex != B_ERROR)
                filePath.Remove(openBraceIndex - 1, filePath.Length() - openBraceIndex + 1);

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



status_t ZipArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
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
    m_pipeMgr << m_zipPath << m_archivePath.Path() << "-d";

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcardSet(pathString);
        // Use SupressWildcardSet (which does not supress * character as zip needs * to delete folders fully)
    }

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle unzip unloadable error here
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
        BString errContent;
        Archiver::ReadStream(err, errContent);
        if (errContent.Length() > 1)
        {
            if (!(errContent.FindFirst("zip warning: zip file empty\n") >= 0
                    || errContent.FindFirst("zip warning: couldn't write complete file type") >= 0))
                exitCode = BZR_ERRSTREAM_FOUND;
        }

        close(errdes[0]);
        fclose(err);
    }
    close(outdes[0]);
    fclose(out);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}



status_t ZipArchiver::ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress,
                                 volatile bool* cancel)
{
    char lineString[999];
    BString fullStr;

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        fullStr.Append(lineString);

        // Later must handle "error" and "file #no: error at offset" strings in unzip output. We shall
        // do this as soon as we get an erroraneous zip file. If we code this now, we can't test it
        if (strncmp(lineString, "deleting:", 9) == 0)
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", FinalPathComponent(lineString + 9));
            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}



status_t ZipArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
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
    }

    return result;
}



void ZipArchiver::BuildDefaultMenu()
{
    BMenu* ratioMenu;
    BMenu* addMenu;
    BMenu* extractMenu;
    BMenuItem* item;

    m_settingsMenu = new BMenu(m_typeStr);

    // Build the compression-level sub-menu (sorry we can't avoid using english strings here)
    ratioMenu = new BMenu(B_TRANSLATE("Compression level"));
    ratioMenu->SetRadioMode(true);

    BString menuStr("0");
    menuStr << " " << B_TRANSLATE(S_NONE);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    menuStr = "1";
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    ratioMenu->AddItem(new BMenuItem("2", NULL));
    ratioMenu->AddItem(new BMenuItem("3", NULL));
    ratioMenu->AddItem(new BMenuItem("4", NULL));
    ratioMenu->AddItem(new BMenuItem("5", NULL));
    ratioMenu->AddItem(new BMenuItem("6", NULL));
    ratioMenu->AddItem(new BMenuItem("7", NULL));
    ratioMenu->AddItem(new BMenuItem("8", NULL));
    menuStr = "9";
    menuStr << " " << B_TRANSLATE(S_BEST_DEFAULT);
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    ratioMenu->AddItem(defaultItem);

    defaultItem->SetMarked(true);

    // Build the "While adding" sub-menu
    addMenu = new BMenu(B_TRANSLATE("While adding"));
    addMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE("Add attributes"), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    addMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(S_DIR_RECURSE), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    addMenu->AddItem(item);

    // Build the extract sub-menu
    extractMenu = new BMenu(B_TRANSLATE("While extracting"));
    extractMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE(S_EXTRACT_ATTRS), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    extractMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(S_DIR_EXTRACT), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    extractMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(S_NO_OVERWRITE), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    extractMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(S_UPDATE_FILES), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    extractMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(S_FRESHEN_FILES), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    extractMenu->AddItem(item);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(ratioMenu);
    m_settingsMenu->AddItem(addMenu);
    m_settingsMenu->AddItem(extractMenu);
}


