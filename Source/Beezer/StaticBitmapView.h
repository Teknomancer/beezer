/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _STATIC_BITMAP_VIEW_H
#define _STATIC_BITMAP_VIEW_H

#include <View.h>

class BBitmap;

class StaticBitmapView : public BView
{
    public:
        StaticBitmapView (BRect frame, const char *name, BBitmap *bmp,
                          uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
                          uint32 flags = B_WILL_DRAW);
        virtual ~StaticBitmapView ();
        
        virtual void        Draw (BRect updateRect);
        virtual void        AttachedToWindow();
        
    private:
        BBitmap            *m_bitmap;
};

#endif /* _STATIC_BITMAP_VIEW_H */
