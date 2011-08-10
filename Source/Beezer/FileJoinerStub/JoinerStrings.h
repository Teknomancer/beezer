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

#ifndef _JOINER_STRINGS_H
#define _JOINER_STRINGS_H

#include <InterfaceDefs.h>

#define str(X) strR5(X)

extern const char *strR5 (int whichString, int whichLanguage = -1);
extern void SetLanguage (int whichLanguage);
extern const char *GetLanguageName (int whichLanguage, bool inNativeTongue);
extern int GetLanguage ();

enum LanguageNames
{
    LANG_ENGLISH = 0,
    NUM_LANGUAGES
};


// NOTE:: 
// If you add any new language strings please add which version of the code has it been added
// in, notice that 0.06 upwards these labels are present as 0.04, 0.05 had same labels and Zeta 0.04
// language file is as per that order

enum LanguageStrings
{
    S_ENGLISH = 0,
    S_JOINER_WINDOW_TITLE,
    S_JOINING_FILE,
    S_CANCEL,
    S_OK,
    S_JOIN_ERROR,
    S_COMPUTING,
    S_STUB_ERROR,
    NUM_STRINGS
};

#endif /* _JOINER_STRINGS_H */
