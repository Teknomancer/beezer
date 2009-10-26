/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _PREFS_VIEW_WINDOWS_H
#define _PREFS_VIEW_WINDOWS_H

#include "PrefsView.h"

class PrefsViewWindows : public PrefsView
{
    public:
        PrefsViewWindows (BRect frame);
        
        // Inherited hooks
        virtual void        Render ();
        virtual void        Save ();
        virtual void        Load ();
    
    private:
        // Private members
        BCheckBox          *m_searchChk,
                         *m_logChk,
                         *m_commentChk,
                         *m_arkInfoChk,
                         *m_welcomeChk,
                         *m_prefsChk;
};

#endif /* _PREFS_VIEW_WINDOWS_H */
