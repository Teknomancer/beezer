/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <Application.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <Screen.h>
#include <String.h>
#include <Button.h>

#include "AuthorWindow.h"
#include "UIConstants.h"
#include "BitmapPool.h"
#include "ImageButton.h"
#include "LangStrings.h"
#include "AppConstants.h"

#include "URLView.h"

//=============================================================================================================//

AuthorWindow::AuthorWindow ()
    : BWindow (BRect (0, 0, 300, 190), str (S_AUTHOR_TITLE), B_MODAL_WINDOW,
        B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE)
{
    SetFeel (B_MODAL_APP_WINDOW_FEEL);
    SetLook (B_MODAL_WINDOW_LOOK);
    
    m_backView = new BevelView (Bounds(), "AuthorWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    // Get font metrics
    BFont font (be_plain_font);
    font_height fntHt;

    font.SetFace (B_BOLD_FACE);
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    // Get comment icon from resource, construct comment holding view etc.
    BBitmap *appBmp = _glob_bitmap_pool->m_largeAppIcon;

    BevelView *sepView1 = new BevelView (BRect (-1, appBmp->Bounds().Height() + 4 * K_MARGIN,
                                Bounds().right - 1.0, appBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                "CommentWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView1);
    
    // Add icon to ImageButton (since icon we use ImageButton because it draws in B_OP_ALPHA mode to render
    // the icon with transparency)
    ImageButton *appBmpView = new ImageButton (BRect (K_MARGIN * 5, K_MARGIN * 2,
                             appBmp->Bounds().Width() + K_MARGIN * 5,
                             appBmp->Bounds().Height() + K_MARGIN * 2), NULL, NULL, appBmp, appBmp, NULL,
                             false, m_backView->ViewColor(), kBelowIcon, false, true, false, B_FOLLOW_TOP,
                             B_WILL_DRAW);
    m_backView->AddChild (appBmpView);
    appBmpView->SetEnabled (false);
    appBmpView->SetMargin (2, 2);
    appBmpView->ResizeTo (appBmp->Bounds().Width() + 4, appBmp->Bounds().Height() + 4);
    
    ImageButton *appBmpView2 = new ImageButton (BRect (Bounds().right - K_MARGIN * 5 - appBmp->Bounds().Width()-1,
                             K_MARGIN * 2, Bounds().right - K_MARGIN * 5,
                             appBmp->Bounds().Height() + K_MARGIN * 2), NULL, NULL, appBmp, appBmp, NULL,
                             false, m_backView->ViewColor(), kBelowIcon, false, true, false, B_FOLLOW_TOP,
                             B_WILL_DRAW);
    m_backView->AddChild (appBmpView2);
    appBmpView2->SetEnabled (false);
    appBmpView2->SetMargin (2, 2);
    appBmpView2->ResizeTo (appBmp->Bounds().Width() + 4, appBmp->Bounds().Height() + 4);
    
    // Add the file name string view (align it vertically with the icon view)
    BStringView *appTitleView = new BStringView (BRect (appBmpView->Frame().right + K_MARGIN * 3 - 2,
                                    appBmpView->Frame().top, Bounds().right - 1,
                                    appBmpView->Frame().top + totalFontHeight),
                                    "AuthorWindow:appTitleView", K_APP_TITLE, B_FOLLOW_LEFT, B_WILL_DRAW);
    appTitleView->SetFont (&font);
    appTitleView->ResizeToPreferred ();
    appTitleView->SetHighColor (255, 255, 255, 255);
    appTitleView->MoveTo ((Bounds().Width() - appTitleView->Frame().Width()) / 2,
        (appBmpView->Frame().Height() / 2 - totalFontHeight / 2) + totalFontHeight / 2);
    appTitleView->MoveBy (0, 1);
    m_backView->AddChild (appTitleView);

    // Draw main string (the PREVIOUS was the shadow/gradient ie the background)
    BStringView *appTitleView2 = new BStringView (BRect (appBmpView->Frame().right + K_MARGIN * 3 - 2,
                                    appBmpView->Frame().top, Bounds().right - 1,
                                    appBmpView->Frame().top + totalFontHeight),
                                    "AuthorWindow:appTitleView", K_APP_TITLE, B_FOLLOW_LEFT, B_WILL_DRAW);
    appTitleView2->SetFont (&font);
    appTitleView2->ResizeToPreferred ();
    appTitleView2->SetHighColor (135, 48, 48, 255);
    appTitleView2->MoveTo ((Bounds().Width() - appTitleView2->Frame().Width()) / 2,
        (appBmpView->Frame().Height() / 2 - totalFontHeight / 2) + totalFontHeight / 2);
    m_backView->AddChild (appTitleView2);

    m_contentView = new BevelView (BRect (K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                         Bounds().right - K_MARGIN, Bounds().bottom - K_MARGIN),
                         "AuthorWindow:ContentView", btInset);
    m_backView->AddChild (m_contentView);
    m_contentView->SetViewColor (255, 254, 242, 255);
    
    int descStringsCount = 6;
    BString descStrings[] =
    {
        str (S_AUTHOR_AUTHOR_DESC),
        str (S_AUTHOR_WEBSITE_DESC),
        str (S_AUTHOR_BESHARE_DESC),
        str (S_AUTHOR_EMAIL_DESC),
        str (S_AUTHOR_AGE_DESC),
        str (S_AUTHOR_NATIONALITY_DESC),
        str (S_AUTHOR_OCCUPATION_DESC)
    };

    int rightSideCount = 7;
    BString rightSideStrings[] =
    {
        K_APP_AUTHOR,
        K_APP_AUTHOR_AGE,
        K_APP_AUTHOR_COUNTRY,
        K_APP_AUTHOR_OCCUPATION,
        K_APP_AUTHOR_BESHARE_NICK,
        K_APP_WEBSITE_URL,
        K_APP_AUTHOR_MAIL_1,
        K_APP_AUTHOR_MAIL_2
    };
    
    float maxWidth = m_backView->StringWidth (descStrings[0].String());
    for (int32 i = 1; i <= descStringsCount; i++)
        maxWidth = MAX (maxWidth, m_backView->StringWidth (descStrings[i].String()));
    
    float dividerWidth = maxWidth + K_MARGIN * 6 + 4;
        
    float windowExtent = 0;
    for (int32 i = 0; i <= rightSideCount; i++)
        windowExtent = MAX (windowExtent, m_backView->StringWidth (rightSideStrings[i].String()));
    
    // Render controls inside the content view
    BStringView *authorDesc = new BStringView (BRect (K_MARGIN * 6, K_MARGIN,
                                    K_MARGIN * 6 + maxWidth + 2,
                                    K_MARGIN + totalFontHeight + 2), "AuthorWindow:AuthorDesc",
                                    str (S_AUTHOR_AUTHOR_DESC));
    m_contentView->AddChild (authorDesc);
    authorDesc->SetHighColor (K_DARK_GREY_COLOR);
    authorDesc->SetAlignment (B_ALIGN_RIGHT);
    
    BStringView *authorView = new BStringView (BRect (dividerWidth, authorDesc->Frame().top,
                                        dividerWidth + m_backView->StringWidth (K_APP_AUTHOR) + 2,
                                        authorDesc->Frame().bottom), "AuthorWindow:AuthorView", K_APP_AUTHOR);
    m_contentView->AddChild (authorView);
    
    BStringView *ageDesc = new BStringView (BRect (K_MARGIN * 6, authorView->Frame().bottom + totalFontHeight,
                                K_MARGIN * 6 + maxWidth + 2,
                                authorView->Frame().bottom + 2 * totalFontHeight + 2),
                                "AuthorWindow:AgeDesc", str (S_AUTHOR_AGE_DESC));
    m_contentView->AddChild (ageDesc);
    ageDesc->SetHighColor (K_DARK_GREY_COLOR);
    ageDesc->SetAlignment (B_ALIGN_RIGHT);
    
    BStringView *ageView = new BStringView (BRect (dividerWidth, ageDesc->Frame().top,
                                dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_AGE) + 2,
                                ageDesc->Frame().bottom), "AuthorWindow:AgeView", K_APP_AUTHOR_AGE);
    m_contentView->AddChild (ageView);
    
    BStringView *natDesc = new BStringView (BRect (K_MARGIN * 6, ageView->Frame().bottom,
                                K_MARGIN * 6 + maxWidth + 2, ageView->Frame().bottom + totalFontHeight + 2),
                                "AuthorWindow:NatDesc", str (S_AUTHOR_NATIONALITY_DESC));
    m_contentView->AddChild (natDesc);
    natDesc->SetHighColor (K_DARK_GREY_COLOR);
    natDesc->SetAlignment (B_ALIGN_RIGHT);
    
    BStringView *natView = new BStringView (BRect (dividerWidth, natDesc->Frame().top,
                                dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_COUNTRY) + 2,
                                natDesc->Frame().bottom), "AuthorWindow:NatView", K_APP_AUTHOR_COUNTRY);
    m_contentView->AddChild (natView);


    BStringView *occDesc = new BStringView (BRect (K_MARGIN * 6, natView->Frame().bottom,
                                K_MARGIN * 6 + maxWidth + 2, natView->Frame().bottom + totalFontHeight + 2),
                                "AuthorWindow:NatDesc", str (S_AUTHOR_OCCUPATION_DESC));
    m_contentView->AddChild (occDesc);
    occDesc->SetHighColor (K_DARK_GREY_COLOR);
    occDesc->SetAlignment (B_ALIGN_RIGHT);
    
    BStringView *occView = new BStringView (BRect (dividerWidth, occDesc->Frame().top,
                                dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_OCCUPATION) + 2,
                                occDesc->Frame().bottom), "AuthorWindow:NatView", K_APP_AUTHOR_OCCUPATION);
    m_contentView->AddChild (occView);


    BStringView *beshareDesc = new BStringView (BRect (K_MARGIN * 6, occView->Frame().bottom + totalFontHeight,
                                    K_MARGIN * 6 + maxWidth + 2,
                                    occView->Frame().bottom + 2 * totalFontHeight + 2),
                                    "AuthorWindow:BeShareDesc", str (S_AUTHOR_BESHARE_DESC));
    m_contentView->AddChild (beshareDesc);
    beshareDesc->SetHighColor (K_DARK_GREY_COLOR);
    beshareDesc->SetAlignment (B_ALIGN_RIGHT);

    BStringView *beshareView = new BStringView (BRect (dividerWidth, beshareDesc->Frame().top,
                                        dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_BESHARE_NICK)+2,
                                        beshareDesc->Frame().bottom), "AuthorWindow:BeShareView",
                                        K_APP_AUTHOR_BESHARE_NICK);
    m_contentView->AddChild (beshareView);


    BStringView *webDesc = new BStringView (BRect (K_MARGIN * 6, beshareView->Frame().bottom,
                                    K_MARGIN * 6 + maxWidth + 2,
                                    beshareView->Frame().bottom + totalFontHeight + 2),
                                    "AuthorWindow:AuthorDesc", str (S_AUTHOR_WEBSITE_DESC));
    m_contentView->AddChild (webDesc);
    webDesc->SetHighColor (K_DARK_GREY_COLOR);
    webDesc->SetAlignment (B_ALIGN_RIGHT);
    
    URLView *webView = new URLView (BRect (dividerWidth + 1, webDesc->Frame().top,
                                    dividerWidth + m_backView->StringWidth (K_APP_WEBSITE) + 2,
                                    webDesc->Frame().bottom), "AuthorWindow:WebView", K_APP_WEBSITE,
                                    K_APP_WEBSITE_URL);
    m_contentView->AddChild (webView);
    webView->SetColor (K_DEEP_BLUE_COLOR);
    webView->SetHoverEnabled (true);
    webView->SetHoverColor (K_DEEP_RED_COLOR);
    webView->SetClickColor (K_ORANGE_COLOR);

    BStringView *mailDesc = new BStringView (BRect (K_MARGIN * 6, webView->Frame().bottom,
                                    K_MARGIN * 6 + maxWidth + 2,
                                    webView->Frame().bottom + totalFontHeight + 2),
                                    "AuthorWindow:MailDesc", str (S_AUTHOR_EMAIL_DESC));
    m_contentView->AddChild (mailDesc);
    mailDesc->SetHighColor (K_DARK_GREY_COLOR);
    mailDesc->SetAlignment (B_ALIGN_RIGHT);

    URLView *mailView1 = new URLView (BRect (dividerWidth + 1, mailDesc->Frame().top,
                                    dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_MAIL_1) + 2,
                                    mailDesc->Frame().bottom), "AuthorWindow:MailView1", K_APP_AUTHOR_MAIL_1,
                                    K_APP_AUTHOR_MAIL_URL_1);
    m_contentView->AddChild (mailView1);
    mailView1->SetEnabled (true);
    mailView1->SetColor (K_DEEP_BLUE_COLOR);
    mailView1->SetHoverEnabled (true);
    mailView1->SetHoverColor (K_DEEP_RED_COLOR);
    mailView1->SetClickColor (K_ORANGE_COLOR);
    mailView1->AddAttribute ("META:name", K_APP_AUTHOR);
    mailView1->AddAttribute ("META:nickname", K_APP_AUTHOR_BESHARE_NICK);
    mailView1->AddAttribute ("META:url", K_APP_WEBSITE_URL);
    mailView1->AddAttribute ("META:country", K_APP_AUTHOR_COUNTRY);
    
    URLView *mailView2 = new URLView (BRect (dividerWidth + 1, mailDesc->Frame().bottom,
                                    dividerWidth + m_backView->StringWidth (K_APP_AUTHOR_MAIL_2) + 2,
                                    mailDesc->Frame().bottom + totalFontHeight + 2),
                                    "AuthorWindow:MailView2", K_APP_AUTHOR_MAIL_2, K_APP_AUTHOR_MAIL_URL_2);
    m_contentView->AddChild (mailView2);
    mailView2->SetEnabled (true);
    mailView2->SetColor (K_DEEP_BLUE_COLOR);
    mailView2->SetHoverEnabled (true);
    mailView2->SetHoverColor (K_DEEP_RED_COLOR);
    mailView2->SetClickColor (K_ORANGE_COLOR);
    mailView2->AddAttribute ("META:name", K_APP_AUTHOR);
    mailView2->AddAttribute ("META:nickname", K_APP_AUTHOR_BESHARE_NICK);
    mailView2->AddAttribute ("META:url", K_APP_WEBSITE_URL);
    mailView2->AddAttribute ("META:country", K_APP_AUTHOR_COUNTRY);


    // Auto-size the content view's height
    m_contentView->ResizeTo (m_contentView->Frame().Width(), mailView2->Frame().bottom + 4 + K_MARGIN);

    BevelView *sepView2 = new BevelView (BRect (-1, m_contentView->Frame().bottom + K_MARGIN + 1,
                                Bounds().right - 1.0, m_contentView->Frame().bottom + K_MARGIN + 2),
                                "CommentWindow:SepView2", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView2);

    BButton *closeButton = new BButton (BRect (Bounds().right - 3 * K_MARGIN - K_BUTTON_WIDTH,
                                sepView2->Frame().bottom + K_MARGIN, Bounds().right - 3 * K_MARGIN,
                                sepView2->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
                                "AuthorWindow:CloseButton", str (S_AUTHOR_CLOSE),
                                new BMessage (B_QUIT_REQUESTED));
    m_backView->AddChild (closeButton);
    closeButton->MakeDefault (true);
    closeButton->MoveBy (0, 4);
    
    float wndHeight = MAX (Frame().Height(), closeButton->Frame().bottom + K_MARGIN);
    ResizeTo (Frame().Width(), wndHeight);

    char titleBuf [strlen (Title()) + strlen (K_APP_TITLE) + 2];
    sprintf (titleBuf, Title(), K_APP_TITLE);
    SetTitle (titleBuf);
    
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
    
    // Check if the contents exceed the width of the window (in case user has LARGE and i mean LARGE fonts)
    // this _can_ happen but its rare, anyhow
    if (dividerWidth + windowExtent + 3 >= Frame().Width())
    {
        float diff = dividerWidth + windowExtent + 3 - Frame().Width();
        ResizeBy (diff, 0);
        m_contentView->ResizeBy (diff, 0);
        appBmpView2->MoveBy (diff, 0);
        appTitleView->MoveBy (diff / 2, 0);
        appTitleView2->MoveBy (diff / 2, 0);
        closeButton->MoveBy (diff, 0);
    }
    
    Show();
}

//=============================================================================================================//

void AuthorWindow::Quit()
{
    be_app_messenger.SendMessage (M_CLOSE_AUTHOR);
    return BWindow::Quit();
}

//=============================================================================================================//
