/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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
#include "Beezer.h"
#include "BeezerStringView.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "FileSplitterWindow.h"
#include "ImageButton.h"
#include "LangStrings.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "RecentMgr.h"
#include "StartupWindow.h"
#include "UIConstants.h"



StartupWindow::StartupWindow(RecentMgr* recentMgr, bool startup)
    : BWindow(BRect(10, 10, 0, 100), K_APP_TITLE, B_TITLED_WINDOW,
              B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
    m_recentMgr(recentMgr)
{
    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    float width, height;
    m_headingView = new BStringView("StartupWindow:HeadingView", str(S_WELCOME_TO));
    m_headingView->SetFont(be_bold_font);
    m_headingView->SetHighColor(K_STARTUP_MAIN_HEADING);
    m_headingView->SetLowColor(m_headingView->ViewColor());

    BView* sepViewLiteEdge = new BView("StartupWindow:SepViewLiteEdge", B_WILL_DRAW);
    sepViewLiteEdge->SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));

    BView* sepViewDarkEdge = new BView("StartupWindow:SepViewDarkEdge", B_WILL_DRAW);
    sepViewDarkEdge->SetViewColor(K_WHITE_COLOR);

    // FIXME this is a bogus rect used until ImageButton has been made more layout friendly
    BRect buttonRect(0, 0, 1, 1);

    BitmapPool* _bmps = _glob_bitmap_pool;

    m_createBtn = new ImageButton(buttonRect, "StartupWindow:New", str(S_TOOLBAR_NEW),
                                  _bmps->m_tbarNewBmp, NULL, new BMessage(M_FILE_NEW), false, ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon,
                                  false, true, true, B_FOLLOW_H_CENTER);
    m_createBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_openBtn = new ImageButton(buttonRect, "StartupWindow:Open", str(S_TOOLBAR_OPEN),
                                _bmps->m_tbarOpenBmp, NULL, new BMessage(M_FILE_OPEN), false, ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon,
                                false, true, true, B_FOLLOW_H_CENTER);
    m_openBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_openRecentBtn = new ImageButton(buttonRect, "StartupWindow:OpenRecent", str(S_TOOLBAR_OPEN_RECENT),
                                      _bmps->m_tbarOpenRecentBmp, NULL, new BMessage(M_FILE_OPEN_RECENT), false,
                                      ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true, B_FOLLOW_H_CENTER);
    m_openRecentBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_toolsBtn = new ImageButton(buttonRect, "StartupWindow:Tools", str(S_TOOLBAR_TOOLS),
                                 _bmps->m_tbarToolsBmp, NULL, new BMessage(M_TOOLS_LIST), false,
                                 ui_color(B_PANEL_BACKGROUND_COLOR), kBelowIcon, false, true, true, B_FOLLOW_H_CENTER);
    m_toolsBtn->SetExplicitMinSize(BSize(K_TOOLBAR_WIDTH, -1));

    m_prefsBtn = new ImageButton(buttonRect, "StartupWindow:Prefs", str(S_TOOLBAR_PREFS),
                                 _bmps->m_tbarPrefsBmp, NULL, new BMessage(M_EDIT_PREFERENCES), false, ui_color(B_PANEL_BACKGROUND_COLOR),
                                 kBelowIcon,    false, true, true, B_FOLLOW_H_CENTER);

    AddChild(BGroupLayoutBuilder(B_VERTICAL, 0)
             .AddStrut(5)
             .AddGroup(B_HORIZONTAL)
             .AddGlue()
             .Add(m_headingView)
             .AddGlue()
             .End()
             .AddStrut(5)
             .Add(sepViewLiteEdge, 0)
             .Add(sepViewDarkEdge, 0)
             .AddStrut(5)
             .AddGroup(B_HORIZONTAL)
             .Add(m_createBtn)
             .Add(m_openBtn)
             .Add(m_openRecentBtn)
             .Add(m_toolsBtn)
             .Add(m_prefsBtn)
             .SetInsets(5, 5, 5, 5)
             .End()
            );

    // Setup the tooltips
    m_createBtn->SetToolTip(const_cast<char*>(str(S_BUBBLEHELP_NEW)));
    m_openBtn->SetToolTip(const_cast<char*>(str(S_BUBBLEHELP_OPEN)));
    m_openRecentBtn->SetToolTip(const_cast<char*>(str(S_BUBBLEHELP_OPEN_RECENT)));
    m_prefsBtn->SetToolTip(const_cast<char*>(str(S_BUBBLEHELP_PREFS)));
    m_toolsBtn->SetToolTip(const_cast<char*>(str(S_BUBBLEHELP_TOOLS)));

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

            // FIXME not sure what's up with these odd calculations.
            // they y coordinate of the point should be recentRect.bottom
            // but that puts the menu near the top
            BRect recentRect(m_openRecentBtn->Frame());
            BPoint point(recentRect.left, recentRect.bottom + recentRect.Height() - 4);
            BPoint screenPt = point;
            BRect ignoreClickRect(m_openRecentBtn->Frame());

            ConvertToScreen(&screenPt);
            ConvertToScreen(&ignoreClickRect);

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

            // FIXME not sure what's up with these odd calculations.
            // they y coordinate of the point should be toolRect.bottom
            // but that puts the menu near the top
            BRect toolRect(m_toolsBtn->Frame());
            BPoint point(toolRect.left, toolRect.bottom + toolRect.Height() - 4);
            BPoint screenPt = point;
            BRect ignoreClickRect(m_toolsBtn->Frame());

            ConvertToScreen(&screenPt);
            ConvertToScreen(&ignoreClickRect);
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


