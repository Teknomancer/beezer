/*
 * Copyright (c) 2009-2021, Ramshankar (aka Teknomancer)
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

#include "GZipArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GZipArchiver"
#else
#define B_TRANSLATE(x) x
#endif


#define S_FASTEST "(fastest)"
#define S_DEFAULT "(default)"
#define S_BEST "(best)"



Archiver* load_archiver()
{
    return new GZipArchiver();
}



GZipArchiver::GZipArchiver()
    : TarArchiver(true)
{
    // The list of supported mimetypes by this add-on, note the first (index 0) mime-type
    // in the list will be the one that will be used while creating files using this add-on
    m_tarArk = true;
    m_mimeList.AddItem(strdup("application/x-gzip"));
    m_mimeList.AddItem(strdup("application/x-gunzip"));
    m_mimeList.AddItem(strdup("application/gzip"));
    m_mimeList.AddItem(strdup("application/gunzip"));
    SetArchiveType("gzip");
    SetArchiveExtension(".tar.gz");

    m_error = BZR_DONE;
    if (IsBinaryFound(m_gzipPath, BZR_ARK) == false)
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}



status_t GZipArchiver::ReadOpen(FILE* fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
         sizeStr[15], methodStr[15], packedStr[15], ratioStr[10], dayStr[5],
         monthStr[5], hourStr[5], minuteStr[5], crcStr[15],
         pathStr[B_PATH_NAME_LENGTH + 1];

    // Skip first header line
    fgets(lineString, len, fp);

    while (!feof(fp) && fgets(lineString, len, fp))
    {
        lineString[strlen(lineString) - 1] = '\0';

        sscanf(lineString,
               "%[^ ] %[^ ] %[^ ] %[^ ] %[0-9]:%[0-9] %[0-9] %[0-9] %[^ ]%[^\n]",
               methodStr, crcStr, monthStr, dayStr, hourStr, minuteStr, packedStr, sizeStr, ratioStr,
               pathStr);

        BString pathString = pathStr;
        pathString.Remove(0, 1);

        // Ugly cruft -- gzip does NOT report the file time of the compressed file correctly - no year
        // hard-to-read output format etc, so we take the last modified time of the archive as the modified
        // time of the file inside it - this should be accurate
        time_t modTime;
        BEntry archiveEntry(m_archivePath.Path(), true);
        archiveEntry.GetModificationTime(&modTime);

        // Check to see if last char of pathStr = '/' add it as folder, else as a file
        uint16 pathLength = pathString.Length() - 1;
        if (pathString[pathLength] == '/')
        {
            m_entriesList.AddItem(new ArchiveEntry(true, pathString.String(), sizeStr, packedStr, modTime, methodStr, crcStr));
        }
        else
        {
            m_entriesList.AddItem(new ArchiveEntry(false, pathString.String(), sizeStr, packedStr, modTime, methodStr, crcStr));
        }
    }

    return BZR_DONE;
}



status_t GZipArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    // We are redirecting (>) shell output to file, therefore we need to use /bin/sh -c <command>
    BString destPath = InitTarFilePath(ref->name);
    BString cmd;
    cmd << "\"" << m_gzipPath << "\"" << " -c -d \"" << m_archivePath.Path() << "\" > " << "\"" <<
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

    // It's a "pure" GZip.
    m_tarArk = false;
    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_gzipPath << "-lv" << m_archivePath.Leaf();

    FILE* out, *err;
    int outdes[2], errdes[2];

    BPath parentPath;
    m_archivePath.GetParent(&parentPath);
    chdir(parentPath.Path());
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

    return BZR_DONE;
}



status_t GZipArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
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
            cmd << "\"" << m_gzipPath << "\"" << " -c -d \"" << m_archivePath.Path() << "\" > " << "\"" <<
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



status_t GZipArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(m_archivePath.Path(), true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_gzipPath << "-t" << m_archivePath.Path();

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



status_t GZipArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
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



status_t GZipArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
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



status_t GZipArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
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



bool GZipArchiver::NeedsTempDirectory() const
{
    return true;
}



void GZipArchiver::BuildDefaultMenu()
{
    BMenu* ratioMenu;
    m_settingsMenu = new BMenu(m_typeStr);

    // Build the compression-level sub-menu
    ratioMenu = new BMenu(B_TRANSLATE("Compression level"));
    ratioMenu->SetRadioMode(true);

    BString menuStr("1");
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));
    ratioMenu->AddItem(new BMenuItem("2", NULL));
    ratioMenu->AddItem(new BMenuItem("3", NULL));
    ratioMenu->AddItem(new BMenuItem("4", NULL));
    ratioMenu->AddItem(new BMenuItem("5", NULL));
    menuStr = "6";
    menuStr << " " << B_TRANSLATE(S_DEFAULT);
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    ratioMenu->AddItem(defaultItem);
    ratioMenu->AddItem(new BMenuItem("7", NULL));
    ratioMenu->AddItem(new BMenuItem("8", NULL));
    menuStr = "9";
    menuStr << " " << B_TRANSLATE(S_BEST);
    ratioMenu->AddItem(new BMenuItem(menuStr, NULL));

    defaultItem->SetMarked(true);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(ratioMenu);
}



BString GZipArchiver::OutputFileName(const char* fullFileName) const
{
    // Given a full filename (with extension) this function removes
    // if the filename ends with one of the extensions in extns[], otherwise it returns the full filename
    BString outputFileName = fullFileName;

    int8 extnsCount = 2;
    const char* extns[] = { ".gz", ".Z" };

    int32 found = -1;
    for (int32 i = 0; i < extnsCount && found <= 0; i++)
        found = outputFileName.IFindLast(extns[i]);

    if (found > 0)
        outputFileName.Truncate(found);

    else if ((found = outputFileName.IFindLast(".tgz")) > 0)     // special case
    {
        outputFileName.Truncate(found);
        outputFileName += ".tar";
    }

    return outputFileName;
}



BList GZipArchiver::HiddenColumns(BList* columns) const
{
    if (m_tarArk == true)
        return TarArchiver::HiddenColumns(columns);
    else
    {
        BList noHiddenColumns;
        return noHiddenColumns;
    }
}



void GZipArchiver::CompressFromTemp()
{
    // Get the compression ratio from the settings menu
    char level[10];
    BString menuStr("1");
    menuStr << " " << B_TRANSLATE(S_FASTEST);
    BMenu* ratioMenu = m_settingsMenu->FindItem(menuStr)->Menu();
    sprintf(level, " -%ld ", 1 + ratioMenu->IndexOf(ratioMenu->FindMarked()));

    // Re-compress file, from .tar in temp to gzip
    BString cmd;
    cmd << "\"" << m_gzipPath << "\"" << " -c " << level << "\"" << m_tarFilePath << "\" > " << "\"" <<
    m_archivePath.Path() << "\"";

    m_pipeMgr.FlushArgs();
    m_pipeMgr << "/bin/sh" << "-c" << cmd.String();
    m_pipeMgr.Pipe();
}



BString GZipArchiver::InitTarFilePath(char* leaf)
{
    BString destPath = TempDirectoryPath();
    destPath << "/" << OutputFileName(leaf);
    strcpy(m_tarFilePath, destPath.String());
    return destPath;
}



void GZipArchiver::SendProgressMessage(BMessenger* progress) const
{
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);
    progress->SendMessage(&updateMessage, &reply);
}



bool GZipArchiver::CanAddFiles() const
{
    if (m_tarArk == false)
        return false;
    else
        return TarArchiver::CanAddFiles();
}
