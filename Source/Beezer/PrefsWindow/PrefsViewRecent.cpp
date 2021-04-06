// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <CheckBox.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include <stdlib.h>

#include "AppConstants.h"
#include "CommonStrings.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewRecent.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewRecent"
#else
#define B_TRANSLATE(x) x
#endif


PrefsViewRecent::PrefsViewRecent(BRect frame)
    : PrefsView(frame, B_TRANSLATE(skRecentString),
                B_TRANSLATE("Configure recent archives & extract locations"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Recent", 20, 20));
    Render();
}


void PrefsViewRecent::Render()
{
    font_height fntHt;
    be_plain_font->GetHeight(&fntHt);
    float const lineHeight = fntHt.ascent + fntHt.descent + fntHt.leading;

    BStringView* arkStrView = new BStringView(BRect(m_margin, m_margin, 0, 0), NULL,
            B_TRANSLATE("Recent archives"));
    arkStrView->SetFont(&m_sectionFont);
    arkStrView->ResizeToPreferred();

    float strW = StringWidth(B_TRANSLATE("Number of recent archives"));
    strW += 6;

    m_recentArkView = new BTextControl(BRect(3 * m_margin, arkStrView->Frame().bottom + m_vGap + 2,
                                       3 * m_margin + strW + StringWidth("WWW"),
                                       arkStrView->Frame().bottom + m_vGap + 2 + lineHeight),
                                       "PrefsViewRecent:recentArkView", B_TRANSLATE("Number of recent archives"), NULL, NULL,
                                       B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_recentArkView->TextView()->SetMaxBytes(2);
    m_recentArkView->TextView()->DisallowChar(B_INSERT);
    m_recentArkView->SetDivider(strW);

    m_showPathChk = new BCheckBox(BRect(3 * m_margin, m_recentArkView->Frame().bottom + m_vGap + 2, 0, 0),
                                  "PrefsViewRecent:showPathChk", B_TRANSLATE("Show full path in recent archives"), NULL);
    m_showPathChk->ResizeToPreferred();

    BStringView* extStrView = new BStringView(BRect(m_margin, m_showPathChk->Frame().bottom + m_vGap + 8, 0, 0),
            NULL, B_TRANSLATE("Recent extract paths"));
    extStrView->SetFont(&m_sectionFont);
    extStrView->ResizeToPreferred();

    strW = StringWidth(B_TRANSLATE("Number of recent extract paths"));
    strW += 6;
    m_recentExtView = new BTextControl(BRect(3 * m_margin, extStrView->Frame().bottom + m_vGap + 2,
                                       3 * m_margin + strW + StringWidth("WWW"),
                                       extStrView->Frame().bottom + m_vGap + 2 + lineHeight),
                                       "PrefsViewRecent:recentExtView", B_TRANSLATE("Number of recent extract paths"), NULL, NULL,
                                       B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_recentExtView->TextView()->SetMaxBytes(2);
    m_recentExtView->TextView()->DisallowChar(B_INSERT);
    m_recentExtView->SetDivider(strW);

    AddChild(arkStrView);
    AddChild(m_recentArkView);
    AddChild(m_showPathChk);
    AddChild(extStrView);
    AddChild(m_recentExtView);
    AddRevertButton();
}


void PrefsViewRecent::Save()
{
    _prefs_recent.SetBool(kPfShowPathInRecent, IsChecked(m_showPathChk));
    _prefs_recent.SetInt8(kPfNumRecentArk, (int8)abs(atoi(m_recentArkView->Text())));
    _prefs_recent.SetInt8(kPfNumRecentExt, (int8)abs(atoi(m_recentExtView->Text())));
    _prefs_state.WritePrefs();
}


void PrefsViewRecent::Load()
{
    m_showPathChk->SetValue(_prefs_recent.FindBoolDef(kPfShowPathInRecent, false));

    int8 num = 0;
    BString buf;

    if (_prefs_recent.FindInt8(kPfNumRecentArk, &num) != B_OK)
        num = 10;

    buf << num;
    m_recentArkView->SetText(buf.String());
    buf = "";

    if (_prefs_recent.FindInt8(kPfNumRecentExt, &num) != B_OK)
        num = 5;

    buf << num;
    m_recentExtView->SetText(buf.String());
}
