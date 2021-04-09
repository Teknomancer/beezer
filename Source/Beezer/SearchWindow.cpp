// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

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

#include "AppConstants.h"
#include "Archiver.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "SearchWindow.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SearchWindow"
#else
#define B_TRANSLATE(x) x
#endif


SearchWindow::SearchWindow(BWindow* callerWindow, BMessage* loadMessage,
                           const BEntry* entry, const BList* columnList, const Archiver* ark)
    : BWindow(BRect(0, 0, 300, 300), B_TRANSLATE("Search archive"), B_FLOATING_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
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

    if (m_loadMessage != NULL)
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

    m_columnField = new BMenuField("SearchWindow:ColumnField", B_TRANSLATE("Column:"), columnMenu);

    // Setup the match type and the match type menu
    BMenu* matchMenu = new BPopUpMenu("");
    matchMenu->AddItem(new BMenuItem(B_TRANSLATE("Starts with"), NULL));
    matchMenu->AddItem(new BMenuItem(B_TRANSLATE("Ends with"), NULL));
    matchMenu->AddItem(new BMenuItem(B_TRANSLATE("Contains"), NULL));
    matchMenu->AddItem(new BMenuItem(B_TRANSLATE("Matches wildcard expression"), NULL));
    matchMenu->AddItem(new BMenuItem(B_TRANSLATE("Matches regular expression"), NULL));

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
    scopeBox->SetLabel(BZ_TR(kSearchString));
    scopeBox->SetFont(be_plain_font);

    // Draw the radio buttons inside the group box (co-ordinates are relative to the group box)
    m_allEntriesOpt = new BRadioButton("SearchWindow:AllEntriesOpt", B_TRANSLATE("All entries"), new BMessage(M_ALL_ENTRIES));
    m_allEntriesOpt->SetValue(allFiles == true ? B_CONTROL_ON : B_CONTROL_OFF);
    if (allFiles)
        m_allEntriesOpt->Invoke();

    m_visibleEntriesOpt = new BRadioButton("SearchWindow:VisibleEntriesOpt", B_TRANSLATE("Visible entries"), new BMessage(M_VISIBLE_ENTRIES));
    m_visibleEntriesOpt->SetValue(allFiles == false ? B_CONTROL_ON : B_CONTROL_OFF);
    if (!allFiles)
        m_visibleEntriesOpt->Invoke();

    m_selEntriesOpt = new BRadioButton("SearchWindow:SelectedEntriesOpt", B_TRANSLATE("Selected entries"), new BMessage(M_SELECTED_ENTRIES));
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
    optionsBox->SetLabel(B_TRANSLATE("Options"));
    optionsBox->SetFont(be_plain_font);

    // Draw the checkboxes for the (All, Visible) scope
    m_addToSelChk = new BCheckBox("SearchWindow:AddSelChk", B_TRANSLATE("Add to selection"), NULL);
    m_addToSelChk->SetValue(addToSelection == true ? B_CONTROL_ON : B_CONTROL_OFF);

    m_ignoreCaseChk = new BCheckBox("SearchWindow:IgnoreCaseChk", B_TRANSLATE("Ignore case"), NULL);
    m_ignoreCaseChk->SetValue(ignoreCase == true ? B_CONTROL_ON : B_CONTROL_OFF);

    m_invertChk = new BCheckBox("SearchWindow:InvertChk", B_TRANSLATE("Invert"), NULL);
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
    m_searchBtn = new BButton("SearchWindow:SearchBtn", BZ_TR(kSearchString), new BMessage(M_SEARCH_CLICKED));
    m_searchBtn->MakeDefault(true);
    m_searchBtn->SetEnabled(searchText ? true : false);

    // Render the close after search button
    m_persistentChk = new BCheckBox("SearchWindow:CloseChk", B_TRANSLATE("Persistent window"), NULL);
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

    if (callerWindow != NULL && windowrect.left < 0)
    {
        UpdateSizeLimits();
        BRect callerRect(callerWindow->Frame());
        BPoint windowPoint(callerRect.left + callerRect.Width()/2 - Bounds().Width()/2, callerRect.top + callerRect.Height()/2 - Bounds().Height()/2);
        // looks a bit better if we offset the window and don't cover the contents completely
        windowPoint.x += 100;
        windowPoint.y += 100;
        MoveTo(windowPoint);
    }
    else
        CenterOnScreen();

    // Move according to the previous position (if any)
    if (windowrect.left > 0)
    {
        MoveTo(windowrect.LeftTop());
        ResizeTo(windowrect.Width(), windowrect.Height());
    }

    // Assign tooltips
    SetToolTips();

    AddShortcut('W', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));
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
            if (strcmp(m_addToSelChk->Label(), B_TRANSLATE("Add to selection")) != 0)
            {
                m_addToSelChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Adds found items to existing selection (if any)")));
                m_addToSelChk->SetLabel(B_TRANSLATE("Add to selection"));
                m_addToSelChk->ResizeToPreferred();
            }

            break;
        }

        case M_SELECTED_ENTRIES:
        {
            if (strcmp(m_addToSelChk->Label(), B_TRANSLATE("Deselect unmatched entries")) != 0)
            {
                m_addToSelChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Deselects entries that don't match the search criteria")));
                m_addToSelChk->SetLabel(B_TRANSLATE("Deselect unmatched entries"));
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
    m_allEntriesOpt->SetToolTip(const_cast<char*>(B_TRANSLATE("Searches the entire archive(Automatically unfolds all folders before searching)")));
    m_visibleEntriesOpt->SetToolTip(const_cast<char*>(B_TRANSLATE("Searches all visible entries(Contents of folded folders will not be searched)")));
    m_selEntriesOpt->SetToolTip(const_cast<char*>(B_TRANSLATE("Searches only selected entries")));

    m_addToSelChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Adds found items to existing selection (if any)")));
    m_ignoreCaseChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Treats uppercase and lowercase letters as the same")));
    m_invertChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Selects all entries that don't match the search criteria i.e. inverts the result of the search")));

    m_searchTextControl->SetToolTip(const_cast<char*>(B_TRANSLATE("What you are searching for")));
    m_matchField->SetToolTip(const_cast<char*>(B_TRANSLATE("The type of matching to do in the search")));
    m_columnField->SetToolTip(const_cast<char*>(B_TRANSLATE("Which column to search in")));

    m_persistentChk->SetToolTip(const_cast<char*>(B_TRANSLATE("Prevents closing of this window before searching")));
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
