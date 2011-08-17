/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _BEEZER_H
#define _BEEZER_H

#include <Application.h>
#include <FilePanel.h>
#include <String.h>
#include <PopUpMenu.h>
#include <MenuField.h>

class MainWindow;
class AboutWindow;
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
        MainWindow         *RegisterWindow (entry_ref *ref = NULL);
        void                UnRegisterWindow (bool closeApp);
        void                CreateFilePanel (BFilePanel *&panel, file_panel_mode mode);
        void                DeleteFilePanel (BFilePanel *&panel);
        MainWindow         *IsFirstWindowADummy ();
        MainWindow         *WindowForRef (entry_ref *ref);
        MainWindow         *CreateWindow (entry_ref *ref);
        BMenu              *BuildToolsMenu () const;
        BPopUpMenu         *BuildToolsPopUpMenu () const;

        // Public members
        BString             m_settingsPathStr;
        BDirectory          m_addonsDir,
                            m_docsDir,
                            m_settingsDir,
                            m_stubDir;

    private:
        // Private members
        void                InitPaths ();
        void                InitPrefs ();
        void                LoadArchivers ();
        void                UnloadArchivers ();
        const char*         CompileTimeString (bool writeToResIfNeeded) const;
        void                WriteToCTFile (BFile *ctFile, BString *compileTimeStr) const;
        void                ShowCreateFilePanel ();
        int8                RegisterFileTypes () const;

        AboutWindow        *m_aboutWnd;
        StartupWindow      *m_startupWnd;
        PrefsWindow        *m_prefsWnd;
        FileSplitterWindow *m_splitWnd;
        FileJoinerWindow   *m_joinWnd;
        AddOnWindow        *m_addOnWnd;

        uint32              m_nextWindowID,
                            m_nWindows;
        BRect               m_defaultWindowRect,
                            m_newWindowRect;
        BFilePanel         *m_openFilePanel,
                           *m_createFilePanel;
        WindowMgr          *m_windowMgr;
        RecentMgr          *m_recentMgr,
                           *m_extractMgr,
                           *m_splitFilesMgr,
                           *m_splitDirsMgr;
        RuleMgr            *m_ruleMgr;
        BPopUpMenu         *m_arkTypePopUp;
        BMenu              *m_toolsMenu;
        BMenuField         *m_arkTypeField;
        BList               m_arkTypes,
                            m_arkExtensions;
        BubbleHelper       *m_bubbleHelper;
};

#endif /* _BEEZER_H */
