/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _BITMAP_MENU_ITEM_H
#define _BITMAP_MENU_ITEM_H

#include <Bitmap.h>
#include <MenuItem.h>

class BitmapMenuItem : public BMenuItem
{
    public:
        BitmapMenuItem (BMenu *subMenu, BBitmap *bitmap);
        
        // Inherited hooks
        virtual void        DrawContent ();
        virtual void        GetContentSize (float *width, float *height);
        
    private:
        // Private members
        BBitmap            *m_bitmap;
};

#endif /* _BITMAP_MENU_ITEM_H */
