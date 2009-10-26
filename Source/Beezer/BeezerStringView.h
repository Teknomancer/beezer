/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
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
