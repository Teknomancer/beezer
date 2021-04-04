// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include <MenuBar.h>

class BitmapMenuItem;

// these menu titles are shared with MainWindow.cpp and must be consistent so
// that it can find the menus to enable/disable them
#define S_EXTRACT_TO "Extract to…"
#define S_EXTRACT_SELECTED "Extract selected…"
#define S_OPEN "Open…"


class MainMenu : public BMenuBar
{
    public:
        MainMenu(BRect frame);
        virtual ~MainMenu();

        // Public hooks
        void                SetRecentMenu(BMenu* menu);
        void                SetExtractPathsMenu(BMenu* menu);
        void                SetExtractSelPathsMenu(BMenu* menu);
        BMenu*              RecentMenu() const;
        BMenu*              ExtractPathsMenu() const;

        // Public members
        BMenu*              m_fileMenu,
                            *m_recentMenu,
                            *m_extractPathsMenu,
                            *m_extractSelPathsMenu,
                            *m_editMenu,
                            *m_selectAllMenu,
                            *m_actionsMenu,
                            *m_columnsSubMenu,
                            *m_sortBySubMenu,
                            *m_sortOrderSubMenu,
                            *m_foldingMenu,
                            *m_toolsMenu,
                            *m_windowsMenu,
                            *m_viewMenu;
        BPopUpMenu*         m_archiveContextMenu,
                            *m_logContextMenu;
        BitmapMenuItem*     m_systemMenu;
};



#endif /* _MAIN_MENU_H */
