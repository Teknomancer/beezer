// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "JoinerStrings.h"

static int _language = LANG_ENGLISH;

const char* strR5(int whichLang, int language);

static const char* _englishStrings [NUM_STRINGS] =
{
    "English",
    "Beezer: File Joiner",
    "Joining: ",
    "Cancel",
    "OK",
    "An unknown error occured while joining the files.",
    "Computing file size...",
    "Beezer: Self-Joining Stub\n© 2005 Ramshankar.",
};



int GetLanguage()
{
    return _language;
}



void SetLanguage(int whichLang)
{
    if ((whichLang >= 0) && (whichLang < NUM_LANGUAGES))
        _language = whichLang;
}



const char* GetLanguageName(int whichLang, bool native)
{
    switch (whichLang)
    {
        case LANG_ENGLISH: return strR5(S_ENGLISH, native ? LANG_ENGLISH : _language);

        default: debugger("bad language index!"); return 0;
    }
}



const char* strR5(int whichLang, int language)
{
    const char** base = NULL;
    switch ((language < 0) ? _language : language)
    {
        case LANG_ENGLISH:        base = _englishStrings;        break;
    }

    return ((base) && (whichLang >= 0) && (whichLang < NUM_STRINGS)) ? base[whichLang] : NULL;
}


