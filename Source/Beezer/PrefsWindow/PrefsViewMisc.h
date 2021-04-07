// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PREFS_VIEW_MISC_H
#define _PREFS_VIEW_MISC_H

#include "PrefsView.h"

#include <List.h>

class BPopUpMenu;
class BMenuField;

class PrefsViewMisc : public PrefsView
{
    public:
        PrefsViewMisc(BRect frame);

        // Inherited hooks
        virtual void        AttachedToWindow();
        virtual void        MessageReceived(BMessage* message);

        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        BCheckBox*          m_commentChk,
                            *m_mimeChk;
        BButton*            m_mimeBtn;
        BPopUpMenu*         m_arkTypePopUp,
                            *m_quitPopUp,
                            *m_startupPopUp;
        BMenuField*         m_arkTypeField,
                            *m_quitField,
                            *m_startupField;
        BList               m_arkTypes;
};

#endif /* _PREFS_VIEW_MISC_H */
