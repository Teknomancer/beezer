/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <String.h>
#include <TextControl.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <Button.h>
#include <FilePanel.h>
#include <Window.h>
#include <RadioButton.h>
#include <ListView.h>
#include <Bitmap.h>
#include <Alert.h>
#include <ScrollView.h>
#include <CheckBox.h>

#include "PrefsViewPaths.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "AppConstants.h"
#include "Preferences.h"
#include "PrefsListItem.h"
#include "ImageButton.h"
#include "LocalUtils.h"
#include "Shared.h"

#define M_PATH_SELECTED				'pths'
#define M_SELECT_OPEN_PATH			'sopp'
#define M_SELECT_ADD_PATH			'sadp'
#define M_SELECT_EXTRACT_PATH		'setp'
#define M_USE_DIR					'used'
#define M_ARK_DIR					'arkd'
#define M_ADD_CLICKED				'addd'
#define M_REMOVE_CLICKED			'remc'

const char *const kTextCtrlPtr =	"txtctrptr";
const char *const kListCtrlPtr =	"lstctrptr";
const char *const kArkDir =			":arkdir:";

//=============================================================================================================//

PrefsViewPaths::PrefsViewPaths (BRect frame)
	: PrefsView (frame, str (S_PREFS_TITLE_PATHS), str (S_PREFS_DESC_PATHS))
{
	m_messenger = NULL;
	m_message = NULL;
	m_openPanel = NULL;
	SetBitmap (ResBitmap ("Img:Prefs_Paths"));
	Render();
}

//=============================================================================================================//

PrefsViewPaths::~PrefsViewPaths()
{
	delete m_messenger;
	if (m_openPanel)
		delete m_openPanel;
	
	delete m_addBmp;
	delete m_removeBmp;
}

//=============================================================================================================//

void PrefsViewPaths::Render ()
{
	BStringView *defaultStrView = new BStringView (BRect (m_margin, m_margin, 0, 0), NULL,
										str (S_PREFS_PATHS_DEFAULT));
	defaultStrView->SetFont (&m_sectionFont);
	defaultStrView->ResizeToPreferred();
	defaultStrView->SetLowColor (ViewColor());
	
	int8 dividerStrCount = 3;
	BString dividerStrings[] = {
		str (S_PREFS_PATHS_OPEN),
		str (S_PREFS_PATHS_ADD),
		str (S_PREFS_PATHS_EXTRACT)
	};
	
	float divider = -1;
	for (int8 i = 0; i < dividerStrCount - 1; i++)
		divider = MAX (StringWidth (dividerStrings[i].String()), StringWidth (dividerStrings[i+1].String()));
	
	divider += 8;
	
	m_openPathView = new BTextControl (BRect (3 * m_margin, defaultStrView->Frame().bottom + m_vGap + 6, 
					Bounds().right - (2 * m_margin) - K_BUTTON_WIDTH, 0), "PrefsViewPaths:openPathView",
					str (S_PREFS_PATHS_OPEN), NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP,
					B_WILL_DRAW | B_NAVIGABLE);
	m_openPathView->SetDivider (divider);
	m_openPathView->SetAlignment (B_ALIGN_RIGHT, B_ALIGN_LEFT);
	m_openPathView->TextView()->DisallowChar (B_INSERT);
	
	BString buttonText = str (S_PREFS_PATHS_SELECT); buttonText << "...";
	m_openPathBtn = new BButton (BRect (m_openPathView->Frame().right + m_margin, m_openPathView->Frame().top - 4,
							m_openPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
							m_openPathView->Frame().top - 4 + K_BUTTON_HEIGHT), "PrefsViewPaths:openPathBtn",
							buttonText.String(), new BMessage (M_SELECT_OPEN_PATH),
							B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	
	m_addPathView = new BTextControl (BRect (3 * m_margin, m_openPathView->Frame().bottom + m_vGap + 6,
					m_openPathView->Frame().right, 0), "PrefsViewPaths:addPathView", str (S_PREFS_PATHS_ADD),
					NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	m_addPathView->SetDivider (divider);
	m_addPathView->SetAlignment (B_ALIGN_RIGHT, B_ALIGN_LEFT);
	m_addPathView->TextView()->DisallowChar (B_INSERT);
	
	m_addPathBtn = new BButton (BRect (m_addPathView->Frame().right + m_margin, m_addPathView->Frame().top - 4,
							m_addPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
							m_addPathView->Frame().top - 4 + K_BUTTON_HEIGHT), "PrefsViewPaths:addPathBtn",
							buttonText.String(), new BMessage (M_SELECT_ADD_PATH),
							B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	
	BStringView *extractStrView = new BStringView (BRect (m_addPathView->Frame().left,
									m_addPathView->Frame().bottom + 2 * m_vGap + 10,
									m_addPathView->Frame().left + StringWidth (str (S_PREFS_PATHS_EXTRACT)), 0),
									"PrefsViewPaths:extractStrView", str (S_PREFS_PATHS_EXTRACT),
									B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	extractStrView->ResizeToPreferred();
	
	m_arkDirOpt = new BRadioButton (BRect (extractStrView->Frame().left + 3 * m_margin,
							extractStrView->Frame().bottom + m_vGap, 0, 0), "PrefsViewPaths:arkDirOpt",
							str (S_PREFS_PATHS_ARKDIR), new BMessage (M_ARK_DIR), B_FOLLOW_LEFT | B_FOLLOW_TOP,
							B_WILL_DRAW | B_NAVIGABLE);
	m_arkDirOpt->ResizeToPreferred();
	
	m_useDirOpt = new BRadioButton (BRect (m_arkDirOpt->Frame().left, m_arkDirOpt->Frame().bottom + m_vGap + 1,
							0, 0), "PrefsViewPaths:useDirOpt", str (S_PREFS_PATHS_USEDIR),
							new BMessage (M_USE_DIR), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	m_useDirOpt->ResizeToPreferred();
	m_useDirOpt->SetValue (B_CONTROL_ON);
	
	float strW = m_useDirOpt->Frame().right;
	strW = MAX (m_useDirOpt->Frame().right + 4, 3 * m_margin + divider);
		
	m_extractPathView = new BTextControl (BRect (strW, m_arkDirOpt->Frame().bottom + m_vGap,
								m_addPathView->Frame().right, 0), "PrefsViewPaths:extractPathView", NULL,
								NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	m_extractPathView->SetAlignment (B_ALIGN_RIGHT, B_ALIGN_LEFT);
	m_extractPathView->TextView()->DisallowChar (B_INSERT);
	
	m_extractPathBtn = new BButton (BRect (m_extractPathView->Frame().right + m_margin,
								m_extractPathView->Frame().top - 4,
								m_extractPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
								m_extractPathView->Frame().top - 4 + K_BUTTON_HEIGHT), 
								"PrefsViewPaths:extractPathBtn", buttonText.String(),
								new BMessage (M_SELECT_EXTRACT_PATH), B_FOLLOW_LEFT | B_FOLLOW_TOP,
								B_WILL_DRAW | B_NAVIGABLE);
	
	BStringView *favStrView = new BStringView (BRect (defaultStrView->Frame().left,
										m_extractPathView->Frame().bottom + m_margin + m_vGap + 8, 0, 0),
										"PrefsViewPaths:favStrView", str (S_PREFS_PATHS_FAVOURITE),
										B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	favStrView->SetFont (&m_sectionFont);
	favStrView->ResizeToPreferred();
	favStrView->SetLowColor (ViewColor());

	m_genChk = new BCheckBox (BRect (3 * m_margin, favStrView->Frame().bottom, 0, 0),
						"PrefsViewPaths:genChk", str (S_PREFS_PATHS_GENERATE), NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP,
						B_WILL_DRAW | B_NAVIGABLE);
	m_genChk->ResizeToPreferred();
	
	m_favListView = new BListView (BRect (5  * m_margin, favStrView->Frame().bottom + m_vGap + m_margin, 
					m_openPathView->Frame().right - B_V_SCROLL_BAR_WIDTH - m_margin,
					Bounds().bottom - m_margin - B_H_SCROLL_BAR_HEIGHT - m_genChk->Frame().Height() - 6),
					"PrefsViewPaths:favListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT,
					B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);

	m_scrollView = new BScrollView ("PrefsViewPaths:scrollView", m_favListView,
								B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS, true, true,
								B_FANCY_BORDER);

	m_genChk->MoveTo (m_genChk->Frame().left, m_scrollView->Frame().bottom + m_vGap + 4);

	m_addBmp = ResBitmap ("Img:PlusSign");
	m_removeBmp = ResBitmap ("Img:MinusSign");
	
	m_addBtn = new ImageButton (BRect (m_scrollView->Frame().right + m_margin, m_scrollView->Frame().top + 1,
					m_scrollView->Frame().right + 20, m_scrollView->Frame().top + 21), "PrefsViewPaths:addBnt",
					NULL, m_addBmp, NULL, new BMessage (M_ADD_CLICKED), false, ViewColor(),
					kBelowIcon, true, true, true, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);

	m_removeBtn = new ImageButton (BRect (m_addBtn->Frame().left, m_addBtn->Frame().bottom + 2 * m_margin - 3,
					m_addBtn->Frame().right, m_addBtn->Frame().bottom + m_margin + 21), 
					"PrefsViewPaths:removeBtn", NULL, m_removeBmp, NULL, new BMessage (M_REMOVE_CLICKED), false,
					ViewColor(), kBelowIcon, true, true, true, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	
	AddChild (defaultStrView);
	AddChild (m_openPathView);
	AddChild (m_openPathBtn);
	AddChild (m_addPathView);
	AddChild (m_addPathBtn);
	AddChild (extractStrView);
	AddChild (m_arkDirOpt);
	AddChild (m_useDirOpt);
	AddChild (m_extractPathView);
	AddChild (m_extractPathBtn);
	AddChild (favStrView);
	AddChild (m_scrollView);
	AddChild (m_genChk);
	AddChild (m_addBtn);
	AddChild (m_removeBtn);
	AddRevertButton();
}

//=============================================================================================================//

void PrefsViewPaths::Save ()
{
	_prefs_paths.SetString (kPfDefOpenPath, m_openPathView->Text());
	_prefs_paths.SetString (kPfDefAddPath, m_addPathView->Text());
	_prefs_paths.SetString (kPfDefExtractPath, m_extractPathView->Text());
	_prefs_paths.SetBool (kPfUseArkDir, m_arkDirOpt->Value() == B_CONTROL_ON ? true : false);

	BMessage favPathMsg ('fav!');
	for (int32 i = 0; i < m_favListView->CountItems(); i++)
		favPathMsg.AddString (kPath, ((PrefsListItem*)m_favListView->ItemAt(i))->Text());
	
	_prefs_paths.SetMessage (kPfFavPathsMsg, &favPathMsg);
	_prefs_paths.SetBool (kPfGenPath, IsChecked (m_genChk));
	_prefs_paths.WritePrefs();
}

//=============================================================================================================//

void PrefsViewPaths::Load ()
{
	const char *foundPath;
	if (_prefs_paths.FindString (kPfDefOpenPath, &foundPath) == B_OK)
		m_openPathView->SetText (foundPath);
	
	if (_prefs_paths.FindString (kPfDefAddPath, &foundPath) == B_OK)
		m_addPathView->SetText (foundPath);
	
	if (_prefs_paths.FindString (kPfDefExtractPath, &foundPath) == B_OK)
		m_extractPathView->SetText (foundPath);
		
	bool useArkDir;
	if (_prefs_paths.FindBool (kPfUseArkDir, &useArkDir) == B_OK)
	{
		if (useArkDir == true)
			m_arkDirOpt->SetValue (B_CONTROL_ON);
		else
			m_useDirOpt->SetValue (B_CONTROL_ON);

		// m_useDirOpt->Invoke()won't work because SetTarget() in AttchdToWnd() would not yet have been exec
		ToggleExtractPathView (!useArkDir);
	}
	
	BMessage favPathMsg;
	if (_prefs_paths.FindMessage (kPfFavPathsMsg, &favPathMsg) == B_OK)
	{
		m_favListView->MakeEmpty();
		int32 i = 0L;
		while (favPathMsg.FindString (kPath, i++, &foundPath) == B_OK)
			m_favListView->AddItem (new PrefsListItem (foundPath, NULL, false));
	}
	
	m_genChk->SetValue (_prefs_paths.FindBoolDef (kPfGenPath, true));
}

//=============================================================================================================//

void PrefsViewPaths::AttachedToWindow ()
{
	m_messenger = new BMessenger (this);
	m_openPathBtn->SetTarget (this);
	m_addPathBtn->SetTarget (this);
	m_extractPathBtn->SetTarget (this);
	m_arkDirOpt->SetTarget (this);
	m_useDirOpt->SetTarget (this);
	m_addBtn->SetTarget (this);
	m_removeBtn->SetTarget (this);
	PrefsView::AttachedToWindow();
}

//=============================================================================================================//

void PrefsViewPaths::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_SELECT_OPEN_PATH: case M_SELECT_ADD_PATH: case M_SELECT_EXTRACT_PATH: case M_ADD_CLICKED:
		{
			if (m_openPanel == NULL)
			{
				m_openPanel = new BFilePanel (B_OPEN_PANEL, m_messenger, NULL, B_DIRECTORY_NODE, false, NULL,
									NULL, true, true);
				m_openPanel->SetButtonLabel (B_DEFAULT_BUTTON, str (S_PREFS_PATHS_SELECT));
			}

			if (m_message == NULL)
				m_message = new BMessage (M_PATH_SELECTED);
			else
				m_message->RemoveName (kTextCtrlPtr);
			
			BString panelWindowTitle;
			if (message->what == M_SELECT_OPEN_PATH)
			{
				m_message->AddPointer (kTextCtrlPtr, m_openPathView);
				panelWindowTitle = str (S_PREFS_PATHS_OPEN_TITLE);
			}
			else if (message->what == M_SELECT_ADD_PATH)
			{
				m_message->AddPointer (kTextCtrlPtr, m_addPathView);
				panelWindowTitle = str (S_PREFS_PATHS_ADD_TITLE);
			}
			else if (message->what == M_SELECT_EXTRACT_PATH)
			{
				m_message->AddPointer (kTextCtrlPtr, m_extractPathView);
				panelWindowTitle = str (S_PREFS_PATHS_EXTRACT_TITLE);
			}
			else if (message->what == M_ADD_CLICKED)
			{
				m_message->AddPointer (kListCtrlPtr, m_favListView);
				panelWindowTitle = str (S_PREFS_PATHS_FAV_TITLE);
			}
			
			m_openPanel->Window()->SetTitle (panelWindowTitle.String());
			m_openPanel->SetMessage (m_message);
			m_openPanel->Show();
			break;
		}
		
		case M_PATH_SELECTED:
		{
			BTextControl *textControl (NULL);
			BListView *listControl (NULL);
			entry_ref ref;
			message->FindRef ("refs", &ref);
			message->FindPointer (kTextCtrlPtr, reinterpret_cast<void**>(&textControl));
			message->FindPointer (kListCtrlPtr, reinterpret_cast<void**>(&listControl));
			
			BPath pathOfRef (&ref);
			if (textControl)
				textControl->SetText (pathOfRef.Path());
			else
			{
				bool isUnique = true;
				// check if the path is already not there in the list, only then add it
				for (int32 i = 0; i < listControl->CountItems(); i++)
				{
					BString existingPath = ((PrefsListItem*)listControl->ItemAt(i))->Text();

					// don't add if same path is being added
					if (strcmp (existingPath.String(), pathOfRef.Path()) == 0)
					{
						BString errString = str (S_PREFS_PATHS_NOT_UNIQUE);
						errString.ReplaceAll ("%s", existingPath.String());
						BAlert *errAlert = new BAlert ("error", errString.String(), str (S_OK),
												NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
						errAlert->SetShortcut (0L, B_ESCAPE);
						errAlert->Go();
						isUnique = false;
						break;
					}
				}
				
				// Add only paths that are already not present 
				if (isUnique)
				{
					int32 selIndex = listControl->CurrentSelection (0L);
					if (selIndex >= 0)
						listControl->AddItem (new PrefsListItem (pathOfRef.Path(), NULL, false), selIndex);
					else
						listControl->AddItem (new PrefsListItem (pathOfRef.Path(), NULL, false));
				}
			}
			
			break;
		}
		

		case M_USE_DIR:
		{
			ToggleExtractPathView (true);
			break;
		}
		
		case M_ARK_DIR:
		{
			ToggleExtractPathView (false);
			break;
		}
		
		case M_REMOVE_CLICKED:
		{
			int32 selIndex = m_favListView->CurrentSelection (0L);
			if (selIndex >= 0L)
			{
				m_favListView->RemoveItem (selIndex);
				if (m_favListView->CountItems() - 1 >= selIndex)
					m_favListView->Select (selIndex, false);
				else if (m_favListView->CountItems() > 0)
					m_favListView->Select (m_favListView->CountItems() - 1, false);
			}
			
			break;
		}
		
		default:
			PrefsView::MessageReceived (message);
			break;
	}
}

//=============================================================================================================//

void PrefsViewPaths::ToggleExtractPathView (bool enable)
{
	m_extractPathView->SetEnabled (enable);
	m_extractPathBtn->SetEnabled (enable);
}

//=============================================================================================================//
