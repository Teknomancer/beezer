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

#ifndef _IMAGE_BUTTON_H
#define _IMAGE_BUTTON_H

const float kContextWidth = 14;

enum textPosition
{
    kRightOfIcon,
    kBelowIcon
};

class BPopUpMenu;

class ImageButton : public BView
{
    public:
        ImageButton (BRect frame, const char *name, const char *text, BBitmap *bitmap, BBitmap *disabled,
                     BMessage *message, bool popUpMenu, const rgb_color backColor,
                     textPosition textPos = kBelowIcon, bool borders = true, bool smallFont = true,
                     bool hoverHighlight = true, uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
                     uint32 flags = B_WILL_DRAW);
        virtual ~ImageButton ();
        
        // Inherited hooks
        virtual void        Draw (BRect updateRect);
        virtual void        MouseMoved (BPoint point, uint32 status, const BMessage *dragInfo);
        virtual void        MouseDown (BPoint point);
        virtual void        MouseUp (BPoint point);
        virtual void        GetPreferredSize (float *width, float *height);
        virtual void        AttachedToWindow ();
        virtual void        WindowActivated (bool state);

        // Additional hooks
        virtual void        SetTarget (const BHandler *handler);
        virtual void        SetContextMenu (BPopUpMenu *contextMenu);
        virtual void        DrawContextMenuTriangle (BPoint topLeftPoint);
        virtual void        SetBorderPadding (float horizontal, float vertical);
        virtual void        GetBorderPadding (float *horizontal, float *vertical);
        virtual void        SetEnabled (bool enable);
        virtual bool        IsEnabled () const;
        virtual void        SetMessage (BMessage *msg);
        virtual BMessage    *Message () const;
        virtual void        SetClickable (bool clickable);
        virtual bool        IsClickable () const;
        virtual status_t    SetMargin (float width, float height);
        virtual void        GetMargin (float *width, float *height);
        
    protected:
        // Protected hooks
        virtual void        DrawOutsideEdge (BRect rect);
        virtual void        DrawShinyEdge (BRect rect, bool isPressing);
        virtual void        PushButton (BRect rect);
        virtual void        ShowContextMenu (BPoint point);
        virtual void        HighlightNow (bool isPressing);

        // Protected members
        const char         *m_buttonText;
        BHandler           *m_handler;
        BBitmap            *m_clickBitmap,
                           *m_disabledBitmap;
        BMessage           *m_clickMessage;
        BPopUpMenu         *m_contextMenu;
        rgb_color           m_backColor,
                            m_darkEdge1,
                            m_darkEdge2,
                            m_lightEdge,
                            m_lightEdge2;
        bool                m_drawingTriangle,
                            m_popUpMenu,
                            m_firstClick,
                            m_isPushed,
                            m_isEnabled,
                            m_isClickable,
                            m_borders,
                            m_smallFont,
                            m_hoverHighlight,
                            m_mouseInside,
                            m_globalMouse;
        int                 m_textPosition;
        float               m_fontPlacement,
                            m_marginWidth,
                            m_marginHeight;
};

#endif /* _IMAGE_BUTTON_H */
