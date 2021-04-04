// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _PREFS_VIEW_PATHS_H
#define _PREFS_VIEW_PATHS_H

#include "PrefsView.h"

class BFilePanel;
class BRadioButton;
class BTextControl;

class ImageButton;

class PrefsViewPaths : public PrefsView
{
    public:
        PrefsViewPaths(BRect frame);
        virtual ~PrefsViewPaths();

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);
        virtual void        AttachedToWindow();

        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

        // Additional hooks
        virtual void        ToggleExtractPathView(bool enable);

    private:
        // Private members
        BMessage*           m_message;
        BMessenger*         m_messenger;
        BFilePanel*         m_openPanel;
        BTextControl*       m_openPathView,
                            *m_addPathView,
                            *m_extractPathView;
        BButton*            m_openPathBtn,
                            *m_addPathBtn,
                            *m_extractPathBtn;
        BRadioButton*       m_arkDirOpt,
                            *m_useDirOpt;
        BCheckBox*          m_genChk;
        BListView*          m_favListView;
        BScrollView*        m_scrollView;
        ImageButton*        m_addBtn,
                            *m_removeBtn;
        BBitmap*            m_addBmp,
                            *m_removeBmp;
};

#endif /* _PREFS_VIEW_PATHS_H */
