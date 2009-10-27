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

#include "JoinerStrings.h"

static int _language = LANG_ENGLISH;

const char* strR5 (int whichLang, int language);
#ifdef B_ZETA_VERSION
    const char* _X (const char *key, int32 keyindex);
#endif

static const char * _englishStrings [NUM_STRINGS] =
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

//=============================================================================================================//

int GetLanguage ()
{
    return _language;
}

//=============================================================================================================//

void SetLanguage (int whichLang)
{
    if ((whichLang >= 0) && (whichLang < NUM_LANGUAGES))
        _language = whichLang;
}

//=============================================================================================================//

const char* GetLanguageName (int whichLang, bool native)
{
    switch (whichLang)
    {
        case LANG_ENGLISH: return strR5 (S_ENGLISH, native ? LANG_ENGLISH : _language);
        
        default: debugger ("bad language index!"); return 0;
    }
}

//=============================================================================================================//

const char* strR5 (int whichLang, int language)
{
    const char **base = NULL;
    switch ((language < 0) ? _language : language)
    {
        case LANG_ENGLISH:        base = _englishStrings;        break;
    }

    return ((base) && (whichLang >= 0) && (whichLang < NUM_STRINGS)) ? base[whichLang] : NULL;
}

//=============================================================================================================//

#ifdef B_ZETA_VERSION
const char* _X (const char *key, int keyindex)
{
    BString result = _T (key);
    if (result == key)
        return strR5 (keyindex);
    else
        return _T (key);
}
#endif

//=============================================================================================================//
