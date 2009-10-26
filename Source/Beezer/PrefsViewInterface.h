/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _PREFS_VIEW_Interface_H
#define _PREFS_VIEW_Interface_H

#include "PrefsView.h"

class BColorControl;

class PrefsViewInterface : public PrefsView
{
    public:
        PrefsViewInterface (BRect frame);
        
        // Inherited hooks
        virtual void        MessageReceived (BMessage *message);
        virtual void        AttachedToWindow ();
        
        virtual void        Render ();
        virtual void        Save ();
        virtual void        Load ();
        
    private:
        // Private hooks
        void                UpdateColorWell ();
        void                UpdateColorControl (BMenuItem *item);
        bool                FindBoolDef (BMessage *msg, const char *name, bool defaultValue);
        
        // Private members
        BCheckBox            *m_fullLengthBarsChk,
                            *m_toolbarChk,
                            *m_infobarChk,
                            *m_actionLogChk;
        BView                *m_colorWell;
        BPopUpMenu            *m_colorPopUp,
                            *m_foldingPopUp;
        BMenuField            *m_colorField,
                            *m_foldingField;
        BColorControl        *m_colorControl;
        rgb_color            m_actFore,
                            m_actBack;
};

#endif /* _PREFS_VIEW_EXTRACT_H */
