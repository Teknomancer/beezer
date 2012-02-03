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

#include <Button.h>
#include <Path.h>
#include <String.h>
#include <Window.h>

#include "SelectDirPanel.h"



SelectDirPanel::SelectDirPanel (file_panel_mode mode, BMessenger *target, const entry_ref *start_directory,
                  uint32 node_flavors, bool allow_multiple_selection, BMessage *message, BRefFilter *filter,
                  bool modal, bool hide_when_done)
    : BFilePanel (mode, target, start_directory, B_DIRECTORY_NODE, allow_multiple_selection, message, filter,
               modal, hide_when_done)
{
    m_buttonName = "bzr:special_button";

    Window()->LockLooper();

    BButton *cancelBtn = (BButton*)Window()->FindView ("cancel button");
    m_curDirBtn = new BButton (BRect (cancelBtn->Frame().left - 20 - 185, cancelBtn->Frame().top,
                         cancelBtn->Frame().left - 20, cancelBtn->Frame().bottom), m_buttonName.String(),
                         NULL, message, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
    m_curDirBtn->SetTarget (target->Target(NULL));

    // Tweak the default button (turn it off) so <ENTER> doesn't select the folder, instead
    // enters the folder in the BFilePanel's list
    ((BButton*)Window()->FindView ("default button"))->MakeDefault (false);

    Window()->ChildAt(0L)->AddChild (m_curDirBtn);
    Window()->UnlockLooper();
    UpdateButton();
}



SelectDirPanel::~SelectDirPanel()
{
}



void SelectDirPanel::SetCurrentDirButton (const char *label)
{
    m_buttonLabel = label;
    UpdateButton();
}



void SelectDirPanel::SelectionChanged ()
{
    UpdateButton();
    BFilePanel::SelectionChanged();
}



void SelectDirPanel::Refresh()
{
    UpdateButton ();
    BFilePanel::Refresh();
}



void SelectDirPanel::UpdateButton ()
{
    entry_ref dirRef;
    GetPanelDirectory (&dirRef);

    BPath dirPath (&dirRef);
    BString btnLabel = m_buttonLabel;
    btnLabel << " " << '\'' << dirPath.Leaf() << '\'';

    Window()->LockLooper();
    BButton *curDirBtn = (BButton*)Window()->FindView (m_buttonName.String());
    if (curDirBtn)
    {
        curDirBtn->SetLabel (btnLabel.String());
        BMessage *msg = curDirBtn->Message();
        if (msg)
        {
           msg->RemoveName ("refs");
           msg->AddRef ("refs", &dirRef);
        }
    }
    Window()->UnlockLooper();
}



void SelectDirPanel::SetMessage (BMessage *msg)
{
    BMessage *curDirMsg = new BMessage (*msg);
    m_curDirBtn->SetMessage (curDirMsg);
    UpdateButton ();

    BFilePanel::SetMessage (msg);
}



void SelectDirPanel::SendMessage (const BMessenger *target, BMessage *message)
{
    // Bug-fix: 0.07 -- a possibility because BFilePanel has a "Favourites" menu that lists
    // files also, and apparently BFilePanel sends the message when a file is selected from Favs,
    // this is fixed here
    entry_ref ref;
    message->FindRef ("refs", &ref);
    BEntry entry (&ref, true);

    if (entry.IsDirectory() == true)
        return BFilePanel::SendMessage (target, message);
}


