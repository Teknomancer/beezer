// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <CheckBox.h>
#include <Debug.h>
#include <String.h>

#include "AppConstants.h"
#include "CommonStrings.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewExtract.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewExtract"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


PrefsViewExtract::PrefsViewExtract(BRect frame)
    : PrefsView(frame, B_TRANSLATE(skExtractString), B_TRANSLATE("Extraction related settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Extract", 20, 20));
    Render();
}


void PrefsViewExtract::Render()
{
    m_openFolderChk = new BCheckBox(BRect(m_margin, m_margin, 0, 0), "PrefsViewExtract:openFolderChk",
                                    B_TRANSLATE("Open destination folder after extracting"), NULL);
    m_openFolderChk->ResizeToPreferred();

    m_closeChk = new BCheckBox(BRect(m_margin, m_openFolderChk->Frame().bottom + m_vGap, 0, 0),
                               "PrefsViewExtract:closeChk", B_TRANSLATE("Close window after extracting"), NULL);
    m_closeChk->ResizeToPreferred();

    BString bufStr(B_TRANSLATE("Quit %appname% after extracting"));
    bufStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    m_quitChk = new BCheckBox(BRect(m_margin, m_closeChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewExtract:closeChk", bufStr, NULL);
    m_quitChk->ResizeToPreferred();

    m_dragChk = new BCheckBox(BRect(m_margin, m_quitChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewExtract:dragChk", B_TRANSLATE("Extract with full path during drag 'n drop"), NULL);
    m_dragChk->ResizeToPreferred();

    AddChild(m_openFolderChk);
    AddChild(m_closeChk);
    AddChild(m_quitChk);
    AddChild(m_dragChk);
    AddRevertButton();
}


void PrefsViewExtract::Save()
{
    _prefs_extract.SetBool(kPfOpen, IsChecked(m_openFolderChk));
    _prefs_extract.SetBool(kPfClose, IsChecked(m_closeChk));
    _prefs_extract.SetBool(kPfQuit, IsChecked(m_quitChk));
    _prefs_extract.SetBool(kPfDrag, IsChecked(m_dragChk));

    _prefs_extract.WritePrefs();
}


void PrefsViewExtract::Load()
{
    m_openFolderChk->SetValue(_prefs_extract.FindBoolDef(kPfOpen, true));
    m_closeChk->SetValue(_prefs_extract.FindBoolDef(kPfClose, false));
    m_quitChk->SetValue(_prefs_extract.FindBoolDef(kPfQuit, false));
    m_dragChk->SetValue(_prefs_extract.FindBoolDef(kPfDrag, true));
}
