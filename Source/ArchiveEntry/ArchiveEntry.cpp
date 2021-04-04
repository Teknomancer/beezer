// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <DateTimeFormat.h>

#include <stdio.h>


ArchiveEntry::ArchiveEntry()
{
}


ArchiveEntry::ArchiveEntry(bool dir, const char* pathStr, const char* sizeStr, const char* packedStr,
                           time_t timeValue, const char* methodStr, const char* crcStr)
    : m_dirStr(NULL)
{
    m_isDir = dir;
    m_nameStr = strdup(LeafFromPath(pathStr));         // Never call FinalPathComponent here - only use
    m_pathStr = strdup(pathStr);                       // LeafFromPath() bug fixed

    // Get path of parent directory
    int32 len = strlen(pathStr) - strlen(m_nameStr);
    if (len > 0)
    {
        m_dirStr = (char*)malloc(len +1);
        strncpy(m_dirStr, pathStr, len);
        m_dirStr[len] = 0;
    }

    BString dateBuf;
    if (BDateTimeFormat().Format(dateBuf, timeValue, B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT) != B_OK)
        dateBuf = "???";

    m_dateStr = strdup(dateBuf.String());
    m_timeValue = timeValue;
    m_sizeStr = strdup(sizeStr);
    m_packedStr = strdup(packedStr);

    m_ratioStr = (char*)malloc(8);
    RecalculateRatio();

    if (methodStr)
        m_methodStr = strdup(methodStr);
    else
        m_methodStr = NULL;

    if (crcStr)
        m_crcStr = strdup(crcStr);
    else
        m_crcStr = NULL;
}


ArchiveEntry::~ArchiveEntry()
{
    if (m_nameStr) free(m_nameStr);
    if (m_pathStr) free(m_pathStr);
    if (m_dirStr) free(m_dirStr);
    if (m_sizeStr) free(m_sizeStr);
    if (m_packedStr) free(m_packedStr);
    if (m_ratioStr) free(m_ratioStr);
    if (m_dateStr) free(m_dateStr);
    if (m_methodStr) free(m_methodStr);
    if (m_crcStr) free(m_crcStr);
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
