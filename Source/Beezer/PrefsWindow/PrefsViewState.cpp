// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewState.h"
#include "BitmapPool.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewState"
#else
#define B_TRANSLATE(x) x
#endif


PrefsViewState::PrefsViewState(BRect frame)
    : PrefsView(frame, B_TRANSLATE("State"), B_TRANSLATE("Configure what to tag along with archives as attributes"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_State", 20, 20));
    Render();
}


void PrefsViewState::Render()
{
    BStringView* storeStrView = new BStringView(NULL, B_TRANSLATE("Store automatically"));
    storeStrView->SetFont(&m_sectionFont);

    m_storeUIChk = new BCheckBox("PrefsViewState:storeUIChk", B_TRANSLATE("Interface state"), NULL);

    m_storeArkChk = new BCheckBox("PrefsViewState:storeArkChk", B_TRANSLATE("Archiver settings"), NULL);

    BStringView* restoreStrView = new BStringView(NULL, B_TRANSLATE("Restore automatically"));
    restoreStrView->SetFont(&m_sectionFont);

    m_restoreUIChk = new BCheckBox("PrefsViewState:restoreUIChk", B_TRANSLATE("Interface state"), NULL);

    m_restoreArkChk = new BCheckBox("PrefsViewState:restoreArkChk", B_TRANSLATE("Archiver settings"), NULL);


    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(m_margin)
        .Add(storeStrView)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(m_margin)
            .Add(m_storeUIChk)
            .AddGlue()
        .End()
        .AddGroup(B_HORIZONTAL)
            .AddStrut(m_margin)
            .Add(m_storeArkChk)
            .AddGlue()
        .End()
        .AddStrut(m_margin) // small space between sections
        .Add(restoreStrView)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(m_margin)
            .Add(m_restoreUIChk)
            .AddGlue()
        .End()
        .AddGroup(B_HORIZONTAL)
            .AddStrut(m_margin)
            .Add(m_restoreArkChk)
            .AddGlue()
        .End()
        .AddGlue() // add some extra space at the bottom
        .End();

    AddRevertButton(builder);
}


void PrefsViewState::Save()
{
    _prefs_state.SetBool(kPfStoreUI, IsChecked(m_storeUIChk));
    _prefs_state.SetBool(kPfRestoreUI, IsChecked(m_restoreUIChk));
    _prefs_state.SetBool(kPfStoreArk, IsChecked(m_storeArkChk));
    _prefs_state.SetBool(kPfRestoreArk, IsChecked(m_restoreArkChk));

    _prefs_state.WritePrefs();
}


void PrefsViewState::Load()
{
    m_storeUIChk->SetValue(_prefs_state.FindBoolDef(kPfStoreUI, false));
    m_restoreUIChk->SetValue(_prefs_state.FindBoolDef(kPfRestoreUI, true));
    m_storeArkChk->SetValue(_prefs_state.FindBoolDef(kPfStoreArk, false));
    m_restoreArkChk->SetValue(_prefs_state.FindBoolDef(kPfRestoreArk, true));
}
