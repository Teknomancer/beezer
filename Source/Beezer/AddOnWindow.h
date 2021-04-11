// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ADDON_WINDOW_H
#define _ADDON_WINDOW_H

#include <Window.h>

class BButton;
class BTextControl;
class BMenuField;
class BPopUpMenu;
class BStringView;

class ArchiveRep;
class BarberPole;
class BevelView;

const uint32 M_CLOSE_ADDON       = 'adtx';
const uint32 M_FILENAME_CHANGED  = 'fnch';

class AddOnWindow : public BWindow
{
    public:
        AddOnWindow(BMessage* refsMessage);

        // Inherited hooks
        virtual bool        QuitRequested();
        virtual void        MessageReceived(BMessage* message);
        virtual void        Quit();

    private:
        bool                ReplaceExtensionWith(const char* newExt);
        void                UpdateStatus(const char* text);
        void                ValidateData();
        void                CreateArchiveRepAndMenus();
        void                EnableControls(bool enable);
        void                RefsReceived(BMessage* message);

        BevelView*          m_backView,
                            *m_backViewMain,
                            *m_backViewAlt,
                            *m_addView;
        BTextControl*        m_fileName,
                             *m_password;
        BMenuField*         m_arkTypeField,
                            *m_arkSettingsMenuField;
        BPopUpMenu*         m_arkTypePopUp;
        BList               m_arkTypes,
                            m_arkExtensions;
        BButton*            m_helpBtn,
                            *m_createBtn;
        BStringView*        m_statusStr,
                            *m_addingFileStr;
        BarberPole*         m_barberPole;
        BString             m_archiveDirPath;
        ArchiveRep*         m_archive;
        BMessage            m_refsMessage;

        bool                m_readyMode,
                            m_inProgress,
                            m_quitNow;
        rgb_color           m_statusColor;
        volatile bool        m_cancel;
        float               m_strWidthOfArkTypes;
        thread_id           m_thread;
};

#endif /* _ADDON_WINDOW_H */
