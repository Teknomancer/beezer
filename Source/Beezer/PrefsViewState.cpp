/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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
#include "LangStrings.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewState.h"
#include "UIConstants.h"

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
