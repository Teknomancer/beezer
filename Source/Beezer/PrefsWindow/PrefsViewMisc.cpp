// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

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
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewMisc.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewMisc"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


PrefsViewMisc::PrefsViewMisc(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Miscellaneous"), B_TRANSLATE("Other general settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs", 20, 20));
    Render();
}


void PrefsViewMisc::Render()
{
    BString buf2 = B_TRANSLATE("When %appname% starts:");
    buf2.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    buf2 << " ";

    float maxWidth = MAX(be_plain_font->StringWidth(B_TRANSLATE("When last archive is closed:")),
                         be_plain_font->StringWidth(buf2.String())) + 5;

    // Add the startup fields
    m_startupPopUp = new BPopUpMenu("");
    m_startupField = new BMenuField(BRect(m_margin, m_margin, Frame().Width(), 0),
                                    "PrefsViewMisc:startupField", buf2.String(), (BMenu*)m_startupPopUp,
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_startupField->SetDivider(maxWidth);
    m_startupField->SetAlignment(B_ALIGN_RIGHT);
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show welcome window"), NULL));
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show create archive panel"), NULL));
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show open archive panel"), NULL));
    m_startupPopUp->ResizeToPreferred();


    // Add the quit fields
    m_quitPopUp = new BPopUpMenu("");
    m_quitField = new BMenuField(BRect(m_margin, m_startupField->Frame().top + m_vGap, Frame().Width(), 0),
                                 "PrefsViewMisc:quitField", B_TRANSLATE("When last archive is closed:"), (BMenu*)m_quitPopUp,
                                 B_FOLLOW_LEFT, B_WILL_DRAW);
    m_quitField->SetDivider(maxWidth);
    m_quitField->SetAlignment(B_ALIGN_RIGHT);
    m_quitPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show welcome window"), NULL));

    BString buf = B_TRANSLATE("Quit %appname%");
    buf.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));

    m_quitPopUp->AddItem(new BMenuItem(buf.String(), NULL));
    m_quitPopUp->ResizeToPreferred();
    m_quitField->MoveBy(0, m_quitPopUp->Frame().Height() - m_margin - m_vGap);

    // Add other controls
    m_commentChk = new BCheckBox(BRect(m_margin, 2 * m_quitPopUp->Frame().Height(), 0, 0),
                                 "PrefsViewMisc:commentChk", B_TRANSLATE("Show comments (if any) after opening an archive"),
                                 NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_commentChk->ResizeToPreferred();

    m_mimeChk = new BCheckBox(BRect(m_margin, m_commentChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewMisc:mimeChk", B_TRANSLATE("Check file types at startup"), NULL, B_FOLLOW_LEFT,
                              B_WILL_DRAW | B_NAVIGABLE);
    m_mimeChk->ResizeToPreferred();

    float btnWidth = MAX(K_BUTTON_WIDTH, StringWidth(B_TRANSLATE("Register file types now")) + 22);
    m_mimeBtn = new BButton(BRect(5 * m_margin, m_mimeChk->Frame().bottom + m_vGap + 2,
                                  5 * m_margin + btnWidth, m_mimeChk->Frame().bottom + m_vGap + 2 + K_BUTTON_HEIGHT),
                            "PrefsViewMisc:mimeBtn", B_TRANSLATE("Register file types now"), new BMessage(M_REGISTER_TYPES),
                            B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    m_arkTypePopUp = new BPopUpMenu("");
    m_arkTypeField = new BMenuField(BRect(m_margin, m_mimeBtn->Frame().bottom + 2 * m_margin,
                                          Frame().Width(), 0), "PrefsViewMisc:arkTypeField", B_TRANSLATE("Default archiver:"),
                                    (BMenu*)m_arkTypePopUp, B_FOLLOW_LEFT, B_WILL_DRAW);
    m_arkTypeField->SetDivider(be_plain_font->StringWidth(B_TRANSLATE("Default archiver:")) + 5);

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
