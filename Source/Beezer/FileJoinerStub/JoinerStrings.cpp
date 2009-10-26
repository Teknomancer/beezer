/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
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
		case LANG_ENGLISH:		base = _englishStrings;		break;
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
