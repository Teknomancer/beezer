/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <Window.h>
#include <TextView.h>

#include "AppConstants.h"
#include "LangStrings.h"

#define K_SCROLL_DELAY        35000
#define M_CLOSE_ABOUT        'clab'

class MarqueeView : public BTextView
{
    public:
        MarqueeView (BRect frame, const char *name, BRect textRect, uint32 resizeMask,
                uint32 flags = B_WILL_DRAW);
        MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
                const rgb_color *initialColor, uint32 resizeMask, uint32 flags);
    
        // Inherited hooks
        void                ScrollBy (float dh, float dv);
        void                ScrollTo (float x, float y);
    
    private:    
        // Private members
        float                m_curPos,
                            m_rightEdge;
};

class AboutWindow : public BWindow
{
    public:
        AboutWindow (const char *compileTimeStr);
        
        // Inherited hooks
        virtual void        DispatchMessage (BMessage *message, BHandler *handler);
        virtual void        Quit ();
                
    private:
        // Thread functions
        static int32        _scroller (void *data);

        // Private members
        BView                *m_backView;
        BString                m_lineFeeds;
        thread_id            m_scrollThreadID;
        const char            *m_creditsText;

        MarqueeView            *m_textView;
        
};

#endif /* _ABOUT_WINDOW_H */
