// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewAdd.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewAdd"
#else
#define B_TRANSLATE(x) x
#endif

#include <cstdlib> // gcc2

static const uint32 M_WARN = 'warn';


PrefsViewAdd::PrefsViewAdd(BRect frame)
    : PrefsView(frame, BZ_TR(kAddString), B_TRANSLATE("Settings related to adding entries to archives"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Add", 20, 20));
    Render();
}


void PrefsViewAdd::Render()
{
    m_replaceMenu = new BPopUpMenu("");
    m_replaceMenu->AddItem(new BMenuItem(B_TRANSLATE("Never replace"), NULL));
    m_replaceMenu->AddItem(new BMenuItem(B_TRANSLATE("Ask before replacing"), NULL));
    m_replaceMenu->AddItem(new BMenuItem(B_TRANSLATE("Without asking"), NULL));
    m_replaceMenu->AddItem(new BMenuItem(B_TRANSLATE("When new file is more recent"), NULL));

    m_replaceField = new BMenuField("PrefsViewAdd:replaceField", B_TRANSLATE("Replace files:"), (BMenu*)m_replaceMenu,
                                    B_WILL_DRAW | B_NAVIGABLE);

    m_warnMBChk = new BCheckBox("PrefsViewAdd:warnMBChk", B_TRANSLATE("Confirm when adding more than "), new BMessage(M_WARN),
                                B_WILL_DRAW | B_NAVIGABLE);

    m_mbView = new BTextControl("PrefsViewAdd:mbView", NULL, NULL, NULL, B_WILL_DRAW | B_NAVIGABLE);
    m_mbView->TextView()->DisallowChar(B_INSERT);
    m_mbView->TextView()->SetMaxBytes(4);

    BStringView* mbStrView = new BStringView("PrefsViewAdd:mbStrView", BZ_TR(kMegabyteString), B_WILL_DRAW);

    m_dropChk = new BCheckBox("PrefsViewAdd:dropChk", B_TRANSLATE("Confirm when adding through drag 'n drop"), NULL,
                              B_WILL_DRAW | B_NAVIGABLE);

    m_sortChk = new BCheckBox("PrefsViewAdd:sortChk", B_TRANSLATE("Sort after add (n/a for reloading archivers)"), NULL,
                              B_WILL_DRAW | B_NAVIGABLE);

    // split the BTextControl but only use the input since the label is NULL
    BLayoutItem* mbInput = m_mbView->CreateTextViewLayoutItem();
    // add a few extra 9's to account for the border of the text input
    mbInput->SetExplicitMinSize(BSize(StringWidth("9999"), B_SIZE_UNSET));
    mbInput->SetExplicitMaxSize(BSize(StringWidth("999999"), B_SIZE_UNSET));

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .AddGroup(B_HORIZONTAL)
            // put it in a group with glue so that it doesn't extend the entire length
            .Add(m_replaceField)
            .AddGlue()
        .End()
        .AddGroup(B_HORIZONTAL, 0) // 0 spacing so the text/textcontrol is close to each other
            .Add(m_warnMBChk)
            .Add(mbInput)
            .Add(mbStrView)
            .AddGlue()
        .End()
        .Add(m_dropChk)
        .Add(m_sortChk)
        .AddGlue() // add some free space at the bottom
        .End();

    AddRevertButton(builder);
}


void PrefsViewAdd::AttachedToWindow()
{
    m_warnMBChk->SetTarget(this);
    return PrefsView::AttachedToWindow();
}


void PrefsViewAdd::Save()
{
    _prefs_add.SetInt8(kPfReplaceFiles, m_replaceMenu->IndexOf(m_replaceMenu->FindMarked()));
    _prefs_add.SetBool(kPfWarnBeforeAdd, IsChecked(m_warnMBChk));
    _prefs_add.SetInt16(kPfWarnAmount, (int16)abs(atoi(m_mbView->Text())));
    _prefs_add.SetBool(kPfSortAfterAdd, IsChecked(m_sortChk));
    _prefs_add.SetBool(kPfConfirmDropAdd, IsChecked(m_dropChk));
    _prefs_state.WritePrefs();
}


void PrefsViewAdd::Load()
{
    m_replaceMenu->ItemAt(_prefs_add.FindInt8Def(kPfReplaceFiles, 1))->SetMarked(true);

    bool const warn = _prefs_add.FindBoolDef(kPfWarnBeforeAdd, true);
    m_warnMBChk->SetValue(warn);
    if (!warn)
        ToggleMBView(false);

    BString buf;
    int16 const mbSize = _prefs_add.FindInt16Def(kPfWarnAmount, 100);
    buf << mbSize;
    m_mbView->SetText(buf.String());

    m_sortChk->SetValue(_prefs_add.FindBoolDef(kPfSortAfterAdd, true));
    m_dropChk->SetValue(_prefs_add.FindBoolDef(kPfConfirmDropAdd, true));
}


void PrefsViewAdd::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_WARN:
        {
            ToggleMBView(m_warnMBChk->Value() == B_CONTROL_ON ? true : false);
            break;
        }

        default:
            PrefsView::MessageReceived(message);
            break;
    }
}


void PrefsViewAdd::ToggleMBView(bool enable)
{
    m_mbView->SetEnabled(enable);
}
