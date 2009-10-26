/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <Bitmap.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <Button.h>
#include <Screen.h>

#include "StatusWindow.h"
#include "BevelView.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "AppConstants.h"
#include "BarberPole.h"
#include "StaticBitmapView.h"

//=============================================================================================================//

StatusWindow::StatusWindow (const char *title, BWindow *callerWindow, const char *text, volatile bool *cancel,
		bool showWindow)
	: BWindow (BRect (0, 0, 300, 0), title, B_TITLED_WINDOW,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE, B_CURRENT_WORKSPACE)
{
	if (callerWindow)
	{
		SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
		AddToSubset (callerWindow);
	}
	SetLook (B_MODAL_WINDOW_LOOK);

	BFont font (be_plain_font);
	font.SetFace (B_BOLD_FACE);
	font_height fntHt;
	font.GetHeight (&fntHt);
	float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
	
	BRect bounds (Bounds());
	m_backView = new BevelView (bounds, "StatusWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	m_backView->SetViewColor (K_BACKGROUND_COLOR);
	AddChild (m_backView);

	BBitmap *icon = ResBitmap ("Img:WarnAlert");

	StaticBitmapView *iconView = new StaticBitmapView (BRect (2 * K_MARGIN + 2, 2 * K_MARGIN + 2,
										2 * K_MARGIN + 2 + 31.0, 2 * K_MARGIN + 2 + 31.0),
										"StatusWindow:iconView", icon);
	iconView->SetViewColor (m_backView->ViewColor());
	AddChild (iconView);
	
	float width, height;
	BStringView *titleStrView = new BStringView (BRect (iconView->Frame().right + 4 * K_MARGIN,
								K_MARGIN + 3.0, iconView->Frame().right + 4 * K_MARGIN +
								m_backView->StringWidth (title) + m_backView->StringWidth ("W"),
								totalFontHeight + K_MARGIN + 3.0), "StatusWindow:StringView",
								title, B_FOLLOW_LEFT, B_WILL_DRAW);
	titleStrView->SetFont (&font, B_FONT_ALL);
	titleStrView->SetHighColor (K_STARTUP_MAIN_HEADING);
	titleStrView->SetLowColor (titleStrView->ViewColor());
	titleStrView->SetAlignment (B_ALIGN_CENTER);
	titleStrView->GetPreferredSize (&width, &height);
	titleStrView->ResizeTo (width, titleStrView->Frame().Height());
	m_backView->AddChild (titleStrView);
	
	m_barberPole = new BarberPole (BRect (titleStrView->Frame().left + 1.0,
									titleStrView->Frame().bottom + K_MARGIN / 2.0,
									0, iconView->Frame().bottom), "StatusWindow::BarberPole");
	m_backView->AddChild (m_barberPole);

	// Recalculate font height for plain font as we don't use bold font anymore
	be_plain_font->GetHeight (&fntHt);
	totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

	BStringView *textLabel = new BStringView (BRect (m_barberPole->Frame().right + 2 * K_MARGIN,
								m_barberPole->Frame().top + totalFontHeight / 2.0,
								m_barberPole->Frame().right + 2 * K_MARGIN + m_backView->StringWidth (text) +
								m_backView->StringWidth ("W"), totalFontHeight +
								m_barberPole->Frame().top + totalFontHeight / 2.0), "StatusWindow:StringView",
								text, B_FOLLOW_LEFT, B_WILL_DRAW);
	textLabel->ResizeToPreferred();
	textLabel->MoveTo (textLabel->Frame().left, m_barberPole->Frame().bottom - textLabel->Frame().Height());
	m_backView->AddChild (textLabel);

	if (cancel)
	{
		BevelView *edgeView = new BevelView (BRect (-1, textLabel->Frame().bottom + 2 * K_MARGIN,
									bounds.right - 1, textLabel->Frame().bottom + 2 * K_MARGIN + 1.0),
									"StatusWindow:EdgeView", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
		m_backView->AddChild (edgeView);
	
		BButton *cancelButton = new BButton (BRect (bounds.right - K_BUTTON_WIDTH - 2 * K_MARGIN,
								edgeView->Frame().bottom + K_MARGIN, bounds.right - 2 * K_MARGIN,
								edgeView->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
								"StatusWindow:CancelButton", str (S_STOP_OPERATION),
								new BMessage (M_STOP_OPERATION), B_FOLLOW_RIGHT, B_WILL_DRAW);
		m_backView->AddChild (cancelButton);
		ResizeTo (Frame().Width(), cancelButton->Frame().bottom + K_MARGIN);
	}
	else
		ResizeTo (Frame().Width(), textLabel->Frame().bottom + 2 * K_MARGIN + 4);
	
	BRect screen_rect (BScreen().Frame());
	MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);

	SetPulseRate (K_BARBERPOLE_PULSERATE);
	m_barberPole->SetValue (true, true);

	if (cancel)
		*cancel = false;
	m_cancel = cancel;

	if (showWindow == true)
		Show();
}

//=============================================================================================================//

void StatusWindow::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_STOP_OPERATION:
		{
			m_barberPole->SetValue (false, false);
			if (m_cancel)
				*m_cancel = true;
			break;
		}
		
		case M_CLOSE:
		{
			snooze (70000);
			m_barberPole->SetValue (false, false);
			Quit();
			break;
		}
		
		default:
		{
			BWindow::MessageReceived (message);
			break;
		}
	}
}

//=============================================================================================================//

