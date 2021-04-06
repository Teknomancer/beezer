// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <CheckBox.h>
#include <String.h>
#include <StringView.h>

#include "AppConstants.h"
#include "CommonStrings.h"
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
    : PrefsView(frame, B_TRANSLATE(skWindowsString),
                B_TRANSLATE("Configure remembering of windows' settings"))
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
                                 "PrefsViewWindows:arkInfoChk", B_TRANSLATE("Archive information window"), NULL);
    m_arkInfoChk->ResizeToPreferred();

    m_welcomeChk = new BCheckBox(BRect(m_margin, m_arkInfoChk->Frame().bottom + m_vGap, 0, 0),
                                 "PrefsViewWindows:welcomeChk", B_TRANSLATE("Welcome window"), NULL);
    m_welcomeChk->ResizeToPreferred();

    m_prefsChk = new BCheckBox(BRect(m_margin, m_welcomeChk->Frame().bottom + m_vGap, 0, 0),
                               "PrefsViewWindows:presfChk", B_TRANSLATE("Settings window"), NULL);
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
