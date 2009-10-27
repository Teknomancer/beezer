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
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
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

#ifndef _COMMENT_WINDOW_H
#define _COMMENT_WINDOW_H

#include <Window.h>

class BTextView;
class BFont;

class BevelView;

class CommentWindow : public BWindow
{
    public:
        CommentWindow (BWindow *callerWindow, const char *archiveName,
                     const char *commentText, BFont *displayFont = NULL);
        
        // Inherited hooks
        virtual bool        QuitRequested ();
        virtual void        MessageReceived (BMessage *message);
        
    private:
        // Private members
        const char         *m_commentText;
        
        BevelView          *m_backView;
        BTextView          *m_textView;
        BButton            *m_closeButton,
                         *m_saveButton;
        BWindow            *m_callerWindow;
};

#endif /* _COMMENT_WINDOW_H */
