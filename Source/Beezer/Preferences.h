// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#include <GraphicsDefs.h>
#include <Message.h>

class Preferences : public BMessage
{
    public:
        Preferences();
        Preferences(const char* dir, const char* file);
        virtual ~Preferences();

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
        void                FreePathString();

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
