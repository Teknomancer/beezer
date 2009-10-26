/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/


#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <Screen.h>
#include <Bitmap.h>
#include <Message.h>
#include <Application.h>
#include <MenuItem.h>

#include "StartupWindow.h"
#include "LangStrings.h"
#include "AppConstants.h"
#include "UIConstants.h"
#include "BevelView.h"
#include "ImageButton.h"
#include "ArchiverMgr.h"
#include "AppUtils.h"
#include "MsgConstants.h"
#include "BitmapPool.h"
#include "BeezerStringView.h"
#include "RecentMgr.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "FileSplitterWindow.h"

#include "BubbleHelper.h"

//=============================================================================================================//

StartupWindow::StartupWindow (RecentMgr *recentMgr, BubbleHelper *helper, bool startup)
    : BWindow (BRect (10, 10, 0, 100), K_APP_TITLE, B_TITLED_WINDOW,
        B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE),
    m_recentMgr (recentMgr),
    m_bubbleHelper (helper)
{
    m_backView = new BevelView (Bounds(), "StartupWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_TOOLBAR_BACK_COLOR);
    AddChild (m_backView);
    
    float width, height;
    m_headingView = new BeezerStringView (BRect (1, 1, Bounds().right - 1, 25), "StartupWindow:HeadingView",
                         str (S_WELCOME_TO), B_FOLLOW_H_CENTER, B_WILL_DRAW);
    m_headingView->SetFont (be_bold_font);
    m_headingView->SetHighColor (K_STARTUP_MAIN_HEADING);
    m_headingView->SetLowColor (m_headingView->ViewColor());
    m_headingView->GetPreferredSize (&width, &height);
    m_headingView->ResizeTo (width, m_headingView->Frame().Height());
    m_backView->AddChild (m_headingView);

    BView *sepViewLiteEdge = new BView (BRect (1, m_headingView->Frame().bottom + 9, Bounds().right - 1,
                                m_headingView->Frame().bottom + 9), "StartupWindow:SepViewLiteEdge",
                                B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    rgb_color backColor = m_backView->ViewColor();
    backColor.red -= 40; backColor.green -= 40; backColor.blue -= 40;
    sepViewLiteEdge->SetViewColor (backColor);

    BView *sepViewDarkEdge = new BView (BRect (1, sepViewLiteEdge->Frame().bottom + 1, Bounds().right - 1,
                                sepViewLiteEdge->Frame().bottom + 1), "StartupWindow:SepViewDarkEdge",
                                B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    sepViewDarkEdge->SetViewColor (K_WHITE_COLOR);

    m_backView->AddChild (sepViewLiteEdge);
    m_backView->AddChild (sepViewDarkEdge);

    float gapBetweenBtns = 9;

    BRect buttonRect (gapBetweenBtns, sepViewDarkEdge->Frame().bottom + 5, K_TOOLBAR_WIDTH + gapBetweenBtns, 0);
    backColor = K_TOOLBAR_BACK_COLOR;
    
    BitmapPool *_bmps = _glob_bitmap_pool;
    
    m_createBtn = new ImageButton (buttonRect, "StartupWindow:New", str (S_TOOLBAR_NEW),
                         _bmps->m_tbarNewBmp, NULL, new BMessage (M_FILE_NEW), false, backColor, kBelowIcon,
                         false, true, true, B_FOLLOW_H_CENTER);
    m_createBtn->MoveBy (gapBetweenBtns-1, 0);
    m_createBtn->ResizeToPreferred();

    m_openBtn = new ImageButton (buttonRect, "StartupWindow:Open", str (S_TOOLBAR_OPEN),
                         _bmps->m_tbarOpenBmp, NULL, new BMessage (M_FILE_OPEN), false, backColor, kBelowIcon,
                         false, true, true, B_FOLLOW_H_CENTER);
    m_openBtn->MoveBy (m_createBtn->Frame().right + gapBetweenBtns, 0);
    m_openBtn->ResizeToPreferred();

    m_openRecentBtn = new ImageButton (buttonRect, "StartupWindow:OpenRecent", str (S_TOOLBAR_OPEN_RECENT),
                         _bmps->m_tbarOpenRecentBmp, NULL, new BMessage (M_FILE_OPEN_RECENT), false,
                         backColor, kBelowIcon, false, true, true, B_FOLLOW_H_CENTER);
    m_openRecentBtn->MoveBy (m_openBtn->Frame().right + gapBetweenBtns, 0);
    m_openRecentBtn->ResizeToPreferred();
    
    m_toolsBtn = new ImageButton (buttonRect, "StartupWindow:Tools", str (S_TOOLBAR_TOOLS),
                         _bmps->m_tbarToolsBmp, NULL, new BMessage (M_TOOLS_LIST), false,
                         backColor, kBelowIcon, false, true, true, B_FOLLOW_H_CENTER);
    m_toolsBtn->MoveBy (m_openRecentBtn->Frame().right + gapBetweenBtns, 0);
    m_toolsBtn->ResizeToPreferred();
    
    m_prefsBtn = new ImageButton (buttonRect, "StartupWindow:Prefs", str (S_TOOLBAR_PREFS),
                         _bmps->m_tbarPrefsBmp, NULL, new BMessage (M_EDIT_PREFERENCES), false, backColor,
                         kBelowIcon,    false, true, true, B_FOLLOW_H_CENTER);
    m_prefsBtn->MoveBy (m_toolsBtn->Frame().right + gapBetweenBtns, 0);
    m_prefsBtn->ResizeToPreferred();

    // Set the width of the window before adding the horizontally centering controls, otherwise we
    // won't be able to set the correct width as controls will keep moving as we call ResizeTo()
    ResizeTo (m_prefsBtn->Frame().right + 2 * gapBetweenBtns, Frame().Height());

    m_backView->AddChild (m_createBtn);
    m_backView->AddChild (m_openBtn);
    m_backView->AddChild (m_openRecentBtn);
    m_backView->AddChild (m_toolsBtn);
    m_backView->AddChild (m_prefsBtn);
    
    // Setup the bubblehelps
    m_bubbleHelper->SetHelp (m_createBtn, const_cast<char*>(str (S_BUBBLEHELP_NEW)));
    m_bubbleHelper->SetHelp (m_openBtn, const_cast<char*>(str (S_BUBBLEHELP_OPEN)));
    m_bubbleHelper->SetHelp (m_openRecentBtn, const_cast<char*>(str (S_BUBBLEHELP_OPEN_RECENT)));
    m_bubbleHelper->SetHelp (m_prefsBtn, const_cast<char*>(str (S_BUBBLEHELP_PREFS)));
    m_bubbleHelper->SetHelp (m_toolsBtn, const_cast<char*>(str (S_BUBBLEHELP_TOOLS)));

    // Constrain window size
    ResizeTo (Frame().Width(), m_createBtn->Frame().bottom + 10);
    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    minH = m_prefsBtn->Frame().right + gapBetweenBtns;
    SetSizeLimits (minH, maxH, minV, maxV);
    ResizeTo (minH, Frame().Height());

    // Center window on-screen
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    MoveBy (0, -Frame().Height() / 2);

    // Center the heading view
    BRect window_rect (Frame());
    m_headingView->MoveTo (window_rect.Width() / 2 - m_headingView->Frame().Width() / 2,
                      m_headingView->Frame().top);

    // Restore from prefs
    BRect frame;
    if (_prefs_windows.FindBoolDef (kPfWelcomeWnd, true))
        if (_prefs_windows.FindRect (kPfWelcomeWndFrame, &frame) == B_OK)
        {
           MoveTo (frame.LeftTop());
           ResizeTo (frame.Width(), frame.Height());
        }
    
    m_recentMenu = NULL;
    m_toolsMenu = NULL;
    
    int8 startupAction = _prefs_misc.FindInt8Def (kPfStartup, 0);
    if (startupAction == 0 || startup == false)
        Show();
    else if (startupAction == 1)
        be_app_messenger.SendMessage (M_FILE_NEW);
    else if (startupAction == 2)
        be_app_messenger.SendMessage (M_FILE_OPEN);
}

//=============================================================================================================//

bool StartupWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef (kPfWelcomeWnd, true))
        _prefs_windows.SetRect (kPfWelcomeWndFrame, Frame());
    
    return BWindow::QuitRequested();
}

//=============================================================================================================//

void StartupWindow::Quit()
{
    be_app_messenger.SendMessage (M_CLOSE_STARTUP);
    return BWindow::Quit();
}

//=============================================================================================================//

void StartupWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_FILE_OPEN: case M_FILE_NEW: case M_EDIT_PREFERENCES:
        {
           be_app_messenger.SendMessage (message);
           break;
        }
        
        case B_SIMPLE_DATA:
        {
           message->what = B_REFS_RECEIVED;
           be_app_messenger.SendMessage (message);
           break;
        }
        
        case M_FILE_OPEN_RECENT:
        {
           if (m_recentMenu)
           {
               delete m_recentMenu;
               m_recentMenu = NULL;
           }
               
           m_recentMenu = m_recentMgr->BuildPopUpMenu (NULL, "refs", be_app);
           
           BPoint point (m_openRecentBtn->Frame().left, m_openRecentBtn->Frame().bottom + 4);
           BPoint screenPt = point;
           BRect ignoreClickRect (m_openRecentBtn->Frame());
           
           ConvertToScreen (&screenPt);
           ConvertToScreen (&ignoreClickRect);
        
           m_recentMenu->SetAsyncAutoDestruct (true);
           m_recentMenu->Go (screenPt, true, true, ignoreClickRect, false);
           break;
        }

        case M_TOOLS_LIST:
        {
           if (m_toolsMenu)
           {
               delete m_toolsMenu;
               m_toolsMenu = NULL;
           }
           
           m_toolsMenu = _bzr()->BuildToolsPopUpMenu ();
           m_toolsMenu->SetTargetForItems (be_app);        // send-directly to be_app object ;)
           
           BPoint point (m_toolsBtn->Frame().left, m_toolsBtn->Frame().bottom + 4);
           BPoint screenPt = point;
           BRect ignoreClickRect (m_toolsBtn->Frame());
           
           ConvertToScreen (&screenPt);
           ConvertToScreen (&ignoreClickRect);

           m_toolsMenu->SetAsyncAutoDestruct (true);
           m_toolsMenu->Go (screenPt, true, true, ignoreClickRect, false);
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
           BWindow::MessageReceived (message);
    }
}

//=============================================================================================================//
