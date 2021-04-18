// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <DateTimeFormat.h>

#include <cstdio>
#include <cstdlib> // gcc2


ArchiveEntry::ArchiveEntry()
{
}


ArchiveEntry::ArchiveEntry(bool dir, const char* pathStr, const char* sizeStr, const char* packedStr,
                           time_t timeValue, const char* methodStr, const char* crcStr)
    : m_nameStr(NULL),
    m_pathStr(strdup(pathStr)),
    m_sizeStr(strdup(sizeStr)),     // bytes as a string.
    m_packedStr(strdup(packedStr)), // bytes as a string.
    m_ratioStr((char*)malloc(8)),
    m_dateStr(NULL),
    m_methodStr(methodStr ? strdup(methodStr) : NULL),
    m_crcStr(crcStr ? strdup(crcStr) : NULL),
    m_dirStr(NULL),
    m_timeValue(timeValue),
    m_isDir(dir)
{
    // Never call FinalPathComponent here - only use LeafFromPath.
    m_nameStr = strdup(LeafFromPath(pathStr));

    // Get path of parent directory
    int32 const len = strlen(pathStr) - strlen(m_nameStr);
    if (len > 0)
    {
        m_dirStr = (char*)malloc(len + 1);
        strncpy(m_dirStr, pathStr, len);
        m_dirStr[len] = 0;
    }

    // Format date using system settings
    BString dateBuf;
    if (BDateTimeFormat().Format(dateBuf, timeValue, B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT) != B_OK)
        dateBuf = "???";
    m_dateStr = strdup(dateBuf.String());

    // Calculate ratio and update m_ratioStr
    RecalculateRatio();
}


ArchiveEntry::~ArchiveEntry()
{
    free(m_nameStr);
    free(m_pathStr);
    free(m_dirStr);
    free(m_sizeStr);
    free(m_packedStr);
    free(m_ratioStr);
    free(m_dateStr);
    free(m_methodStr);
    free(m_crcStr);
}


void ArchiveEntry::RecalculateRatio()
{
    float ratio = 100 * (atof(m_sizeStr) - atof(m_packedStr));
    if (ratio > 0)    // to prevent 0 bytes 0 packed files (like those in BeBookmarks.zip)
        ratio /= atof(m_sizeStr);
    else if (ratio < 0)
        ratio = 0;

    sprintf(m_ratioStr, "%.1f%%", ratio);
}
