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

    AddShortcut('w', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));

    // Center window on-screen
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
