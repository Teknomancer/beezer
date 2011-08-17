/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _JOINER_WINDOW_H
#define _JOINER_WINDOW_H

#include <Window.h>
#include <String.h>

class BevelView;

class BStatusBar;
class BButton;
class BMessenger;

#define M_CANCEL               'canc'
#define M_OPERATION_COMPLETE    'opcc'

const char *const kResult =     "result";

class JoinerWindow : public BWindow
{
    public:
        JoinerWindow ();

        // Inherited hooks
        virtual bool        QuitRequested ();
        virtual void        MessageReceived (BMessage *message);

    private:
        // Private hooks
        status_t            ReadSelf ();
        static int32        _joiner (void *arg);

        // Private members
        BevelView          *m_backView;
        BStatusBar         *m_statusBar;
        BButton            *m_cancelBtn;
        BString             m_separatorStr,
                          m_chunkPathStr,
                          m_dirPathStr;
        volatile bool        m_cancel;
        bool                m_joinInProgress;
        BMessenger         *m_messenger;
        thread_id           m_thread;
};

#endif /* _FILE_JOINER_STUB_H */
