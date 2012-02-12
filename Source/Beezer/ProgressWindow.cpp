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
#include <Button.h>
#include <Messenger.h>
#include <Resources.h>
#include <StatusBar.h>
#include <StringView.h>

#include "AppConstants.h"
#include "ArchiveEntry.h"
#include "Archiver.h"
#include "BarberPole.h"
#include "BevelView.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "ProgressWindow.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"



ProgressWindow::ProgressWindow (BWindow *callerWindow, BMessage *actionMessage,
                  BMessenger *&messenger, volatile bool *&cancel)
    : BWindow (BRect (0, 0, 370, 0), NULL, B_MODAL_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
           B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE),
    m_progressCount (1L),
    m_cancel (false)
{
    if (callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (callerWindow);
    }

    BFont font (be_plain_font);
    font.SetFace (B_BOLD_FACE);
    font_height fntHt;
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    BRect bounds (Bounds());
    m_backView = new BevelView (bounds, "ProgressWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    BBitmap *actionIcon = NULL;
    int32 fileCount (0L);
    const char *strOfStrView = NULL, *prepareString = NULL;
    switch (actionMessage->what)
    {
        case M_EXTRACT_TO: case M_EXTRACT_SELECTED_TO:
        {
           actionIcon = ResBitmap ("Img:ExtractStatus");
           fileCount = actionMessage->FindInt32 (kCount);
           actionMessage->FindString (kProgressAction, &strOfStrView);
           actionMessage->FindString (kPreparing, &prepareString);
           break;
        }

        case M_ACTIONS_DELETE:
        {
           actionIcon = ResBitmap ("Img:DeleteStatus");
           fileCount = actionMessage->FindInt32 (kCount);
           actionMessage->FindString (kProgressAction, &strOfStrView);
           actionMessage->FindString (kPreparing, &prepareString);
           break;
        }

        case M_ACTIONS_TEST:
        {
           actionIcon = ResBitmap ("Img:TestStatus");
           fileCount = actionMessage->FindInt32 (kCount);
           actionMessage->FindString (kProgressAction, &strOfStrView);
           actionMessage->FindString (kPreparing, &prepareString);
           break;
        }

        case M_ACTIONS_ADD:
        {
           actionIcon = ResBitmap ("Img:AddStatus");
           fileCount = actionMessage->FindInt32 (kCount);
           actionMessage->FindString (kProgressAction, &strOfStrView);
           actionMessage->FindString (kPreparing, &prepareString);
           break;
        }
    }

    StaticBitmapView *iconView = new StaticBitmapView (BRect (2 * K_MARGIN + 2, 2 * K_MARGIN + 2,
                                    2 * K_MARGIN + 2 + 31.0, 2 * K_MARGIN + 2 + 31.0),
                                    "ProgressWindow:iconView", actionIcon);
    iconView->SetViewColor (m_backView->ViewColor());
    AddChild (iconView);

    BStringView *strView = new BStringView (BRect (iconView->Frame().right + totalFontHeight,
                             K_MARGIN + 3.0, iconView->Frame().right + totalFontHeight +
                             font.StringWidth (strOfStrView) + font.StringWidth ("W"),
                             totalFontHeight + K_MARGIN + 3.0), "ProgressWindow:StringView",
                             strOfStrView, B_FOLLOW_LEFT, B_WILL_DRAW);
    strView->SetFont (&font, B_FONT_ALL);
    strView->SetHighColor (K_STARTUP_MAIN_HEADING);
    strView->SetLowColor (strView->ViewColor());
    m_backView->AddChild (strView);

    m_statusBar = new BStatusBar (BRect (strView->Frame().left + 3 * K_MARGIN,
                         strView->Frame().bottom + K_MARGIN, bounds.right - 2 * K_MARGIN, 0),
                         "ProgressWindow:StatusBar", NULL, NULL);
    m_statusBar->SetText (prepareString);
    m_statusBar->SetResizingMode (B_FOLLOW_LEFT_RIGHT);
    m_statusBar->SetBarHeight (K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor (K_PROGRESS_COLOR);
    m_statusBar->SetMaxValue (fileCount);
    m_fileCount = fileCount;
    m_backView->AddChild (m_statusBar);

    BevelView *edgeView = new BevelView (BRect (-1, m_statusBar->Frame().bottom + 2 * K_MARGIN,
                             bounds.right - 1, m_statusBar->Frame().bottom + 2 * K_MARGIN + 1.0),
                             "ProgressWindow:EdgeView", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (edgeView);

    m_cancelButton = new BButton (BRect (bounds.right - K_BUTTON_WIDTH - 2 * K_MARGIN,
                         edgeView->Frame().bottom + K_MARGIN, bounds.right - 2 * K_MARGIN,
                         edgeView->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
                         "ProgressWindow:CancelButton", str (S_STOP_OPERATION),
                         new BMessage (M_STOP_OPERATION), B_FOLLOW_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (m_cancelButton);
    ResizeTo (Frame().Width(), m_cancelButton->Frame().bottom + K_MARGIN);
    m_backView->ResizeBy(0, K_MARGIN);

    m_barberPole = new BarberPole (BRect (strView->Frame().left, m_statusBar->Frame().bottom - 30, 0,
                      m_statusBar->Frame().bottom - 3), "ProgressWindow::BarberPole");
    m_backView->AddChild (m_barberPole);

    // Center window on-screen & set the constraints
    CenterOnScreen();

    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (Bounds().Width(), maxH, Bounds().Height(), maxV);

    messenger = new BMessenger (this);
    m_messenger = messenger;
    cancel = &m_cancel;
    m_barberPole->SetValue (false, false);
    SetPulseRate (K_BARBERPOLE_PULSERATE);
    Show();
}



ProgressWindow::~ProgressWindow ()
{
    delete m_messenger;
}



void ProgressWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case BZR_UPDATE_PROGRESS:
        {
           message->what = B_UPDATE_STATUS_BAR;

           char fileCountUpdateStr [60];
           sprintf (fileCountUpdateStr, "%ld of %ld", m_progressCount++, m_fileCount);
           message->AddString ("trailing_text", fileCountUpdateStr);

           const char *mainText;
           if (message->FindString ("text", &mainText) != B_OK)
               mainText = "";

           m_statusBar->Update (1.0, mainText, fileCountUpdateStr);
           message->SendReply ('repl');
           break;
        }

        case M_STOP_OPERATION:
        {
           m_barberPole->SetValue (false, false);
           m_cancel = true;
           break;
        }

        case M_CLOSE:
        {
           // The below fills the status bar fully - for example, gzip tests only 1 file
           // even in a tar.gzip (which has many files) thus the status won't fill fully
           // hence, we do this for it - but maybe we shouldn't be doing it ... don't know
           // Hmm, we don't do it for cancelled operations
           if (m_cancel == false)
           {
               float currentValue = m_statusBar->CurrentValue();
               float maxValue = m_statusBar->MaxValue();
               if (currentValue < maxValue)
                  m_statusBar->Update (maxValue - currentValue);
               snooze (65000);
           }

           // Added minor time delay so that window doesn't close before progress bar updates
           snooze (90000);
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


