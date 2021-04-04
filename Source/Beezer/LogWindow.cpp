// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "LogWindow.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <TextView.h>


LogWindow::LogWindow(BWindow* callerWindow, const char* title, const char* logText, BFont* displayFont)
    : BWindow(BRect(80, 140, 605, 355), title, B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
    if (callerWindow)
    {
        SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset(callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL));

    BTextView* textView = new BTextView("LogWindow:TextView", displayFont, NULL, B_WILL_DRAW | B_FRAME_EVENTS);

    BScrollView* scrollView = new BScrollView("LogWindow:ScrollView", textView, B_WILL_DRAW, true, true, B_PLAIN_BORDER);

    textView->SetText(logText);
    textView->SetWordWrap(false);
    textView->MakeEditable(false);

    AddChild(scrollView);

    AddShortcut('W', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));

    if (callerWindow != NULL)
    {
        UpdateSizeLimits();
        BRect callerRect(callerWindow->Frame());
        BPoint windowPoint(callerRect.left + callerRect.Width()/2 - Bounds().Width()/2, callerRect.top + callerRect.Height()/2 - Bounds().Height()/2);
        MoveTo(windowPoint);
    }
    else
        CenterOnScreen();

    // Load from prefs if it allows
    BRect frame;
    if (_prefs_windows.FindBoolDef(kPfLogWnd, true))
        if (_prefs_windows.FindRect(kPfLogWndFrame, &frame) == B_OK)
        {
            MoveTo(frame.LeftTop());
            ResizeTo(frame.Width(), frame.Height());
        }

    Show();
}


bool LogWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef(kPfLogWnd, true))
        _prefs_windows.SetRect(kPfLogWndFrame, Frame());

    return BWindow::QuitRequested();
}
