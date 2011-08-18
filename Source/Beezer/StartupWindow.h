/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _STARTUP_WINDOW_H
#define _STARTUP_WINDOW_H

#include <Window.h>

class BBitmap;
class BPopUpMenu;

class BeezerStringView;
class BevelView;
class ImageButton;
class RecentMgr;

class BubbleHelper;

#define M_CLOSE_STARTUP 'stcl'

class StartupWindow : public BWindow
{
    public:
        StartupWindow (RecentMgr *recentMgr, BubbleHelper *helper, bool startup);

        // Inherited hooks
        virtual void        MessageReceived (BMessage *message);
        virtual void        Quit ();
        virtual bool        QuitRequested ();

    protected:
        // Protected members
        BevelView          *m_backView;
        BeezerStringView    *m_headingView;
        ImageButton        *m_createBtn,
                         *m_openBtn,
                         *m_openRecentBtn,
                         *m_toolsBtn,
                         *m_prefsBtn;
        RecentMgr          *m_recentMgr;
        BubbleHelper        *m_bubbleHelper;
        BPopUpMenu         *m_recentMenu,
                         *m_toolsMenu;
};

#endif /* _STARTUP_WINDOW_H */
