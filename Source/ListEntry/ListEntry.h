// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _LIST_ENTRY_H
#define _LIST_ENTRY_H

#include "CLVEasyItem.h"

#include <String.h>

const float kListEntryHeight = 20.0f;

class ListEntry : public CLVEasyItem
{
    public:
#if __GNUC__ != 2
    using CLVEasyItem::Update;
#endif

        // TODO: Why are these text0..text7 (give them better names?)
        ListEntry(uint32 level, bool superitem, bool expanded, BBitmap* icon, const char* text0,
                  const char* text1, const char* text2, const char* text3, const char* text4,
                  const char* text5, const char* text6, const char* text7, const char* dirPath,
                  const char* fullPath, int32 length, int32 packed, time_t timeValue);

        // TODO: Why are these text0..text7 (give them better names?)
        ListEntry(uint32 level, bool superitem, bool expanded, BBitmap* icon, char* text0, char* text1,
                  char* text2, char* text3, char* text4, char* text5, char* text6, char* text7,
                  const char* dirPath, const char* fullPath, int32 length, int32 packed, time_t timeValue);

        // Public hooks
        void               Update(ListEntry* newItem);

        // Public members
        BString            m_dirPath,
                           m_fullPath;
        int32              m_length,
                           m_packed;
        int8               m_ratio;
        bool               m_added;
        time_t             m_timeValue;
};

#endif /* _LIST_ENTRY_H */
