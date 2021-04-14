// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewRecent.h"
#include "CommonStrings.h"
#include "BitmapPool.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewRecent"
#else
#define B_TRANSLATE(x) x
#endif

#include <cstdlib>


PrefsViewRecent::PrefsViewRecent(BRect frame)
    : PrefsView(frame, BZ_TR(kRecentString),
                B_TRANSLATE("Configure recent archives & extract locations"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Recent", 20, 20));
    Render();
}


void PrefsViewRecent::Render()
{

    BStringView* arkStrView = new BStringView(NULL, B_TRANSLATE("Recent archives"));
    arkStrView->SetFont(&m_sectionFont);

    m_recentArkView = new BTextControl("PrefsViewRecent:recentArkView", B_TRANSLATE("Number of recent archives"), NULL, NULL,
                                       B_WILL_DRAW | B_NAVIGABLE);
    m_recentArkView->TextView()->SetMaxBytes(2);
    m_recentArkView->TextView()->DisallowChar(B_INSERT);

    m_showPathChk = new BCheckBox("PrefsViewRecent:showPathChk", B_TRANSLATE("Show full path in recent archives"), NULL);

    BStringView* extStrView = new BStringView(NULL, B_TRANSLATE("Recent extract paths"));
    extStrView->SetFont(&m_sectionFont);

    m_recentExtView = new BTextControl("PrefsViewRecent:recentExtView", B_TRANSLATE("Number of recent extract paths"), NULL, NULL,
                                       B_WILL_DRAW | B_NAVIGABLE);
    m_recentExtView->TextView()->SetMaxBytes(2);
    m_recentExtView->TextView()->DisallowChar(B_INSERT);

    // split the BTextControls so that we have finer control over the size and layout
    BLayoutItem* recentArkLabel = m_recentArkView->CreateLabelLayoutItem();
    BLayoutItem* recentArkInput = m_recentArkView->CreateTextViewLayoutItem();
    recentArkInput->SetExplicitMinSize(BSize(StringWidth("999"), B_SIZE_UNSET));
    recentArkInput->SetExplicitMaxSize(BSize(StringWidth("99999"), B_SIZE_UNSET));

    BLayoutItem* recentExtLabel = m_recentExtView->CreateLabelLayoutItem();
    BLayoutItem* recentExtInput = m_recentExtView->CreateTextViewLayoutItem();
    recentExtInput->SetExplicitMinSize(BSize(StringWidth("999"), B_SIZE_UNSET));
    recentExtInput->SetExplicitMaxSize(BSize(StringWidth("99999"), B_SIZE_UNSET));

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .Add(arkStrView)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(B_USE_ITEM_SPACING)
            .AddGroup(B_HORIZONTAL, 0)
                .Add(recentArkLabel)
                .Add(recentArkInput)
            .End()
            .AddGlue()
        .End()
        .AddGroup(B_HORIZONTAL)
            .AddStrut(B_USE_ITEM_SPACING)
            .Add(m_showPathChk)
            .AddGlue()
        .End()
        .AddStrut(B_USE_ITEM_SPACING) // spacer between sections
        .Add(extStrView)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(B_USE_ITEM_SPACING)
            .AddGroup(B_HORIZONTAL, 0)
                .Add(recentExtLabel)
                .Add(recentExtInput)
            .End()
            .AddGlue()
        .End()
        .AddGlue() // add some extra space at the bottom
        .End();

    AddRevertButton(builder);
}


void PrefsViewRecent::Save()
{
    int8 const numRecentArks = abs(atoi(m_recentArkView->Text()));
    int8 const numRecentExt = abs(atoi(m_recentExtView->Text()));

    _prefs_recent.SetBool(kPfShowPathInRecent, IsChecked(m_showPathChk));
    _prefs_recent.SetInt8(kPfNumRecentArk, numRecentArks);
    _prefs_recent.SetInt8(kPfNumRecentExt, numRecentExt);
    _prefs_state.WritePrefs();
}


void PrefsViewRecent::Load()
{
    m_showPathChk->SetValue(_prefs_recent.FindBoolDef(kPfShowPathInRecent, false));

    int8 const numRecentArks = abs(_prefs_recent.GetInt8(kPfNumRecentArk, 10));
    BString bufRecentArks;
    bufRecentArks << numRecentArks;
    m_recentArkView->SetText(bufRecentArks.String());

    int8 const numRecentExt = abs(_prefs_recent.GetInt8(kPfNumRecentExt, 5));
    BString bufRecentExt;
    bufRecentExt << numRecentExt;
    m_recentExtView->SetText(bufRecentExt.String());
}
