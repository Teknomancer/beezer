// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "TarArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <NodeInfo.h>
#include <Messenger.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TarArchiver"
#else
#define B_TRANSLATE(x) x
#endif


#ifndef STATIC_LIB_BUILD
Archiver* load_archiver(BMessage* metaDataMsg)
{
    return new TarArchiver(metaDataMsg);
}
#endif


TarArchiver::TarArchiver(BMessage* metaDataMsg)
    : Archiver(metaDataMsg)
{
    m_error = InitBinaryPath();
}


status_t TarArchiver::InitBinaryPath()
{
    if (GetBinaryPath(m_tarPath, "tar") == true)
        return BZR_DONE;
    else
        return BZR_BINARY_MISSING;
}


status_t TarArchiver::ReadOpen(FILE* fp)
{
    char lineString[B_PATH_NAME_LENGTH + 512],
         permStr[15], ownerStr[100], sizeStr[15],
         dayStr[5], monthStr[5], yearStr[8], hourStr[5], minuteStr[5],
         pathStr[2 * B_PATH_NAME_LENGTH + 10];
    uint16 const len = sizeof(lineString);

    while (fgets(lineString, len, fp))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString,
               "%[^ ] %[^ ] %[0-9] %[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]%[^\n]",
               permStr, ownerStr, sizeStr, yearStr, monthStr, dayStr, hourStr, minuteStr, pathStr);

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, "00");

        // Bugfix workaround for files/folders with space before them
        BString pathString = pathStr;
        pathString.Remove(0, 1);

        // Handle linked files/folder by tar
        if (permStr[0] == 'l')
        {
            BString fullPath = pathString;
            uint16 foundIndex = fullPath.FindLast(" -> ");
            fullPath.Remove(foundIndex, fullPath.Length() - foundIndex);
            pathString = fullPath.String();
        }

        // Check to see if last char of pathStr = '/' add it as folder, else as a file
        uint16 pathLength = pathString.Length() - 1;
        if (pathString[pathLength] == '/' || permStr[0] == 'd')
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, sizeStr, timeValue, "-", "-"));
        else
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, sizeStr, timeValue, "-", "-"));
    }

    return BZR_DONE;
}


status_t TarArchiver::Open(entry_ref* ref, BMessage* /*fileList*/)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_tarPath << "-tv" << "-f" << m_archivePath.Path();

    // Currently, we don't list individual files specified in fileList. This is
    // because while deleting files from tar, it doesn't report the deleted files thus
    // the entire archive is reloaded after deleting, so specific files are skipped

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle unloadable error here

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    exitCode = ReadOpen(out);

    close(outdes[0]);
    fclose(out);

    err = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(err, NULL);
    close(errdes[0]);
    fclose(err);

    return exitCode;
}


status_t TarArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
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
    m_pipeMgr << m_tarPath << "-xpv" << "-f";
    m_pipeMgr << m_archivePath.Path() << "-C" << dirPath.Path();

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
        return B_ERROR;           // Handle unloadable error here

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


status_t TarArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
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


status_t TarArchiver::Test(char*& /*outputStr*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t TarArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
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
    m_pipeMgr << m_tarPath << "-pv" << "-f" << m_archivePath.Path() << "-r";

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
        return B_ERROR;        // Handle unloadable error here

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
    return exitCode;
}


status_t TarArchiver::ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel)
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
        const char* fileName = FinalPathComponent(lineString);

        // Don't update progress bar for folders
        if (fileName[strlen(fileName) - 1] != '/' && progress)
        {
            updateMessage.RemoveName("text");
            updateMessage.AddString("text", fileName);
            progress->SendMessage(&updateMessage, &reply);
        }

        addedPaths->AddString(kPath, lineString);
    }

    return exitCode;
}


status_t TarArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
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
    m_pipeMgr << m_tarPath << "-pv" << "-f" << m_archivePath.Path() << "--delete";

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcards(pathString);
    }

    FILE* out, *err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle unloadable error here
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
    return exitCode;
}


status_t TarArchiver::ReadDelete(FILE* fp, char*& /*outputStr*/, BMessenger* /*progress*/,
                                 volatile bool* /*cancel*/)
{
    char lineString[999];
    while (!feof(fp) && fgets(lineString, 998, fp))
        ;

    return BZR_DONE;
}


status_t TarArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
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


bool TarArchiver::IsTarArchive(const char *filePath) const
{
    // Update mime type of the file.
    update_mime_info(filePath, false /* recursive */, true /* synchronous */, false /* force */);

    // Get the updated mime type
    BNode destNode(filePath);
    BNodeInfo destNodeInfo(&destNode);
    char mimeBuf[B_MIME_TYPE_LENGTH];
    destNodeInfo.GetType(mimeBuf);

    // Check if the mime type is that of a tar archive
    for (int32 i = 0; i < m_mimeList.CountItems(); i++)
    {
        if (!strcmp(mimeBuf, (const char *)m_mimeList.ItemAtFast(i)))
            return true;
    }

    // Check if the file extension is ".tar"
    BString extensionStr = filePath;
    int32 found = extensionStr.IFindLast(".tar");
    if (found == extensionStr.Length() - 4)
        return true;

    return false;
}


BList TarArchiver::HiddenColumns(BList const& columns) const
{
    // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
    // Tar.GZip or Tar don't have 2,3,6,7 -- Pure Gzip have all
    BList hiddenColumns(columns);
    hiddenColumns.RemoveItems(0, 2);     // Remove name and size

    // Now list has 0-packed 1-ratio 2-path 3-date 4-method 5-crc
    hiddenColumns.RemoveItems(2, 2);     // Remove path and date

    // Now list has 0-packed 1-ratio 2-method 3-crc <-- these columns are to be hidden
    return hiddenColumns;
}


bool TarArchiver::CanReplaceFiles() const
{
    return false;
}


bool TarArchiver::CanPartiallyOpen() const
{
    return false;
}
