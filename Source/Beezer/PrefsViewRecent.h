/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _PREFS_VIEW_RECENT_H
#define _PREFS_VIEW_RECENT_H

#include "PrefsView.h"

class PrefsViewRecent : public PrefsView
{
    public:
        PrefsViewRecent (BRect frame);
        
        // Inherited hooks
        virtual void        Render ();
        virtual void        Save ();
        virtual void        Load ();
    
    private:
        // Private members
        BCheckBox          *m_showPathChk;
        BTextControl        *m_recentArkView,
                         *m_recentExtView;

};

#endif /* _PREFS_VIEW_RECENT_H */
