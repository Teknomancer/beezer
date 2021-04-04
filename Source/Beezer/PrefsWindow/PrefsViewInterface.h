// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFS_VIEW_INTERFACE_H
#define _PREFS_VIEW_INTERFACE_H

#include "PrefsView.h"

class BColorControl;
class BMenuItem;

class PrefsViewInterface : public PrefsView
{
    public:
        PrefsViewInterface(BRect frame);

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);
        virtual void        AttachedToWindow();

        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        // Private hooks
        void                UpdateColorWell();
        void                UpdateColorControl(BMenuItem* item);
        bool                FindBoolDef(BMessage* msg, const char* name, bool defaultValue);

        // Private members
        BCheckBox*          m_fullLengthBarsChk,
                            *m_toolbarChk,
                            *m_infobarChk,
                            *m_actionLogChk;
        BView*              m_colorWell;
        BPopUpMenu*         m_colorPopUp,
                            *m_foldingPopUp;
        BMenuField*         m_colorField,
                            *m_foldingField;
        BColorControl*       m_colorControl;
        rgb_color           m_actFore,
                            m_actBack;
};

#endif /* _PREFS_VIEW_INTERFACE_H */
