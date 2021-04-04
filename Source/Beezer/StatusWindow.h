// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _STATUS_WINDOW_H
#define _STATUS_WINDOW_H

#include <Window.h>

class BarberPole;

class StatusWindow : public BWindow
{
    public:
        StatusWindow(const char* title, BWindow* callerWindow, const char* text, volatile bool* cancel,
                     bool showWindow = true);

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);

    private:
        BarberPole*         m_barberPole;
        volatile bool*      m_cancel;
};

#endif /* _STATUS_WINDOW_H */
