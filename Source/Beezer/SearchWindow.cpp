/*
 * Copyright (c) 2012, Ramshankar (aka Teknomancer)
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

#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Entry.h>
#include <GroupLayoutBuilder.h>
#include <List.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "CLVColumn.h"
#include "RegExString.h"

#include "Archiver.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "SearchWindow.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"



SearchWindow::SearchWindow(BWindow* callerWindow, BMessage* loadMessage,
                           const BEntry* entry, const BList* columnList, const Archiver* ark)
    : BWindow(BRect(30, 30, 440, 312), str(S_SEARCH_WINDOW_TITLE), B_FLOATING_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
    m_callerWindow(callerWindow),
    m_loadMessage(loadMessage)
{
    AddToSubset(callerWindow);

    // Get previous settings (assign default values - for the first time)
    const char* searchText = NULL;
    int32 exprType = 2L;
    CLVColumn* column = NULL;
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
        m_loadMessage->FindString(kExpr, &searchText);
        m_loadMessage->FindInt32(kExprType, &exprType);
        m_loadMessage->FindPointer(kColumnPtr, reinterpret_cast<void**>(&column));
        m_loadMessage->FindBool(kIgnoreCase, &ignoreCase);
        m_loadMessage->FindBool(kInvertSelection, &invertSearch);
        m_loadMessage->FindBool(kAddToSelection, &addToSelection);
        m_loadMessage->FindBool(kSearchSelection, &searchSelection);
        m_loadMessage->FindBool(kAllFiles, &allFiles);
        m_loadMessage->FindBool(kPersistent, &persistent);
        m_loadMessage->FindRect(kWindowRect, &windowrect);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap* searchBmp = BitmapPool::LoadAppVector("Img:SearchArchive", 32, 32);

    // Add icon view, make it hold the search picture
    StaticBitmapView* searchBmpView = new StaticBitmapView(BRect(0, 0, searchBmp->Bounds().Width(), searchBmp->Bounds().Height()),
            "SearchWindow:searchBmpView", searchBmp);

    // Add the file name string view (align it vertically with the icon view)
    char buf[B_FILE_NAME_LENGTH];
    entry->GetName(buf);
    BStringView* fileNameStr = new BStringView("SearchWindow:FileNameView", buf);
    fileNameStr->SetFont(be_bold_font);

    // Get the available list of columns - no point in searching columns that has no information
    // Eg: Tar archives won't have the columns: compressed size, ratio etc.
    BList hiddenColumnList = ark->HiddenColumns(const_cast<BList*>(columnList));
    m_tmpList = *columnList;
    int32 hiddenCount = hiddenColumnList.CountItems();
    for (int32 i = 0L; i < hiddenCount; i++)
        m_tmpList.RemoveItem(hiddenColumnList.ItemAt(i));

    // Setup the column names and the column menu
    BMenu* columnMenu = new BPopUpMenu("");
    for (int32 i = 0; i < m_tmpList.CountItems(); i++)
        columnMenu->AddItem(new BMenuItem(((CLVColumn*)m_tmpList.ItemAtFast(i))->GetLabel(), NULL));

    columnMenu->SetLabelFromMarked(true);
    if (!column)
        columnMenu->ItemAt(0L)->SetMarked(true);
    else
        columnMenu->ItemAt(m_tmpList.IndexOf(column))->SetMarked(true);

    m_columnField = new BMenuField("SearchWindow:ColumnField", str(S_SEARCH_COLUMN), columnMenu);

    // Setup the match type and the match type menu
    BMenu* matchMenu = new BPopUpMenu("");
    matchMenu->AddItem(new BMenuItem(str(S_SEARCH_STARTS_WITH), NULL));
    matchMenu->AddItem(new BMenuItem(str(S_SEARCH_ENDS_WITH), NULL));
    matchMenu->AddItem(new BMenuItem(str(S_SEARCH_CONTAINS), NULL));
    matchMenu->AddItem(new BMenuItem(str(S_SEARCH_WILDCARD_EXPRESSION), NULL));
    matchMenu->AddItem(new BMenuItem(str(S_SEARCH_REGULAR_EXPRESSION), NULL));

    matchMenu->SetLabelFromMarked(true);
    matchMenu->ItemAt(exprType)->SetMarked(true);

    m_matchField = new BMenuField("SearchWindow:MatchField", NULL, matchMenu);

    // Setup the 'search for' text control
    // removed label - str (S_SEARCH_FOR), add it when needed as people thot "Find:" was not needed
    m_searchTextControl = new BTextControl("SearchWindow:SearchTextControl", "", searchText, NULL);
    m_searchTextControl->SetModificationMessage(new BMessage(M_SEARCH_TEXT_MODIFIED));
    BTextView* textView = m_searchTextControl->TextView();

    // Disallow the below meta keys as they aren't valid search text characters
    textView->DisallowChar(B_ESCAPE);
    textView->DisallowChar(B_INSERT);
    textView->DisallowChar(B_TAB);
    textView->DisallowChar(B_FUNCTION_KEY);
    textView->DisallowChar(B_DELETE);
    textView->DisallowChar(B_PAGE_UP);
    textView->DisallowChar(B_PAGE_DOWN);
    textView->DisallowChar(B_HOME);
    textView->DisallowChar(B_END);

    // Setup the scope group box
    BBox* scopeBox = new BBox("SearchWindow:ScopeBox");
    scopeBox->SetLabel(str(S_SEARCH_SEARCH));
    scopeBox->SetFont(be_plain_font);

    // Draw the radio buttons inside the group box (co-ordinates are relative to the group box)
    m_allEntriesOpt = new BRadioButton("SearchWindow:AllEntriesOpt", str(S_SEARCH_ALL_ENTRIES), new BMessage(M_ALL_ENTRIES));
    m_allEntriesOpt->SetValue(allFiles == true ? B_CONTROL_ON : B_CONTROL_OFF);
    if (allFiles)
        m_allEntriesOpt->Invoke();

    m_visibleEntriesOpt = new BRadioButton("SearchWindow:VisibleEntriesOpt", str(S_SEARCH_VISIBLE_ENTRIES), new BMessage(M_VISIBLE_ENTRIES));
    m_visibleEntriesOpt->SetValue(allFiles == false ? B_CONTROL_ON : B_CONTROL_OFF);
    if (!allFiles)
        m_visibleEntriesOpt->Invoke();

    m_selEntriesOpt = new BRadioButton("SearchWindow:SelectedEntriesOpt", str(S_SEARCH_SELECTED_ENTRIES), new BMessage(M_SELECTED_ENTRIES));
    m_selEntriesOpt->SetValue(searchSelection == true ? B_CONTROL_ON : B_CONTROL_OFF);
    if (searchSelection)
        m_selEntriesOpt->Invoke();

    BView* view = new BGroupView();
    view->SetLayout(new BGroupLayout(B_VERTICAL));
    view->AddChild(BGroupLayoutBuilder(B_VERTICAL, 0)
                   .Add(m_allEntriesOpt)
                   .Add(m_visibleEntriesOpt)
                   .Add(m_selEntriesOpt)
                   .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
                  );

    scopeBox->AddChild(view);

    // Setup the scoping options group box
    BBox* optionsBox = new BBox("SearchWindow:OptionsBox");
    optionsBox->SetLabel(str(S_SEARCH_OPTIONS));
    optionsBox->SetFont(be_plain_font);

    // Draw the checkboxes for the (All, Visible) scope
    m_addToSelChk = new BCheckBox("SearchWindow:AddSelChk", str(S_SEARCH_ADD_TO_SELECTION), NULL);
    m_addToSelChk->SetValue(addToSelection == true ? B_CONTROL_ON : B_CONTROL_OFF);

    m_ignoreCaseChk = new BCheckBox("SearchWindow:IgnoreCaseChk", str(S_SEARCH_IGNORE_CASE), NULL);
    m_ignoreCaseChk->SetValue(ignoreCase == true ? B_CONTROL_ON : B_CONTROL_OFF);

    m_invertChk = new BCheckBox("SearchWindow:InvertChk", str(S_SEARCH_INVERT), NULL);
    m_invertChk->SetValue(invertSearch == true ? B_CONTROL_ON : B_CONTROL_OFF);

    view = new BGroupView();
    view->SetLayout(new BGroupLayout(B_VERTICAL));
    view->AddChild(BGroupLayoutBuilder(B_VERTICAL, 0)
                   .Add(m_addToSelChk)
                   .Add(m_ignoreCaseChk)
                   .Add(m_invertChk)
                   .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
                  );

    optionsBox->AddChild(view);

    // Render the search button
    m_searchBtn = new BButton("SearchWindow:SearchBtn", str(S_SEARCH), new BMessage(M_SEARCH_CLICKED));
    m_searchBtn->MakeDefault(true);
    m_searchBtn->SetEnabled(searchText ? true : false);

    // Render the close after search button
    m_persistentChk = new BCheckBox("SearchWindow:CloseChk", str(S_SEARCH_PERSISTENT), NULL);
    m_persistentChk->SetValue(persistent == true ? B_CONTROL_ON : B_CONTROL_OFF);

    AddChild(BGroupLayoutBuilder(B_VERTICAL)
             .AddGroup(B_HORIZONTAL)
             .Add(searchBmpView, 0)
             .Add(fileNameStr, 0)
             .AddGlue()
             .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
             .End()
             .AddGroup(B_HORIZONTAL)
             .Add(m_columnField)
             .Add(m_matchField)
             .End()
             .Add(m_searchTextControl)
             .AddGroup(B_HORIZONTAL)
             .Add(scopeBox)
             .Add(optionsBox)
             .End()
             .AddGroup(B_HORIZONTAL)
             .Add(m_persistentChk)
             .Add(m_searchBtn)
             .End()
             .SetInsets(4 * K_MARGIN, 2 * K_MARGIN, 4 * K_MARGIN, 2 * K_MARGIN)
            );

    m_searchTextControl->MakeFocus(true);

    // Center window on-screen
    CenterOnScreen();

    // Move according to the previous position (if any)
    if (windowrect.left > 0)
    {
        MoveTo(windowrect.LeftTop());
        ResizeTo(windowrect.Width(), windowrect.Height());
    }

    // Assign tooltips
    SetToolTips();
}



void SearchWindow::Quit()
{
    BMessage msg;
    GetSettings(msg, M_SEARCH_CLOSED);

    m_callerWindow->PostMessage(&msg);
    BWindow::Quit();
}



void SearchWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_SEARCH_TEXT_MODIFIED:
        {
            m_searchBtn->SetEnabled(strlen(m_searchTextControl->Text()) > 0L ? true : false);
            break;
        }

        case M_ALL_ENTRIES: case M_VISIBLE_ENTRIES:
        {
            if (strcmp(m_addToSelChk->Label(), str(S_SEARCH_ADD_TO_SELECTION)) != 0)
            {
                m_addToSelChk->SetToolTip(const_cast<char*>(str(S_SEARCH_ADD_TO_SELECTION_BH)));
                m_addToSelChk->SetLabel(str(S_SEARCH_ADD_TO_SELECTION));
                m_addToSelChk->ResizeToPreferred();
            }

            break;
        }

        case M_SELECTED_ENTRIES:
        {
            if (strcmp(m_addToSelChk->Label(), str(S_SEARCH_DESELECT_UNMATCHED_ENTRIES)) != 0)
            {
                m_addToSelChk->SetToolTip(const_cast<char*>(str(S_SEARCH_DESELECT_UNMATCHED_ENTRIES_BH)));
                m_addToSelChk->SetLabel(str(S_SEARCH_DESELECT_UNMATCHED_ENTRIES));
                m_addToSelChk->ResizeToPreferred();
            }

            break;
        }

        case M_SEARCH_CLICKED:
        {
            const char* searchText = m_searchTextControl->Text();
            if (!searchText || strlen(searchText) == 0L)
            {
                // Shouldn't really come here
                m_searchBtn->SetEnabled(false);
                break;
            }

            BMessage msg;
            GetSettings(msg, M_SEARCH);
            bool persistent = msg.FindBool(kPersistent);
            if (!persistent)
                Hide();

            m_callerWindow->PostMessage(&msg);

            if (!persistent)
                Quit();

            break;
        }
    }

    BWindow::MessageReceived(message);
}



int32 SearchWindow::ExpressionType() const
{
    // Return the expression code from the menu field object m_matchField (assumed looper locked)
    BMenuItem* item = m_matchField->Menu()->FindMarked();
    if (!item)
        return kNone;

    int32 index = m_matchField->Menu()->IndexOf(item);

    if (index < kStartsWith || index > kRegexpMatch)
        return kNone;

    RegExStringExpressionType typeArray[] =
    {
        kStartsWith,
        kEndsWith,
        kContains,
        kGlobMatch,
        kRegexpMatch
    };

    return typeArray[index];
}



CLVColumn* SearchWindow::Column() const
{
    // Return the pointer to the column that is selected in the m_columnField menu
    BMenuItem* item = m_columnField->Menu()->FindMarked();
    if (!item)
        return NULL;

    int32 index = m_columnField->Menu()->IndexOf(item);

    return (CLVColumn*)m_tmpList.ItemAtFast(index);
}



void SearchWindow::SetToolTips()
{
    m_allEntriesOpt->SetToolTip(const_cast<char*>(str(S_SEARCH_ALL_ENTRIES_BH)));
    m_visibleEntriesOpt->SetToolTip(const_cast<char*>(str(S_SEARCH_VISIBLE_ENTRIES_BH)));
    m_selEntriesOpt->SetToolTip(const_cast<char*>(str(S_SEARCH_SELECTED_ENTRIES_BH)));

    m_addToSelChk->SetToolTip(const_cast<char*>(str(S_SEARCH_ADD_TO_SELECTION_BH)));
    m_ignoreCaseChk->SetToolTip(const_cast<char*>(str(S_SEARCH_IGNORE_CASE_BH)));
    m_invertChk->SetToolTip(const_cast<char*>(str(S_SEARCH_INVERT_BH)));

    m_searchTextControl->SetToolTip(const_cast<char*>(str(S_SEARCH_FOR_BH)));
    m_matchField->SetToolTip(const_cast<char*>(str(S_SEARCH_MATCH_BH)));
    m_columnField->SetToolTip(const_cast<char*>(str(S_SEARCH_COLUMN_BH)));

    m_persistentChk->SetToolTip(const_cast<char*>(str(S_SEARCH_PERSISTENT_BH)));
}



void SearchWindow::GetSettings(BMessage& msg, uint32 msgwhat) const
{
    msg.what = msgwhat;
    msg.AddInt32(kExprType, ExpressionType());
    msg.AddString(kExpr, m_searchTextControl->Text());
    msg.AddPointer(kColumnPtr, Column());
    msg.AddBool(kIgnoreCase, m_ignoreCaseChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool(kInvertSelection, m_invertChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool(kAddToSelection, m_addToSelChk->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool(kSearchSelection, m_selEntriesOpt->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool(kAllFiles, m_allEntriesOpt->Value() == B_CONTROL_ON ? true : false);
    msg.AddBool(kPersistent, m_persistentChk->Value() == B_CONTROL_OFF ? false : true);
    msg.AddRect(kWindowRect, Frame());
}


