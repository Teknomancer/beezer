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

#ifndef _SELECT_DIR_PANEL_H
#define _SELECT_DIR_PANEL_H

#include <FilePanel.h>

class SelectDirPanel : public BFilePanel
{
    public:
        SelectDirPanel (file_panel_mode mode = B_OPEN_PANEL, BMessenger *target = 0,
                      const entry_ref *start_directory = 0, uint32 node_flavors = 0,
                      bool allow_multiple_selection = true, BMessage *message = 0, BRefFilter * = 0,
                      bool modal = false, bool hide_when_done = true);
        ~SelectDirPanel ();

        // Inherited hooks
        virtual void        SelectionChanged();
        void               Refresh();
        void               SetMessage (BMessage *msg);

        // Additional hooks
        void               SetCurrentDirButton (const char *label);
        void               UpdateButton ();

    protected:
        // Protected hooks
        void               SendMessage (const BMessenger *target, BMessage *message);

    private:
        // Private members
        BString            m_buttonLabel,
                         m_buttonName;
        BButton           *m_curDirBtn;
};

#endif /* _CURRENT_DIR_FILTER_H */
