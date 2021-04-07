// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PREFS_VIEW_EXTRACT_H
#define _PREFS_VIEW_EXTRACT_H

#include "PrefsView.h"

class PrefsViewExtract : public PrefsView
{
    public:
        PrefsViewExtract(BRect frame);

        // Inherited hooks
        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        BCheckBox*          m_openFolderChk,
                            *m_closeChk,
                            *m_quitChk,
                            *m_dragChk;
};

#endif /* _PREFS_VIEW_EXTRACT_H */
