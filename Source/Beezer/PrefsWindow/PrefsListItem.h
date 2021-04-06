// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFS_LIST_ITEM_H
#define _PREFS_LIST_ITEM_H

#include <GraphicsDefs.h>
#include <StringItem.h>

class BView;
class BBitmap;

class PrefsListItem : public BStringItem
{
    public:
        PrefsListItem(const char* text, BBitmap* bmp, bool makeBold = true);
        virtual ~PrefsListItem();

        // Inherited hooks
        virtual void        DrawItem(BView* owner, BRect frame, bool complete = false);
        virtual float       FontHeight() const;

    private:
        float               m_fontDelta;
        bool                m_makeBold;
        BBitmap*            m_bitmap;
        rgb_color           m_selBackColor,
                            m_selTextColor;
};

#endif /* _PREFS_LIST_ITEM_H */
