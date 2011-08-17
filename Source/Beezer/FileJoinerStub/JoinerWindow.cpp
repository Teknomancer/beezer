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

#include <View.h>
#include <Application.h>
#include <StatusBar.h>
#include <Button.h>
#include <CheckBox.h>
#include <Alert.h>
#include <Resources.h>
#include <Entry.h>
#include <Roster.h>
#include <Path.h>
#include <Messenger.h>
#include <Debug.h>

#include <stdio.h>

#include "JoinerWindow.h"
#include "JoinerStrings.h"

#include "UIConstants.h"
#include "Joiner.h"
#include "BevelView.h"
#include "AppConstants.h"

#include "Shared.h"

//=============================================================================================================//

JoinerWindow::JoinerWindow ()
    : BWindow (BRect (100, 100, 450, 210), str (S_JOINER_WINDOW_TITLE), B_TITLED_WINDOW,
        B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_V_RESIZABLE, B_CURRENT_WORKSPACE)
{
    m_backView = new BevelView (Bounds(), "JoinerWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    m_statusBar = new BStatusBar (BRect (2 * K_MARGIN, 2 * K_MARGIN, Bounds().right - 2 * K_MARGIN, 0),
                         "JoinerWindow:StatusBar", str (S_JOINING_FILE), NULL);
    m_statusBar->ResizeToPreferred();
    m_statusBar->ResizeTo (Bounds().right - 4 * K_MARGIN - 1, m_statusBar->Frame().Height());
    m_statusBar->SetResizingMode (B_FOLLOW_LEFT_RIGHT);
    m_statusBar->SetBarHeight (K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor (K_PROGRESS_COLOR);
    m_backView->AddChild (m_statusBar);

    m_cancelBtn = new BButton (BRect (Bounds().right - 2 * K_MARGIN - K_BUTTON_WIDTH,
                         m_statusBar->Frame().bottom + K_MARGIN, Bounds().right - 2 * K_MARGIN,
                         m_statusBar->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
                         "JoinerWindow:CancelBtn", str (S_CANCEL), new BMessage (M_CANCEL),
                         B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild (m_cancelBtn);

    ResizeTo (Frame().Width(), m_cancelBtn->Frame().bottom + 2 * K_MARGIN);

    // Center window on-screen
    CenterOnScreen();

    // Constrain window size
    float minWidth = 350;
    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (minWidth, maxH, minV, maxV);
    if (Frame().Width() < minWidth)
        ResizeTo (minWidth, Frame().Height());

    m_cancel = false;
    m_joinInProgress = false;
    m_messenger = new BMessenger (this);

    status_t result = ReadSelf();
    if (result == BZR_DONE)
        Show();
    else
    {
        (new BAlert ("error", str (S_STUB_ERROR), str (S_OK)))->Go();
        be_app->PostMessage (B_QUIT_REQUESTED);
    }
}

//=============================================================================================================//

bool JoinerWindow::QuitRequested ()
{
    m_cancel = true;
    be_app->PostMessage (B_QUIT_REQUESTED);
    return BWindow::QuitRequested();
}

//=============================================================================================================//

void JoinerWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_OPERATION_COMPLETE:
        {
           status_t result = message->FindInt32 (kResult);
           if (result == BZR_ERROR)
           {
               BAlert *alert = new BAlert ("Error", str (S_JOIN_ERROR), str (S_OK), NULL, NULL,
                                    B_WIDTH_AS_USUAL, B_STOP_ALERT);
               alert->Go();
           }

           snooze (100000);
           PostMessage (B_QUIT_REQUESTED);
           break;
        }

        case M_CANCEL:
        {
           m_cancel = true;
           break;
        }

        case BZR_UPDATE_PROGRESS:
        {
           char percentStr [100];
           float delta = message->FindFloat ("delta");
           int8 percent = (int8)ceil(100 * ((m_statusBar->CurrentValue() + delta) / m_statusBar->MaxValue()));
           sprintf (percentStr, "%d%%", percent);

           BString text = message->FindString ("text");

           m_statusBar->Update (delta, text.String(), percentStr);
           message->SendReply ('DUMB');
           break;
        }

        default:
           return BWindow::MessageReceived (message);
    }
}

//=============================================================================================================//

status_t JoinerWindow::ReadSelf ()
{
    // Reads resource from itself (binary file)
    BResources *res = be_app->AppResources();
    size_t readSize;

    // need to NULL terminate strings read from resource, wow what a head-ache to finally find
    // and fix this :) heh
    char *buf = (char*)res->LoadResource (B_STRING_TYPE, K_FILENAME_ATTRIBUTE, &readSize);
    buf[readSize] = '\0';
    BString fileName = buf;

    char *buf2 = (char*)res->LoadResource (B_STRING_TYPE, K_SEPARATOR_ATTRIBUTE, &readSize);
    buf2[readSize] = '\0';
    m_separatorStr = buf2;

    // Why bother when the user deliberately messes up his resources :) nah, i'm thinking giving detailed
    // error messages will further increase the executable file size which I want to avoid
    if (fileName.Length() <= 0 || m_separatorStr.Length() <= 0)
    {
        PostMessage (B_QUIT_REQUESTED);
        return BZR_ERROR;
    }

    // Now read self (resources)
    BEntry appEntry;
    app_info appInfo;

    be_app->GetAppInfo (&appInfo);
    appEntry.SetTo (&(appInfo.ref), true);

    appEntry.GetParent (&appEntry);
    BPath outputDirPath;
    appEntry.GetPath (&outputDirPath);

    m_dirPathStr = outputDirPath.Path();
    m_chunkPathStr = m_dirPathStr;
    m_chunkPathStr << "/" << fileName;

    m_statusBar->Update (0, str (S_COMPUTING));
    // Call findchunks to find out the total size of the joined file to assign to the
    // progress bar
    int32 fileCount = 0;
    off_t totalSize = 0;
    FindChunks (m_chunkPathStr.String(), m_separatorStr.String(), fileCount, totalSize, &m_cancel);
    m_statusBar->SetMaxValue (totalSize);
    m_statusBar->Update (0, str (S_JOINING_FILE));

    // Now we have all we want to start the join process, then what're we waiting for :)
    m_thread = spawn_thread (_joiner, "_joiner", B_NORMAL_PRIORITY, (void*)this);
    m_joinInProgress = true;
    resume_thread (m_thread);
    return BZR_DONE;
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

int32 JoinerWindow::_joiner (void *arg)
{
    JoinerWindow *wnd = (JoinerWindow*)arg;

    status_t result = JoinFile (wnd->m_chunkPathStr.String(), wnd->m_dirPathStr.String(),
                         wnd->m_separatorStr.String(), wnd->m_messenger, &(wnd->m_cancel));

    BMessage completeMsg (M_OPERATION_COMPLETE);
    completeMsg.AddInt32 (kResult, result);
    wnd->m_messenger->SendMessage (&completeMsg);

    return result;
}

//=============================================================================================================//
