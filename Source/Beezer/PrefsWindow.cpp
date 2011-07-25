/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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
#include <Screen.h>
#include <ListView.h>
#include <TextView.h>
#include <ScrollView.h>
#include <String.h>
#include <Button.h>

#include "PrefsWindow.h"
#include "PrefsListItem.h"
#include "LangStrings.h"
#include "BevelView.h"
#include "UIConstants.h"
#include "MsgConstants.h"
#include "PrefsViewExtract.h"
#include "PrefsViewState.h"
#include "PrefsViewWindows.h"
#include "PrefsViewPaths.h"
#include "PrefsViewRecent.h"
#include "PrefsViewAdd.h"
#include "PrefsViewInterface.h"
#include "PrefsViewMisc.h"
#include "Preferences.h"
#include "BitmapPool.h"

//=============================================================================================================//

PrefsWindow::PrefsWindow ()
    : BWindow (BRect (0, 0, 570+30, 320+100), str (S_PREFERENCES_TITLE), B_TITLED_WINDOW,
           B_NOT_ZOOMABLE | B_NOT_RESIZABLE, B_CURRENT_WORKSPACE),
    m_currentPanel (NULL)
{
    SetFeel (B_MODAL_APP_WINDOW_FEEL);

    AddControls ();
    m_panelList.AddItem ((void*)new PrefsViewExtract (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewAdd (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewState (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewWindows (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewPaths (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewRecent (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewInterface (m_panelFrame));
    m_panelList.AddItem ((void*)new PrefsViewMisc (m_panelFrame));
    
    for (int32 i = 0; i < m_panelList.CountItems(); i++)
    {
        PrefsView *prefPanel = (PrefsView*)m_panelList.ItemAtFast(i);
        prefPanel->Load();
        m_backView->AddChild (prefPanel);
        prefPanel->Hide();
        PrefsListItem *listItem = new PrefsListItem (prefPanel->Title(), prefPanel->Bitmap());
        m_listView->AddItem (listItem);
    
        if (prefPanel->Bitmap())
        {
           listItem->SetHeight
                  (MAX (prefPanel->Bitmap()->Bounds().Height() + 6, listItem->FontHeight() * 2 + 3));
        }
        else
           listItem->SetHeight (listItem->Height() + 6);
    }
    
    // Critical order
    m_listView->SetSelectionMessage (new BMessage (M_PREFS_PANEL_SELECTED));
    m_listView->SetTarget (this);
    if (m_panelList.CountItems() >= 0)
    {
        m_currentPanel = (PrefsView*)m_panelList.ItemAtFast(0);
        m_currentPanel->Show();
        SetActivePanel (m_currentPanel);
        m_listView->Select (_prefs_misc.FindInt8Def (kPfPrefPanelIndex, 0), false);
        m_listView->ScrollToSelection();
    }
    
    // Center window on-screen & set the constraints
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);

    // Restore from prefs
    BRect frame;
    if (_prefs_windows.FindBoolDef (kPfPrefsWnd, true))
        if (_prefs_windows.FindRect (kPfPrefsWndFrame, &frame) == B_OK)
           MoveTo (frame.LeftTop());
    
    Show();
}

//=============================================================================================================//

void PrefsWindow::Quit()
{
    _prefs_misc.SetInt8 (kPfPrefPanelIndex, m_listView->CurrentSelection (0));

    if (_prefs_windows.FindBoolDef (kPfPrefsWnd, true))
        _prefs_windows.SetRect (kPfPrefsWndFrame, Frame());
    
    be_app_messenger.SendMessage (M_CLOSE_PREFS);
    return BWindow::Quit();
}

//=============================================================================================================//

void PrefsWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_PREFS_PANEL_SELECTED:
        {
           int32 selectedItem = m_listView->CurrentSelection();
           if (selectedItem >= 0L && selectedItem < m_panelList.CountItems())
           {
               PrefsView *selectedPanel = (PrefsView*)m_panelList.ItemAtFast (selectedItem);
               if (m_currentPanel != selectedPanel)
                  SetActivePanel (selectedPanel);
           }
           else if (m_currentPanel != NULL)
           {
               // Restore selection as user has deselected
               m_listView->Select (m_panelList.IndexOf ((void*)m_currentPanel));
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
           be_app_messenger.SendMessage (message);
           break;
        }
        
        case M_REVERT:
        {
           m_currentPanel->Load();
           break;
        }
        
        default:
           return BWindow::MessageReceived (message);
    }
}

//=============================================================================================================//

void PrefsWindow::SetActivePanel (PrefsView *activePanel)
{
    m_currentPanel->Hide();

    m_currentPanel = activePanel;
    BString descText = m_currentPanel->Title();
    descText << '\n' << m_currentPanel->Description();
    if (strcmp (m_descTextView->Text(), descText.String()) != 0)
    {
        int32 tlen = strlen (m_currentPanel->Title());
        int32 dlen = strlen (m_currentPanel->Description());
        m_descTextView->SetText (descText.String());
        m_descTextView->SetFontAndColor (0, tlen, be_bold_font, B_FONT_ALL,    &(K_DEEP_RED_COLOR));
        m_descTextView->SetFontAndColor (tlen, tlen+dlen+1, be_plain_font, B_FONT_ALL,
                                &(K_BLACK_COLOR));
    }
    
    m_currentPanel->Show();
}

//=============================================================================================================//

void PrefsWindow::AddControls ()
{
    m_backView = new BevelView (Bounds(), "PrefsWindow:backView", btOutset, B_FOLLOW_ALL_SIDES);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);
    
    float margin = K_MARGIN + 2;
    float maxWidth = 110+20;
    font_height fntHt, boldFntHt;
    be_plain_font->GetHeight (&fntHt);
    be_bold_font->GetHeight (&boldFntHt);
    float totalHeight = fntHt.ascent + fntHt.descent + fntHt.leading + boldFntHt.ascent +
               boldFntHt.descent + boldFntHt.leading + 8;
    
    m_listView = new BListView (BRect (margin, margin, maxWidth,
                  Bounds().bottom - margin), "PrefsWindow:listView",
                  B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    
    BScrollView *scrollView = new BScrollView ("PrefsWindow:scrollView", m_listView, B_FOLLOW_LEFT,
                             B_WILL_DRAW, false, true, B_FANCY_BORDER);
    m_backView->AddChild (scrollView);
    m_listView->TargetedByScrollView (scrollView);
    
    BevelView *descViewDecor = new BevelView (BRect (scrollView->Frame().right + margin, margin,
                         Bounds().right - margin, margin + totalHeight + btDeepThickness),
                         "PrefsWindow:descViewDecor", btDeep, B_FOLLOW_LEFT);
    m_backView->AddChild (descViewDecor);
    
    float border = descViewDecor->EdgeThickness();
    m_descTextView = new BTextView (BRect (border, border, descViewDecor->Frame().Width() - border,
                      descViewDecor->Frame().Height() - border), "PrefsWindow:descTextView", 
                      BRect (2, 2, descViewDecor->Frame().Width() - 2 * border - 4, 0), B_FOLLOW_LEFT,
                      B_WILL_DRAW);
    m_descTextView->SetViewColor (255, 252, 232, 255);
    m_descTextView->SetStylable (true);
    m_descTextView->MakeEditable (false);
    m_descTextView->MakeSelectable (false);
    descViewDecor->AddChild (m_descTextView);
    
    m_panelFrame.Set (scrollView->Frame().right + margin, descViewDecor->Frame().bottom + margin,
                      Bounds().right - margin, scrollView->Frame().bottom - K_BUTTON_HEIGHT - margin);

    BButton *discardBtn = new BButton (BRect (scrollView->Frame().right + margin,
                             Bounds().bottom - K_BUTTON_HEIGHT - margin,
                             scrollView->Frame().right + margin + K_BUTTON_WIDTH, Bounds().bottom - margin),
                             "PrefsWindow:discardBtn", str (S_PREFS_DISCARD), new BMessage (B_QUIT_REQUESTED),
                             B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    
    BButton *saveBtn = new BButton (BRect (discardBtn->Frame().right + margin, discardBtn->Frame().top,
                             discardBtn->Frame().right + margin + K_BUTTON_WIDTH, discardBtn->Frame().bottom),
                             "PrefsWindow:saveBtn", str (S_PREFS_SAVE), new BMessage (M_SAVE_PREFS),
                             B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    BButton *helpBtn = new BButton (BRect (Bounds().right - margin - K_BUTTON_WIDTH, discardBtn->Frame().top,
                             Bounds().right - margin, discardBtn->Frame().bottom), "PrefsWindow:helpBtn",
                             str (S_HELP), new BMessage (M_PREFS_HELP), B_FOLLOW_LEFT,
                             B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild (saveBtn);
    m_backView->AddChild (discardBtn);
    m_backView->AddChild (helpBtn);
}

//=============================================================================================================//
