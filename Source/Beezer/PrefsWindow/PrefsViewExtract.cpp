// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewExtract.h"
#include "AppConstants.h"
#include "CommonStrings.h"
#include "BitmapPool.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewExtract"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


PrefsViewExtract::PrefsViewExtract(BRect frame)
    : PrefsView(frame, BZ_TR(kExtractString), B_TRANSLATE("Extraction related settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Extract", 20, 20));
    Render();
}


void PrefsViewExtract::Render()
{
    m_openFolderChk = new BCheckBox("PrefsViewExtract:openFolderChk",
                                    B_TRANSLATE("Open destination folder after extracting"), NULL);

    m_closeChk = new BCheckBox("PrefsViewExtract:closeChk", B_TRANSLATE("Close window after extracting"), NULL);

    BString bufStr(B_TRANSLATE("Quit %appname% after extracting"));
    bufStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    m_quitChk = new BCheckBox("PrefsViewExtract:closeChk", bufStr, NULL);

    m_dragChk = new BCheckBox("PrefsViewExtract:dragChk", B_TRANSLATE("Extract with full path during drag 'n drop"), NULL);

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .Add(m_openFolderChk)
        .Add(m_closeChk)
        .Add(m_quitChk)
        .Add(m_dragChk)
        .AddGlue() // add some free space at the bottom
        .End();

    AddRevertButton(builder);
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
