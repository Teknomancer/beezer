/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include <MenuBar.h>
#include <PopUpMenu.h>

#include "BitmapMenuItem.h"

//=============================================================================================================//

class MainMenu : public BMenuBar
{
	public:
		MainMenu (BRect frame);
		virtual ~MainMenu ();
		
		// Public hooks
		void				SetRecentMenu (BMenu *menu);
		void				SetExtractPathsMenu (BMenu *menu);
		void				SetExtractSelPathsMenu (BMenu *menu);
		BMenu				*RecentMenu () const;
		BMenu				*ExtractPathsMenu () const;
		
		// Public members
		BMenu				*m_fileMenu,
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
							*m_settingsMenu;
		BPopUpMenu			*m_archiveContextMenu,
							*m_logContextMenu;
		BitmapMenuItem		*m_systemMenu;
};

//=============================================================================================================//

#endif /* _MAIN_MENU_H */
