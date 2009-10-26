/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Somebody)
 *    See "License.txt" for licensing info.
*/

#include <String.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <Message.h>
#include <Screen.h>
#include <Alert.h>
#include <Application.h>

#include "AboutWindow.h"
#include "UIConstants.h"

//=============================================================================================================//

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect, uint32 resizeMask,
        uint32 flags)
    : BTextView (frame, name, textRect, resizeMask, flags)
{
    curPos = Bounds().top;
    rightEdge = Bounds().right;
}

//=============================================================================================================//

MarqueeView::MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
        const rgb_color *initialColor, uint32 resizeMask, uint32 flags)
    : BTextView (frame, name, textRect, initialFont, initialColor, resizeMask, flags)
{
    curPos = Bounds().top;
    rightEdge = Bounds().right;
}

//=============================================================================================================//

void MarqueeView::ScrollTo (float x, float y)
{
    // Reset curPos
    curPos = y;
    BTextView::ScrollTo (x, y);
}

//=============================================================================================================//

void MarqueeView::ScrollBy (float dh, float dv)
{
    // Perform the fading effect, curPos records the TOP co-ord of the shading zone
    curPos += dv;

    // Render the fade zone
    SetDrawingMode (B_OP_BLEND);
    SetHighColor (255, 255, 255, 255);
    FillRect (BRect (0, curPos, rightEdge, curPos + 5));

    // Restore the original drawing mode for Draw()
    SetDrawingMode (B_OP_COPY);
    BTextView::ScrollBy (dh, dv);
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

AboutWindow::AboutWindow (const char *compileTimeStr)
    : BWindow (BRect (0, 0, 300, 100+20), str (S_ABOUT_TITLE), B_MODAL_WINDOW,
        B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE)
{
    SetFeel (B_MODAL_APP_WINDOW_FEEL);

    m_heightOfExpandedWindow = 255;
    
    // Create the BBitmap objects and set its data with error checking
    BBitmap *titleBmp = BTranslationUtils::GetBitmap ('PNG ', "Img:AboutTitle");
    m_separatorBmp = BTranslationUtils::GetBitmap ('PNG ', "Img:AboutSeparator");
    if (titleBmp == NULL || m_separatorBmp == NULL)
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
    m_backView->SetViewColor (K_WHITE_COLOR);
    AddChild (m_backView);
    
    m_titleView = new BView (BRect (0, 0, bounds.right, 100), "AboutWindow:TitleView", B_FOLLOW_LEFT,
                      B_WILL_DRAW);
    m_backView->AddChild (m_titleView);
    m_titleView->SetViewBitmap (titleBmp);
    
    m_separatorView = new BView (BRect (0, bounds.bottom - 30, bounds.right, bounds.bottom - 10),
                      "AboutWindow:SeparatorView", B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild (m_separatorView);
    m_separatorView->SetViewBitmap (m_separatorBmp);
    
    delete titleBmp;

    m_textView = new MarqueeView (BRect (5, m_titleView->Frame().bottom - 4,
                                bounds.right - 5,
                                m_heightOfExpandedWindow - m_separatorView->Frame().Height() - 10),
                                "AboutWindow:CreditsView", BRect (0, 0, bounds.right - 5, 0),
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
    int32 noOfLines = (int32)(m_textView->Frame().Height() / fntHt.ascent) - 1;
    for (int32 i = 1; i < (int32)noOfLines; i++)
        m_lineFeeds << "\n";

    BString formatStr = str (S_ABOUT_STRING);
    #if DEBUG
        formatStr.ReplaceAll ("$DEBUG_BUILD$", "Debug Release");
    #else
        formatStr.ReplaceAll ("$DEBUG_BUILD$", "Public Release");
    #endif

    formatStr.ReplaceAll ("$BUILD_DATE$", compileTimeStr);
    free ((char*)compileTimeStr);                  // Free-this as we are passed a ptr in HEAP as per caller Beezer

    formatStr.ReplaceAll ("$S_PROGRAMMING$", str (S_ABOUT_PROGRAMMING));
    formatStr.ReplaceAll ("$S_CREDITS$", str (S_ABOUT_CREDITS));
    formatStr.ReplaceAll ("$S_COLUMN_LIST_VIEW$", str (S_ABOUT_COLUMN_LIST_VIEW));
    formatStr.ReplaceAll ("$S_BUBBLE_HELP$", str (S_ABOUT_BUBBLE_HELP));
    formatStr.ReplaceAll ("$S_URLVIEW$", str (S_ABOUT_URLVIEW));
    formatStr.ReplaceAll ("$S_BESHARE$", str (S_ABOUT_BESHARE));
    formatStr.ReplaceAll ("$S_LEGAL_MUMBO_JUMBO$", str (S_ABOUT_LEGAL_MUMBO_JUMBO));
    formatStr.ReplaceAll ("$S_DISCLAIMER$", str (S_ABOUT_DISCLAIMER));
    formatStr.ReplaceAll ("$S_SPECIAL_THANKS$", str (S_ABOUT_SPECIAL_THANKS));
    m_creditsText = strdup (formatStr.String());
    
    m_textView->SetText (m_lineFeeds.String());
    m_textView->Insert (m_lineFeeds.Length(), m_creditsText, strlen (m_creditsText));

    // Dealloc m_creditsText as our textview would have made a copy of it & we don't use it anywhere else
    free ((char*)m_creditsText);

    int32 nSubHeadings = 6;
    BString subHeadings[] =
    {
        str (S_ABOUT_PROGRAMMING),               // 0
        str (S_ABOUT_COLUMN_LIST_VIEW),           // 1
        str (S_ABOUT_BUBBLE_HELP),               // 2
        str (S_ABOUT_URLVIEW),                  // 3
        str (S_ABOUT_BESHARE),                  // 4
        str (S_ABOUT_DISCLAIMER)               // 5
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
    
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    MoveBy (0, -Frame().Height() / 2);

    Show();
    PostMessage (M_ANIMATE_SLIDE);
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
        case M_ANIMATE_SLIDE:
        {
           float height = Frame().Height();
           while (height < m_heightOfExpandedWindow)
           {
               ResizeBy (0, 1.0f);
               height++;
               m_separatorView->SetViewBitmap (m_separatorBmp);
               snooze (K_EXPAND_DELAY);
           }

           // We don't need to have a copy of this anymore (the BView has it)
           delete m_separatorBmp;
           
           // Spawn & resume the scroller thread now
           m_textView->Show();
           m_scrollThreadID = spawn_thread (_scroller, "Magic_Scroll", B_NORMAL_PRIORITY, (void*)this);
           resume_thread (m_scrollThreadID);
           
           break;
        }
        
        case B_KEY_DOWN: case B_MOUSE_DOWN:
        {
           // According to BeBook its ok to call Quit() from message loop as it shuts down the message
           // loop (and deletes any pending messages, so this will be the last message to be processed
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
