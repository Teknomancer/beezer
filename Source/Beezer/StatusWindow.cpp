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

#include <Bitmap.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>

#include "AppConstants.h"
#include "BarberPole.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"
#include "StatusWindow.h"
#include "UIConstants.h"



StatusWindow::StatusWindow (const char *title, BWindow *callerWindow, const char *text, volatile bool *cancel,
        bool showWindow)
    : BWindow (BRect (0, 0, 300, 0), title, B_MODAL_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
           B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
    if (callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap *icon = ResBitmap ("Img:WarnAlert");

    StaticBitmapView *iconView = new StaticBitmapView (BRect (0, 0, icon->Bounds().Width(), icon->Bounds().Height()), "StatusWindow:iconView", icon);

    BStringView *titleStrView = new BStringView ("StatusWindow:StringView", title);
    titleStrView->SetFont (be_bold_font);
    titleStrView->SetHighColor (K_STARTUP_MAIN_HEADING);

    m_barberPole = new BarberPole (BRect (0, 0, 1, 1), "StatusWindow::BarberPole");

    BStringView *textLabel = new BStringView ("StatusWindow:StringView", text);

    // Start the builder with vertical in case we need to add the cancel button
    BGroupLayoutBuilder layout = BGroupLayoutBuilder(B_VERTICAL)
        .AddGroup(B_HORIZONTAL)
            .Add(iconView, 0)
            .AddGroup(B_VERTICAL, 0)
                .Add(titleStrView)
                .AddGroup(B_HORIZONTAL)
                    .Add(m_barberPole, 0)
                    .Add(textLabel)
                    .AddGlue()
                .End()
            .End()
            .AddGlue()
        .End()
        .SetInsets(4 * K_MARGIN, 2 * K_MARGIN, 4 * K_MARGIN, 2 * K_MARGIN);

    if (cancel)
        layout.Add(BGroupLayoutBuilder(B_HORIZONTAL, 0)
			.AddGlue()
			.Add(new BButton ("StatusWindow:CancelButton", str (S_STOP_OPERATION), new BMessage (M_STOP_OPERATION)))
			.AddGlue()
		);

    AddChild(layout);

    // Center our window on screen
    CenterOnScreen();

    SetPulseRate (K_BARBERPOLE_PULSERATE);
    m_barberPole->SetValue (true, true);

    if (cancel)
        *cancel = false;
    m_cancel = cancel;

    if (showWindow == true)
        Show();
}


void StatusWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_STOP_OPERATION:
        {
           m_barberPole->SetValue (false, false);
           if (m_cancel)
               *m_cancel = true;
           break;
        }

        case M_CLOSE:
        {
           snooze (70000);
           m_barberPole->SetValue (false, false);
           Quit();
           break;
        }

        default:
        {
           BWindow::MessageReceived (message);
           break;
        }
    }
}
