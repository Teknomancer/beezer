// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "MainWindow.h"
#include "AppConstants.h"
#include "AppUtils.h"
#include "Archiver.h"
#include "ArchiverMgr.h"
#include "ArkInfoWindow.h"
#include "BarberPole.h"
#include "BeezerApp.h"
#include "BeezerListView.h"
#include "BitmapPool.h"
#include "CLVColumnLabelView.h"
#include "CommentWindow.h"
#include "CommonStrings.h"
#include "FSUtils.h"
#include "HashTable.h"
#include "ImageButton.h"
#include "InfoBar.h"
#include "InputAlert.h"
#include "KeyedMenuItem.h"
#include "ListEntry.h"
#include "LocalUtils.h"
#include "LogTextView.h"
#include "LogWindow.h"
#include "MainMenu.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "ProgressWindow.h"
#include "RecentMgr.h"
#include "RuleMgr.h"
#include "SearchWindow.h"
#include "SelectDirPanel.h"
#include "SplitPane.h"
#include "StatusWindow.h"
#include "ToolBar.h"
#include "UIConstants.h"
#include "WindowMgr.h"

#include <FindDirectory.h>
#include <NodeInfo.h>
#include <Resources.h>
#include <Volume.h>
#include <kernel/fs_attr.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


MainWindow::MainWindow(BRect frame, WindowMgr* windowMgr, RecentMgr* recentMgr,
                       RecentMgr* extractMgr, RuleMgr* ruleMgr)
    : BWindow(frame, B_TRANSLATE("Untitled"), B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS),
      m_searchWnd(NULL),
      m_extractToPanel(NULL),
      m_addPanel(NULL),
      m_archiver(NULL),
      m_logHidden(false),
      m_badArchive(false),
      m_createMode(false),
      m_addStarted(false),
      m_dragExtract(false),
      m_archiversDir(NULL),
      m_tempDir(NULL),
      m_tempDirPath(NULL),
      m_deleteFileList(NULL),
      m_deleteDirList(NULL),
      m_windowMgr(windowMgr),
      m_recentMgr(recentMgr),
      m_extractMgr(extractMgr),
      m_ruleMgr(ruleMgr),
      m_searchSettingsMsg(NULL),
      m_cachedUIState(NULL),
      m_cachedArkState(NULL),
      m_archiveSize(0),
      m_foldingLevel(3),
      m_slotOneX(0.0),
      m_slotTwoX(0.0),
      m_progressWnd(NULL),
      m_statusWnd(NULL),
      m_publicThreadCancel(false),
      m_criticalSection(false),
      m_addPanelMenuBar(NULL)
{
    // Setup the background view and the menu, add to window
    m_backView = new BView(Bounds(), "MainWindow:BackView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    m_mainMenu = new MainMenu(Bounds());
    AddChild(m_mainMenu);
    AddChild(m_backView);

    // Setup toolbars (added to m_backView), toolbar, infobar, CLV and logtextviews and FINALLY splitter
    // The order here is critical!! DO NOT CHANGE THE ORDER!!
    AddToolBar();
    AddInfoBar();
    AddListView();
    AddLogTextView();
    AddSplitterBar();

    // Disable things we need to like toolbar commands etc.
    UpdateFileSelectNeeders(false);
    UpdateSelectNeeders(false);
    UpdateValidArchiveNeeders(false);
    UpdateListItemNeeders(false);

    // Center window on-screen if its the first window
    if (m_windowMgr->CountWindows() == 0)
        CenterOnScreen();

    // Constrain the width and height of the window
    float minH, maxH, minV, maxV;
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    SetSizeLimits(m_toolBar->InnerFrame().right - 2, maxH,
                  m_mainMenu->Frame().Height() + m_toolBar->Frame().Height() + 157, maxV);
}


MainWindow::~MainWindow()
{
    // Do it in the destructor so that when LARGE temp dir exists, the window will appear closed
    // but (Deskbar entry will remain) we will clean up after that
    if (m_tempDir)
    {
        RemoveDirectory(m_tempDir);
        delete m_tempDir;
        free((char*)m_tempDirPath);
    }
}


bool MainWindow::QuitRequested()
{
    if (m_criticalSection)
        return false;

    return BWindow::QuitRequested();
}


void MainWindow::Quit()
{
    // Save interface state to archive if prefs allows it and it actually is an archive
    if (m_archiver && _prefs_state.FindBoolDef(kPfStoreUI, false) == true)
        SaveSettingsToArchive(NULL);
    else if (m_archiver && m_cachedUIState)        // In case the attrs are blown away by add-on after an add/delete
        SaveSettingsToArchive(m_cachedUIState);

    // Save archiver settings if needed
    if (m_archiver && _prefs_state.FindBoolDef(kPfStoreArk, false) == true)
        SaveArchiverToArchive(NULL);
    else if (m_archiver && m_cachedArkState)    // In case the attrs are blown away by add-on after an add/delete
        SaveArchiverToArchive(m_cachedArkState);

    if (m_cachedUIState)
    {
        delete m_cachedUIState;
        m_cachedUIState = NULL;
    }

    if (m_cachedArkState)
    {
        delete m_cachedArkState;
        m_cachedArkState = NULL;
    }

    if (m_progressWnd)
    {
        m_progressWnd->PostMessage(M_STOP_OPERATION);
        if (m_progressWnd->Lock())
            m_progressWnd->Quit();
    }

    m_publicThreadCancel = true;

    if (m_searchWnd)
    {
        m_searchWnd->Quit();
        m_searchWnd = NULL;
    }

    if (m_searchSettingsMsg && _prefs_windows.FindBoolDef(kPfSearchWnd, false))
    {
        m_searchSettingsMsg->RemoveName(kColumnPtr);
        _prefs_windows.SetMessage(kPfSearchWndFrame, m_searchSettingsMsg);
        delete m_searchSettingsMsg;
    }

    if (m_extractToPanel)
        delete m_extractToPanel;

    if (m_addPanel)
        delete m_addPanel;

    if (m_archiver)
        delete m_archiver;

    // Clear these incase -- some erroneous files may cause DeleteUpdate() not to be called,
    // in which case clear from memory -- mostly this shouldn't happen!
    ClearDeleteLists();

    UpdateWindowManager(true);
    be_app_messenger.SendMessage(M_UNREG_WINDOW);

    return BWindow::Quit();
}


void MainWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_ADD_FOLDERS:
        {
            int32 i, totalItems;
            if (message->FindInt32("start_index", &i) != B_OK)
            {
                i = 0L;
                m_archiver->FillLists();
                m_archiver->GetLists(m_fileList, m_dirList);
            }

            i = AddFoldersFromList(i, &totalItems);
            // Bug-fix it should be < not <= -Bug was found when opening BeIDE.zip
            if (i < totalItems)
            {
                message->RemoveName("start_index");
                message->AddInt32("start_index", i);
                PostMessage(message);
            }
            else
                PostMessage(M_ADD_ITEMS);

            break;
        }

        case M_ADD_ITEMS:
        {
            int32 index, totalItems;
            if (message->FindInt32("start_index", &index) != B_OK)
                index = 0L;
            index = AddItemsFromList(index, &totalItems);

            // Check if all items have been added
            if (index < totalItems)
            {
                message->RemoveName("start_index");
                message->AddInt32("start_index", index);
                PostMessage(message);
            }
            else
            {
                // Calculate the total size of the archive
                for (int32 k = 0; k < totalItems; k++)
                {
                    ListEntry* item = ((HashEntry*)m_fileList->ItemAtFast(k))->m_clvItem;
                    if (item)
                        m_archiveSize += item->m_length;
                }

                m_infoBar->UpdateFilesDisplay(0L, totalItems + m_dirList->CountItems(), true);
                m_infoBar->UpdateBytesDisplay(0L, m_archiveSize, true);

                SetBusyState(false);
                PostMessage(M_OPEN_FINISHED);
            }

            break;
        }

        case M_BROADCAST_STATUS:
        {
            // Called from WindowMgr when some other windows modified
            // When this window is being created - be_app will do the updating of Window Manager
            UpdateWindowsMenu();
            break;
        }

        case M_UPDATE_RECENT:
        {
            // Called from WindowMgr (see OpenArchivePartTwo) when some file has been opened
            // and we need to add it to our recent file list or when Preferences window is closed (ie from
            // BeApp object)
            UpdateRecentMenu();

            if (m_archiver)
                UpdateExtractMenu();

            break;
        }

        case M_UPDATE_INTERFACE:
        {
            // Called from beapp object when prefs are closed
            m_toolBar->Redraw();
            m_infoBar->Redraw();
            UpdateListView(true);
            break;
        }

        case M_SWITCH_WINDOW:
        {
            MainWindow* wnd = NULL;
            message->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
            if (wnd)
                wnd->Activate(true);

            break;
        }

        case M_OPEN_REQUESTED:
        {
            // Open file notifications come here from be_app
            if (message->FindRef(kRef, &m_archiveRef) == B_OK)
            {
                m_archivePath.SetTo(&m_archiveRef);

                SetTitle(m_archivePath.Leaf());
                OpenArchive();

                if (m_archiver == NULL)
                {
                    BMessage reply;
                    reply.AddBool(kFailOnNull, true);
                    message->SendReply(&reply);
                    Quit();
                }
                else
                    message->SendReply('repl');
            }

            break;
        }

        case M_OPEN_PART_TWO:
        {
            status_t result;
            message->FindInt32(kResult, &result);
            OpenArchivePartTwo(result);
            PostMessage(M_ADD_FOLDERS);
            break;
        }

        case M_OPEN_FINISHED:
        {
            // Output that an error occurred while reading the archive
            if (m_badArchive == true)
            {
                // Mimetypes are correct but archive is corrupted/buggy
                BMessage* msg = m_archiver->ErrorMessage();
                const char* errString;
                if (msg->FindString(kErrorString, &errString) == B_OK)
                {
                    m_logTextView->AddText(B_TRANSLATE("Error!"), false, false, false);
                    BString alertStr(B_TRANSLATE("The archiver has encountered errors."));
                    alertStr << "\n" << B_TRANSLATE("Do you want to view a detailed report?");
                    BAlert* errAlert = new BAlert("Error", alertStr, B_TRANSLATE("View report"),
                                                  BZ_TR(kCancelString), NULL, B_WIDTH_AS_USUAL,
                                                  B_EVEN_SPACING, B_STOP_ALERT);
                    errAlert->SetDefaultButton(errAlert->ButtonAt(1L));
                    errAlert->SetShortcut(1L, B_ESCAPE);
                    errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                    errAlert->AddToSubset(this);
                    int32 buttonIndex = errAlert->Go();
                    if (buttonIndex == 0L)
                    {
                        BString errWindowTitle = B_TRANSLATE("Errors & warnings:");
                        errWindowTitle << " " << Title();
                        new LogWindow(this, errWindowTitle.String(), errString);
                    }
                }
            }
            else
                m_logTextView->AddText(B_TRANSLATE("Done."), false, false, false);

            if (m_archiver->SupportsPassword() && m_archiver->PasswordRequired())
                m_logTextView->AddText(B_TRANSLATE("Password protected file(s) found."), true, false, false);

            ActivateUIForValidArchive();
            break;
        }

        case M_CREATE_REQUESTED:
        {
            entry_ref dirRef;
            const char* fileName(NULL);
            message->FindRef("directory", &dirRef);
            message->FindString("name", &fileName);
            message->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&m_archiver));

            if (!m_archiver)
                break;

            m_createMode = true;
            InitArchiver();

            // Get filename with the extension, if user entered "a" as the filename we
            // always append the extension because if we don't and the ZipArchiver's binary (zip eg)
            // does it we will end up with a different name than the one we have
            BString correctedFileName = fileName;

            // Don't correct filenames here as we already correct in-front of the user in the
            // create file panel in beezer - if he has adjusted the filename after that maybe he wants it
            // that way -- keep this commented and finally verdict later
            //const char *extension = m_archiver->ArchiveExtension();
            //int32 foundIndex = correctedFileName.FindLast (extension);
            //if (foundIndex < 0L || foundIndex != (int32)(correctedFileName.Length() - strlen (extension)))
            //    correctedFileName.Append (extension);

            BPath path(&dirRef);
            path.Append(correctedFileName.String(), false);

            m_archivePath = path;
            m_archiveEntry.SetTo(m_archivePath.Path(), true);
            m_archiveEntry.GetRef(&m_archiveRef);
            message->what = M_ACTIONS_ADD;
            PostMessage(message);

            break;
        }

        case M_ACTIONS_CREATE_FOLDER:
        {
            AddNewFolder();
            break;
        }

        case M_ACTIONS_TEST:
        {
            TestArchive();
            break;
        }

        case M_TEST_DONE:
        {
            TestDone(message);
            break;
        }

        case M_DELETE_DONE:
        {
            DeleteDone(message);
            break;
        }

        case M_TOGGLE_TOOLBAR:
        {
            // User chose this menu item to show/hide the toolbar
            ToggleToolBar();
            break;
        }

        case M_TOGGLE_INFOBAR:
        {
            // User chose this menu item to show/hide the infobar
            ToggleInfoBar();
            break;
        }

        case M_TOGGLE_LOG:
        {
            ToggleActionLog();
            break;
        }

        case M_TOOLBAR_TOGGLED: case M_INFOBAR_TOGGLED:
        {
            // User has hidden/unhidden the toolbar/infobar object which has sent us this message. Act
            // accordingly to move/resize other controls on the window
            bool isHidden = message->FindBool(kHidden);
            float height = message->FindFloat(kBarHeight);
            if (isHidden == true)
                height *= -1;

            if (message->what == M_TOOLBAR_TOGGLED)
                m_infoBar->MoveBy(0, height);

            m_splitPane->MoveBy(0, height);
            m_splitPane->ResizeBy(0, -height);

            // If the log is hidden, keep it that way, Otherwise it will pop-back up
            if (m_logHidden)
            {
                m_splitPane->SetBarLocked(false);
                m_splitPane->SetBarPosition(BPoint(Bounds().Width(), Bounds().Height()));
                m_splitPane->SetBarLocked(true);
            }

            if (message->what == M_TOOLBAR_TOGGLED)
                m_mainMenu->m_viewMenu->FindItem(M_TOGGLE_TOOLBAR)->SetMarked(!isHidden);
            else
                m_mainMenu->m_viewMenu->FindItem(M_TOGGLE_INFOBAR)->SetMarked(!isHidden);

            break;
        }

        case M_SELECTION_ADDED:
        {
            // For performance reason we get this message - we avoid calling
            // UpdateInfoBar() which RECOUNTS all selected items and readds the selected bytes
            // This will simply add to the infobar's already existing selection information
            // This is sent by BeezerListView::KeyDown() implementation when we get this we
            // will NOT get M_SELECTION_CHANGED message
            ListEntry* item;
            message->FindPointer(kListItem, reinterpret_cast<void**>(&item));

            if (item)
            {
                UpdateUIAsPerSelection();
                m_infoBar->UpdateBy(1, item->m_length);
            }

            break;
        }

        case M_SELECTION_CHANGED:
        {
            // Check if ONLY directories has been selected, if so don't enable
            int32 count;
            int64 bytes;
            message->FindInt32(kCount, &count);
            message->FindInt64(kBytes, &bytes);
            UpdateUIAsPerSelection();
            UpdateInfoBar(count, bytes);
            break;
        }

        case M_ENTER: case M_ACTIONS_VIEW: case M_ACTIONS_OPEN_WITH:
        {
            bool isSuper;
            if (message->FindBool(kSuperItem, &isSuper) != B_OK)
                isSuper = false;

            if (isSuper == true && message->what == M_ENTER)
                break;

            ListEntry* selEntry(NULL);
            BMessage* viewMsg = new BMessage(message->what != M_ENTER ? message->what : (int32)M_ACTIONS_VIEW);
            int32 i = 0L;
            int32 fileCount = 0L;
            while ((selEntry =
                    (ListEntry*)m_listView->FullListItemAt(m_listView->FullListCurrentSelection(i))) != NULL)
            {
                if (!selEntry->IsSuperItem())
                {
                    viewMsg->AddString(kPath, selEntry->m_fullPath.String());
                    fileCount++;
                }
                i++;
            }

            if (fileCount > 10)
            {
                BString confirmStr(B_TRANSLATE("You are opening %numfiles% files simultenously."));
                confirmStr << "\n" << B_TRANSLATE("Do you want to continue?");
                BString countBuf;
                countBuf.SetToFormat("%d", fileCount);
                confirmStr.ReplaceAll("%numfiles%", countBuf);
                BAlert* confirmAlert = new BAlert("Confirm", confirmStr, B_TRANSLATE("Open files"),
                                                  BZ_TR(kCancelString), NULL, B_WIDTH_AS_USUAL,
                                                  B_EVEN_SPACING, B_WARNING_ALERT);
                confirmAlert->SetShortcut(1, B_ESCAPE);
                confirmAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                confirmAlert->AddToSubset(this);
                delete[] confirmStr;

                int32 ans = confirmAlert->Go();
                if (ans == 1)
                    break;
            }

            // Create temp directory if needed
            MakeTempDirectory();
            BEntry tempDirEntry(m_tempDirPath, false);
            entry_ref tempDirRef;
            tempDirEntry.GetRef(&tempDirRef);

            m_statusWnd = new StatusWindow(B_TRANSLATE("Extracting" B_UTF8_ELLIPSIS), this, B_TRANSLATE("Please wait" B_UTF8_ELLIPSIS), &m_publicThreadCancel);

            viewMsg->AddPointer(kWindowPtr, (void*)this);
            viewMsg->AddPointer(kArchiverPtr, (void*)m_archiver);
            viewMsg->AddPointer(kCancel, (void*)(&m_publicThreadCancel));
            viewMsg->AddPointer(kStatusPtr, (void*)m_statusWnd);
            viewMsg->AddString(kTempPath, m_tempDirPath);
            viewMsg->AddRef(kRef, &tempDirRef);

            // Make extraction (viewing) on separate thread as it will not freeze the interface
            // while extracting large files
            resume_thread(spawn_thread(_viewer, "_viewer", B_NORMAL_PRIORITY, (void*)viewMsg));

            // Also this thread that is NOT controlled by StatusWindow is controlled by
            // m_publicThreadCancel -- this way when the window quits it will set that cancel flag to true
            // thus quitting this thread as well

            break;
        }

        case M_ACTIONS_EXTRACT:
        {
            // Get the extract path from prefs, if none call M_ACTIONS_EXTRACT_TO
            BString extractDirPath;
            bool arkDir;
            if (_prefs_paths.FindBool(kPfUseArkDir, &arkDir) == B_OK)
            {
                if (arkDir == true)
                {
                    BPath arkDirPath;
                    m_archivePath.GetParent(&arkDirPath);
                    extractDirPath = arkDirPath.Path();
                }
                else
                    _prefs_paths.FindString(kPfDefExtractPath, &extractDirPath);
            }

            if (extractDirPath.Length() > 0 && IsExtractPathValid(extractDirPath.String(), true) == false)
                break;

            entry_ref ref;
            get_ref_for_path(extractDirPath.String(), &ref);

            BMessage extractMsg(M_EXTRACT_TO);
            extractMsg.AddRef("refs", &ref);
            extractMsg.AddBool(kDynamic, true);     // Prevent this path from being added to recent paths

            if (extractDirPath.Length() == 0)
                PostMessage(M_ACTIONS_EXTRACT_TO);
            else
                PostMessage(&extractMsg);

            break;
        }

        case M_ACTIONS_EXTRACT_TO: case M_ACTIONS_EXTRACT_SELECTED:
        {
            SetupExtractPanel(message);
            m_extractToPanel->Show();
            break;
        }

        case M_RECENT_EXTRACT_ITEM:
        {
            BMenuItem* source(NULL);
            message->FindPointer("source", reinterpret_cast<void**>(&source));
            bool allFiles = true;
            BMenu* sourceMenu = source->Menu();
            if (sourceMenu && sourceMenu->Superitem() && strcmp(sourceMenu->Superitem()->Label(),
                    BZ_TR(kExtractSelectedString)) == 0)
            {
                allFiles = false;
            }

            if (IsExtractPathValid(source->Label(), true) == false)
                break;

            entry_ref ref;
            get_ref_for_path(source->Label(), &ref);

            BMessage extractMsg(allFiles ? M_EXTRACT_TO : M_EXTRACT_SELECTED_TO);
            extractMsg.AddRef("refs", &ref);
            if (message->HasBool(kDynamic))
                extractMsg.AddBool(kDynamic, true);     // This prevents adding this to recent extract paths
            PostMessage(&extractMsg);

            break;
        }

        case M_EXTRACT_TO: case M_EXTRACT_SELECTED_TO: case M_DROP_MESSAGE:
        {
            entry_ref ref;
            status_t found;
            bool allFiles;

            // If its a drag 'n dropped message callback, then Tracker will return the dropped
            // destination directory ref in "directory" & we would have added a "kFieldFull" to see if all
            // files where dropped or not
            if (message->what == M_DROP_MESSAGE)
            {
                found = message->FindRef("directory", &ref);
                if (message->FindBool(kFieldFull, &allFiles) != B_OK)
                    allFiles = false;

                m_dragExtract = true;
            }
            else
            {
                found = message->FindRef("refs", &ref);

                if (m_extractToPanel && m_extractToPanel->IsShowing())
                    m_extractToPanel->Hide();

                allFiles = message->what == M_EXTRACT_TO ? true : false;

                if (message->HasBool(kDynamic) == false)
                {
                    BPath extractPath(&ref);
                    m_extractMgr->AddPath(extractPath.Path());
                    UpdateExtractMenu();
                    m_windowMgr->UpdateFrom(this, new BMessage(M_UPDATE_RECENT), true);
                }
            }

            if (found == B_OK)
                ExtractArchive(&ref, allFiles);

            break;
        }

        case M_EXTRACT_DONE:
        {
            ExtractDone(message);
            break;
        }

        case M_ACTIONS_ADD:
        {
            if (CanAddFiles() == false && m_createMode == false)
                break;

            if (CanWriteArchive() == false)
                break;

            // NOTE: do NOT add check here to see if the archive exists or not because it need not
            // (eg: while creating archives)    -- bugfix 0.05

            SetupAddPanel();
            m_addStarted = false;
            m_addPanel->Show();
            break;
        }

        case B_CANCEL:
        {
            int32 oldWhat;
            message->FindInt32("old_what", &oldWhat);
            if ((uint32)oldWhat == M_ADD && m_createMode == true && m_addStarted == false)
                PostMessage(M_FILE_CLOSE);
            break;
        }

        case M_ADD:
        {
            m_addStarted = true;
            if (CanWriteArchive() == false)        // We need to check here too: because dropped files come here
                break;                         // and NOT through "M_ACTIONS_ADD", hence this check again.

            if (CanAddFiles() == false && m_createMode == false)    // Same here
                break;

            // NOTE: do NOT add check here to see if the archive exists or not because it need not
            // (eg: while creating archives)    -- bugfix 0.05

            if (m_addPanel)    // If we get from a drag-drop operation (there would not be a filepanel)
            {
                BTextControl* pwdText = (BTextControl*)m_addPanel->Window()->FindView("pwdText");
                if (m_archiver->SupportsPassword() && pwdText)
                    m_archiver->SetPassword(pwdText->Text());
            }

            // While this thread counts the directories and files sizes in message, we setup a blocker-status
            // window so that it shows barber pole PLUS blocks our MainWindow preventing the user from
            // doing other operations (2-in-1) ! Plus user can cancel the operation - (3-in-1) !! Yay!
            m_statusWnd = new StatusWindow(B_TRANSLATE("Preparing to add" B_UTF8_ELLIPSIS), this, B_TRANSLATE("Gathering information"),
                                           &m_publicThreadCancel);

            BMessage* countMsg = new BMessage(*message);
            countMsg->AddPointer(kCancel, (void*)(&m_publicThreadCancel));
            countMsg->AddPointer(kStatusPtr, (void*)m_statusWnd);
            countMsg->AddPointer(kWindowPtr, (void*)this);

            resume_thread(spawn_thread(_counter, "_counter", B_NORMAL_PRIORITY, (void*)countMsg));
            break;
        }

        case M_COUNT_COMPLETE:
        {
            // Now we really get down to business...
            int32 fileCount(0);
            if (message->FindInt32(kFiles, &fileCount) != B_OK)
                break;

            // If drag and drop must be confirmed -- do it
            if ((message->HasBool(kRoot) || message->HasPointer(kListItem)) &&
                    _prefs_add.FindBoolDef(kPfConfirmDropAdd, true))
            {
                BAlert* confirmAlert = new BAlert("confirm", B_TRANSLATE("Continue adding the dropped files to the archive?"),
                                                  B_TRANSLATE("Add files"), BZ_TR(kCancelString),
                                                  NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
                confirmAlert->SetShortcut(1, B_ESCAPE);
                confirmAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                confirmAlert->AddToSubset(this);
                int32 ans = confirmAlert->Go();
                if (ans == 1)
                    break;
            }

            off_t totalSize(0);
            if (message->FindInt64(kSize, &totalSize) == B_OK && _prefs_add.FindBoolDef(kPfWarnBeforeAdd, true))
            {
                // if more than "n" MB is being added, warn, get "n" from prefs or default to 100 MB
                if (totalSize > _prefs_add.FindInt16Def(kPfWarnAmount, 100) * 1024 * 1024)
                {
                    BString confirmStr(B_TRANSLATE("Adding %size% of data could take some time."));
                    confirmStr << "\n" << B_TRANSLATE("Do you want to continue?");
                    confirmStr.ReplaceAll("%size%", StringFromBytes(totalSize).String());
                    BAlert* confirmAlert = new BAlert("confirm", confirmStr, B_TRANSLATE("Add files"),
                                                      BZ_TR(kCancelString), NULL, B_WIDTH_AS_USUAL,
                                                      B_EVEN_SPACING, B_WARNING_ALERT);
                    confirmAlert->SetShortcut(1, B_ESCAPE);
                    confirmAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                    confirmAlert->AddToSubset(this);

                    int32 ans = confirmAlert->Go();
                    if (ans == 1)
                    {
                        if (m_createMode == true)
                            PostMessage(M_FILE_CLOSE);

                        break;
                    }
                }
            }

            ListEntry* selectedItem = NULL;
            // kListItem, will be added by BeezerListView's drag-drop implementation hence we must
            // not take into account the selected item, rather take the item passed to us
            if (message->FindPointer(kListItem, reinterpret_cast<void**>(&selectedItem)) != B_OK)
            {
                int32 i = m_listView->CurrentSelection(0L);
                if (i >= 0L)
                    selectedItem = (ListEntry*)m_listView->ItemAt(i);
            }

            // Drag and drop (with SHIFT held down) will have a Bool field saying forcefully add at root
            if (message->HasBool(kRoot) == true)
                selectedItem = NULL;

            // Add common fields applicable to both "adding at root" or "adding at sub-dir or archive"
            BMessage* copyMsg = new BMessage(*message);
            copyMsg->AddInt32(kCount, (int32)fileCount);

            bool addingAtRoot = false;
            if (!selectedItem)
                addingAtRoot = true;
            else if (selectedItem->IsSuperItem() == false && selectedItem->OutlineLevel() == 0L)
                addingAtRoot = true;

            if (addingAtRoot == false)
            {
                // If a folder item is selected, add files inside it otherwise add files inside
                // the directory of the selected file item
                const char* itemPath = selectedItem->m_dirPath.String();
                if (selectedItem->IsSuperItem() == true)
                    itemPath = selectedItem->m_fullPath.String();

                int32 count = 0L, skipped = 0L;
                if (ConfirmAddOperation(itemPath, copyMsg, &count, &skipped) == false
                        || count == skipped)
                {
                    delete copyMsg;
                    break;
                }

                copyMsg->what = M_ACTIONS_ADD;
                MakeTempDirectory();

                BString copyToDir = m_tempDirPath;
                copyToDir << "/" << itemPath;
                create_directory(copyToDir.String(), 0777);

                BMessenger* messenger(NULL);
                volatile bool* cancel;

                copyMsg->AddString(kTempPath, copyToDir.String());
                copyMsg->AddString(kLaunchDir, m_tempDirPath);
                copyMsg->AddString(kProgressAction, B_TRANSLATE("Copying" B_UTF8_ELLIPSIS));
                copyMsg->AddString(kPreparing, B_TRANSLATE("Preparing to add" B_UTF8_ELLIPSIS));
                copyMsg->AddPointer(kSuperItem, (void*)selectedItem);         // Will be used in M_ADD_DONE
                copyMsg->AddString(kSuperItemPath, itemPath);                // Will be used in _copier

                m_progressWnd = new ProgressWindow(this, copyMsg, messenger, cancel);

                copyMsg->AddMessenger(kProgressMessenger, *messenger);
                copyMsg->AddPointer(kCancel, (void*)cancel);
                copyMsg->AddPointer(kWindowPtr, (void*)this);

                m_logTextView->AddText(B_TRANSLATE("Preparing to add" B_UTF8_ELLIPSIS), true, false, false);
                m_logTextView->AddText(" ", false, false, false);
                resume_thread(spawn_thread(_copier, "_copier", B_NORMAL_PRIORITY, (void*)copyMsg));
            }
            else           // user is adding at the root of the archive so DON'T copy files to temp
            {
                if (m_createMode == false)
                {
                    int32 count = 0L, skipped = 0L;
                    if (ConfirmAddOperation(NULL, copyMsg, &count, &skipped) == false || count == skipped)
                    {
                        delete copyMsg;
                        break;
                    }
                }

                copyMsg->what = M_READY_TO_ADD;
                copyMsg->AddInt32(kResult, M_SKIPPED);         // Tell READY_TO_ADD we skipped the copying of files

                // Either get refs from FilePanel if available, or incase of drag and drop (where no
                // file panel would have been created/used get from kDirectoryRef field)
                entry_ref tmpRef;
                if (message->FindRef(kDirectoryRef, &tmpRef) != B_OK)
                    m_addPanel->GetPanelDirectory(&tmpRef);            // Get directory of "refs"

                BPath launchPath(&tmpRef);
                copyMsg->AddString(kLaunchDir, launchPath.Path());         // add launching dir

                uint32 type;
                int32 count;
                copyMsg->GetInfo("refs", &type, &count);
                for (int32 i = --count; i >= 0; i--)
                    if (copyMsg->FindRef("refs", i, &tmpRef) == B_OK)
                        copyMsg->AddString(kPath, tmpRef.name);

                PostMessage(copyMsg);
            }

            break;
        }

        case M_READY_TO_ADD:
        {
            // We must NOT delete "message" here as it was used through PostMessage, I thought it might
            // need to be deleted but PostMessage (new BMessage) should delete it. In effect that is what
            // we are doing (look at _copier thread func -- it uses PostMessage to the new'ly created message
            // (copyMsg) we did in case M_ADD.
            status_t result;
            message->FindInt32(kResult, &result);
            if (result == BZR_CANCEL)
            {
                m_logTextView->AddText(B_TRANSLATE("Cancelled"), false, false, false);
                break;
            }

            if (result != (int32)M_SKIPPED)
                m_logTextView->AddText(B_TRANSLATE("Done"), false, false, false);

            BMessage* addMsg = new BMessage(*message);
            BMessenger* messenger(NULL);
            volatile bool* cancel;

            addMsg->what = M_ACTIONS_ADD;
            addMsg->RemoveName(kProgressAction);
            addMsg->AddString(kProgressAction, B_TRANSLATE("Adding" B_UTF8_ELLIPSIS));
            addMsg->AddString(kPreparing, B_TRANSLATE("Preparing to add" B_UTF8_ELLIPSIS));

            m_progressWnd = new ProgressWindow(this, addMsg, messenger, cancel);

            addMsg->RemoveName(kArchiverPtr);
            addMsg->AddPointer(kArchiverPtr, (void*)m_archiver);
            addMsg->AddMessenger(kProgressMessenger, *messenger);
            addMsg->AddPointer(kCancel, (void*)cancel);
            addMsg->AddPointer(kWindowPtr, (void*)this);
            addMsg->AddBool(kCreateMode, m_createMode);

            if (m_createMode == true)
                m_archiveEntry.Remove();    // Overwrite existing file if any

            SetTitle(m_archivePath.Leaf());
            m_logTextView->AddText(B_TRANSLATE("Adding" B_UTF8_ELLIPSIS), true, false, false);
            m_logTextView->AddText(" ", false, false, false);
            resume_thread(spawn_thread(_adder, "_adder", B_NORMAL_PRIORITY, (void*)addMsg));
            break;
        }

        case M_ADD_DONE:
        {
            int32 result;
            message->FindInt32(kResult, &result);

            if (result == BZR_CANCEL_ARCHIVER)
            {
                BString alertStr(B_TRANSLATE("Cannot continue."));
                alertStr << "\n" << B_TRANSLATE("A critical operation has been cancelled and the archive is in an unknown state.");
                BAlert* errAlert = new BAlert("Error", alertStr, BZ_TR(kCloseWindowString),
                                              NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
                errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                errAlert->AddToSubset(this);
                errAlert->Go();
                PostMessage(M_FILE_CLOSE);
                break;
            }
            else if (result == BZR_NOT_SUPPORTED)
            {
                ShowOpNotSupported();
                break;
            }
            else if (result != BZR_DONE)
            {
                m_logTextView->AddText(B_TRANSLATE("Error!"), false, false, false);
                if (m_archiver->CanPartiallyOpen() == false)
                    break;
            }

            BMessage addedFiles;
            message->FindMessage(kFileList, &addedFiles);
            SetBusyState(true);
            m_archiver->Open(&m_archiveRef, &addedFiles);
            UpdateIfNeeded();
            EmptyListViewIfNeeded();
            PostMessage(M_ADD_FOLDERS_LIST);

            break;
        }

        case M_ADD_FOLDERS_LIST:
        {
            int32 i;
            if (message->FindInt32("start_index", &i) != B_OK)
            {
                i = 0L;
                m_archiver->FillLists(&m_addedFileList, &m_addedDirList);
                if (m_createMode == true)
                    m_archiver->GetLists(m_fileList, m_dirList);
            }

            i = AddFoldersFromList(&m_addedDirList, i);
            if (i < m_addedDirList.CountItems())
            {
                message->RemoveName("start_index");
                message->AddInt32("start_index", i);
                PostMessage(message);
            }
            else
                PostMessage(M_ADD_ITEMS_LIST);

            break;
        }

        case M_ADD_ITEMS_LIST:
        {
            int32 index;
            if (message->FindInt32("start_index", &index) != B_OK)
                index = 0L;

            index = AddItemsFromList(&m_addedFileList, index);
            // Check if all items have been added
            if (index < m_addedFileList.CountItems())
            {
                message->RemoveName("start_index");
                message->AddInt32("start_index", index);
                PostMessage(message);
            }
            else
            {
                // Calculate the total size of the archive
                for (int32 k = 0; k < m_addedFileList.CountItems(); k++)
                {
                    ListEntry* item = (ListEntry*)m_addedFileList.ItemAtFast(k);
                    if (item)
                        m_archiveSize += item->m_length;
                }

                m_addedFileList.MakeEmpty();
                m_addedDirList.MakeEmpty();

                m_infoBar->UpdateFilesDisplay(0L, m_fileList->CountItems() + m_dirList->CountItems(), true);
                m_infoBar->UpdateBytesDisplay(0L, m_archiveSize, true);

                SetBusyState(false);
                m_logTextView->AddText(B_TRANSLATE("Done."), false, false, false);

                if (m_createMode == true)
                {
                    ActivateUIForValidArchive();
                    update_mime_info(m_archivePath.Path(), false, false, false);

                    // Now update the window manager and recent file manager, now that creation is over
                    UpdateNewWindow();

                    m_createMode = false;
                    if (m_addPanelMenuBar && m_archiver->SettingsMenu())
                    {
                        m_addPanelMenuBar->RemoveItem(m_archiver->SettingsMenu());
                        AddArchiverMenu();
                    }
                }

                if (_prefs_add.FindBoolDef(kPfSortAfterAdd, true) == true)
                    m_listView->SortItems();
            }

            break;
        }

        case M_ACTIONS_DELETE:
        {
            DeleteFilesFromArchive();
            break;
        }

        case M_FILE_NEW: case M_FILE_OPEN: case M_EDIT_PREFERENCES:
        case M_HELP_MANUAL: case M_HELP_WEBSITE: case M_HELP_GITHUB:
        case B_ABOUT_REQUESTED:
        {
            be_app_messenger.SendMessage(message);
            break;
        }

        case M_TOOLS_FILE_SPLITTER: case M_TOOLS_FILE_JOINER:
        {
            message->AddRef("refs", &m_archiveRef);
            be_app_messenger.SendMessage(message);
            break;
        }

        case M_FILE_QUIT:
        {
            be_app_messenger.SendMessage(B_QUIT_REQUESTED);
            break;
        }

        case M_FILE_CLOSE:
        {
            // According to BeBook its ok to call Quit() from message loop as it shuts down the message
            // loop (and deletes any pending messages, so this will be the last message to be processed
            Quit();
            break;
        }

        case M_ACTIONS_SEARCH_ARCHIVE:
        {
            if (m_searchWnd == NULL)
            {
                if (m_searchSettingsMsg == NULL && _prefs_windows.FindBoolDef(kPfSearchWnd, false))
                {
                    m_searchSettingsMsg = new BMessage;
                    _prefs_windows.FindMessage(kPfSearchWndFrame, m_searchSettingsMsg);
                }

                m_searchWnd = new SearchWindow(this, m_searchSettingsMsg, &m_archiveEntry, m_columnList, m_archiver);
                m_searchWnd->Show();
            }
            else
                //TODO recenter on active window?
                m_searchWnd->Activate();

            break;
        }

        case M_SEARCH_CLOSED:
        {
            // Cache the search settings so we can pass it to the SearchWindow next time
            if (m_searchSettingsMsg)
                delete m_searchSettingsMsg;

            m_searchSettingsMsg = new BMessage(*message);
            m_searchWnd = NULL;
            break;
        }

        case M_SEARCH:
        {
            // Cache the search settings so we can pass it to the SearchWindow the next time
            if (m_searchSettingsMsg)
                delete m_searchSettingsMsg;

            m_searchSettingsMsg = new BMessage(*message);

            bool persistent;
            message->FindBool(kPersistent, &persistent);

            // Add logtext view descriptive text
            BString searchText = B_TRANSLATE("Searching for \"%searchstring%\"" B_UTF8_ELLIPSIS);
            searchText << " ";
            searchText.ReplaceFirst("%searchstring%", message->FindString(kExpr));
            m_logTextView->AddText(searchText.String(), true, true, false);

            const char* errorString = NULL;
            int32 foundCount = m_listView->Search(message, errorString);

            if (!persistent && m_searchWnd->Lock())
            {
                m_searchWnd->Quit();
                m_searchWnd = NULL;
            }

            // Add logtext number of entries
            if (foundCount >= 0)
            {
                BString numberOfEntries;
                if (foundCount == 1)
                    numberOfEntries = B_TRANSLATE("Found 1 entry.");
                else {
                    numberOfEntries = B_TRANSLATE("Found %number% entries.");
                    BString countBuf;
                    countBuf.SetToFormat("%d", foundCount);
                    numberOfEntries.ReplaceFirst("%number%", countBuf);
                }

                numberOfEntries << " ";

                m_logTextView->AddText(numberOfEntries.String(), false, false, false);
            }

            m_logTextView->AddText(B_TRANSLATE("Done."), false, false, false);

            if (foundCount == -1)
            {
                // We need to do this little hack (ugly) but without Hide() our BAlert() don't work!
                if (persistent)
                {
                    m_searchWnd->LockLooper();
                    m_searchWnd->Hide();
                    m_searchWnd->UnlockLooper();
                }

                BString errorStr = B_TRANSLATE("Error in regular expression:");
                errorStr << "\n\n'" << errorString << "'";
                (new BAlert("", errorStr.String(), BZ_TR(kOKString), NULL, NULL,
                            B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();

                free((char*)errorString);
                PostMessage(M_ACTIONS_SEARCH_ARCHIVE);
            }

            break;
        }

        case M_ACTIONS_COMMENT:
        {
            bool failIfNoComment;
            if (message->FindBool(kFailOnNull, &failIfNoComment) != B_OK)
                EditComment(false);
            else
            {
                EditComment(failIfNoComment);
                if (message->IsSourceWaiting())
                    message->SendReply('repl');
            }

            break;
        }

        case M_SAVE_COMMENT:
        {
            if (m_archiveEntry.Exists() == false)
            {
                ShowArkPathError();
                break;
            }

            if (CanWriteArchive() == false)
                break;

            const char* comment = NULL;
            message->FindString(kCommentContent, &comment);

            m_statusWnd = new StatusWindow(B_TRANSLATE("Saving comment" B_UTF8_ELLIPSIS), this, B_TRANSLATE("Please wait" B_UTF8_ELLIPSIS), NULL);
            m_archiver->SetComment(const_cast<char*>(comment), MakeTempDirectory());
            update_mime_info(m_archivePath.Path(), false, true, true);
            m_statusWnd->PostMessage(M_CLOSE);

            break;
        }

        case M_LOG_CONTEXT_COPY:
        {
            m_logTextView->Copy();
            break;
        }

        case M_EDIT_COPY:
        {
            if (m_logTextView->IsFocus() == true)
                PostMessage(M_LOG_CONTEXT_COPY);
            else
                PostMessage(M_CONTEXT_COPY);

            break;
        }

        case M_GOT_FOCUS:
        {
            // Handle focus changed and selection
            if (m_listView->IsFocus() == true)
            {
                if (m_listView->FullListCurrentSelection() >= 0L)
                    UpdateFocusNeeders(true);
                else
                    UpdateFocusNeeders(false);
            }
            else if (m_logTextView->IsFocus() == true)    // Because the menu can have focus too
                UpdateFocusNeeders(true);

            break;
        }

        case M_CONTEXT_COPY:
        {
            m_listView->CopyToClipboard(';');
            break;
        }

        case M_FILE_ARCHIVE_INFO:
        {
            new ArkInfoWindow(this, m_archiver, &m_archiveEntry);
            break;
        }

        // We call SendSelectionMessage (true) because KeyDown of BeezerListView might have turned it off
        case M_EDIT_SELECT_ALL:    m_listView->SendSelectionMessage(true); m_listView->SelectAll(); break;
        case M_EDIT_SELECT_ALL_DIRS: m_listView->SendSelectionMessage(true); m_listView->SelectAllEx(true); break;
        case M_EDIT_SELECT_ALL_FILES: m_listView->SendSelectionMessage(true); m_listView->SelectAllEx(false); break;
        case M_EDIT_DESELECT_ALL: m_listView->SendSelectionMessage(true); m_listView->DeselectAll(); break;
        case M_EDIT_INVERT_SELECTION: m_listView->SendSelectionMessage(true); m_listView->InvertSelection(); break;

        case M_EDIT_EXPAND_ALL: case M_EDIT_COLLAPSE_ALL:
        {
            m_listView->ToggleAllSuperItems(message->what == M_EDIT_EXPAND_ALL ? true : false);
            break;
        }

        case M_EDIT_EXPAND_SELECTED: case M_EDIT_COLLAPSE_SELECTED:
        {
            m_listView->ToggleSelectedSuperItems(message->what == M_EDIT_EXPAND_SELECTED ? true : false);
            break;
        }

        case M_CONTEXT_SELECT:
        {
            m_listView->SelectSubItemsOfSelection(true);
            break;
        }

        case M_CONTEXT_DESELECT:
        {
            m_listView->SelectSubItemsOfSelection(false);
            break;
        }

        case BZR_MENUITEM_SELECTED:
        {
            // An archiver (add-on)'s menu item has been toggled (we should toggle it to be precise)
            // Get the source and toggle it
            BMenuItem* source_item = NULL;
            message->FindPointer("source", reinterpret_cast<void**>(&source_item));
            if (source_item)
                source_item->SetMarked(!(source_item->IsMarked()));

            break;
        }

        case M_SAVE_AS_DEFAULT:
        {
            BAlert* alert = new BAlert(B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE),
                                       B_TRANSLATE("Save interface and folding settings as defaults?"),
                                       BZ_TR(kCancelString), BZ_TR(kSaveString),
                                       NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
            alert->SetDefaultButton(alert->ButtonAt(1));
            alert->SetShortcut(0, B_ESCAPE);
            if (alert->Go() == 1)
                SaveSettingsAsDefaults();

            break;
        }

        case M_SAVE_TO_ARCHIVE:
        {
            BAlert* alert = new BAlert(B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE),
                                       B_TRANSLATE("Save interface and folding settings to the archive?"),
                                       BZ_TR(kCancelString), BZ_TR(kSaveString),
                                       NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
            alert->SetDefaultButton(alert->ButtonAt(1));
            alert->SetShortcut(0, B_ESCAPE);
            alert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
            alert->AddToSubset(this);
            if (alert->Go() == 1)
                SaveSettingsToArchive(NULL);

            break;
        }

        case M_SAVE_ARK_AS_DEFAULT:
        {
        	BString confirmStr(B_TRANSLATE("Save %archivetype% settings as defaults for %archivetype% files?"));
            confirmStr.ReplaceAll("%archivetype%", m_archiver->ArchiveType());
            BAlert* alert = new BAlert(B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE), confirmStr, BZ_TR(kCancelString),
                                       BZ_TR(kSaveString), NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
            alert->SetDefaultButton(alert->ButtonAt(1));
            alert->SetShortcut(0, B_ESCAPE);
            if (alert->Go() == 1)
                m_archiver->SaveSettingsMenu();

            break;
        }

        case M_SAVE_ARK_TO_ARCHIVE:
        {
            BString confirmStr(B_TRANSLATE("Save %archivetype% settings to the archive?"));
            confirmStr.ReplaceAll("%archivetype%", m_archiver->ArchiveType());
            BAlert* alert = new BAlert(B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE), confirmStr, BZ_TR(kCancelString),
                                       BZ_TR(kSaveString), NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
            alert->SetDefaultButton(alert->ButtonAt(1));
            alert->SetShortcut(0, B_ESCAPE);
            if (alert->Go() == 1)
                SaveArchiverToArchive(NULL);

            break;
        }

        case M_RESET_ARK_TO_DEFAULT:
        {
            KeyedMenuItem::ResetMenu(m_archiver->SettingsMenu());
            m_archiver->SetCompressionLevel(m_archiver->GetDefaultCompressionLevel());
            break;
        }

        case M_RESET_TO_DEFAULT:
        {
            KeyedMenuItem::ResetMenu(m_mainMenu->m_viewMenu);
            BMenu* colMenu = m_mainMenu->m_columnsSubMenu;
            m_sizeColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_SIZE)->IsMarked());
            m_packedColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_PACKED)->IsMarked());
            m_ratioColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_RATIO)->IsMarked());
            m_pathColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_PATH)->IsMarked());
            m_dateColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_DATE)->IsMarked());
            m_methodColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_METHOD)->IsMarked());
            m_crcColumn->SetShown(colMenu->FindItem(M_TOGGLE_COLUMN_CRC)->IsMarked());

            BMenu* viewMenu = m_mainMenu->m_viewMenu;
            if (viewMenu->FindItem(M_TOGGLE_TOOLBAR)->IsMarked() != m_toolBar->IsShown())
                m_toolBar->Toggle();

            if (viewMenu->FindItem(M_TOGGLE_INFOBAR)->IsMarked() != m_infoBar->IsShown())
                m_infoBar->Toggle();

            if (viewMenu->FindItem(M_TOGGLE_LOG)->IsMarked() == m_logHidden)
                ToggleActionLog();

            break;
        }

        case M_TOGGLE_COLUMN_SIZE: ToggleColumn(m_sizeColumn, message); break;
        case M_TOGGLE_COLUMN_PACKED: ToggleColumn(m_packedColumn, message); break;
        case M_TOGGLE_COLUMN_RATIO: ToggleColumn(m_ratioColumn, message); break;
        case M_TOGGLE_COLUMN_PATH: ToggleColumn(m_pathColumn, message); break;
        case M_TOGGLE_COLUMN_DATE: ToggleColumn(m_dateColumn, message); break;
        case M_TOGGLE_COLUMN_METHOD: ToggleColumn(m_methodColumn, message); break;
        case M_TOGGLE_COLUMN_CRC: ToggleColumn(m_crcColumn, message); break;

        case M_FILE_DELETE:
        {
            BString alertStr(B_TRANSLATE("Delete this archive from disk?"));
            alertStr << "\n" << B_TRANSLATE("This operation cannot be reverted.");
            BAlert* confirm = new BAlert("", alertStr, BZ_TR(kCancelString),
                                         BZ_TR(kDeleteString), B_TRANSLATE("Move to Trash"),
                                         B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
            confirm->SetShortcut(0, B_ESCAPE);
            confirm->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
            confirm->AddToSubset(this);
            int32 index = confirm->Go();

            if (index == 2)                  // Trash archive
            {
                BPath trashPath;
                find_directory(B_TRASH_DIRECTORY, &trashPath, false);
                BDirectory trashDir(trashPath.Path());

                m_archiveEntry.MoveTo(&trashDir, NULL, false);
            }
            else if (index == 1)           // Delete archive
                m_archiveEntry.Remove();
            else                         // Cancel
                break;

            PostMessage(M_FILE_CLOSE);
            break;
        }

        case M_FILE_PASSWORD:
        {
            if (!m_archiver)
                break;

            InputAlert* passwordAlert = new InputAlert(B_TRANSLATE("Enter the password to be used while extracting and adding files."),
                                                       B_TRANSLATE("Password:"), m_archiver->Password().String(), true,
                                                       BZ_TR(kCancelString),
                                                       B_TRANSLATE("Clear password"), B_TRANSLATE("Set password"),
                                                       B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
            BMessage msg = passwordAlert->GetInput(this);

            int32 buttonIndex;
            const char* password;
            if (msg.FindInt32(InputAlert::kButtonIndex, &buttonIndex) == B_OK &&
                    msg.FindString(InputAlert::kInputText, &password) == B_OK)
            {
                if (buttonIndex == 2L)
                    m_archiver->SetPassword(password);

                if (buttonIndex == 1L)
                    m_archiver->SetPassword("");
            }
            break;
        }

        case M_LOG_CONTEXT_CLEAR: m_logTextView->SetText(NULL, (int32)0); break;

        default:
            BWindow::MessageReceived(message);
    }
}


void MainWindow::Show()
{
    m_logTextView->MakeFocus(false);
    m_listView->MakeFocus(true);
    BWindow::Show();
}


void MainWindow::FrameResized(float newWidth, float newHeight)
{
    if (m_logHidden)
    {
        BRect bounds(Bounds());
        m_splitPane->SetBarLocked(false);
        m_splitPane->SetBarPosition(BPoint(bounds.Width(), bounds.Height()));
        m_splitPane->SetBarLocked(true);
    }

    // Bug fix - 0.05 (check with Splitter bar resizing to least possible height at a fast pace)
    // Now the column header in CLV doesn't get ugly
    float minH, maxH, minV, maxV;
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    if (Frame().Height() <= minV + 20)
        m_listView->Invalidate();

    BWindow::FrameResized(newWidth, newHeight);
}


//
// --- Interface Functions ---
//


void MainWindow::AddInfoBar()
{
    // We need not free slotPositions, it will be freed by InfoBar's destructor
    BList* slotPositions = new BList(3);
    slotPositions->AddItem((void*)&m_slotOneX);
    slotPositions->AddItem((void*)&m_slotTwoX);

    font_height fntHt;
    be_plain_font->GetHeight(&fntHt);
    float infoBarHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 10;

    m_infoBar = new InfoBar(BRect(0, m_toolBar->Frame().bottom + 1, Bounds().right,
                                  m_toolBar->Frame().bottom + 1 + infoBarHeight), slotPositions, "MainWindow:InfoBar",
                            ui_color(B_PANEL_BACKGROUND_COLOR));
    m_backView->AddChild(m_infoBar);
}


void MainWindow::AddSplitterBar()
{
    // Setup the splitter bar, which handles adding child operation
    m_splitPane = new SplitPane(BRect(0, m_infoBar->Frame().bottom + 1, Bounds().right,
                                      Bounds().bottom), m_listContainer, m_outputBackView, B_FOLLOW_ALL_SIDES);
    m_splitPane->SetResizeViewOne(true, true);
    m_splitPane->SetAlignment(B_HORIZONTAL);
    m_splitPane->SetBarThickness(BPoint(8.0, 8.0));
    m_splitPane->SetViewInsetBy(BPoint(0.0, 0.0));
    m_splitPane->SetMinSizeOne(BPoint(m_fileNameColumn->Width() + 36.0 + B_V_SCROLL_BAR_WIDTH + 8.0,
                                      m_listView->GetColumnLabelView()->Frame().Height() + B_H_SCROLL_BAR_HEIGHT + 10.0));
    m_splitPane->SetBarPosition(BPoint(Bounds().Width() * (75.0 / 100.0),
                                       Bounds().Height() * (62.0 / 100.0)));
    m_splitPane->SetViewTwoDetachable(true);
    m_splitPane->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    m_backView->AddChild(m_splitPane);
}


void MainWindow::AddToolBar()
{
    BRect buttonRect(0, 0, K_TOOLBAR_WIDTH + 8, 0);
    rgb_color backColor = ui_color(B_PANEL_BACKGROUND_COLOR);

    BitmapPool* _bmps = _glob_bitmap_pool;

    // Construct the toolbar buttons
    m_newButton = new ImageButton(buttonRect, "MainWindow:New", BZ_TR(kNewString), _bmps->m_tbarNewBmp,
                                  NULL, new BMessage(M_FILE_NEW), false, backColor);
    m_newButton->SetToolTip(const_cast<char*>(B_TRANSLATE("Create a new archive")));

    m_openButton = new ImageButton(buttonRect, "MainWindow:Open", BZ_TR(kOpenString), _bmps->m_tbarOpenBmp,
                                   NULL, new BMessage(M_FILE_OPEN), true, backColor);
    m_openButton->SetToolTip(const_cast<char*>(BZ_TR(kOpenExistingString)));

    m_searchButton = new ImageButton(buttonRect, "MainWindow:Search", BZ_TR(kSearchString),
                                     _bmps->m_tbarSearchBmp, _bmps->m_tbarSearchDisabledBmp,
                                     new BMessage(M_ACTIONS_SEARCH_ARCHIVE), false, backColor);
    m_searchButton->SetToolTip(const_cast<char*>(B_TRANSLATE("Search for files in the archive")));

    m_extractButton = new ImageButton(buttonRect, "MainWindow:Extact", BZ_TR(kExtractString),
                                      _bmps->m_tbarExtractBmp, _bmps->m_tbarExtractDisabledBmp,
                                      new BMessage(M_ACTIONS_EXTRACT), true, backColor);
    m_extractButton->SetEnabled(false);
    m_extractButton->SetToolTip(const_cast<char*>(B_TRANSLATE("Extract contents of archive")));

    m_viewButton = new ImageButton(buttonRect, "MainWindow:View", BZ_TR(kViewString),
                                   _bmps->m_tbarViewBmp, _bmps->m_tbarViewDisabledBmp, new BMessage(M_ACTIONS_VIEW), false,
                                   backColor);
    m_viewButton->SetEnabled(false);
    m_viewButton->SetToolTip(const_cast<char*>(B_TRANSLATE("View selected file(s)")));

    m_addButton = new ImageButton(buttonRect, "MainWindow:Add", BZ_TR(kAddString),
                                  _bmps->m_tbarAddBmp, _bmps->m_tbarAddDisabledBmp, new BMessage(M_ACTIONS_ADD), false,
                                  backColor);
    m_addButton->SetEnabled(false);
    m_addButton->SetToolTip(const_cast<char*>(B_TRANSLATE("Add files to archive")));

    m_deleteButton = new ImageButton(buttonRect, "MainWindow:Delete", BZ_TR(kDeleteString),
                                     _bmps->m_tbarDeleteBmp, _bmps->m_tbarDeleteDisabledBmp,
                                     new BMessage(M_ACTIONS_DELETE), false, backColor);
    m_deleteButton->SetEnabled(false);

    m_deleteButton->SetToolTip(const_cast<char*>(B_TRANSLATE("Delete selected files & folders")));

    // Construct the toolbar object
    float btnWidth, btnHeight;
    m_newButton->GetPreferredSize(&btnWidth, &btnHeight);

    m_toolBar = new ToolBar(BRect(0, m_mainMenu->Bounds().Height() + 1, Bounds().right,
                                  m_mainMenu->Bounds().Height() + 2 + btnHeight + 2 * ToolBar::kBorder),
                            "MainWindow:ToolBar", backColor);

    // Add toolbar to the window's view and add toolbar buttons to the toolbar object
    m_backView->AddChild(m_toolBar);
    m_toolBar->AddItem(m_newButton);
    m_toolBar->AddItem(m_openButton);
    m_slotOneX = m_toolBar->AddSeparatorItem();         // Remember this slot position
    m_toolBar->AddItem(m_searchButton);
    m_toolBar->AddItem(m_extractButton);
    m_toolBar->AddItem(m_viewButton);
    m_toolBar->AddItem(m_addButton);
    m_toolBar->AddItem(m_deleteButton);
    m_slotTwoX = m_toolBar->AddSeparatorItem(true);     // Remember this slot position as well
}


void MainWindow::AddListView()
{
    // Just an initial setup of the ColumnListView to keep the window constructor fairly clear
    m_listView = new BeezerListView(BRect(0, 0, Bounds().right - B_V_SCROLL_BAR_WIDTH - 1,
                                          Bounds().bottom - B_H_SCROLL_BAR_HEIGHT), &m_listContainer,
                                    "MainWindow:ListView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS |
                                    B_NAVIGABLE, B_MULTIPLE_SELECTION_LIST, true, true, true, true, B_NO_BORDER);

    // Set up the ColumnListView columns, first one is the expander column. Next column holds the icon
    m_listView->AddColumn(new CLVColumn(NULL, 20.0, CLV_EXPANDER | CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE));
    m_listView->AddColumn(new CLVColumn(NULL, 20.0, CLV_USER_SPECIAL_1 | CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE |
                                        CLV_NOT_RESIZABLE | CLV_PUSH_PASS | CLV_MERGE_WITH_RIGHT));

    // Create the data columns
    uint32 columnFlags = CLV_SORT_KEYABLE | CLV_TELL_ITEMS_WIDTH | CLV_HEADER_TRUNCATE;
    m_fileNameColumn = new CLVColumn(BZ_TR(kNameString), 300, columnFlags | CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE);
    m_sizeColumn = new CLVColumn(BZ_TR(kSizeString), 96, columnFlags | CLV_RIGHT_JUSTIFIED);
    m_packedColumn = new CLVColumn(BZ_TR(kPackedString), 72, columnFlags | CLV_RIGHT_JUSTIFIED);
    m_ratioColumn = new CLVColumn(BZ_TR(kRatioString), 64, columnFlags | CLV_RIGHT_JUSTIFIED);
    m_pathColumn = new CLVColumn(BZ_TR(kPathString), 128, columnFlags);
    m_dateColumn = new CLVColumn(BZ_TR(kDateString), 142, columnFlags);
    m_methodColumn = new CLVColumn(BZ_TR(kMethodString), 72, columnFlags);
    m_crcColumn = new CLVColumn(BZ_TR(kCRCString), 64, columnFlags);

    // Add all the columns to the list View
    m_columnList.AddItem(m_fileNameColumn);
    m_columnList.AddItem(m_sizeColumn);
    m_columnList.AddItem(m_packedColumn);
    m_columnList.AddItem(m_ratioColumn);
    m_columnList.AddItem(m_pathColumn);
    m_columnList.AddItem(m_dateColumn);
    m_columnList.AddItem(m_methodColumn);
    m_columnList.AddItem(m_crcColumn);

    m_listView->AddColumnList(&m_columnList);

    m_listView->SetSortFunction(BeezerListView::SortFunction);
    UpdateListView(false);
}


void MainWindow::AddLogTextView()
{
    // Setup the output text view's background (parent) view (AddChild done by SplitPane)
    m_outputBackView = new BevelView(BRect(0, Bounds().bottom - (Bounds().Height() * (35.0 / 100.0)),
                                           Bounds().right, Bounds().bottom), "MainWindow:OutputBackView",
                                     BevelView::OUTSET, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_outputBackView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    // Create an inset view just for looks
    BevelView* insetView = new BevelView(m_outputBackView->Bounds().InsetByCopy(K_MARGIN, K_MARGIN),
                                         "MainWindow:InnerLogBackView", BevelView::INSET,
                                         B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_outputBackView->AddChild(insetView);
    float edge = insetView->EdgeThickness();

    // Setup the LogTextView
    m_logTextView = new LogTextView(BRect(edge, edge, insetView->Bounds().right - B_V_SCROLL_BAR_WIDTH
                                          - 2 * edge, insetView->Bounds().bottom - edge), "MainWindow:LogTextView",
                                    B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

    m_logTextView->SetContextMenu(m_mainMenu->m_logContextMenu);

    // Setup scrolling for the LogTextView
    BScrollView* scrollView = new BScrollView("MainWindow:LogScrollView", m_logTextView,
            B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true, B_NO_BORDER);
    insetView->AddChild(scrollView);
}


void MainWindow::ToggleColumn(CLVColumn* col, BMessage* message)
{
    // Toggle the column & menu item (the message is passed to us when the BMenuItem is clicked)
    col->SetShown(!col->IsShown());
    BMenuItem* item = NULL;
    if (message)
        message->FindPointer("source", reinterpret_cast<void**>(&item));
    else
    {
        BMenu* colMenu = m_mainMenu->m_columnsSubMenu;

        if (col == m_sizeColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_SIZE);
        else if (col == m_packedColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_PACKED);
        else if (col == m_ratioColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_RATIO);
        else if (col == m_pathColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_PATH);
        else if (col == m_dateColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_DATE);
        else if (col == m_methodColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_METHOD);
        else if (col == m_crcColumn) item = colMenu->FindItem(M_TOGGLE_COLUMN_CRC);
    }

    if (item)
        item->SetMarked(!(item->IsMarked()));
}


void MainWindow::ToggleToolBar()
{
    m_mainMenu->m_viewMenu->FindItem(M_TOGGLE_TOOLBAR)->SetMarked(!(m_toolBar->IsShown()));
    m_toolBar->Toggle();
}


void MainWindow::ToggleInfoBar()
{
    m_mainMenu->m_viewMenu->FindItem(M_TOGGLE_INFOBAR)->SetMarked(!(m_infoBar->IsShown()));
    m_infoBar->Toggle();
}


void MainWindow::ToggleActionLog()
{
    // Toggle the activity log by resizing it to the end of the window (almost like hiding)
    if (m_logHidden == true)
    {
        //m_splitPane->SetBarPosition (BPoint (Bounds().Height() * 40.0 / 100.0,
        //               Bounds().Width() * 40.0 / 100.0));
        m_splitPane->RestoreBarPosition();
        m_splitPane->ViewAt(2L)->Invalidate();
    }
    else
    {
        m_splitPane->StoreBarPosition();
        m_splitPane->SetBarPosition(BPoint(Bounds().Width(), Bounds().Height()));
    }

    m_mainMenu->m_viewMenu->FindItem(M_TOGGLE_LOG)->SetMarked(m_logHidden);
    m_logHidden = !m_logHidden;
    m_splitPane->SetBarLocked(m_logHidden);
}


void MainWindow::AdjustColumns()
{
    // Hide columns that the archiver doesn't have info about
    BList newColumnList = m_archiver->HiddenColumns(m_columnList);
    for (int8 i = 0L; i < newColumnList.CountItems(); i++)
    {
        CLVColumn* hiddenColumn = (CLVColumn*)newColumnList.ItemAtFast(i);
        for (int8 j = 0L; j < m_columnList.CountItems(); j++)
        {
            CLVColumn* shownColumn = (CLVColumn*)m_columnList.ItemAtFast(j);
            if (shownColumn == hiddenColumn && shownColumn->IsShown())
                ToggleColumn(hiddenColumn, NULL);
        }
    }
}


//
// --- Update Functions ---
//


void MainWindow::UpdateListView(bool invalidate)
{
    rgb_color actFore = _prefs_interface.FindColorDef(kPfActFore, K_ACTIVE_FORE_COLOR);
    rgb_color actBack = _prefs_interface.FindColorDef(kPfActBack, K_ACTIVE_SELECT_COLOR);

    m_listView->SetItemSelectForeColor(true, actFore);
    m_listView->SetItemSelectForeColor(false, actFore);
    m_listView->SetItemSelectColor(true, actBack);
    m_listView->SetItemSelectColor(false, actBack);
    if (invalidate)
        m_listView->Invalidate(Bounds());
}


void MainWindow::UpdateFileSelectNeeders(bool enable)
{
    // Items that should be enabled when file items are selected, disabled otherwise
    m_viewButton->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_VIEW)->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_OPEN_WITH)->SetEnabled(enable);
}


void MainWindow::UpdateFocusNeeders(bool enable)
{
    // When listview has focus- enable/disable according to selection - otherwise enable
    m_mainMenu->m_editMenu->FindItem(M_EDIT_COPY)->SetEnabled(enable);
}


void MainWindow::UpdateSelectNeeders(bool enable)
{
    // Items that should be enabled when any item is selected, disabled when no selection
    m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractSelectedString))->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_COPY)->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_DELETE)->SetEnabled(enable);
    m_deleteButton->SetEnabled(enable);
}


void MainWindow::UpdateListItemNeeders(bool enable)
{
    // Items that should be disabled when no items are there in the list, enabled otherwise
    m_mainMenu->m_editMenu->FindItem(M_EDIT_EXPAND_SELECTED)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_COLLAPSE_SELECTED)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_EXPAND_ALL)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_COLLAPSE_ALL)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(BZ_TR(kSelectAllString))->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_DESELECT_ALL)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_INVERT_SELECTION)->SetEnabled(enable);
}


void MainWindow::UpdateFolderSelectNeeders(bool enable)
{
    // Items that should enabled when folder items are selected, disabled otherwsie
    m_mainMenu->m_editMenu->FindItem(M_EDIT_EXPAND_SELECTED)->SetEnabled(enable);
    m_mainMenu->m_editMenu->FindItem(M_EDIT_COLLAPSE_SELECTED)->SetEnabled(enable);
}


void MainWindow::UpdateValidArchiveNeeders(bool enable)
{
    // Here the commands that need a valid archive like "Extract" commands etc. are disabled/enabled
    m_addButton->SetEnabled(enable);
    m_extractButton->SetEnabled(enable);
    m_searchButton->SetEnabled(enable);

    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_EXTRACT)->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractToString))->SetEnabled(enable);

    if (enable == true)
        m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractSelectedString))->SetEnabled(m_listView->HasSelection());
    else
        m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractSelectedString))->SetEnabled(false);

    // Comments-alone we will handle it differently - first the format must support comments,
    BMenuItem* item = m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_COMMENT);
    if (item)
        if (!m_badArchive)
            item->SetEnabled(enable);

    // We handle archive passwords differently
    item = m_mainMenu->m_fileMenu->FindItem(M_FILE_PASSWORD);
    if (item)
        item->SetEnabled(enable);

    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_SEARCH_ARCHIVE)->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_TEST)->SetEnabled(enable);

    m_mainMenu->m_fileMenu->FindItem(M_FILE_DELETE)->SetEnabled(enable);
    m_mainMenu->m_fileMenu->FindItem(M_FILE_ARCHIVE_INFO)->SetEnabled(enable);

    if (m_badArchive)
        enable = false;

    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_ADD)->SetEnabled(enable);
    m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_CREATE_FOLDER)->SetEnabled(enable);
}


void MainWindow::UpdateInfoBar(int32 count, int64 bytes) const
{
    // The below "false" indicate NOT to change the total count/bytes
    m_infoBar->UpdateFilesDisplay(count, 0, false);
    m_infoBar->UpdateBytesDisplay(bytes, 0, false);
}


void MainWindow::UpdateUIAsPerSelection()
{
    bool enable = false;
    bool allDirs = true;
    bool atLeastOneSelection = false;
    int32 selected, i = 0L;

    while ((selected = m_listView->FullListCurrentSelection(i++)) >= 0)
    {
        atLeastOneSelection = true;
        ListEntry* item = dynamic_cast<ListEntry*>(m_listView->FullListItemAt(selected));
        if (item->IsSuperItem() == false)
        {
            enable = true;
            allDirs = false;
            break;
        }
    }

    UpdateFileSelectNeeders(enable);

    // Because we have allDirs initially to true we need to check for no selection case
    UpdateSelectNeeders(atLeastOneSelection);
    if (atLeastOneSelection == true)
        UpdateFolderSelectNeeders(allDirs);
    else
        UpdateFolderSelectNeeders(false);
}


void MainWindow::UpdateWindowManager(bool isClosing)
{
    if (isClosing)
        m_windowMgr->RemoveWindow(dynamic_cast<BWindow*>(this));
    else
    {
        UpdateWindowsMenu();
        m_windowMgr->UpdateFrom(dynamic_cast<BWindow*>(this), false);
    }
}


void MainWindow::ActivateUIForValidArchive()
{
    // Bad archive may mean corrupted or a non-archive acting like an archive, we can't differentiate
    // as both come from err stream, so give benefit of doubt and enable things as tho valid archive
    UpdateValidArchiveNeeders(true);

    if (m_fileList->CountItems() > 0L || m_dirList->CountItems() > 0L)
        UpdateListItemNeeders(true);

    // Allow context-menu to be invoked from this point on, its safe now as loading is complete
    m_listView->SetContextMenu(m_mainMenu->m_archiveContextMenu);
    m_listView->SortItems();
}


void MainWindow::UpdateNewWindow()
{
    m_recentMgr->AddPath(m_archivePath.Path());
    m_windowMgr->UpdateFrom(this, new BMessage(M_UPDATE_RECENT), true);
    UpdateRecentMenu();
    UpdateExtractMenu();
    UpdateWindowManager();
}


void MainWindow::UpdateRecentMenu()
{
    m_mainMenu->SetRecentMenu(m_recentMgr->BuildMenu(BZ_TR(kOpenMenuString), "refs", be_app));
    m_openButton->SetContextMenu(m_recentMgr->BuildPopUpMenu(NULL, "refs", be_app));
}


void MainWindow::UpdateExtractMenu()
{
    BMenu* menu = m_extractMgr->BuildMenu(BZ_TR(kExtractToString), kPath, NULL);
    AddFavouriteExtractPaths(menu);
    AddDynamicExtractPaths(menu);

    BPopUpMenu* popupMenu = m_extractMgr->BuildPopUpMenu(NULL, kPath, NULL);
    AddFavouriteExtractPaths((BMenu*)popupMenu);
    AddDynamicExtractPaths((BMenu*)popupMenu);

    BMenuItem* item = m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractSelectedString));
    bool enable = item->IsEnabled();

    m_mainMenu->SetExtractPathsMenu(menu);
    m_extractButton->SetContextMenu(popupMenu);

    m_mainMenu->m_actionsMenu->FindItem(BZ_TR(kExtractSelectedString))->SetEnabled(enable);
}


void MainWindow::UpdateWindowsMenu()
{
    // Need to reconstruct the window list as something changed - new/close/title change happened
    BMenu* wndMenu = m_mainMenu->m_windowsMenu;
    int32 wndCount = wndMenu->CountItems();
    m_mainMenu->m_windowsMenu->RemoveItems(0L, wndCount, true);

    wndCount = m_windowMgr->CountWindows();
    for (int32 i = 0; i < wndCount; i++)
    {
        BWindow* wndPtr = m_windowMgr->WindowAt(i);
        if (wndPtr)
        {
            BMessage* msg = new BMessage(M_SWITCH_WINDOW);
            msg->AddPointer(kWindowPtr, wndPtr);

            BMenuItem* menuItem = new BMenuItem(wndPtr->Title(), msg, i < 10 ? i + 48 : 0);
            wndMenu->AddItem(menuItem);
            if (wndPtr == this)
                menuItem->SetMarked(true);
        }
    }
}


void MainWindow::SetBusyState(bool on) const
{
    // Note - the BarberPole() will NOT adjust pulse when it stops animating
    // as we will be passing false to the "adjustPulse" parameter -- but we
    // dont' adjust infobar through Pulse anymore so this wouldn't matter now
    // Because pulse messages accumulates we find ourselfs calling UpdateInfoBar
    // after long selectall processes which locks the window unnecessarily- so we don't
    // use pulse anymore for the window
    m_infoBar->LoadIndicator()->SetValue(on, on);
}


//
// --- Delete Functions ----
//


void MainWindow::ClearDeleteLists()
{
    if (m_deleteFileList != NULL)
    {
        m_deleteFileList->MakeEmpty();
        delete m_deleteFileList;
        m_deleteFileList = NULL;
    }

    if (m_deleteDirList != NULL)
    {
        m_deleteDirList->MakeEmpty();
        delete m_deleteFileList;
        m_deleteDirList = NULL;
    }
}


void MainWindow::DeleteFilesFromArchive()
{
    // Check if archive is NOT readonly
    if (CanWriteArchive() == false)
        return;

    // 0.06: Check if the archiver supports deleting of files
    if (m_archiver->CanDeleteFiles() == false)
    {
        ShowOpNotSupported();
        return;
    }

    // Setup deletion process
    ClearDeleteLists();
    BMessage* msg = new BMessage(M_ACTIONS_DELETE);
    m_listView->SendSelectionMessage(false);
    m_deleteFileList = new BList(m_listView->FullListSelectionCount());
    m_deleteDirList = new BList(m_listView->FullListSelectionCount());

    m_publicThreadCancel = false;        // Reset this
    m_statusWnd = new StatusWindow(B_TRANSLATE("Preparing to delete" B_UTF8_ELLIPSIS), this,
                                   B_TRANSLATE("Gathering information"), &m_publicThreadCancel);

    ListEntry* selEntry(NULL);
    BList hackEntryList;
    int32 i = 0L;
    int32 count = 0L;
    while ((selEntry =
            (ListEntry*)m_listView->FullListItemAt(m_listView->FullListCurrentSelection(i))) != NULL)
    {
        if (m_publicThreadCancel == true)
            break;

        // Addition of files to the list of files to remove
        if (selEntry->IsSuperItem() == false)
        {
            // Don't add file entries if the folder entry has been added (SELECTED) but add files
            // that are at the root i.e. no parent folder (BUG-FIX)
            if (m_listView->Superitem((CLVListItem*)selEntry) == NULL ||
                    (m_listView->Superitem((CLVListItem*)selEntry))->IsSelected() == false)
            {
                BString buf = selEntry->m_fullPath.String();
                buf.ReplaceAll("*", "\\*");
                msg->AddString(kPath, buf.String());
            }

            m_deleteFileList->AddItem(selEntry);
            count++;
        }
        else           // Addition of folders
        {
            // For add-ons that SupportFolderEntity() == false (like zip) if we simply pass the folder
            // name it won't delete files inside it, we need to pass folder/* but before this we need
            // to escape * with \\* from the path
            if (m_archiver->SupportsFolderEntity() == false)
            {
                BString buf = selEntry->m_fullPath.String();
                buf.ReplaceAll("*", "\\*");         // escape name manually, then add the real wildcard
                buf << '/' << '*';                  // so that names with * are escaped and finally * is appended
                msg->AddString(kPath, buf);

                // If a folder is collapsed and selected add all its subitems to our message,
                // else select its subitems this is critical as we rely on selection (as mentioned above)
                if (selEntry->IsExpanded() == false)
                    count += AddFolderToMessage(selEntry, msg, false, m_deleteFileList, m_deleteDirList, true);
                else
                    m_listView->SelectSubItems((CLVListItem*)selEntry);  // alter selection so items under
                // this dir are included in the loop
                m_deleteDirList->AddItem(selEntry);
                count++;
            }
            else
            {
                if (selEntry->IsExpanded() == true)
                    hackEntryList.AddItem((void*)selEntry);

                msg->AddString(kPath, selEntry->m_fullPath.String());
                ++count;

                m_archiver->Table()->FindUnder(selEntry->m_fullPath.String(), *m_deleteFileList, *m_deleteDirList);
                // Don't select any sub-items here as what happens is (since this main loop loops selected
                // items we re-add items that are selected, so don't change selection here)
            }
        }

        i++;
    }

    // Change selection if needed (for hackentries)
    for (int32 g = 0; g < hackEntryList.CountItems(); g++)
    {
        ListEntry* hackEntry = (ListEntry*)hackEntryList.ItemAtFast(g);
        m_listView->SelectSubItems((CLVListItem*)hackEntry);
    }

    m_statusWnd->PostMessage(M_CLOSE);
    if (m_publicThreadCancel == true)
    {
        delete msg;
        CancelDelete();
        return;
    }

    // Don't update infobar - as it will result in recount of items, bytes which will
    // further slow down the process while deleting LARGE number of files (like BeBookmarks.zip)
    // Now infobar will display incorrect (outdated) information

    // Ask confirmation!!
    BString alertStr(B_TRANSLATE("Delete the selected item(s)?"));
    alertStr << "\n" << B_TRANSLATE("This operation cannot be reverted.");
    BAlert* warnAlert = new BAlert("Warning", alertStr, B_TRANSLATE("Delete"), BZ_TR(kCancelString),
                                   NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT);
    warnAlert->SetDefaultButton(warnAlert->ButtonAt(1L));
    warnAlert->SetShortcut(1L, B_ESCAPE);
    warnAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
    warnAlert->AddToSubset(this);
    int32 buttonIndex = warnAlert->Go();

    // If cancel, cancel gracefully - bug fixed here
    if (buttonIndex == 1L || count == 0L)
    {
        delete msg;
        CancelDelete();
        return;
    }

    msg->AddInt32(kCount, count);
    msg->AddString(kPreparing, B_TRANSLATE("Preparing to delete" B_UTF8_ELLIPSIS));
    msg->AddString(kProgressAction, B_TRANSLATE("Deleting" B_UTF8_ELLIPSIS));

    // Proceed to remove files
    BMessenger* messenger(NULL);
    volatile bool* cancel;
    m_progressWnd = new ProgressWindow(this, msg, messenger, cancel);

    BMessage* threadInfo = new BMessage('inf_');
    threadInfo->AddMessenger(kProgressMessenger, *messenger);
    threadInfo->AddPointer(kArchiverPtr, (void*)m_archiver);
    threadInfo->AddPointer(kWindowPtr, (void*)this);
    threadInfo->AddPointer(kCancel, (void*)cancel);
    threadInfo->AddMessage(kFileList, msg);
    delete msg;

    m_logTextView->AddText(B_TRANSLATE("Deleting" B_UTF8_ELLIPSIS), true, false, false);

    resume_thread(spawn_thread(_deletor, "_deletor", B_NORMAL_PRIORITY, (void*)threadInfo));
}


void MainWindow::CancelDelete()
{
    m_deleteFileList->MakeEmpty();        // Very important or else ClearDeleteList() will delete selected
    m_deleteDirList->MakeEmpty();        // items, to avoid this free the pointers in list, then call Clear...
    ClearDeleteLists();

    m_listView->SendSelectionMessage(true);
    m_listView->SelectionChanged();
}


void MainWindow::DeleteDone(BMessage* message)
{
    status_t result;
    message->FindInt32(kResult, &result);
    switch (result)
    {
        case BZR_CANCEL_ARCHIVER:
        {
            // Cancelling a delete is painful
            BString alertStr(B_TRANSLATE("Cannot continue."));
            alertStr << "\n" << B_TRANSLATE("A critical operation has been cancelled and the archive is in an unknown state.");
            BAlert* errAlert = new BAlert("Error", alertStr, BZ_TR(kCloseWindowString), NULL,
                                          NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
            errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
            errAlert->AddToSubset(this);
            errAlert->Go();
            PostMessage(M_FILE_CLOSE);
            break;
        }

        case BZR_DONE: case BZR_ERRSTREAM_FOUND:
        {
            if (result == BZR_DONE)
            {
                m_logTextView->AddText(B_TRANSLATE("Done."), false, false, false);
                DeleteUpdate();
            }
            else
                m_logTextView->AddText(B_TRANSLATE("Error(s) found."), false, false, false);
            break;
        }


        // 0.06: We check this is in "DeleteFilesFromArchive()" itself, hence avoid re-checking
        //case BZR_NOT_SUPPORTED:
        //    ShowOpNotSupported ();
        //    break;

        case BZR_ARCHIVE_PATH_INIT_ERROR:
            ShowArkPathError();
            break;
    }
}


void MainWindow::DeleteUpdate()
{
    // Update all hash items, list view of the deleted items
    ListEntry* selEntry(NULL);
    size_t bytesRemoved = 0;

    m_listView->SendSelectionMessage(false);

    // Delete file items by looking it up from the hashtable
    for (int32 x = 0; x < m_deleteFileList->CountItems(); x++)
    {
        selEntry = (ListEntry*)m_deleteFileList->ItemAtFast(x);
        BString path = selEntry->m_fullPath;

        // We put this in a while loop so that archivers like tar delete all occurrences of the file
        // if there are duplicate files
        HashEntry* entry(NULL);
        while ((entry = m_archiver->Table()->Find(path.String())) != NULL)
        {
            bytesRemoved += entry->m_clvItem->m_length;
            m_fileList->RemoveItem(entry);
            m_listView->RemoveItem(entry->m_clvItem);

            delete entry->m_clvItem;
            m_archiver->Table()->Delete(entry);

            // No need to waste time Find()ing selEntry again if the archiver has only
            // unique files
            if (m_archiver->CanReplaceFiles() == true)
                break;
        }
    }

    // Delete folder items -- we look-up the hashtable and find the CLVItem we need from the list
    // of HashTable folder items
    for (int32 x = 0; x < m_deleteDirList->CountItems(); x++)
    {
        selEntry = (ListEntry*)m_deleteDirList->ItemAtFast(x);
        HashEntry* entry = m_archiver->Table()->Find(selEntry->m_fullPath.String());
        if (entry)
        {
            bytesRemoved += selEntry->m_length;
            m_dirList->RemoveItem(entry);
            m_listView->RemoveItem(selEntry);

            m_archiver->Table()->Delete(entry);     // Bug fixed
            delete selEntry;
        }
    }

    // Update archive size, displays
    m_archiveSize -= bytesRemoved;
    m_infoBar->UpdateFilesDisplay(0L, m_fileList->CountItems() + m_dirList->CountItems(), true);
    m_infoBar->UpdateBytesDisplay(0L, m_archiveSize, true);

    m_listView->SendSelectionMessage(true);
    m_listView->SelectionChanged();
    ClearDeleteLists();
}


//
// --- Test Functions ---
//


void MainWindow::TestArchive()
{
    // Setup test process
    BMessage msg(M_ACTIONS_TEST);
    volatile bool* cancel;

    msg.AddInt32(kCount, m_fileList->CountItems());
    msg.AddString(kPreparing, B_TRANSLATE("Preparing to test" B_UTF8_ELLIPSIS));
    msg.AddString(kProgressAction, B_TRANSLATE("Testing" B_UTF8_ELLIPSIS));

    BMessenger* messenger = NULL;
    m_progressWnd = new ProgressWindow(this, &msg, messenger, cancel);

    BMessage* threadInfo = new BMessage('inf_');
    threadInfo->AddPointer(kWindowPtr, (void*)this);
    threadInfo->AddMessenger(kProgressMessenger, *messenger);
    threadInfo->AddPointer(kArchiverPtr, (void*)m_archiver);
    threadInfo->AddPointer(kCancel, (void*)cancel);

    m_logTextView->AddText(B_TRANSLATE("Testing" B_UTF8_ELLIPSIS));
    thread_id tst_id = spawn_thread(_tester, "_tester", B_NORMAL_PRIORITY, (void*)threadInfo);
    resume_thread(tst_id);
}


void MainWindow::TestDone(BMessage* message)
{
    status_t result;
    message->FindInt32(kResult, &result);
    switch (result)
    {
        case BZR_CANCEL_ARCHIVER:
            m_logTextView->AddText(B_TRANSLATE("Cancelled."), false, false, false);
            break;

        case BZR_DONE: case BZR_ERRSTREAM_FOUND:
        {
            m_logTextView->AddText(result == BZR_DONE ? B_TRANSLATE("Done.") : B_TRANSLATE("Error(s) found."),
                                   false, false, false);

            const char* testResult = NULL;
            if (message->FindString(kText, &testResult) == B_OK)
            {
                BAlert* errAlert;
                if (result == BZR_ERRSTREAM_FOUND)
                {
                    BString alertStr(B_TRANSLATE("The archiver has encountered errors while deleting."));
                    alertStr << "\n" << B_TRANSLATE("Do you want to view a detailed report?");
                    errAlert = new BAlert("Error", alertStr, B_TRANSLATE("View report"), BZ_TR(kCancelString),
                                          NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
                }
                else
                {
                    BString alertStr(B_TRANSLATE("Test completed successfully without any errors."));
                    alertStr << "\n" << B_TRANSLATE("Do you want to view a detailed report?");
                    errAlert = new BAlert("Result", alertStr, B_TRANSLATE("View report"), BZ_TR(kOKString),
                                          NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
                }

                errAlert->SetDefaultButton(errAlert->ButtonAt(1L));
                errAlert->SetShortcut(1L, B_ESCAPE);
                errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                errAlert->AddToSubset(this);
                if (errAlert->Go() == 0L)
                {
                    BString title = (result == BZR_ERRSTREAM_FOUND ?
                                     B_TRANSLATE("Errors & warnings:") : B_TRANSLATE("Test results:"));
                    title << " " << Title();
                    new LogWindow(this, title.String(), testResult);
                }
            }
            break;
        }

        case BZR_ARCHIVE_PATH_INIT_ERROR:
            ShowArkPathError();
            break;

        case BZR_NOT_SUPPORTED:
            ShowOpNotSupported();
            break;
    }
}


//
// --- Extract Functions ---
//


void MainWindow::ExtractDone(BMessage* message)
{
    status_t result;
    entry_ref refToDir;
    message->FindInt32(kResult, &result);
    message->FindRef(kRef, &refToDir);

    switch (result)
    {
        case BZR_CANCEL_ARCHIVER:
        {
            m_logTextView->AddText(B_TRANSLATE("Extraction cancelled."), true, true, true);

            if (_prefs_extract.FindBoolDef(kPfOpen, true))
                TrackerOpenFolder(&refToDir);

            break;
        }

        case BZR_PASSWORD_ERROR:
        {
            m_logTextView->AddText(B_TRANSLATE("Error (password protected file)"), true, true, true);
            if (message->HasBool(kFailOnNull) == false)
            {
                BAlert* alert = new BAlert("Error",
                                           B_TRANSLATE("A password protection error has occurred.  Please set the correct password and retry"),
                                           BZ_TR(kCancelString), B_TRANSLATE("Set password" B_UTF8_ELLIPSIS),
                                           NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
                if (alert->Go() == 1L)
                    PostMessage(M_FILE_PASSWORD);
            }

            break;
        }

        case BZR_DONE:
        {
            m_logTextView->AddText(B_TRANSLATE("Done extracting."), true, true, true);

            if (_prefs_extract.FindBoolDef(kPfOpen, true))
                TrackerOpenFolder(&refToDir);

            if (_prefs_extract.FindBoolDef(kPfQuit, false))
                PostMessage(M_FILE_QUIT);
            else if (_prefs_extract.FindBoolDef(kPfClose, false))
                PostMessage(M_FILE_CLOSE);

            break;
        }

        case BZR_EXTRACT_DIR_INIT_ERROR:
        {
            m_logTextView->AddText(B_TRANSLATE("Couldn't create destination folder."), true, true, true);
            break;
        }

        case BZR_ARCHIVE_PATH_INIT_ERROR:
        {
            ShowArkPathError();
            break;
        }

        default:
        {
            m_logTextView->AddText(B_TRANSLATE("An unknown error occurred."), true, true, true);
            break;
        }
    }

    if ((result == BZR_DONE || result == BZR_CANCEL_ARCHIVER) && m_dragExtract == true)
    {
        if (_prefs_extract.FindBoolDef(kPfDrag, true) == false)
        {
            // Move files to destination folder as user has extracted without full path
            BPath path(&refToDir);
            BDirectory destDir(&refToDir);

            BString pathStr;
            int32 i = 0;
            status_t allMoved = B_ERROR;
            while (m_dragExtractItems.FindString(kPath, i++, &pathStr) == B_OK)
            {
                BPath buf(path);
                buf.Append(pathStr.String());

                BEntry bufEntry(buf.Path(), false);
                allMoved = bufEntry.MoveTo(&destDir);
            }

            // Delete base directory from path
            i = 0;
            while (m_dragExtractItems.FindString(kPath, i++, &pathStr) == B_OK)
            {
                BString buf;
                int32 index = pathStr.FindFirst('/');
                pathStr.CopyInto(buf, 0, index);

                buf.Prepend("/");
                buf.Prepend(path.Path());

                BDirectory dir(buf.String());
                BEntry dirEntry(&dir, NULL, false);
                if (dirEntry.Exists() == true && dirEntry.IsDirectory() == true && allMoved == B_OK)
                    RemoveDirectory(&dir);
            }

            m_dragExtractItems.MakeEmpty();
        }

        m_dragExtract = false;
    }

    // Bugfix: 0.06
    m_criticalSection = false;
}


void MainWindow::ExtractArchive(entry_ref *refToDir, bool fullArchive)
{
    // Setup extraction process

    // This is to prevent this window from quitting when it's still extracting and it receives a quit
    // message after some other archive has completed extraction -- try extracting BeBook, a smaller zip file
    // using "Quit after extract" option -- without setting this variable it will quit ALL archives before
    // they complete their extraction tasks, this m_criticalSection (to true) would prevent closing till the
    // extracting is done and close after all archives have completed their task
    // Bugfix: 0.06 (reported by Jess in BeBits Talkback)
    if (_prefs_extract.FindBoolDef(kPfQuit, false))     // Set only when Quit after extract option is used
        m_criticalSection = true;

    m_publicThreadCancel = false;        // Reset this incase some previous public thread was cancelled
    // as it will be checked in AddFolderToMessage()
    BMessage msg(fullArchive == true ? M_EXTRACT_TO : M_EXTRACT_SELECTED_TO);
    msg.AddRef(kRef, refToDir);

    // Don't include folder to the items to be extracted simply because archivers won't include
    // a progress-bar update for folder entries
    if (fullArchive == true)
        msg.AddInt32(kCount, m_fileList->CountItems());
    else
    {
        m_dragExtractItems.MakeEmpty();    // Contains list of paths (archive items) that are to be moved to
        // destination extract folder after extraction

        ListEntry* selEntry(NULL);
        int32 i = 0L;
        int32 fileCount = 0L;
        while ((selEntry =
                (ListEntry*)m_listView->FullListItemAt(m_listView->FullListCurrentSelection(i))) != NULL)
        {
            if (selEntry->IsSuperItem() == false)               // File - no problems just add it!
            {
                msg.AddString(kPath, selEntry->m_fullPath.String());
                fileCount++;

                if (_prefs_extract.FindBoolDef(kPfDrag, true) == false && m_dragExtract == true
                        && m_listView->Superitem(selEntry)
                        && m_listView->Superitem(selEntry)->IsSelected() == false)
                {
                    m_dragExtractItems.AddString(kPath, selEntry->m_fullPath.String());
                }
            }
            else
            {
                // If a folder is collapsed and selected add all its subitems to our message
                if (selEntry->IsExpanded() == false)    // Count only file entries not folder entries
                {
                    fileCount += AddFolderToMessage(selEntry, &msg, true);
                }
                else
                {
                    // For expanded+selected folders just select all its sub-items
                    // This is because for add-ons that DOES support folder entry instead of JUST
                    // extract that ONE dir (without contents) it would actually extract WITH the contents
                    // So what we do is: Whether the add-on supports folders or NOT we always extract
                    // the contents of a expanded+selected folder by selecting its subitems
                    m_listView->SelectSubItems((CLVListItem*)selEntry);

                    // Old method: Commented out - BUG FIX -- don't delete it just yet
                    // For expanded+selected folders just add the folder entry (add its path with
                    //    a trailing slash)
                    //BString pathWithTrailingSlash = selEntry->m_fullPath.String();
                    //pathWithTrailingSlash << '/';
                    //msg.AddString (kPath, pathWithTrailingSlash.String());
                }

                if (m_dragExtract == true && _prefs_extract.FindBoolDef(kPfDrag, true) == false)
                    m_dragExtractItems.AddString(kPath, selEntry->m_fullPath.String());
            }
            i++;
        }

        // fileCount shouldn't be zero due to disabling/enabling of the menuitem
        msg.AddInt32(kCount, fileCount);
    }

    msg.AddString(kPreparing, B_TRANSLATE("Preparing to extract" B_UTF8_ELLIPSIS));
    msg.AddString(kProgressAction, B_TRANSLATE("Extracting" B_UTF8_ELLIPSIS));

    BMessenger* messenger(NULL);
    volatile bool* cancel;
    m_progressWnd =  new ProgressWindow(this, &msg, messenger, cancel);

    BMessage* threadInfo = new BMessage('inf_');
    threadInfo->AddRef(kRef, refToDir);
    threadInfo->AddMessenger(kProgressMessenger, *messenger);
    threadInfo->AddPointer(kArchiverPtr, (void*)m_archiver);
    threadInfo->AddBool(kAllFiles, fullArchive);
    threadInfo->AddPointer(kWindowPtr, (void*)this);
    threadInfo->AddPointer(kCancel, (void*)cancel);

    if (!fullArchive)
        threadInfo->AddMessage(kFileList, &msg);

    thread_id ext_id = spawn_thread(_extractor, "_extractor", B_NORMAL_PRIORITY, (void*)threadInfo);

    m_logTextView->AddText(B_TRANSLATE("Extracting to:"));
    BPath dirPath(refToDir);
    m_logTextView->AddText(dirPath.Path(), false, false, false);

    resume_thread(ext_id);
}


bool MainWindow::IsExtractPathValid(const char* path, bool throwAlertErrorIfAny) const
{
    BEntry dirEntry(path, true);
    if (dirEntry.Exists() == false)
        create_directory(path, 0777);

    if (dirEntry.IsDirectory() == false)
    {
        if (throwAlertErrorIfAny)
        {
            BAlert* errAlert = new BAlert("Error",
                                          B_TRANSLATE("Failed to create destination folder.  File with the same name exists."),
                                          BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL,
                                          B_EVEN_SPACING, B_INFO_ALERT);
            errAlert->SetShortcut(0L, B_ESCAPE);
            errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
            errAlert->AddToSubset((BWindow*)this);
            errAlert->Go();
        }
        return false;
    }

    return true;
}


void MainWindow::SetupExtractPanel(BMessage* extractMessage)
{
    // Setups up the BFilePanel for extraction with a BRefFilter to handle updating of
    // current directory extract button
    if (m_extractToPanel == NULL)
    {
        m_extractToPanel = new SelectDirPanel(B_OPEN_PANEL, new BMessenger(this), 0,
                                              B_DIRECTORY_NODE, false, NULL);
        m_extractToPanel->SetButtonLabel(B_DEFAULT_BUTTON, BZ_TR(kExtractString));
        m_extractToPanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        m_extractToPanel->Window()->AddToSubset(this);
        m_extractToPanel->SetCurrentDirButton(B_TRANSLATE("Extract to"));
    }

    uint32 w = extractMessage->what;
    if (m_extractToPanel->Window()->LockLooper())
    {
        // Set the title for the BFilePanel
        BString titleStr = w == M_ACTIONS_EXTRACT_TO ? B_TRANSLATE("Extract archive") :
        B_TRANSLATE("Extract selected entries of");
        titleStr << " " << Title() << ":";
        m_extractToPanel->Window()->SetTitle(titleStr.String());
        m_extractToPanel->Window()->UnlockLooper();
    }

    // Reset the message of the BFilePanel
    BMessage msg(w == M_ACTIONS_EXTRACT_TO ? M_EXTRACT_TO : M_EXTRACT_SELECTED_TO);
    m_extractToPanel->SetMessage(&msg);
}


//
// --- Open Functions ---
//


int32 MainWindow::AddFolderToMessage(ListEntry* item, BMessage* message, bool countOnlyFiles,
                                     BList* fileList, BList* dirList, bool skipFiles)
{
    // The following code gets all subitems of an item both for expanded/unexpanded items
    uint32 parentLevel = item->OutlineLevel();
    int32 count = 0L, folderCount = 0L;
    int32 itemPos = m_listView->FullListIndexOf(item);
    int32 i = 0L;

    if (itemPos >= 0)
    {
        for (i = itemPos + 1; i >= 1; i++)
        {
            if (m_publicThreadCancel == true)
                break;

            ListEntry* subItem = (ListEntry*)m_listView->FullListItemAt(i);
            if (subItem == NULL || subItem->OutlineLevel() <= parentLevel)
                break;

            count++;
            if (subItem->IsSuperItem() == false)           // File
            {
                if (fileList)
                    fileList->AddItem(subItem);

                if (skipFiles == false)
                    message->AddString(kPath, subItem->m_fullPath.String());
            }
            else                                        // Folder
            {
                folderCount++;
                BString pathWithTrailingSlash = subItem->m_fullPath.String();
                pathWithTrailingSlash << '/';
                if (dirList)
                    dirList->AddItem(subItem);

                if (skipFiles == true)
                    pathWithTrailingSlash << '*';

                message->AddString(kPath, pathWithTrailingSlash.String());
            }
        }
    }

    return countOnlyFiles == true ? count - folderCount : count;
}


int32 MainWindow::AddFoldersFromList(int32 index, int32* totalItems)
{
    // Add split folders at a time, then we will be called again (MessageReceived:M_ADD_ITEMS), Any messages
    // inbetween can be processed this way -- for smaller archives (< split folders) add all at once */
    int32 count = m_dirList->CountItems();
    *totalItems = count;
    int32 limit = 0, split = 60;

    if (count >= split * 2)
    {
        if (index + (count / split) > count)
            limit = count;
        else
            limit = index + (count / split);

        for (; index < limit; index++)
        {
            ListEntry* dirEntry = ((HashEntry*)m_dirList->ItemAtFast(index))->m_clvItem;
            HashEntry* parent = m_archiver->Table()->Find(dirEntry->m_dirPath.String());
            if (parent)
                m_listView->AddUnderFast(dirEntry, parent->m_clvItem);
            else
                m_listView->AddItemFastHierarchical(dirEntry);

            dirEntry->m_added = true;
        }

        return index;
    }
    else
    {
        int32 i = 0L;
        for (; i < count; i++)
        {
            ListEntry* dirEntry = (ListEntry*)(((HashEntry*)m_dirList->ItemAtFast(i))->m_clvItem);
            HashEntry* parent = m_archiver->Table()->Find(dirEntry->m_dirPath.String());
            if (parent)
                m_listView->AddUnderFast(dirEntry, parent->m_clvItem);
            else
                m_listView->AddItemFastHierarchical(dirEntry);

            dirEntry->m_added = true;
        }

        return i;
    }
}


int32 MainWindow::AddItemsFromList(int32 index, int32* totalItems)
{
    // Add split files at a time, then we will be called again (MessageReceived:M_ADD_ITEMS), Any messages
    // inbetween can be processed this way -- for smaller archives (< split files) add all at once */
    int32 count = m_fileList->CountItems();
    *totalItems = count;
    int32 limit = 0, split = 300;

    if (count >= split * 2)
    {
        if (index + (count / split) > count)
            limit = count;
        else
            limit = index + (count / split);

        for (; index < limit; index++)
        {
            ListEntry* item = ((HashEntry*)m_fileList->ItemAtFast(index))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(item->m_dirPath.String());

            // In case the entry doesn't have a parent folder at all
            if (parentHash)
                m_listView->AddUnderFast(item, parentHash->m_clvItem);
            else
                m_listView->AddItemFastHierarchical(item);

            item->m_added = true;
        }

        // We have added index number of files
        return index;
    }
    else
    {
        int32 i = 0L, c = m_fileList->CountItems();
        for (; i < c; i++)
        {
            ListEntry* item = ((HashEntry*)m_fileList->ItemAtFast(i))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(item->m_dirPath.String());

            // In case the entry doesn't have a parent folder at all
            if (parentHash)
                m_listView->AddUnderFast(item, parentHash->m_clvItem);
            else
                m_listView->AddItemFastHierarchical(item);

            item->m_added = true;
        }

        // We have added all files
        return i;
    }
}


void MainWindow::SetupArchiver(entry_ref* ref, char* mimeString)
{
    // Initialise archiver based either on ref, or on the passed-in mimeString
    m_logTextView->AddText(B_TRANSLATE("Detecting format" B_UTF8_ELLIPSIS), true, false, false);
    char type[B_MIME_TYPE_LENGTH];
    if (ref)
    {
        m_archivePath.SetTo(ref);
        m_archiveEntry.SetTo(ref);

        if (!mimeString)
        {
            update_mime_info(m_archivePath.Path(), false, true, false);
            BNode node(&m_archiveEntry);
            BNodeInfo nodeInfo(&node);
            nodeInfo.GetType(type);
        }
        else
            strcpy(type, mimeString);
    }
    else if (mimeString)
    {
        strcpy(type, mimeString);
    }

    status_t errCode;
    m_archiver = _archiverMgr()->ArchiverForMime(type);

    if (m_archiver == NULL)        // Handle unsupported types
    {
        m_logTextView->AddText(B_TRANSLATE("Failed!"), false, false, false);
        BString errStr(B_TRANSLATE("%filename% is not an archive or is an unsupported type"));
        errStr.ReplaceAll("%filename%", m_archivePath.Leaf());

        BAlert* errAlert = new BAlert("Error", errStr, BZ_TR(kOKString), NULL, NULL,
                                      B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
        errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        errAlert->AddToSubset(this);
        errAlert->Go();

        //PostMessage (M_FILE_CLOSE);
        return;
    }

    m_logTextView->AddText(B_TRANSLATE("Done."), false, false, false);
    m_logTextView->AddText(B_TRANSLATE("Verifying archiver" B_UTF8_ELLIPSIS), true, false, false);
    if ((errCode = m_archiver->InitCheck()) != BZR_DONE)    // Type is supported,
    {
        // but add-on has error initializing
        switch (errCode)
        {
            case BZR_BINARY_MISSING:           // Add-on couldn't trace its binary in the Binaries folder
            {
                m_logTextView->AddText(B_TRANSLATE("Failed!"), false, false, false);
                BString alertStr(B_TRANSLATE("Cannot continue."));
                alertStr << "\n" << B_TRANSLATE("Archiver binary is missing.");
                (new BAlert("Error", alertStr, BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL,
                            B_EVEN_SPACING, B_STOP_ALERT))->Go();

                PostMessage(M_FILE_CLOSE);
                return;
            }

            case BZR_OPTIONAL_BINARY_MISSING:    // Add-on couldn't find a non-critical binary (optional)
            {
                m_logTextView->AddText(B_TRANSLATE("Partially successful."), false, false, false);
                (new BAlert("Error",
                            B_TRANSLATE("Optional binary is missing. Some features may not be available"),
                            BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL,
                            B_EVEN_SPACING, B_INFO_ALERT))->Go();
                break;
            }
        }
    }
    else
        m_logTextView->AddText(B_TRANSLATE("Success."), false, false, false);

    InitArchiver();
}


void MainWindow::InitArchiver()
{
    m_archiver->SetFoldingLevel(m_foldingLevel);
    m_archiver->SetIconList(&(_glob_bitmap_pool->m_iconList));
    m_archiver->SetSettingsDirectoryPath((_bzr()->m_settingsPathStr).String());
    if (m_archiver->NeedsTempDirectory())
        m_archiver->SetTempDirectoryPath(MakeTempDirectory());

    // Remove the "Comments" menu-item if the Archiver's format doesn't support comments
    if (m_archiver->SupportsComment() == false)
        m_mainMenu->m_actionsMenu->RemoveItem(m_mainMenu->m_actionsMenu->FindItem(M_ACTIONS_COMMENT));

    if (m_archiver->SupportsPassword() == false)
    {
        m_mainMenu->m_fileMenu->RemoveItem(m_mainMenu->m_fileMenu->FindItem(M_FILE_PASSWORD));
        // Remove separator item at the end
        m_mainMenu->m_fileMenu->RemoveItems(m_mainMenu->m_fileMenu->CountItems() - 1, 1, true);
    }

    if (m_createMode == false)
    {
        if (_prefs_state.FindBoolDef(kPfRestoreArk, true) == true)
        {
            if (LoadArchiverFromArchive(&m_archiveRef) == NULL)
                // no attribute on the archive file, try to load app defaults
                m_archiver->LoadSettingsMenu();
        }
        else
        {
            BMessage empty;
            m_archiver->BuildMenu(empty);
        }
    }
    else
        m_archiver->LoadSettingsMenu();

    if (m_archiver->SettingsMenu() != NULL && m_createMode == false)
        AddArchiverMenu();
}


void MainWindow::AddArchiverMenu()
{
    if (m_createMode == false)
    {
        m_archiver->SettingsMenu()->AddSeparatorItem();
        m_archiver->SettingsMenu()->AddItem(new BMenuItem(BZ_TR(kResetToDefaultsString), new BMessage(M_RESET_ARK_TO_DEFAULT)));
        m_archiver->SettingsMenu()->AddItem(new BMenuItem(BZ_TR(kSaveAsDefaultsString), new BMessage(M_SAVE_ARK_AS_DEFAULT)));
        m_archiver->SettingsMenu()->AddItem(new BMenuItem(BZ_TR(kSaveToArchiveString), new BMessage(M_SAVE_ARK_TO_ARCHIVE)));
        m_mainMenu->AddItem(m_archiver->SettingsMenu(), m_mainMenu->IndexOf(m_mainMenu->m_toolsMenu) + 1);
    }
}


void MainWindow::OpenArchive()
{
    // Setup the archiver and get mime from ValidateFileType (NULL if its available in the BNodeInfo or
    //     NULL if no mime string or extension was recognized, if no mimestring but extension, then
    //  a mime string from the rules would be returned
    char* mime = m_ruleMgr->ValidateFileType(&m_archivePath);
    SetupArchiver(&m_archiveRef, mime);
    if (mime)
        delete[] mime;
    if (m_archiver == NULL)
        return;

    m_logTextView->AddText(B_TRANSLATE("Loading archive" B_UTF8_ELLIPSIS));
    UpdateIfNeeded();

    BMessage* openMsg = new BMessage('open');
    openMsg->AddPointer(kWindowPtr, (void*)this);
    openMsg->AddPointer(kArchiverPtr, (void*)m_archiver);
    openMsg->AddRef(kRef, &m_archiveRef);

    thread_info tinfo;
    bigtime_t pollTime = 50000;           // time interval to check each time if the thread is still running
    bigtime_t waitTime = pollTime;        // initialize waitTime, this keeps incrementing
    bigtime_t maxWaitTime = 1600000;    // 1.6 seconds before popping status window

    // What we do below looks ugly but its quite nice What happens is , we see if the load
    // process (Open thread) runs for more than "maxWaitTime" usecs, if so we pop-up the status window
    // to keep the mainwindow's drawing still functional and blocking the user from closing the window
    // which would lead to the worker thread not being quit. Otherwise it does a normal open

    m_criticalSection = true;        // Tells QuitRequested() not to grant permission to close window
    m_statusWnd = new StatusWindow(B_TRANSLATE("Preparing to open" B_UTF8_ELLIPSIS), this, B_TRANSLATE("Please wait" B_UTF8_ELLIPSIS), NULL, false);
    thread_id tid = spawn_thread(_opener, "_opener", B_NORMAL_PRIORITY, (void*)openMsg);
    resume_thread(tid);
    while (1)
    {
        status_t isAlive = get_thread_info(tid, &tinfo);
        if (isAlive == B_OK)
            snooze(pollTime);
        else
        {
            // Bug-fix: we can't use m_statusWnd->PostMessage(M_CLOSE) when it is NOT Show()ing
            // therefore quit manually
            if (m_statusWnd->Lock())
            {
                m_statusWnd->Quit();
                m_statusWnd = NULL;
            }
            m_criticalSection = false;
            break;
        }

        waitTime += pollTime;
        if (waitTime >= maxWaitTime)
        {
            m_statusWnd->Show();
            break;
        }
    }
}


void MainWindow::OpenArchivePartTwo(status_t result)
{
    if (m_statusWnd)
        m_statusWnd->PostMessage(M_CLOSE);

    SetBusyState(true);
    AdjustColumns();
    UpdateIfNeeded();

    if (result == BZR_ERRSTREAM_FOUND)
        m_badArchive = true;

    UpdateNewWindow();
    m_criticalSection = false;
}


//
// --- Add Functions ---
//


int32 MainWindow::AddFoldersFromList(BList* folderList, int32 index)
{
    // CALLED ONLY WHEN ADDING FOLDERS TO EXISTING ARCHIVE --
    // THIS IS BECAUSE THIS IS A SLOW ROUTINE WHEN COMPARED TO THE ONE USED WHILE OPENING THE ARCHIVE
    // Add split folders at a time, then we will be called again , Any messages
    // inbetween can be processed this way -- for smaller archives (< split folders) add all at once */
    int32 count = folderList->CountItems();
    int32 limit = 0, split = 60;

    if (count >= split * 2)
    {
        if (index + (count / split) > count)
            limit = count;
        else
            limit = index + (count / split);

        int32 j = index;
        for (; index < limit; index++)
        {
            ListEntry* dirEntry = ((HashEntry*)folderList->ItemAtFast(index))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(dirEntry->m_dirPath.String());
            if (dirEntry->m_added == false)
            {
                if (parentHash)
                    m_listView->AddUnderFast(dirEntry, parentHash->m_clvItem);
                else
                    m_listView->AddItemFastHierarchical(dirEntry);

                dirEntry->m_added = true;
                j++;
            }
        }

        return j;
    }
    else
    {
        int32 i = 0L, j = 0L;
        for (; i < count; i++)
        {
            ListEntry* dirEntry = ((HashEntry*)folderList->ItemAtFast(i))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(dirEntry->m_dirPath.String());
            if (dirEntry->m_added == false)
            {
                if (parentHash)
                    m_listView->AddUnderFast(dirEntry, parentHash->m_clvItem);
                else
                    m_listView->AddItemFastHierarchical(dirEntry);

                dirEntry->m_added = true;
                j++;
            }
        }

        return j;
    }
}


int32 MainWindow::AddItemsFromList(BList* fileList, int32 index)
{
    // CALLED ONLY WHEN ADDING FILES TO EXISTING ARCHIVE --
    // THIS IS BECAUSE THIS IS A SLOW ROUTINE WHEN COMPARED TO THE ONE USED WHILE OPENING THE ARCHIVE
    // Add split files at a time, then we will be called again any messages
    // inbetween can be processed this way -- for smaller archives (< split files) add all at once */
    int32 count = fileList->CountItems();
    int32 limit = 0, split = 300;

    if (count >= split * 2)
    {
        if (index + (count / split) > count)
            limit = count;
        else
            limit = index + (count / split);

        int32 j = index;
        for (; index < limit; index++)
        {
            ListEntry* item = ((HashEntry*)fileList->ItemAtFast(index))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(item->m_dirPath.String());

            if (item->m_added == false)
            {
                // In case the entry doesn't have a parent folder at all
                if (parentHash)
                    m_listView->AddUnderFast(item, parentHash->m_clvItem);
                else
                    m_listView->AddItemFastHierarchical(item);

                item->m_added = true;
                j++;
            }
        }

        // We have added "j" number of files
        return j;
    }
    else
    {
        int32 i = 0L, j = 0L;
        for (; i < count; i++)
        {
            ListEntry* item = ((HashEntry*)fileList->ItemAtFast(i))->m_clvItem;
            HashEntry* parentHash = m_archiver->Table()->Find(item->m_dirPath.String());

            if (item->m_added == false)
            {
                // In case the entry doesn't have a parent folder at all
                if (parentHash)
                    m_listView->AddUnderFast(item, parentHash->m_clvItem);
                else
                    m_listView->AddItemFastHierarchical(item);

                item->m_added = true;
                j++;
            }
        }

        // We have added all files
        return j;
    }
}


bool MainWindow::ConfirmAddOperation(const char* addingUnderPath, BMessage* refsMessage, int32* countL,
                                     int32* skipped)
{
    // Function that return true, if there files are added/appended to archive - in the process
    // what it does is, it confirms overwriting of files/folders, prevents overwriting of file-with-folder
    // and folder-with-file. If all files are SKIPPED (overwriting skipped) it returns false
    // warnType: 0 = Never replace
    //            1 = Ask before replace
    //            2 = Replace without asking
    //            3 = If file being added has newer modified timestamp

    int8 warnType = _prefs_add.FindInt8Def(kPfReplaceFiles, 1);

    BString parentPath;
    if (addingUnderPath)
    {
        parentPath << addingUnderPath;
        parentPath << "/";
    }

    uint32 type;
    int32 count;
    entry_ref ref;

    refsMessage->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return true;

    *skipped = 0;
    *countL = count;
    for (int32 i = --count; i >= 0; i--)
        if (refsMessage->FindRef("refs", i, &ref) == B_OK)
        {
            BString buf = parentPath;
            buf << ref.name;

            HashEntry* hashEntry = m_archiver->Table()->Find(buf.String());
            if (hashEntry)
            {
                BEntry entry(&ref, false);
                BString confirmBufStr;

                char nameBuf[B_FILE_NAME_LENGTH+1];
                entry.GetName(nameBuf);

                bool existingEntrySuper = hashEntry->m_clvItem->IsSuperItem();
                bool addingEntrySuper = entry.IsDirectory();
                bool showError = false;
                if (existingEntrySuper && !addingEntrySuper)        // Trying to add file in place of folder
                {
                    confirmBufStr = B_TRANSLATE("Cannot add \"%name%\" as a folder with the same name already exists.");
                    showError = true;
                }
                else if (!existingEntrySuper && addingEntrySuper)    // Trying to add folder in place of file
                {
                    confirmBufStr = B_TRANSLATE("Cannot add \"%name%\" as a file with the same name already exists.");
                    showError = true;
                }
                else if (existingEntrySuper && addingEntrySuper)    // Replacement of folder
                {
                    if (warnType == 0)    // Never replace folders that already exist, so skip them
                    {
                        refsMessage->RemoveData("refs", i);
                        *skipped = *skipped + 1;
                        continue;
                    }
                    else if (warnType == 2 || warnType == 3)
                    {
                        // Replace without asking (2), for (3) we don't compare time-stamp of folders
                        // so automatically replace as well
                        continue;
                    }

                    // The warnType left is (1) i.e. to ask the user
                    confirmBufStr = B_TRANSLATE("A folder named \"%name%\" already exists.");
                    confirmBufStr << "\n" << B_TRANSLATE("Do you want to replace it?");
                    confirmBufStr.ReplaceAll("%name%", hashEntry->m_clvItem->GetColumnContentText(2));

                    BAlert* confAlert = new BAlert("Confirm", confirmBufStr.String(), BZ_TR(kCancelString),
                                                   B_TRANSLATE("Skip"), B_TRANSLATE("Replace folder"), B_WIDTH_AS_USUAL,
                                                   B_EVEN_SPACING, B_INFO_ALERT);
                    confAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                    confAlert->AddToSubset(this);
                    int32 buttonIndex = confAlert->Go();

                    if (buttonIndex == 0L)
                        return false;
                    else if (buttonIndex == 1L)
                    {
                        refsMessage->RemoveData("refs", i);
                        *skipped = *skipped + 1;
                        continue;
                    }

                }
                else if (!existingEntrySuper && !addingEntrySuper)        // Replacement of file
                {
                    if (warnType == 0)    // Never replace files that already exist, so skip them
                    {
                        refsMessage->RemoveData("refs", i);
                        *skipped = *skipped + 1;
                        continue;
                    }
                    else if (warnType == 2)    // Replace without asking
                        continue;

                    time_t existingEntryTime = hashEntry->m_clvItem->m_timeValue;
                    time_t modTime;
                    tm mod_tm;
                    tm existingTime;
                    off_t size;
                    entry.GetModificationTime(&modTime);
                    entry.GetSize(&size);
                    localtime_r(&modTime, &mod_tm);
                    localtime_r(&existingEntryTime, &existingTime);

                    if (warnType == 3)    // Auto-replace if modTime > existingEntryTime
                    {
                        if (modTime > existingEntryTime)    // means existingFile is older
                            continue;
                        else                             // means existingFile is newer, so skip new file
                        {
                            refsMessage->RemoveData("refs", i);
                            *skipped = *skipped + 1;
                            continue;
                        }
                    }

                    // warnType left is (1) i.e. to ask user
                    char dateTimeBuf[256];
                    char existingTimeBuf[256];
                    strftime(dateTimeBuf, 256, "%b %d %Y, %I:%M:%S %p", &mod_tm);
                    strftime(existingTimeBuf, 256, "%b %d %Y, %I:%M:%S %p", &existingTime);

                    confirmBufStr = B_TRANSLATE("File already exists" B_UTF8_ELLIPSIS);

                    // append ... \n\tname\n\t\t(size, date)
                    confirmBufStr << "\n\t" << hashEntry->m_clvItem->GetColumnContentText(2) << "\n\t\t("
                        << hashEntry->m_clvItem->GetColumnContentText(3) << ", " << existingTimeBuf << ")\n\n";

                    if (m_archiver->CanReplaceFiles() == true)
                        confirmBufStr << B_TRANSLATE("Do you want to replace it with the file you are adding?");
                    else
                        confirmBufStr << B_TRANSLATE("Do you want to append another file with the same name?");

                    // append ... \n\tname\n\t\t(size, date)
                    confirmBufStr << "\n\t" << nameBuf << "\n\t\t(" << StringFromBytes(size).String() << ", " << dateTimeBuf << ")";

                    confirmBufStr.ReplaceAll("\t", "    ");

                    BAlert* confAlert = new BAlert("Confirm", confirmBufStr.String(), BZ_TR(kCancelString),
                                                   B_TRANSLATE("Skip"),
                                                   m_archiver->CanReplaceFiles() ? B_TRANSLATE("Replace file") : B_TRANSLATE("Append file"),
                                                   B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
                    confAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                    confAlert->AddToSubset(this);
                    int32 buttonIndex = confAlert->Go();
                    if (buttonIndex == 0L)
                        return false;
                    else if (buttonIndex == 1L)
                    {
                        refsMessage->RemoveData("refs", i);
                        *skipped = *skipped + 1;
                        continue;
                    }
                }

                if (showError == true)
                {
                    confirmBufStr.ReplaceAll("%name%", nameBuf);

                    BAlert* errAlert = new BAlert("Error", confirmBufStr.String(), BZ_TR(kOKString),
                                                  NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
                    errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                    errAlert->AddToSubset(this);
                    errAlert->Go();
                    *countL = 0L;
                    *skipped = 0L;
                    return false;
                }
            }
        }

    return true;
}


void MainWindow::SetupAddPanel()
{
    if (m_addPanel == NULL)
    {
        m_addPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), 0,
                                    B_DIRECTORY_NODE | B_FILE_NODE | B_SYMLINK_NODE, true,
                                    new BMessage(M_ADD));
        m_addPanel->SetButtonLabel(B_DEFAULT_BUTTON, BZ_TR(kAddString));
        m_addPanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        m_addPanel->Window()->AddToSubset(this);

        const char* addDirPath = _prefs_paths.FindString(kPfDefAddPath);
        if (addDirPath)
            m_addPanel->SetPanelDirectory(addDirPath);

        if (m_archiver && m_archiver->SettingsMenu() && m_createMode == true)
        {
            m_addPanelMenuBar = (BMenuBar*)m_addPanel->Window()->FindView("MenuBar");
            m_addPanelMenuBar->AddItem(m_archiver->SettingsMenu());

            // It seems BMenu::SetTargetForItems() does NOT descent into sub-menus, but unfortunately
            // we need to do this as we are adding this BMenu to BFilePanel which won't handle
            // the menuitem's message (which would be BZR_MENUITEM_SELECTED)
            SetTargetForMenuRecursive(m_archiver->SettingsMenu(), this);
        }

        if (m_archiver && m_archiver->SupportsPassword() && m_addPanel->Window()->LockLooper())
        {
            BButton* button = (BButton*)m_addPanel->Window()->FindView("cancel button");
            BTextControl* pwdText = new BTextControl(BRect(10, button->Frame().top + 2, 10 + 150, 0),
                                                     "pwdText", BZ_TR(kPasswordString),
                                                     NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
            m_addPanel->Window()->ChildAt(0)->AddChild(pwdText);
            pwdText->SetDivider(pwdText->StringWidth(pwdText->Label()) + 1);
            pwdText->TextView()->HideTyping(true);
            pwdText->SetText(m_archiver->Password().String());
            pwdText->TextView()->DisallowChar(B_ESCAPE);
            pwdText->TextView()->DisallowChar(B_INSERT);
            pwdText->ResizeToPreferred();
            m_addPanel->Window()->UnlockLooper();
        }
    }

    BString titleStr = B_TRANSLATE("Add to:");
    titleStr << " " << m_archivePath.Leaf();

    // Tweak the default button (turn it off) so <ENTER> doesn't select the folder, instead
    // enters the folder in the BFilePanel's list
    if (m_addPanel->Window()->Lock())
    {
        m_addPanel->Window()->SetTitle(titleStr.String());

        ((BButton*)m_addPanel->Window()->FindView("default button"))->MakeDefault(false);
        m_addPanel->Window()->Unlock();
    }
}


void MainWindow::AddNewFolder()
{
    // Create a new blank folder and add it to the archive
    if (m_archiveEntry.Exists() == false)
    {
        ShowArkPathError();
        return;
    }

    // Check if archiver can add files
    if (CanAddFiles() == false)
        return;

    // 0.06: Check if the archiver can add empty folders
    if (m_archiver && m_archiver->CanAddEmptyFolders() == false)
    {
        ShowOpNotSupported();
        return;
    }

    // Check if archive is on writeable partition
    if (CanWriteArchive() == false)
        return;

    ListEntry* selectedItem = NULL;
    int32 i = m_listView->CurrentSelection(0L);
    if (i >= 0L)
        selectedItem = (ListEntry*)m_listView->ItemAt(i);

    bool addingAtRoot = false;
    if (!selectedItem)
        addingAtRoot = true;
    else if (selectedItem->IsSuperItem() == false && selectedItem->OutlineLevel() == 0L)
        addingAtRoot = true;

    BString createDirStr = B_TRANSLATE("Create folder in:");
    const char* parentPath = NULL;
    if (addingAtRoot == false)
    {
        if (selectedItem->IsSuperItem() == false)
            parentPath = selectedItem->m_dirPath.String();
        else
            parentPath = selectedItem->m_fullPath.String();
    }
    else
        parentPath = B_TRANSLATE("(root of the archive)");

    createDirStr << "\n" << parentPath << "\n";
    InputAlert* dirAlert = new InputAlert(createDirStr.String(), B_TRANSLATE("New folder name:"), "", false,
                                          BZ_TR(kCancelString), B_TRANSLATE("Create folder"));
    dirAlert->SetDefaultButton(dirAlert->ButtonAt(1L));
    dirAlert->ButtonAt(1L)->SetLabel(B_TRANSLATE("Create folder"));
    dirAlert->TextControl()->TextView()->DisallowChar(':');
    dirAlert->TextControl()->TextView()->DisallowChar('*');
    dirAlert->TextControl()->TextView()->DisallowChar('/');
    dirAlert->TextControl()->TextView()->DisallowChar('\\');
    dirAlert->TextControl()->TextView()->SetMaxBytes(B_FILE_NAME_LENGTH);

    BMessage msg = dirAlert->GetInput(this);

    int32 buttonIndex;
    const char* folderName;
    if (msg.FindInt32(InputAlert::kButtonIndex, &buttonIndex) == B_OK &&
            msg.FindString(InputAlert::kInputText, &folderName) == B_OK)
    {
        if (buttonIndex == 1L)
        {
            m_statusWnd = new StatusWindow(B_TRANSLATE("Creating folder" B_UTF8_ELLIPSIS), this,
                                           B_TRANSLATE("Please wait" B_UTF8_ELLIPSIS), NULL);

            MakeTempDirectory();
            BString mkdirPath = m_tempDirPath;
            mkdirPath << "/";
            BString folderPath;
            if (addingAtRoot == false)
            {
                mkdirPath << parentPath << "/";
                folderPath << parentPath << "/";
            }
            folderPath << folderName;
            mkdirPath << folderName;

            // Make sure folderPath is unique
            HashEntry* hashEntry = m_archiver->Table()->Find(folderPath.String());
            if (hashEntry)    // uh-oh a folder with the same path exists, cancel this operation with
            {
                // an error
                m_statusWnd->PostMessage(M_CLOSE);
                BAlert* errAlert = new BAlert("Error", B_TRANSLATE("Cannot create folder. A folder with this name already exists."),
                                              BZ_TR(kOKString));
                errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                errAlert->AddToSubset(this);
                errAlert->Go();
                return;
            }

            m_logTextView->AddText(B_TRANSLATE("Creating folder" B_UTF8_ELLIPSIS), true, false, false);
            m_logTextView->AddText(" ", false, false, false);

            create_directory(mkdirPath.String(), 0777);
            BEntry dirEntry(mkdirPath.String(), false);
            entry_ref dirRef;
            dirEntry.GetRef(&dirRef);

            BMessage addMsg;
            addMsg.AddString(kPath, folderPath.String());
            BMessage newlyAddedPaths;

            UpdateIfNeeded();
            status_t result = m_archiver->Add(false, m_tempDirPath, &addMsg, &newlyAddedPaths, NULL, NULL);
            UpdateIfNeeded();

            BMessage* doneMsg = new BMessage(M_ADD_DONE);
            doneMsg->AddRef("refs", &dirRef);
            doneMsg->AddPointer(kSuperItem, (void*)selectedItem);
            doneMsg->AddInt32(kResult, result);
            doneMsg->AddMessage(kFileList, &newlyAddedPaths);
            PostMessage(doneMsg);
            m_statusWnd->PostMessage(M_CLOSE);
        }
    }
}


//
// --- State Functions ---
//


void MainWindow::SaveSettingsAsDefaults() const
{
    // Save interface, folding state to settings file
    BMessage msg('bezr');
    GetState(msg);

    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;
    BFile file(path.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
    msg.Flatten(&file);
}


void MainWindow::SaveSettingsToArchive(BMessage* message)
{
    // Save interace, folding state to the archive as an attribute
    BMessage msg('bezr');

    // If no "message" is provided, save the current state
    if (!message)
    {
        GetState(msg);

        // Cache the UI state -- see Quit() for why
        CacheState(&m_cachedUIState, &msg);
    }
    else    // save the state that is passed in "message"
        msg = *message;

    // Proceed to writing state as attributes
    BNode archiveNode(&m_archiveEntry);

    ssize_t msgLength = msg.FlattenedSize();
    char* msgBuf = new char [msgLength];
    if (msg.Flatten(msgBuf, msgLength) == B_OK)
        archiveNode.WriteAttr(K_UI_ATTRIBUTE, B_MESSAGE_TYPE, 0, msgBuf, msgLength);
    delete[] msgBuf;
}


void MainWindow::SaveArchiverToArchive(BMessage* message)
{
    // Save archiver menu to archive -- this function should probably be named
    // SaveArkSettingsToArchive() but anyway ;-)
    BMessage msg('arkv');

    // If no "message" is passed save the current archiver menu
    if (!message)
    {
        m_archiver->ArchiveSettings(msg);

        // Cache the archiver state
        CacheState(&m_cachedArkState, &msg);
    }
    else        // else save the state in "message" that is passed
        msg = *message;

    BNode archiveNode(&m_archiveEntry);
    ssize_t msgLength = msg.FlattenedSize();
    char* msgBuf = new char[msgLength];
    if (msg.Flatten(msgBuf, msgLength) == B_OK)
        archiveNode.WriteAttr(K_ARK_ATTRIBUTE, B_MESSAGE_TYPE, 0, msgBuf, msgLength);
    delete[] msgBuf;
}


BMenu* MainWindow::LoadArchiverFromArchive(entry_ref* ref)
{
    // Load archiver settings from archive's attribute -- this function should probably be named
    // LoadArkSettingsFromArchive but anyway :)
    BMessage msg;
    BNode archiveNode(ref);

    attr_info attribInfo;
    if (archiveNode.GetAttrInfo(K_ARK_ATTRIBUTE, &attribInfo) != B_OK)
        return NULL;

    char* msgBuf = new char [attribInfo.size];
    archiveNode.ReadAttr(K_ARK_ATTRIBUTE, B_MESSAGE_TYPE, 0, msgBuf, attribInfo.size);
    msg.Unflatten(msgBuf);
    delete[] msgBuf;

    // Cache the archiver state
    CacheState(&m_cachedArkState, &msg);

    m_archiver->BuildMenu(msg);

    return m_archiver->SettingsMenu();
}


void MainWindow::LoadSettingsFromArchive(entry_ref* ref)
{
    // This function is usually called even before the window's Show()
    // So that it won't look ugly (resizing the window after showing it)
    BMessage msg;
    BNode archiveNode(ref);

    attr_info attribInfo;
    if (archiveNode.GetAttrInfo(K_UI_ATTRIBUTE, &attribInfo) == B_OK
            && _prefs_state.FindBoolDef(kPfRestoreUI, true) == true)
    {
        char* msgBuf = new char [attribInfo.size];

        archiveNode.ReadAttr(K_UI_ATTRIBUTE, B_MESSAGE_TYPE, 0, msgBuf, attribInfo.size);
        msg.Unflatten(msgBuf);

        // Cache the UI state
        CacheState(&m_cachedUIState, &msg);

        SetState(&msg);
        delete[] msgBuf;
    }
    else
        LoadDefaultSettings();
}


void MainWindow::LoadDefaultSettings()
{
    // Public function as it is called from BApp object
    // Load UI settings file
    BMessage msg;

    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;

    BFile file(path.String(), B_READ_ONLY);
    if (msg.Unflatten(&file) != B_OK)
        return;

    SetState(&msg);
}


void MainWindow::CacheState(BMessage** cache, BMessage* msg)
{
    // Cache the UI state message in a cached member variable
    // The reason why we cache states is because when the user saves the state (UI/archiver) and
    // then does an operation like Add, the saved state (attributes) are blown away (by zip etc)
    // so we cache what we save (if user saves) and resave the cached state when we Quit()
    if (*cache)
        delete *cache;

    *cache = new BMessage(*msg);
}


void MainWindow::SetState(BMessage* msg)
{
    BMessage bufMsg;
    BRect rect;
    bool visible;

    // Restore window size
    if (msg->FindRect(kWindowSize, &rect) == B_OK && rect.IsValid())
    {
        MoveTo(rect.left, rect.top);
        ResizeTo(rect.right - rect.left, rect.bottom - rect.top);
    }

    // Restore toolbar visibilty
    if ((msg->FindBool(kToolBar, &visible) == B_OK) && visible == false)
        ToggleToolBar();

    // Restore infobar visiblity
    if ((msg->FindBool(kInfoBar, &visible) == B_OK) && visible == false)
        ToggleInfoBar();

    // Restore action-log visiblity
    if ((msg->FindBool(kSplitter, &visible) == B_OK) && visible == false)
        ToggleActionLog();

    // Restore folding options
    if (msg->FindInt8(kFolding, &m_foldingLevel) == B_OK)
        m_mainMenu->m_foldingMenu->ItemAt(m_foldingLevel)->SetMarked(true);

    // Restore listview state
    msg->FindMessage(kListViewState, &bufMsg);
    m_listView->SetState(&bufMsg);

    // Restore splitpane state
    msg->FindMessage(kSplitterState, &bufMsg);
    // Don't call m_splitPane->SetState() because it doesn't work correctly when toolbar/infobar is hidden
    // Buf fix: use the asynchornous version for setting the state, works nicely
    PostMessage(&bufMsg, m_splitPane);
    PostMessage(&bufMsg, m_splitPane);  // stupid i know, but only if we call it once again does it work

    // Restore menu-items to match listview state
    // start from 3 because 0, 1 are indent, icon column and 2 is Name column we NEVER hide
    // so start from 3
    int8 nColumns = m_listView->CountColumns();
    for (int8 i = 3; i < nColumns; i++)
        (m_mainMenu->m_columnsSubMenu->ItemAt(i - 2))->SetMarked((m_listView->ColumnAt(i))->IsShown());
}


void MainWindow::GetState(BMessage& msg) const
{
    // Store interface state, folding state to message
    msg.AddBool(kToolBar, m_toolBar->IsShown());
    msg.AddBool(kInfoBar, m_infoBar->IsShown());
    msg.AddBool(kSplitter, !m_logHidden);
    msg.AddRect(kWindowSize, Frame());
    msg.AddInt8(kFolding, m_mainMenu->m_foldingMenu->IndexOf(m_mainMenu->m_foldingMenu->FindMarked()));

    BMessage splitterMsg(SPLITPANE_STATE);
    m_splitPane->GetState(splitterMsg);
    msg.AddMessage(kSplitterState, &splitterMsg);

    BMessage listMsg('list');
    m_listView->GetState(listMsg);
    msg.AddMessage(kListViewState, &listMsg);
}


//
// --- Thread Functions ---
//


int32 MainWindow::_viewer(void* arg)
{
    MainWindow* wnd(NULL);
    BWindow* statusWnd(NULL);
    Archiver* ark(NULL);
    entry_ref tempDirRef;
    const char* tempPath;
    volatile bool* cancel;

    BMessage* message = reinterpret_cast<BMessage*>(arg);
    message->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    message->FindPointer(kStatusPtr, reinterpret_cast<void**>(&statusWnd));
    message->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    message->FindPointer(kCancel, (void**)&cancel);
    tempPath = message->FindString(kTempPath);
    message->FindRef(kRef, &tempDirRef);

    uint32 type;
    int32 count;
    entry_ref ref;
    const char* entryPath;

    message->GetInfo(kPath, &type, &count);
    if (type != B_STRING_TYPE)
        return B_ERROR;

    // Extract all files in this message to temp and view/open with them
    for (int32 i = --count; i >= 0; i--)
        if (message->FindString(kPath, i, &entryPath) == B_OK)
        {
            if (*cancel == true)
                break;

            BMessage msg('msg_');
            msg.AddString(kPath, entryPath);

            if (wnd->LockLooper())
            {
                wnd->m_logTextView->AddText(B_TRANSLATE("Viewing:"), true);
                wnd->m_logTextView->AddText(entryPath, false, false, false);
                wnd->UnlockLooper();
            }

            status_t result = ark->Extract(&tempDirRef, &msg, NULL, cancel);
            bool fileWasExtracted = false;
            if (*cancel == false && result == BZR_DONE)
            {
                fileWasExtracted = OpenEntry(tempPath, entryPath,
                                             message->what == M_ACTIONS_OPEN_WITH ? true : false);
            }

            // Possibly a password error, report it to the MainWindow
            if (fileWasExtracted == false && ark->SupportsPassword() == true && result == BZR_PASSWORD_ERROR)
            {
                BMessage errorMsg(M_EXTRACT_DONE);
                errorMsg.AddInt32(kResult, BZR_PASSWORD_ERROR);
                errorMsg.AddBool(kFailOnNull, true);
                if (wnd->LockLooper())
                {
                    wnd->ExtractDone(&errorMsg);
                    wnd->UnlockLooper();
                }
            }
        }

    delete message;
    statusWnd->PostMessage(M_CLOSE);

    return BZR_DONE;
}


int32 MainWindow::_counter(void* arg)
{
    // Thread that does getting information about a directory
    // such as number of files, folders and size of directory etc.
    // We have put this up in a thread so that for large directories on slower system it will
    // keep the interface responsive
    MainWindow* wnd(NULL);
    BWindow* statusWnd(NULL);
    volatile bool* cancel;

    BMessage* message = reinterpret_cast<BMessage*>(arg);
    message->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    message->FindPointer(kCancel, (void**)&cancel);
    message->FindPointer(kStatusPtr, reinterpret_cast<void**>(&statusWnd));

    uint32 type;
    int32 count;
    entry_ref ref;
    message->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return B_ERROR;

    int32 fileCount(0), folderCount(0);
    off_t totalSize(0);
    for (int32 i = --count; i >= 0; i--)
        if (message->FindRef("refs", i, &ref) == B_OK)
        {
            BEntry entry(&ref, false);         // Do NOT traverse links
            if (entry.IsDirectory() == true)
                GetDirectoryInfo(&entry, fileCount, folderCount, totalSize, cancel);
            else
            {
                off_t size;
                entry.GetSize(&size);
                totalSize += size;
                fileCount++;
            }
        }

    // Remove the following fields from message as they will be re-add (when message is re-used)
    // and that time the Add... WILL fail unless we remove it
    message->RemoveName(kWindowPtr);
    message->RemoveName(kCancel);

    // Don't delete message here as it doesn't belong to us - it is passed
    // from MessageReceived()'s M_ADD segment which doesn't create the message it simply
    // passes the argument in MessageReceived()
    message->what = M_COUNT_COMPLETE;                  // See we are reusing the message
    message->AddInt32(kFiles, fileCount);                // Add the critical fields as that is what
    message->AddInt32(kFolders, folderCount);            // we are here for in the first place
    message->AddInt64(kSize, totalSize);
    statusWnd->PostMessage(M_CLOSE);
    if (*cancel == false)
        wnd->PostMessage(message);

    return BZR_DONE;
}


int32 MainWindow::_copier(void* arg)
{
    // Copies files to kLaunchDir directory before adding to the archive
    // This is done so that files/folder can be added to any dir inside the archive
    // as we will be launching archive from kLaunchDir
    BMessage* msg = reinterpret_cast<BMessage*>(arg);
    status_t result = BZR_DONE;
    volatile bool* cancel;
    BMessenger messenger;
    BWindow* wnd(NULL);

    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindMessenger(kProgressMessenger, &messenger);
    msg->FindPointer(kCancel, (void**)&cancel);
    const char* dirInArchive = msg->FindString(kSuperItemPath);
    const char* tempDir = msg->FindString(kTempPath);

    // Now we must copy the refs to the launchDir folder
    uint32 type;
    int32 count;
    entry_ref ref;

    msg->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return B_ERROR;

    BDirectory destDir(tempDir);

    for (int32 i = --count; i >= 0; i--)
        if (msg->FindRef("refs", i, &ref) == B_OK)
        {
            if (cancel && *cancel) break;

            BEntry entry(&ref, false);         // Do NOT TRAVERSE LINKS
            if (entry.IsDirectory() == true)
                result = CopyDirectory(&entry, &destDir, &messenger, cancel);
            else
                result = CopyFile(&entry, &destDir, &messenger, cancel);

            // Make message have new relative paths as THOSE are the refs that will
            // be added to the archive as they are copied now to a temp dir
            BString relPath = dirInArchive;
            relPath << '/' << ref.name;
            msg->AddString(kPath, relPath.String());
        }

    messenger.SendMessage(M_CLOSE);

    // don't delete msg yet, use it in the back message
    // Don't delete kLaunchDir as M_READY_TO_ADD of MessageReceived needs it
    // Remove the following fields form message as it will be re-added as it is going to be
    // reused and at that time the add will fail
    msg->RemoveName(kWindowPtr);
    msg->RemoveName(kProgressMessenger);
    msg->RemoveName(kCancel);
    msg->what = M_READY_TO_ADD;
    msg->AddInt32(kResult, result);
    wnd->PostMessage(msg);

    return B_OK;
}


int32 MainWindow::_extractor(void* arg)
{
    // Thread that does extraction
    volatile bool* cancel;
    entry_ref refToDir;
    status_t result;
    BMessenger messenger;
    BMessage selection;
    BWindow* wnd(NULL);
    Archiver* ark(NULL);
    bool allFiles;
    BMessage* msg = reinterpret_cast<BMessage*>(arg);

    msg->FindRef(kRef, &refToDir);
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindBool(kAllFiles, &allFiles);
    msg->FindMessenger(kProgressMessenger, &messenger);

    if (allFiles == false)
    {
        msg->FindMessage(kFileList, &selection);
        result = ark->Extract(&refToDir, &selection, &messenger, cancel);
    }
    else
        result = ark->Extract(&refToDir, NULL, &messenger, cancel);

    messenger.SendMessage(M_CLOSE);
    BMessage backMessage(M_EXTRACT_DONE);
    backMessage.AddInt32(kResult, result);
    backMessage.AddRef(kRef, &refToDir);

    delete msg;
    wnd->PostMessage(&backMessage);

    return result;
}


int32 MainWindow::_adder(void* arg)
{
    // Thread that does adding of files to archive
    volatile bool* cancel;
    status_t result;
    BMessenger messenger;
    BMessage filesToAdd;
    MainWindow* wnd(NULL);
    Archiver* ark(NULL);
    BMessage* msg = reinterpret_cast<BMessage*>(arg);
    const char* relativePath(NULL);
    bool createMode;

    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindMessenger(kProgressMessenger, &messenger);
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindString(kLaunchDir, &relativePath);
    if (msg->FindBool(kCreateMode, &createMode) != B_OK)
        createMode = false;

    BMessage newlyAddedPaths;
    if (createMode == true)
        result = ark->Create(&(wnd->m_archivePath), relativePath, msg, &newlyAddedPaths, &messenger, cancel);
    else
        result = ark->Add(false, relativePath, msg, &newlyAddedPaths, &messenger, cancel);

    msg->what = M_ADD_DONE;
    msg->RemoveName(kResult);
    msg->AddInt32(kResult, result);
    msg->RemoveName(kPath);
    msg->AddMessage(kFileList, &newlyAddedPaths);
    messenger.SendMessage(M_CLOSE);
    wnd->PostMessage(msg);

    return result;
}


int32 MainWindow::_deletor(void* arg)
{
    // Thread that does the deleting of files from archive
    volatile bool* cancel;
    BWindow* wnd = NULL;
    Archiver* ark = NULL;
    BMessenger messenger;
    BMessage selection;
    BMessage* msg = reinterpret_cast<BMessage*>(arg);

    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindMessenger(kProgressMessenger, &messenger);
    msg->FindMessage(kFileList, &selection);

    char* outputStr = NULL;
    status_t result = ark->Delete(outputStr, &selection, &messenger, cancel);

    messenger.SendMessage(M_CLOSE);
    BMessage backMessage(M_DELETE_DONE);
    backMessage.AddInt32(kResult, result);
    if (outputStr)
        backMessage.AddString(kText, outputStr);

    delete msg;
    wnd->PostMessage(&backMessage);

    return result;
}


int32 MainWindow::_tester(void* arg)
{
    // Thread that does the testing
    volatile bool* cancel;
    BWindow* wnd = NULL;
    Archiver* ark = NULL;
    BMessenger messenger;
    BMessage* msg = reinterpret_cast<BMessage*>(arg);

    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindMessenger(kProgressMessenger, &messenger);

    char* outputStr = NULL;
    status_t result = ark->Test(outputStr, &messenger, cancel);

    messenger.SendMessage(M_CLOSE);
    BMessage backMessage(M_TEST_DONE);
    backMessage.AddInt32(kResult, result);
    if (outputStr)
        backMessage.AddString(kText, outputStr);

    delete msg;
    wnd->PostMessage(&backMessage);

    return result;
}


int32 MainWindow::_opener(void* arg)
{
    // Thread that does the opening
    BWindow* wnd = NULL;
    Archiver* ark = NULL;
    entry_ref ref;

    BMessage* msg = reinterpret_cast<BMessage*>(arg);
    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindRef(kRef, &ref);

    status_t result = ark->Open(&ref);
    delete msg;

    BMessage backMessage(M_OPEN_PART_TWO);
    backMessage.AddInt32(kResult, result);
    wnd->PostMessage(&backMessage);
    return result;
}


//
// --- Miscellaneous ---
//


void MainWindow::EditComment(bool failIfNoComment)
{
    if (m_archiver && m_archiver->SupportsComment() == true)
    {
        char* commentStr = NULL;
        m_archiver->GetComment(commentStr);

        if (failIfNoComment == false || (commentStr && strlen(commentStr) > 0L))
            new CommentWindow(this, Title(), commentStr, (BFont*)be_fixed_font);

        delete[] commentStr;
    }
}


const char* MainWindow::MakeTempDirectory()
{
    if (m_tempDir == NULL)
        m_tempDirPath = strdup(CreateTempDirectory(NULL, &m_tempDir, true).String());

    return m_tempDirPath;
}


void MainWindow::EmptyListViewIfNeeded()
{
    if (m_archiver->CanPartiallyOpen() == false)
    {
        int32 count = m_listView->FullListCountItems();
        for (int32 i = 0; i < count; i++)
            delete m_listView->RemoveItem((int32)0);
    }
}


void MainWindow::ShowArkPathError() const
{
    m_logTextView->AddText(B_TRANSLATE("Couldn't initialize archive path."), true, true, true);
    BAlert* errAlert = new BAlert("Error",
                                  B_TRANSLATE("Couldn't initialize archive path.  The location of the archive has been changed.  Either the archive was deleted or moved to another location."),
                                  BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
    errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
    errAlert->AddToSubset((BWindow*)this);
    errAlert->Go();
    return;
}


void MainWindow::ShowOpNotSupported() const
{
    m_logTextView->AddText(B_TRANSLATE("Operation unavailable."), true, false, false);
    BAlert* errAlert = new BAlert("Cannot operate", B_TRANSLATE("The archiver does not support this operation."),
                                  BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
    errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
    errAlert->AddToSubset((BWindow*)this);
    errAlert->Go();
}


void MainWindow::ShowReadOnlyError() const
{
    BAlert* errAlert = new BAlert("Cannot operate", B_TRANSLATE("The archive is read-only, or is in a read-only partition."),
                                  BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
    errAlert->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
    errAlert->AddToSubset((BWindow*)this);
    errAlert->Go();
}


bool MainWindow::CanAddFiles() const
{
    // Return true if the archiver supports adding of files (eg: not possible for pure gzip, bzip2)
    if (m_archiver && m_archiver->CanAddFiles() == false)
    {
        ShowOpNotSupported();
        return false;
    }

    return true;
}


bool MainWindow::CanWriteArchive() const
{
    // Since this is ALSO called while creating, don't flag error if archive doesn't exist
    if (m_archiveEntry.Exists() == true)
    {
        // Check if the archive is on a writeable partition
        BVolume volume;
        status_t err = volume.SetTo(m_archiveRef.device);
        if (err == B_OK && volume.IsReadOnly() == true)
        {
            ShowReadOnlyError();
            return false;
        }
    }

    return true;
}


void MainWindow::AddDynamicExtractPaths(BMenu* menu) const
{
    if (!m_archiver)
        return;

    int32 addedCount = 0L;
    BString bufStr;
    BPath bufPath = m_archivePath.Path();
    bufPath.GetParent(&bufPath);
    bufStr = bufPath.Path();

    BMessage* msg = new BMessage(M_RECENT_EXTRACT_ITEM);
    msg->AddBool(kDynamic, true);     // dynamic prevents it from going to recent-extract-paths
    menu->AddItem(new BMenuItem(bufStr.String(), msg), 0);
    addedCount ++;

    bufStr << '/' << StrippedArchiveName();
    menu->AddItem(new BMenuItem(bufStr.String(), new BMessage(*msg)), 1);
    addedCount++;

    if (menu->CountItems() > addedCount)
        menu->AddItem(new BSeparatorItem(), addedCount);
}


void MainWindow::AddFavouriteExtractPaths(BMenu* menu) const
{
    bool genPath = _prefs_paths.FindBoolDef(kPfGenPath, true);
    BString archiveName;
    if (genPath)
        archiveName = StrippedArchiveName();

    BMessage favPathMsg;
    int32 addedCount = 0L;
    if (_prefs_paths.FindMessage(kPfFavPathsMsg, &favPathMsg) == B_OK)
    {
        int32 i = 0L, addIndex = 0L;
        const char* foundPath(NULL);
        while (favPathMsg.FindString(kPath, i++, &foundPath) == B_OK)
        {
            BMessage* msg = new BMessage(M_RECENT_EXTRACT_ITEM);
            msg->AddBool(kDynamic, true);     // dynamic prevents it from going to recent-extract-paths
            menu->AddItem(new BMenuItem(foundPath, msg), addIndex++);
            addedCount++;

            if (genPath == true && archiveName.Length() > 0)
            {
                BString bufStr = foundPath;
                bufStr << '/' << archiveName;
                menu->AddItem(new BMenuItem(bufStr.String(), new BMessage(*msg)), addIndex++);
                addedCount++;
            }
        }

        if (addedCount > 0 && menu->CountItems() > addedCount)
            menu->AddItem(new BSeparatorItem(), addedCount);
    }
}


BString MainWindow::StrippedArchiveName() const
{
    // Returns the name of the archive without extension
    BString archiveName = m_archivePath.Leaf();
    if (archiveName.IFindLast(m_archiver->ArchiveExtension()) > 0)
        archiveName.RemoveLast(m_archiver->ArchiveExtension());

//    // Try if there are any other extensions left, for example abc.pkg.zip, in which case
//    // we must take away the ".pkg" part as well
//    // The problem is if the user opens say "mozilla-0.2.3.zip" instead of returning "mozilla-0.2.3"
//    // we would be returning "mozilla-"
//    while (true)
//    {
//        int32 found = archiveName.FindLast (".");
//        if (found > 0)
//           archiveName.Remove (found, archiveName.Length() - found);
//        else
//           break;
//    }

    // New from 0.05
    // New system where we get supported file extensions from Beezer binary and then
    // compare against that
    size_t size;
    BMessage msg;
    const char* buf = NULL;
    buf = reinterpret_cast<const char*>(be_app->AppResources()->LoadResource(B_MESSAGE_TYPE,
                                        "BEOS:FILE_TYPES", &size));
    msg.Unflatten(buf);

    type_code type;
    int32 found(-1), count;

    char* name;
    msg.GetInfo(B_STRING_TYPE, 0L, &name, &type, &count);

    BString mimeTypeStr;
    for (int32 i = --count; i >= 0; i--)
    {
        if (msg.FindString(name, i, &mimeTypeStr) == B_OK)
        {
            BMimeType mimeType(mimeTypeStr.String());
            BMessage fileExtensions;
            mimeType.GetFileExtensions(&fileExtensions);

            uint32 k = 0;
            BString extension;
            bool foundExt = false;
            for (;;)
            {
                if (fileExtensions.FindString("extensions", k++, &extension) != B_OK)
                    break;

                if (extension.ByteAt(0) != '.')
                    extension.Prepend('.', 1L);

                found = archiveName.IFindLast(extension);
                if (found > 1)
                {
                    archiveName.Remove(found, extension.Length());
                    //archiveName.RemoveLast (extension);
                    foundExt = true;
                    break;
                }
            }

            if (foundExt == true)
                break;
        }
    }

    // Some generic hardcoded extensions -- grr!! System don't define these
    // neither do the archivers
    int32 extraExtCount = 5;
    BString extraExts [] =
    {
        ".pkg",           // 0
        ".jar",           // 1
        ".tgz",           // 2
        ".tbz",           // 3
        ".tbz2"           // 4
    };

    for (int32 i = 0; i < extraExtCount; i++)
    {
        found = archiveName.IFindLast(extraExts[i]);
        if (found > 1)
            archiveName.Remove(found, extraExts[i].Length());
    }

    return archiveName;
}
