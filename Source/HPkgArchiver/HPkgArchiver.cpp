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
#include <cctype>


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
        const char colPadStr[] = "  ";                  // whitespace padding prior to each distinct column.
        uint8 const colPadLen = sizeof(colPadStr) - 1;

        uint8 const dateTimeLen = sizeof("YYYY-MM-DD HH:MM:SS") - 1;
        uint8 const dateTimePaddedLen = colPadLen + dateTimeLen;

        uint8 const permLen = sizeof("lrwxrwxrwx") - 1;
        uint8 const permPaddedLen = colPadLen + permLen;

        uint16 const len = B_PATH_NAME_LENGTH + 1024;
        char lineStr[len],
             sizeStr[64], yearStr[8], monthStr[8], dayStr[8], hourStr[8], minuteStr[8], secondStr[8],
             permStr[permLen + 1],
             dateTimeStr[dateTimeLen + 1],
             leafStr[B_FILE_NAME_LENGTH + 1];

        // Read file list.
        while (!feof(fp) && fgets(lineStr, len, fp))
        {
            lineStr[strlen(lineStr) - 1] = '\0';
            BString lineString = lineStr;

            bool permStringValid = false;
            memset(&permStr[0], 0, sizeof(permStr));

            // Could this be a symlink?
            int32 const found = lineString.FindLast("  -> ");
            if (found > permPaddedLen + dateTimePaddedLen /* + length of file name and file size could be included but don't know file name yet. */)
            {
                // Try get file permissions
                lineString.CopyInto(&permStr[0], found - permLen, permLen);
                if (permStr[0] == 'l')
                {
                    permStringValid = IsPermString(permStr, permLen);
                    if (permStringValid)
                    {
                        // This looks like a symlink, discard target path and continue parsing.
                        lineString.Truncate(found, true /* lazy, postpone memory optimization */);
                    }
                }
            }

            // Get the new line length (now that we've may have truncated symlink target path)
            int32 const lineLen = lineString.Length();

            // Parse and validate file permission if we have not already parsed the permission
            // while handling symlinks (above)
            if (permStringValid == false
                && lineLen > permPaddedLen + dateTimePaddedLen)
            {
                lineString.CopyInto(&permStr[0], lineLen - permLen, permLen);
                permStringValid = IsPermString(permStr, permLen);
            }

            if (permStringValid == true)
            { /* likely */ }
            else
                return BZR_ERROR;

            // Parse file date and time
            assert(lineLen > permPaddedLen + dateTimePaddedLen);
            int32_t const dateTimeIndex = lineLen - permPaddedLen - dateTimeLen;
            memset(&dateTimeStr[0], 0, sizeof(dateTimeStr));
            lineString.CopyInto(&dateTimeStr[0], dateTimeIndex, dateTimeLen);
            sscanf(dateTimeStr, "%[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]:%[0-9]",
                yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);

            // Parse file size
            if (dateTimeIndex > 2 * colPadLen
                && lineString.FindFirst(colPadStr, dateTimeIndex - colPadLen) == dateTimeIndex - colPadLen)
            { /* likely */ }
            else
                return BZR_ERROR;

            int32 sizeRevIndex = dateTimeIndex - colPadLen - 1;
            int32 sizeIndex = 0;
            memset(&sizeStr[0], 0, sizeof(sizeStr));
            do
            {
                char ch = lineString[sizeRevIndex];
                if (isdigit(ch))
                    sizeStr[sizeIndex++] = ch;
                else
                    break;
                --sizeRevIndex;
            } while (sizeRevIndex > 0 && sizeIndex < (int32)sizeof(sizeStr) - 1);
            StrReverse(sizeStr, sizeIndex);

            struct tm timeStruct;
            time_t timeValue;
            MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, secondStr);

            // TODO: Parse filename and directory
            (void)leafStr;
        }
    }

    free(metaStr);
    
    return BZR_DONE;
}


status_t HPkgArchiver::Open(entry_ref* ref, BMessage* /*fileList*/)
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


status_t HPkgArchiver::Extract(entry_ref* /*refToDir*/, BMessage* /*message*/, BMessenger* /*progress*/,
                              volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t HPkgArchiver::ReadExtract(FILE* /*fp*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_DONE;
}


status_t HPkgArchiver::Test(char*& /*outputStr*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t HPkgArchiver::ReadTest(FILE* /*fp*/, char*& /*outputStr*/, BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t HPkgArchiver::Add(bool /*createMode*/, const char* /*relativePath*/, BMessage* /*message*/, BMessage* /*addedPaths*/,
                          BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;   
}


status_t HPkgArchiver::Delete(char*& /*outputStr*/, BMessage* /*message*/, BMessenger* /*progress*/,
                             volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


status_t HPkgArchiver::Create(BPath* /*archivePath*/, const char* /*relPath*/, BMessage* /*fileList*/, BMessage* /*addedPaths*/,
                             BMessenger* /*progress*/, volatile bool* /*cancel*/)
{
    return BZR_NOT_SUPPORTED;
}


void HPkgArchiver::BuildMenu(BMessage& /*message*/)
{
}
