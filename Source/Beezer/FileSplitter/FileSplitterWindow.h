// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _FILE_SPLITTER_WINDOW_H
#define _FILE_SPLITTER_WINDOW_H

#include <Entry.h>
#include <Directory.h>
#include <String.h>
#include <Window.h>

class BButton;
class BCheckBox;
class BFilePanel;
class BMenu;
class BMenuField;
class BPopUpMenu;
class BStatusBar;
class BStringView;
class BTextControl;

class BevelView;
class SelectDirPanel;
class RecentMgr;

class FileSplitterWindow : public BWindow
{
    public:
        FileSplitterWindow(RecentMgr* files, RecentMgr* dirs);
        virtual ~FileSplitterWindow();

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);
        virtual void        Quit();
        virtual bool        QuitRequested();

    private:
        // Private hooks
        void                ToggleWindowHeight(bool expand);
        void                UpdateData();
        void                UpdateRecentMenus();
        void                CreateSelfJoiner();

        // Thread functions
        static int32        _splitter(void* data);

        // Private members
        BevelView*          m_backView;
        BMenu*              m_fileMenu,
                            *m_folderMenu;
        BMenuField*         m_prefixField;
        BPopUpMenu*         m_sizePopUp,
                            *m_prefixPopUp;
        BTextControl*       m_filePathView,
                            *m_folderPathView,
                            *m_customSizeView,
                            *m_separatorView;
        BButton*            m_splitBtn;
        BStringView*        m_descStr,
                            *m_descStr2,
                            *m_piecesStr,
                            *m_sizeStr;
        BCheckBox*          m_createChk,
                            *m_openDirChk,
                            *m_closeChk;
        BStatusBar*         m_statusBar;

        SelectDirPanel*     m_dirPanel;
        BFilePanel*         m_filePanel;

        float               m_hideProgress,
                            m_showProgress;

        BEntry              m_fileEntry;
        BDirectory          m_destDir;
        BString             m_firstChunkName;

        uint64              m_fragmentSize;
        uint16              m_fragmentCount;
        char*               m_sepString;
        volatile bool       m_cancel;
        bool                m_splitInProgress,
                            m_quitNow;
        BMessenger*         m_messenger;
        thread_id           m_thread;

        RecentMgr*          m_recentSplitFiles,
                            *m_recentSplitDirs;
};

#endif /* _FILE_SPLITTER_WINDOW_H */
