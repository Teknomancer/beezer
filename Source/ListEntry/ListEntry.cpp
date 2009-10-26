/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <stdlib.h>

#include "ListEntry.h"
#include "ColumnListView.h"

//=============================================================================================================//

ListEntry::ListEntry (uint32 level, bool superitem, bool expanded, BBitmap *icon, const char *text0,
               const char *text1, const char *text2, const char *text3, const char *text4, const char *text5,
               const char *text6, const char *text7, const char *dirPath, const char *fullPath, int32 length,
               int32 packed, time_t timeValue)
    : CLVEasyItem (level, superitem, expanded, kListEntryHeight, true)
{
    SetColumnContent (1, icon, 2.0, false);
    SetColumnContent (2, text0, true);
    SetColumnContent (3, text1, true, true);
    SetColumnContent (4, text2, true, true);
    SetColumnContent (5, text3, true, true);
    SetColumnContent (6, text4, true);
    SetColumnContent (7, text5, true);
    SetColumnContent (8, text6, true);
    SetColumnContent (9, text7, true);

    m_length = length;
    m_packed = packed;

    if (text3)
        m_ratio = (int8)atoi(text3);
    else
        m_ratio = 0;

    m_dirPath = dirPath;
    m_fullPath = fullPath;
    m_added = false;
    m_timeValue = timeValue;
}

//=============================================================================================================//

ListEntry::ListEntry (uint32 level, bool superitem, bool expanded, BBitmap *icon, char *text0, char *text1,
               char *text2, char *text3, char *text4, char *text5, char *text6, char *text7,
               const char *dirPath, const char *fullPath, int32 length, int32 packed, time_t timeValue)
    : CLVEasyItem (level, superitem, expanded, kListEntryHeight, true)
{
    SetColumnContent (1, icon, 2.0, false);
    SetColumnContent (2, text0, true);
    SetColumnContent (3, text1, true, true);
    SetColumnContent (4, text2, true, true);
    SetColumnContent (5, text3, true, true);
    SetColumnContent (6, text4, true);
    SetColumnContent (7, text5, true);
    SetColumnContent (8, text6, true);
    SetColumnContent (9, text7, true);

    m_length = length;
    m_packed = packed;
    
    if (text3)
        m_ratio = (int8)atoi(text3);
    else
        m_ratio = 0;
    
    m_dirPath = dirPath;
    m_fullPath = fullPath;
    m_added = false;
    m_timeValue = timeValue;
}

//=============================================================================================================//

void ListEntry::Update (ListEntry *newItem)
{
    // Never ever replace file with folder
    if (newItem->IsSuperItem() == true)
        return;
    
    // Possible update fields: are size, packed, ratio, date, method, crc - i.e. 3, 4, 5, 7, 8, 9
    const char *size = newItem->GetColumnContentText (3);
    const char *packed = newItem->GetColumnContentText (4);
    const char *ratio = newItem->GetColumnContentText (5);
    const char *date = newItem->GetColumnContentText (7);
    const char *method = newItem->GetColumnContentText (8);
    const char *crc = newItem->GetColumnContentText (9);
    m_length = newItem->m_length;
    m_packed = newItem->m_packed;
    m_ratio = newItem->m_ratio;
    m_timeValue = newItem->m_timeValue;

    // Update UI
    SetColumnContent (3, size, true, true);
    SetColumnContent (4, packed, true, true);
    SetColumnContent (5, ratio, true, true);
    SetColumnContent (7, date, true);
    SetColumnContent (8, method, true);
    SetColumnContent (9, crc, true);
}

//=============================================================================================================//
