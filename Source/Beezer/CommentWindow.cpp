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

#include <Bitmap.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextView.h>

#include <malloc.h>
#include <string.h>

#include "CommentWindow.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"


CommentWindow::CommentWindow (BWindow *callerWindow, const char *archiveName, const char *commentText,
                  BFont *displayFont)
    : BWindow (BRect (0, 0, 590, 290), str (S_COMMENT_WINDOW_TITLE), B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
        B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
    m_callerWindow = callerWindow;
    if (m_callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (m_callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    // Get comment icon from resource, construct comment holding view etc.
    BBitmap *commentBmp = ResBitmap ("Img:Comment");

    // Add icon view, make it hold the picture
    StaticBitmapView *commentBmpView = new StaticBitmapView (BRect (0, 0, commentBmp->Bounds().Width(), commentBmp->Bounds().Height()),
                                                "CommentWindow:commentBmpView", commentBmp);

    // Add the file name string view (align it vertically with the icon view)
    BStringView *fileNameStr = new BStringView ("CommentWindow:FileNameView", archiveName);
    fileNameStr->SetFont (be_bold_font);

    AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
        .AddStrut(30)
        .Add(commentBmpView, 0.0f)
        .Add(fileNameStr, 1.0f)
        .AddGlue()
        .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
    );

    m_textView = new BTextView ("CommentWindow:TextView", displayFont, NULL, B_WILL_DRAW);

    BScrollView *scrollView = new BScrollView ("CommentWindow:ScrollView", m_textView, B_WILL_DRAW, true, true, B_PLAIN_BORDER);

    m_textView->SetWordWrap (false);
    m_textView->SetText (commentText);
    m_textView->DisallowChar (B_INSERT);
    m_textView->DisallowChar (B_ESCAPE);
    m_textView->DisallowChar (B_DELETE);
    m_textView->DisallowChar (B_TAB);
    m_textView->DisallowChar (B_FUNCTION_KEY);
    m_textView->DisallowChar (B_PAGE_UP);
    m_textView->DisallowChar (B_PAGE_DOWN);
    m_textView->DisallowChar (B_HOME);
    m_textView->DisallowChar (B_END);
    m_textView->SetMaxBytes (32768L);

    AddChild(scrollView);

    BButton *saveButton = new BButton ("CommentWindow:SaveButton", str (S_COMMENT_WINDOW_SAVE), new BMessage (M_SAVE_COMMENT));

    BButton *closeButton = new BButton ("CommentWindow:CloseButton", str (S_COMMENT_WINDOW_CLOSE), new BMessage (B_QUIT_REQUESTED));

    AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
        .AddGlue()
        .Add(closeButton, 0.0f)
        .Add(saveButton, 0.0f)
        .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
    );

    // Center our window on screen
    CenterOnScreen();

    m_textView->MakeFocus (true);

    // Load from prefs the window dimensions
    BRect frame;
    if (_prefs_windows.FindBoolDef (kPfCommentWnd, true))
        if (_prefs_windows.FindRect (kPfCommentWndFrame, &frame) == B_OK)
        {
           MoveTo (frame.LeftTop());
           ResizeTo (frame.Width(), frame.Height());
        }

    Show();
}


bool CommentWindow::QuitRequested ()
{
    if (_prefs_windows.FindBoolDef (kPfCommentWnd, true))
        _prefs_windows.SetRect (kPfCommentWndFrame, Frame());

    return BWindow::QuitRequested();
}


void CommentWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_SAVE_COMMENT:
        {
           // Let the main window handle talking with the archiver to write the comments
           // We will just pass it the comment (if any) as a field in a BMessage
           const char *commentStr = m_textView->Text();
           if (commentStr && strlen (commentStr) > 0L)
               message->AddString (kCommentContent, commentStr);

           m_callerWindow->PostMessage (message);
           PostMessage(B_QUIT_REQUESTED);
           break;
        }

        default:
           BWindow::MessageReceived (message);
           break;
    }
}
