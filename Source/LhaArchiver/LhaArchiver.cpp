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
#endif


#define S_FASTEST "(fastest)"
#define S_BEST "(best)"
#define S_LHARC_COMPAT "LHarc compatible format"



Archiver* load_archiver()
{
    return new LhaArchiver();
}



LhaArchiver::LhaArchiver()
{
    // The list of supported mimetypes by this add-on, note the first (index 0) mime-type
    // in the list will be the one that will be used while creating files using this add-on
    m_mimeList.AddItem(strdup("application/x-lzh"));
    m_mimeList.AddItem(strdup("application/x-lharc"));
    m_mimeList.AddItem(strdup("application/lzh"));
    m_mimeList.AddItem(strdup("application/lharc"));
    SetArchiveType("lha");
    SetArchiveExtension(".lzh");

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
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
         permStr[25], sizeStr[25], uidgidStr[20], methodStr[25], packedStr[25], ratioStr[15], dayStr[5],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], secondStr[5], crcStr[25],
         pathStr[B_PATH_NAME_LENGTH + 1];

    // Lha doesn't report the time for files (only date) hence we use the modification time of the
    // archive along with the corresponding date reported by lha
    time_t modTime;
    tm mod_tm;
    BEntry archiveEntry(m_archivePath.Path(), true);
    archiveEntry.GetModificationTime(&modTime);
    localtime_r(&modTime, &mod_tm);
    sprintf(hourStr, "%d", mod_tm.tm_hour);
    sprintf(minuteStr, "%d", mod_tm.tm_min);
    sprintf(secondStr, "%d", mod_tm.tm_sec);

    do
    {
        fgets(lineString, len, fp);
    }
    while (!feof(fp) && (strstr(lineString, "----------") == NULL));

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
        {
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, packedStr, timeValue, methodStr, crcStr));
        }
        else
        {
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, packedStr, timeValue, methodStr, crcStr));
        }

        fgets(lineString, len, fp);
    }

    return BZR_DONE;
}



status_t LhaArchiver::Open(entry_ref* ref, BMessage* fileList)
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



status_t LhaArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    // Don't EVER check if archive exist (FOR LHA ONLY) this is because when all files from an open lha ark are
    // deleted, (lha binary) deletes the archive itself
    m_pipeMgr.FlushArgs();
    char level[10];
    BString menuStr("0");
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    BMenu* ratioMenu = m_settingsMenu->FindItem(menuStr)->Menu();

    BString buf = "-a";
    if (m_settingsMenu->FindItem(B_TRANSLATE(S_LHARC_COMPAT))->IsMarked() == true)
        buf << "g";

    sprintf(level, "%ld", ratioMenu->IndexOf(ratioMenu->FindMarked()));
    buf << level;
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

    FILE* out, *err;
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



status_t LhaArchiver::ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress,
                                 volatile bool* cancel)
{
    int32 len = B_PATH_NAME_LENGTH + strlen("Delete ") + 2;
    char lineString[len];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, len - 1, fp))
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



int8 LhaArchiver::MonthStrToNum(const char* month) const
{
    // Converts "Jan", "Feb" etc to 1, 2 etc.
    if (strcasecmp(month, "Jan") == 0) return 1;
    if (strcasecmp(month, "Feb") == 0) return 2;
    if (strcasecmp(month, "Mar") == 0) return 3;
    if (strcasecmp(month, "Apr") == 0) return 4;
    if (strcasecmp(month, "May") == 0) return 5;
    if (strcasecmp(month, "Jun") == 0) return 6;
    if (strcasecmp(month, "Jul") == 0) return 7;
    if (strcasecmp(month, "Aug") == 0) return 8;
    if (strcasecmp(month, "Sep") == 0) return 9;
    if (strcasecmp(month, "Oct") == 0) return 10;
    if (strcasecmp(month, "Nov") == 0) return 11;
    if (strcasecmp(month, "Dec") == 0) return 12;

    return 1;
}



void LhaArchiver::BuildDefaultMenu()
{
    BMenu* ratioMenu;
    BMenu* otherMenu;
    BMenuItem* item;

    m_settingsMenu = new BMenu(m_typeStr);

    // Build the header-level sub-menu
    ratioMenu = new BMenu(B_TRANSLATE("Header level"));
    ratioMenu->SetRadioMode(true);

    BString menuStr("0");
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    ratioMenu->AddItem(new BMenuItem("1", NULL));
    menuStr = "2";
    menuStr << " " << B_TRANSLATE(S_BEST);
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));

    defaultItem->SetMarked(true);

    // Build the "Other options" sub-menu
    otherMenu = new BMenu(B_TRANSLATE("Other settings"));
    otherMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE(S_LHARC_COMPAT), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    otherMenu->AddItem(item);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(ratioMenu);
    m_settingsMenu->AddItem(otherMenu);
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


