// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ARCHIVE_ENTRY_H
#define _ARCHIVE_ENTRY_H

#include <ctime>

class ArchiveEntry
{
    public:
        ArchiveEntry();
        ArchiveEntry(bool dir, const char* pathStr, const char* sizeStr, const char* packedStr,
                     time_t timeValue, const char* methodStr, const char* crcStr);
        ~ArchiveEntry();

        void              RecalculateRatio();

        char*             m_nameStr,
                          *m_pathStr,
                          *m_sizeStr,     // bytes as a string.
                          *m_packedStr,   // bytes as a string.
                          *m_ratioStr,
                          *m_dateStr,
                          *m_methodStr,
                          *m_crcStr,
                          *m_dirStr;
        time_t            m_timeValue;
        bool              m_isDir;
};

#endif /* _ARCHIVE_ENTRY_H */
