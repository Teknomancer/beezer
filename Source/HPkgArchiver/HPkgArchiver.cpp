// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "HPkgArchiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <NodeInfo.h>
#include <Messenger.h>
#include <Directory.h>
#include <MenuItem.h>


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "HPkgArchiver"
#else
#define B_TRANSLATE(x) x
#endif

// TODO: Update RuleDefaults.h
// TODO: Update CMakeLists.txt

Archiver* load_archiver(const char* addonImagePath)
{
	return new HPkgArchiver(addonImagePath);
}



HPkgArchiver::HPkgArchiver(const char* addonImagePath)
	: Archiver(addonImagePath)
{
    if (GetBinaryPath(m_hpkgPath, "package") == true)
		m_error = BZR_DONE;
	else
	{
        m_error = BZR_BINARY_MISSING;
        return;
    }
}



status_t HPkgArchiver::ReadOpen(FILE* fp, const char* metaInfo)
{
    // Read (and drain) meta information from the file.
    size_t const metaLen = strlen(metaInfo);
    char *metaStr = (char *)malloc(metaLen);
    fread(metaStr, sizeof(char), metaLen, fp);
    metaStr[metaLen] = '\0';

    // Meta information matches what we expect, proceed to read the actual file list.
    if (strcmp(metaStr, metaInfo) == 0)
    {
        // Read file list.
        uint16 len = B_PATH_NAME_LENGTH * 2 + 512;
        char lineString[len],
             sizeStr[64], yearStr[8], monthStr[8], dayStr[8], hourStr[8], minuteStr[8], secondStr[8], attrStr[16],
             leafStr[B_FILE_NAME_LENGTH + 1];
        
        while (!feof(fp) && fgets(lineString, len, fp))
        {
            lineString[strlen(lineString) - 1] = '\0';

            printf("line='%s'\n", lineString);
        }
    }

    free(metaStr);
    
    return BZR_ERRSTREAM_FOUND;
}



status_t HPkgArchiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    // First read the meta package information because we have no reliable
    // way to weed this out from the regular "list" output. So we read the
    // meta information first and use the entire string to filter out the
    // output of "package list".
    status_t exitCode;
    BString metaInfo;
    {
        m_pipeMgr.FlushArgs();
        m_pipeMgr << m_hpkgPath << "list" << "-i" << m_archivePath.Path();

        FILE* out, *err;
        int outdes[2], errdes[2];
        thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

        if (tid == B_ERROR || tid == B_NO_MEMORY)
            return B_ERROR;

        resume_thread(tid);

        close(errdes[1]);
        close(outdes[1]);

        out = fdopen(outdes[0], "r");
        Archiver::ReadStream(out, metaInfo);
        close(outdes[0]);
        fclose(out);

        err = fdopen(errdes[0], "r");
        exitCode = Archiver::ReadErrStream(err);
        close(errdes[0]);
        fclose(err);
    }

    // Next, list the contents of the package and read it.
    if (   exitCode == BZR_DONE
        && metaInfo.IsEmpty() == false)
    {
        m_pipeMgr.FlushArgs();
        m_pipeMgr << m_hpkgPath << "list" << m_archivePath.Path();

        FILE* out, *err;
        int outdes[2], errdes[2];
        thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

        if (tid == B_ERROR || tid == B_NO_MEMORY)
            return B_ERROR;

        resume_thread(tid);

        close(errdes[1]);
        close(outdes[1]);

        out = fdopen(outdes[0], "r");
        exitCode = ReadOpen(out, metaInfo.String());

        close(outdes[0]);
        fclose(out);

        if (exitCode == BZR_DONE)
        {
            err = fdopen(errdes[0], "r");
            exitCode = Archiver::ReadErrStream(err, NULL);
            close(errdes[0]);
        }
        fclose(err);
    }
    
    return exitCode;
}



status_t HPkgArchiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                              volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t HPkgArchiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    return BZR_DONE;
}



status_t HPkgArchiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t HPkgArchiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t HPkgArchiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                          BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;   
}



status_t HPkgArchiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
                             volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



status_t HPkgArchiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                             BMessenger* progress, volatile bool* cancel)
{
    return BZR_NOT_SUPPORTED;
}



void HPkgArchiver::BuildDefaultMenu()
{
}
