// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _JOINER_WINDOW_H
#define _JOINER_WINDOW_H

#include <Window.h>
#include <String.h>

class BevelView;

class BStatusBar;
class BButton;
class BMessenger;

#define M_CANCEL                'canc'
#define M_OPERATION_COMPLETE    'opcc'

class JoinerWindow : public BWindow
{
    public:
        JoinerWindow();

        // Inherited hooks
        virtual bool        QuitRequested();
        virtual void        MessageReceived(BMessage* message);

    private:
        // Thread functions
        static int32        _joiner(void* arg);

        // Static data
        static const char*  kJoinResult;

        status_t            ReadSelf();

        BevelView*          m_backView;
        BStatusBar*         m_statusBar;
        BButton*            m_cancelBtn;
        BString             m_separatorStr,
                            m_chunkPathStr,
                            m_dirPathStr;
        volatile bool       m_cancel;
        BMessenger*         m_messenger;
        thread_id           m_thread;
};

#endif /* _JOINER_WINDOW_H */
