/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _COMMENT_WINDOW_H
#define _COMMENT_WINDOW_H

#include <Window.h>

class BTextView;
class BFont;

class BevelView;

class CommentWindow : public BWindow
{
    public:
        CommentWindow (BWindow *callerWindow, const char *archiveName,
                       const char *commentText, BFont *displayFont = NULL);
        
        // Inherited hooks
        virtual bool         QuitRequested ();
        virtual void         MessageReceived (BMessage *message);
        
    private:
        // Private members
        const char          *m_commentText;
        
        BevelView           *m_backView;
        BTextView           *m_textView;
        BButton             *m_closeButton,
                            *m_saveButton;
        BWindow             *m_callerWindow;
};

#endif /* _COMMENT_WINDOW_H */
