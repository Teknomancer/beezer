/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _WINDOW_MGR_H
#define _WINDOW_MGR_H

class BWindow;
class BList;
class BLocker;

extern BLocker _wnd_locker;

class WindowMgr
{
    public:
        WindowMgr ();
        ~WindowMgr ();
        
        // Public hooks
        bool                AddWindow (BWindow *wnd);
        bool                RemoveWindow (BWindow *wnd);
        void                UpdateFrom (BWindow *sourceWnd, bool updateBeApp);
        void                UpdateFrom (BWindow *sourceWnd, BMessage *message, bool updateBeApp);
        int32                CountWindows () const;
        BWindow                *WindowAt (int32 index) const;

    private:
        // Private members
        BList                *m_windowList;
        
        static int32        m_runCount;
};

#endif /* _WINDOW_MGR_H */
