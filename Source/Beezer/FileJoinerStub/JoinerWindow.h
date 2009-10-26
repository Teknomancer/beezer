/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

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

const char *const kResult =        "result";

class JoinerWindow : public BWindow
{
    public:
        JoinerWindow ();
        
        // Inherited hooks
        virtual bool        QuitRequested ();
        virtual void        MessageReceived (BMessage *message);
        
    private:
        // Private hooks
        status_t            ReadSelf ();
        static int32        _joiner (void *arg);
        
        // Private members
        BevelView            *m_backView;
        BStatusBar            *m_statusBar;
        BButton                *m_cancelBtn;
        BString                m_separatorStr,
                            m_chunkPathStr,
                            m_dirPathStr;
        volatile bool        m_cancel;
        bool                m_joinInProgress;
        BMessenger            *m_messenger;
        thread_id            m_thread;
};

#endif /* _FILE_JOINER_STUB_H */
