/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include <stdlib.h>

#include "AppConstants.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewAdd.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewAdd"
#else
#define B_TRANSLATE(x) x
#endif


#define M_WARN               'warn'



PrefsViewAdd::PrefsViewAdd(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Add"), B_TRANSLATE("Settings related to adding entries to archives"))
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

    m_replaceField = new BMenuField(BRect(m_margin, m_margin, Bounds().right - m_margin, 0),
                                    "PrefsViewAdd:replaceField", B_TRANSLATE("Replace files:"), (BMenu*)m_replaceMenu,
                                    B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_replaceField->SetDivider(StringWidth(m_replaceField->Label()) + StringWidth("W"));

    font_height fntHt;
    be_plain_font->GetHeight(&fntHt);

    m_warnMBChk = new BCheckBox(BRect(m_margin, 3 * m_margin + fntHt.ascent + fntHt.descent + m_vGap + 4, 0, 0),
                                "PrefsViewAdd:warnMBChk", B_TRANSLATE("Confirm when adding more than "), new BMessage(M_WARN), B_FOLLOW_LEFT,
                                B_WILL_DRAW | B_NAVIGABLE);
    m_warnMBChk->ResizeToPreferred();

    m_mbView = new BTextControl(BRect(m_warnMBChk->Frame().right, m_warnMBChk->Frame().top - 2,
                                      m_warnMBChk->Frame().right + StringWidth("88888") + 4, 0), "PrefsViewAdd:mbView",
                                NULL, NULL, NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_mbView->TextView()->DisallowChar(B_INSERT);
    m_mbView->TextView()->SetMaxBytes(4);
    m_mbView->SetDivider(0);

    BStringView* mbStrView = new BStringView(BRect(m_mbView->Frame().right + 4, m_warnMBChk->Frame().top + 1,
            0, 0), "PrefsViewAdd:mbStrView", B_TRANSLATE("MiB"), B_FOLLOW_LEFT, B_WILL_DRAW);
    mbStrView->ResizeToPreferred();

    m_dropChk = new BCheckBox(BRect(m_margin,    m_warnMBChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewAdd:dropChk", B_TRANSLATE("Confirm when adding through drag 'n drop"), NULL, B_FOLLOW_LEFT,
                              B_WILL_DRAW | B_NAVIGABLE);
    m_dropChk->ResizeToPreferred();

    m_sortChk = new BCheckBox(BRect(m_margin, m_dropChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewAdd:sortChk", B_TRANSLATE("Sort after add (n/a for reloading archivers)"), NULL, B_FOLLOW_LEFT,
                              B_WILL_DRAW | B_NAVIGABLE);
    m_sortChk->ResizeToPreferred();

    AddChild(m_replaceField);
    AddChild(m_warnMBChk);
    AddChild(m_mbView);
    AddChild(mbStrView);
    AddChild(m_dropChk);
    AddChild(m_sortChk);
    AddRevertButton();
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

    bool warn = _prefs_add.FindBoolDef(kPfWarnBeforeAdd, true);
    m_warnMBChk->SetValue(warn);
    if (!warn)
        ToggleMBView(false);

    BString buf;
    int16 mbSize = _prefs_add.FindInt16Def(kPfWarnAmount, 100);
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


