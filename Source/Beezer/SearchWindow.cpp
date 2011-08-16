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

#include <List.h>
#include <Screen.h>
#include <String.h>
#include <Bitmap.h>
#include <StringView.h>
#include <Entry.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <Box.h>
#include <RadioButton.h>
#include <CheckBox.h>
#include <Button.h>

#include "CLVColumn.h"
#include "RegExString.h"
#include "BubbleHelper.h"

#include "SearchWindow.h"
#include "BevelView.h"
#include "UIConstants.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "Archiver.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"

//=============================================================================================================//

SearchWindow::SearchWindow (BWindow *callerWindow, BMessage *loadMessage, BubbleHelper *bubbleHelper,
               const BEntry *entry, const BList *columnList, const Archiver *ark)
    : BWindow (BRect (30, 30, 440, 312), str (S_SEARCH_WINDOW_TITLE), B_TITLED_WINDOW,
        B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE, B_CURRENT_WORKSPACE),
           m_callerWindow (callerWindow),
           m_loadMessage (loadMessage),
           m_helper (bubbleHelper)
{
    SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
    AddToSubset (callerWindow);

    // Get previous settings (assign default values - for the first time)
    const char *searchText = NULL;
    int32 exprType = 2L;
    CLVColumn *column = NULL;
    bool ignoreCase = true;
    bool invertSearch = false;
    bool addToSelection = false;
    bool searchSelection = false;
    bool allFiles = false;
    bool persistent = false;
    BRect windowrect;
    windowrect.left = -1;

    if (m_loadMessage)
    {
        m_loadMessage->FindString (kExpr, &searchText);
        m_loadMessage->FindInt32 (kExprType, &exprType);
        m_loadMessage->FindPointer (kColumnPtr, reinterpret_cast<void**>(&column));
        m_loadMessage->FindBool (kIgnoreCase, &ignoreCase);
        m_loadMessage->FindBool (kInvertSelection, &invertSearch);
        m_loadMessage->FindBool (kAddToSelection, &addToSelection);
        m_loadMessage->FindBool (kSearchSelection, &searchSelection);
        m_loadMessage->FindBool (kAllFiles, &allFiles);
        m_loadMessage->FindBool (kPersistent, &persistent);
        m_loadMessage->FindRect (kWindowRect, &windowrect);
    }

    // Get font metrics
    BFont font (be_plain_font);
    font_height fntHt;

    font.GetHeight (&fntHt);
    float normFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
    
    font.SetFace (B_BOLD_FACE);
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    // Render controls
    m_backView = new BevelView (Bounds(), "SearchWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);
           
    BBitmap *searchBmp = ResBitmap ("Img:SearchArchive");

    BevelView *sepView1 = new BevelView (BRect (-1, searchBmp->Bounds().Height() + 4 * K_MARGIN,
                                Bounds().right - 1.0, searchBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                "SearchWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView1);
    
    // Add icon view, make it hold the search picture
    StaticBitmapView *searchBmpView = new StaticBitmapView (BRect (K_MARGIN * 5, K_MARGIN * 2,
                             searchBmp->Bounds().Width() + K_MARGIN * 5,
                             searchBmp->Bounds().Height() + K_MARGIN * 2), "SearchWindow:searchBmpView",
                             searchBmp);
    searchBmpView->SetViewColor (m_backView->ViewColor());
    AddChild (searchBmpView);
    
    // Add the file name string view (align it vertically with the icon view)
    char buf[B_FILE_NAME_LENGTH];
    entry->GetName (buf);
    BStringView *fileNameStr = new BStringView (BRect (searchBmpView->Frame().right + K_MARGIN * 3,
                                    searchBmpView->Frame().top, Bounds().right - 1,
                                    searchBmpView->Frame().top + totalFontHeight),
                                    "SearchWindow:FileNameView", buf, B_FOLLOW_LEFT, B_WILL_DRAW);
    fileNameStr->SetFont (&font);
    m_backView->AddChild (fileNameStr);
    fileNameStr->MoveTo (fileNameStr->Frame().left,
        (searchBmpView->Frame().Height() / 2 - totalFontHeight / 2) + totalFontHeight / 2 + 1);
    fileNameStr->ResizeToPreferred ();

    float marginLeft = 4 * K_MARGIN;
    float marginTop = 2 * K_MARGIN;
    float vGap = 6;

    // Get the available list of columns - no point in searching columns that has no information
    // Eg: Tar archives won't have the columns: compressed size, ratio etc.
    BList hiddenColumnList = ark->HiddenColumns (const_cast<BList*>(columnList));
    m_tmpList = *columnList;
    int32 hiddenCount = hiddenColumnList.CountItems();
    for (int32 i = 0L; i < hiddenCount; i++)
        m_tmpList.RemoveItem (hiddenColumnList.ItemAt(i));

    // Setup the column names and the column menu
    BMenu *columnMenu = new BPopUpMenu ("");
    for (int32 i = 0; i < m_tmpList.CountItems(); i++)
        columnMenu->AddItem (new BMenuItem (((CLVColumn*)m_tmpList.ItemAtFast(i))->GetLabel(), NULL));

    columnMenu->SetLabelFromMarked (true);
    if (!column)
        columnMenu->ItemAt(0L)->SetMarked (true);
    else
        columnMenu->ItemAt(m_tmpList.IndexOf (column))->SetMarked (true);
    
    m_columnField = new BMenuField (BRect (marginLeft, sepView1->Frame().bottom + marginTop - 1,
                         Bounds().Width() - marginLeft, 0), "SearchWindow:ColumnField",
                         str (S_SEARCH_COLUMN), columnMenu);
     m_backView->AddChild (m_columnField);
    m_columnField->SetDivider (m_backView->StringWidth (m_columnField->Label()) + 10.0);
    m_columnField->ResizeToPreferred();

    // Setup the match type and the match type menu    
    BMenu *matchMenu = new BPopUpMenu ("");
    matchMenu->AddItem (new BMenuItem (str (S_SEARCH_STARTS_WITH), NULL));
    matchMenu->AddItem (new BMenuItem (str (S_SEARCH_ENDS_WITH), NULL));
    matchMenu->AddItem (new BMenuItem (str (S_SEARCH_CONTAINS), NULL));
    matchMenu->AddItem (new BMenuItem (str (S_SEARCH_WILDCARD_EXPRESSION), NULL));
    matchMenu->AddItem (new BMenuItem (str (S_SEARCH_REGULAR_EXPRESSION), NULL));

    matchMenu->SetLabelFromMarked (true);
    matchMenu->ItemAt(exprType)->SetMarked (true);

    m_matchField = new BMenuField (BRect (
                         m_backView->StringWidth (columnMenu->ItemAt(0L)->Label()) + 
                         m_backView->StringWidth (m_columnField->Label()) + 50 + 2 * marginLeft,
                         sepView1->Frame().bottom + marginTop - 1, Bounds().Width() - marginLeft, 0),
                         "SearchWindow:MatchField", NULL, matchMenu);
    m_backView->AddChild (m_matchField);
    m_matchField->SetDivider (m_backView->StringWidth (m_matchField->Label()) + 10.0);
    m_matchField->ResizeToPreferred();
    
    // Setup the 'search for' text control
    // removed label - str (S_SEARCH_FOR), add it when needed as people thot "Find:" was not needed
    m_searchTextControl = new BTextControl (BRect (marginLeft, m_matchField->Frame().bottom + vGap,
                             Bounds().right - marginLeft - 2, 0), "SearchWindow:SearchTextControl",
                             "", searchText, NULL, B_FOLLOW_LEFT_RIGHT);
    m_backView->AddChild (m_searchTextControl);
    m_searchTextControl->SetDivider (m_backView->StringWidth (m_searchTextControl->Label()));
    m_searchTextControl->MakeFocus (true);
    m_searchTextControl->SetModificationMessage (new BMessage (M_SEARCH_TEXT_MODIFIED));
    BTextView *textView = m_searchTextControl->TextView();
    
    // Disallow the below meta keys as they aren't valid search text characters
    textView->DisallowChar (B_ESCAPE);
    textView->DisallowChar (B_INSERT);
    textView->DisallowChar (B_TAB);
    textView->DisallowChar (B_FUNCTION_KEY);
    textView->DisallowChar (B_DELETE);
    textView->DisallowChar (B_PAGE_UP);
    textView->DisallowChar (B_PAGE_DOWN);
    textView->DisallowChar (B_HOME);
    textView->DisallowChar (B_END);

    // Setup the scope group box
    BBox *scopeBox = new BBox (BRect (marginLeft, m_searchTextControl->Frame().bottom + 2 * vGap, 0, 0),
                         "SearchWindow:ScopeBox", B_FOLLOW_LEFT, B_WILL_DRAW);
    scopeBox->SetLabel (str (S_SEARCH_SCOPE));
    scopeBox->SetFont (be_plain_font);
    m_backView->AddChild (scopeBox);
    
    // Draw the radio buttons inside the group box (co-ordinates are relative to the group box)
    m_allEntriesOpt = new BRadioButton (BRect (marginLeft/2, normFontHeight + vGap/2,
                         marginLeft/2 + m_backView->StringWidth (str (S_SEARCH_ALL_ENTRIES)) +
                         25.0, 0), "SearchWindow:AllEntriesOpt", str (S_SEARCH_ALL_ENTRIES),
                         new BMessage (M_ALL_ENTRIES));
    scopeBox->AddChild (m_allEntriesOpt);
    m_allEntriesOpt->ResizeToPreferred();
    m_allEntriesOpt->SetValue (allFiles == true ? B_CONTROL_ON : B_CONTROL_OFF);
    if (allFiles)
        m_allEntriesOpt->Invoke ();

    m_visibleEntriesOpt = new BRadioButton (BRect (m_allEntriesOpt->Frame().left, m_allEntriesOpt->Frame().bottom,
                             m_allEntriesOpt->Frame().left +
                             m_backView->StringWidth (str (S_SEARCH_VISIBLE_ENTRIES)) + 25.0, 0),
                             "SearchWindow:VisibleEntriesOpt", str (S_SEARCH_VISIBLE_ENTRIES),
                             new BMessage (M_VISIBLE_ENTRIES));
    scopeBox->AddChild (m_visibleEntriesOpt);
    m_visibleEntriesOpt->ResizeToPreferred();
    m_visibleEntriesOpt->SetValue (allFiles == false ? B_CONTROL_ON : B_CONTROL_OFF);
    if (!allFiles)
        m_visibleEntriesOpt->Invoke ();

    m_selEntriesOpt = new BRadioButton (BRect (m_visibleEntriesOpt->Frame().left, m_visibleEntriesOpt->Frame().bottom,
                         m_visibleEntriesOpt->Frame().left +
                         m_backView->StringWidth (str (S_SEARCH_SELECTED_ENTRIES)) + 25.0, 0),
                         "SearchWindow:SelectedEntriesOpt", str (S_SEARCH_SELECTED_ENTRIES),
                         new BMessage (M_SELECTED_ENTRIES));
    scopeBox->AddChild (m_selEntriesOpt);
    m_selEntriesOpt->ResizeToPreferred();
    m_selEntriesOpt->SetValue (searchSelection == true ? B_CONTROL_ON : B_CONTROL_OFF);
    if (searchSelection)
        m_selEntriesOpt->Invoke ();
        
    // Dynamically size the scope box (auto-fit)
    float maxWidth = m_selEntriesOpt->Frame().Width();
    maxWidth = MAX (maxWidth, m_visibleEntriesOpt->Frame().Width());
    maxWidth = MAX (maxWidth, m_allEntriesOpt->Frame().Width());
    
    scopeBox->ResizeTo (maxWidth + marginLeft + 2, m_selEntriesOpt->Frame().bottom + vGap);
    
    // Setup the scoping options group box
    BBox *optionsBox = new BBox (BRect (scopeBox->Frame().right + marginLeft, scopeBox->Frame().top, 0, 0),
                         "SearchWindow:OptionsBox");
    optionsBox->SetLabel (str (S_SEARCH_OPTIONS));
    optionsBox->SetFont (be_plain_font);
    m_backView->AddChild (optionsBox);
    
    // Draw the checkboxes for the (All, Visible) scope
    m_addToSelChk = new BCheckBox (BRect (marginLeft / 2, normFontHeight -1 + vGap / 2, 0, 0),
                      "SearchWindow:AddSelChk", str (S_SEARCH_ADD_TO_SELECTION), NULL);
    optionsBox->AddChild (m_addToSelChk);
    m_addToSelChk->ResizeToPreferred();
    m_addToSelChk->SetValue (addToSelection == true ? B_CONTROL_ON : B_CONTROL_OFF);
    
    m_ignoreCaseChk = new BCheckBox (BRect (m_addToSelChk->Frame().left, m_addToSelChk->Frame().bottom-1,
                                0, 0), "SearchWindow:IgnoreCaseChk", str (S_SEARCH_IGNORE_CASE), NULL);
    optionsBox->AddChild (m_ignoreCaseChk);
    m_ignoreCaseChk->ResizeToPreferred();
    m_ignoreCaseChk->SetValue (ignoreCase == true ? B_CONTROL_ON : B_CONTROL_OFF);
    
    m_invertChk = new BCheckBox (BRect (m_ignoreCaseChk->Frame().left, m_ignoreCaseChk->Frame().bottom-1,
                                0, 0), "SearchWindow:InvertChk", str (S_SEARCH_INVERT), NULL);
    optionsBox->AddChild (m_invertChk);
    m_invertChk->ResizeToPreferred();
    m_invertChk->SetValue (invertSearch == true ? B_CONTROL_ON : B_CONTROL_OFF);
    
    // Dynamically size the group box (auto-fit)
    maxWidth = m_addToSelChk->Frame().Width();
    maxWidth = MAX (maxWidth, m_ignoreCaseChk->Frame().Width());
    maxWidth = MAX (maxWidth, m_invertChk->Frame().Width());
    maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_SEARCH_DESELECT_UNMATCHED_ENTRIES)) + 21.0);
    
    optionsBox->ResizeTo (maxWidth + marginLeft + 2, m_invertChk->Frame().bottom + vGap);
    
    // Render the search button
    m_searchBtn = new BButton (BRect (Bounds().right - marginLeft - K_BUTTON_WIDTH - 4,
                             optionsBox->Frame().bottom + 2 * vGap, Bounds().right - marginLeft - 4,
                             optionsBox->Frame().bottom + 2 * vGap + K_BUTTON_HEIGHT),
                             "SearchWindow:SearchBtn", str (S_SEARCH), new BMessage (M_SEARCH_CLICKED),
                             B_FOLLOW_RIGHT);
    m_searchBtn->MakeDefault (true);
    m_searchBtn->SetEnabled (searchText ? true : false);
    m_backView->AddChild (m_searchBtn);
    
    // Render the close after search button
    m_persistentChk = new BCheckBox (BRect (marginLeft, m_searchBtn->Frame().top + normFontHeight / 2,
                      0, 0), "SearchWindow:CloseChk", str (S_SEARCH_PERSISTENT), NULL);
    m_backView->AddChild (m_persistentChk);
    m_persistentChk->ResizeToPreferred();
    m_persistentChk->SetValue (persistent == true ? B_CONTROL_ON : B_CONTROL_OFF);

        
    // Resize the window, then set contraints
    ResizeTo (optionsBox->Frame().right + marginLeft + 2, m_searchBtn->Frame().bottom + vGap + 2);

    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (Bounds().Width(), maxH, Bounds().Height(), maxV);
        
    // Center window on-screen
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    
    // Move according to the previous position (if any)
    if (windowrect.left > 0)
    {
        MoveTo (windowrect.LeftTop());
        ResizeTo (windowrect.Width(), windowrect.Height());
    }
    
    // Assign bubble helps
    SetBubbleHelps();
}

//=============================================================================================================//

void SearchWindow::Quit ()
{
    BMessage msg;
    GetSettings (msg, M_SEARCH_CLOSED);
    
    m_callerWindow->PostMessage (&msg);
    BWindow::Quit();
}

//=============================================================================================================//

void SearchWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_SEARCH_TEXT_MODIFIED:
        {
           m_searchBtn->SetEnabled (strlen (m_searchTextControl->Text()) > 0L ? true : false);
           break;
        }

        case M_ALL_ENTRIES: case M_VISIBLE_ENTRIES:
        {
           if (strcmp (m_addToSelChk->Label(), str (S_SEARCH_ADD_TO_SELECTION)) != 0)
           {
               m_helper->SetHelp (m_addToSelChk, const_cast<char*>(str (S_SEARCH_ADD_TO_SELECTION_BH)));
               m_addToSelChk->SetLabel (str (S_SEARCH_ADD_TO_SELECTION));
               m_addToSelChk->ResizeToPreferred();
           }

           break;
        }
        
        case M_SELECTED_ENTRIES:
        {
           if (strcmp (m_addToSelChk->Label(), str (S_SEARCH_DESELECT_UNMATCHED_ENTRIES)) != 0)
           {
               m_helper->SetHelp (m_addToSelChk,
                             const_cast<char*>(str (S_SEARCH_DESELECT_UNMATCHED_ENTRIES_BH)));
               m_addToSelChk->SetLabel (str (S_SEARCH_DESELECT_UNMATCHED_ENTRIES));
               m_addToSelChk->ResizeToPreferred();
           }
           
           break;
        }
        
        case M_SEARCH_CLICKED:
        {
           const char *searchText = m_searchTextControl->Text();
           if (!searchText || strlen (searchText) == 0L)
           {
               // Shouldn't really come here
               m_searchBtn->SetEnabled (false);
               break;
           }
           
           BMessage msg;
           GetSettings (msg, M_SEARCH);
           bool persistent = msg.FindBool (kPersistent);
           if (!persistent)
               Hide();
    
           m_callerWindow->PostMessage (&msg);
    
           if (!persistent)
               Quit();
           
           break;
        }
    }
           
    BWindow::MessageReceived (message);
}

//=============================================================================================================//

int32 SearchWindow::ExpressionType () const
{
    // Return the expression code from the menu field object m_matchField (assumed looper locked)
    BMenuItem *item = m_matchField->Menu()->FindMarked ();
    if (!item)
        return kNone;

    int32 index = m_matchField->Menu()->IndexOf (item);

    if (index < kStartsWith || index > kRegexpMatch)
        return kNone;
    
    RegExStringExpressionType typeArray[] =
    {    kStartsWith,
        kEndsWith,
        kContains,
        kGlobMatch,
        kRegexpMatch
    };
    
    return typeArray[index];
}

//=============================================================================================================//

CLVColumn* SearchWindow::Column () const
{
    // Return the pointer to the column that is selected in the m_columnField menu
    BMenuItem *item = m_columnField->Menu()->FindMarked ();
    if (!item)
        return NULL;

    int32 index = m_columnField->Menu()->IndexOf (item);

    return (CLVColumn*)m_tmpList.ItemAtFast (index);
}

//=============================================================================================================//

void SearchWindow::SetBubbleHelps ()
{
    m_helper->SetHelp (m_allEntriesOpt, const_cast<char*>(str (S_SEARCH_ALL_ENTRIES_BH)));
    m_helper->SetHelp (m_visibleEntriesOpt, const_cast<char*>(str (S_SEARCH_VISIBLE_ENTRIES_BH)));
    m_helper->SetHelp (m_selEntriesOpt, const_cast<char*>(str (S_SEARCH_SELECTED_ENTRIES_BH)));

    m_helper->SetHelp (m_addToSelChk, const_cast<char*>(str (S_SEARCH_ADD_TO_SELECTION_BH)));
    m_helper->SetHelp (m_ignoreCaseChk, const_cast<char*>(str (S_SEARCH_IGNORE_CASE_BH)));
    m_helper->SetHelp (m_invertChk, const_cast<char*>(str (S_SEARCH_INVERT_BH)));

    m_helper->SetHelp (m_searchTextControl, const_cast<char*>(str (S_SEARCH_FOR_BH)));
    m_helper->SetHelp (m_matchField, const_cast<char*>(str (S_SEARCH_MATCH_BH)));
    m_helper->SetHelp (m_columnField, const_cast<char*>(str (S_SEARCH_COLUMN_BH)));

    m_helper->SetHelp (m_persistentChk, const_cast<char*>(str (S_SEARCH_PERSISTENT_BH)));
}

//=============================================================================================================//

void SearchWindow::GetSettings (BMessage &msg, uint32 msgwhat) const
{
    msg.what = msgwhat;
    msg.AddInt32 (kExprType, ExpressionType());
    msg.AddString (kExpr, m_searchTextControl->Text());
    msg.AddPointer (kColumnPtr, Column());
    msg.AddBool (kIgnoreCase, m_ignoreCaseChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool (kInvertSelection, m_invertChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool (kAddToSelection, m_addToSelChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool (kSearchSelection, m_selEntriesOpt->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool (kAllFiles, m_allEntriesOpt->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool (kPersistent, m_persistentChk->Value() == B_CONTROL_OFF ? false : true);
    msg.AddRect (kWindowRect, Frame());
}

//=============================================================================================================//
