// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <Messenger.h>
#include <Resources.h>
#include <StatusBar.h>
#include <StringView.h>

#include "AppConstants.h"
#include "ArchiveEntry.h"
#include "Archiver.h"
#include "BarberPole.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "ProgressWindow.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ProgressWindow"
#else
#define B_TRANSLATE(x) x
#endif


ProgressWindow::ProgressWindow(BWindow* callerWindow, BMessage* actionMessage,
                               BMessenger*& messenger, volatile bool*& cancel)
    : BWindow(BRect(0, 0, 370, 0), NULL, B_MODAL_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE | B_AUTO_UPDATE_SIZE_LIMITS),
    m_progressCount(1L),
    m_cancel(false)
{
    if (callerWindow)
    {
        SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset(callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap* actionIcon = NULL;
    int32 fileCount(0L);
    const char* strOfStrView = NULL, *prepareString = NULL;
    switch (actionMessage->what)
    {
        case M_EXTRACT_TO: case M_EXTRACT_SELECTED_TO:
        {
            actionIcon = BitmapPool::LoadAppVector("Img:ExtractStatus", 32, 32);
            fileCount = actionMessage->FindInt32(kCount);
            actionMessage->FindString(kProgressAction, &strOfStrView);
            actionMessage->FindString(kPreparing, &prepareString);
            break;
        }

        case M_ACTIONS_DELETE:
        {
            actionIcon = BitmapPool::LoadAppVector("Img:DeleteStatus", 32, 32);
            fileCount = actionMessage->FindInt32(kCount);
            actionMessage->FindString(kProgressAction, &strOfStrView);
            actionMessage->FindString(kPreparing, &prepareString);
            break;
        }

        case M_ACTIONS_TEST:
        {
            actionIcon = BitmapPool::LoadAppVector("Img:TestStatus", 32, 32);
            fileCount = actionMessage->FindInt32(kCount);
            actionMessage->FindString(kProgressAction, &strOfStrView);
            actionMessage->FindString(kPreparing, &prepareString);
            break;
        }

        case M_ACTIONS_ADD:
        {
            actionIcon = BitmapPool::LoadAppVector("Img:AddStatus", 32, 32);
            fileCount = actionMessage->FindInt32(kCount);
            actionMessage->FindString(kProgressAction, &strOfStrView);
            actionMessage->FindString(kPreparing, &prepareString);
            break;
        }
    }

    StaticBitmapView* iconView = new StaticBitmapView(BRect(0, 0, actionIcon->Bounds().Width(), actionIcon->Bounds().Height()),
            "ProgressWindow:iconView", actionIcon);

    BStringView* strView = new BStringView("ProgressWindow:StringView", strOfStrView);
    strView->SetFont(be_bold_font);
    strView->SetHighColor(K_STARTUP_MAIN_HEADING);

    m_statusBar = new BStatusBar("ProgressWindow:StatusBar", NULL, NULL);
    m_statusBar->SetText(prepareString);
    m_statusBar->SetBarHeight(K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor(K_PROGRESS_COLOR);
    m_statusBar->SetMaxValue(fileCount);
    m_fileCount = fileCount;

    m_cancelButton = new BButton("ProgressWindow:CancelButton", B_TRANSLATE(skCancelString),
                                 new BMessage(M_STOP_OPERATION));

    // FIXME? this doesn't show up unless the SetValue call is changed below
    // leaving it as is for now
    m_barberPole = new BarberPole(BRect(0, 0, 1, 1), "ProgressWindow::BarberPole");

    AddChild(BGroupLayoutBuilder(B_VERTICAL)
             .AddGroup(B_HORIZONTAL)
             .Add(iconView, 0)
             .AddGroup(B_VERTICAL, 0)
             .AddGroup(B_HORIZONTAL)
             .Add(strView, 0)
             .AddGlue()
             .End()
             .AddGroup(B_HORIZONTAL)
             .Add(m_barberPole, 0)
             .Add(m_statusBar)
             .End()
             .End()
             .End()
             .AddGroup(B_HORIZONTAL)
             .AddGlue()
             .Add(m_cancelButton, 0)
             .End()
             .SetInsets(4 * K_MARGIN, 2 * K_MARGIN, 4 * K_MARGIN, 2 * K_MARGIN)
            );

    if (callerWindow != NULL)
    {
        UpdateSizeLimits();
        BRect callerRect(callerWindow->Frame());
        BPoint windowPoint(callerRect.left + callerRect.Width()/2 - Bounds().Width()/2, callerRect.top + callerRect.Height()/2 + Bounds().Height()/2);
        MoveTo(windowPoint);
    }
    else
        CenterOnScreen();

    messenger = new BMessenger(this);
    m_messenger = messenger;
    cancel = &m_cancel;
    m_barberPole->SetValue(false, false);
    SetPulseRate(K_BARBERPOLE_PULSERATE);
    Show();
}


ProgressWindow::~ProgressWindow()
{
    delete m_messenger;
}


void ProgressWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case BZR_UPDATE_PROGRESS:
        {
            message->what = B_UPDATE_STATUS_BAR;

            BString fileCountUpdateStr;
            fileCountUpdateStr.SetToFormat("%ld of %ld", m_progressCount++, m_fileCount);
            message->AddString("trailing_text", fileCountUpdateStr);

            const char* mainText;
            if (message->FindString("text", &mainText) != B_OK)
                mainText = "";

            m_statusBar->Update(1.0, mainText, fileCountUpdateStr);
            message->SendReply('repl');
            break;
        }

        case M_STOP_OPERATION:
        {
            m_barberPole->SetValue(false, false);
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
                    m_statusBar->Update(maxValue - currentValue);
                snooze(65000);
            }

            // Added minor time delay so that window doesn't close before progress bar updates
            snooze(90000);
            m_barberPole->SetValue(false, false);
            Quit();
            break;
        }

        default:
        {
            BWindow::MessageReceived(message);
            break;
        }
    }
}
