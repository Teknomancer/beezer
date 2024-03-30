// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// Copyright (c) 2024 Oscar Lesta.
// All rights reserved.

#include "SquashFSArchiver.h"
#include "AppUtils.h"
#include "ArchiveEntry.h"
#include "KeyedMenuItem.h"

#include <NodeInfo.h>
#include <Messenger.h>
#include <MenuItem.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SquashFSArchiver"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_MARK(x) x
#define B_TRANSLATE_NOCOLLECT(x) x
#endif

#include <cstdlib> // gcc2


// keep track of our custom options/menuitems
static const char* kProcessAttrs = B_TRANSLATE_MARK("Process attributes");


Archiver* load_archiver(BMessage* metaDataMsg)
{
    return new SquashFSArchiver(metaDataMsg);
}


// #pragma mark -


SquashFSArchiver::SquashFSArchiver(BMessage* metaDataMsg)
    : Archiver(metaDataMsg)
{
    m_error = BZR_BINARY_MISSING;
    if (GetBinaryPath(m_unsquashfsPath, "unsquashfs") == true)
        m_error = BZR_DONE;
}


void SquashFSArchiver::BuildMenu(BMessage& message)
{
    m_settingsMenu = new BMenu(m_typeStr);
    m_settingsMenu->AddItem(
        new KeyedMenuItem("bzr:ProcessAttrs", B_TRANSLATE_NOCOLLECT(kProcessAttrs),
                          message, true, new BMessage(BZR_MENUITEM_SELECTED)));
}


BList SquashFSArchiver::HiddenColumns(BList const& columns) const
{
    // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
    BList hiddenColumns(columns);
    hiddenColumns.RemoveItems(0, 2);     // Remove name and size

    // Now list has 0-packed 1-ratio 2-path 3-date 4-method 5-crc
    hiddenColumns.RemoveItems(2, 2);     // Remove path and date

    // Now list has 0-packed 1-ratio 2-method 3-crc <-- these columns are to be hidden
    return hiddenColumns;
}


// #pragma mark -


status_t SquashFSArchiver::Open(entry_ref* ref, BMessage* /*fileList*/)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_unsquashfsPath << "-d" << "" << "-llc" << m_archivePath.Path();

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;

    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadOpen(out);

    close(outdes[0]);
    fclose(out);

    err = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(err, NULL);
    close(errdes[0]);
    fclose(err);

    return exitCode;
}


status_t SquashFSArchiver::ReadOpen(FILE* fp)
{
    char lineString[B_PATH_NAME_LENGTH + 512];
    char permStr[15];
    char ownerStr[100];
    char sizeStr[15];
    char dayStr[5];
    char monthStr[5];
    char yearStr[8];
    char hourStr[5];
    char minuteStr[5];
    char pathStr[2 * B_PATH_NAME_LENGTH + 10];
    uint16 const len = sizeof(lineString);

    while (fgets(lineString, len, fp))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString,
               "%[^ ] %[^ ] %[0-9] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9] %[^\n]",
               permStr, ownerStr, sizeStr, yearStr, monthStr, dayStr, hourStr, minuteStr, pathStr);

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");

        BString pathString = pathStr;
        pathString.RemoveFirst("/"); // avoids having an empty first-level dir.

        // Handle linked files/folders
        if (permStr[0] == 'l')
        {
            BString fullPath = pathString;
            uint16 foundIndex = fullPath.FindLast(" -> ");
            fullPath.Remove(foundIndex, fullPath.Length() - foundIndex);
            pathString = fullPath.String();
        }

        // Check for emtpy dirs:
        bool isDir = permStr[0] == 'd';

        if (isDir)
            pathString.Append("/"); // Without this Beezer doesn't shows the entry for some reason.

        m_entriesList.AddItem(
            new ArchiveEntry(isDir, pathString.String(), sizeStr, "-", timeValue, "-", "-"));
    }

    return BZR_DONE;
}


status_t SquashFSArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                              volatile bool* cancel)
{
    if (progress)
    {
        BEntry dirEntry(refToDir);
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

    m_pipeMgr.FlushArgs();

    // Arguments we use here:
    // quiet, no percentage, print info, overwrite files (needed if the dirPath already exists).
    m_pipeMgr << m_unsquashfsPath << "-q" << "-n" << "-i"  << "-f" << "-d" <<  dirPath.Path();

    if (m_settingsMenu->FindItem(B_TRANSLATE_NOCOLLECT(kProcessAttrs))->IsMarked() == false)
        m_pipeMgr << "-no-xattrs";

    m_pipeMgr << m_archivePath.Path();

    for (int32 i = 0; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcards(pathString);
    }

    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;

    if (progress)
        resume_thread(tid);
    else
    {
        status_t threadExitCode;
        wait_for_thread(tid, &threadExitCode);
    }

    close(errdes[1]);
    close(outdes[1]);

    status_t exitCode = BZR_DONE;
    if (progress)
    {
        FILE* outFile = fdopen(outdes[0], "r");
        exitCode = ReadExtract(outFile, progress, cancel);
        fclose(outFile);
    }

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}


status_t SquashFSArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    // Reads output while extracting files and updates progress window (thru messenger)
    char lineString[999];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        int32 len = strlen(lineString);
        lineString[--len] = '\0';
        if (len >= 1 && lineString[len - 1] != '/')
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", LeafFromPath(lineString));

            progress->SendMessage(&updateMessage, &reply);
        }
    }

    return BZR_DONE;
}


// #pragma mark -


status_t SquashFSArchiver::Test(char*& /*outputStr*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t SquashFSArchiver::Add(bool /*createMode*/, const char* /*relativePath*/, BMessage* /*message*/,
                          BMessage* /*addedPaths*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t SquashFSArchiver::Delete(char*& /*outputStr*/, BMessage* /*message*/, BMessenger* /*progress*/,
                             volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t SquashFSArchiver::Create(BPath* /*archivePath*/, const char* /*relPath*/, BMessage* /*fileList*/,
                             BMessage* /*addedPaths*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


bool SquashFSArchiver::CanAddFiles() const
{
    return false;
}


bool SquashFSArchiver::CanDeleteFiles() const
{
    return false;
}
