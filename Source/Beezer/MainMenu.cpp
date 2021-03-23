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

#include <Application.h>
#include <Menu.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <String.h>

#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "Beezer.h"
#include "BitmapMenuItem.h"
#include "BitmapPool.h"
#include "MainMenu.h"
#include "MsgConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainMenu"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


MainMenu::MainMenu(BRect frame)
    : BMenuBar(frame, "MainMenu", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_ITEMS_IN_ROW, true)
{
    m_recentMenu = NULL;
    m_extractPathsMenu = NULL;

    m_fileMenu = new BMenu(B_TRANSLATE("File"));
    m_fileMenu->AddItem(new BMenuItem(B_TRANSLATE("New"), new BMessage(M_FILE_NEW), 'N'));
    SetRecentMenu(new BMenu(B_TRANSLATE("Open")));

    m_fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Close"), new BMessage(M_FILE_CLOSE), 'W'));
    m_fileMenu->AddSeparatorItem();
    m_fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Delete"), new BMessage(M_FILE_DELETE)));
    m_fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Archive info"), new BMessage(M_FILE_ARCHIVE_INFO)));
    m_fileMenu->AddSeparatorItem();
    m_fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Password"), new BMessage(M_FILE_PASSWORD)));


    m_editMenu = new BMenu(B_TRANSLATE("Edit"));
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Copy"), new BMessage(M_EDIT_COPY), 'C'));
    m_editMenu->AddSeparatorItem();
    m_selectAllMenu = new BMenu(B_TRANSLATE("Select all"));
    m_selectAllMenu->AddItem(new BMenuItem(B_TRANSLATE("Folders"), new BMessage(M_EDIT_SELECT_ALL_DIRS)));
    m_selectAllMenu->AddItem(new BMenuItem(B_TRANSLATE("Files"), new BMessage(M_EDIT_SELECT_ALL_FILES)));
    m_editMenu->AddItem(m_selectAllMenu);
    BMenuItem* selectAllItem = m_editMenu->FindItem(B_TRANSLATE("Select all"));
    selectAllItem->SetMessage(new BMessage(M_EDIT_SELECT_ALL));
    selectAllItem->SetShortcut('A', 0);

    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Deselect all"), new BMessage(M_EDIT_DESELECT_ALL)));
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Invert selection"), new BMessage(M_EDIT_INVERT_SELECTION), 'I',
                                      B_SHIFT_KEY));
    m_editMenu->AddSeparatorItem();
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Expand all"), new BMessage(M_EDIT_EXPAND_ALL)));
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Expand selected"), new BMessage(M_EDIT_EXPAND_SELECTED)));
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Collapse all"), new BMessage(M_EDIT_COLLAPSE_ALL)));
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Collapse selected"), new BMessage(M_EDIT_COLLAPSE_SELECTED)));
    m_editMenu->AddSeparatorItem();
    m_editMenu->AddItem(new BMenuItem(B_TRANSLATE("Settings…"), new BMessage(M_EDIT_PREFERENCES)));

    m_actionsMenu = new BMenu(B_TRANSLATE("Actions"));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Extract"), new BMessage(M_ACTIONS_EXTRACT), 'X'));
    SetExtractPathsMenu(new BMenu(B_TRANSLATE(S_EXTRACT_TO)));

    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("View file"), new BMessage(M_ACTIONS_VIEW), 'V'));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Open with…"), new BMessage(M_ACTIONS_OPEN_WITH), 'O'));
    m_actionsMenu->AddSeparatorItem();
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Test"), new BMessage(M_ACTIONS_TEST), 'T'));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Search archive"), new BMessage(M_ACTIONS_SEARCH_ARCHIVE), 'F'));
    //m_actionsMenu->AddItem (new BMenuItem (str (S_DEEP_SEARCH), new BMessage (M_ACTIONS_DEEP_SEARCH), 'F',
    //                  B_SHIFT_KEY));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Comment"), new BMessage(M_ACTIONS_COMMENT), 'C', B_SHIFT_KEY));
    m_actionsMenu->AddSeparatorItem();
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Delete"), new BMessage(M_ACTIONS_DELETE), 'D'));
    //m_actionsMenu->AddItem (new BMenuItem (str (S_RENAME), new BMessage (M_ACTIONS_RENAME), 'E'));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Create folder"), new BMessage(M_ACTIONS_CREATE_FOLDER), 'M'));
    m_actionsMenu->AddItem(new BMenuItem(B_TRANSLATE("Add"), new BMessage(M_ACTIONS_ADD), 'A', B_SHIFT_KEY));

    m_settingsMenu = new BMenu(B_TRANSLATE("Settings"));

    m_settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Save as defaults"), new BMessage(M_SAVE_AS_DEFAULT)));
    m_settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Save to archive"), new BMessage(M_SAVE_TO_ARCHIVE)));
    m_settingsMenu->AddSeparatorItem();

    m_settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Toolbar"), new BMessage(M_TOGGLE_TOOLBAR)));
    m_settingsMenu->FindItem(M_TOGGLE_TOOLBAR)->SetMarked(true);
    m_settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Infobar"), new BMessage(M_TOGGLE_INFOBAR)));
    m_settingsMenu->FindItem(M_TOGGLE_INFOBAR)->SetMarked(true);
    m_settingsMenu->AddItem(new BMenuItem(B_TRANSLATE("Action log"), new BMessage(M_TOGGLE_LOG)));
    m_settingsMenu->FindItem(M_TOGGLE_LOG)->SetMarked(true);

    m_columnsSubMenu = new BMenu(B_TRANSLATE("Columns"));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Name"), new BMessage(M_TOGGLE_COLUMN_NAME)));
    m_columnsSubMenu->ItemAt(0L)->SetEnabled(false);
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Size"), new BMessage(M_TOGGLE_COLUMN_SIZE)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Packed"), new BMessage(M_TOGGLE_COLUMN_PACKED)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Ratio"), new BMessage(M_TOGGLE_COLUMN_RATIO)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Path"), new BMessage(M_TOGGLE_COLUMN_PATH)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Date"), new BMessage(M_TOGGLE_COLUMN_DATE)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("Method"), new BMessage(M_TOGGLE_COLUMN_METHOD)));
    m_columnsSubMenu->AddItem(new BMenuItem(B_TRANSLATE("CRC"), new BMessage(M_TOGGLE_COLUMN_CRC)));
    m_settingsMenu->AddItem(m_columnsSubMenu);

    int32 columnCount = m_columnsSubMenu->CountItems();
    for (int32 i = 0; i < columnCount; i++)
        m_columnsSubMenu->ItemAt(i)->SetMarked(true);

    m_foldingMenu = new BMenu(B_TRANSLATE("While opening"));
    m_foldingMenu->SetRadioMode(true);
    m_foldingMenu->AddItem(new BMenuItem(B_TRANSLATE("Show all items folded"), NULL));
    m_foldingMenu->AddItem(new BMenuItem(B_TRANSLATE("Show first level unfolded"), NULL));
    m_foldingMenu->AddItem(new BMenuItem(B_TRANSLATE("Show first 2 levels unfolded"), NULL));
    m_foldingMenu->AddItem(new BMenuItem(B_TRANSLATE("Show all items unfolded"), NULL));
    m_foldingMenu->ItemAt(3)->SetMarked(true);

    m_settingsMenu->AddItem(m_foldingMenu);

    m_windowsMenu = new BMenu(B_TRANSLATE("Windows"));

    BMenu* systemMenu = new BMenu("");
    BString strBuf = B_TRANSLATE("About");
    strBuf << " " << B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE) << B_UTF8_ELLIPSIS;

    systemMenu->AddItem(new BMenuItem(B_TRANSLATE("Help"), new BMessage(M_FILE_HELP)));
    systemMenu->AddSeparatorItem();
    systemMenu->AddItem(new BMenuItem(strBuf.String(), new BMessage(M_FILE_ABOUT)));

    systemMenu->AddSeparatorItem();
    systemMenu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(M_FILE_QUIT), 'Q'));

    m_systemMenu = new BitmapMenuItem(systemMenu, _glob_bitmap_pool->m_smallAppIcon);

    // Convert the popup tools menu into a proper BMenu, mere type-casting won't work
    m_toolsMenu = _bzr()->BuildToolsMenu();

    AddItem(m_systemMenu);
    AddItem(m_fileMenu);
    AddItem(m_editMenu);
    AddItem(m_actionsMenu);
    AddItem(m_toolsMenu);
    AddItem(m_settingsMenu);
    AddItem(m_windowsMenu);

    m_archiveContextMenu = new BPopUpMenu("_cntxt", false, false);
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("View"), new BMessage(M_ACTIONS_VIEW)));
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Open with"), new BMessage(M_ACTIONS_OPEN_WITH)));
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Extract"), new BMessage(M_ACTIONS_EXTRACT_SELECTED)));
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Delete"), new BMessage(M_ACTIONS_DELETE)));
    m_archiveContextMenu->AddSeparatorItem();
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Copy row as text"), new BMessage(M_CONTEXT_COPY)));
    m_archiveContextMenu->AddSeparatorItem();
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Select folder"), new BMessage(M_CONTEXT_SELECT)));
    m_archiveContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Deselect folder"), new BMessage(M_CONTEXT_DESELECT)));

    m_logContextMenu = new BPopUpMenu("_cntxt", false, false);
    m_logContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Clear log"), new BMessage(M_LOG_CONTEXT_CLEAR)));
    m_logContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Copy log"), new BMessage(M_LOG_CONTEXT_COPY)));
    //m_logContextMenu->AddItem (new BMenuItem (str (S_LOG_CONTEXT_SAVE), new BMessage (M_LOG_CONTEXT_SAVE)));
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
    BMenuItem* openItem = m_fileMenu->FindItem(B_TRANSLATE("Open"));
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
    BMenuItem* extractItem = m_actionsMenu->FindItem(B_TRANSLATE(S_EXTRACT_TO));
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
    menu->Superitem()->SetLabel(B_TRANSLATE(S_EXTRACT_SELECTED));
    BMenuItem* extractItem = m_actionsMenu->FindItem(B_TRANSLATE(S_EXTRACT_SELECTED));
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


