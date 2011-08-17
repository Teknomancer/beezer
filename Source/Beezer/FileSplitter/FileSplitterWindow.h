/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#ifndef _FILE_SPLITTER_WINDOW_H
#define _FILE_SPLITTER_WINDOW_H

#include <Window.h>
#include <String.h>

class BStringView;
class BCheckBox;
class BTextControl;
class BStatusBar;
class BString;

class BevelView;
class SelectDirPanel;
class RecentMgr;

#define M_SPLIT_NOW                      'splt'
#define M_SELECT_SPLIT_FILE               'file'
#define M_SELECT_SPLIT_FOLDER             'fold'
#define M_SPLIT_FILE_SELECTED             'fise'
#define M_SPLIT_FOLDER_SELECTED           'flse'
#define M_CUSTOM_SIZE                    'cuss'
#define M_PREDEFINED_SIZE                'pres'
#define M_UPDATE_DATA                    'updd'
#define M_OPERATION_COMPLETE              'opcc'
#define M_SEPARATOR_CHANGED               'spch'

#define K_WINDOW                        "split_wnd"

class FileSplitterWindow : public BWindow
{
    public:
        FileSplitterWindow (RecentMgr *files, RecentMgr *dirs);
        virtual ~FileSplitterWindow ();

        // Inherited hooks
        virtual void        MessageReceived (BMessage *message);
        virtual void        Quit ();
        virtual bool        QuitRequested ();

    private:
        // Private hooks
        void                ToggleWindowHeight (bool expand);
        void                UpdateData ();
        void                UpdateRecentMenus ();
        void                CreateSelfJoiner ();

        // Thread functions
        static int32        _splitter (void *data);

        // Private members
        BevelView          *m_backView,
                         *m_innerView;
        BMenu              *m_fileMenu,
                         *m_folderMenu;
        BMenuField         *m_fileField,
                         *m_folderField,
                         *m_sizeField,
                         *m_prefixField,
                         *m_separatorField;
        BPopUpMenu         *m_sizePopUp,
                         *m_prefixPopUp,
                         *m_separatorPopUp;
        BTextControl        *m_filePathView,
                         *m_folderPathView,
                         *m_customSizeView,
                         *m_separatorView;
        BButton            *m_selectFileBtn,
                         *m_selectFolderBtn,
                         *m_splitBtn;
        BStringView        *m_descStr,
                         *m_descStr2,
                         *m_piecesStr,
                         *m_sizeStr;
        BCheckBox          *m_createChk,
                         *m_openDirChk,
                         *m_closeChk;
        BStatusBar         *m_statusBar;

        SelectDirPanel      *m_dirPanel;
        BFilePanel         *m_filePanel;

        float               m_hideProgress,
                          m_showProgress;

        BEntry              m_fileEntry;
        BDirectory          m_destDir;
        BString             m_firstChunkName;

        uint64              m_fragmentSize;
        uint16              m_fragmentCount;
        char               *m_sepString;
        volatile bool        m_cancel;
        bool                m_splitInProgress,
                          m_quitNow;
        BMessenger         *m_messenger;
        thread_id           m_thread;

        RecentMgr          *m_recentSplitFiles,
                         *m_recentSplitDirs;
};

#endif /* _FILE_SPLITTER_WINDOW_H */
