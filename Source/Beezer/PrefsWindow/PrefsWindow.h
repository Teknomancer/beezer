// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PREFS_WINDOW_H
#define _PREFS_WINDOW_H

#include <Window.h>

class BListView;
class BTextView;

class BevelView;
class PrefsView;

#define M_CLOSE_PREFS              'clpf'
#define M_SAVE_PREFS               'svpf'
#define M_PREFS_PANEL_SELECTED      'pspf'

class PrefsWindow : public BWindow
{
    public:
        PrefsWindow();

        // Inherited hooks
        virtual void        Quit();
        virtual void        MessageReceived(BMessage* message);

    private:
        // Private hooks
        void                AddControls();
        void                SetActivePanel(PrefsView* panel);

        // Private members
        BRect               m_panelFrame;
        BList               m_panelList;
        PrefsView*          m_currentPanel;
        BevelView*          m_backView;
        BListView*          m_listView;
        BTextView*          m_descTextView;
};

#endif /* _PREFS_WINDOW_H */
