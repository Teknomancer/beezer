// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewWindows.h"
#include "CommonStrings.h"
#include "BitmapPool.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewWindows"
#else
#define B_TRANSLATE(x) x
#endif


PrefsViewWindows::PrefsViewWindows(BRect frame)
    : PrefsView(frame, BZ_TR(kWindowsString),
                B_TRANSLATE("Configure remembering of windows' settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Windows", 20, 20));
    Render();
}


void PrefsViewWindows::Render()
{
    m_searchChk = new BCheckBox("PrefsViewWindows:searchChk", B_TRANSLATE("Search window"), NULL);

    m_logChk = new BCheckBox("PrefsViewWindows:logChk", B_TRANSLATE("Log window"), NULL);

    m_commentChk = new BCheckBox("PrefsViewWindows:commentChk", B_TRANSLATE("Comment window"), NULL);

    m_arkInfoChk = new BCheckBox("PrefsViewWindows:arkInfoChk", B_TRANSLATE("Archive information window"), NULL);

    m_welcomeChk = new BCheckBox("PrefsViewWindows:welcomeChk", B_TRANSLATE("Welcome window"), NULL);

    m_prefsChk = new BCheckBox("PrefsViewWindows:presfChk", B_TRANSLATE("Settings window"), NULL);

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(m_margin)
        .Add(m_searchChk)
        .Add(m_logChk)
        .Add(m_commentChk)
        .Add(m_arkInfoChk)
        .Add(m_welcomeChk)
        .Add(m_prefsChk)
        .AddGlue() // add some free space at the bottom
        .End();

    AddRevertButton(builder);
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
