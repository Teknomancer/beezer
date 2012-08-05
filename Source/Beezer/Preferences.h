/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#include <GraphicsDefs.h>
#include <Message.h>

class Preferences : public BMessage
{
    public:
        Preferences();
        Preferences(const char* dir, const char* file);
        ~Preferences();

        // Additional hooks
        virtual void        Init(const char* dir, const char* file);
        virtual void        WritePrefs();
        virtual void        ReadPrefs();
        virtual void        SetLocation(const char* path);
        const char*         Location() const;

        status_t            SetBool(const char* name, bool b);
        status_t            SetInt8(const char* name, int8 i);
        status_t            SetInt16(const char* name, int16 i);
        status_t            SetInt32(const char* name, int32 i);
        status_t            SetInt64(const char* name, int64 i);
        status_t            SetFloat(const char* name, float f);
        status_t            SetDouble(const char* name, double d);
        status_t            SetString(const char* name, const char* string);
        status_t            SetPoint(const char* name, BPoint p);
        status_t            SetRect(const char* name, BRect r);
        status_t            SetMessage(const char* name, const BMessage* message);
        status_t            SetColor(const char* name, rgb_color& color);
        status_t            SetFlat(const char* name, const BFlattenable* obj);

        bool                FindBoolDef(const char* name, bool defaultValue);
        int8                FindInt8Def(const char* name, int8 defaultValue);
        int16               FindInt16Def(const char* name, int16 defaultValue);
        rgb_color           FindColorDef(const char* name, rgb_color defaultValue);

    private:
        // Private hooks
        void                FreePathString();

        // Private members
        const char*         m_prefsPathStr;
};

extern Preferences _prefs_colors;
extern Preferences _prefs_paths;
extern Preferences _prefs_extract;
extern Preferences _prefs_add;
extern Preferences _prefs_state;
extern Preferences _prefs_windows;
extern Preferences _prefs_recent;
extern Preferences _prefs_lang;
extern Preferences _prefs_misc;
extern Preferences _prefs_recent_archives;
extern Preferences _prefs_recent_extract;
extern Preferences _prefs_interface;
extern Preferences _prefs_recent_splitfiles;
extern Preferences _prefs_recent_splitdirs;

#endif /* _PREFERENCES_H */
