/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _BEEZER_H
#define _BEEZER_H

#include <Application.h>
#include <FilePanel.h>
#include <String.h>
#include <PopUpMenu.h>
#include <MenuField.h>

class MainWindow;
class AboutWindow;
class AuthorWindow;
class StartupWindow;
class PrefsWindow;
class FileSplitterWindow;
class FileJoinerWindow;
class AddOnWindow;
class WindowMgr;
class RecentMgr;
class RuleMgr;

class BubbleHelper;

class Beezer : public BApplication
{
    public:
        Beezer ();
        virtual ~Beezer();
        
        // Inherited hooks
        virtual void        MessageReceived (BMessage *message);
        virtual void        RefsReceived (BMessage *message);
        virtual void        ReadyToRun ();
        virtual void        Quit ();
        
        // Additional hooks
        MainWindow            *RegisterWindow (entry_ref *ref = NULL);
        void                UnRegisterWindow (bool closeApp);
        void                CreateFilePanel (BFilePanel *&panel, file_panel_mode mode);
        void                DeleteFilePanel (BFilePanel *&panel);
        MainWindow            *IsFirstWindowADummy ();
        MainWindow            *WindowForRef (entry_ref *ref);
        MainWindow            *CreateWindow (entry_ref *ref);
        BMenu                *BuildToolsMenu () const;
        BPopUpMenu            *BuildToolsPopUpMenu () const;
        
        // Public members
        BString                m_settingsPathStr;
        BDirectory            m_addonsDir,
                            m_docsDir,
                            m_settingsDir,
                            m_binDir,
                            m_stubDir;

    private:
        // Private members
        void                InitPaths ();
        void                InitPrefs ();
        void                LoadArchivers ();
        void                UnloadArchivers ();
        const char*            CompileTimeString (bool writeToResIfNeeded) const;
        void                WriteToCTFile (BFile *ctFile, BString *compileTimeStr) const;
        void                ShowCreateFilePanel ();
        int8                RegisterFileTypes () const;
        
        AboutWindow            *m_aboutWnd;
        AuthorWindow        *m_authorWnd;
        StartupWindow        *m_startupWnd;
        PrefsWindow            *m_prefsWnd;
        FileSplitterWindow    *m_splitWnd;
        FileJoinerWindow    *m_joinWnd;
        AddOnWindow            *m_addOnWnd;
        
        uint32                m_nextWindowID,
                            m_nWindows;
        BRect                m_defaultWindowRect,
                            m_newWindowRect;
        BFilePanel            *m_openFilePanel,
                            *m_createFilePanel;
        BubbleHelper        *m_bubbleHelper;
        WindowMgr            *m_windowMgr;
        RecentMgr            *m_recentMgr,
                            *m_extractMgr,
                            *m_splitFilesMgr,
                            *m_splitDirsMgr;
        RuleMgr                *m_ruleMgr;
        BPopUpMenu            *m_arkTypePopUp;
        BMenu                *m_toolsMenu;
        BMenuField            *m_arkTypeField;
        BList                m_arkTypes,
                            m_arkExtensions;
};

#endif /* _BEEZER_H */
