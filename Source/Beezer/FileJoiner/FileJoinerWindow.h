/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FILE_JOINER_WINDOW_H
#define _FILE_JOINER_WINDOW_H

#include <Window.h>

class BStringView;
class BTextControl;
class BMenu;
class BMenuField;
class BButton;
class BFilePanel;
class BCheckBox;
class BStatusBar;

class BevelView;
class SelectDirPanel;
class RecentMgr;

#define M_SELECT_JOIN_FILE               'sljf'
#define M_SELECT_JOIN_FOLDER             'sldr'
#define M_JOIN_FILE_SELECTED             'jfsl'
#define M_JOIN_FOLDER_SELECTED           'jfsd'
#define M_UPDATE_DATA                    'updd'
#define M_SEPARATOR_CHANGED              'spch'
#define M_CALC_COMPLETE                  'calc'
#define M_REFRESH_INFO                   'reff'
#define M_JOIN_NOW                       'join'
#define M_OPERATION_COMPLETE             'opcc'

class FileJoinerWindow : public BWindow
{
    public:
        FileJoinerWindow (RecentMgr *dirs);
        virtual ~FileJoinerWindow ();

        // Inherited hooks
        virtual void        Quit ();
        virtual bool        QuitRequested ();
        virtual void        MessageReceived (BMessage *message);

    private:
        // Private threads
        static int32        _calcsize (void *arg);
        static int32        _joiner (void *arg);

        // Private hooks
        void                UpdateData ();
        void                UpdateRecentMenus ();
        void                RefreshInfo ();
        void                GetDirectoryInfo (BEntry *srcDir, int32 &fileCount,
                                              off_t &totalSize, volatile bool *cancel);
        void                DeleteChunks (const char *firstChunkPathStr, const char *separator);
        void                ToggleWindowHeight (bool expand);
        
        // Private members
        BevelView          *m_backView,
                           *m_innerView;
        BStringView        *m_descStr,
                           *m_descStr2,
                           *m_piecesStr,
                           *m_sizeStr;
        BTextControl       *m_filePathView,
                           *m_folderPathView,
                           *m_separatorView;
        BMenu              *m_folderMenu;
        BMenuField         *m_folderField;
        BButton            *m_selectFileBtn,
                           *m_selectFolderBtn,
                           *m_joinBtn,
                           *m_refreshBtn;
        BFilePanel         *m_filePanel;
        SelectDirPanel     *m_dirPanel;
        BCheckBox          *m_openChk,
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
        BStatusBar         *m_statusBar;
        float               m_hideProgress,
                            m_showProgress;

        BMessenger         *m_messenger;
        RecentMgr          *m_recentSplitDirs;    // yes split is used for both !
};

#endif /* _FILE_JOINER_WINDOW_H */
