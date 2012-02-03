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
#include "LangStrings.h"
#include "MainMenu.h"
#include "MsgConstants.h"



MainMenu::MainMenu (BRect frame)
    : BMenuBar (frame, "MainMenu", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_ITEMS_IN_ROW, true)
{
    m_recentMenu = NULL;
    m_extractPathsMenu = NULL;

    m_fileMenu = new BMenu (str (S_FILE));
    m_fileMenu->AddItem (new BMenuItem (str (S_NEW), new BMessage (M_FILE_NEW), 'N'));
        SetRecentMenu (new BMenu (str (S_OPEN)));

    m_fileMenu->AddItem (new BMenuItem (str (S_CLOSE), new BMessage (M_FILE_CLOSE), 'W'));
    m_fileMenu->AddSeparatorItem ();
    m_fileMenu->AddItem (new BMenuItem (str (S_DELETE), new BMessage (M_FILE_DELETE)));
    m_fileMenu->AddItem (new BMenuItem (str (S_ARCHIVE_INFO), new BMessage (M_FILE_ARCHIVE_INFO)));
    m_fileMenu->AddSeparatorItem();
    m_fileMenu->AddItem (new BMenuItem (str (S_PASSWORD), new BMessage (M_FILE_PASSWORD)));


    m_editMenu = new BMenu (str (S_EDIT));
    m_editMenu->AddItem (new BMenuItem (str (S_COPY), new BMessage (M_EDIT_COPY), 'C'));
    m_editMenu->AddSeparatorItem();
        m_selectAllMenu = new BMenu (str (S_SELECT_ALL));
        m_selectAllMenu->AddItem (new BMenuItem (str (S_ALL_DIRS), new BMessage (M_EDIT_SELECT_ALL_DIRS)));
        m_selectAllMenu->AddItem (new BMenuItem (str (S_ALL_FILES), new BMessage (M_EDIT_SELECT_ALL_FILES)));
    m_editMenu->AddItem (m_selectAllMenu);
        BMenuItem *selectAllItem = m_editMenu->FindItem (str (S_SELECT_ALL));
        selectAllItem->SetMessage (new BMessage (M_EDIT_SELECT_ALL));
        selectAllItem->SetShortcut ('A', 0);

    m_editMenu->AddItem (new BMenuItem (str (S_DESELECT_ALL), new BMessage (M_EDIT_DESELECT_ALL)));
    m_editMenu->AddItem (new BMenuItem (str (S_INVERT_SELECTION), new BMessage (M_EDIT_INVERT_SELECTION), 'I',
                  B_SHIFT_KEY));
    m_editMenu->AddSeparatorItem();
    m_editMenu->AddItem (new BMenuItem (str (S_EXPAND_ALL), new BMessage (M_EDIT_EXPAND_ALL)));
    m_editMenu->AddItem (new BMenuItem (str (S_EXPAND_SELECTED), new BMessage (M_EDIT_EXPAND_SELECTED)));
    m_editMenu->AddItem (new BMenuItem (str (S_COLLAPSE_ALL), new BMessage (M_EDIT_COLLAPSE_ALL)));
    m_editMenu->AddItem (new BMenuItem (str (S_COLLAPSE_SELECTED), new BMessage (M_EDIT_COLLAPSE_SELECTED)));
    m_editMenu->AddSeparatorItem();
    m_editMenu->AddItem (new BMenuItem (str (S_PREFERENCES), new BMessage (M_EDIT_PREFERENCES)));

    m_actionsMenu = new BMenu (str (S_ACTIONS));
    m_actionsMenu->AddItem (new BMenuItem (str (S_EXTRACT), new BMessage (M_ACTIONS_EXTRACT), 'X'));
        SetExtractPathsMenu (new BMenu (str (S_EXTRACT_TO)));

    m_actionsMenu->AddItem (new BMenuItem (str (S_VIEW), new BMessage (M_ACTIONS_VIEW), 'V'));
    m_actionsMenu->AddItem (new BMenuItem (str (S_OPEN_WITH), new BMessage (M_ACTIONS_OPEN_WITH), 'O'));
    m_actionsMenu->AddSeparatorItem();
    m_actionsMenu->AddItem (new BMenuItem (str (S_TEST), new BMessage (M_ACTIONS_TEST), 'T'));
    m_actionsMenu->AddItem (new BMenuItem (str (S_SEARCH_ARCHIVE), new BMessage (M_ACTIONS_SEARCH_ARCHIVE), 'F'));
    //m_actionsMenu->AddItem (new BMenuItem (str (S_DEEP_SEARCH), new BMessage (M_ACTIONS_DEEP_SEARCH), 'F',
    //                  B_SHIFT_KEY));
    m_actionsMenu->AddItem (new BMenuItem (str (S_COMMENT), new BMessage (M_ACTIONS_COMMENT), 'C', B_SHIFT_KEY));
    m_actionsMenu->AddSeparatorItem();
    m_actionsMenu->AddItem (new BMenuItem (str (S_DELETE_IN_ACTIONS), new BMessage (M_ACTIONS_DELETE), 'D'));
    //m_actionsMenu->AddItem (new BMenuItem (str (S_RENAME), new BMessage (M_ACTIONS_RENAME), 'E'));
    m_actionsMenu->AddItem (new BMenuItem (str (S_CREATE_FOLDER), new BMessage (M_ACTIONS_CREATE_FOLDER), 'M'));
    m_actionsMenu->AddItem (new BMenuItem (str (S_ADD), new BMessage (M_ACTIONS_ADD), 'A', B_SHIFT_KEY));

    m_settingsMenu = new BMenu (str (S_SETTINGS));

    m_settingsMenu->AddItem (new BMenuItem (str (S_SAVE_AS_DEFAULT), new BMessage (M_SAVE_AS_DEFAULT)));
    m_settingsMenu->AddItem (new BMenuItem (str (S_SAVE_TO_ARCHIVE), new BMessage (M_SAVE_TO_ARCHIVE)));
    m_settingsMenu->AddSeparatorItem();

    m_settingsMenu->AddItem (new BMenuItem (str (S_TOOLBAR), new BMessage (M_TOGGLE_TOOLBAR)));
    m_settingsMenu->FindItem(M_TOGGLE_TOOLBAR)->SetMarked (true);
    m_settingsMenu->AddItem (new BMenuItem (str (S_INFOBAR), new BMessage (M_TOGGLE_INFOBAR)));
    m_settingsMenu->FindItem(M_TOGGLE_INFOBAR)->SetMarked (true);
    m_settingsMenu->AddItem (new BMenuItem (str (S_ACTIVITY_LOG), new BMessage (M_TOGGLE_LOG)));
    m_settingsMenu->FindItem(M_TOGGLE_LOG)->SetMarked (true);

        m_columnsSubMenu = new BMenu (str (S_COLUMNS));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_NAME), new BMessage (M_TOGGLE_COLUMN_NAME)));
        m_columnsSubMenu->ItemAt(0L)->SetEnabled (false);
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_SIZE), new BMessage (M_TOGGLE_COLUMN_SIZE)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_PACKED), new BMessage (M_TOGGLE_COLUMN_PACKED)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_RATIO), new BMessage (M_TOGGLE_COLUMN_RATIO)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_PATH), new BMessage (M_TOGGLE_COLUMN_PATH)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_DATE), new BMessage (M_TOGGLE_COLUMN_DATE)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_METHOD), new BMessage (M_TOGGLE_COLUMN_METHOD)));
        m_columnsSubMenu->AddItem (new BMenuItem (str (S_COLUMN_CRC), new BMessage (M_TOGGLE_COLUMN_CRC)));
        m_settingsMenu->AddItem (m_columnsSubMenu);

        int32 columnCount = m_columnsSubMenu->CountItems();
        for (int32 i = 0; i < columnCount; i++)
           m_columnsSubMenu->ItemAt(i)->SetMarked (true);

        m_foldingMenu = new BMenu (str (S_SETTINGS_FOLDING));
        m_foldingMenu->SetRadioMode (true);
        m_foldingMenu->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_NONE), NULL));
        m_foldingMenu->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_ONE), NULL));
        m_foldingMenu->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_TWO), NULL));
        m_foldingMenu->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_ALL), NULL));
        m_foldingMenu->ItemAt(3)->SetMarked (true);

        m_settingsMenu->AddItem (m_foldingMenu);

    m_windowsMenu = new BMenu (str (S_WINDOWS));

    BMenu *systemMenu = new BMenu ("");
    BString strBuf = str (S_ABOUT);
    strBuf << " " << K_APP_TITLE << B_UTF8_ELLIPSIS;

    systemMenu->AddItem (new BMenuItem (str (S_HELP), new BMessage (M_FILE_HELP)));
    systemMenu->AddSeparatorItem();
    systemMenu->AddItem (new BMenuItem (strBuf.String(), new BMessage (M_FILE_ABOUT)));

    systemMenu->AddSeparatorItem();
    systemMenu->AddItem (new BMenuItem (str (S_QUIT), new BMessage (M_FILE_QUIT), 'Q'));

    m_systemMenu = new BitmapMenuItem (systemMenu, _glob_bitmap_pool->m_smallAppIcon);

    // Convert the popup tools menu into a proper BMenu, mere type-casting won't work
    m_toolsMenu = _bzr()->BuildToolsMenu();

    AddItem (m_systemMenu);
    AddItem (m_fileMenu);
    AddItem (m_editMenu);
    AddItem (m_actionsMenu);
    AddItem (m_toolsMenu);
    AddItem (m_settingsMenu);
    AddItem (m_windowsMenu);

    m_archiveContextMenu = new BPopUpMenu ("_cntxt", false, false);
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_VIEW), new BMessage (M_ACTIONS_VIEW)));
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_OPEN_WITH), new BMessage (M_ACTIONS_OPEN_WITH)));
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_EXTRACT), new BMessage (M_ACTIONS_EXTRACT_SELECTED)));
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_DELETE), new BMessage (M_ACTIONS_DELETE)));
    m_archiveContextMenu->AddSeparatorItem();
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_COPY), new BMessage (M_CONTEXT_COPY)));
    m_archiveContextMenu->AddSeparatorItem();
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_SELECT), new BMessage (M_CONTEXT_SELECT)));
    m_archiveContextMenu->AddItem (new BMenuItem (str (S_CONTEXT_DESELECT), new BMessage (M_CONTEXT_DESELECT)));

    m_logContextMenu = new BPopUpMenu ("_cntxt", false, false);
    m_logContextMenu->AddItem (new BMenuItem (str (S_LOG_CONTEXT_CLEAR), new BMessage (M_LOG_CONTEXT_CLEAR)));
    m_logContextMenu->AddItem (new BMenuItem (str (S_LOG_CONTEXT_COPY), new BMessage (M_LOG_CONTEXT_COPY)));
    //m_logContextMenu->AddItem (new BMenuItem (str (S_LOG_CONTEXT_SAVE), new BMessage (M_LOG_CONTEXT_SAVE)));
}



MainMenu::~MainMenu ()
{
    delete m_archiveContextMenu;
    m_archiveContextMenu = NULL;

    delete m_logContextMenu;
    m_logContextMenu = NULL;
}



void MainMenu::SetRecentMenu (BMenu *menu)
{
    if (m_recentMenu != NULL)
        if (m_fileMenu->RemoveItem (m_recentMenu))
           delete m_recentMenu;

    m_recentMenu = menu;
    m_fileMenu->AddItem (m_recentMenu, 1);
    BMenuItem *openItem = m_fileMenu->FindItem (str (S_OPEN));
    openItem->SetMessage (new BMessage (M_FILE_OPEN));
    openItem->SetShortcut ('O', 0);

}



BMenu* MainMenu::RecentMenu () const
{
    return m_recentMenu;
}



void MainMenu::SetExtractPathsMenu (BMenu *menu)
{
    if (m_extractPathsMenu != NULL)
        if (m_actionsMenu->RemoveItem (m_extractPathsMenu))
           delete m_extractPathsMenu;

    m_extractPathsMenu = menu;
    m_actionsMenu->AddItem (m_extractPathsMenu, 1);
    BMenuItem *extractItem = m_actionsMenu->FindItem (str (S_EXTRACT_TO));
    extractItem->SetMessage (new BMessage (M_ACTIONS_EXTRACT_TO));
    extractItem->SetShortcut ('X', B_SHIFT_KEY);

    BMessage archiveMessage;
    menu->Archive (&archiveMessage, true);
    SetExtractSelPathsMenu (new BMenu (&archiveMessage));
}



void MainMenu::SetExtractSelPathsMenu (BMenu *menu)
{
    // Preserve enabled/disabled state
    if (m_extractSelPathsMenu != NULL)
        if (m_actionsMenu->RemoveItem (m_extractSelPathsMenu))
           delete m_extractSelPathsMenu;

    m_extractSelPathsMenu = menu;
    m_actionsMenu->AddItem (m_extractSelPathsMenu, 2);
    menu->Superitem()->SetLabel (str (S_EXTRACT_SELECTED));
    BMenuItem *extractItem = m_actionsMenu->FindItem (str (S_EXTRACT_SELECTED));
    if (extractItem)
    {
        extractItem->SetMessage (new BMessage (M_ACTIONS_EXTRACT_SELECTED));
        extractItem->SetShortcut ('S', B_SHIFT_KEY);
    }
}



BMenu* MainMenu::ExtractPathsMenu () const
{
    return m_extractPathsMenu;
}


