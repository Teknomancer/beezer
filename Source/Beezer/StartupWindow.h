// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _STARTUP_WINDOW_H
#define _STARTUP_WINDOW_H

#include <Window.h>

class BBitmap;
class BPopUpMenu;
class BStringView;

class BevelView;
class ImageButton;
class RecentMgr;

class StartupWindow : public BWindow
{
    public:
        StartupWindow(RecentMgr* recentMgr, bool startup);

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);
        virtual void        Quit();
        virtual bool        QuitRequested();

    private:
        BStringView*        m_headingView;
        ImageButton*        m_createBtn,
                            *m_openBtn,
                            *m_openRecentBtn,
                            *m_toolsBtn,
                            *m_prefsBtn,
                            *m_helpBtn;
        RecentMgr*          m_recentMgr;
        BPopUpMenu*         m_recentMenu,
                            *m_toolsMenu;
};

#endif /* _STARTUP_WINDOW_H */
