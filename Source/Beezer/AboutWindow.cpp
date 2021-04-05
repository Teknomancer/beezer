// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Message.h>
#include <String.h>
#include <TranslationUtils.h>

#include <malloc.h>

#include "AboutWindow.h"
#include "AppConstants.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AboutWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_CONTEXT(x, y) x
#endif

const char* kAboutText =
        "Version %appversion%\n"
        "%hdr_debug%\n\n"
        "Compiled on:\n%hdr_builddate%\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "%hdr_programming%\n%appauthor%\n%authoremail%\n\n"
        "%appauthor2%\n%author2email%\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "%hdr_credits%\n\n"
        "%hdr_clv%\nBrian Tietz\n\n"
        "%hdr_splitview%\nYNOP Talton\n\n"
        "%hdr_7zip%\nMarcin Konicki\n\n"
        "%hdr_docupdates%\nhumdinger\n\n"
        "Thank you for your contributions\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "A special round of applause\n"
        "to BeShare/IRC members:\n\n"
        "Zartan\nxyphn\nBiPolar\nTechnix\ntschertel\nDaaT\nEsperantolo\nIcewarp\nAlienSoldier\nSoulbender\ndisreali\n"
        "RepairmanJack\nJack Burton\nslaad\nBegasus\nWilliam Kakes\nJeremy Friesner\n"
        "BeGeistert\nBGA\nhUMUNGUs\nmmu_man\nmahlzeit\nBryan\nPahtz\nfyysik\nSir Mik\nIngenu\nTenzin\nand "
        "others…\n\n"
        "for testing, feedback, suggestions\n"
        "& some timely help as well… :)\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "Also thanks to:\n\n"
        "BeIDE\nbdb\nEvaluator\nRL's Icon Collection\nJess Tipton (BeWicked icons)\n\n"
        "… for various reasons :^)\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "%hdr_legal%\n\n"
        "This program is distributed under\n"
        "the 3-clause BSD license.\n\n"

        "For third party code, the license's\n"
        "terms and conditions are explicitly\n"
        "stated and the author disclaimed of\n"
        "any and all liabilities.\n\n"

        "For the full license read the\n"
        "License section of the documentation\n\n"
        "*  *  *\n\n\n\n\n\n\n\n\n\n"

        "%hdr_disclaimer%\n\n"
        "Because the software is licensed\n"
        "free of charge, there is no warranty\n"
        "for the software. The copyright\n"
        "holders and/or other parties provide\n"
        "the software \"AS IS\" without warranty\n"
        "of any kind, either expressed or\n"
        "implied, including, but not limited to,\n"
        "the implied warranties of merchantability\n"
        "and fitness for a particular purpose.\n"
        "The entire risk as to the quality and\n"
        "performance of the software is with you.\n"
        "Should the software prove defective, you\n"
        "assume the cost of all necessary\n"
        "servicing, repair or correction.\n\n"

        "In no event will the copyright holder,\n"
        "or any other party who may modify and/or\n"
        "redistribute the software as permitted\n"
        "above, be liable to you for damages,\n"
        "including any general, special, incidental\n"
        "or consequential damages arising out of\n"
        "the use or inability to use the software\n"
        "(including but not limited to the loss of\n"
        "data or data being rendered inaccurate or\n"
        "losses sustained by you or third parties\n"
        "or a failure of the software to operate\n"
        "with any other programs), even if such\n"
        "holder or other party has been advised\n"
        "of the possibility of such damages.\n\n\n\n\n\n\n\n\n"

        "%hdr_thanks%\n\n"
        "Be Inc., for making this OS\n"
        "in the first place\n\n"
        "Haiku Inc. for their efforts with\n"
        "Haiku\n\n"
        "BeBits.com, BeGroovy.com, BeZip.de and\n"
        "other BeOS related sites for their\n"
        "continued enthusiasm and effort!\n\n"
        "BeOS, Haiku programmers, designers, artists for\n"
        "their contributions to the OS' growth\n\n"
        "and a big applause goes to the\n"
        "community\n\n*  *  *\n\n\n\n\n\n\n\n\n\n"
        "OK…\n\nYou can close this window now :)\n\n\n\n\n";


MarqueeView::MarqueeView(BRect frame, const char* name, BRect textRect, uint32 resizeMask,
                         uint32 flags)
    : BTextView(frame, name, textRect, resizeMask, flags)
{
    m_curPos = Bounds().top;
    m_rightEdge = Bounds().right;
}


MarqueeView::MarqueeView(BRect frame, const char* name, BRect textRect, const BFont* initialFont,
                         const rgb_color* initialColor, uint32 resizeMask, uint32 flags)
    : BTextView(frame, name, textRect, initialFont, initialColor, resizeMask, flags)
{
    m_curPos = Bounds().top;
    m_rightEdge = Bounds().right;
}


void MarqueeView::ScrollTo(float x, float y)
{
    // Reset curPos
    m_curPos = y;
    return BTextView::ScrollTo(x, y);
}


void MarqueeView::ScrollBy(float dh, float dv)
{
    // Perform the fading effect, curPos records the TOP co-ord of the shading zone
    m_curPos += dv;

    // Render the fade zone
    SetDrawingMode(B_OP_BLEND);
    SetHighColor(255, 255, 255, 255);
    FillRect(BRect(0, m_curPos, m_rightEdge, m_curPos + 5));

    // Restore the original drawing mode for Draw()
    SetDrawingMode(B_OP_COPY);
    return BTextView::ScrollBy(dh, dv);
}


AboutWindow::AboutWindow(const char* versionStr, const char* compileTimeStr)
    : BWindow(BRect(0, 0, 319, 374), B_TRANSLATE_COMMENT("About", "title of the window"), B_MODAL_WINDOW,
              B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE)
{
    // Create the BBitmap objects and set its data with error checking
    BBitmap* titleBmp = BTranslationUtils::GetBitmap('PNG ', "Img:AboutBox");
    if (titleBmp == NULL)
    {
        Hide();
        (new BAlert("Error", B_TRANSLATE("An error was encountered while trying to load resources for the About window."),
                    B_TRANSLATE_CONTEXT("Close window", K_I18N_COMMON), NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING,
                    B_STOP_ALERT))->Go();
        PostMessage(B_QUIT_REQUESTED);
        Show();
        return;
    }

    BRect bounds(Bounds());
    m_backView = new BView(bounds, "AboutWindow:BackView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    AddChild(m_backView);
    m_backView->SetViewBitmap(titleBmp);

    delete titleBmp;

    m_textView = new MarqueeView(BRect(15, 130, bounds.right - 15, bounds.bottom - 45),
                                 "AboutWindow:CreditsView", BRect(0, 0, bounds.right - 2 * (15) - 5, 0),
                                 B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(m_textView);
    m_textView->SetStylable(true);
    m_textView->MakeSelectable(false);
    m_textView->MakeEditable(false);
    m_textView->SetAlignment(B_ALIGN_CENTER);
    m_textView->SetViewColor(m_backView->ViewColor());
    rgb_color textColor = ui_color(B_DOCUMENT_TEXT_COLOR);
    m_textView->SetFontAndColor(be_plain_font, B_FONT_ALL, &textColor);
    m_textView->Hide();

    // Calculate no of '\n's to leave to make the text go to the bottom, calculate the no. of lines
    font_height fntHt;
    m_textView->GetFontHeight(&fntHt);
    int32 noOfLines = (int32)(m_textView->Frame().Height() / (fntHt.ascent + fntHt.descent + fntHt.leading));
    for (int32 i = 0; i < (int32)noOfLines; i++)
        m_lineFeeds << "\n";

    BString formatStr(kAboutText);
#ifdef DEBUG
    formatStr.ReplaceAll("%hdr_debug%", B_TRANSLATE("Debug release"));
#else
    formatStr.ReplaceAll("%hdr_debug%", B_TRANSLATE("Public release"));
#endif

    formatStr.ReplaceAll("%appversion%", versionStr);
    formatStr.ReplaceAll("%appauthor%", K_APP_AUTHOR);
    formatStr.ReplaceAll("%appauthor2%", K_APP_AUTHOR_2);
    formatStr.ReplaceAll("%authoremail%", K_APP_AUTHOR_MAIL);
    formatStr.ReplaceAll("%author2email%", K_APP_AUTHOR_2_MAIL);

    formatStr.ReplaceAll("%hdr_builddate%", compileTimeStr);
    formatStr.ReplaceAll("%hdr_programming%", B_TRANSLATE("[ Programming ]"));
    formatStr.ReplaceAll("%hdr_credits%", B_TRANSLATE("CREDITS"));
    formatStr.ReplaceAll("%hdr_clv%", B_TRANSLATE("[ ColumnListView ]"));
    formatStr.ReplaceAll("%hdr_splitview%", B_TRANSLATE("[ SplitPane ]"));
    formatStr.ReplaceAll("%hdr_7zip%", B_TRANSLATE("[ 7zip Add-on ]"));
    formatStr.ReplaceAll("%hdr_docupdates%", B_TRANSLATE("[ Documentation Updates ]"));
    formatStr.ReplaceAll("%hdr_legal%", B_TRANSLATE("LEGAL MUMBO JUMBO"));
    formatStr.ReplaceAll("%hdr_disclaimer%", B_TRANSLATE("[ Disclaimer ]"));
    formatStr.ReplaceAll("%hdr_thanks%", B_TRANSLATE("SPECIAL THANKS TO"));

    m_creditsText = strdup(formatStr.String());

    m_textView->SetText(m_lineFeeds.String());
    m_textView->Insert(m_lineFeeds.Length(), m_creditsText, strlen(m_creditsText));

    // Dealloc m_creditsText as our textview would have made a copy of it & we don't use it anywhere else
    free((char*)m_creditsText);

    int32 nSubHeadings = 8;
    BString subHeadings[] =
    {
        B_TRANSLATE("[ Programming ]"),
        B_TRANSLATE("[ ColumnListView ]"),
        B_TRANSLATE("[ SplitPane ]"),
        B_TRANSLATE("[ 7zip Add-on ]"),
        B_TRANSLATE("[ Documentation Updates ]"),
        B_TRANSLATE("[ Disclaimer ]")
    };

    int32 nMainHeadings = 3;
    BString mainHeadings[] =
    {
        B_TRANSLATE("CREDITS"),                    // 0
        B_TRANSLATE("LEGAL MUMBO JUMBO"),          // 1
        B_TRANSLATE("SPECIAL THANKS TO")           // 2
    };

    // Search and color sub headings
    BString temp = m_textView->Text();
    int32 strt;
    for (int32 i = 0; i < nSubHeadings; i++)
        if ((strt = temp.FindFirst(subHeadings[i].String())) != B_ERROR)
        {
            m_textView->SetFontAndColor(strt, strt + strlen(subHeadings[i].String()),
                                        be_plain_font, B_FONT_ALL, &K_ABOUT_SUB_HEADING);
        }

    // Search and color main headings
    for (int32 i = 0; i < nMainHeadings; i++)
        if ((strt = temp.FindFirst(mainHeadings[i].String())) != B_ERROR)
        {
            m_textView->SetFontAndColor(strt, strt + strlen(mainHeadings[i].String()),
                                        be_plain_font, B_FONT_ALL, &K_ABOUT_MAIN_HEADING);
        }

    // Center window on-screen
    CenterOnScreen();

    // Spawn & resume the scroller thread now
    m_textView->Show();
    m_scrollThreadID = spawn_thread(_scroller, "_magic_scroller", B_NORMAL_PRIORITY, (void*)this);

    Show();
    resume_thread(m_scrollThreadID);
}


void AboutWindow::Quit()
{
    be_app_messenger.SendMessage(M_CLOSE_ABOUT);
    return BWindow::Quit();
}


void AboutWindow::DispatchMessage(BMessage* message, BHandler* handler)
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

    BWindow::DispatchMessage(message, handler);
}


int32 AboutWindow::_scroller(void* data)
{
    // This thread function controls the scrolling of the marqueeview
    AboutWindow* wnd = reinterpret_cast<AboutWindow*>(data);
    float textLen, height;
    float ptY;
    BPoint pt;

    // Calculate a few things here so that our loop isn't strained
    if (wnd && wnd->Lock())
    {
        textLen = wnd->m_textView->TextLength() - 1;
        height = wnd->Bounds().Height();
        pt = wnd->m_textView->PointAt(wnd->m_textView->TextLength() - 1);
        wnd->Unlock();
    }
    else
        return 0;

    ptY = pt.y + height;
    MarqueeView* vw = wnd->m_textView;

    // Control the scrolling view
    for (;;)
    {
        if (wnd->Lock() == true)
            vw->ScrollBy(0, 1);
        else
            return 0;

        if (vw->Bounds().bottom > ptY)
            vw->ScrollTo(0, 0);

        wnd->Unlock();
        snooze(K_SCROLL_DELAY);
    }

    return 0;
}
