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

const uint32 M_CLOSE_PREFS = 'clpf';

class PrefsWindow : public BWindow
{
    public:
        PrefsWindow();
        virtual ~PrefsWindow();

        // Inherited hooks
        virtual void        Quit();
        virtual void        MessageReceived(BMessage* message);

    private:
        void                AddControls(BRect *panelFrame);
        void                AddPanels();
        void                SetActivePanel(PrefsView* panel);

        BList               m_panelList;
        BFont*              m_panelTitleFont;
        PrefsView*          m_currentPanel;
        BevelView*          m_backView;
        BListView*          m_listView;
        BTextView*          m_descTextView;
};

#endif /* _PREFS_WINDOW_H */
