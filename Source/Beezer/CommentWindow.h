// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _COMMENT_WINDOW_H
#define _COMMENT_WINDOW_H

#include <Window.h>

class BFont;
class BTextView;

class CommentWindow : public BWindow
{
    public:
        CommentWindow(BWindow* callerWindow, const char* archiveName,
                      const char* commentText, BFont* displayFont = NULL);

        // Inherited hooks
        virtual bool        QuitRequested();
        virtual void        MessageReceived(BMessage* message);

    private:
        BWindow*            m_callerWindow;
        BTextView*          m_textView;
};

#endif /* _COMMENT_WINDOW_H */
