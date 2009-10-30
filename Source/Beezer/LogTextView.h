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

#ifndef _LOG_TEXT_VIEW_H
#define _LOG_TEXT_VIEW_H

#include <TextView.h>

class BPopUpMenu;

const BRect kInternalTextRect (2, 2, 6000, 0);

class LogTextView : public BTextView
{
    public:
        LogTextView (BRect frame, const char *name, uint32 resizeMask, uint32 flags = B_WILL_DRAW);
        LogTextView (BRect frame, const char *name, const BFont *initialFont,
                     const rgb_color *initialColor, uint32 resizeMask, uint32 flags);
        
        // Inherited hooks
        virtual void        MakeFocus (bool focused = true);
        virtual void        MouseDown (BPoint point);
        
        // Additional hooks
        void                Copy ();
        void                SetContextMenu (BPopUpMenu *contextMenu);
        void                AddText (const char *text, bool newLine = true,
                                     bool capitalFirstLetter = false, bool trimLeadingSpaces = true);

    private:
        // Private hooks
        void                InitSelf ();
        
        // Private members
        BPopUpMenu         *m_contextMenu;
};

#endif /* _LOG_TEXT_VIEW_H */
