// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "LhaArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <NodeInfo.h>
#include <Messenger.h>
#include <MenuItem.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "LhaArchiver"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_MARK(x) x
#define B_TRANSLATE_NOCOLLECT(x) x
#endif


// keep track of our custom options/menuitems
static const char* kLharcCompat = B_TRANSLATE_MARK("LHarc compatible format");


Archiver* load_archiver(const char* addonImagePath)
{
    return new LhaArchiver(addonImagePath);
}


LhaArchiver::LhaArchiver(const char* addonImagePath)
    : Archiver(addonImagePath)
{
    // Detect lha binary
    if (GetBinaryPath(m_lhaPath, "lha") == true)
        m_error = BZR_DONE;
    else
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}


status_t LhaArchiver::ReadOpen(FILE* fp)
{
    char lineString[B_PATH_NAME_LENGTH + 512],
         permStr[25], sizeStr[25], uidgidStr[20], methodStr[25], packedStr[25], ratioStr[15], dayStr[5],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], secondStr[5], crcStr[25],
         pathStr[B_PATH_NAME_LENGTH + 1];
    uint16 const len = sizeof(lineString);


    // Lha doesn't report the time for files (only date) hence we use the modification time of the
    // archive along with the corresponding date reported by lha
    time_t const modTime = ArchiveModificationTime();
    tm mod_tm;
    localtime_r(&modTime, &mod_tm);
    sprintf(hourStr, "%d", mod_tm.tm_hour);
    sprintf(minuteStr, "%d", mod_tm.tm_min);
    sprintf(secondStr, "%d", mod_tm.tm_sec);

    do
    {
        fgets(lineString, len, fp);
    } while (!feof(fp) && (strstr(lineString, "----------") == NULL));

    fgets(lineString, len, fp);

    while (!feof(fp) && (strstr(lineString, "----------") == NULL))
    {
        lineString[strlen(lineString) - 1] = '\0';

        if (strncmp(lineString, "[generic]", 9) == 0)
        {
            sscanf(lineString,
                   "%[^ ] %[0-9] %[0-9] %[^ ] %[^ ] %[^ ] %[^ ] %[^ ] %[^ ]%[^\n]", permStr, packedStr, sizeStr,
                   ratioStr, methodStr, crcStr, monthStr, dayStr, yearStr, pathStr);
        }
        else
        {
            sscanf(lineString,
                   "%[^ ] %[^ ] %[0-9] %[0-9] %[^ ] %[^ ] %[^ ] %[^ ] %[0-9] %[^ ]%[^\n]",
                   permStr, uidgidStr, packedStr, sizeStr, ratioStr, methodStr, crcStr, monthStr, dayStr, yearStr,
                   pathStr);
        }

        // Workaround bug fix for files/folder with space before them
        BString pathString = pathStr;
        pathString.Remove(0, 1);

        BString monthStrCorrect;
        monthStrCorrect << MonthStrToNum(monthStr);

        // Stupid lha! for directories and some other files reports time in-place of year,
        // fix that with current system year
        if (strstr(yearStr, ":"))
            sprintf(yearStr, "%d", mod_tm.tm_year);

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, (char*)monthStrCorrect.String(), yearStr, hourStr,
                 minuteStr, secondStr);

        // Check to see if last char of pathStr = '/' add it as folder, else as a file
        uint16 pathLength = pathString.Length() - 1;
        if (pathString[pathLength] == '/' || permStr[0] == 'd')
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, packedStr, timeValue, methodStr, crcStr));
        else
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, packedStr, timeValue, methodStr, crcStr));

        fgets(lineString, len, fp);
    }

    return BZR_DONE;
}


status_t LhaArchiver::Open(entry_ref* ref, BMessage* /*fileList*/)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_lhaPath << "-v" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle lha unloadable error here

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


status_t LhaArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
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
    BString comboArg = "-xfw="; comboArg << dirPath.Path();

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_lhaPath << comboArg.String() << m_archivePath.Path();

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
        return B_ERROR;           // Handle lha unloadable error here

    if (progress)
        resume_thread(tid);
    else
    {
        status_t threadExitCode;
        wait_for_thread(tid, &threadExitCode);
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


status_t LhaArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    // Reads output of lha while extracting files and updates progress window (thru messenger)
    char lineString[928];
    BString buf;

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 927, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        buf = lineString;
        int32 found = buf.FindLast("- Melting  :");

        // Don't report making of directories (only files)
        if (found > 0)
        {
            buf.Truncate(found - 1);
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", LeafFromPath(buf.String()));
            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}


status_t LhaArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_lhaPath << "-t" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle lha unloadable error here
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


status_t LhaArchiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of lha -t
    status_t exitCode = BZR_DONE;
    BString fullOutputStr;

    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (1)
    {
        BString buf;
        int c;
        while (1)
        {
            c = fgetc(fp);
            if (c == EOF || c == '\r' || c == '\n')
                break;
            buf << (char)c;
        }
        if (c == EOF)
            break;

        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        if (c == '\r')
            continue;
        else if (c == '\n')
        {
            fullOutputStr << buf.String() << "\n";
            int32 found = buf.FindLast("- Tested");

            if (found > 0)
            {
                buf.Truncate(found - 1);
                updateMessage.RemoveName("text");
                updateMessage.AddString("text", LeafFromPath(buf.String()));
                progress->SendMessage(&updateMessage, &reply);
            }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());

    return exitCode;
}


bool LhaArchiver::SupportsFolderEntity() const
{
    // This means lha binary will not delete entire folders given the folder name, we need to be
    // passed either a wildcard like folder/* or each entry inside the folder
    return false;
}


bool LhaArchiver::CanPartiallyOpen() const
{
    // Here is why we cannot partially open - the lha binary does NOT report adding of folders (or its
    // content) when both files & folders are being added, hence we need to reload each time - oh well
    return false;
}


status_t LhaArchiver::Add(bool /*createMode*/, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    // Don't EVER check if archive exist (FOR LHA ONLY) this is because when all files from an open lha ark are
    // deleted, (lha binary) deletes the archive itself
    m_pipeMgr.FlushArgs();

    BString buf = "-a";
    if (m_settingsMenu->FindItem(B_TRANSLATE_NOCOLLECT(kLharcCompat))->IsMarked() == true)
        buf << "g";

    buf << GetCompressionLevel();
    m_pipeMgr << m_lhaPath << buf.String() << m_archivePath.Path();

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
        return B_ERROR;        // Handle lha unloadable error here

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


status_t LhaArchiver::ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel)
{
    // Read output while adding files to archive
    status_t exitCode = BZR_DONE;
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (1)
    {
        BString buf;
        int c;
        while (1)
        {
            c = fgetc(fp);
            if (c == EOF || c == '\r' || c == '\n')
                break;
            buf << (char)c;
        }
        if (c == EOF)
            break;

        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        if (c == '\r')
            continue;
        else if (c == '\n')
        {
            int32 found = buf.FindLast("- Frozen(");
            if (found > 0)
            {
                buf.Truncate(found - 1);
                addedPaths->AddString(kPath, buf.String());
                updateMessage.RemoveName("text");
                updateMessage.AddString("text", LeafFromPath(buf.String()));
                progress->SendMessage(&updateMessage, &reply);
            }
        }
    }

    return exitCode;
}


status_t LhaArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
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
    m_pipeMgr << m_lhaPath << "-d" << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcardSet(pathString);
        // Use SupressWildcardSet (which does not supress * character as lha needs * to delete folders fully)
    }

    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle lha unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    FILE *outFile = fdopen(outdes[0], "r");
    status_t exitCode = ReadDelete(outFile, outputStr, progress, cancel);
    fclose(outFile);

    if (exitCode != BZR_CANCEL_ARCHIVER)
    {
        FILE *errFile = fdopen(errdes[0], "r");
        exitCode = Archiver::ReadErrStream(errFile);
        fclose(errFile);
    }

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}


status_t LhaArchiver::ReadDelete(FILE* fp, char*& /*outputStr*/, BMessenger* progress,
                                 volatile bool* cancel)
{
    char lineString[B_PATH_NAME_LENGTH + sizeof("Delete ") + 2];
    int32 const len = sizeof(lineString);

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, len, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        if (strncmp(lineString, "delete ", 7) == 0)
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", FinalPathComponent(lineString + 7));
            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}


status_t LhaArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
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


void LhaArchiver::BuildMenu(BMessage& message)
{
    BMenu* otherMenu;
    BMenuItem* item;

    m_settingsMenu = new BMenu(m_typeStr);

    // Build the header-level sub-menu
    m_compressionMenu = new BMenu(B_TRANSLATE("Header level"));
    m_compressionMenu->SetRadioMode(true);

    BString menuStr("0");
    menuStr << " " << B_TRANSLATE("(fastest)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));
    m_compressionMenu->AddItem(new BMenuItem("1", NULL));
    menuStr = "2";
    menuStr << " " << B_TRANSLATE("(best)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));

    SetCompressionLevel(message.GetInt32(kCompressionLevelKey, 2));

    // Build the "Other options" sub-menu
    otherMenu = new BMenu(B_TRANSLATE("Other settings"));
    otherMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE_NOCOLLECT(kLharcCompat), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(message.GetBool(kLharcCompat, false));
    otherMenu->AddItem(item);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(m_compressionMenu);
    m_settingsMenu->AddItem(otherMenu);
}


status_t LhaArchiver::ArchiveSettings(BMessage& message)
{
    BMenuItem* item = m_settingsMenu->FindItem(B_TRANSLATE_NOCOLLECT(kLharcCompat));
    if (item != NULL)
        message.AddBool(kLharcCompat, item->IsMarked());

    return Archiver::ArchiveSettings(message);
}


void LhaArchiver::SetMimeType()
{
    // It seems the lha binary resets the mime-type of archives to "application/octet-stream", after
    // operations such as delete, add, create - this function sets it back to the correct type
    BNode node(m_archivePath.Path());
    if (node.InitCheck() == B_OK)
    {
        BNodeInfo nodeInfo(&node);
        nodeInfo.SetType((const char*)m_mimeList.ItemAtFast(0L));
    }
}
