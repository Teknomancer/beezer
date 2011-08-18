/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include <MenuBar.h>

class BitmapMenuItem;

//=============================================================================================================//

class MainMenu : public BMenuBar
{
    public:
        MainMenu (BRect frame);
        virtual ~MainMenu ();

        // Public hooks
        void                SetRecentMenu (BMenu *menu);
        void                SetExtractPathsMenu (BMenu *menu);
        void                SetExtractSelPathsMenu (BMenu *menu);
        BMenu              *RecentMenu () const;
        BMenu              *ExtractPathsMenu () const;

        // Public members
        BMenu              *m_fileMenu,
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
        BPopUpMenu         *m_archiveContextMenu,
                           *m_logContextMenu;
        BitmapMenuItem     *m_systemMenu;
};

//=============================================================================================================//

#endif /* _MAIN_MENU_H */
