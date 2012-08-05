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

#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewMisc.h"
#include "UIConstants.h"



PrefsViewMisc::PrefsViewMisc(BRect frame)
    : PrefsView(frame, str(S_PREFS_TITLE_MISC), str(S_PREFS_DESC_MISC))
{
    SetBitmap(ResBitmap("Img:Prefs_Miscellaneous"));
    Render();
}



void PrefsViewMisc::Render()
{
    BString buf2 = str(S_PREFS_MISC_STARTUP);
    buf2.ReplaceAll("%s", K_APP_TITLE);

    float maxWidth = MAX(be_plain_font->StringWidth(str(S_PREFS_MISC_ALLCLOSE)),
                         be_plain_font->StringWidth(buf2.String())) + 5;

    // Add the startup fields
    m_startupPopUp = new BPopUpMenu("");
    m_startupField = new BMenuField(BRect(m_margin, m_margin, Frame().Width(), 0),
                                    "PrefsViewMisc:startupField", buf2.String(), (BMenu*)m_startupPopUp,
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_startupField->SetDivider(maxWidth);
    m_startupField->SetAlignment(B_ALIGN_RIGHT);
    m_startupPopUp->AddItem(new BMenuItem(str(S_PREFS_MISC_WELCOME), NULL));
    m_startupPopUp->AddItem(new BMenuItem(str(S_PREFS_MISC_CREATE), NULL));
    m_startupPopUp->AddItem(new BMenuItem(str(S_PREFS_MISC_OPEN), NULL));
    m_startupPopUp->ResizeToPreferred();


    // Add the quit fields
    m_quitPopUp = new BPopUpMenu("");
    m_quitField = new BMenuField(BRect(m_margin, m_startupField->Frame().top + m_vGap, Frame().Width(), 0),
                                 "PrefsViewMisc:quitField", str(S_PREFS_MISC_ALLCLOSE), (BMenu*)m_quitPopUp,
                                 B_FOLLOW_LEFT, B_WILL_DRAW);
    m_quitField->SetDivider(maxWidth);
    m_quitField->SetAlignment(B_ALIGN_RIGHT);
    m_quitPopUp->AddItem(new BMenuItem(str(S_PREFS_MISC_WELCOME), NULL));

    BString buf = str(S_PREFS_MISC_QUIT);
    buf.ReplaceAll("%s", K_APP_TITLE);

    m_quitPopUp->AddItem(new BMenuItem(buf.String(), NULL));
    m_quitPopUp->ResizeToPreferred();
    m_quitField->MoveBy(0, m_quitPopUp->Frame().Height() - m_margin - m_vGap);

    // Add other controls
    m_commentChk = new BCheckBox(BRect(m_margin, 2 * m_quitPopUp->Frame().Height(), 0, 0),
                                 "PrefsViewMisc:commentChk", str(S_PREFS_MISC_COMMENTS), NULL, B_FOLLOW_LEFT,
                                 B_WILL_DRAW | B_NAVIGABLE);
    m_commentChk->ResizeToPreferred();

    buf = str(S_PREFS_MISC_MIME);
    buf.ReplaceAll("%s", K_APP_TITLE);
    m_mimeChk = new BCheckBox(BRect(m_margin, m_commentChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewMisc:mimeChk", buf.String(), NULL, B_FOLLOW_LEFT,
                              B_WILL_DRAW | B_NAVIGABLE);
    m_mimeChk->ResizeToPreferred();

    float btnWidth = MAX(K_BUTTON_WIDTH, StringWidth(str(S_PREFS_MISC_MIMENOW)) + 22);
    m_mimeBtn = new BButton(BRect(5 * m_margin, m_mimeChk->Frame().bottom + m_vGap + 2,
                                  5 * m_margin + btnWidth, m_mimeChk->Frame().bottom + m_vGap + 2 + K_BUTTON_HEIGHT),
                            "PrefsViewMisc:mimeBtn", str(S_PREFS_MISC_MIMENOW), new BMessage(M_REGISTER_TYPES),
                            B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    m_arkTypePopUp = new BPopUpMenu("");
    m_arkTypeField = new BMenuField(BRect(m_margin, m_mimeBtn->Frame().bottom + 2 * m_margin,
                                          Frame().Width(), 0), "PrefsViewMisc:arkTypeField", str(S_PREFS_MISC_DEFARK),
                                    (BMenu*)m_arkTypePopUp, B_FOLLOW_LEFT, B_WILL_DRAW);
    m_arkTypeField->SetDivider(be_plain_font->StringWidth(str(S_PREFS_MISC_DEFARK)) + 5);

    m_arkTypes = ArchiversInstalled(NULL);
    for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
        m_arkTypePopUp->AddItem(new BMenuItem((const char*)m_arkTypes.ItemAtFast(i), NULL));

    m_arkTypePopUp->ResizeToPreferred();

    AddChild(m_quitField);
    AddChild(m_startupField);
    AddChild(m_commentChk);
    AddChild(m_mimeChk);
    AddChild(m_mimeBtn);
    AddChild(m_arkTypeField);
    AddRevertButton();
}



void PrefsViewMisc::Save()
{
    _prefs_misc.SetBool(kPfWelcomeOnQuit, m_quitPopUp->ItemAt(0)->IsMarked());
    _prefs_misc.SetInt8(kPfStartup, m_startupPopUp->IndexOf(m_startupPopUp->FindMarked()));
    _prefs_misc.SetBool(kPfShowCommentOnOpen, IsChecked(m_commentChk));
    _prefs_misc.SetBool(kPfMimeOnStartup, IsChecked(m_mimeChk));

    BMenuItem* item = m_arkTypePopUp->FindMarked();
    if (item)
        _prefs_misc.SetString(kPfDefaultArk, item->Label());

    _prefs_misc.WritePrefs();
}



void PrefsViewMisc::Load()
{
    if (_prefs_misc.FindBoolDef(kPfWelcomeOnQuit, true))
        m_quitPopUp->ItemAt(0)->SetMarked(true);
    else
        m_quitPopUp->ItemAt(1)->SetMarked(true);

    int8 startupAction = _prefs_misc.FindInt8Def(kPfStartup, 0);
    m_startupPopUp->ItemAt(startupAction)->SetMarked(true);

    m_commentChk->SetValue(_prefs_misc.FindBoolDef(kPfShowCommentOnOpen, true));
    m_mimeChk->SetValue(_prefs_misc.FindBoolDef(kPfMimeOnStartup, false));

    BString arkType;
    BMenuItem* item = NULL;
    status_t wasFound = _prefs_misc.FindString(kPfDefaultArk, &arkType);
    if (wasFound == B_OK)
        item = m_arkTypePopUp->FindItem(arkType.String());

    if (wasFound != B_OK || item == NULL)
    {
        if (m_arkTypePopUp->CountItems() > 0)
            item = m_arkTypePopUp->ItemAt(m_arkTypePopUp->CountItems() - 1);
    }

    if (item)
        item->SetMarked(true);
}



void PrefsViewMisc::AttachedToWindow()
{
    m_mimeBtn->SetTarget(this);
    return PrefsView::AttachedToWindow();
}



void PrefsViewMisc::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_REGISTER_TYPES:
        {
            be_app->PostMessage(message);
            break;
        }

        default:
            PrefsView::MessageReceived(message);
            break;
    }
}


