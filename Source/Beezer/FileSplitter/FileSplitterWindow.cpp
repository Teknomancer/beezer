/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <Screen.h>
#include <Menu.h>
#include <MenuField.h>
#include <TextControl.h>
#include <Button.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <interface/StringView.h>
#include <Bitmap.h>
#include <CheckBox.h>
#include <StatusBar.h>
#include <FilePanel.h>
#include <Path.h>
#include <MenuBar.h>
#include <Alert.h>
#include <Application.h>
#include <String.h>
#include <File.h>
#include <Resources.h>

#include "BevelView.h"
#include "LangStrings.h"
#include "FileSplitterWindow.h"
#include "UIConstants.h"
#include "StaticBitmapView.h"
#include "LocalUtils.h"
#include "SelectDirPanel.h"
#include "MsgConstants.h"
#include "FSUtils.h"
#include "Shared.h"
#include "RecentMgr.h"
#include "ArchiverMgr.h"
#include "AppConstants.h"

//=============================================================================================================//

FileSplitterWindow::FileSplitterWindow (RecentMgr *files, RecentMgr *dirs)
	: BWindow (BRect (10, 10, 540, 350), str (S_FILE_SPLITTER_TITLE), B_TITLED_WINDOW,
				B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE),
	m_dirPanel (NULL), 
	m_filePanel (NULL),
	m_sepString (NULL),
	m_recentSplitFiles (files),
	m_recentSplitDirs (dirs)
{
	// If you change the above BRect in the BWindow call, change accordingly the code at the end
	// of this constructor to match, i.e. current 550 - 10 = 540 is used below, if you change them
	// change the code below (see "Constrain size of window" comment)
	
	m_backView = new BevelView (Bounds(), "FileSplitterWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES,
							B_WILL_DRAW);
	m_backView->SetViewColor (K_BACKGROUND_COLOR);
	AddChild (m_backView);

	BFont font (be_plain_font);
	font_height fntHt;

	font.GetHeight (&fntHt);
	float normFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
	
	font.SetFace (B_BOLD_FACE);
	font.GetHeight (&fntHt);
	float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
	
	BBitmap *splitBmp = ResBitmap ("Img:FileSplitter");
	
	BevelView *sepView1 = new BevelView (BRect (-1, splitBmp->Bounds().Height() + 4 * K_MARGIN,
									Bounds().right - 1.0, splitBmp->Bounds().Height() + 4 * K_MARGIN + 1),
									"FileSplitterWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	m_backView->AddChild (sepView1);
	
	StaticBitmapView *splitBmpView = new StaticBitmapView (BRect (K_MARGIN * 5, K_MARGIN * 2,
							splitBmp->Bounds().Width() + K_MARGIN * 5,
							splitBmp->Bounds().Height() + K_MARGIN * 2), "FileSplitterWindow:splitBmpView",
							splitBmp);
	splitBmpView->SetViewColor (m_backView->ViewColor());
	AddChild (splitBmpView);

	// Add the file name string view (align it vertically with the icon view)
	m_descStr = new BStringView (BRect (splitBmpView->Frame().right + K_MARGIN * 3,
										splitBmpView->Frame().top, Bounds().right - 1,
										splitBmpView->Frame().top + totalFontHeight),
										"FileSplitterWindow:DescStr", str (S_FILE_SPLITTER_DESC),
										B_FOLLOW_LEFT, B_WILL_DRAW);
										
	//m_descStr->SetFont (&font);
	m_backView->AddChild (m_descStr);
	m_descStr->MoveTo (m_descStr->Frame().left,
		(splitBmpView->Frame().Height() / 2 - normFontHeight / 2) + normFontHeight / 2 + 1);
	m_descStr->MoveBy (0, -normFontHeight / 2 - 1);
	m_descStr->ResizeToPreferred ();

	m_descStr2 = new BStringView (BRect (m_descStr->Frame().left,
										m_descStr->Frame().bottom + 1, Bounds().right - 1, 0),
										"FileSplitterWindow:DescStr2", str (S_FILE_SPLITTER_DESC2),
										B_FOLLOW_LEFT, B_WILL_DRAW);
	m_backView->AddChild (m_descStr2);
	m_descStr2->ResizeToPreferred();
	
	m_innerView = new BevelView (BRect (K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
									Bounds().right - K_MARGIN,
									Bounds().bottom - K_MARGIN), "FileSplitterWindow:InnerView", btNoBevel,
									B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	m_backView->AddChild (m_innerView);
	
	m_fileMenu = new BMenu (str (S_FILE_TO_SPLIT));
	m_fileField = new BMenuField (BRect (K_MARGIN, K_MARGIN,
							K_MARGIN + m_backView->StringWidth (str (S_FILE_TO_SPLIT)) + 40, 0),
							"FileSplitterWindow:FileField", NULL, m_fileMenu);
	m_innerView->AddChild (m_fileField);

	m_folderMenu = new BMenu (str (S_FOLDER_FOR_PIECES));
	m_folderField = new BMenuField (BRect (K_MARGIN, m_fileField->Frame().bottom + K_MARGIN - 1,
							K_MARGIN + m_backView->StringWidth (str (S_FOLDER_FOR_PIECES)) + 40, 0),
							"FileSplitterWindow:FolderField", NULL, m_folderMenu);
	float maxWidth = MAX (m_folderField->Frame().Width(), m_fileField->Frame().Width());
	maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_SIZE_OF_PIECES)));
	maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_CUSTOM_SIZE_PROMPT)));
	maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_NUMBER_OF_PIECES)));
	maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_SPLIT_FILE_SIZE)));
	maxWidth = MAX (maxWidth, m_backView->StringWidth (str (S_SPLIT_SEPARATOR)));
	maxWidth += 2 * K_MARGIN;
	
	m_filePathView = new BTextControl (BRect (maxWidth, m_fileField->Frame().top + 2,
							m_innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
							"FileSplitterView:FilePathView", NULL, NULL, NULL,
							B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	m_filePathView->SetDivider (0);
	m_filePathView->ResizeToPreferred ();
	m_filePathView->SetModificationMessage (new BMessage (M_UPDATE_DATA));
	
	m_folderPathView = new BTextControl (BRect (maxWidth, m_folderField->Frame().top + 2,
								m_innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
								"FileSplitterView:FolderPathView", NULL, NULL, NULL,
								B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	m_folderPathView->SetDivider (0);
	m_folderPathView->ResizeToPreferred ();
	m_folderPathView->SetModificationMessage (new BMessage (M_UPDATE_DATA));
	
	// Re-use this string here
	BString buttonText = str (S_PREFS_PATHS_SELECT); buttonText << "...";
	m_selectFileBtn = new BButton (BRect (m_filePathView->Frame().right + K_MARGIN,
								m_filePathView->Frame().top - 4,
								m_filePathView->Frame().right + K_MARGIN + K_BUTTON_WIDTH,
								m_filePathView->Frame().top - 4 + K_BUTTON_HEIGHT),
								"FileSplitterWindow:SelectFileBtn", buttonText.String(),
								new BMessage (M_SELECT_SPLIT_FILE), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	m_innerView->AddChild (m_filePathView);
	m_innerView->AddChild (m_selectFileBtn);

	m_selectFolderBtn = new BButton (BRect (m_folderPathView->Frame().right + K_MARGIN,
								m_folderPathView->Frame().top - 4,
								m_folderPathView->Frame().right + K_MARGIN + K_BUTTON_WIDTH,
								m_folderPathView->Frame().top - 4 + K_BUTTON_HEIGHT),
								"FileSplitterWindow:SelectFolderBtn", buttonText.String(),
								new BMessage (M_SELECT_SPLIT_FOLDER), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	m_innerView->AddChild (m_folderField);
	m_innerView->AddChild (m_folderPathView);
	m_innerView->AddChild (m_selectFolderBtn);

	m_sizePopUp = new BPopUpMenu ("Sizes", true, true);
	m_sizePopUp->AddItem (new BMenuItem (str (S_CUSTOM_SIZE), new BMessage (M_CUSTOM_SIZE)));
	m_sizePopUp->AddSeparatorItem();
	m_sizePopUp->AddItem (new BMenuItem (str (S_144_MB_FLOPPY), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_120_MB_FLOPPY), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_100_MB_ZIP), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_250_MB_ZIP), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_650_MB_CD), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_700_MB_CD), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_800_MB_CD), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_1_GB_JAZ), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->AddItem (new BMenuItem (str (S_2_GB_JAZ), new BMessage (M_PREDEFINED_SIZE)));
	m_sizePopUp->ItemAt (0L)->SetMarked (true);
	
	m_sizeField = new BMenuField (BRect (K_MARGIN, m_folderField->Frame().bottom + K_MARGIN - 1, 
							m_innerView->Frame().Width() - K_MARGIN, 0),
							"FileSplitterWindow:SizeField", str (S_SIZE_OF_PIECES), (BMenu*)m_sizePopUp,
							B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_sizeField->SetDivider (m_filePathView->Frame().left - K_MARGIN - 1);
	m_innerView->AddChild (m_sizeField);

	#ifdef B_ZETA_VERSION
		m_sizeField->MoveBy (0, 2);
	#endif

	m_customSizeView = new BTextControl (BRect (K_MARGIN, m_sizeField->Frame().bottom + K_MARGIN - 1,
							K_MARGIN + 182, 0), "FileSplitterWindow:CustomSizeView", str (S_CUSTOM_SIZE_PROMPT),
							NULL, NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_customSizeView->SetDivider (m_sizeField->Divider() - 2);
	m_customSizeView->SetModificationMessage (new BMessage (M_UPDATE_DATA));
	m_innerView->AddChild (m_customSizeView);

	#ifdef B_ZETA_VERSION
		m_customSizeView->MoveBy (0, 2);
	#endif

	// !! IMPORTANT !! Order is very critical, if the order below changes, the change must also be reflected
	// in "UpdateData()" function
	m_prefixPopUp = new BPopUpMenu ("SizePrefix", true, true);
	m_prefixPopUp->AddItem (new BMenuItem (str (S_PREFIX_BYTES), new BMessage (M_UPDATE_DATA)));
	m_prefixPopUp->AddItem (new BMenuItem (str (S_PREFIX_KB), new BMessage (M_UPDATE_DATA)));
	m_prefixPopUp->AddItem (new BMenuItem (str (S_PREFIX_MB), new BMessage (M_UPDATE_DATA)));
	m_prefixPopUp->AddItem (new BMenuItem (str (S_PREFIX_GB), new BMessage (M_UPDATE_DATA)));
	m_prefixPopUp->ItemAt (2L)->SetMarked (true);
	m_prefixField = new BMenuField (BRect (m_customSizeView->Frame().right + K_MARGIN,
						m_customSizeView->Frame().top - 2, m_innerView->Frame().right - K_MARGIN, 0),
						"FileSplitterWindow:PrefixView", NULL, (BMenu*)m_prefixPopUp,
						B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_innerView->AddChild (m_prefixField);

	#ifdef B_ZETA_VERSION
		m_prefixField->MoveBy (0, 2);
	#endif
	
	float maxPrefixWidth = 0.0f;
	for (int32 i = 0; i < m_sizePopUp->CountItems(); i++)
		maxPrefixWidth = MAX (maxPrefixWidth, 36 + m_backView->StringWidth (m_sizePopUp->ItemAt(i)->Label()));
	
	for (int32 i = 0; i < m_prefixPopUp->CountItems(); i++)
	{
		maxPrefixWidth = MAX (maxPrefixWidth,
			m_customSizeView->Frame().Width() - m_customSizeView->Divider() + 36 + 
				m_backView->StringWidth (m_prefixPopUp->ItemAt(i)->Label()));
	}
	
	maxPrefixWidth += m_customSizeView->Divider() + K_MARGIN;
	
	BevelView *sepView4 = new BevelView (BRect (maxPrefixWidth, m_sizeField->Frame().top, maxPrefixWidth + 1,
									m_prefixField->Frame().bottom), "FileSplitterWindow:sepView4", btInset,
									B_FOLLOW_LEFT, B_WILL_DRAW);
	m_innerView->AddChild (sepView4);
	
	m_openDirChk = new BCheckBox (BRect (sepView4->Frame().left + 3 * K_MARGIN, m_sizeField->Frame().top + 2,
							0, 0),  "FileSplitterWindow:OpenDirChk", str (S_OPEN_DIR_AFTER_SPLIT), NULL,
							B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_openDirChk->ResizeToPreferred ();
	m_openDirChk->SetValue (B_CONTROL_ON);

	m_closeChk = new BCheckBox (BRect (m_openDirChk->Frame().left, m_openDirChk->Frame().bottom + 1, 0, 0),
							"FileSplitterWindow:CloseChk", str (S_CLOSE_AFTER_SPLIT), NULL, B_FOLLOW_LEFT,
							B_WILL_DRAW | B_NAVIGABLE);
	m_closeChk->ResizeToPreferred ();
	m_closeChk->SetValue (B_CONTROL_ON);

	m_createChk = new BCheckBox (BRect (m_closeChk->Frame().left,
							m_closeChk->Frame().bottom + 1, 0, 0), "FileSplitterWindow:CreateChk",
							str (S_CREATE_EXE), NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_createChk->ResizeToPreferred ();
	
	m_separatorView = new BTextControl (BRect (m_customSizeView->Frame().left,
								m_customSizeView->Frame().bottom + K_MARGIN,
								m_customSizeView->Frame().right, 0), "FileSplitter:SeparatorView",
								str (S_SPLIT_SEPARATOR), "_", NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
	m_separatorView->SetDivider (m_customSizeView->Divider());
	m_separatorView->TextView()->SetMaxBytes (128);
	m_separatorView->TextView()->DisallowChar ('0');	// too lazy to loop using ASCII value :)
	m_separatorView->TextView()->DisallowChar ('1');
	m_separatorView->TextView()->DisallowChar ('2');
	m_separatorView->TextView()->DisallowChar ('3');
	m_separatorView->TextView()->DisallowChar ('4');
	m_separatorView->TextView()->DisallowChar ('5');
	m_separatorView->TextView()->DisallowChar ('6');
	m_separatorView->TextView()->DisallowChar ('7');
	m_separatorView->TextView()->DisallowChar ('8');
	m_separatorView->TextView()->DisallowChar ('9');
	m_separatorView->TextView()->DisallowChar (':');
	m_separatorView->TextView()->DisallowChar ('/');
	m_separatorView->TextView()->DisallowChar ('\\');
	m_separatorView->TextView()->DisallowChar ('*');
	m_separatorView->TextView()->DisallowChar ('?');
	m_separatorView->SetModificationMessage (new BMessage (M_SEPARATOR_CHANGED));
	m_innerView->AddChild (m_separatorView);
	m_innerView->AddChild (m_openDirChk);			// For tab ordering!
	m_innerView->AddChild (m_closeChk);
	m_innerView->AddChild (m_createChk);
	
	m_innerView->ResizeTo (m_innerView->Frame().Width(), m_separatorView->Frame().bottom + K_MARGIN);
	
	// Extend the line to the above text controls' bottom co-ordinate
	sepView4->ResizeBy (0, m_customSizeView->Frame().Height() + K_MARGIN + 1);
	
	
	// Add the next level of controls
	BevelView *sepView2 = new BevelView (BRect (-1, m_innerView->Frame().bottom + K_MARGIN + 1,
								Bounds().right - 1.0, m_innerView->Frame().bottom + K_MARGIN + 1 + 1),
								"FileSplitterWindow:SepView2", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
	m_backView->AddChild (sepView2);

	BStringView *noPiecesStr = new BStringView (BRect (2 * K_MARGIN, sepView2->Frame().bottom + 2 * K_MARGIN,
									2 * K_MARGIN + m_backView->StringWidth (str (S_NUMBER_OF_PIECES)) + 3, 0),
									"FileSplitterWindow:noPiecesStr", str (S_NUMBER_OF_PIECES), B_FOLLOW_LEFT,
									B_WILL_DRAW);
	m_backView->AddChild (noPiecesStr);
	noPiecesStr->ResizeToPreferred ();

	m_piecesStr = new BStringView (BRect (m_filePathView->Frame().left + K_MARGIN, noPiecesStr->Frame().top,
							Bounds().right - K_MARGIN, 0), "FileSplitterWindow:PiecesStr", "-", B_FOLLOW_LEFT,
							B_WILL_DRAW);
	m_backView->AddChild (m_piecesStr);
	m_piecesStr->ResizeToPreferred ();

	BStringView *sizeStr = new BStringView (BRect (noPiecesStr->Frame().left,
									noPiecesStr->Frame().bottom + K_MARGIN - 1, 0, 0),
									"FileSplitterWindow:sizeStr", str (S_SPLIT_FILE_SIZE), B_FOLLOW_LEFT,
									B_WILL_DRAW);
	sizeStr->ResizeToPreferred ();
	m_backView->AddChild (sizeStr);
	
	m_sizeStr = new BStringView (BRect (m_piecesStr->Frame().left, sizeStr->Frame().top, 0, 0),
						"FileSplitterWindow:SizeStr", "-", B_FOLLOW_LEFT, B_WILL_DRAW);
	m_sizeStr->ResizeToPreferred ();
	m_backView->AddChild (m_sizeStr);

	BevelView *sepView3 = new BevelView (BRect (-1, m_sizeStr->Frame().bottom + K_MARGIN, Bounds().right - 1.0,
											m_sizeStr->Frame().bottom + K_MARGIN + 1),
											"FileSplitterWindow:SepView3", btInset, B_FOLLOW_LEFT_RIGHT,
											B_WILL_DRAW);
	m_backView->AddChild (sepView3);
	sepView3->Hide();

	m_splitBtn = new BButton (BRect (Bounds().right - 2 * K_MARGIN - K_BUTTON_WIDTH - 3,
						sepView3->Frame().bottom - K_MARGIN - 6 - K_BUTTON_HEIGHT,
						Bounds().right - 2 * K_MARGIN - 3, sepView3->Frame().bottom - K_MARGIN - 6),
						"FileSplitterWindow:SplitBtn", str (S_SPLIT), new BMessage (M_SPLIT_NOW),
						B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
	m_splitBtn->MakeDefault (true);
	m_backView->AddChild (m_splitBtn);
	m_splitBtn->SetEnabled (false);

	// Add the status bar and buttons
	m_statusBar = new BStatusBar (BRect (3 * K_MARGIN, sepView3->Frame().bottom + 2 * K_MARGIN + 1,
							Bounds().right - 8 * K_MARGIN - K_BUTTON_WIDTH, 0),
							"FileSplitterWindow:StatusBar", NULL, NULL);
	m_statusBar->ResizeToPreferred ();
	m_statusBar->SetResizingMode (B_FOLLOW_LEFT_RIGHT);
	m_statusBar->ResizeTo (m_splitBtn->Frame().right - 4 * K_MARGIN, m_statusBar->Frame().Height());
	m_statusBar->SetBarHeight (K_PROGRESSBAR_HEIGHT);
	m_statusBar->SetBarColor (K_PROGRESS_COLOR);
	m_backView->AddChild (m_statusBar);

	// Resize window as needed
	m_hideProgress = sepView3->Frame().top + K_MARGIN;
	m_showProgress =  m_statusBar->Frame().bottom + K_MARGIN;
	ResizeTo (Frame().Width(), m_hideProgress);
	
	// Center window on-screen
	BRect screen_rect (BScreen().Frame());
	MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);
	
	// Calculatate maximum desc label size
	float maxLabelLen = MAX (m_descStr->StringWidth (m_descStr->Text()),
								m_descStr2->StringWidth (m_descStr2->Text()));
	maxLabelLen += splitBmpView->Frame().right + 2 * K_MARGIN * 5;
	maxLabelLen = MAX (maxLabelLen, m_openDirChk->Frame().right);
	maxLabelLen = MAX (maxLabelLen, m_createChk->Frame().right);
	maxLabelLen += 2 * K_MARGIN;
	maxLabelLen = MAX (maxLabelLen, 540);
	
	// Constrain size of window
	float minH, maxH, minV, maxV;
	GetSizeLimits (&minH, &maxH, &minV, &maxV);
	SetSizeLimits (maxLabelLen, maxH, minV, maxV);
	if (Frame().Width() < maxLabelLen)
		ResizeTo (maxLabelLen, Frame().Height());
	
	// Change focus
	m_filePathView->MakeFocus (true);
	m_messenger = new BMessenger (this);
	
	UpdateRecentMenus();
	
	m_splitInProgress = false;
	m_quitNow = false;
}

//=============================================================================================================//

FileSplitterWindow::~FileSplitterWindow ()
{
	if (m_dirPanel)
		delete m_dirPanel;
	
	if (m_filePanel);	
		delete m_filePanel;
		
	if (m_sepString)
		free (m_sepString);

	delete m_messenger;
}

//=============================================================================================================//

bool FileSplitterWindow::QuitRequested ()
{
	if (m_splitInProgress && m_quitNow == false)	// m_quitNow is checked so this isn't called twice
	{
		suspend_thread (m_thread);
		
		BAlert *alert = new BAlert ("Quit", str (S_FORCE_SPLIT_CLOSE_WARNING), str (S_DONT_FORCE_CLOSE),
								str (S_FORCE_CLOSE), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->SetShortcut (0L, B_ESCAPE);
		alert->SetDefaultButton (alert->ButtonAt (1L));
		int32 index = alert->Go();
		if (index == 1L)
		{
			m_cancel = true;
			m_splitBtn->SetEnabled (false);
		}

		m_quitNow = true;
		resume_thread (m_thread);
		return false;
	}
	else if (m_quitNow)	// Incase QuitRequested is called a second time, still don't do anything silly
		return false;
	else
		return BWindow::QuitRequested();
}

//=============================================================================================================//

void FileSplitterWindow::Quit ()
{
	be_app_messenger.SendMessage (M_CLOSE_FILE_SPLITTER);
	return BWindow::Quit();
}

//=============================================================================================================//

void FileSplitterWindow::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_SPLIT_NOW:
		{
			if (m_splitInProgress == false)
			{
				UpdateData();

				if (Frame().Height() == m_hideProgress)
					ToggleWindowHeight (true);
				
				m_cancel = false;
				m_thread = spawn_thread (_splitter, "_splitter", B_NORMAL_PRIORITY, (void*)this);
				resume_thread (m_thread);
				m_splitBtn->SetLabel (str (S_CANCEL));
				m_splitInProgress = true;
				m_splitBtn->MakeDefault (false);
			}
			else
			{
				m_cancel = true;
				m_splitBtn->SetEnabled (false);
			}
			
			break;
		}
		
		case BZR_UPDATE_PROGRESS:
		{
			char percentStr [100];
			float delta = message->FindFloat ("delta");
			int8 percent = (int8)ceil(100 * ((m_statusBar->CurrentValue() + delta) / m_statusBar->MaxValue()));
			sprintf (percentStr, "%d%%", percent);
			
			BString text = str (S_SPLITTING_FILE);
			text << " " << message->FindString ("text");
			
			m_statusBar->Update (delta, text.String(), percentStr);
			message->SendReply ('DUMB');
			break;
		}

		case M_OPERATION_COMPLETE:
		{
			m_splitInProgress = false;
			m_splitBtn->SetEnabled (true);
			m_splitBtn->MakeDefault (true);
			
			status_t result = message->FindInt32 (kResult);
			snooze (10000);
			ToggleWindowHeight (false);
			m_statusBar->Reset();
			
			BAlert *alert = NULL;
			if (result == BZR_DONE)
			{
				alert = new BAlert ("Done", str (S_SPLIT_SUCCESS), str (S_OK), NULL, NULL,
										B_WIDTH_AS_USUAL, B_INFO_ALERT);
			}
			else if (result == BZR_CANCEL)
			{
				alert = new BAlert ("Cancel", str (S_SPLIT_CANCEL), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
								B_WARNING_ALERT);
			}
			else
			{
				alert = new BAlert ("Error", str (S_SPLIT_ERROR), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
								B_STOP_ALERT);
			}

			if (result == BZR_DONE && m_createChk->Value() == B_CONTROL_ON)
				CreateSelfJoiner ();

			// Incase of no-errors with input files and output dirs, save them to recent lists
			if (result == BZR_DONE || result == BZR_CANCEL)
			{
				BPath path;
				BEntry temp;
				m_fileEntry.GetPath (&path);
				m_recentSplitFiles->AddPath (path.Path());
				
				m_destDir.GetEntry (&temp);
				temp.GetPath (&path);
				m_recentSplitDirs->AddPath (path.Path());
				
				UpdateRecentMenus ();
			}
			
			m_splitBtn->SetLabel (str (S_SPLIT));
			alert->SetShortcut (0L, B_ESCAPE);
			alert->SetDefaultButton (alert->ButtonAt(0L));
			alert->Go();
			
			if (m_openDirChk->Value() == B_CONTROL_ON)
			{
				entry_ref dirRef;
				BEntry destDirEntry;
				m_destDir.GetEntry (&destDirEntry);
				
				destDirEntry.GetRef (&dirRef);
				TrackerOpenFolder (&dirRef);
			}

			if (m_quitNow)
			{
				// If this is the last window to quit, quit be_app because it's message loop would have
				// gone down and thus PostMessage() won't work
				if (be_app->CountWindows() <= 1L)
					be_app->Quit();

				Quit();
				break;
			}

			if (result == BZR_DONE && m_closeChk->Value() == B_CONTROL_ON)
				Quit();
			
			break;
		}
		
		case M_SELECT_SPLIT_FILE:
		{
			if (m_filePanel == NULL)
			{
				m_filePanel = new BFilePanel (B_OPEN_PANEL, new BMessenger (this), NULL,
										B_FILE_NODE, false, new BMessage (M_SPLIT_FILE_SELECTED), NULL,
										true, true);
				m_filePanel->Window()->SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
				m_filePanel->Window()->AddToSubset (this);
				if (m_filePanel->Window()->LockLooper())
				{
					m_filePanel->Window()->SetTitle (str (S_SPLIT_FILE_SELECT_TITLE));
					m_filePanel->Window()->UnlockLooper();
				}
			}
			
			m_filePanel->Show();
			break;
		}
		
		case M_SPLIT_FILE_SELECTED: case M_TOOLS_FILE_SPLITTER:
		{
			entry_ref ref;
			message->FindRef ("refs", &ref);

			BPath filePath (&ref);
			m_filePathView->SetText (filePath.Path());
		
			BPath parentPath;
			filePath.GetParent (&parentPath);
			m_folderPathView->SetText (parentPath.Path());
			UpdateData();
			
			if (message->what == M_TOOLS_FILE_SPLITTER && m_customSizeView->IsEnabled())
				m_customSizeView->MakeFocus (true);
			
			break;
		}

		case M_SELECT_SPLIT_FOLDER:
		{
			if (m_dirPanel == NULL)
			{
				m_dirPanel = new SelectDirPanel (B_OPEN_PANEL, new BMessenger (this), NULL, B_DIRECTORY_NODE,
										false, new BMessage (M_SPLIT_FOLDER_SELECTED), NULL, true, false);

				m_dirPanel->SetButtonLabel (B_DEFAULT_BUTTON, str (S_SPLIT_FOLDER_SELECT));
				m_dirPanel->Window()->SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
				m_dirPanel->Window()->AddToSubset (this);
				m_dirPanel->SetCurrentDirButton (str (S_SPLIT_FOLDER_SELECT));
				
				if (m_dirPanel->Window()->LockLooper())
				{
					m_dirPanel->Window()->SetTitle (str (S_SPLIT_FOLDER_SELECT_TITLE));
					m_dirPanel->Window()->Unlock();
				}
			}
			m_dirPanel->Show();
			break;
		}
		
		case M_SPLIT_FOLDER_SELECTED:
		{
			if (m_dirPanel && m_dirPanel->IsShowing())
				m_dirPanel->Hide();
			
			entry_ref ref;
			message->FindRef ("refs", &ref);

			BPath folderPath (&ref);
			m_folderPathView->SetText (folderPath.Path());
			break;
		}
		
		case M_CUSTOM_SIZE:
		{
			m_customSizeView->SetEnabled (true);
			m_prefixField->SetEnabled (true);
			UpdateData ();
			break;
		}
		
		case M_PREDEFINED_SIZE:
		{
			m_customSizeView->SetText (NULL);
			m_customSizeView->SetEnabled (false);
			m_prefixField->SetEnabled (false);
			UpdateData ();
			break;
		}
		
		case M_UPDATE_DATA:
		{
			UpdateData ();
			break;
		}
		
		case M_RECENT_SPLIT_FILE:
		{
			// Don't load from label of BMenuItem as if that changes to display only filename than path
			// we would have errors, hence load from the Path field that "RecentMgr" adds to each recent item
			BPath path;
			message->FindFlat (kRecentSplitFile, &path);
			m_filePathView->SetText (path.Path());
			break;
		}
		
		case M_RECENT_SPLIT_DIR:
		{
			// Load from Path object -- even though it's not a file item
			BPath path;
			message->FindFlat (kRecentSplitDir, &path);
			m_folderPathView->SetText (path.Path());
			break;
		}
		
		case M_SEPARATOR_CHANGED:
		{
			UpdateData();
			break;
		}
		
		case B_SIMPLE_DATA:
		{
			// Handle drag 'n drop from Tracker
			BPath path;
			entry_ref ref;
			message->FindRef ("refs", &ref);
			
			BEntry entry (&ref, true);
			entry.GetPath (&path);
			if (entry.IsFile())								// File is dropped
				m_filePathView->SetText (path.Path());
			else if (entry.IsDirectory())					// Folder is dropped
				m_folderPathView->SetText (path.Path());
			
			break;
		}

		default:
			BWindow::MessageReceived (message);
			break;
	}
}

//=============================================================================================================//

void FileSplitterWindow::ToggleWindowHeight (bool expand)
{
	float start = m_hideProgress;
	float finish = m_showProgress + 1;
	float step = 1;
	float snoozeTime = (m_showProgress - m_hideProgress) * 5.0f;
	if (expand == false)
	{
		start = m_showProgress;
		finish = m_hideProgress - 1;
		step = -1;
	}

	for (float i = start; i != finish; i += step)
	{
		ResizeTo (Frame().Width(), i);
		if (expand)
			m_backView->Invalidate (BRect (0, i - 10, Bounds().right, i + 10));
	
		UpdateIfNeeded ();
		snooze ((int32)snoozeTime);
		Flush();
		UpdateIfNeeded ();
	}
}

//=============================================================================================================//

void FileSplitterWindow::UpdateData ()
{
	// Updates the filename entry, folder entry, size and number of pieces display etc
	if (m_filePathView->Text() == NULL)
		return;
	
	m_fragmentCount = m_fragmentSize = 0;
		
	m_fileEntry.SetTo (m_filePathView->Text(), true);
	m_destDir.SetTo (m_folderPathView->Text());

	if (m_sepString)
	{
		free (m_sepString);
		m_sepString = NULL;
	}
	
	if (m_separatorView->Text())
		m_sepString = strdup (m_separatorView->Text());
	else
		m_sepString = strdup ("_");
	
	if (m_fileEntry.Exists() && m_fileEntry.IsFile())
	{
		off_t size;
		m_fileEntry.GetSize (&size);

		BString sizeStr = LocaleStringFromBytes (size);
		if (size > 1024L)
			sizeStr << "  " << "(" << CommaFormatString (size) << " " << str (S_PREFIX_BYTES) << ")";

		m_sizeStr->SetText (sizeStr.String());
		
		// Determine the size of each piece that the user has selected in bytes
		int64 fragmentSize = 0;
		if (m_customSizeView->IsEnabled())
		{
			BString val;
			val << m_customSizeView->Text();
			val << " ";
			
			switch (m_prefixPopUp->IndexOf (m_prefixPopUp->FindMarked()))
			{
				case 0: val << "bytes"; break;
				case 1: val << "KB"; break;
				case 2: val << "MB"; break;
				case 3: val << "GB"; break;
			}
			
			fragmentSize = BytesFromString ((char*)val.String());
			m_fragmentSize = fragmentSize;
		}
		else				// Predefined size
		{
			BString sizeLabel = m_sizePopUp->FindMarked()->Label();
			int32 index = sizeLabel.FindFirst ("-");
			sizeLabel.Truncate (index - 1);

			fragmentSize = BytesFromString ((char*)sizeLabel.String());
			m_fragmentSize = fragmentSize;
		}
		
		m_piecesStr->SetHighColor ((rgb_color){0, 0, 0, 255});
		if (fragmentSize > 0)
		{
			int32 noOfPieces = (size / fragmentSize);
			if (noOfPieces <= 0 && m_customSizeView->Text())
			{
				m_piecesStr->SetText (str (S_WRONG_SPLIT_SIZE));
				m_piecesStr->SetHighColor ((rgb_color){210, 0, 0, 255});
			}
			else if (noOfPieces > kMaxFragmentCount)
			{
				m_piecesStr->SetText (str (S_TOO_MANY_PIECES));
				m_piecesStr->SetHighColor ((rgb_color){0, 0, 210, 255});
			}
			else
			{
				m_fragmentCount = noOfPieces;
				if ((int64)(m_fragmentCount * m_fragmentSize) < size)
					m_fragmentCount++;

				BString temp;
				temp << m_fragmentCount;
				m_piecesStr->SetText (temp.String ());
			}
		}
		else if (fragmentSize < 0)
		{
			m_piecesStr->SetText (str (S_WRONG_SPLIT_SIZE));
			m_piecesStr->SetHighColor ((rgb_color){210, 0, 0, 255});
		}
		else
			m_piecesStr->SetText ("-");
		
		m_statusBar->SetMaxValue (size);
	}
	else
	{
		m_sizeStr->SetText ("-");
		m_piecesStr->SetText ("-");	
	}
		
	m_sizeStr->ResizeToPreferred ();
	m_piecesStr->ResizeToPreferred ();
	if (m_fragmentCount > 0 && m_fragmentSize > 0 && m_destDir.IsDirectory() &&
		m_separatorView->TextView()->TextLength() > 0)
			m_splitBtn->SetEnabled (true);
	else
		m_splitBtn->SetEnabled (false);
}

//=============================================================================================================//

void FileSplitterWindow::UpdateRecentMenus ()
{
	m_recentSplitFiles->UpdateMenu (m_fileMenu, kRecentSplitFile, this);
	m_recentSplitDirs->UpdateMenu (m_folderMenu, kRecentSplitDir, this);
}

//=============================================================================================================//

void FileSplitterWindow::CreateSelfJoiner ()
{
	// Rather a copy self joiner to outputdir etc
	BDirectory *stubDir = &(_bzr()->m_stubDir);
	if (stubDir->IsDirectory() == false)
	{
		BAlert *err = new BAlert ("error", str (S_STUB_DIR_NOT_FOUND), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
					B_STOP_ALERT);
		err->Go();
		return;
	}
	
	stubDir->Rewind();
	BEntry stubEntry;
	stubDir->FindEntry (K_STUB_JOINER_FILE, &stubEntry, true);
	
	char *stubName = new char[B_FILE_NAME_LENGTH + 1];	// if symlink was traversed, name might change
	stubEntry.GetName (stubName);						// hence read the name from the entry again
	
	CopyFile (&stubEntry, &m_destDir, NULL, &m_cancel);		// search the destination dir for the name
	m_destDir.Rewind();										// and then we have our file that we want to
	m_destDir.FindEntry (stubName, &stubEntry, true);		// add the resources to
	delete[] stubName;
	
	char *buf = new char[B_FILE_NAME_LENGTH + 1];
	m_fileEntry.GetName (buf);
	BString name = str (S_STUB_NAME);
	name.IReplaceAll ("%s", buf);
	delete[] buf;
	
	stubEntry.Rename (name.String(), true);						// Rename to "create_filename"
	
	BFile stubFile (&stubEntry, B_READ_WRITE);

	BResources res (&stubFile, false);
	res.AddResource (B_STRING_TYPE, 1, m_firstChunkName.String(), m_firstChunkName.Length(),
				K_FILENAME_ATTRIBUTE);
	res.AddResource (B_STRING_TYPE, 2, m_sepString, strlen (m_sepString), K_SEPARATOR_ATTRIBUTE);

	res.Sync();
	stubFile.Sync();
	stubFile.Unset();
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

int32 FileSplitterWindow::_splitter (void *arg)
{
	// The thread that does controls the split process
	FileSplitterWindow *wnd = (FileSplitterWindow*)arg;
	
	status_t result = SplitFile (&(wnd->m_fileEntry), &(wnd->m_destDir), wnd->m_messenger,
						wnd->m_fragmentSize, wnd->m_fragmentCount, wnd->m_sepString, wnd->m_firstChunkName,
						&(wnd->m_cancel));

	BMessage completeMsg (M_OPERATION_COMPLETE);
	completeMsg.AddInt32 (kResult, result);
	wnd->m_messenger->SendMessage (&completeMsg);

	return result;
}

//=============================================================================================================//
