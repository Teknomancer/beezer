/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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


#include <Application.h>
#include <Bitmap.h>
#include <GroupLayoutBuilder.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>

#include "AppConstants.h"
#include "AppUtils.h"
#include "ArchiverMgr.h"
#include "BeezerApp.h"
#include "BeezerStringView.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "FileSplitterWindow.h"
#include "ImageButton.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "RecentMgr.h"
#include "StartupWindow.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "StartupWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


StartupWindow::StartupWindow(RecentMgr* recentMgr, bool startup)
    : BWindow(BRect(10, 10, 0, 100), B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE), B_TITLED_WINDOW,
              B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
    m_recentMgr(recentMgr)
{
    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    float width, height;
    BString welcome(B_TRANSLATE("Welcome to %appname%"));
    welcome.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    m_headingView = new BStringView("StartupWindow:HeadingView", welcome);
    BFont font(be_bold_font);
    font.SetSize(font.Size()+2);
    m_headingView->SetFont(&font);
    m_headingView->SetHighColor(K_STARTUP_MAIN_HEADING);

    BView* sepViewLiteEdge = new BView("StartupWindow:SepViewLiteEdge", B_WILL_DRAW);
    sepViewLiteEdge->SetViewUIColor(B_PANEL_BACKGROUND_COLOR, B_LIGHTEN_1_TINT);
    sepViewLiteEdge->SetExplicitSize(BSize(B_SIZE_UNSET, 0));  // 0 height gives us 1 pixel

    BView* sepViewDarkEdge = new BView("StartupWindow:SepViewDarkEdge", B_WILL_DRAW);
    sepViewDarkEdge->SetViewUIColor(B_PANEL_BACKGROUND_COLOR, B_DARKEN_1_TINT);
    sepViewDarkEdge->SetExplicitSize(BSize(B_SIZE_UNSET, 0));  // 0 height gives us 1 pixel


    BitmapPool* _bmps = _glob_bitmap_pool;

    m_createBtn = new ImageButton("StartupWindow:New", B_TRANSLATE("New"),
                                  _bmps->m_tbarNewBmp, NULL, new BMessage(M_FILE_NEW), false,
                                  ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true);
    m_createBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_openBtn = new ImageButton("StartupWindow:Open", B_TRANSLATE("Open"),
                                _bmps->m_tbarOpenBmp, NULL, new BMessage(M_FILE_OPEN), false,
                                ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true);
    m_openBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_openRecentBtn = new ImageButton("StartupWindow:OpenRecent", B_TRANSLATE("Recent"),
                                      _bmps->m_tbarOpenRecentBmp, NULL, new BMessage(M_FILE_OPEN_RECENT), false,
                                      ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true);
    m_openRecentBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_toolsBtn = new ImageButton("StartupWindow:Tools", B_TRANSLATE("Tools"),
                                 _bmps->m_tbarToolsBmp, NULL, new BMessage(M_TOOLS_LIST), false,
                                 ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true);
    m_toolsBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_prefsBtn = new ImageButton("StartupWindow:Prefs", B_TRANSLATE("Settings"),
                                 _bmps->m_tbarPrefsBmp, NULL, new BMessage(M_EDIT_PREFERENCES), false,
                                 ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true);

    AddChild(BGroupLayoutBuilder(B_VERTICAL, 0)
             .AddStrut(5)
             .AddGroup(B_HORIZONTAL)
             .AddGlue()
             .Add(m_headingView)
             .AddGlue()
             .End()
             .AddStrut(5)
             .Add(sepViewDarkEdge, 0)
             .Add(sepViewLiteEdge, 0)
             .AddStrut(5)
             .AddGlue()
             .AddGroup(B_HORIZONTAL)
             .Add(m_createBtn)
             .Add(m_openBtn)
             .Add(m_openRecentBtn)
             .Add(m_toolsBtn)
             .Add(m_prefsBtn)
             .SetInsets(5, 5, 5, 5)
             .End()
             .AddGlue()
            );

    // Setup the tooltips
    m_createBtn->SetToolTip(const_cast<char*>(B_TRANSLATE("Create a new archive")));
    m_openBtn->SetToolTip(const_cast<char*>(B_TRANSLATE("Open an existing archive")));
    m_openRecentBtn->SetToolTip(const_cast<char*>(B_TRANSLATE("Open a recently opened archive")));
    m_prefsBtn->SetToolTip(const_cast<char*>(B_TRANSLATE("Edit application preferences")));
    m_toolsBtn->SetToolTip(const_cast<char*>(B_TRANSLATE("Additional tools")));

    // Center window on-screen
    CenterOnScreen();

    // Restore from prefs
    BRect frame;
    if (_prefs_windows.FindBoolDef(kPfWelcomeWnd, true))
        if (_prefs_windows.FindRect(kPfWelcomeWndFrame, &frame) == B_OK)
        {
            MoveTo(frame.LeftTop());
            ResizeTo(frame.Width(), frame.Height());
        }

    m_recentMenu = NULL;
    m_toolsMenu = NULL;

    int8 startupAction = _prefs_misc.FindInt8Def(kPfStartup, 0);
    if (startupAction == 0 || startup == false)
        Show();
    else if (startupAction == 1)
        be_app_messenger.SendMessage(M_FILE_NEW);
    else if (startupAction == 2)
        be_app_messenger.SendMessage(M_FILE_OPEN);
}



bool StartupWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef(kPfWelcomeWnd, true))
        _prefs_windows.SetRect(kPfWelcomeWndFrame, Frame());

    return BWindow::QuitRequested();
}



void StartupWindow::Quit()
{
    be_app_messenger.SendMessage(M_CLOSE_STARTUP);
    return BWindow::Quit();
}



void StartupWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_FILE_OPEN: case M_FILE_NEW: case M_EDIT_PREFERENCES:
        {
            be_app_messenger.SendMessage(message);
            break;
        }

        case B_SIMPLE_DATA:
        {
            message->what = B_REFS_RECEIVED;
            be_app_messenger.SendMessage(message);
            break;
        }

        case M_FILE_OPEN_RECENT:
        {
            if (m_recentMenu)
            {
                delete m_recentMenu;
                m_recentMenu = NULL;
            }

            m_recentMenu = m_recentMgr->BuildPopUpMenu(NULL, "refs", be_app);

            BPoint screenPt(0, m_openRecentBtn->Frame().bottom);
            BRect ignoreClickRect(m_openRecentBtn->Frame());

            m_openRecentBtn->ConvertToScreen(&screenPt);
            m_openRecentBtn->ConvertToScreen(&ignoreClickRect);

            m_recentMenu->SetAsyncAutoDestruct(true);
            m_recentMenu->Go(screenPt, true, true, ignoreClickRect, false);
            break;
        }

        case M_TOOLS_LIST:
        {
            if (m_toolsMenu)
            {
                delete m_toolsMenu;
                m_toolsMenu = NULL;
            }

            m_toolsMenu = _bzr()->BuildToolsPopUpMenu();
            m_toolsMenu->SetTargetForItems(be_app);         // send-directly to be_app object ;)

            BPoint screenPt(0, m_toolsBtn->Frame().bottom);
            BRect ignoreClickRect(m_toolsBtn->Frame());

            m_toolsBtn->ConvertToScreen(&screenPt);
            m_toolsBtn->ConvertToScreen(&ignoreClickRect);
            m_toolsMenu->SetAsyncAutoDestruct(true);
            m_toolsMenu->Go(screenPt, true, true, ignoreClickRect, false);
            break;
        }

        case M_UPDATE_RECENT:
        {
            // this isn't needed as we can always call BuildPopUpMenu() not a big deal, but i have
            // provided for the future where we may be in a situation where we may not control the
            // showing of the context menu in which case we will need to update it every time a change is
            // made, this is completely useless for the time being -- but its better implemented this way
            // If this need to be implemented also see  Beezer::MessageReceived()'s M_UPDATE_RECENT case.
            break;
        }

        default:
            BWindow::MessageReceived(message);
    }
}


