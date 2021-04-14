// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "CommentWindow.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"

#include <Bitmap.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextView.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CommentWindow"
#else
#define B_TRANSLATE(x) x
#endif

#include <cstring>


CommentWindow::CommentWindow(BWindow* callerWindow, const char* archiveName, const char* commentText,
                             BFont* displayFont)
    : BWindow(BRect(0, 0, 590, 290), BZ_TR(kCommentString), B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
      m_callerWindow(callerWindow),
      m_textView(NULL)
{
    if (m_callerWindow)
    {
        SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset(m_callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    // Get comment icon from resource, construct comment holding view etc.
    BBitmap* commentBmp = BitmapPool::LoadAppVector("Img:Comment", 32, 32);

    // Add icon view, make it hold the picture
    StaticBitmapView* commentBmpView = new StaticBitmapView(BRect(0, 0, commentBmp->Bounds().Width(), commentBmp->Bounds().Height()),
                                                            "CommentWindow:commentBmpView", commentBmp);

    // Add the file name string view (align it vertically with the icon view)
    BStringView* fileNameStr = new BStringView("CommentWindow:FileNameView", archiveName);
    fileNameStr->SetFont(be_bold_font);

    AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
             .AddStrut(30)
             .Add(commentBmpView, 0.0f)
             .Add(fileNameStr, 1.0f)
             .AddGlue()
             .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
            );

    m_textView = new BTextView("CommentWindow:TextView", displayFont, NULL, B_WILL_DRAW);

    BScrollView* scrollView = new BScrollView("CommentWindow:ScrollView", m_textView, B_WILL_DRAW, true, true, B_PLAIN_BORDER);

    m_textView->SetWordWrap(false);
    m_textView->SetText(commentText);
    m_textView->DisallowChar(B_INSERT);
    m_textView->DisallowChar(B_ESCAPE);
    m_textView->DisallowChar(B_DELETE);
    m_textView->DisallowChar(B_TAB);
    m_textView->DisallowChar(B_FUNCTION_KEY);
    m_textView->DisallowChar(B_PAGE_UP);
    m_textView->DisallowChar(B_PAGE_DOWN);
    m_textView->DisallowChar(B_HOME);
    m_textView->DisallowChar(B_END);
    m_textView->SetMaxBytes(32768L);

    AddChild(scrollView);

    BButton* saveButton = new BButton("CommentWindow:SaveButton", BZ_TR(kSaveString),
                                      new BMessage(M_SAVE_COMMENT));

    BButton* closeButton = new BButton("CommentWindow:CloseButton", BZ_TR(kCloseWindowString),
                                       new BMessage(B_QUIT_REQUESTED));

    AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
             .AddGlue()
             .Add(closeButton, 0.0f)
             .Add(saveButton, 0.0f)
             .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
            );

    // Center our window on screen
    CenterOnScreen();

    m_textView->MakeFocus(true);

    // Load from prefs the window dimensions
    BRect frame;
    if (_prefs_windows.FindBoolDef(kPfCommentWnd, true))
        if (_prefs_windows.FindRect(kPfCommentWndFrame, &frame) == B_OK)
        {
            MoveTo(frame.LeftTop());
            ResizeTo(frame.Width(), frame.Height());
        }

    Show();
}


bool CommentWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef(kPfCommentWnd, true))
        _prefs_windows.SetRect(kPfCommentWndFrame, Frame());

    return BWindow::QuitRequested();
}


void CommentWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_SAVE_COMMENT:
        {
            // Let the main window handle talking with the archiver to write the comments
            // We will just pass it the comment (if any) as a field in a BMessage
            const char* commentStr = m_textView->Text();
            if (commentStr && strlen(commentStr) > 0L)
                message->AddString(kCommentContent, commentStr);

            m_callerWindow->PostMessage(message);
            PostMessage(B_QUIT_REQUESTED);
            break;
        }

        default:
            BWindow::MessageReceived(message);
            break;
    }
}
