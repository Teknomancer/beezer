// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFS_VIEW_H
#define _PREFS_VIEW_H

#include "BevelView.h"

#define M_REVERT           'pfrv'

class BBitmap;
class BButton;
class BCheckBox;

class PrefsView : public BevelView
{
    public:
        PrefsView(BRect frame, const char* title, const char* description);
        virtual ~PrefsView();

        // Additional hooks
        const char*         Description() const;
        const char*         Title() const;
        virtual bool        IsChecked(BCheckBox* chkBox) const;
        virtual void        Render();
        virtual void        Save();
        virtual void        Load();
        virtual void        SetBitmap(BBitmap* bmp);
        virtual BBitmap*     Bitmap() const;

    protected:
        // Protected hooks
        void                AddRevertButton();

        // Protected members
        BButton*            m_revertBtn;
        float               m_margin;
        float               m_vGap;
        BFont               m_sectionFont;

    private:
        // Private hooks
        void                DeleteBitmap();

        // Private members
        const char*         m_titleStr;
        const char*         m_descriptionStr;
        BBitmap*            m_bitmap;
};

#endif /* _PREFS_VIEW_H */
