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

#include <String.h>
#include <CheckBox.h>
#include <StringView.h>
#include <TextControl.h>

#include <stdlib.h>

#include "PrefsViewRecent.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "AppConstants.h"
#include "Preferences.h"

//=============================================================================================================//

PrefsViewRecent::PrefsViewRecent (BRect frame)
    : PrefsView (frame, str (S_PREFS_TITLE_RECENT), str (S_PREFS_DESC_RECENT))
{
    SetBitmap (ResBitmap ("Img:Prefs_Recent"));
    Render();
}

//=============================================================================================================//

void PrefsViewRecent::Render ()
{
    BStringView *arkStrView = new BStringView (BRect (m_margin, m_margin, 0, 0), NULL,
                                    str (S_PREFS_RECENT_ARCHIVES));
    arkStrView->SetFont (&m_sectionFont);
    arkStrView->ResizeToPreferred();
    arkStrView->SetLowColor (ViewColor());
                         
    float strW = StringWidth (str (S_PREFS_RECENT_NUMARK));
    strW += 6;
    
    m_recentArkView = new BTextControl (BRect (3 * m_margin, arkStrView->Frame().bottom + m_vGap + 2,
                             3 * m_margin + strW + StringWidth("WWW"), 0),
                             "PrefsViewRecent:recentArkView", str (S_PREFS_RECENT_NUMARK), NULL, NULL,
                             B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_recentArkView->TextView()->SetMaxBytes (2);
    m_recentArkView->TextView()->DisallowChar (B_INSERT);
    m_recentArkView->SetDivider (strW);

    m_showPathChk = new BCheckBox (BRect (3 * m_margin, m_recentArkView->Frame().bottom + m_vGap + 2, 0, 0),
                         "PrefsViewRecent:showPathChk", str (S_PREFS_RECENT_SHOWPATH), NULL);
    m_showPathChk->ResizeToPreferred();

    BStringView *extStrView = new BStringView (BRect (m_margin, m_showPathChk->Frame().bottom + m_vGap + 8, 0,0),
                                NULL, str (S_PREFS_RECENT_EXTRACTS));
    extStrView->SetFont (&m_sectionFont);
    extStrView->ResizeToPreferred();
    extStrView->SetLowColor (ViewColor());

    strW = StringWidth (str (S_PREFS_RECENT_NUMEXT));
    strW += 6;
    m_recentExtView = new BTextControl (BRect (3 * m_margin, extStrView->Frame().bottom + m_vGap + 2,
                             3 * m_margin + strW + StringWidth("WWW"), 0),
                             "PrefsViewRecent:recentExtView", str (S_PREFS_RECENT_NUMEXT), NULL, NULL,
                             B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_recentExtView->TextView()->SetMaxBytes (2);
    m_recentExtView->TextView()->DisallowChar (B_INSERT);
    m_recentExtView->SetDivider (strW);

    AddChild (arkStrView);
    AddChild (m_recentArkView);
    AddChild (m_showPathChk);
    AddChild (extStrView);
    AddChild (m_recentExtView);
    AddRevertButton();
}

//=============================================================================================================//

void PrefsViewRecent::Save ()
{
    _prefs_recent.SetBool (kPfShowPathInRecent, IsChecked (m_showPathChk));
    _prefs_recent.SetInt8 (kPfNumRecentArk, (int8)abs(atoi (m_recentArkView->Text())));
    _prefs_recent.SetInt8 (kPfNumRecentExt, (int8)abs(atoi (m_recentExtView->Text())));
    _prefs_state.WritePrefs();
}

//=============================================================================================================//

void PrefsViewRecent::Load ()
{
    m_showPathChk->SetValue (_prefs_recent.FindBoolDef (kPfShowPathInRecent, false));
    
    int8 num = 0;
    BString buf;
    
    if (_prefs_recent.FindInt8 (kPfNumRecentArk, &num) != B_OK)
        num = 10;
    
    buf << num;
    m_recentArkView->SetText (buf.String());
    buf = "";
    
    if (_prefs_recent.FindInt8 (kPfNumRecentExt, &num) != B_OK)
        num = 5;
    
    buf << num;
    m_recentExtView->SetText (buf.String());
}

//=============================================================================================================//
