// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsWindow.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsListItem.h"
#include "PrefsViewAdd.h"
#include "PrefsViewExtract.h"
#include "PrefsViewInterface.h"
#include "PrefsViewMisc.h"
#include "PrefsViewPaths.h"
#include "PrefsViewRecent.h"
#include "PrefsViewState.h"
#include "PrefsViewWindows.h"
#include "UIConstants.h"

#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <ListView.h>
#include <ScrollView.h>

#include <cassert>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsWindow"
#else
#define B_TRANSLATE(x) x
#endif

static const uint32 M_SAVE_PREFS           = 'svpf';
static const uint32 M_PREFS_PANEL_SELECTED = 'pspf';


PrefsWindow::PrefsWindow()
    : BWindow(BRect(0, 0, 620, 440), BZ_TR(kSettingsString), B_TITLED_WINDOW,
              B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
    m_currentPanel(NULL)
{
    SetFeel(B_MODAL_APP_WINDOW_FEEL);

    m_panelTitleFont = new BFont(be_plain_font);
    m_panelTitleFont->SetFace(B_BOLD_FACE);
    m_panelTitleFont->SetSize(be_plain_font->Size() + 1);

    // Add all controls ((but the panels) to the UI.
    BRect panelFrame;
    AddControls(&panelFrame);

    m_panelList.AddItem((void*)new PrefsViewExtract(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewAdd(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewState(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewWindows(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewPaths(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewRecent(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewInterface(panelFrame));
    m_panelList.AddItem((void*)new PrefsViewMisc(panelFrame));

    // Load all panels.
    for (int32 i = 0; i < m_panelList.CountItems(); i++)
    {
        PrefsView* prefPanel = (PrefsView*)m_panelList.ItemAtFast(i);
        prefPanel->Load();
    }

    // Add panels to the UI.
    AddPanels();

    // Pick which panel to display and validate if settings are sane.
    int8 panelIndex = _prefs_misc.FindInt8Def(kPfPrefPanelIndex, 0);
    if (panelIndex < 0 || panelIndex >= m_panelList.CountItems())
        panelIndex = 0;

    // Set a panel (or restore last selected or 0 if none) as the active panel (critical order of operations below)
    m_listView->SetSelectionMessage(new BMessage(M_PREFS_PANEL_SELECTED));
    m_listView->SetTarget(this);
    m_listView->Select(panelIndex, false);
    m_listView->ScrollToSelection();

    CenterOnScreen();

    // Restore window position from prefs
    BRect frame;
    if (_prefs_windows.FindBoolDef(kPfPrefsWnd, true))
        if (_prefs_windows.FindRect(kPfPrefsWndFrame, &frame) == B_OK)
        {
            // TODO: If rect is off-screen don't bother moving it (leave it centered)
            MoveTo(frame.LeftTop());
        }

    Show();
}


PrefsWindow::~PrefsWindow()
{
    delete m_panelTitleFont;
}

void PrefsWindow::Quit()
{
    _prefs_misc.SetInt8(kPfPrefPanelIndex, m_listView->CurrentSelection(0));

    if (_prefs_windows.FindBoolDef(kPfPrefsWnd, true))
        _prefs_windows.SetRect(kPfPrefsWndFrame, Frame());

    be_app_messenger.SendMessage(M_CLOSE_PREFS);
    return BWindow::Quit();
}


void PrefsWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_PREFS_PANEL_SELECTED:
        {
            int32 const selectedItem = m_listView->CurrentSelection();
            if (selectedItem >= 0L && selectedItem < m_panelList.CountItems())
            {
                PrefsView* selectedPanel = (PrefsView*)(m_panelList.ItemAtFast(selectedItem));
                SetActivePanel(selectedPanel);
            }
            else if (m_currentPanel != NULL)
            {
                // Restore selection as user has deselected
                m_listView->Select(m_panelList.IndexOf((void*)m_currentPanel));
                m_listView->ScrollToSelection();
            }
            break;
        }

        case M_SAVE_PREFS:
        {
            for (int32 i = 0; i < m_panelList.CountItems(); i++)
                ((PrefsView*)m_panelList.ItemAtFast(i))->Save();

            // It's okay to call Quit from message loop - BeBook
            Quit();
            break;
        }

        case M_PREFS_HELP:
        {
            be_app_messenger.SendMessage(message);
            break;
        }

        case M_REVERT:
        {
            m_currentPanel->Load();
            break;
        }

        default:
            return BWindow::MessageReceived(message);
    }
}


void PrefsWindow::SetActivePanel(PrefsView* activePanel)
{
    assert(activePanel);

    if (m_currentPanel)
        m_currentPanel->Hide();
    m_currentPanel = activePanel;
    m_currentPanel->Show();

    BString descText = m_currentPanel->Title();
    descText << '\n' << m_currentPanel->Description();
    if (strcmp(descText.String(), m_descTextView->Text()) != 0)
    {
        rgb_color textColor = ui_color(B_TOOL_TIP_TEXT_COLOR);
        int32 const titleLen = strlen(m_currentPanel->Title());
        int32 const descLen  = strlen(m_currentPanel->Description());
        m_descTextView->SetText(descText.String());
        m_descTextView->SetFontAndColor(0, titleLen, m_panelTitleFont, B_FONT_ALL, &(K_DEEP_RED_COLOR));
        m_descTextView->SetFontAndColor(titleLen, titleLen + descLen + 1, be_plain_font, B_FONT_ALL, &textColor);
    }
}


void PrefsWindow::AddControls(BRect *panelFrame)
{
    m_backView = new BevelView(Bounds(), "PrefsWindow:backView", BevelView::bevel_type::OUTSET,
                               B_FOLLOW_ALL_SIDES);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    AddChild(m_backView);

    float const margin = K_MARGIN + 2;
    float const maxWidth = 140;
    font_height fntHt, boldFntHt;
    be_plain_font->GetHeight(&fntHt);
    m_panelTitleFont->GetHeight(&boldFntHt);
    float descTextHeight = boldFntHt.ascent + boldFntHt.descent + boldFntHt.leading  // panel title height
                         + fntHt.ascent + fntHt.descent + fntHt.leading              // panel description height
                         + 10;                                                       // extra interline space

    m_listView = new BListView(BRect(margin, margin, maxWidth,
                                     Bounds().bottom - margin), "PrefsWindow:listView",
                               B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    BScrollView* scrollView = new BScrollView("PrefsWindow:scrollView", m_listView, B_FOLLOW_LEFT,
                                              B_WILL_DRAW, false, true, B_FANCY_BORDER);
    m_backView->AddChild(scrollView);
    m_listView->TargetedByScrollView(scrollView);

    rgb_color backColor = tint_color(ui_color(B_TOOL_TIP_BACKGROUND_COLOR), B_LIGHTEN_1_TINT);
    BevelView* descViewDecor = new BevelView(BRect(scrollView->Frame().right + margin, margin,
                                                   Bounds().right - margin, margin + descTextHeight + BevelView::kDeepThickness),
                                             "PrefsWindow:descViewDecor", BevelView::bevel_type::DEEP, B_FOLLOW_LEFT);
    m_backView->AddChild(descViewDecor);
    descViewDecor->SetViewColor(backColor);

    float const border = descViewDecor->EdgeThickness();
    m_descTextView = new BTextView(BRect(border + K_MARGIN, border, descViewDecor->Frame().Width() - border,
                                         descViewDecor->Frame().Height() - border), "PrefsWindow:descTextView",
                                   BRect(2, 2, descViewDecor->Frame().Width() - 2 * border - 4, 0), B_FOLLOW_LEFT,
                                   B_WILL_DRAW);
    m_descTextView->SetViewColor(backColor);
    m_descTextView->SetStylable(true);
    m_descTextView->MakeEditable(false);
    m_descTextView->MakeSelectable(false);
    descViewDecor->AddChild(m_descTextView);

    BButton* discardBtn = new BButton(BRect(scrollView->Frame().right + margin,
                                            Bounds().bottom - K_BUTTON_HEIGHT - margin,
                                            scrollView->Frame().right + margin + K_BUTTON_WIDTH, Bounds().bottom - margin),
                                      "PrefsWindow:discardBtn", B_TRANSLATE("Discard"), new BMessage(B_QUIT_REQUESTED),
                                      B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    BButton* saveBtn = new BButton(BRect(discardBtn->Frame().right + margin, discardBtn->Frame().top,
                                         discardBtn->Frame().right + margin + K_BUTTON_WIDTH, discardBtn->Frame().bottom),
                                   "PrefsWindow:saveBtn", BZ_TR(kSaveString), new BMessage(M_SAVE_PREFS),
                                   B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    BButton* helpBtn = new BButton(BRect(Bounds().right - margin - K_BUTTON_WIDTH, discardBtn->Frame().top,
                                         Bounds().right - margin, discardBtn->Frame().bottom), "PrefsWindow:helpBtn",
                                   BZ_TR(kHelpString), new BMessage(M_PREFS_HELP), B_FOLLOW_LEFT,
                                   B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(saveBtn);
    m_backView->AddChild(discardBtn);
    m_backView->AddChild(helpBtn);

    assert(panelFrame);
    panelFrame->Set(scrollView->Frame().right + margin, descViewDecor->Frame().bottom + margin,
                    Bounds().right - margin, scrollView->Frame().bottom - K_BUTTON_HEIGHT - margin);
}


void PrefsWindow::AddPanels()
{
    assert(m_panelList.CountItems() > 0);

    // Add all panels as children (but keep them hidden)
    // Add also corresponding list-items of the panels
    for (int32 i = 0; i < m_panelList.CountItems(); i++)
    {
        PrefsView* prefPanel = (PrefsView*)m_panelList.ItemAtFast(i);
        m_backView->AddChild(prefPanel);
        prefPanel->Hide();

        PrefsListItem* listItem = new PrefsListItem(prefPanel->Title(), prefPanel->Bitmap());
        m_listView->AddItem(listItem);

        if (prefPanel->Bitmap())
            listItem->SetHeight(MAX(prefPanel->Bitmap()->Bounds().Height() + 6, listItem->FontHeight() * 2 + 3));
        else
            listItem->SetHeight(listItem->Height() + 6);
    }
}
