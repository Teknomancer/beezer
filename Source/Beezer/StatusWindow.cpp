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
#include <StringView.h>

#include "AppConstants.h"
#include "BarberPole.h"
#include "BevelView.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"
#include "StatusWindow.h"
#include "UIConstants.h"

//=============================================================================================================//

StatusWindow::StatusWindow (const char *title, BWindow *callerWindow, const char *text, volatile bool *cancel,
        bool showWindow)
    : BWindow (BRect (0, 0, 300, 0), title, B_TITLED_WINDOW,
           B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE, B_CURRENT_WORKSPACE)
{
    if (callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (callerWindow);
    }
    SetLook (B_MODAL_WINDOW_LOOK);

    BFont font (be_plain_font);
    font.SetFace (B_BOLD_FACE);
    font_height fntHt;
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    BRect bounds (Bounds());
    m_backView = new BevelView (bounds, "StatusWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    BBitmap *icon = ResBitmap ("Img:WarnAlert");

    StaticBitmapView *iconView = new StaticBitmapView (BRect (2 * K_MARGIN + 2, 2 * K_MARGIN + 2,
                                    2 * K_MARGIN + 2 + 31.0, 2 * K_MARGIN + 2 + 31.0),
                                    "StatusWindow:iconView", icon);
    iconView->SetViewColor (m_backView->ViewColor());
    AddChild (iconView);

    float width, height;
    BStringView *titleStrView = new BStringView (BRect (iconView->Frame().right + 4 * K_MARGIN,
                             K_MARGIN + 3.0, iconView->Frame().right + 4 * K_MARGIN +
                             m_backView->StringWidth (title) + m_backView->StringWidth ("W"),
                             totalFontHeight + K_MARGIN + 3.0), "StatusWindow:StringView",
                             title, B_FOLLOW_LEFT, B_WILL_DRAW);
    titleStrView->SetFont (&font, B_FONT_ALL);
    titleStrView->SetHighColor (K_STARTUP_MAIN_HEADING);
    titleStrView->SetLowColor (titleStrView->ViewColor());
    titleStrView->SetAlignment (B_ALIGN_CENTER);
    titleStrView->GetPreferredSize (&width, &height);
    titleStrView->ResizeTo (width, titleStrView->Frame().Height());
    m_backView->AddChild (titleStrView);

    m_barberPole = new BarberPole (BRect (titleStrView->Frame().left + 1.0,
                                titleStrView->Frame().bottom + K_MARGIN / 2.0,
                                0, iconView->Frame().bottom), "StatusWindow::BarberPole");
    m_backView->AddChild (m_barberPole);

    // Recalculate font height for plain font as we don't use bold font anymore
    be_plain_font->GetHeight (&fntHt);
    totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    BStringView *textLabel = new BStringView (BRect (m_barberPole->Frame().right + 2 * K_MARGIN,
                             m_barberPole->Frame().top + totalFontHeight / 2.0,
                             m_barberPole->Frame().right + 2 * K_MARGIN + m_backView->StringWidth (text) +
                             m_backView->StringWidth ("W"), totalFontHeight +
                             m_barberPole->Frame().top + totalFontHeight / 2.0), "StatusWindow:StringView",
                             text, B_FOLLOW_LEFT, B_WILL_DRAW);
    textLabel->ResizeToPreferred();
    textLabel->MoveTo (textLabel->Frame().left, m_barberPole->Frame().bottom - textLabel->Frame().Height());
    m_backView->AddChild (textLabel);

    if (cancel)
    {
        BevelView *edgeView = new BevelView (BRect (-1, textLabel->Frame().bottom + 2 * K_MARGIN,
                                bounds.right - 1, textLabel->Frame().bottom + 2 * K_MARGIN + 1.0),
                                "StatusWindow:EdgeView", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
        m_backView->AddChild (edgeView);

        BButton *cancelButton = new BButton (BRect (bounds.right - K_BUTTON_WIDTH - 2 * K_MARGIN,
                             edgeView->Frame().bottom + K_MARGIN, bounds.right - 2 * K_MARGIN,
                             edgeView->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
                             "StatusWindow:CancelButton", str (S_STOP_OPERATION),
                             new BMessage (M_STOP_OPERATION), B_FOLLOW_RIGHT, B_WILL_DRAW);
        m_backView->AddChild (cancelButton);
        ResizeTo (Frame().Width(), cancelButton->Frame().bottom + K_MARGIN);
    }
    else
        ResizeTo (Frame().Width(), textLabel->Frame().bottom + 2 * K_MARGIN + 4);

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

//=============================================================================================================//

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

//=============================================================================================================//

