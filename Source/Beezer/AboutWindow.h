// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <String.h> // gcc2 - forward declaration wouldn't work
#include <TextView.h>
#include <Window.h>


static const uint32 M_CLOSE_ABOUT = 'clab';

class MarqueeView : public BTextView
{
    public:
        MarqueeView(BRect frame, const char* name, BRect textRect, uint32 resizeMask,
                    uint32 flags = B_WILL_DRAW);
        MarqueeView(BRect frame, const char* name, BRect textRect, const BFont* initialFont,
                    const rgb_color* initialColor, uint32 resizeMask, uint32 flags);

        // Inherited hooks
        void                ScrollBy(float dh, float dv);
        void                ScrollTo(float x, float y);

    private:
        float               m_curPos,
                            m_rightEdge;
};


class AboutWindow : public BWindow
{
    public:
        AboutWindow(const char* versionStr, const char* compileTimeStr);

        // Inherited hooks
        virtual void        DispatchMessage(BMessage* message, BHandler* handler);
        virtual void        Quit();

    private:
        // Thread functions
        static int32        _scroller(void* data);

        BView*              m_backView;
        BString             m_lineFeeds;
        thread_id           m_scrollThreadId;

        MarqueeView*        m_textView;

};

#endif /* _ABOUT_WINDOW_H */
