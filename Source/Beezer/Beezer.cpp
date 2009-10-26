/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <Debug.h>
#include <Roster.h>
#include <Screen.h>
#include <List.h>
#include <FilePanel.h>
#include <Bitmap.h>
#include <Resources.h>
#include <FindDirectory.h>
#include <Button.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <be/add-ons/tracker/TrackerAddOn.h>

#include <fs_attr.h>

#include <image.h>
#include <stdlib.h>

#include "Alert.h"
#include "Beezer.h"
#include "AppConstants.h"
#include "MainWindow.h"
#include "AboutWindow.h"
#include "AuthorWindow.h"
#include "MsgConstants.h"
#include "StartupWindow.h"
#include "AppUtils.h"
#include "WindowMgr.h"
#include "BitmapPool.h"
#include "UIConstants.h"
#include "ArchiverMgr.h"
#include "Preferences.h"
#include "PrefsWindow.h"
#include "PrefsFields.h"
#include "RecentMgr.h"
#include "RuleMgr.h"
#include "FileSplitterWindow.h"
#include "FileJoinerWindow.h"
#include "AddOnWindow.h"

#include "BubbleHelper.h"

//=============================================================================================================//

Beezer::Beezer ()
	: BApplication (K_APP_SIGNATURE),
		m_aboutWnd (NULL),
		m_authorWnd (NULL),
		m_startupWnd (NULL),
		m_prefsWnd (NULL),
		m_splitWnd (NULL),
		m_joinWnd (NULL),
		m_addOnWnd (NULL),
		m_nextWindowID (0L),
		m_nWindows (0L),
		m_openFilePanel (NULL),
		m_createFilePanel (NULL),
		m_bubbleHelper (NULL),
		m_windowMgr (new WindowMgr()),
		m_arkTypePopUp (NULL),
		m_arkTypeField (NULL)
{
	// Setup the bubble helper (tooltips for all you windows users)
	// Currently bubblehelps are disabled due to a conflict between bubblehelps and ImageButton
	//m_bubbleHelper = new BubbleHelper();
	//m_bubbleHelper->SetDelayTime (1200000);
	//m_bubbleHelper->EnableHelp (false);
	
	// Let the initial rectangle be small enough to fit on 640x480 screens
	m_defaultWindowRect.Set (40, 40, 610, 440);

	// If we detect a bigger screen size, make our rectangle bigger
	BScreen screenRect;
	if (screenRect.Frame().Width() > 641 && screenRect.Frame().Height() > 481)
		m_defaultWindowRect.Set (0, 0, 710, 500);
	
	m_newWindowRect = m_defaultWindowRect;
	
	// Very important, critically ordered!
	InitPaths();
	InitPrefs();
	CompileTimeString (true);
	_glob_bitmap_pool = new BitmapPool();
	
	// Load preferences, recents and all that stuff
	int8 numArk, numExt;
	if (_prefs_recent.FindInt8 (kPfNumRecentArk, &numArk) != B_OK) numArk = 10;
	if (_prefs_recent.FindInt8 (kPfNumRecentExt, &numExt) != B_OK) numExt = 5;
	
	m_recentMgr = new RecentMgr (numArk, &_prefs_recent_archives, ritFile,
						_prefs_recent.FindBoolDef (kPfShowPathInRecent, false));
	m_extractMgr = new RecentMgr (numExt, &_prefs_recent_extract, ritFolder, true);
	m_extractMgr->SetCommand (M_RECENT_EXTRACT_ITEM);
	
	m_splitFilesMgr = new RecentMgr (5, &_prefs_recent_splitfiles, ritFile, true);
	m_splitDirsMgr = new RecentMgr (5, &_prefs_recent_splitdirs, ritFolder, true);
	m_splitFilesMgr->SetCommand (M_RECENT_SPLIT_FILE);
	m_splitDirsMgr->SetCommand (M_RECENT_SPLIT_DIR);
	
	m_ruleMgr = new RuleMgr (m_settingsPathStr.String(), K_RULE_FILE);
	
	if (_prefs_misc.FindBoolDef (kPfMimeOnStartup, false))
		RegisterFileTypes();

	// Setup tools menu and popups
	m_toolsMenu = new BMenu (str (S_TOOLS));
	BString buf = str (S_FILE_SPLITTER_TITLE); buf << B_UTF8_ELLIPSIS;
	m_toolsMenu->AddItem (new BMenuItem (buf.String(), new BMessage (M_TOOLS_FILE_SPLITTER)));

	buf = str (S_FILE_JOINER_TITLE); buf << B_UTF8_ELLIPSIS;
	m_toolsMenu->AddItem (new BMenuItem (buf.String(), new BMessage (M_TOOLS_FILE_JOINER)));
	
	// Start the message loop
	Run();
}

//=============================================================================================================//

Beezer::~Beezer ()
{
	DeleteFilePanel (m_openFilePanel);
	DeleteFilePanel (m_createFilePanel);

	for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
	{
		char *arkTypeString = (char*)m_arkTypes.ItemAtFast(i);
		if (arkTypeString)
			free ((char*)arkTypeString);
	}
	
	delete m_toolsMenu;
	delete _glob_bitmap_pool;
	delete m_recentMgr;
	delete m_extractMgr;
	delete m_splitFilesMgr;
	delete m_splitDirsMgr;
	delete m_windowMgr;
	delete m_ruleMgr;

	if (m_arkTypePopUp != NULL)
	{
		delete m_arkTypePopUp;
		m_arkTypePopUp = NULL;
	}
	
	for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
		free ((char*)m_arkExtensions.ItemAtFast(i));
}

//=============================================================================================================//

void Beezer::Quit()
{
	if (CountWindows() > 0)
		for (int32 i = 0; i < CountWindows(); i++)
			WindowAt(i)->PostMessage (B_QUIT_REQUESTED);

	return BApplication::Quit();
}

//=============================================================================================================//

void Beezer::ReadyToRun()
{
	if (m_nWindows == 0 && m_startupWnd == NULL && m_addOnWnd == NULL)
		m_startupWnd = new StartupWindow (m_recentMgr, m_bubbleHelper, true);
	
	return BApplication::ReadyToRun();
}

//=============================================================================================================//

void Beezer::MessageReceived (BMessage *message)
{
	switch (message->what)
	{
		case M_FILE_NEW:
		{
			ShowCreateFilePanel();
			break;
		}
		
		case M_FILE_ABOUT:
		{
			if (m_aboutWnd == NULL)
				m_aboutWnd = new AboutWindow (CompileTimeString (false));
			else
				m_aboutWnd->Activate();
				
			break;
		}
		
		case M_LAUNCH_TRACKER_ADDON:
		{
			if (m_addOnWnd == NULL)
				m_addOnWnd = new AddOnWindow (message);
			else
				m_addOnWnd->Activate();
				
			break;
		}
		
		case M_CLOSE_ADDON:
		{
			m_addOnWnd = NULL;
			if (m_startupWnd == NULL)
				PostMessage (B_QUIT_REQUESTED);
			break;
		}
		
		case M_CLOSE_ABOUT:
		{
			m_aboutWnd = NULL;
			break;
		}

		case M_EDIT_PREFERENCES:
		{
			if (m_prefsWnd == NULL)
				m_prefsWnd = new PrefsWindow ();
			else
				m_prefsWnd->Activate();
			break;
		}
		
		case B_CANCEL:
		{
			// 0.05 bug fix ("calling IsHidden without locking the window") error
			if (m_startupWnd)
				m_startupWnd->LockLooper();
			
			// 0.04 bugfix:: added check for m_startupWnd
			if (m_nWindows == 0 && (m_startupWnd == NULL || m_startupWnd->IsHidden()))
				be_app_messenger.SendMessage (B_QUIT_REQUESTED);

			if (m_startupWnd)
				m_startupWnd->UnlockLooper();

			break;
		}
		
		case M_REGISTER_TYPES:
		{
			int8 regCount = RegisterFileTypes();
			char *buf = new char [strlen (str (S_COMPLETED_REGISTER)) + strlen (K_APP_TITLE) + 10];
			if (regCount > 0)
				sprintf (buf, str (S_COMPLETED_REGISTER), regCount, K_APP_TITLE);
			else if (regCount == 0)
				sprintf (buf, str (S_ALREADY_REGISTERED), K_APP_TITLE);
			else
			{
				delete[] buf;
				buf = NULL;
			}

			if (buf)
			{
				BAlert *a = new BAlert ("done", buf, str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
								B_EVEN_SPACING, B_INFO_ALERT);
				a->SetShortcut (0L, B_ESCAPE);
				a->Go();
				delete[] buf;
			}
						
			break;
		}
		
		case M_CLOSE_PREFS:
		{
			int8 numArk, numExt;
			if (_prefs_recent.FindInt8 (kPfNumRecentArk, &numArk) != B_OK) numArk = 10;
			if (_prefs_recent.FindInt8 (kPfNumRecentExt, &numExt) != B_OK) numExt = 5;
			
			m_recentMgr->SetMaxPaths (numArk);
			m_recentMgr->SetShowFullPath (_prefs_recent.FindBoolDef (kPfShowPathInRecent, false));
			m_extractMgr->SetMaxPaths (numExt);
			
			// Update extract path menu
			m_windowMgr->UpdateFrom (NULL, new BMessage (M_UPDATE_RECENT), false);
			m_windowMgr->UpdateFrom (NULL, new BMessage (M_UPDATE_INTERFACE), false);
			
			m_prefsWnd = NULL;
			break;
		}
	
		case M_FILE_DEVELOPER_INFO:
		{
			if (m_authorWnd == NULL)
				m_authorWnd = new AuthorWindow();
			else
				m_authorWnd->Activate();
			break;
		}
		
		case M_CLOSE_AUTHOR:
		{
			m_authorWnd = NULL;
			break;
		}
		
		case M_CLOSE_STARTUP:
		{
			m_startupWnd = NULL;
			if (m_nWindows == 0)
				be_app_messenger.SendMessage (B_QUIT_REQUESTED);
			
			break;
		}
		
		case M_UNREG_WINDOW:
		{
			UnRegisterWindow (false);
			break;
		}
				
		case M_FILE_OPEN:
		{
			bool firstTime = false;
			if (m_openFilePanel == NULL)
				firstTime = true;
			
			CreateFilePanel (m_openFilePanel, B_OPEN_PANEL);
			m_openFilePanel->Window()->SetTitle (str (S_OPEN_PANEL_TITLE));
			
			const char *openDirPath = _prefs_paths.FindString (kPfDefOpenPath);
			if (firstTime && openDirPath)
				m_openFilePanel->SetPanelDirectory (openDirPath);
			
			m_openFilePanel->Show();
			break;
		}
		
		case M_RECENT_ITEM:
		{
			RefsReceived (message);
			break;
		}
		
		case M_UPDATE_RECENT:
		{
			// this isn't needed as we can StartUpWindow always builds it menu since it is in control
			// of showing the menu, but for windows that is NOT in control of showing (say for example a
			//   window that uses a context menu'd ImageButton() will need to be told of changes to update
			//   its context menu)
			// So i have provided for the future where we may be in a situation where we may not control the
			// showing of the context menu in which case we will need to update it every time a change is
			// made, this is completely useless for the time being -- but its better implemented this way
			break;
		}
		
		case M_CREATE_REQUESTED:
		{
			BMenuItem *arkType = m_arkTypePopUp->FindMarked();
			if (!arkType)
				break;
			
			status_t result;
			Archiver *ark = NewArchiver (arkType->Label(), true, &result);
			if (!ark)
				break;
				
			if (result == BZR_BINARY_MISSING)
				break;
			
			message->AddPointer (kArchiverPtr, ark);
			RegisterWindow()->PostMessage (message);
			break;
		}
		
		case M_ARK_TYPE_SELECTED:
		{
			if (!m_createFilePanel)
				break;
			
			const char *newExtStr = message->FindString (kText);
			if (m_createFilePanel->Window()->LockLooper())
			{
				BTextView *fileNameView = ((BTextControl*)m_createFilePanel->Window()
																->FindView ("text view"))->TextView();
				int32 start, end;
				fileNameView->GetSelection (&start, &end);
				
				BString existingName = fileNameView->Text();
				
				for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
					if (existingName.FindFirst ((char*)m_arkExtensions.ItemAtFast (i)) >= 0L
						&& existingName != newExtStr)
					{
						BString newExtension = existingName;
						newExtension.ReplaceLast ((char*)m_arkExtensions.ItemAtFast(i), newExtStr);

						// overcome a bug in BString in BeOS R5's (libbe.so), BONE/DANO fixes this bug
						#if !B_BEOS_VERSION_DANO
							if (newExtension == (char*)m_arkExtensions.ItemAtFast(i))
								newExtension = newExtStr;
						#endif
						
						fileNameView->SetText (newExtension.String());
						fileNameView->Select (start, end);
						break;
					}
					
				m_createFilePanel->Window()->UnlockLooper();
			}
			
			break;
		}
		
		case M_TOOLS_FILE_SPLITTER:
		{
			if (m_splitWnd == NULL)
			{
				m_splitWnd = new FileSplitterWindow (m_splitFilesMgr, m_splitDirsMgr);
				m_splitWnd->Show ();

				// message might have "refs" field thus pass it, so that the ref can be used if needed
				if (message->HasRef ("refs"))
					m_splitWnd->PostMessage (message);
			}
			else
				m_splitWnd->Activate();

			break;
		}
		
		case M_CLOSE_FILE_SPLITTER:
		{
			m_splitWnd = NULL;
			break;
		}
		

		case M_TOOLS_FILE_JOINER:
		{
			if (m_joinWnd == NULL)
			{
				m_joinWnd = new FileJoinerWindow (m_splitDirsMgr);
				m_joinWnd->Show ();
			}
			else
				m_joinWnd->Activate ();
				
			break;
		}
		
		case M_CLOSE_FILE_JOINER:
		{
			m_joinWnd = NULL;
			break;
		}
		
		case M_FILE_HELP: case M_ADDON_HELP: case M_PREFS_HELP:
		{
			BPath helpFilePath (&m_docsDir, "Index.html");
			BEntry helpFileEntry (helpFilePath.Path(), true);

			if (helpFileEntry.Exists() == false)
			{
				(new BAlert ("error", str (S_HELP_FILE_NOT_FOUND), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
						B_STOP_ALERT))->Go();
			}
			else
			{
				entry_ref ref;
				helpFileEntry.GetRef (&ref);
				be_roster->Launch (&ref);
			}
			break;
		}
		
		default:
			return BApplication::MessageReceived (message);
	}
}

//=============================================================================================================//

MainWindow* Beezer::RegisterWindow (entry_ref *ref)
{
	m_newWindowRect.OffsetBy (15,15);
	if (m_newWindowRect.bottom >= BScreen().Frame().bottom || m_newWindowRect.right >= BScreen().Frame().right)
		m_newWindowRect = m_defaultWindowRect;

	MainWindow *wndPtr = NULL;
	if (ref != NULL)
	{
		// Try and locate the window that has this file open, if so activate THAT window and return
		if ((wndPtr = WindowForRef (ref)) != NULL)
		{
			wndPtr->Activate (true);
			return NULL;
		}
	}

	return CreateWindow (ref);
}

//=============================================================================================================//

void Beezer::UnRegisterWindow (bool closeApp)
{
	// Decrement the REAL window count, if zero quit the entire app
	m_nWindows --;

	// If File->Quit or Close All from Deskbar, we will quit app, otherwise when File->Close is used
	// and if its the last window, bring back the startup window (if prefs allows it or else quit)
	if (m_nWindows == 0)
	{
		if (closeApp)
			be_app_messenger.SendMessage (B_QUIT_REQUESTED);
		else
		{
			if (_prefs_misc.FindBoolDef (kPfWelcomeOnQuit, true))
			{
				if (!m_startupWnd)
					m_startupWnd = new StartupWindow (m_recentMgr, m_bubbleHelper, false);
				else
					m_startupWnd->Show();
			}
			else
				be_app_messenger.SendMessage (B_QUIT_REQUESTED);
		}
	}
}

//=============================================================================================================//

void Beezer::RefsReceived (BMessage *message)
{
	uint32 type;
	int32 count;
	entry_ref ref;
	
	message->GetInfo ("refs", &type, &count);
	if (type != B_REF_TYPE)
		return;

	for (int32 i = --count; i >= 0; i--)
		if (message->FindRef ("refs", i, &ref) == B_OK)
		{
			BEntry entry (&ref, true);		// Traverse link
			entry.GetRef (&ref);
			RegisterWindow (&ref);
		}
}

//=============================================================================================================//

void Beezer::CreateFilePanel (BFilePanel *&panel, file_panel_mode mode)
{
	if (!panel)
		panel = new BFilePanel (mode, &be_app_messenger, NULL, B_FILE_NODE, true);
}

//=============================================================================================================//

void Beezer::DeleteFilePanel (BFilePanel *&panel)
{
	if (panel)
		delete panel;

	panel = NULL;
}

//=============================================================================================================//

MainWindow* Beezer::WindowForRef (entry_ref *ref)
{
	// Find a window that has the corresponding ref already open. We will try as much as possible not
	// to have 2 copies of the same archive running in 2 different windows
	if (ref != NULL)
	{
		int32 wndCount = m_windowMgr->CountWindows();
		for (int32 i = 0; i < wndCount; i++)
		{
			MainWindow *wndPtr = reinterpret_cast<MainWindow*>(m_windowMgr->WindowAt(i));
			if (wndPtr->m_archiveRef == *ref)
				return wndPtr;
		}
	}
	
	return NULL;
}

//=============================================================================================================//

MainWindow* Beezer::CreateWindow (entry_ref *ref)
{
	// Increment real window count and unique window ID (we never decrement Window ID to keep it unique)
	m_nWindows++;
	m_nextWindowID++;

	// Hide startup window
	if (m_startupWnd && m_startupWnd->Lock())
	{
		if (m_startupWnd->IsHidden() == false)
			m_startupWnd->Hide();

		m_startupWnd->Unlock();
	}

	MainWindow *wndPtr = new MainWindow (m_newWindowRect, m_bubbleHelper, m_windowMgr, m_recentMgr,
									m_extractMgr, m_ruleMgr);
	m_windowMgr->AddWindow (wndPtr);
	if (ref)
		wndPtr->LoadSettingsFromArchive (ref);
	else
		wndPtr->LoadDefaultSettings();	// in case of creating an archive
	
	wndPtr->Show();
		
	if (ref)
	{
		// It is important we get a synchronous reply till the window has completed its
		// critical section
		BMessage msg (M_OPEN_REQUESTED), reply;
		msg.AddRef (kRef, ref);
		
		BMessenger messenger (wndPtr);
		messenger.SendMessage (&msg, &reply);
		
		// Later get from prefs if we must show comments when archive loads
		if (_prefs_misc.FindBoolDef (kPfShowCommentOnOpen, true) && reply.HasBool (kFailOnNull) == false)
		{
			msg.what = M_ACTIONS_COMMENT;
			msg.AddBool (kFailOnNull, true);
			msg.RemoveName (kRef);
			messenger.SendMessage (&msg, &reply);
		}
	}
	
	return wndPtr;
}

//=============================================================================================================//

inline void Beezer::InitPaths ()
{
	// Initialize paths (maybe we can get folder names from prefs someday)
	app_info appInfo;
	be_app->GetAppInfo (&appInfo);
	
	BEntry appEntry (&appInfo.ref);
	appEntry.GetParent (&appEntry);

#ifdef B_ZETA_VERSION
	BPath langDirPath (&appEntry);
	if (langDirPath.Append (K_LANG_DIR_NAME, true) == B_OK)
	{
		langDirPath.Append (K_APP_TITLE, false);
		be_locale.LoadLanguageFile (langDirPath.Path());
	}
#endif

	BPath addonsDirPath (&appEntry);
	if (addonsDirPath.Append (K_ARK_DIR_NAME) == B_OK)
		m_addonsDir.SetTo (addonsDirPath.Path());

	BPath docsPath (&appEntry);
	if (docsPath.Append (K_DOC_DIR_NAME) == B_OK)
		m_docsDir.SetTo (docsPath.Path());
	
	BPath settingsPath (&appEntry);
	m_settingsPathStr = settingsPath.Path();
	m_settingsPathStr << "/" << K_SETTINGS_DIR_NAME;
	if (settingsPath.Append (K_SETTINGS_DIR_NAME) == B_OK)
		m_settingsDir.SetTo (settingsPath.Path());
		
	BPath binDirPath (&appEntry);
	if (binDirPath.Append (K_BIN_DIR_NAME) == B_OK)
		m_binDir.SetTo (binDirPath.Path());
	else
	{
		char binDir[B_PATH_NAME_LENGTH];
		find_directory (B_BEOS_BIN_DIRECTORY, (dev_t)NULL, false, binDir, B_PATH_NAME_LENGTH);
		m_binDir.SetTo (binDir);
	}
	
	BPath stubDirPath (&appEntry);
	if (stubDirPath.Append (K_STUB_DIR_NAME) == B_OK)
		m_stubDir.SetTo (stubDirPath.Path());
}

//=============================================================================================================//

void Beezer::InitPrefs ()
{
	const char *dir = m_settingsPathStr.String();
	_prefs_colors.Init (dir, K_SETTINGS_COLORS);
	_prefs_paths.Init (dir, K_SETTINGS_PATHS);
	_prefs_extract.Init (dir, K_SETTINGS_EXTRACT);
	_prefs_add.Init (dir, K_SETTINGS_ADD);
	_prefs_state.Init (dir, K_SETTINGS_STATE);
	_prefs_windows.Init (dir, K_SETTINGS_WINDOWS);
	_prefs_recent.Init (dir, K_SETTINGS_RECENT);
	_prefs_lang.Init (dir, K_SETTINGS_LANG);
	_prefs_misc.Init (dir, K_SETTINGS_MISC);
	_prefs_recent_archives.Init (dir, K_SETTINGS_RECENT_ARCHIVES);
	_prefs_recent_extract.Init (dir, K_SETTINGS_RECENT_EXTRACT);
	_prefs_interface.Init (dir, K_SETTINGS_INTERFACE);
	_prefs_recent_splitfiles.Init (dir, K_SETTINGS_RECENT_SPLIT_FILES);
	_prefs_recent_splitdirs.Init (dir, K_SETTINGS_RECENT_SPLIT_DIRS);
}

//=============================================================================================================//

const char* Beezer::CompileTimeString (bool writeToResIfNeeded) const
{
	bool expand_month = true;
	bool strip_seconds = false;
	bool twelve_hour = true;
	
	const char *buildTime = __TIME__;
	const char *buildDate = __DATE__;

	char *month = new char[15];
	int i = 0;
	for (; i < 3; i++)
		month[i] = buildDate[i];
	month[i] = '\0';
	
	if (expand_month)
	{
		if (strcmp (month, "Jan") == 0)			sprintf (month, "%s", "January ");
		else if (strcmp (month, "Feb") == 0)	sprintf (month, "%s", "February ");
		else if (strcmp (month, "Mar") == 0)	sprintf (month, "%s", "March ");
		else if (strcmp (month, "Apr") == 0)	sprintf (month, "%s", "April ");
		else if (strcmp (month, "May") == 0)	sprintf (month, "%s", "May ");
		else if (strcmp (month, "Jun") == 0)	sprintf (month, "%s", "June ");
		else if (strcmp (month, "Jul") == 0)	sprintf (month, "%s", "July ");
		else if (strcmp (month, "Aug") == 0)	sprintf (month, "%s", "August ");
		else if (strcmp (month, "Sep") == 0)	sprintf (month, "%s", "September ");
		else if (strcmp (month, "Oct") == 0)	sprintf (month, "%s", "October ");
		else if (strcmp (month, "Nov") == 0)	sprintf (month, "%s", "November ");
		else if (strcmp (month, "Dec") == 0)	sprintf (month, "%s", "December ");
	}
	
	char *timestr = new char[12];
	strcpy (timestr, buildTime);
	if (strip_seconds)
		timestr[5] = '\0';
	
	if (twelve_hour)
	{
		for (int i = 0; i < 2; i++)
			timestr[i] = buildTime[i];

		if (atoi (timestr) > 12)
		{
			int twelvehr = atoi (timestr) - 12;
			sprintf (timestr, "%d%s", twelvehr, timestr + 2);
			strcat (timestr, " PM");
		}
		else
			strcat (timestr, " AM");
	}
	
	BString compileTimeStr = buildDate + 3;
	switch (*(buildDate + 5))
	{
		case '1': compileTimeStr.Insert ("st", 3); break;
		case '2': compileTimeStr.Insert ("nd", 3); break;
		case '3': compileTimeStr.Insert ("rd", 3); break;
		case '0': case '4': case '5':
			case '6': case '7': case '8': case '9': compileTimeStr.Insert ("th", 3); break;
	}
	compileTimeStr.Insert (month, 6);

	// Aargh... we need to correct these :)
	compileTimeStr.ReplaceFirst ("12nd", "12th");
	compileTimeStr.ReplaceFirst ("13rd", "13th");
	compileTimeStr.ReplaceFirst ("11st", "11th");

	#ifdef B_ZETA_VERSION
		compileTimeStr.Remove (0, 2);
	#endif

	while (compileTimeStr.ByteAt (0) == ' ')
			compileTimeStr.Remove (0, 1);

	compileTimeStr << ", " << timestr;

	if (writeToResIfNeeded == true)
	{
		BString ctFileStr = m_settingsPathStr.String();
		ctFileStr << K_COMPILED_TIME_FILE;
		BFile ctFile (ctFileStr.String(), B_READ_WRITE);
		if (ctFile.InitCheck() == B_OK)
		{
			off_t size;
			ctFile.GetSize (&size);
			char oldCompileTimeStr [size+1];
			ssize_t amtRead = ctFile.Read (&oldCompileTimeStr, size);
			oldCompileTimeStr[amtRead] = '0';
			if (strncmp (oldCompileTimeStr, compileTimeStr.String(), amtRead) != 0)
				WriteToCTFile (&ctFile, &compileTimeStr);
		}
		else
		{
			ctFile.SetTo (ctFileStr.String(), B_WRITE_ONLY | B_CREATE_FILE);
			WriteToCTFile (&ctFile, &compileTimeStr);
		}
			
		ctFile.Unset();
	}
	
	return strdup (compileTimeStr.String());
}

//=============================================================================================================//

void Beezer::WriteToCTFile (BFile *ctFile, BString *compileTimeStr) const
{
	// Only called by CompileTimeString() -- never call from anywhere else unless you know
	// what you're doing
	ctFile->SetSize (0);
	ctFile->Seek (0, SEEK_SET);
	ctFile->Write ((void*)compileTimeStr->String(), compileTimeStr->Length());
}

//=============================================================================================================//

void Beezer::ShowCreateFilePanel ()
{
	CreateFilePanel (m_createFilePanel, B_SAVE_PANEL);
	BWindow *panelWnd = m_createFilePanel->Window();
	panelWnd->SetTitle (str (S_CREATE_PANEL_TITLE));
	
	if (m_arkTypeField == NULL && panelWnd->LockLooper())
	{
		m_createFilePanel->SetButtonLabel (B_DEFAULT_BUTTON, str (S_CREATE));
		
		BView *backView = panelWnd->ChildAt (0L);
		BButton *saveBtn = (BButton*)panelWnd->FindView ("default button");
		saveBtn->SetLabel (str (S_CREATE));
		BTextControl *textField = (BTextControl*)panelWnd->FindView ("text view");
		textField->ResizeBy (-20, 0);
		textField->TextView()->DisallowChar ('*');
		
		if (m_arkTypePopUp)
			delete m_arkTypePopUp;
		
		m_arkTypes = ArchiversInstalled (&m_arkExtensions);
		m_arkTypePopUp = BuildArchiveTypesMenu (this, &m_arkExtensions);
		m_arkTypeField = new BMenuField (BRect (textField->Frame().right + K_MARGIN,
								textField->Frame().top - 2, backView->Frame().Width(), 0),
								"Beezer:arkTypeField", str (S_ARCHIVE_TYPE), (BMenu*)m_arkTypePopUp,
								B_FOLLOW_BOTTOM, B_WILL_DRAW);
		m_arkTypeField->SetDivider (be_plain_font->StringWidth (str (S_ARCHIVE_TYPE)) + 5);
		
		if (m_arkTypes.CountItems() > 0)
		{
			// Restore default archiver from prefs or set it to the LAST archiver on the list
			BString arkType;
			BMenuItem *item = NULL;
			status_t wasFound = _prefs_misc.FindString (kPfDefaultArk, &arkType);
			if (wasFound == B_OK)
				item = m_arkTypePopUp->FindItem (arkType.String());
			
			if (wasFound != B_OK || item == NULL)
				item = m_arkTypePopUp->ItemAt (m_arkTypePopUp->CountItems() - 1);
			
			item->SetMarked (true);
			textField->SetText ((char*)m_arkExtensions.ItemAtFast(m_arkTypePopUp->IndexOf (item)));
		}
		
		backView->AddChild (m_arkTypeField);
		
		if (strlen(textField->Text()) == 0)
			saveBtn->SetEnabled (false);

		panelWnd->UnlockLooper();
		m_createFilePanel->SetMessage (new BMessage (M_CREATE_REQUESTED));
		m_createFilePanel->SetTarget (this);
	}
	
	m_createFilePanel->Show();
}

//=============================================================================================================//

int8 Beezer::RegisterFileTypes () const
{
	const BString fileTypeFieldName = "BEOS:FILE_TYPES";
	app_info appInfo;
	be_app->GetAppInfo (&appInfo);
	
	BNode appNode (&appInfo.ref);
	BMessage attrMsg;
	attr_info attrInfo;
	appNode.GetAttrInfo (fileTypeFieldName.String(), &attrInfo);
	
	// This BMallocIO thing was suggested by mmu_man - many thanks to him!
	BMallocIO mio;
	mio.SetSize (attrInfo.size);

	appNode.ReadAttr (fileTypeFieldName.String(), B_MESSAGE_TYPE, 0, (void*)mio.Buffer(), attrInfo.size);
	attrMsg.Unflatten (&mio);
	
	uint32 type;
	int32 count;
	BString mimeTypeStr;
	
	attrMsg.GetInfo ("types", &type, &count);
	if (type != B_STRING_TYPE)
		return -1;

	bool alreadyAssociated = true;
	bool skipFurtherAlerts = false;
	int8 regCount = 0L;
	for (int32 i = --count; i >= 0; i--)
		if (attrMsg.FindString ("types", i, &mimeTypeStr) == B_OK)
		{
			BMimeType mimeType (mimeTypeStr.String());
			char currentPreferredApp[B_MIME_TYPE_LENGTH];

			mimeType.GetPreferredApp (currentPreferredApp, B_OPEN);
			if (strcmp (currentPreferredApp, K_APP_SIGNATURE) != 0 || !mimeType.IsInstalled())
			{
				entry_ref ref;
				be_roster->FindApp (currentPreferredApp, &ref);
				
				BString buf;
				if (!mimeType.IsInstalled())
					buf = str (S_INSTALL_MIMETYPE);
				else
					buf = str (S_MAKE_APP_PREFERRED);
				
				buf.ReplaceAll ("%s", K_APP_TITLE);
				buf.ReplaceAll ("%o", ref.name);
				buf.ReplaceAll ("%t", mimeTypeStr.String());
				
				int32 index = 2L;
				if (skipFurtherAlerts == false)
				{
					BAlert *confAlert = new BAlert ("mime", buf.String(), str (S_NO), str (S_MAKE_DEFAULT),
											str (S_REGISTER_ALL), B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
											B_WARNING_ALERT);
					confAlert->SetDefaultButton (confAlert->ButtonAt (1L));
					index = confAlert->Go();
				}
				
				alreadyAssociated = false;
				if (index == 2L)
				{
					skipFurtherAlerts = true;
					index = 1L;
				}
				
				if (index == 1L)
				{
					if (!mimeType.IsInstalled())
						mimeType.Install();
					
					mimeType.SetPreferredApp (K_APP_SIGNATURE, B_OPEN);
					regCount++;
				}
			}
		}
	
	if (alreadyAssociated)
		return 0L;
	else if (regCount != 0)
		return regCount;
	else
		return -1L;
}

//=============================================================================================================//

BMenu* Beezer::BuildToolsMenu () const
{
	// We archive and instantiate menu because BMenu doesn't have copy constructor, and
	// neither do BMenuItems
	BMessage toolsMenuMsg;
	m_toolsMenu->Archive (&toolsMenuMsg, true);
	
	return new BMenu (&toolsMenuMsg);
}

//=============================================================================================================//

BPopUpMenu* Beezer::BuildToolsPopUpMenu () const
{
	// We archive and instantiate menu because BPopUpsMenu doesn't have copy constructor, and
	// neither do BMenuItems
	BMessage toolsMenuMsg;
	m_toolsMenu->Archive (&toolsMenuMsg, true);

	return new BPopUpMenu (&toolsMenuMsg);
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

int main ()
{
	srand (0);
	delete new Beezer();
	return B_OK;
}

//=============================================================================================================//
