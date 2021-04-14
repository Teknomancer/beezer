// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _FILE_JOINER_WINDOW_H
#define _FILE_JOINER_WINDOW_H

#include <String.h> // gcc2 - forward declaration wouldn't work
#include <Window.h>

class BStringView;
class BTextControl;
class BMenu;
class BMenuField;
class BButton;
class BFilePanel;
class BCheckBox;
class BStatusBar;
class BEntry; // gcc2

class BevelView;
class SelectDirPanel;
class RecentMgr;

class FileJoinerWindow : public BWindow
{
    public:
        FileJoinerWindow(RecentMgr* dirs);
        virtual ~FileJoinerWindow();

        // Inherited hooks
        virtual void        Quit();
        virtual bool        QuitRequested();
        virtual void        MessageReceived(BMessage* message);

    private:
        // Threads
        static int32        _calcsize(void* arg);
        static int32        _joiner(void* arg);

        void                UpdateData();
        void                UpdateRecentMenus();
        void                RefreshInfo();
        void                GetDirectoryInfo(BEntry* srcDir, int32& fileCount,
                                             off_t& totalSize, volatile bool* cancel);
        void                DeleteChunks(const char* firstChunkPathStr, const char* separator);
        void                ToggleWindowHeight(bool expand);

        BevelView*          m_backView,
                            *m_innerView;
        BStringView*        m_descStr,
                            *m_descStr2,
                            *m_piecesStr,
                            *m_sizeStr;
        BTextControl*       m_filePathView,
                            *m_folderPathView,
                            *m_separatorView;
        BMenu*              m_folderMenu;
        BMenuField*         m_folderField;
        BButton*            m_selectFileBtn,
                            *m_selectFolderBtn,
                            *m_joinBtn,
                            *m_refreshBtn;
        BFilePanel*         m_filePanel;
        SelectDirPanel*     m_dirPanel;
        BCheckBox*          m_openChk,
                            *m_deleteChk,
                            *m_closeChk;
        bool                m_calcSize,
                            m_joinInProgress,
                            m_quitNow;
        thread_id           m_thread;
        volatile bool       m_cancel;
        BString             m_separatorStr,
                            m_chunkPathStr,
                            m_dirPathStr;
        uint16              m_noOfPieces;
        off_t               m_totalSize;
        BStatusBar*         m_statusBar;
        float               m_hideProgress,
                            m_showProgress;

        BMessenger*         m_messenger;
        RecentMgr*          m_recentSplitDirs;    // yes split is used for both !
};

#endif /* _FILE_JOINER_WINDOW_H */
