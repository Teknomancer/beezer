/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _LOG_WINDOW_H
#define _LOG_WINDOW_H

#include <Window.h>

class BTextView;
class BFont;

class BevelView;

class BetterScrollView;

class LogWindow : public BWindow
{
    public:
        LogWindow (BWindow *callerWindow, const char *title, const char *logText, BFont *displayFont = NULL);
        
        // Inherited hooks
        virtual bool         QuitRequested ();
        virtual void         FrameResized (float newWidth, float newHeight);
        
    private:
        // Private members
        BevelView           *m_backView;
        BTextView           *m_textView;
        BButton             *m_closeButton;
        BetterScrollView    *m_scrollView;
        float                m_maxLineWidth;
};

#endif /* _LOG_WINDOW_H */
