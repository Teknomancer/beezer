/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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

#ifndef _PROGRESS_WINDOW_H
#define _PROGRESS_WINDOW_H

#include <Window.h>
#include <Messenger.h>

class BButton;

class BevelView;
class BarberPole;

class ProgressWindow : public BWindow
{
    public:
        ProgressWindow (BWindow *callerWindow, BMessage *actionMessage,
                      BMessenger *&messenger, volatile bool *&cancel);
        virtual ~ProgressWindow ();
        
        // Inherited hooks
        virtual void        MessageReceived (BMessage *message);
        
    protected:
        // Protected members
        BevelView          *m_backView;
        BarberPole         *m_barberPole;
        BStatusBar         *m_statusBar;
        BButton            *m_cancelButton;
        int32               m_fileCount,
                          m_progressCount;
        char                m_updateText[B_PATH_NAME_LENGTH + 1];
        volatile bool        m_cancel;
        BMessenger         *m_messenger;
};

#endif /* _PROGRESS_WINDOW_H */
