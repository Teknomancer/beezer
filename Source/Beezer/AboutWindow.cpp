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

#include <String.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <Message.h>
#include <Alert.h>
#include <Application.h>

#include "AboutWindow.h"
#include "UIConstants.h"

//=============================================================================================================//

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect, uint32 resizeMask,
        uint32 flags)
    : BTextView (frame, name, textRect, resizeMask, flags)
{
    m_curPos = Bounds().top;
    m_rightEdge = Bounds().right;
}

//=============================================================================================================//

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
        const rgb_color *initialColor, uint32 resizeMask, uint32 flags)
    : BTextView (frame, name, textRect, initialFont, initialColor, resizeMask, flags)
{
    m_curPos = Bounds().top;
    m_rightEdge = Bounds().right;
}

//=============================================================================================================//

void MarqueeView::ScrollTo (float x, float y)
{
    // Reset curPos
    m_curPos = y;
    return BTextView::ScrollTo (x, y);
}

//=============================================================================================================//

void MarqueeView::ScrollBy (float dh, float dv)
{
    // Perform the fading effect, curPos records the TOP co-ord of the shading zone
    m_curPos += dv;

    // Render the fade zone
    SetDrawingMode (B_OP_BLEND);
    SetHighColor (255, 255, 255, 255);
    FillRect (BRect (0, m_curPos, m_rightEdge, m_curPos + 5));

    // Restore the original drawing mode for Draw()
    SetDrawingMode (B_OP_COPY);
    return BTextView::ScrollBy (dh, dv);
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

AboutWindow::AboutWindow (const char *compileTimeStr)
    : BWindow (BRect (0, 0, 319, 374), str (S_ABOUT_TITLE), B_MODAL_WINDOW,
        B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE)
{
    SetFeel (B_MODAL_APP_WINDOW_FEEL);
    SetLook (B_MODAL_WINDOW_LOOK);

    // Create the BBitmap objects and set its data with error checking
    BBitmap *titleBmp = BTranslationUtils::GetBitmap ('PNG ', "Img:AboutBox");
    if (titleBmp == NULL)
    {
        Hide();
        (new BAlert ("Error", str (S_ERROR_LOADING_ABOUT_RSRC), str (S_CLOSE_WORD), NULL, NULL,
                      B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT))->Go();
        PostMessage (B_QUIT_REQUESTED);
        Show();
        return;
    }

    BRect bounds (Bounds());
    m_backView = new BView (bounds, "AboutWindow:BackView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    AddChild (m_backView);
    m_backView->SetViewBitmap (titleBmp);

    delete titleBmp;

    m_textView = new MarqueeView (BRect (15, 130, bounds.right - 15, bounds.bottom - 45),
                                "AboutWindow:CreditsView", BRect (0, 0, bounds.right - 2*(15)-5, 0),
                                B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_textView);
    m_textView->SetStylable (true);
    m_textView->MakeSelectable (false);
    m_textView->MakeEditable (false);
    m_textView->SetAlignment (B_ALIGN_CENTER);
    m_textView->SetViewColor (m_backView->ViewColor());
    m_textView->SetFontAndColor (be_plain_font, B_FONT_ALL, &K_BLACK_COLOR);
    m_textView->Hide();

    // Calculate no of '\n's to leave to make the text go to the bottom, calculate the no. of lines
    font_height fntHt;
    m_textView->GetFontHeight (&fntHt);
    int32 noOfLines = (int32)(m_textView->Frame().Height() / (fntHt.ascent + fntHt.descent + fntHt.leading));
    for (int32 i = 0; i < (int32)noOfLines; i++)
        m_lineFeeds << "\n";

    BString formatStr = str (S_ABOUT_STRING);
    #ifdef DEBUG
        formatStr.ReplaceAll ("$DEBUG_BUILD$", str (S_DEBUG_RELEASE));
    #else
        formatStr.ReplaceAll ("$DEBUG_BUILD$", str (S_PUBLIC_RELEASE));
    #endif

    formatStr.ReplaceAll ("$BUILD_DATE$", compileTimeStr);
    free ((char*)compileTimeStr);                  // Free-this as we are passed a ptr in HEAP as per caller Beezer

    formatStr.ReplaceAll ("$S_PROGRAMMING$", str (S_ABOUT_PROGRAMMING));
    formatStr.ReplaceAll ("$S_CREDITS$", str (S_ABOUT_CREDITS));
    formatStr.ReplaceAll ("$S_COLUMN_LIST_VIEW$", str (S_ABOUT_COLUMN_LIST_VIEW));
    formatStr.ReplaceAll ("$S_SPLITPANE$", str (S_ABOUT_SPLITPANE));
    formatStr.ReplaceAll ("$S_BUBBLE_HELP$", str (S_ABOUT_BUBBLE_HELP));
    formatStr.ReplaceAll ("$S_URLVIEW$", str (S_ABOUT_URLVIEW));
    formatStr.ReplaceAll ("$S_BESHARE$", str (S_ABOUT_BESHARE));
    formatStr.ReplaceAll ("$S_7ZIP$", str (S_ABOUT_7ZIP));
    formatStr.ReplaceAll ("$S_LEGAL_MUMBO_JUMBO$", str (S_ABOUT_LEGAL_MUMBO_JUMBO));
    formatStr.ReplaceAll ("$S_DISCLAIMER$", str (S_ABOUT_DISCLAIMER));
    formatStr.ReplaceAll ("$S_SPECIAL_THANKS$", str (S_ABOUT_SPECIAL_THANKS));

    m_creditsText = strdup (formatStr.String());

    m_textView->SetText (m_lineFeeds.String());
    m_textView->Insert (m_lineFeeds.Length(), m_creditsText, strlen (m_creditsText));

    // Dealloc m_creditsText as our textview would have made a copy of it & we don't use it anywhere else
    free ((char*)m_creditsText);

    int32 nSubHeadings = 8;
    BString subHeadings[] =
    {
        str (S_ABOUT_PROGRAMMING),               // 0
        str (S_ABOUT_COLUMN_LIST_VIEW),           // 1
        str (S_ABOUT_SPLITPANE),               // 2
        str (S_ABOUT_BUBBLE_HELP),               // 3
        str (S_ABOUT_URLVIEW),                  // 4
        str (S_ABOUT_BESHARE),                  // 5
        str (S_ABOUT_7ZIP),                      // 6
        str (S_ABOUT_DISCLAIMER)               // 7
    };

    int32 nMainHeadings = 3;
    BString mainHeadings[] =
    {
        str (S_ABOUT_CREDITS),                  // 0
        str (S_ABOUT_LEGAL_MUMBO_JUMBO),        // 1
        str (S_ABOUT_SPECIAL_THANKS)           // 2
    };

    // Search and color sub headings
    BString temp = m_textView->Text();
    int32 strt;
    for (int32 i = 0; i < nSubHeadings; i++)
        if ((strt = temp.FindFirst (subHeadings[i].String())) != B_ERROR)
        {
           m_textView->SetFontAndColor (strt, strt + strlen(subHeadings[i].String()),
                         be_plain_font, B_FONT_ALL, &K_ABOUT_SUB_HEADING);
        }

    // Search and color main headings
    for (int32 i = 0; i < nMainHeadings; i++)
        if ((strt = temp.FindFirst (mainHeadings[i].String())) != B_ERROR)
        {
           m_textView->SetFontAndColor (strt, strt + strlen(mainHeadings[i].String()),
                         be_plain_font, B_FONT_ALL, &K_ABOUT_MAIN_HEADING);
        }

    // Center window on-screen
    CenterOnScreen();

    // Spawn & resume the scroller thread now
    m_textView->Show();
    m_scrollThreadID = spawn_thread (_scroller, "_magic_scroller", B_NORMAL_PRIORITY, (void*)this);

    Show();
    resume_thread (m_scrollThreadID);
}

//=============================================================================================================//

void AboutWindow::Quit ()
{
    be_app_messenger.SendMessage (M_CLOSE_ABOUT);
    return BWindow::Quit();
}

//=============================================================================================================//

void AboutWindow::DispatchMessage (BMessage *message, BHandler *handler)
{
    switch (message->what)
    {
        case B_KEY_DOWN: case B_MOUSE_DOWN:
        {
           // According to BeBook its ok to call Quit() from message loop as it shuts down the message
           // loop (and deletes any pending messages), so this will be the last message to be processed
           Quit();
           break;
        }
    }

    BWindow::DispatchMessage (message, handler);
}

//=============================================================================================================//

int32 AboutWindow::_scroller (void *data)
{
    // This thread function controls the scrolling of the marqueeview
    AboutWindow *wnd = reinterpret_cast<AboutWindow*>(data);
    float textLen, height;
    float ptY;
    BPoint pt;

    // Calculate a few things here so that our loop isn't strained
    if (wnd && wnd->Lock())
    {
        textLen = wnd->m_textView->TextLength() - 1;
        height = wnd->Bounds().Height();
        pt = wnd->m_textView->PointAt (wnd->m_textView->TextLength() - 1);
        wnd->Unlock();
    }
    else
        return 0;

    ptY = pt.y + height;
    MarqueeView *vw = wnd->m_textView;

    // Control the scrolling view
    for (;;)
    {
        if (wnd->Lock() == true)
           vw->ScrollBy (0, 1);
        else
           return 0;

        if (vw->Bounds().bottom > ptY)
           vw->ScrollTo (0, 0);

        wnd->Unlock();
        snooze (K_SCROLL_DELAY);
    }

    return 0;
}

//=============================================================================================================//
