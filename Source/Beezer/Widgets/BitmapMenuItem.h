// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _BITMAP_MENU_ITEM_H
#define _BITMAP_MENU_ITEM_H

#include <MenuItem.h>

class BBitmap;

class BitmapMenuItem : public BMenuItem
{
    public:
        BitmapMenuItem(BMenu* subMenu, BBitmap* bitmap);

        // Inherited hooks
        virtual void        DrawContent();
        virtual void        GetContentSize(float* width, float* height);

    private:
        // Private members
        BBitmap*            m_bitmap;
};

#endif /* _BITMAP_MENU_ITEM_H */
