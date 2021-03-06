// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <Window.h>
#include <Path.h>
#include <String.h>

class BFilePanel;
class BMenu;
class BStatusBar;
struct entry_ref;

class Archiver;
class BeezerListView;
class BevelView;
class ImageButton;
class InfoBar;
class ListEntry;
class LogTextView;
class MainMenu;
class ProgressWindow;
class RecentMgr;
class SearchWindow;
class SelectDirPanel;
class StatusWindow;
class ToolBar;
class WindowMgr;

class CLVColumn;
class CLVContainerView;
class SplitPane;

class MainWindow : public BWindow
{
    public:
        MainWindow(BRect frame, WindowMgr* windowMgr, RecentMgr* recentMgr,
                   RecentMgr* extractMgr);
        ~MainWindow();

        // Inherited hooks
        virtual void        Quit();
        virtual bool        QuitRequested();
        virtual void        Show();
        virtual void        MessageReceived(BMessage* message);
        virtual void        FrameResized(float newWidth, float newHeight);

        // Public hooks
        void                LoadSettingsFromArchive(entry_ref* ref);
        void                LoadDefaultSettings();

        // Pubilc members
        BPath               m_archivePath;        // used from thread while creating archives
        LogTextView*        m_logTextView;

    private:
        // Friends who can access private things
        friend class        BeezerApp;

        // Thread functions
        static int32        _extractor(void* arg);
        static int32        _tester(void* arg);
        static int32        _deletor(void* arg);
        static int32        _adder(void* arg);
        static int32        _copier(void* arg);
        static int32        _counter(void* arg);
        static int32        _viewer(void* arg);
        static int32        _opener(void* arg);

        // Private hooks
        void                SaveSettingsAsDefaults() const;
        void                SaveSettingsToArchive(BMessage* message);
        void                SaveArchiverToArchive(BMessage* message);
        BMenu*              LoadArchiverFromArchive(entry_ref* ref);
        void                SetState(BMessage* msg);
        void                GetState(BMessage& msg) const;
        void                CacheState(BMessage** cache, BMessage* msg);

        void                UpdateWindowManager(bool isClosing = false);
        void                UpdateWindowsMenu();
        void                UpdateRecentMenu();
        void                UpdateExtractMenu();
        void                UpdateNewWindow();
        void                AddDynamicExtractPaths(BMenu* menu) const;
        void                AddFavouriteExtractPaths(BMenu* menu) const;
        BString             StrippedArchiveName() const;
        void                ToggleColumn(CLVColumn* col, BMessage* message);
        void                ToggleToolBar();
        void                ToggleInfoBar();
        void                ToggleActionLog();

        void                AddToolBar();
        void                AddInfoBar();
        void                AddListView();
        void                AddLogTextView();
        void                AddSplitterBar();
        void                UpdateListView(bool invalidate);
        void                UpdateUIAsPerSelection();
        void                UpdateFileSelectNeeders(bool enable);
        void                UpdateValidArchiveNeeders(bool enable);
        void                UpdateListItemNeeders(bool enable);
        void                UpdateSelectNeeders(bool enable);
        void                UpdateFocusNeeders(bool enable);
        void                UpdateFolderSelectNeeders(bool enable);
        void                ActivateUIForValidArchive();
        void                UpdateInfoBar(int32 count, int64 bytes) const;
        void                SetupArchiver(entry_ref* ref, char* mimeString);
        void                AddArchiverMenu();
        void                InitArchiver();
        void                AdjustColumns();
        void                OpenArchive();
        void                OpenArchivePartTwo(status_t result);
        void                ViewFile(BMessage* message);
        void                ExtractArchive(entry_ref *refToDir, bool fullArchive);
        bool                IsExtractPathValid(const char* path, bool throwAlertErrorIfAny) const;
        void                SetupExtractPanel(BMessage* extractMessage);
        void                SetupAddPanel();
        void                DeleteFilesFromArchive();
        void                CancelDelete();
        void                DeleteDone(BMessage* message);
        void                DeleteUpdate();
        void                ExtractDone(BMessage* message);
        void                TestArchive();
        void                TestDone(BMessage* message);
        void                ClearDeleteLists();
        void                EditComment(bool failIfNoComment);
        void                SetBusyState(bool on) const;
        void                EmptyListViewIfNeeded();
        void                ShowOpNotSupported() const;
        void                ShowReadOnlyError() const;
        void                ShowArkPathError() const;
        bool                CanWriteArchive() const;
        bool                CanAddFiles() const;
        const char*         MakeTempDirectory();
        void                AddNewFolder();
        int32               AddItemsFromList(int32 start, int32* totalItems);
        int32               AddFoldersFromList(int32 start, int32* totalItems);
        int32               AddItemsFromList(BList* list, int32 start);
        int32               AddFoldersFromList(BList* list, int32 start);
        int32               AddFolderToMessage(ListEntry* item, BMessage* message, bool countOnlyFiles,
                                               BList* fileList = NULL, BList* dirList = NULL, bool skipFiles = false);
        bool                ConfirmAddOperation(const char* addingUnderDirPath, BMessage* refsMessage,
                                                int32* count, int32* skipped);

        // Private members
        BView*              m_backView;
        BevelView*          m_outputBackView;
        MainMenu*           m_mainMenu;
        ToolBar*            m_toolBar;
        InfoBar*            m_infoBar;
        SearchWindow*       m_searchWnd;
        ImageButton*        m_newButton,
                            *m_openButton,
                            *m_searchButton,
                            *m_viewButton,
                            *m_addButton,
                            *m_deleteButton,
                            *m_extractButton;
        CLVContainerView*   m_listContainer;
        CLVColumn*          m_fileNameColumn,
                            *m_sizeColumn,
                            *m_packedColumn,
                            *m_ratioColumn,
                            *m_pathColumn,
                            *m_dateColumn,
                            *m_methodColumn,
                            *m_crcColumn;
        BeezerListView*     m_listView;
        SelectDirPanel*     m_extractToPanel;
        BFilePanel*         m_addPanel;
        Archiver*           m_archiver;
        SplitPane*          m_splitPane;
        BEntry              m_archiveEntry;
        entry_ref           m_archiveRef;
        bool                m_logHidden,
                            m_badArchive,
                            m_createMode,
                            m_addStarted,
                            m_dragExtract;
        BDirectory*         m_archiversDir,
                            *m_tempDir;
        const char*         m_tempDirPath;
        BList*              m_fileList,
                            *m_dirList,
                            m_addedFileList,
                            m_addedDirList,
                            m_columnList,
                            *m_deleteFileList,
                            *m_deleteDirList;
        WindowMgr*          m_windowMgr;
        RecentMgr*          m_recentMgr,
                            *m_extractMgr;
        BMessage*           m_searchSettingsMsg,
                            *m_cachedUIState,
                            *m_cachedArkState,
                            m_dragExtractItems;
        off_t               m_archiveSize;
        int8                m_foldingLevel;
        float               m_slotOneX,
                            m_slotTwoX;
        ProgressWindow*     m_progressWnd;
        StatusWindow*       m_statusWnd;
        volatile bool       m_publicThreadCancel,
                            m_criticalSection;
        BMenuBar*           m_addPanelMenuBar;
};

#endif /* _MAIN_WINDOW_H */
