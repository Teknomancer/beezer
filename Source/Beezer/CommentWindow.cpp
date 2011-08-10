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

#include <TextView.h>
#include <ScrollView.h>
#include <Button.h>
#include <Screen.h>
#include <Bitmap.h>
#include <StringView.h>

#include <string.h>

#include "CommentWindow.h"
#include "BevelView.h"
#include "LangStrings.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "UIConstants.h"

#include "BetterScrollView.h"

//=============================================================================================================//

CommentWindow::CommentWindow (BWindow *callerWindow, const char *archiveName, const char *commentText,
                  BFont *displayFont)
    : BWindow (BRect (0, 0, 590, 290), str (S_COMMENT_WINDOW_TITLE), B_TITLED_WINDOW,
        B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE)
{
    m_callerWindow = callerWindow;
    if (m_callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (m_callerWindow);
    }

    m_commentText = strdup (commentText);

    BRect bounds (Bounds());
    m_backView = new BevelView (bounds, "CommentWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    // Get font metrics
    BFont font (be_plain_font);
    font_height fntHt;

    font.SetFace (B_BOLD_FACE);
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    // Get comment icon from resource, construct comment holding view etc.
    BBitmap *commentBmp = ResBitmap ("Img:Comment");

    BevelView *sepView1 = new BevelView (BRect (-1, commentBmp->Bounds().Height() + 4 * K_MARGIN,
                                Bounds().right - 1.0, commentBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                "CommentWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView1);
    
    // Add icon view, make it hold the picture
    StaticBitmapView *commentBmpView = new StaticBitmapView (BRect (K_MARGIN * 5, K_MARGIN * 2,
                             commentBmp->Bounds().Width() + K_MARGIN * 5,
                             commentBmp->Bounds().Height() + K_MARGIN * 2), "CommentWindow:commentBmpView",
                             commentBmp);
    commentBmpView->SetViewColor (m_backView->ViewColor());
    AddChild (commentBmpView);
    
    // Add the file name string view (align it vertically with the icon view)
    BStringView *fileNameStr = new BStringView (BRect (commentBmpView->Frame().right + K_MARGIN * 3,
                                    commentBmpView->Frame().top, Bounds().right - 1,
                                    commentBmpView->Frame().top + totalFontHeight),
                                    "CommentWindow:FileNameView", archiveName, B_FOLLOW_LEFT, B_WILL_DRAW);
    fileNameStr->SetFont (&font);
    m_backView->AddChild (fileNameStr);
    fileNameStr->MoveTo (fileNameStr->Frame().left,
        (commentBmpView->Frame().Height() / 2 - totalFontHeight / 2) + totalFontHeight / 2 + 1);
    fileNameStr->ResizeToPreferred ();

    m_textView = new BTextView (BRect (K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                      bounds.right - K_MARGIN - B_V_SCROLL_BAR_WIDTH,
                      bounds.bottom - 2 * K_MARGIN - K_BUTTON_HEIGHT - B_H_SCROLL_BAR_HEIGHT),
                      "CommentWindow:TextView", BRect (2, 2, 100000, 0), B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    
    BetterScrollView *scrollView = new BetterScrollView ("CommentWindow:ScrollView", m_textView,
                                B_FOLLOW_ALL_SIDES,    B_WILL_DRAW, true, true, true, B_PLAIN_BORDER);
    m_backView->AddChild (scrollView);
    
    m_textView->SetWordWrap (false);
    m_textView->SetText (m_commentText);
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
    
    if (displayFont != NULL)
        m_textView->SetFontAndColor (displayFont);
    
    m_saveButton = new BButton (BRect (bounds.right - K_MARGIN - K_BUTTON_WIDTH,
                         bounds.bottom - K_MARGIN - K_BUTTON_HEIGHT, bounds.right - K_MARGIN,
                         bounds.bottom - K_MARGIN), "CommentWindow:SaveButton", 
                         str (S_COMMENT_WINDOW_SAVE), new BMessage (M_SAVE_COMMENT),
                         B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild (m_saveButton);

    m_closeButton = new BButton (BRect (m_saveButton->Frame().left - K_MARGIN - K_BUTTON_WIDTH,
                         bounds.bottom - K_MARGIN - K_BUTTON_HEIGHT,
                         m_saveButton->Frame().left - K_MARGIN, bounds.bottom - K_MARGIN),
                         "CommentWindow:CloseButton", str (S_COMMENT_WINDOW_CLOSE),
                         new BMessage (B_QUIT_REQUESTED), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild (m_closeButton);

    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    float rightLimit = MAX (m_saveButton->Frame().Width() + m_closeButton->Frame().Width() + 3 * K_MARGIN,
                         fileNameStr->Frame().right + K_MARGIN * 5);
    SetSizeLimits (rightLimit, maxH, m_saveButton->Frame().Height() + 2 * K_MARGIN + 50, maxV);
    
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    
    m_textView->MakeFocus (true);
    free ((char*)m_commentText);
    
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

//=============================================================================================================//

bool CommentWindow::QuitRequested ()
{
    if (_prefs_windows.FindBoolDef (kPfCommentWnd, true))
        _prefs_windows.SetRect (kPfCommentWndFrame, Frame());

    return BWindow::QuitRequested();
}

//=============================================================================================================//

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
           m_closeButton->Invoke();
           break;
        }
        
        default:
           BWindow::MessageReceived (message);
           break;
    }
}

//=============================================================================================================//
