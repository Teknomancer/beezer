// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _BEEZER_APP_H
#define _BEEZER_APP_H

#include <Application.h>
#include <FilePanel.h>

class BDirectory;
class BFile;
class BMenuField;
class BPopUpMenu;
class BString;

class AboutWindow;
class AddOnWindow;
class FileJoinerWindow;
class FileSplitterWindow;
class MainWindow;
class PrefsWindow;
class RecentMgr;
class RuleMgr;
class StartupWindow;
class WindowMgr;



class BeezerApp : public BApplication
{
    public:
        BeezerApp();
        virtual ~BeezerApp();

        // Inherited hooks
        virtual void        AboutRequested();
        virtual void        ArgvReceived(int32 argc, char** argv);
        virtual void        MessageReceived(BMessage* message);
        virtual void        RefsReceived(BMessage* message);
        virtual void        ReadyToRun();
        virtual void        Quit();

        // Additional hooks
        MainWindow*         RegisterWindow(entry_ref* ref = NULL);
        void                UnRegisterWindow(bool closeApp);
        void                CreateFilePanel(BFilePanel*& panel, file_panel_mode mode);
        void                DeleteFilePanel(BFilePanel*& panel);
        MainWindow*         IsFirstWindowADummy();
        MainWindow*         WindowForRef(entry_ref* ref);
        MainWindow*         CreateWindow(entry_ref* ref);
        BMenu*              BuildToolsMenu() const;
        BPopUpMenu*         BuildToolsPopUpMenu() const;

        // Public members
        BString             m_settingsPathStr;
        BDirectory          m_addonsDir,
                            m_docsDir,
                            m_settingsDir,
                            m_stubDir;

    private:
        // Private members
        void                InitPaths();
        void                InitPrefs();
        void                LoadArchivers();
        void                UnloadArchivers();
        void                CompileTimeString(BString& output);
        void                VersionString(BString& output);
        void                ShowCreateFilePanel();
        int8                RegisterFileTypes() const;

        AboutWindow*        m_aboutWnd;
        StartupWindow*      m_startupWnd;
        PrefsWindow*        m_prefsWnd;
        FileSplitterWindow* m_splitWnd;
        FileJoinerWindow*   m_joinWnd;
        AddOnWindow*        m_addOnWnd;

        uint32              m_nextWindowID,
                            m_nWindows;
        BRect               m_defaultWindowRect,
                            m_newWindowRect;
        BFilePanel*         m_openFilePanel,
                            *m_createFilePanel;
        WindowMgr*          m_windowMgr;
        RecentMgr*          m_recentMgr,
                            *m_extractMgr,
                            *m_splitFilesMgr,
                            *m_splitDirsMgr;
        RuleMgr*            m_ruleMgr;
        BPopUpMenu*         m_arkTypePopUp;
        BMenu*              m_toolsMenu;
        BMenuField*         m_arkTypeField;
        BList               m_arkTypes,
                            m_arkExtensions;
};


BeezerApp* _bzr();


#endif /* _BEEZER_APP_H */
