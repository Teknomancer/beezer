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
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
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

#include <TextView.h>
#include <ScrollView.h>
#include <Button.h>
#include <Screen.h>

#include "LogWindow.h"
#include "BevelView.h"
#include "UIConstants.h"
#include "LangStrings.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include "BetterScrollView.h"

//=============================================================================================================//

LogWindow::LogWindow (BWindow *callerWindow, const char *title, const char *logText, BFont *displayFont)
    : BWindow (BRect (80, 140, 605, 355), title, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE)
{
    if (!callerWindow)
        SetFeel (B_MODAL_APP_WINDOW_FEEL);
    else
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (callerWindow);
    }
    
    BRect bounds (Bounds());
    m_backView = new BevelView (bounds, "LogWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView); 

    m_textView = new BTextView (BRect (K_MARGIN, K_MARGIN, bounds.right - K_MARGIN - B_V_SCROLL_BAR_WIDTH,
                      bounds.bottom - 2 * K_MARGIN - K_BUTTON_HEIGHT - B_H_SCROLL_BAR_HEIGHT),
                      "LogWindow:TextView", BRect (2, 2, 100000, 0), B_FOLLOW_ALL_SIDES,
                      B_WILL_DRAW | B_FRAME_EVENTS);
    
    m_scrollView = new BetterScrollView ("LogWindow:ScrollView", m_textView,
                                B_FOLLOW_ALL_SIDES, B_WILL_DRAW, true, true, true, B_PLAIN_BORDER);
    m_backView->AddChild (m_scrollView);

    m_textView->SetText (logText);
    m_textView->SetWordWrap (false);
    m_textView->MakeEditable (false);

    // Calculate the longest line's width
    m_maxLineWidth = 0;
    for (int32 i = 0; i < m_textView->CountLines(); i++)
        m_maxLineWidth = MAX (m_textView->LineWidth(i), m_maxLineWidth);
    m_textView->SetTextRect (BRect (1, 1, m_maxLineWidth, 0));

    if (displayFont != NULL)
        m_textView->SetFontAndColor (displayFont);
    
    m_closeButton = new BButton (BRect (bounds.right - K_MARGIN - K_BUTTON_WIDTH,
                         bounds.bottom - K_MARGIN - K_BUTTON_HEIGHT, bounds.right - K_MARGIN,
                         bounds.bottom - K_MARGIN), "LogWindow:CloseButton", str (S_LOG_WINDOW_CLOSE),
                         new BMessage (B_QUIT_REQUESTED), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild (m_closeButton);

    // Constrain the width and height of the window
    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (m_closeButton->Frame().Width() + 2 * K_MARGIN, maxH,
           m_closeButton->Frame().Height() + 2 * K_MARGIN + 50, maxV);
    
    // Center window on-screen
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    
    // Load from prefs if it allows
    BRect frame;
    if (_prefs_windows.FindBoolDef (kPfLogWnd, true))
        if (_prefs_windows.FindRect (kPfLogWndFrame, &frame) == B_OK)
        {
           MoveTo (frame.LeftTop());
           ResizeTo (frame.Width(), frame.Height());
        }
    
    Show();
}

//=============================================================================================================//

bool LogWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef (kPfLogWnd, true))
        _prefs_windows.SetRect (kPfLogWndFrame, Frame());

    return BWindow::QuitRequested();
}

//=============================================================================================================//

void LogWindow::FrameResized (float newWidth, float newHeight)
{
    // fix, as scrollview failed to be enabled, we do the below trick
    m_scrollView->SetDataRect (BRect (1, 1, m_maxLineWidth + 3, m_textView->TextRect().Height()), true);
    BWindow::FrameResized (newWidth, newHeight);
}

//=============================================================================================================//
