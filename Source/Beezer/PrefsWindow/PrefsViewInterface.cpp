// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewInterface.h"
#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "CommonStrings.h"
#include "BeezerApp.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "UIConstants.h"

#include <CheckBox.h>
#include <ColorControl.h>
#include <File.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewInterface"
#else
#define B_TRANSLATE(x) x
#endif

static const uint32 M_COLOR_CHANGE = 'clch';
static const uint32 M_ITEM_CHANGE  = 'itch';


PrefsViewInterface::PrefsViewInterface(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Interface"), B_TRANSLATE("Configure graphical user interface settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Interface", 20, 20));
    Render();
}


void PrefsViewInterface::Render()
{
    m_fullLengthBarsChk = new BCheckBox("PrefsViewInterface:fullLenBars", B_TRANSLATE("Full length toolbar & infobar"), NULL);

    BStringView* colorStrView = new BStringView("PrefsViewInterface:colorStrView", B_TRANSLATE("Configure colors:"), B_WILL_DRAW);
    colorStrView->SetFont(&m_sectionFont);

    BevelView* colorWellContainerView = new BevelView(BRect(3 * m_margin, colorStrView->Frame().bottom + m_margin, 3 * m_margin + 30,
                                                   colorStrView->Frame().bottom + m_margin + 30),
                                             "PrefsViewInterface:colorWellContainerView", BevelView::bevel_type::DEEP,
                                             B_FOLLOW_LEFT, B_WILL_DRAW);
    float const edgeThickness = colorWellContainerView->EdgeThickness();
    m_colorWell = new BView(colorWellContainerView->Bounds().InsetBySelf(edgeThickness, edgeThickness),
                            "PrefsViewInterface:colorWell", B_FOLLOW_LEFT, B_WILL_DRAW);
    colorWellContainerView->AddChild(m_colorWell);
    colorWellContainerView->SetExplicitMaxSize(BSize(30, 30));

    m_colorPopUp = new BPopUpMenu("");
    m_colorPopUp->AddItem(new BMenuItem(B_TRANSLATE("Selected text color"), new BMessage(M_ITEM_CHANGE)));
    m_colorPopUp->AddItem(new BMenuItem(B_TRANSLATE("Selected background color"), new BMessage(M_ITEM_CHANGE)));
    m_colorField = new BMenuField("PrefsViewInterface:colorField", "", (BMenu*)m_colorPopUp);

    m_colorControl = new BColorControl(BPoint(3 * m_margin,
                                              MAX(m_colorPopUp->Frame().bottom, colorWellContainerView->Frame().bottom) + m_margin + 2),
                                       B_CELLS_32x8, 8,
                                       "PrefsViewInteface:colorControl", new BMessage(M_COLOR_CHANGE));

    BStringView* defStrView = new BStringView("PrefsViewInterface:defStrView", B_TRANSLATE("Default interface settings:"), B_WILL_DRAW);
    defStrView->SetFont(&m_sectionFont);

    m_toolbarChk = new BCheckBox("PrefsViewInterface:toolbarChk", B_TRANSLATE("Show toolbar"), NULL);
    m_infobarChk = new BCheckBox("PrefsViewInterface:infoBarChk", B_TRANSLATE("Show infobar"), NULL);
    m_actionLogChk = new BCheckBox("PrefsViewInterface:actionLogChk", B_TRANSLATE("Show action log"), NULL);

    m_foldingPopUp = new BPopUpMenu("");
    m_foldingPopUp->AddItem(new BMenuItem(BZ_TR(kAllFoldedString), NULL));
    m_foldingPopUp->AddItem(new BMenuItem(BZ_TR(kFirstUnfoldedString), NULL));
    m_foldingPopUp->AddItem(new BMenuItem(BZ_TR(kTwoUnfoldedString), NULL));
    m_foldingPopUp->AddItem(new BMenuItem(BZ_TR(kAllUnfoldedString), NULL));
    m_foldingPopUp->ItemAt(3)->SetMarked(true);

    float maxW = MAX(StringWidth(B_TRANSLATE("Show infobar")), StringWidth(B_TRANSLATE("Show toolbar")));
    maxW = MAX(maxW, StringWidth(B_TRANSLATE("Show action log")));
    maxW += 5 * m_margin + 30;

    m_foldingField = new BMenuField("PrefsViewInterface:foldingField", B_TRANSLATE("Folding:"), (BMenu*)m_foldingPopUp);
    float const div = m_foldingField->StringWidth(m_foldingField->Label()) + 10;
    m_foldingField->SetDivider(div);

    font_height fntHt;
    m_sectionFont.GetHeight(&fntHt);

    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .Add(m_fullLengthBarsChk)
        .AddStrut(B_USE_HALF_ITEM_SPACING)  // extra space before starting next logical group
        .Add(colorStrView)
        .AddGroup(B_HORIZONTAL)
            .Add(colorWellContainerView)
            .Add(m_colorField)
            .AddGlue()  // glue so color field doesn't stretch the full width
        .End()
        .Add(m_colorControl)
        .AddStrut(B_USE_ITEM_SPACING)  // extra space before starting next logical group
        .Add(defStrView)
        .AddGroup(B_HORIZONTAL)
            .AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
                .Add(m_toolbarChk)
                .Add(m_infobarChk)
                .Add(m_actionLogChk)
            .End()
            .AddStrut(B_USE_ITEM_SPACING)
            .AddGroup(B_VERTICAL)
                .Add(m_foldingField)
                .AddGlue()      // glue to push the folding field to top
            .End()
        .End()
        .AddGlue() // add some free space at the bottom
        .End();

    AddRevertButton(builder);
}


void PrefsViewInterface::Save()
{
    _prefs_interface.SetBool(kPfFullLengthBars, IsChecked(m_fullLengthBarsChk));
    _prefs_interface.SetColor(kPfActFore, m_actFore);
    _prefs_interface.SetColor(kPfActBack, m_actBack);
    _prefs_interface.SetInt16(kPfColorIndex, (int16)m_colorPopUp->IndexOf(m_colorPopUp->FindMarked()));
    _prefs_interface.WritePrefs();

    // A sort of hack, What we do is load the settings file from the hard-disk and replace
    // the settings
    BMessage msg;
    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;

    BFile file(path.String(), B_READ_ONLY);
    msg.Unflatten(&file);                       // may fail (if settings file is missing, but doesn't matter

    // Change only the fields we need, rest (such as Window size and position, columns etc. remain unchanged
    // and will be preserved)
    msg.RemoveName(kToolBar);
    msg.RemoveName(kInfoBar);
    msg.RemoveName(kSplitter);
    msg.RemoveName(kFolding);
    msg.AddBool(kToolBar, IsChecked(m_toolbarChk));
    msg.AddBool(kInfoBar, IsChecked(m_infobarChk));
    msg.AddBool(kSplitter, IsChecked(m_actionLogChk));
    msg.AddInt8(kFolding, m_foldingPopUp->IndexOf(m_foldingPopUp->FindMarked()));

    // Re-write message (using a NEW FILE and erase older one)
    // Don't worry all the settings have been retreived in "msg" (using above Unflatten) so other settings
    // like window position and size are still retained!
    BFile writeFile(path.String(), B_ERASE_FILE | B_CREATE_FILE | B_READ_WRITE);
    msg.Flatten(&writeFile);
}


void PrefsViewInterface::Load()
{
    m_fullLengthBarsChk->SetValue(_prefs_interface.FindBoolDef(kPfFullLengthBars, false));

    m_colorPopUp->ItemAt(_prefs_interface.FindInt16Def(kPfColorIndex, 0))->SetMarked(true);
    m_actFore = _prefs_interface.FindColorDef(kPfActFore, K_ACTIVE_FORE_COLOR);
    m_actBack = _prefs_interface.FindColorDef(kPfActBack, K_ACTIVE_SELECT_COLOR);
    UpdateColorControl(m_colorPopUp->FindMarked());

    // Load MainWindow settings file to retrieve the appropriate settings (semi-hack!)
    BMessage msg;
    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;

    BFile file(path.String(), B_READ_ONLY);

    // restore defaults or load from file
    msg.Unflatten(&file);         // may fail (if settings file is missing)

    m_toolbarChk->SetValue(FindBoolDef(&msg, kToolBar, true));
    m_infobarChk->SetValue(FindBoolDef(&msg, kInfoBar, true));
    m_actionLogChk->SetValue(FindBoolDef(&msg, kSplitter, true));

    // Restore folding level if present or else the default one
    int8 v;
    status_t result = msg.FindInt8(kFolding, &v);
    if (result == B_OK)
        m_foldingPopUp->ItemAt(v)->SetMarked(true);
    else
        m_foldingPopUp->ItemAt(3)->SetMarked(true);
}


void PrefsViewInterface::AttachedToWindow()
{
    m_colorControl->SetTarget(this);
    m_colorPopUp->SetTargetForItems(this);
    return PrefsView::AttachedToWindow();
}


void PrefsViewInterface::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_COLOR_CHANGE:
        {
            BMenuItem* item = m_colorPopUp->FindMarked();
            if (!item)
                break;

            BString const itemText = item->Label();
            if (itemText == B_TRANSLATE("Selected text color"))
                m_actFore = m_colorControl->ValueAsColor();
            else if (itemText == B_TRANSLATE("Selected background color"))
                m_actBack = m_colorControl->ValueAsColor();

            UpdateColorWell();
            break;
        }

        case M_ITEM_CHANGE:
        {
            BMenuItem* item = NULL;
            message->FindPointer("source", (void**)&item);
            if (!item)
                break;
            UpdateColorControl(item);
            break;
        }
    }

    return PrefsView::MessageReceived(message);
}


void PrefsViewInterface::UpdateColorWell()
{
    m_colorWell->SetViewColor(m_colorControl->ValueAsColor());
    m_colorWell->Invalidate();
}


void PrefsViewInterface::UpdateColorControl(BMenuItem* item)
{
    BString itemText = item->Label();
    if (itemText == B_TRANSLATE("Selected text color"))
        m_colorControl->SetValue(m_actFore);
    else if (itemText == B_TRANSLATE("Selected background color"))
        m_colorControl->SetValue(m_actBack);

    UpdateColorWell();
}


bool PrefsViewInterface::FindBoolDef(BMessage* msg, const char* name, bool defaultValue)
{
    // Because we are loading from ANOTHER settings file (via a BMessage and not Preferences class)!!
    // This supports the semi-hack we do in Load and Save in this View
    bool v;
    status_t result = msg->FindBool(name, &v);
    if (result == B_OK)
        return v;
    else
        return defaultValue;
}
