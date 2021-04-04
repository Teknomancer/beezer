// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <CheckBox.h>
#include <String.h>
#include <StringView.h>

#include "AppConstants.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewState.h"
#include "UIConstants.h"

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
    BStringView* storeStrView = new BStringView(BRect(m_margin, m_margin, 0, 0), NULL,
            B_TRANSLATE("Store automatically"));
    storeStrView->SetFont(&m_sectionFont);
    storeStrView->ResizeToPreferred();
    storeStrView->SetLowColor(ViewColor());

    m_storeUIChk = new BCheckBox(BRect(3 * m_margin, storeStrView->Frame().bottom + m_vGap, 0, 0),
                                 "PrefsViewState:storeUIChk", B_TRANSLATE("Interface state"), NULL);
    m_storeUIChk->ResizeToPreferred();

    m_storeArkChk = new BCheckBox(BRect(3 * m_margin, m_storeUIChk->Frame().bottom + m_vGap, 0, 0),
                                  "PrefsViewState:storeArkChk", B_TRANSLATE("Archiver settings"), NULL);
    m_storeArkChk->ResizeToPreferred();

    BStringView* restoreStrView = new BStringView(BRect(m_margin, m_storeArkChk->Frame().bottom + m_vGap +
            m_storeArkChk->Frame().Height() / 2, 0, 0), NULL, B_TRANSLATE("Restore automatically"));
    restoreStrView->SetFont(&m_sectionFont);
    restoreStrView->ResizeToPreferred();
    restoreStrView->SetLowColor(ViewColor());

    m_restoreUIChk = new BCheckBox(BRect(3 * m_margin, restoreStrView->Frame().bottom + m_vGap, 0, 0),
                                   "PrefsViewState:restoreUIChk", B_TRANSLATE("Interface state"), NULL);
    m_restoreUIChk->ResizeToPreferred();

    m_restoreArkChk = new BCheckBox(BRect(3 * m_margin, m_restoreUIChk->Frame().bottom + m_vGap, 0, 0),
                                    "PrefsViewState:restoreArkChk", B_TRANSLATE("Archiver settings"), NULL);
    m_restoreArkChk->ResizeToPreferred();

    AddChild(storeStrView);
    AddChild(m_storeUIChk);
    AddChild(m_storeArkChk);
    AddChild(restoreStrView);
    AddChild(m_restoreUIChk);
    AddChild(m_restoreArkChk);
    AddRevertButton();
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
