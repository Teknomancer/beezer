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

#include <cassert>
#include <cstdlib>


Archiver* load_archiver(BMessage* metaDataMsg)
{
    return new HPkgArchiver(metaDataMsg);
}


HPkgArchiver::HPkgArchiver(BMessage* metaDataMsg)
	: Archiver(metaDataMsg)
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
    char *metaStr = (char*)malloc(metaLen);
    fread(metaStr, sizeof(char), metaLen, fp);
    metaStr[metaLen] = '\0';

    // Meta information matches what we expect, proceed to read the actual file list.
    if (strcmp(metaStr, metaInfo) == 0)
    {
        // Read file list.
        uint16 len = B_PATH_NAME_LENGTH * 2 + 512;
        char lineStr[len],
             sizeStr[64], yearStr[8], monthStr[8], dayStr[8], hourStr[8], minuteStr[8], secondStr[8], attrStr[16],
             leafStr[B_FILE_NAME_LENGTH + 1];

        const char colPadStr[] = "  ";                  // whitespace padding prior to each distinct column.
        uint8 const colPadLen = sizeof(colPadStr) - 1;

        uint8 const dateTimeLen = sizeof("YYYY-MM-DD HH:MM:SS") - 1;
        uint8 const dateTimePaddedLen = colPadLen + dateTimeLen;

        uint8 const permLen = sizeof("lrwxrwxrwx") - 1;
        uint8 const permPaddedLen = colPadLen + permLen;

        while (!feof(fp) && fgets(lineStr, len, fp))
        {
            lineStr[strlen(lineStr) - 1] = '\0';

            BString permString;
            BString lineString = lineStr;
            bool permStringValid = false;

            // Could this be a symlink?
            int32 const found = lineString.FindLast("  -> ");
            if (found > permPaddedLen + dateTimePaddedLen /* + length of file name and file size could be included but don't know file name yet. */)
            {
                // Try get file permissions
                lineString.CopyInto(permString, found - permPaddedLen, permPaddedLen);
                if (permString.StartsWith(colPadStr, colPadLen) == true)
                {
                    permString.Remove(0, colPadLen);
                    if (permString[0] == 'l')
                    {
                        permStringValid = IsPermString(permString.String(), permString.Length());
                        if (permStringValid)
                        {
                            // This looks like a symlink, discard target path and continue parsing.
                            lineString.Truncate(found, true /* lazy, postpone memory optimization */);
                        }
                    }
                }
            }

            int32 const lineLen = lineString.Length();

            // Have we already parsed the permission string (for symlinks above)?
            if (permStringValid == false)
            {
                if (lineLen > permPaddedLen + dateTimePaddedLen)
                {
                    lineString.CopyInto(permString, lineLen - permLen, permLen);
                    permStringValid = IsPermString(permString.String(), permString.Length());
                }
            }

            // If we -still- don't have a permission string, something is wrong.
            // Bail with an error.
            if (permStringValid == false)
                return BZR_ERROR;

            BString dateTimeString;
            assert(lineLen > permPaddedLen + dateTimePaddedLen);
            lineString.CopyInto(dateTimeString, lineLen - permPaddedLen - dateTimeLen, dateTimeLen);

            char yearStr[8], monthStr[4], dayStr[4], hourStr[4], minuteStr[4], secondStr[4];
            sscanf(dateTimeString.String(),
               "%[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]:%[0-9]",
               yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);

            struct tm timeStruct; time_t timeValue;
            MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, secondStr);

            // TODO: Parse file size and file path (including directory depth)
        }
    }

    free(metaStr);
    
    return BZR_DONE;
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


void HPkgArchiver::BuildMenu(BMessage& message)
{
}
