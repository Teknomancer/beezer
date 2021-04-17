// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewMisc.h"
#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "BitmapPool.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

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
    BString bufStr(B_TRANSLATE("When %appname% starts:"));
    bufStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));

    // Add the startup fields
    m_startupPopUp = new BPopUpMenu("");
    m_startupField = new BMenuField("PrefsViewMisc:startupField", bufStr, (BMenu*)m_startupPopUp, B_WILL_DRAW);
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show welcome window"), NULL));
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show create archive panel"), NULL));
    m_startupPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show open archive panel"), NULL));

    // Add the quit fields
    m_quitPopUp = new BPopUpMenu("");
    m_quitField = new BMenuField("PrefsViewMisc:quitField", B_TRANSLATE("When last archive is closed:"), (BMenu*)m_quitPopUp,
                                 B_WILL_DRAW);
    m_quitPopUp->AddItem(new BMenuItem(B_TRANSLATE("Show welcome window"), NULL));

    bufStr = B_TRANSLATE("Quit %appname%");
    bufStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    m_quitPopUp->AddItem(new BMenuItem(bufStr, NULL));

    // Add other controls
    m_commentChk = new BCheckBox("PrefsViewMisc:commentChk", B_TRANSLATE("Show comments (if any) after opening an archive"),
                                 NULL, B_WILL_DRAW | B_NAVIGABLE);

    m_mimeChk = new BCheckBox("PrefsViewMisc:mimeChk", B_TRANSLATE("Check file types at startup"), NULL,
                              B_WILL_DRAW | B_NAVIGABLE);

    m_mimeBtn = new BButton("PrefsViewMisc:mimeBtn", B_TRANSLATE("Register file types now"), new BMessage(M_REGISTER_TYPES),
                            B_WILL_DRAW | B_NAVIGABLE);

    m_arkTypePopUp = new BPopUpMenu("");
    m_arkTypeField = new BMenuField("PrefsViewMisc:arkTypeField", B_TRANSLATE("Default archiver:"), (BMenu*)m_arkTypePopUp,
                                    B_WILL_DRAW);

    ArchiversInstalled(m_arkTypes, NULL);
    for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
        m_arkTypePopUp->AddItem(new BMenuItem((const char*)m_arkTypes.ItemAtFast(i), NULL));

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .AddGrid(0.0, B_USE_SMALL_SPACING)
            .AddMenuField(m_startupField, 0, 0, B_ALIGN_RIGHT)
            .AddMenuField(m_quitField, 0, 1, B_ALIGN_RIGHT)
            .AddGlue(2, 0, 1, 2) // so the menufields don't extend the entire width
        .End()
        .AddStrut(B_USE_ITEM_SPACING) // spacer between sections
        .Add(m_commentChk)
        .AddGroup(B_VERTICAL, 0) // vertical group with no spacing to get the checkbox and button closer
            .Add(m_mimeChk)
            .AddGroup(B_HORIZONTAL)
                .AddStrut(B_USE_ITEM_SPACING)
                .Add(m_mimeBtn)
                .AddGlue() // padding on the right to push the button left
            .End()
        .End()
        .AddStrut(B_USE_ITEM_SPACING) // spacer between sections
        .AddGroup(B_HORIZONTAL, 0)
            .Add(m_arkTypeField)
            .AddGlue(2.0) // heavy glue to force the menufield even smaller
        .End()
        .AddGlue() // add some extra space at the bottom
        .End();

    AddRevertButton(builder);
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
