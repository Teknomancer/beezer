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
#include <Autolock.h>
#include <List.h>
#include <Message.h>
#include <Window.h>

#include "MsgConstants.h"
#include "WindowMgr.h"

int32 WindowMgr::m_runCount = 0;
BLocker _wnd_locker ("_window_mgr_lock", true);



WindowMgr::WindowMgr ()
{
    // You only need one instance per application - critical for proper window management
    if (atomic_add (&m_runCount, 1) == 0)
        m_windowList = new BList (5L);
    else
        debugger ("only one WindowMgr instance allowed/necessary");
}



WindowMgr::~WindowMgr ()
{
    delete m_windowList;
}



bool WindowMgr::AddWindow (BWindow *wnd)
{
    BAutolock autolocker (_wnd_locker);

    if (autolocker.IsLocked())
        if (m_windowList->HasItem (reinterpret_cast<void*>(wnd)) == false)
           return m_windowList->AddItem (reinterpret_cast<void*>(wnd));

    return false;
}



bool WindowMgr::RemoveWindow (BWindow *wnd)
{
    BAutolock autolocker (_wnd_locker);
    bool result = false;

    if (autolocker.IsLocked())
    {
        result = m_windowList->RemoveItem (reinterpret_cast<void*>(wnd));
        UpdateFrom (wnd, false);
    }

    return result;
}



BWindow* WindowMgr::WindowAt (int32 index) const
{
    return reinterpret_cast<BWindow*>(m_windowList->ItemAtFast (index));
}



int32 WindowMgr::CountWindows () const
{
    return m_windowList->CountItems();
}



void WindowMgr::UpdateFrom (BWindow *sourceWnd, bool updateBeApp)
{
    BAutolock autolocker (_wnd_locker);
    if (!autolocker.IsLocked())
        return;

    BMessage msg (M_BROADCAST_STATUS);
    msg.AddPointer (kWindowList, reinterpret_cast<void**>(&m_windowList));
    UpdateFrom (sourceWnd, &msg, updateBeApp);
}



void WindowMgr::UpdateFrom (BWindow *sourceWnd, BMessage *message, bool updateBeApp)
{
    BAutolock autolocker (_wnd_locker);
    if (!autolocker.IsLocked())
        return;

    int32 winCount = m_windowList->CountItems();
    BWindow *destWnd = NULL;
    message->AddPointer (kWindowList, reinterpret_cast<void**>(&m_windowList));

    for (int32 i = 0; i < winCount; i++)
    {
        destWnd = reinterpret_cast<BWindow*>(m_windowList->ItemAtFast(i));
        if (destWnd != NULL && sourceWnd != destWnd)
           destWnd->PostMessage (message);
    }

    if (updateBeApp)
        be_app->PostMessage (message);
}


