// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PREFS_VIEW_STATE_H
#define _PREFS_VIEW_STATE_H

#include "PrefsView.h"

class PrefsViewState : public PrefsView
{
    public:
        PrefsViewState(BRect frame);

        // Inherited hooks
        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        // Private members
        BCheckBox*          m_storeUIChk,
                            *m_restoreUIChk,
                            *m_storeArkChk,
                            *m_restoreArkChk;
};

#endif /* _PREFS_VIEW_STATE_H */
