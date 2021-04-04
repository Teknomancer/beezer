// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFS_VIEW_ADD_H
#define _PREFS_VIEW_ADD_H

#include "PrefsView.h"

class BMenuField;
class BPopUpMenu;
class BTextControl;

class PrefsViewAdd : public PrefsView
{
    public:
        PrefsViewAdd(BRect frame);

        // Inherited hooks
        virtual void        AttachedToWindow();
        virtual void        MessageReceived(BMessage* message);

        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        // Private hook
        void                ToggleMBView(bool enable);

        // Private members
        BMenuField*         m_replaceField;
        BPopUpMenu*         m_replaceMenu;
        BCheckBox*          m_warnMBChk,
                            *m_sortChk,
                            *m_dropChk;
        BTextControl*       m_mbView;
};

#endif /* _PREFS_VIEW_ADD_H */
