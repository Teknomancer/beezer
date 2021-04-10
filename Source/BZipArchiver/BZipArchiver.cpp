// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "BZipArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <NodeInfo.h>
#include <Messenger.h>
#include <MenuItem.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BZipArchiver"
#else
#define B_TRANSLATE(x) x
#endif



Archiver* load_archiver(const char* addonImagePath)
{
    return new BZipArchiver(addonImagePath);
}


BZipArchiver::BZipArchiver(const char* addonImagePath)
    : TarArchiver(addonImagePath)
{
    m_tarArk = true;

    if (GetBinaryPath(m_bzipPath, "bzip2") == true)
        m_error = BZR_DONE;
    else
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}


status_t BZipArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    // We are redirecting (>) shell output to file, therefore we need to use /bin/sh -c <command>
    BString destPath = InitTarFilePath(ref->name);
    BString cmd;
    cmd << "\"" << m_bzipPath << "\"" << " -c -d \"" << m_archivePath.Path() << "\" > " << "\"" <<
    m_tarFilePath << "\"";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
    m_pipeMgr.Pipe();

    if (TarArchiver::IsTarArchive(destPath.String()))
    {
        m_tarArk = true;
        BEntry destEntry(destPath.String(), false);
        entry_ref destRef;
        destEntry.GetRef(&destRef);
        status_t exitCode = TarArchiver::Open(&destRef, fileList);

        // Reset these as TarArchiver's Open() would have changed them
        m_archivePath.SetTo(ref);
        strcpy(m_arkFilePath, m_archivePath.Path());
        m_archiveRef = *ref;

        return exitCode;
    }

    // It's a "pure" BZip2.
    // bzip2 does not list its file like gzip, so we fake it :)
    m_tarArk = false;
    char sizeStr[30], pathStr[B_PATH_NAME_LENGTH];

    time_t const modTime = ArchiveModificationTime();
    off_t size;
    BEntry deflatedEntry(destPath.String(), false);
    if (deflatedEntry.Exists() == false)
        return BZR_ERRSTREAM_FOUND;

    deflatedEntry.GetSize(&size);
    sprintf(sizeStr, "%Ld", size);
    BPath tempPath(destPath.String());
    strcpy(pathStr, tempPath.Leaf());

    m_entriesList.AddItem(new ArchiveEntry(false, tempPath.Leaf(), sizeStr, "-", modTime, "-", "-"));

    return BZR_DONE;
}


status_t BZipArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                               volatile bool* cancel)
{
    if (m_tarArk == false)
    {
        BPath destPath(refToDir);
        if (strcmp(destPath.Path(), TempDirectoryPath()) == 0)
            return BZR_DONE;        // as we already have unpacked it in temp, don't repeat
        else
        {
            BString destFilePath = destPath.Path();
            destFilePath << '/' << OutputFileName(m_archivePath.Leaf());

            BString cmd;
            cmd << "\"" << m_bzipPath << "\"" << " -c -d \"" << m_archivePath.Path() << "\" > " << "\"" <<
            destFilePath.String() << "\"";

            m_pipeMgr.FlushArgs();
            m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
            m_pipeMgr.Pipe();

            if (progress)
                SendProgressMessage(progress);

            return BZR_DONE;
        }
    }
    else
    {
        m_archivePath = m_tarFilePath;
        status_t exitCode = TarArchiver::Extract(refToDir, message, progress, cancel);
        m_archivePath = m_arkFilePath;
        return exitCode;
    }
}


status_t BZipArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(m_archivePath.Path(), true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_bzipPath << "-t" << m_archivePath.Path();

    FILE* err;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle unzip unloadable error here
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


status_t BZipArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
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


status_t BZipArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
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


status_t BZipArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                              BMessenger* progress, volatile bool* cancel)
{
    // true=>normalize path, which means everything otherthan the leaf must exist,
    // meaning we have everything ready and only need to create the leaf (by add)
    m_archivePath.SetTo(archivePath->Path(), NULL, true);

    // We only support creation of .tar.gz not pure .gz
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


bool BZipArchiver::NeedsTempDirectory() const
{
    return true;
}


void BZipArchiver::BuildDefaultMenu()
{
    m_settingsMenu = new BMenu(m_typeStr);

    // Build the compression-level sub-menu
    m_compressionMenu = new BMenu(B_TRANSLATE("Compression level"));
    m_compressionMenu->SetRadioMode(true);

    BString menuStr("1");
    menuStr << " " << B_TRANSLATE("(fastest)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));
    m_compressionMenu->AddItem(new BMenuItem("2", NULL));
    m_compressionMenu->AddItem(new BMenuItem("3", NULL));
    m_compressionMenu->AddItem(new BMenuItem("4", NULL));
    m_compressionMenu->AddItem(new BMenuItem("5", NULL));
    menuStr = "6";
    menuStr << " " << B_TRANSLATE("(default)");
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    m_compressionMenu->AddItem(defaultItem);
    m_compressionMenu->AddItem(new BMenuItem("7", NULL));
    m_compressionMenu->AddItem(new BMenuItem("8", NULL));
    menuStr = "9";
    menuStr << " " << B_TRANSLATE("(best)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));

    defaultItem->SetMarked(true);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(m_compressionMenu);
}


BString BZipArchiver::OutputFileName(const char* fullFileName) const
{
    // Given a full filename (with extension) this function removes
    // if the filename ends with one of the extensions in extns[], otherwise it returns the full filename
    BString outputFileName = fullFileName;

    int8 extnsCount = 2;
    const char* extns[] = { ".bz2", ".bz" };

    int32 found = -1;
    for (int32 i = 0; i < extnsCount && found <= 0; i++)
        found = outputFileName.IFindLast(extns[i]);

    if (found > 0)
        outputFileName.Truncate(found);

    else if ((found = outputFileName.IFindLast(".tbz")) > 0)     // special case
    {
        outputFileName.Truncate(found);
        outputFileName += ".tar";
    }

    return outputFileName;
}


BList BZipArchiver::HiddenColumns(BList* columns) const
{
    if (m_tarArk == true)
        return TarArchiver::HiddenColumns(columns);
    else
    {
        // Bzip2 needs to hide some columns, unlike gzip bzip2 doesn't have a list option hence
        // we cannot get certain details namely packed, method, CRC - we hide these 3 columns
        // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
        BList availList(*columns);
        availList.RemoveItems(0, 2);     // Remove 0 and 1

        // Now list has 0-packed 1-ratio 2-path 3-date 4-method 5-crc
        availList.RemoveItems(3, 1);     // Remove 3 we don't want to hide date

        // Now list has 0-packed 1-ratio 2-path 3-method 4-crc <-- these columns are to be hidden
        return availList;
    }
}


void BZipArchiver::CompressFromTemp()
{
    // Get the compression ratio from the settings menu
    BString levelStr;
    levelStr.SetToFormat(" -%ld ", GetCompressionLevel());

    // Re-compress file, from .tar in temp to gzip
    BString cmd;
    cmd << "\"" << m_bzipPath << "\"" << " -c " << levelStr << "\"" << m_tarFilePath << "\" > " << "\"" <<
    m_archivePath.Path() << "\"";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
    m_pipeMgr.Pipe();
}


BString BZipArchiver::InitTarFilePath(char* leaf)
{
    BString destPath = TempDirectoryPath();
    destPath << "/" << OutputFileName(leaf);
    strcpy(m_tarFilePath, destPath.String());
    return destPath;
}


void BZipArchiver::SendProgressMessage(BMessenger* progress) const
{
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);
    progress->SendMessage(&updateMessage, &reply);
}


bool BZipArchiver::CanAddFiles() const
{
    if (m_tarArk == false)
        return false;
    else
        return TarArchiver::CanAddFiles();
}
