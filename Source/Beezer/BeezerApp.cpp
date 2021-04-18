// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "BeezerApp.h"
#include "AboutWindow.h"
#include "AddOnWindow.h"
#include "Alert.h"
#include "AppConstants.h"
#include "ArchiverMgr.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "FileJoinerWindow.h"
#include "FileSplitterWindow.h"
#include "MainWindow.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsWindow.h"
#include "RecentMgr.h"
#include "RuleMgr.h"
#include "Shared.h"
#include "StartupWindow.h"
#include "UIConstants.h"
#include "WindowMgr.h"

#include <AppFileInfo.h>
#include <Button.h>
#include <DateTimeFormat.h>
#include <FindDirectory.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <Screen.h>
#include <TextControl.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BeezerApp"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif

#include <kernel/fs_attr.h>
#include <kernel/image.h>
#include <os/add-ons/tracker/TrackerAddOn.h>
#include <support/parsedate.h>


BeezerApp* _bzr()
{
    return dynamic_cast<BeezerApp*>(be_app);
}


BeezerApp::BeezerApp()
    : BApplication(K_APP_SIGNATURE),
      m_aboutWnd(NULL),
      m_startupWnd(NULL),
      m_prefsWnd(NULL),
      m_splitWnd(NULL),
      m_joinWnd(NULL),
      m_addOnWnd(NULL),
      m_nextWindowID(0L),
      m_nWindows(0L),
      m_openFilePanel(NULL),
      m_createFilePanel(NULL),
      m_windowMgr(new WindowMgr()),
      m_arkTypePopUp(NULL),
      m_arkTypeField(NULL)
{
    // Let the initial rectangle be small enough to fit on 640x480 screens
    m_defaultWindowRect.Set(40, 40, 610, 440);

    // If we detect a bigger screen size, make our rectangle bigger
    BRect screenRect(BScreen().Frame());
    if (screenRect.Width() > 641 && screenRect.Height() > 481)
    {
        m_defaultWindowRect.Set(0, 0, 710, 500);
        m_defaultWindowRect.OffsetBy(screenRect.Width()/6, screenRect.Height()/6);
    }

    m_newWindowRect = m_defaultWindowRect;

    // Very important, critically ordered!
    InitPaths();
    InitPrefs();
    _glob_bitmap_pool = new BitmapPool();

    m_archiverMgr = new ArchiverMgr(); // must be done after paths are set up

    // Load preferences, recents and all that stuff
    int8 numArk, numExt;
    if (_prefs_recent.FindInt8(kPfNumRecentArk, &numArk) != B_OK) numArk = 10;
    if (_prefs_recent.FindInt8(kPfNumRecentExt, &numExt) != B_OK) numExt = 5;

    m_recentMgr = new RecentMgr(numArk, &_prefs_recent_archives, ritFile,
                                _prefs_recent.FindBoolDef(kPfShowPathInRecent, false));
    m_extractMgr = new RecentMgr(numExt, &_prefs_recent_extract, ritFolder, true);
    m_extractMgr->SetCommand(M_RECENT_EXTRACT_ITEM);

    m_splitFilesMgr = new RecentMgr(5, &_prefs_recent_splitfiles, ritFile, true);
    m_splitDirsMgr = new RecentMgr(5, &_prefs_recent_splitdirs, ritFolder, true);
    m_splitFilesMgr->SetCommand(M_RECENT_SPLIT_FILE);
    m_splitDirsMgr->SetCommand(M_RECENT_SPLIT_DIR);

    m_ruleMgr = new RuleMgr(&m_settingsDir, K_RULE_FILE);

    m_archiverMgr->MergeArchiverRules(m_ruleMgr);

    if (_prefs_misc.FindBoolDef(kPfMimeOnStartup, false))
        RegisterFileTypes();

    // Setup tools menu and popups
    m_toolsMenu = new BMenu(BZ_TR(kToolsString));
    BString buf = B_TRANSLATE("File splitter"); buf << B_UTF8_ELLIPSIS;
    m_toolsMenu->AddItem(new BMenuItem(buf.String(), new BMessage(M_TOOLS_FILE_SPLITTER)));

    buf = B_TRANSLATE("File joiner"); buf << B_UTF8_ELLIPSIS;
    m_toolsMenu->AddItem(new BMenuItem(buf.String(), new BMessage(M_TOOLS_FILE_JOINER)));
}


BeezerApp::~BeezerApp()
{
    DeleteFilePanel(m_openFilePanel);
    DeleteFilePanel(m_createFilePanel);

    for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
    {
        char* arkTypeString = (char*)m_arkTypes.ItemAtFast(i);
        if (arkTypeString)
            free((char*)arkTypeString);
    }

    delete m_toolsMenu;
    delete _glob_bitmap_pool;
    delete m_recentMgr;
    delete m_extractMgr;
    delete m_splitFilesMgr;
    delete m_splitDirsMgr;
    delete m_windowMgr;
    delete m_ruleMgr;
    delete m_archiverMgr;

    if (m_arkTypePopUp != NULL)
    {
        delete m_arkTypePopUp;
        m_arkTypePopUp = NULL;
    }

    for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
        free((char*)m_arkExtensions.ItemAtFast(i));
}


void BeezerApp::Quit()
{
    if (CountWindows() > 0)
        for (int32 i = 0; i < CountWindows(); i++)
            WindowAt(i)->PostMessage(B_QUIT_REQUESTED);

    return BApplication::Quit();
}


void BeezerApp::ReadyToRun()
{
    if (m_nWindows == 0 && m_startupWnd == NULL && m_addOnWnd == NULL)
        m_startupWnd = new StartupWindow(m_recentMgr, true);

    return BApplication::ReadyToRun();
}


void BeezerApp::AboutRequested()
{
    if (m_aboutWnd == NULL)
    {
        BString compileStr, versionStr;
        CompileTimeString(compileStr);
        VersionString(versionStr);
        m_aboutWnd = new AboutWindow(versionStr, compileStr);
    }
    else
        m_aboutWnd->Activate();
}


void BeezerApp::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_FILE_NEW:
        {
            ShowCreateFilePanel();
            break;
        }

        case M_LAUNCH_TRACKER_ADDON:
        {
            if (m_addOnWnd == NULL)
                m_addOnWnd = new AddOnWindow(message);
            else
                m_addOnWnd->Activate();

            break;
        }

        case M_CLOSE_ADDON:
        {
            m_addOnWnd = NULL;
            if (m_startupWnd == NULL)
                PostMessage(B_QUIT_REQUESTED);
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
                m_prefsWnd = new PrefsWindow();
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
                be_app_messenger.SendMessage(B_QUIT_REQUESTED);

            if (m_startupWnd)
                m_startupWnd->UnlockLooper();

            break;
        }

        case M_REGISTER_TYPES:
        {
            int8 regCount = RegisterFileTypes();
            BString buf;
            if (regCount > 0)
            {
	            buf = B_TRANSLATE("Completed registering of file types. Associated %count% file types with %appname%.");
	            BString countBuf;
	            countBuf.SetToFormat("%d", regCount);
	            buf.ReplaceAll("%count%", countBuf);
            }
            else if (regCount == 0)
                buf = B_TRANSLATE("%appname% has already been associated with all supported archive types.");

            buf.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));

            BAlert* a = new BAlert("Done", buf, BZ_TR(kOKString), NULL, NULL,
                                   B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
            a->SetShortcut(0L, B_ESCAPE);
            a->Go();

            break;
        }

        case M_CLOSE_PREFS:
        {
            int8 numArk, numExt;
            if (_prefs_recent.FindInt8(kPfNumRecentArk, &numArk) != B_OK) numArk = 10;
            if (_prefs_recent.FindInt8(kPfNumRecentExt, &numExt) != B_OK) numExt = 5;

            m_recentMgr->SetMaxPaths(numArk);
            m_recentMgr->SetShowFullPath(_prefs_recent.FindBoolDef(kPfShowPathInRecent, false));
            m_extractMgr->SetMaxPaths(numExt);

            // Update extract path menu
            m_windowMgr->UpdateFrom(NULL, new BMessage(M_UPDATE_RECENT), false);
            m_windowMgr->UpdateFrom(NULL, new BMessage(M_UPDATE_INTERFACE), false);

            m_prefsWnd = NULL;
            break;
        }

        case M_CLOSE_STARTUP:
        {
            m_startupWnd = NULL;
            if (m_nWindows == 0)
                be_app_messenger.SendMessage(B_QUIT_REQUESTED);

            break;
        }

        case M_UNREG_WINDOW:
        {
            UnRegisterWindow(false);
            break;
        }

        case M_FILE_OPEN:
        {
            bool firstTime = false;
            if (m_openFilePanel == NULL)
                firstTime = true;

            CreateFilePanel(m_openFilePanel, B_OPEN_PANEL);
            m_openFilePanel->Window()->SetTitle(B_TRANSLATE("Open archive"));

            const char* openDirPath = _prefs_paths.FindString(kPfDefOpenPath);
            if (firstTime && openDirPath)
                m_openFilePanel->SetPanelDirectory(openDirPath);

            m_openFilePanel->Show();
            break;
        }

        case M_RECENT_ITEM:
        {
            RefsReceived(message);
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
            BMenuItem* arkType = m_arkTypePopUp->FindMarked();
            if (!arkType)
                break;

            status_t result;
            Archiver* ark = m_archiverMgr->NewArchiver(arkType->Label(), true, &result);
            if (!ark)
                break;

            if (result == BZR_BINARY_MISSING)
                break;

            message->AddPointer(kArchiverPtr, ark);
            RegisterWindow()->PostMessage(message);
            break;
        }

        case M_ARK_TYPE_SELECTED:
        {
            if (!m_createFilePanel)
                break;

            const char* newExtStr = message->FindString(kText);
            if (m_createFilePanel->Window()->LockLooper())
            {
                BTextView* fileNameView = ((BTextControl*)m_createFilePanel->Window()
                                           ->FindView("text view"))->TextView();
                int32 start, end;
                fileNameView->GetSelection(&start, &end);

                BString existingName = fileNameView->Text();

                for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
                    if (existingName.FindFirst((char*)m_arkExtensions.ItemAtFast(i)) >= 0L
                            && existingName != newExtStr)
                    {
                        BString newExtension = existingName;
                        newExtension.ReplaceLast((char*)m_arkExtensions.ItemAtFast(i), newExtStr);

                        // overcome a bug in BString in BeOS R5's (libbe.so), BONE/DANO fixes this bug
#if !B_BEOS_VERSION_DANO
                        if (newExtension == (char*)m_arkExtensions.ItemAtFast(i))
                            newExtension = newExtStr;
#endif

                        fileNameView->SetText(newExtension.String());
                        fileNameView->Select(start, end);
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
                m_splitWnd = new FileSplitterWindow(m_splitFilesMgr, m_splitDirsMgr);
                m_splitWnd->Show();

                // message might have "refs" field thus pass it, so that the ref can be used if needed
                if (message->HasRef("refs"))
                    m_splitWnd->PostMessage(message);
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
                m_joinWnd = new FileJoinerWindow(m_splitDirsMgr);
                m_joinWnd->Show();
            }
            else
                m_joinWnd->Activate();

            break;
        }

        case M_CLOSE_FILE_JOINER:
        {
            m_joinWnd = NULL;
            break;
        }

        case M_HELP_MANUAL: case M_ADDON_HELP: case M_PREFS_HELP:
        {
            BPath helpFilePath(&m_docsDir, "Index.html");
            BEntry helpFileEntry(helpFilePath.Path(), true);

            if (helpFileEntry.Exists() == false)
            {
                (new BAlert("Error", B_TRANSLATE("Couldn't locate the help files."), BZ_TR(kOKString),
                            NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
            }
            else
            {
                entry_ref ref;
                helpFileEntry.GetRef(&ref);
                be_roster->Launch(&ref);
            }
            break;
        }

        case M_HELP_WEBSITE: case M_HELP_GITHUB:
        {
            BString launchURL(message->what == M_HELP_WEBSITE ? K_APP_WEBSITE_URL : K_APP_GITHUB_URL);
            char* pURL((char*) launchURL.String());

            status_t rc = be_roster->Launch("application/x-vnd.Be.URL.https", 1, &pURL);
            if (rc != B_OK && rc != B_ALREADY_RUNNING)
                (new BAlert("Error", B_TRANSLATE("Failed to launch URL"), BZ_TR(kOKString),
                            NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
            break;
        }

        default:
            return BApplication::MessageReceived(message);
    }
}


MainWindow* BeezerApp::RegisterWindow(entry_ref* ref)
{
    MainWindow* wndPtr = NULL;
    if (ref != NULL)
    {
        // Try and locate the window that has this file open, if so activate THAT window and return
        if ((wndPtr = WindowForRef(ref)) != NULL)
        {
            wndPtr->Activate(true);
            return NULL;
        }
    }

    return CreateWindow(ref);
}


void BeezerApp::UnRegisterWindow(bool closeApp)
{
    // Decrement the REAL window count, if zero quit the entire app
    m_nWindows--;

    // If File->Quit or Close All from Deskbar, we will quit app, otherwise when File->Close is used
    // and if its the last window, bring back the startup window (if prefs allows it or else quit)
    if (m_nWindows == 0)
    {
        if (closeApp)
            be_app_messenger.SendMessage(B_QUIT_REQUESTED);
        else
        {
            if (_prefs_misc.FindBoolDef(kPfWelcomeOnQuit, true))
            {
                if (!m_startupWnd)
                    m_startupWnd = new StartupWindow(m_recentMgr, false);
                else
                    m_startupWnd->Show();
            }
            else
                be_app_messenger.SendMessage(B_QUIT_REQUESTED);
        }
    }
}


void BeezerApp::ArgvReceived(int32 argc, char** argv)
{
    for (int32 arg = 1; arg < argc; arg++)
    {
        BEntry entry(argv[arg], true);         // Traverse link
        if (entry.InitCheck() == B_OK && entry.Exists())
        {
            entry_ref ref;
            entry.GetRef(&ref);
            RegisterWindow(&ref);
        }
    }
}


void BeezerApp::RefsReceived(BMessage* message)
{
    uint32 type;
    int32 count;
    entry_ref ref;

    message->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return;

    for (int32 i = --count; i >= 0; i--)
        if (message->FindRef("refs", i, &ref) == B_OK)
        {
            BEntry entry(&ref, true);         // Traverse link
            entry.GetRef(&ref);
            RegisterWindow(&ref);
        }
}


void BeezerApp::CreateFilePanel(BFilePanel*& panel, file_panel_mode mode)
{
    if (!panel)
        panel = new BFilePanel(mode, &be_app_messenger, NULL, B_FILE_NODE, true);
}


void BeezerApp::DeleteFilePanel(BFilePanel*& panel)
{
    if (panel)
        delete panel;

    panel = NULL;
}


MainWindow* BeezerApp::WindowForRef(entry_ref* ref)
{
    // Find a window that has the corresponding ref already open. We will try as much as possible not
    // to have 2 copies of the same archive running in 2 different windows
    if (ref != NULL)
    {
        int32 wndCount = m_windowMgr->CountWindows();
        for (int32 i = 0; i < wndCount; i++)
        {
            MainWindow* wndPtr = reinterpret_cast<MainWindow*>(m_windowMgr->WindowAt(i));
            if (wndPtr->m_archiveRef == *ref)
                return wndPtr;
        }
    }

    return NULL;
}


MainWindow* BeezerApp::CreateWindow(entry_ref* ref)
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

    MainWindow* wndPtr = new MainWindow(m_newWindowRect, m_windowMgr, m_recentMgr,
                                        m_extractMgr, m_ruleMgr);

    m_newWindowRect.OffsetBy(30, 30);
    if (m_newWindowRect.bottom >= BScreen().Frame().bottom || m_newWindowRect.right >= BScreen().Frame().right)
        m_newWindowRect = m_defaultWindowRect;

    m_windowMgr->AddWindow(wndPtr);
    if (ref)
        wndPtr->LoadSettingsFromArchive(ref);
    else
        wndPtr->LoadDefaultSettings();    // in case of creating an archive

    wndPtr->Show();

    if (ref)
    {
        // It is important we get a synchronous reply till the window has completed its
        // critical section
        BMessage msg(M_OPEN_REQUESTED), reply;
        msg.AddRef(kRef, ref);

        BMessenger messenger(wndPtr);
        messenger.SendMessage(&msg, &reply);

        // Later get from prefs if we must show comments when archive loads
        if (_prefs_misc.FindBoolDef(kPfShowCommentOnOpen, true) && reply.HasBool(kFailOnNull) == false)
        {
            msg.what = M_ACTIONS_COMMENT;
            msg.AddBool(kFailOnNull, true);
            msg.RemoveName(kRef);
            messenger.SendMessage(&msg, &reply);
        }
    }

    return wndPtr;
}


inline void BeezerApp::InitPaths()
{
    // Initialize paths (maybe we can get folder names from prefs someday)
    app_info appInfo;
    be_app->GetAppInfo(&appInfo);

    BEntry appEntry(&appInfo.ref);
    appEntry.GetParent(&appEntry);

    BPath addonsDirPath(&appEntry);
    if (addonsDirPath.Append(K_ARK_DIR_NAME) == B_OK)
        m_addonsDir.SetTo(addonsDirPath.Path());

    BPath docsPath(&appEntry);
    if (docsPath.Append(K_DOC_DIR_NAME) == B_OK)
        m_docsDir.SetTo(docsPath.Path());

    BPath settingsPath;
    find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
    m_settingsDir.SetTo(settingsPath.Path());
    // Create our settings directory if it doesn't exist
    if (!m_settingsDir.Contains(K_SETTINGS_DIR_NAME))
        m_settingsDir.CreateDirectory(K_SETTINGS_DIR_NAME, NULL);
    if (settingsPath.Append(K_SETTINGS_DIR_NAME) == B_OK)
        m_settingsDir.SetTo(settingsPath.Path());
    m_settingsPathStr = settingsPath.Path();

    BPath stubDirPath(&appEntry);
    if (stubDirPath.Append(K_STUB_DIR_NAME) == B_OK)
        m_stubDir.SetTo(stubDirPath.Path());
}


void BeezerApp::InitPrefs()
{
    const char* dir = m_settingsPathStr.String();
    _prefs_colors.Init(dir, K_SETTINGS_COLORS);
    _prefs_paths.Init(dir, K_SETTINGS_PATHS);
    _prefs_extract.Init(dir, K_SETTINGS_EXTRACT);
    _prefs_add.Init(dir, K_SETTINGS_ADD);
    _prefs_state.Init(dir, K_SETTINGS_STATE);
    _prefs_windows.Init(dir, K_SETTINGS_WINDOWS);
    _prefs_recent.Init(dir, K_SETTINGS_RECENT);
    _prefs_lang.Init(dir, K_SETTINGS_LANG);
    _prefs_misc.Init(dir, K_SETTINGS_MISC);
    _prefs_recent_archives.Init(dir, K_SETTINGS_RECENT_ARCHIVES);
    _prefs_recent_extract.Init(dir, K_SETTINGS_RECENT_EXTRACT);
    _prefs_interface.Init(dir, K_SETTINGS_INTERFACE);
    _prefs_recent_splitfiles.Init(dir, K_SETTINGS_RECENT_SPLIT_FILES);
    _prefs_recent_splitdirs.Init(dir, K_SETTINGS_RECENT_SPLIT_DIRS);
}


void BeezerApp::CompileTimeString(BString& output)
{
    BString buildStr(__DATE__);
    buildStr << " " << __TIME__;
    time_t t = parsedate(buildStr, -1);

    if (BDateTimeFormat().Format(output, t, B_LONG_DATE_FORMAT, B_MEDIUM_TIME_FORMAT) != B_OK)
        output = buildStr;
}


void BeezerApp::VersionString(BString& output)
{
    entry_ref appRef;
    if (be_roster->FindApp(K_APP_SIGNATURE, &appRef) != B_OK)
    {
        output = "<ROSTER ERROR>";
        return;
    }

    BFile file(&appRef, B_READ_ONLY);
    BAppFileInfo appInfo(&file);
    version_info verInfo;
    if (appInfo.GetVersionInfo(&verInfo, B_APP_VERSION_KIND) != B_OK)
    {
        output = "<ERROR>";
        return;
    }

    output = "";
    output << verInfo.major << "." << verInfo.middle << "." << verInfo.minor << " ";
#ifdef __386__
    output << "(x86 - ";
#elif __x86_64__
    output << "(x86_64 - ";
#else
    output << "(unknown arch - ";
#endif

#ifdef __clang__
    output << "clang)";
#elif __GNUC__
    output << "gcc)"; // probably gcc, others like clang also define this
#else
    output << "unknown compiler)";
#endif
}


void BeezerApp::ShowCreateFilePanel()
{
    CreateFilePanel(m_createFilePanel, B_SAVE_PANEL);
    BWindow* panelWnd = m_createFilePanel->Window();
    panelWnd->SetTitle(B_TRANSLATE("Create archive"));

    if (m_arkTypeField == NULL && panelWnd->LockLooper())
    {
        m_createFilePanel->SetButtonLabel(B_DEFAULT_BUTTON, BZ_TR(kCreateString));

        BView* backView = panelWnd->ChildAt(0L);
        BButton* saveBtn = (BButton*)panelWnd->FindView("default button");
        saveBtn->SetLabel(BZ_TR(kCreateString));
        BTextControl* textField = (BTextControl*)panelWnd->FindView("text view");
        textField->ResizeBy(-20, 0);
        textField->TextView()->DisallowChar('*');

        if (m_arkTypePopUp)
            delete m_arkTypePopUp;

        m_archiverMgr->ArchiversInstalled(m_arkTypes, &m_arkExtensions);
        m_arkTypePopUp = m_archiverMgr->BuildArchiveTypesMenu(this, &m_arkExtensions);
        m_arkTypeField = new BMenuField(BRect(textField->Frame().right + K_MARGIN,
                                              textField->Frame().top - 2, backView->Frame().Width(), 0),
                                        "Beezer:arkTypeField", B_TRANSLATE("Type:"), (BMenu*)m_arkTypePopUp,
                                        B_FOLLOW_BOTTOM, B_WILL_DRAW);
        m_arkTypeField->SetDivider(be_plain_font->StringWidth(B_TRANSLATE("Type:")) + 5);

        if (m_arkTypes.CountItems() > 0)
        {
            // Restore default archiver from prefs or set it to the LAST archiver on the list
            BString arkType;
            BMenuItem* item = NULL;
            status_t wasFound = _prefs_misc.FindString(kPfDefaultArk, &arkType);
            if (wasFound == B_OK)
                item = m_arkTypePopUp->FindItem(arkType.String());

            if (wasFound != B_OK || item == NULL)
                item = m_arkTypePopUp->ItemAt(m_arkTypePopUp->CountItems() - 1);

            item->SetMarked(true);
            textField->SetText((char*)m_arkExtensions.ItemAtFast(m_arkTypePopUp->IndexOf(item)));
        }

        backView->AddChild(m_arkTypeField);

        if (strlen(textField->Text()) == 0)
            saveBtn->SetEnabled(false);

        panelWnd->UnlockLooper();
        m_createFilePanel->SetMessage(new BMessage(M_CREATE_REQUESTED));
        m_createFilePanel->SetTarget(this);
    }

    m_createFilePanel->Show();
}


int8 BeezerApp::RegisterFileTypes() const
{
    const BString fileTypeFieldName = "BEOS:FILE_TYPES";
    app_info appInfo;
    be_app->GetAppInfo(&appInfo);

    BNode appNode(&appInfo.ref);
    BMessage attrMsg;
    attr_info attrInfo;
    appNode.GetAttrInfo(fileTypeFieldName.String(), &attrInfo);

    // This BMallocIO thing was suggested by mmu_man - many thanks to him!
    BMallocIO mio;
    mio.SetSize(attrInfo.size);

    appNode.ReadAttr(fileTypeFieldName.String(), B_MESSAGE_TYPE, 0, (void*)mio.Buffer(), attrInfo.size);
    attrMsg.Unflatten(&mio);

    uint32 type;
    int32 count;
    BString mimeTypeStr;

    attrMsg.GetInfo("types", &type, &count);
    if (type != B_STRING_TYPE)
        return -1;

    bool alreadyAssociated = true;
    bool skipFurtherAlerts = false;
    int8 regCount = 0L;
    for (int32 i = --count; i >= 0; i--)
        if (attrMsg.FindString("types", i, &mimeTypeStr) == B_OK)
        {
            BMimeType mimeType(mimeTypeStr.String());
            char currentPreferredApp[B_MIME_TYPE_LENGTH];

            mimeType.GetPreferredApp(currentPreferredApp, B_OPEN);
            if (strcmp(currentPreferredApp, K_APP_SIGNATURE) != 0 || !mimeType.IsInstalled())
            {
                entry_ref ref;
                be_roster->FindApp(currentPreferredApp, &ref);

                BString buf;
                if (!mimeType.IsInstalled())
                    buf = B_TRANSLATE("Register %mimetype% and make %appname% the preferred app for this type?");
                else
                    buf = B_TRANSLATE("Make %appname% the preferred app for this type?");

                buf.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
                buf.ReplaceAll("%mimetype%", mimeTypeStr.String());

                int32 index = 2L;
                if (skipFurtherAlerts == false)
                {
                    BAlert* confAlert = new BAlert("Mimetypes", buf.String(), BZ_TR(kCancelString),
                                                   B_TRANSLATE("Make it preferred"), B_TRANSLATE("Register all types!"),
                                                   B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
                    confAlert->SetDefaultButton(confAlert->ButtonAt(1L));
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

                    mimeType.SetPreferredApp(K_APP_SIGNATURE, B_OPEN);
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


BMenu* BeezerApp::BuildToolsMenu() const
{
    // We archive and instantiate menu because BMenu doesn't have copy constructor, and
    // neither do BMenuItems
    BMessage toolsMenuMsg;
    m_toolsMenu->Archive(&toolsMenuMsg, true);

    return new BMenu(&toolsMenuMsg);
}


BPopUpMenu* BeezerApp::BuildToolsPopUpMenu() const
{
    // We archive and instantiate menu because BPopUpsMenu doesn't have copy constructor, and
    // neither do BMenuItems
    BMessage toolsMenuMsg;
    m_toolsMenu->Archive(&toolsMenuMsg, true);

    return new BPopUpMenu(&toolsMenuMsg);
}


ArchiverMgr* BeezerApp::GetArchiverMgr()
{
    return m_archiverMgr;
}


int main()
{
    srand(0);

    BeezerApp app;
    app.Run();

    return 0;
}
