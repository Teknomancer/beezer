// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _WINDOW_MGR_H
#define _WINDOW_MGR_H

#include <SupportDefs.h>

class BList;
class BLocker;
class BMessage;
class BWindow;

extern BLocker _wnd_locker;

class WindowMgr
{
    public:
        WindowMgr();
        ~WindowMgr();

        bool               AddWindow(BWindow* wnd);
        bool               RemoveWindow(BWindow* wnd);
        void               UpdateFrom(BWindow* sourceWnd, bool updateBeApp);
        void               UpdateFrom(BWindow* sourceWnd, BMessage* message, bool updateBeApp);
        int32              CountWindows() const;
        BWindow*           WindowAt(int32 index) const;

    private:
        BList*             m_windowList;

        static int32       m_runCount;
};

#endif /* _WINDOW_MGR_H */
