// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Bitmap.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>

#include "AppConstants.h"
#include "BarberPole.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"
#include "StatusWindow.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "StatusWindow"
#else
#define B_TRANSLATE(x) x
#endif


StatusWindow::StatusWindow(const char* title, BWindow* callerWindow, const char* text, volatile bool* cancel,
                           bool showWindow)
    : BWindow(BRect(0, 0, 300, 0), title, B_MODAL_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
    if (callerWindow)
    {
        SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset(callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap* icon = BitmapPool::LoadAppVector("Img:WarnAlert", 32, 32);

    StaticBitmapView* iconView = new StaticBitmapView(BRect(0, 0, icon->Bounds().Width(), icon->Bounds().Height()), "StatusWindow:iconView", icon);

    BStringView* titleStrView = new BStringView("StatusWindow:StringView", title);
    titleStrView->SetFont(be_bold_font);
    titleStrView->SetHighColor(K_STARTUP_MAIN_HEADING);

    m_barberPole = new BarberPole(BRect(0, 0, 1, 1), "StatusWindow::BarberPole");

    BStringView* textLabel = new BStringView("StatusWindow:StringView", text);

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
                   .Add(new BButton("StatusWindow:CancelButton", B_TRANSLATE("Cancel"), new BMessage(M_STOP_OPERATION)))
                   .AddGlue()
                  );

    AddChild(layout);

    if (callerWindow != NULL)
    {
        UpdateSizeLimits();
        BRect callerRect(callerWindow->Frame());
        BPoint windowPoint(callerRect.left + callerRect.Width()/2 - Bounds().Width()/2, callerRect.top + callerRect.Height()/2 - Bounds().Height()/2);
        MoveTo(windowPoint);
    }
    else
        CenterOnScreen();

    SetPulseRate(K_BARBERPOLE_PULSERATE);
    m_barberPole->SetValue(true, true);

    if (cancel)
        *cancel = false;
    m_cancel = cancel;

    if (showWindow == true)
        Show();
}


void StatusWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_STOP_OPERATION:
        {
            m_barberPole->SetValue(false, false);
            if (m_cancel)
                *m_cancel = true;
            break;
        }

        case M_CLOSE:
        {
            snooze(70000);
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
