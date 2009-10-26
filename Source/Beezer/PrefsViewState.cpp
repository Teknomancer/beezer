/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <String.h>
#include <CheckBox.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include "PrefsViewState.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "AppConstants.h"
#include "Preferences.h"

//=============================================================================================================//

PrefsViewState::PrefsViewState (BRect frame)
    : PrefsView (frame, str (S_PREFS_TITLE_STATE), str (S_PREFS_DESC_STATE))
{
    SetBitmap (ResBitmap ("Img:Prefs_State"));
    Render();
}

//=============================================================================================================//

void PrefsViewState::Render ()
{
    BStringView *storeStrView = new BStringView (BRect (m_margin, m_margin, 0, 0), NULL,
                                        str (S_PREFS_STATE_STORE));
    storeStrView->SetFont (&m_sectionFont);
    storeStrView->ResizeToPreferred();
    storeStrView->SetLowColor (ViewColor());
                            
    m_storeUIChk = new BCheckBox (BRect (3 * m_margin, storeStrView->Frame().bottom + m_vGap, 0, 0),
                            "PrefsViewState:storeUIChk", str (S_PREFS_STATE_UI), NULL);
    m_storeUIChk->ResizeToPreferred();

    m_storeArkChk = new BCheckBox (BRect (3 * m_margin, m_storeUIChk->Frame().bottom + m_vGap, 0, 0),
                                "PrefsViewState:storeArkChk", str (S_PREFS_STATE_ARK), NULL);
    m_storeArkChk->ResizeToPreferred();

    BStringView *restoreStrView = new BStringView (BRect (m_margin, m_storeArkChk->Frame().bottom + m_vGap +
                    m_storeArkChk->Frame().Height() / 2, 0, 0), NULL, str (S_PREFS_STATE_RESTORE));
    restoreStrView->SetFont (&m_sectionFont);
    restoreStrView->ResizeToPreferred();
    restoreStrView->SetLowColor (ViewColor());
                    
    m_restoreUIChk = new BCheckBox (BRect (3 * m_margin, restoreStrView->Frame().bottom + m_vGap, 0, 0),
                                "PrefsViewState:restoreUIChk", str (S_PREFS_STATE_UI), NULL);
    m_restoreUIChk->ResizeToPreferred();

    m_restoreArkChk = new BCheckBox (BRect (3 * m_margin, m_restoreUIChk->Frame().bottom + m_vGap, 0, 0),
                                "PrefsViewState:restoreArkChk", str (S_PREFS_STATE_ARK), NULL);
    m_restoreArkChk->ResizeToPreferred();

    AddChild (storeStrView);
    AddChild (m_storeUIChk);
    AddChild (m_storeArkChk);
    AddChild (restoreStrView);
    AddChild (m_restoreUIChk);
    AddChild (m_restoreArkChk);
    AddRevertButton();
}

//=============================================================================================================//

void PrefsViewState::Save ()
{
    _prefs_state.SetBool (kPfStoreUI, IsChecked (m_storeUIChk));
    _prefs_state.SetBool (kPfRestoreUI, IsChecked (m_restoreUIChk));
    _prefs_state.SetBool (kPfStoreArk, IsChecked (m_storeArkChk));
    _prefs_state.SetBool (kPfRestoreArk, IsChecked (m_restoreArkChk));

    _prefs_state.WritePrefs();
}

//=============================================================================================================//

void PrefsViewState::Load ()
{
    m_storeUIChk->SetValue (_prefs_state.FindBoolDef (kPfStoreUI, false));
    m_restoreUIChk->SetValue (_prefs_state.FindBoolDef (kPfRestoreUI, true));
    m_storeArkChk->SetValue (_prefs_state.FindBoolDef (kPfStoreArk, false));
    m_restoreArkChk->SetValue (_prefs_state.FindBoolDef (kPfRestoreArk, true));
}

//=============================================================================================================//
