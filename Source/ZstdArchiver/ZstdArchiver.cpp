// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "ZstdArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <Messenger.h>
#include <MenuItem.h>

#include <cassert>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ZstdArchiver"
#else
#define B_TRANSLATE(x) x
#endif


Archiver* load_archiver(BMessage* metaDataMsg)
{
    return new ZstdArchiver(metaDataMsg);
}


ZstdArchiver::ZstdArchiver(BMessage* metaDataMsg)
    : TarArchiver(metaDataMsg)
{
    m_tarArk = true;

    if (GetBinaryPath(m_zstdPath, "zstd") == true)
        m_error = BZR_DONE;
    else
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}


status_t ZstdArchiver::ReadOpen(FILE* fp)
{
    char lineString[B_PATH_NAME_LENGTH + 512],
         framesStr[20], skipsStr[20], packedStr[32], packedUnitStr[12], sizeStr[32], sizeUnitStr[12],
         ratioStr[15], checkStr[15],
         pathStr[B_PATH_NAME_LENGTH + 1];
    uint16 const len = sizeof(lineString);

    // zstd does not report the file time of compressed files so take the last modified time of the archive instead.
    time_t const modTime = ArchiveModificationTime();

    // Skip first header line
    fgets(lineString, len, fp);

    while (!feof(fp) && fgets(lineString, len, fp))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString, " %[^ ] %[^ ] %[^ ] %[^ ] %[^ ] %[^ ] %[^ ] %[^ ] %[^\n]",
               framesStr, skipsStr, packedStr, packedUnitStr, sizeStr, sizeUnitStr, ratioStr, checkStr, pathStr);

        BString packedString = StringFromDigitalSize(packedStr, packedUnitStr);
        BString sizeString = StringFromDigitalSize(sizeStr, sizeUnitStr);

        // Zstd is a block compresses that contains only one file/block, so the path cannot ever be a folder.
        assert(!StrEndsWith(pathStr, "/"));
        assert(FinalPathComponent(pathStr) == pathStr);
        m_entriesList.AddItem(new ArchiveEntry(false, pathStr, sizeString.String(), packedString.String(),
                                               modTime, checkStr, "-"));
    }

    return BZR_DONE;
}


status_t ZstdArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    // We are redirecting (>) shell output to file, therefore we need to use /bin/sh -c <command>
    BString destPath = InitTarFilePath(ref->name);
    BString cmd;
    cmd << "\"" << m_zstdPath << "\"" << " -c -d \"" << m_archivePath.Path() << "\" > " << "\"" << m_tarFilePath << "\"";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
    m_pipeMgr.Pipe();

    if (TarArchiver::IsTarArchive(destPath.String()))
    {
        m_tarArk = true;
        BEntry destEntry(destPath.String(), false);
        entry_ref destRef;
        destEntry.GetRef(&destRef);

        // Open using TarArchiver
        status_t const exitCodeTar = TarArchiver::Open(&destRef, fileList);

        // Reset these as TarArchiver's Open() would have changed them
        m_archivePath.SetTo(ref);
        strcpy(m_arkFilePath, m_archivePath.Path());
        m_archiveRef = *ref;

        return exitCodeTar;
    }

    // It's a "pure" Zstd.
    m_tarArk = false;
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_zstdPath << "-l" << "-q" << m_archivePath.Leaf();

    int outdes[2], errdes[2];

    BPath parentPath;
    m_archivePath.GetParent(&parentPath);
    chdir(parentPath.Path());
    thread_id const tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    FILE *outFile = fdopen(outdes[0], "r");
    exitCode = ReadOpen(outFile);
    fclose(outFile);

    FILE *errFile = fdopen(errdes[0], "r");
    exitCode = Archiver::ReadErrStream(errFile, NULL);
    fclose(errFile);

    close(outdes[0]);
    close(errdes[0]);

    return exitCode;
}


status_t ZstdArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                               volatile bool* cancel)
{
    if (m_tarArk == true)
    {
        m_archivePath = m_tarFilePath;
        status_t const exitCode = TarArchiver::Extract(refToDir, message, progress, cancel);
        m_archivePath = m_arkFilePath;

        return exitCode;
    }
    else
    {
        BPath destPath(refToDir);
        if (strcmp(destPath.Path(), TempDirectoryPath()) == 0)
            return BZR_DONE;        // as we already have unpacked it in temp, don't repeat

        BString destFilePath = destPath.Path();
        destFilePath << '/' << OutputFileName(m_archivePath.Leaf());

        BString cmd;
        cmd << "\"" << m_zstdPath << "\"" << " -c --no-progress -d \"" << m_archivePath.Path() << "\" > " << "\"" <<
        destFilePath.String() << "\"";

        m_pipeMgr.FlushArgs();
        m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
        m_pipeMgr.Pipe();

        if (progress)
            SendProgressMessage(progress);

        return BZR_DONE;
    }
}


status_t ZstdArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* /*cancel*/)
{
    // Setup the archive testing process
    BEntry archiveEntry(m_archivePath.Path(), true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_zstdPath << "-t" << "-q" << m_archivePath.Path();

    FILE* err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle zstd unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    BString errorString;
    close(outdes[1]);
    close(errdes[1]);
    err = fdopen(errdes[0], "r");
    Archiver::ReadStream(err, errorString);
    close(errdes[0]);
    close(outdes[0]);
    fclose(err);

    status_t exitCode = BZR_DONE;
    if (errorString.Length() > 0)
    {
        exitCode = BZR_ERRSTREAM_FOUND;
        outputStr = new char[errorString.Length() + 1];
        strcpy(outputStr, errorString.String());
    }

    SendProgressMessage(progress);
    return exitCode;
}


status_t ZstdArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                           BMessenger* progress, volatile bool* cancel)
{
    if (m_tarArk == true)
    {
        m_archivePath = m_tarFilePath;
        status_t exitCode = TarArchiver::Add(createMode, relativePath, message, addedPaths, progress, cancel);
        m_archivePath = m_arkFilePath;

        CompressFromTemp();
        return exitCode;
    }
    else
        return BZR_NOT_SUPPORTED;
}


status_t ZstdArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
                              volatile bool* cancel)
{
    if (m_tarArk == true)
    {
        m_archivePath = m_tarFilePath;
        status_t exitCode = TarArchiver::Delete(outputStr, message, progress, cancel);
        m_archivePath = m_arkFilePath;

        CompressFromTemp();
        return exitCode;
    }
    else
        return BZR_NOT_SUPPORTED;
}


status_t ZstdArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                              BMessenger* progress, volatile bool* cancel)
{
    // true=>normalize path, which means everything otherthan the leaf must exist,
    // meaning we have everything ready and only need to create the leaf (by add)
    m_archivePath.SetTo(archivePath->Path(), NULL, true);

    // We only support creation of .tar.zst not pure .zst
    m_tarArk = true;
    strcpy(m_arkFilePath, m_archivePath.Path());
    InitTarFilePath((char*)archivePath->Leaf());

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


bool ZstdArchiver::NeedsTempDirectory() const
{
    return true;
}


void ZstdArchiver::BuildMenu(BMessage& message)
{
    m_settingsMenu = new BMenu(m_typeStr);

    // Build the compression-level sub-menu
    m_compressionMenu = new BMenu(B_TRANSLATE("Compression level"));
    m_compressionMenu->SetRadioMode(true);

    BString menuStr("1");
    menuStr << " " << B_TRANSLATE("(fastest)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));
    menuStr = "3";
    menuStr << " " << B_TRANSLATE("(default)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));
    m_compressionMenu->AddItem(new BMenuItem("5", NULL));
    m_compressionMenu->AddItem(new BMenuItem("7", NULL));
    m_compressionMenu->AddItem(new BMenuItem("9", NULL));
    m_compressionMenu->AddItem(new BMenuItem("11", NULL));
    m_compressionMenu->AddItem(new BMenuItem("13", NULL));
    m_compressionMenu->AddItem(new BMenuItem("15", NULL));
    m_compressionMenu->AddItem(new BMenuItem("17", NULL));
    m_compressionMenu->AddItem(new BMenuItem("19", NULL));
    m_compressionMenu->AddItem(new BMenuItem("21", NULL));
    menuStr = "22";
    menuStr << " " << B_TRANSLATE("(best)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));

    SetCompressionLevel(message.GetInt32(kCompressionLevelKey, GetDefaultCompressionLevel()));

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(m_compressionMenu);
}


BString ZstdArchiver::OutputFileName(const char* fullFileName) const
{
    // Given a full filename (with extension) this function removes
    // if the filename ends with one of the extensions in extns[], otherwise it returns the full filename
    BString outputFileName = fullFileName;

    int8 const extnsCount = 2;
    const char* extns[] = { ".zst", ".zstd" };

    int32 found = -1;
    for (int32 i = 0; i < extnsCount && found <= 0; i++)
        found = outputFileName.IFindLast(extns[i]);

    if (found > 0)
        outputFileName.Truncate(found);

    return outputFileName;
}


BList ZstdArchiver::HiddenColumns(BList const& columns) const
{
    if (m_tarArk == true)
        return TarArchiver::HiddenColumns(columns);
    else
    {
        // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
        BList hiddenColumns(columns);
        hiddenColumns.RemoveItems(0, 7);    // Remove name..method

        // Now list has 0-crc <-- this column is to be hidden
        return hiddenColumns;
    }
}


void ZstdArchiver::CompressFromTemp()
{
    // Get the compression ratio from the settings menu
    BString levelStr;
    levelStr.SetToFormat(" -%d ", GetCompressionLevel());

    // Re-compress file, from .tar in temp to zstd
    BString cmd;
    cmd << "\"" << m_zstdPath << "\"" << " -c --ultra" << levelStr << "\"" << m_tarFilePath << "\" -o " << "\"" << m_archivePath.Path() << "\"";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
    m_pipeMgr.Pipe();
}


BString ZstdArchiver::InitTarFilePath(char* leaf)
{
    BString destPath = TempDirectoryPath();
    destPath << "/" << OutputFileName(leaf);
    strcpy(m_tarFilePath, destPath.String());
    return destPath;
}


void ZstdArchiver::SendProgressMessage(BMessenger* progress) const
{
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);
    progress->SendMessage(&updateMessage, &reply);
}


bool ZstdArchiver::CanAddFiles() const
{
    if (m_tarArk == false)
        return false;
    else
        return TarArchiver::CanAddFiles();
}
