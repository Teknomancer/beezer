/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <TextView.h>
#include <Window.h>

#define K_SCROLL_DELAY        35000
#define M_CLOSE_ABOUT        'clab'

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
        // Private members
        float               m_curPos,
                            m_rightEdge;
};

class AboutWindow : public BWindow
{
    public:
        AboutWindow(const char* compileTimeStr);

        // Inherited hooks
        virtual void        DispatchMessage(BMessage* message, BHandler* handler);
        virtual void        Quit();

    private:
        // Thread functions
        static int32        _scroller(void* data);

        // Private members
        BView*              m_backView;
        BString             m_lineFeeds;
        thread_id           m_scrollThreadID;
        const char*         m_creditsText;

        MarqueeView*        m_textView;

};

#endif /* _ABOUT_WINDOW_H */
