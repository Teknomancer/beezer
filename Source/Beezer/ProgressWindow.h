// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PROGRESS_WINDOW_H
#define _PROGRESS_WINDOW_H

#include <Window.h>

class BButton;
class BMessenger;
class BStatusBar;

class BarberPole;

class ProgressWindow : public BWindow
{
    public:
        ProgressWindow(BWindow* callerWindow, BMessage* actionMessage,
                       BMessenger*& messenger, volatile bool*& cancel);
        virtual ~ProgressWindow();

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);

    protected:
        BarberPole*         m_barberPole;
        BStatusBar*         m_statusBar;
        BButton*            m_cancelButton;
        int32               m_fileCount,
                            m_progressCount;
        char                m_updateText[B_PATH_NAME_LENGTH + 1];
        bool volatile       m_cancel;
        BMessenger*         m_messenger;
};

#endif /* _PROGRESS_WINDOW_H */
