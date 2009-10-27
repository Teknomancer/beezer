/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
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

#ifndef _LIST_ENTRY_H
#define _LIST_ENTRY_H

#include <String.h>

#include "CLVEasyItem.h"

const float           kListEntryHeight = 20.0f;

class ListEntry : public CLVEasyItem
{
    public:
        ListEntry (uint32 level, bool superitem, bool expanded, BBitmap *icon, const char *text0,
                 const char *text1, const char *text2, const char *text3, const char *text4,
                 const char *text5, const char *text6, const char *text7, const char *dirPath,
                 const char *fullPath, int32 length, int32 packed, time_t timeValue);
        ListEntry (uint32 level, bool superitem, bool expanded, BBitmap* icon, char* text0, char* text1,
                 char* text2, char *text3, char *text4, char *text5, char *text6, char *text7,
                 const char *dirPath, const char *fullPath, int32 length, int32 packed, time_t timeValue);
        
        // Public hooks
        void               Update (ListEntry *newItem);

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
