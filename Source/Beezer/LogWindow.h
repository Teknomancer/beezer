// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _LOG_WINDOW_H
#define _LOG_WINDOW_H

#include <Window.h>


class BFont;

class LogWindow : public BWindow
{
    public:
        LogWindow(BWindow* callerWindow, const char* title, const char* logText, BFont* displayFont = NULL);

        // Inherited hooks
        virtual bool        QuitRequested();
};

#endif /* _LOG_WINDOW_H */
