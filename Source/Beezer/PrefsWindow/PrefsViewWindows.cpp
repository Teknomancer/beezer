/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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

#include <CheckBox.h>
#include <String.h>
#include <StringView.h>

#include "AppConstants.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewWindows.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewWindows"
#else
#define B_TRANSLATE(x) x
#endif


PrefsViewWindows::PrefsViewWindows(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Windows"), B_TRANSLATE("Configure remembering of windows' settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Windows", 20, 20));
    Render();
}



void PrefsViewWindows::Render()
{
    m_searchChk = new BCheckBox(BRect(m_margin, m_margin, 0, 0), "PrefsViewWindows:searchChk",
                                B_TRANSLATE("Search window"), NULL);
    m_searchChk->ResizeToPreferred();

    m_logChk = new BCheckBox(BRect(m_margin, m_searchChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewWindows:logChk", B_TRANSLATE("Log window"), NULL);
    m_logChk->ResizeToPreferred();

    m_commentChk = new BCheckBox(BRect(m_margin, m_logChk->Frame().bottom + m_vGap, 0, 0),
                                 "PrefsViewWindows:commentChk", B_TRANSLATE("Comment window"), NULL);
    m_commentChk->ResizeToPreferred();

    m_arkInfoChk = new BCheckBox(BRect(m_margin, m_commentChk->Frame().bottom + m_vGap, 0, 0),
                                 "PrefsViewWindows:arkInfoChk", B_TRANSLATE("Archive info window"), NULL);
    m_arkInfoChk->ResizeToPreferred();

    m_welcomeChk = new BCheckBox(BRect(m_margin, m_arkInfoChk->Frame().bottom + m_vGap, 0, 0),
                                 "PrefsViewWindows:welcomeChk", B_TRANSLATE("Welcome window"), NULL);
    m_welcomeChk->ResizeToPreferred();

    m_prefsChk = new BCheckBox(BRect(m_margin, m_welcomeChk->Frame().bottom + m_vGap, 0, 0),
                               "PrefsViewWindows:presfChk", B_TRANSLATE("Preferences window"), NULL);
    m_prefsChk->ResizeToPreferred();

    AddChild(m_searchChk);
    AddChild(m_logChk);
    AddChild(m_commentChk);
    AddChild(m_arkInfoChk);
    AddChild(m_welcomeChk);
    AddChild(m_prefsChk);
    AddRevertButton();
}



void PrefsViewWindows::Save()
{
    _prefs_windows.SetBool(kPfSearchWnd, IsChecked(m_searchChk));
    _prefs_windows.SetBool(kPfLogWnd, IsChecked(m_logChk));
    _prefs_windows.SetBool(kPfCommentWnd, IsChecked(m_commentChk));
    _prefs_windows.SetBool(kPfArkInfoWnd, IsChecked(m_arkInfoChk));
    _prefs_windows.SetBool(kPfWelcomeWnd, IsChecked(m_welcomeChk));
    _prefs_windows.SetBool(kPfPrefsWnd, IsChecked(m_prefsChk));
    _prefs_windows.WritePrefs();
}



void PrefsViewWindows::Load()
{
    m_searchChk->SetValue(_prefs_windows.FindBoolDef(kPfSearchWnd, false));
    m_logChk->SetValue(_prefs_windows.FindBoolDef(kPfLogWnd, true));
    m_commentChk->SetValue(_prefs_windows.FindBoolDef(kPfCommentWnd, true));
    m_arkInfoChk->SetValue(_prefs_windows.FindBoolDef(kPfArkInfoWnd, true));
    m_welcomeChk->SetValue(_prefs_windows.FindBoolDef(kPfWelcomeWnd, true));
    m_prefsChk->SetValue(_prefs_windows.FindBoolDef(kPfPrefsWnd, true));
}


