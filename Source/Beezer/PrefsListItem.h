/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _PREFS_LIST_ITEM_H
#define _PREFS_LIST_ITEM_H

#include <ListItem.h>
#include <InterfaceDefs.h>

class BView;

class PrefsListItem : public BStringItem
{
    public:
        PrefsListItem (const char *text, BBitmap *bmp, bool makeBold = true);
        virtual ~PrefsListItem ();
        
        // Inherited hooks
        virtual void        DrawItem (BView *owner, BRect frame, bool complete = false);
        virtual float        FontHeight () const;
        
    private:
        // Private members
        float                m_fontDelta;
        bool                m_makeBold;
        const char            *m_label;
        BBitmap                *m_bitmap;
        rgb_color            m_selBackColor,
                            m_selTextColor;
};

#endif    /* _PREFS_LIST_ITEM_H */
