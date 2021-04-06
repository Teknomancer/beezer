// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Application.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <String.h>

#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "BeezerApp.h"
#include "BitmapMenuItem.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "MainMenu.h"
#include "MsgConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainMenu"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_COMMENT(x, y) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


MainMenu::MainMenu(BRect frame)
    : BMenuBar(frame, "MainMenu", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_ITEMS_IN_ROW, true)
{
    BString bufStr(B_TRANSLATE(skAboutString));
    bufStr << " " << B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE) << B_UTF8_ELLIPSIS;

    BMenu* appMenu = new BMenu(B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    BLayoutBuilder::Menu<>(appMenu)
        .AddItem(bufStr, B_ABOUT_REQUESTED)
        .AddItem(B_TRANSLATE("Settings" B_UTF8_ELLIPSIS), M_EDIT_PREFERENCES)
        .AddSeparator()
        .AddItem(B_TRANSLATE("Quit"), M_FILE_QUIT, 'Q');

    // keep track of these temporarily so we can call SetMarked()
    BMenuItem* startupFoldedItem = NULL,
             * viewToolbarItem = NULL,
             * viewInfobarItem = NULL,
             * viewActionLogItem = NULL;

    BLayoutBuilder::Menu<>(this)
        .AddItem(new BitmapMenuItem(appMenu, _glob_bitmap_pool->m_smallAppIcon))
        .AddMenu(B_TRANSLATE("File"))
            .GetMenu(m_fileMenu)
            .AddItem(B_TRANSLATE("New" B_UTF8_ELLIPSIS), M_FILE_NEW, 'N')
            .AddMenu(B_TRANSLATE(skOpenMenuString))
                .GetMenu(m_recentMenu)
            .End()
            .AddItem(B_TRANSLATE("Close"), M_FILE_CLOSE, 'W')
            .AddSeparator()
            .AddItem(B_TRANSLATE(skDeleteString), M_FILE_DELETE)
            .AddItem(B_TRANSLATE("Archive information"), M_FILE_ARCHIVE_INFO, 'I')
            .AddSeparator()
            .AddItem(B_TRANSLATE("Password" B_UTF8_ELLIPSIS), M_FILE_PASSWORD)
        .End()
        .AddMenu(B_TRANSLATE("Edit"))
            .GetMenu(m_editMenu)
            .AddItem(B_TRANSLATE("Copy"), M_EDIT_COPY, 'C')
            .AddSeparator()
            .AddMenu(B_TRANSLATE(skSelectAllString))
                .AddItem(B_TRANSLATE_COMMENT("Folders", "for 'Select all' menu"), M_EDIT_SELECT_ALL_DIRS)
                .AddItem(B_TRANSLATE_COMMENT("Files", "for 'Select all' menu"), M_EDIT_DESELECT_ALL_FILES)
            .End()
            .AddItem(B_TRANSLATE("Deselect all"), M_EDIT_DESELECT_ALL)
            .AddItem(B_TRANSLATE("Invert selection"), M_EDIT_INVERT_SELECTION, 'I', B_SHIFT_KEY)
            .AddSeparator()
            .AddItem(B_TRANSLATE("Expand all"), M_EDIT_EXPAND_ALL)
            .AddItem(B_TRANSLATE("Expand selected"), M_EDIT_EXPAND_SELECTED)
            .AddItem(B_TRANSLATE("Collapse all"), M_EDIT_COLLAPSE_ALL)
            .AddItem(B_TRANSLATE("Collapse selected"), M_EDIT_COLLAPSE_SELECTED)
        .End()
        .AddMenu(B_TRANSLATE(skViewString))
            .GetMenu(m_viewMenu)
            .AddItem(B_TRANSLATE("Toolbar"), M_TOGGLE_TOOLBAR)
            .GetItem(viewToolbarItem)
            .AddItem(B_TRANSLATE("Infobar"), M_TOGGLE_INFOBAR)
            .GetItem(viewInfobarItem)
            .AddItem(B_TRANSLATE("Action log"), M_TOGGLE_LOG)
            .GetItem(viewActionLogItem)
            .AddMenu(B_TRANSLATE("Columns"))
                .GetMenu(m_columnsSubMenu)
                .AddItem(B_TRANSLATE(skNameString), M_TOGGLE_COLUMN_NAME).SetEnabled(false)
                .AddItem(B_TRANSLATE(skSizeString), M_TOGGLE_COLUMN_SIZE)
                .AddItem(B_TRANSLATE(skPackedString), M_TOGGLE_COLUMN_PACKED)
                .AddItem(B_TRANSLATE(skRatioString), M_TOGGLE_COLUMN_RATIO)
                .AddItem(B_TRANSLATE(skPathString), M_TOGGLE_COLUMN_PATH)
                .AddItem(B_TRANSLATE(skDateString), M_TOGGLE_COLUMN_DATE)
                .AddItem(B_TRANSLATE(skMethodString), M_TOGGLE_COLUMN_METHOD)
                .AddItem(B_TRANSLATE(skCRCString), M_TOGGLE_COLUMN_CRC)
            .End()
            .AddMenu(B_TRANSLATE("While opening"))
                .GetMenu(m_foldingMenu)
                .AddItem(B_TRANSLATE(skAllFoldedString), (BMessage*)NULL)
                .AddItem(B_TRANSLATE(skFirstUnfoldedString), (BMessage*)NULL)
                .AddItem(B_TRANSLATE(skTwoUnfoldedString), (BMessage*)NULL)
                .AddItem(B_TRANSLATE(skAllUnfoldedString), (BMessage*)NULL)
                .GetItem(startupFoldedItem)
            .End()
            .AddSeparator()
            .AddItem(B_TRANSLATE(skSaveAsDefaultsString), M_SAVE_AS_DEFAULT)
            .AddItem(B_TRANSLATE(skSaveToArchiveString), M_SAVE_TO_ARCHIVE)
        .End()
        .AddMenu(B_TRANSLATE("Actions"))
            .GetMenu(m_actionsMenu)
            .AddItem(B_TRANSLATE(skExtractString), M_ACTIONS_EXTRACT, 'X')
            .AddMenu(B_TRANSLATE(skExtractToString))
                .GetMenu(m_extractPathsMenu)
            .End()
            .AddItem(B_TRANSLATE("View file"), M_ACTIONS_VIEW, 'V')
            .AddItem(B_TRANSLATE("Open with" B_UTF8_ELLIPSIS), M_ACTIONS_OPEN_WITH, 'O')
            .AddSeparator()
            .AddItem(B_TRANSLATE("Test"), M_ACTIONS_TEST, 'T')
            .AddItem(B_TRANSLATE("Search archive" B_UTF8_ELLIPSIS), M_ACTIONS_SEARCH_ARCHIVE, 'F')
            .AddItem(B_TRANSLATE(skCommentString), M_ACTIONS_COMMENT, 'C', B_SHIFT_KEY)
            .AddSeparator()
            .AddItem(B_TRANSLATE("Add" B_UTF8_ELLIPSIS), M_ACTIONS_ADD, 'A', B_SHIFT_KEY)
            .AddItem(B_TRANSLATE(skDeleteString), M_ACTIONS_DELETE, 'D')
            .AddItem(B_TRANSLATE("Create folder" B_UTF8_ELLIPSIS), M_ACTIONS_CREATE_FOLDER, 'M')
        .End()
        .AddMenu(_bzr()->BuildToolsMenu())
            .GetMenu(m_toolsMenu)
        .End()
        .AddMenu(B_TRANSLATE(skWindowsString))
            .GetMenu(m_windowsMenu)
        .End()
        .AddMenu(B_TRANSLATE(skHelpString))
            .AddItem(B_TRANSLATE("Open manual"), M_HELP_MANUAL)
            .AddSeparator()
            .AddItem(B_TRANSLATE("Visit website"), M_HELP_WEBSITE)
            .AddItem(B_TRANSLATE("Github page"), M_HELP_GITHUB)
        .End();


    SetRecentMenu(new BMenu(B_TRANSLATE(skOpenMenuString)));

    SetExtractPathsMenu(new BMenu(B_TRANSLATE(skExtractToString)));

    BMenuItem* selectAllItem = m_editMenu->FindItem(B_TRANSLATE(skSelectAllString));
    selectAllItem->SetMessage(new BMessage(M_EDIT_SELECT_ALL));
    selectAllItem->SetShortcut('A', 0);

    m_foldingMenu->SetRadioMode(true);

    startupFoldedItem->SetMarked(true);
    viewToolbarItem->SetMarked(true);
    viewInfobarItem->SetMarked(true);
    viewActionLogItem->SetMarked(true);
    int32 columnCount = m_columnsSubMenu->CountItems();
    for (int32 i = 0; i < columnCount; i++)
        m_columnsSubMenu->ItemAt(i)->SetMarked(true);


    m_archiveContextMenu = new BPopUpMenu("_cntxt", false, false);
    BLayoutBuilder::Menu<>(m_archiveContextMenu)
        .AddItem(B_TRANSLATE("View file"), M_ACTIONS_VIEW)
        .AddItem(B_TRANSLATE("Open with" B_UTF8_ELLIPSIS), M_ACTIONS_OPEN_WITH)
        .AddItem(B_TRANSLATE(skExtractString), M_ACTIONS_EXTRACT_SELECTED)
        .AddItem(B_TRANSLATE(skDeleteString), M_ACTIONS_DELETE)
        .AddSeparator()
        .AddItem(B_TRANSLATE("Copy row as text"), M_CONTEXT_COPY)
        .AddSeparator()
        .AddItem(B_TRANSLATE("Select folder"), M_CONTEXT_SELECT)
        .AddItem(B_TRANSLATE("Deselect folder"), M_CONTEXT_DESELECT);

    m_logContextMenu = new BPopUpMenu("_cntxt", false, false);
    BLayoutBuilder::Menu<>(m_logContextMenu)
        .AddItem(B_TRANSLATE("Clear log"), M_LOG_CONTEXT_CLEAR)
        .AddItem(B_TRANSLATE("Copy log"), M_LOG_CONTEXT_COPY);
}


MainMenu::~MainMenu()
{
    delete m_archiveContextMenu;
    m_archiveContextMenu = NULL;

    delete m_logContextMenu;
    m_logContextMenu = NULL;
}


void MainMenu::SetRecentMenu(BMenu* menu)
{
    if (m_recentMenu != NULL)
        if (m_fileMenu->RemoveItem(m_recentMenu))
            delete m_recentMenu;

    m_recentMenu = menu;
    m_fileMenu->AddItem(m_recentMenu, 1);
    BMenuItem* openItem = m_fileMenu->FindItem(B_TRANSLATE(skOpenMenuString));
    openItem->SetMessage(new BMessage(M_FILE_OPEN));
    openItem->SetShortcut('O', 0);
}


BMenu* MainMenu::RecentMenu() const
{
    return m_recentMenu;
}


void MainMenu::SetExtractPathsMenu(BMenu* menu)
{
    if (m_extractPathsMenu != NULL)
        if (m_actionsMenu->RemoveItem(m_extractPathsMenu))
            delete m_extractPathsMenu;

    m_extractPathsMenu = menu;
    m_actionsMenu->AddItem(m_extractPathsMenu, 1);
    BMenuItem* extractItem = m_actionsMenu->FindItem(B_TRANSLATE(skExtractToString));
    extractItem->SetMessage(new BMessage(M_ACTIONS_EXTRACT_TO));
    extractItem->SetShortcut('X', B_SHIFT_KEY);

    BMessage archiveMessage;
    menu->Archive(&archiveMessage, true);
    SetExtractSelPathsMenu(new BMenu(&archiveMessage));
}


void MainMenu::SetExtractSelPathsMenu(BMenu* menu)
{
    // Preserve enabled/disabled state
    if (m_extractSelPathsMenu != NULL)
        if (m_actionsMenu->RemoveItem(m_extractSelPathsMenu))
            delete m_extractSelPathsMenu;

    m_extractSelPathsMenu = menu;
    m_actionsMenu->AddItem(m_extractSelPathsMenu, 2);
    menu->Superitem()->SetLabel(B_TRANSLATE(skExtractSelectedString));
    BMenuItem* extractItem = m_actionsMenu->FindItem(B_TRANSLATE(skExtractSelectedString));
    if (extractItem)
    {
        extractItem->SetMessage(new BMessage(M_ACTIONS_EXTRACT_SELECTED));
        extractItem->SetShortcut('S', B_SHIFT_KEY);
    }
}


BMenu* MainMenu::ExtractPathsMenu() const
{
    return m_extractPathsMenu;
}
