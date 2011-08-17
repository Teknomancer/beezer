/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _BEEZER_STRING_VIEW_H
#define _BEEZER_STRING_VIEW_H

#include <View.h>

class BBitmap;

class BeezerStringView : public BView
{
    public:
        BeezerStringView (BRect frame, const char *name, const char *label, uint32 rmask = B_FOLLOW_LEFT,
                        uint32 flags = B_WILL_DRAW);
        virtual ~BeezerStringView ();
        
        // Inherited hooks
        virtual void        MouseDown (BPoint point);
        virtual void        Draw (BRect frame);
        virtual void        GetPreferredSize (float *width, float *height);
        virtual void        Invalidate (BRect rect);
        virtual void        AttachedToWindow ();
        virtual void        SetHighColor (rgb_color a_color);
        virtual void        SetHighColor (uchar r, uchar g, uchar b, uchar a = 255);
        virtual void        SetFont (const BFont *font, uint32 mask = B_FONT_ALL);
        
        // Additional hooks
        virtual void        SetText (const char *text);
        virtual const char  *Text () const;
        virtual void        SendMouseEventsTo (BView *targetView);
        
    protected:
        // Protected hooks
        virtual void        RenderView (bool firstTime = false);
        
        // Data members
        BView              *m_backView,
                         *m_mouseTargetView;
        BBitmap            *m_backBitmap;
        BFont               m_font;
        char               *m_text,
                         *m_oldText;
        bool                m_selected;
        rgb_color           m_foreground,
                          m_background;
};

#endif    /* _BEEZER_STRING_VIEW_H */
