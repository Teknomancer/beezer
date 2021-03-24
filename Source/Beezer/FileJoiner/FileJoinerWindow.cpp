/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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

#include <Application.h>
#include <Font.h>
#include <Bitmap.h>
#include <interface/StringView.h>
#include <TextControl.h>
#include <Menu.h>
#include <MenuField.h>
#include <String.h>
#include <Button.h>
#include <FilePanel.h>
#include <Entry.h>
#include <Path.h>
#include <CheckBox.h>
#include <Alert.h>
#include <StatusBar.h>

#include <stdlib.h>
#include <stdio.h>

#include "FileJoinerWindow.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "UIConstants.h"
#include "MsgConstants.h"
#include "StaticBitmapView.h"
#include "LocalUtils.h"
#include "SelectDirPanel.h"
#include "FSUtils.h"
#include "RecentMgr.h"

#include "Joiner.h"
#include "Shared.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FileJoinerWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


FileJoinerWindow::FileJoinerWindow(RecentMgr* dirs)
    : BWindow(BRect(10, 10, 520, 300), B_TRANSLATE("File Joiner"), B_TITLED_WINDOW,
              B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
    m_filePanel(NULL),
    m_dirPanel(NULL),
    m_calcSize(false),
    m_joinInProgress(false),
    m_quitNow(false),
    m_cancel(false),
    m_recentSplitDirs(dirs)
{
    m_backView = new BevelView(Bounds(), "FileJoinerWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    AddChild(m_backView);

    BFont font(be_plain_font);
    font_height fntHt;

    font.GetHeight(&fntHt);
    float normFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    font.SetFace(B_BOLD_FACE);
    font.GetHeight(&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

    BBitmap* splitBmp = BitmapPool::LoadAppVector("Img:FileJoiner", 32, 32);

    BevelView* sepView1 = new BevelView(BRect(-1, splitBmp->Bounds().Height() + 4 * K_MARGIN,
                                        Bounds().right - 1.0, splitBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                        "FileJoinerWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(sepView1);

    StaticBitmapView* splitBmpView = new StaticBitmapView(BRect(K_MARGIN * 5, K_MARGIN * 2,
            splitBmp->Bounds().Width() + K_MARGIN * 5,
            splitBmp->Bounds().Height() + K_MARGIN * 2), "FileJoinerWindow:splitBmpView",
            splitBmp);
    splitBmpView->SetViewColor(m_backView->ViewColor());
    AddChild(splitBmpView);

    // Add the file name string view (align it vertically with the icon view)
    m_descStr = new BStringView(BRect(splitBmpView->Frame().right + K_MARGIN * 3,
                                      splitBmpView->Frame().top, Bounds().right - 1,
                                      splitBmpView->Frame().top + totalFontHeight),
                                "FileJoinerWindow:DescStr",
                                B_TRANSLATE("This tool lets you join several files into a single file."),
                                B_FOLLOW_LEFT, B_WILL_DRAW);

    m_backView->AddChild(m_descStr);
    m_descStr->MoveTo(m_descStr->Frame().left,
                      (splitBmpView->Frame().Height() / 2 - normFontHeight / 2) + normFontHeight / 2 + 1);
    m_descStr->MoveBy(0, -normFontHeight / 2 - 1);
    m_descStr->ResizeToPreferred();

    m_descStr2 = new BStringView(BRect(m_descStr->Frame().left,
                                       m_descStr->Frame().bottom + 1, Bounds().right - 1, 0),
                                 "FileJoinerWindow:DescStr2",
                                 B_TRANSLATE("The order of joining is based on number suffixes in the filenames…"),
                                 B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(m_descStr2);
    m_descStr2->ResizeToPreferred();

    m_innerView = new BevelView(BRect(K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                                      Bounds().right - K_MARGIN,
                                      Bounds().bottom - K_MARGIN), "FileJoinerWindow:InnerView", btNoBevel,
                                B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(m_innerView);

    m_filePathView = new BTextControl(BRect(K_MARGIN, K_MARGIN,
                                            m_innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                      "FileJoinerWindow:FilePathView", B_TRANSLATE("First file piece:"), NULL, NULL,
                                      B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_innerView->AddChild(m_filePathView);
    m_filePathView->SetModificationMessage(new BMessage(M_UPDATE_DATA));

    m_selectFileBtn = new BButton(BRect(m_innerView->Frame().Width() - K_MARGIN - K_BUTTON_WIDTH,
                                        m_filePathView->Frame().top - 4, m_innerView->Frame().Width() - K_MARGIN,
                                        m_filePathView->Frame().top - 4 + K_BUTTON_HEIGHT), "FileJoinerWindow:SelectFileBtn",
                                  B_TRANSLATE("Select…"), new BMessage(M_SELECT_JOIN_FILE), B_FOLLOW_RIGHT,
                                  B_WILL_DRAW | B_NAVIGABLE);
    m_innerView->AddChild(m_selectFileBtn);

    m_folderMenu = new BMenu(B_TRANSLATE("Folder for output file:"));
    m_folderField = new BMenuField(BRect(K_MARGIN, m_filePathView->Frame().bottom + K_MARGIN,
                                         K_MARGIN + m_innerView->StringWidth(B_TRANSLATE("Folder for output file:")) + 36, 0),
                                   "FileJoinerWindow:FolderField", NULL, m_folderMenu);
    m_innerView->AddChild(m_folderField);

    m_folderPathView = new BTextControl(BRect(m_folderField->Frame().right + K_MARGIN + 1,
                                        m_filePathView->Frame().bottom + K_MARGIN + 2,
                                        m_innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                        "FileJoinerWindow:FolderPathView", NULL, NULL, NULL,
                                        B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_innerView->AddChild(m_folderPathView);
    m_folderPathView->SetModificationMessage(new BMessage(M_UPDATE_DATA));

    m_selectFolderBtn = new BButton(BRect(m_innerView->Frame().Width() - K_MARGIN - K_BUTTON_WIDTH,
                                          m_folderPathView->Frame().top - 4, m_innerView->Frame().Width() - K_MARGIN,
                                          m_folderPathView->Frame().top - 4 + K_BUTTON_HEIGHT),
                                    "FileJoinerWindow:SelectFolderBtn", B_TRANSLATE("Select…"),
                                    new BMessage(M_SELECT_JOIN_FOLDER), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_innerView->AddChild(m_selectFolderBtn);

    float divider;
    divider = MAX(K_MARGIN + m_innerView->StringWidth(m_folderPathView->Label()),
                  m_folderField->Frame().right);
    divider = MAX(divider, K_MARGIN + m_innerView->StringWidth(B_TRANSLATE("File number separator:")));
    divider = MAX(divider, K_MARGIN + m_backView->StringWidth(B_TRANSLATE("Number of pieces:")));
    m_filePathView->SetDivider(divider);
    m_folderPathView->SetDivider(0);
    m_folderPathView->MoveTo(m_filePathView->Frame().left + divider + 1, m_folderPathView->Frame().top);

    m_separatorView = new BTextControl(BRect(K_MARGIN, m_folderPathView->Frame().bottom + K_MARGIN + 2,
                                       K_MARGIN + divider + 50, 0), "FileJoinerWindow:SeparatorView",
                                       B_TRANSLATE("File number separator:"), "_", NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_separatorView->SetDivider(divider + 1);
    m_separatorView->TextView()->SetMaxBytes(128);
    m_separatorView->TextView()->DisallowChar('0');     // too lazy to loop using ASCII value :)
    m_separatorView->TextView()->DisallowChar('1');
    m_separatorView->TextView()->DisallowChar('2');
    m_separatorView->TextView()->DisallowChar('3');
    m_separatorView->TextView()->DisallowChar('4');
    m_separatorView->TextView()->DisallowChar('5');
    m_separatorView->TextView()->DisallowChar('6');
    m_separatorView->TextView()->DisallowChar('7');
    m_separatorView->TextView()->DisallowChar('8');
    m_separatorView->TextView()->DisallowChar('9');
    m_separatorView->TextView()->DisallowChar(':');
    m_separatorView->TextView()->DisallowChar('/');
    m_separatorView->TextView()->DisallowChar('\\');
    m_separatorView->TextView()->DisallowChar('*');
    m_separatorView->TextView()->DisallowChar('?');
    m_separatorView->SetModificationMessage(new BMessage(M_SEPARATOR_CHANGED));
    m_innerView->AddChild(m_separatorView);

    BevelView* sepView2 = new BevelView(BRect(m_separatorView->Frame().right + 4 * K_MARGIN + 2,
                                        m_separatorView->Frame().top - 2,
                                        m_separatorView->Frame().right + 4 * K_MARGIN + 3, 0),
                                        "FileJoinerWindow:sepView2", btInset, B_FOLLOW_LEFT, B_WILL_DRAW);
    m_innerView->AddChild(sepView2);

    m_openChk = new BCheckBox(BRect(sepView2->Frame().left + 3 * K_MARGIN, m_separatorView->Frame().top, 0, 0),
                              "FileJoinerwindow:OpenChk", B_TRANSLATE("Open destination folder after joining"),
                              NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_openChk->ResizeToPreferred();
    m_innerView->AddChild(m_openChk);
    m_openChk->SetValue(B_CONTROL_ON);

    m_closeChk = new BCheckBox(BRect(m_openChk->Frame().left, m_openChk->Frame().bottom + 1, 0, 0),
                               "FileJoinerWindow:CloseChk", B_TRANSLATE("Close window after joining"), NULL, B_FOLLOW_LEFT,
                               B_WILL_DRAW | B_NAVIGABLE);
    m_closeChk->ResizeToPreferred();
    m_innerView->AddChild(m_closeChk);
    m_closeChk->SetValue(B_CONTROL_ON);

    m_deleteChk = new BCheckBox(BRect(m_closeChk->Frame().left, m_closeChk->Frame().bottom + 1, 0, 0),
                                "FileJoinerWindow:DeleteChk", B_TRANSLATE("Delete pieces after joining them"), NULL, B_FOLLOW_LEFT,
                                B_WILL_DRAW | B_NAVIGABLE);
    m_deleteChk->ResizeToPreferred();
    m_innerView->AddChild(m_deleteChk);
    m_deleteChk->SetValue(B_CONTROL_OFF);

    sepView2->ResizeTo(sepView2->Frame().Width(), m_deleteChk->Frame().bottom + 2 - sepView2->Frame().top);

    m_innerView->ResizeTo(m_innerView->Frame().Width(), m_deleteChk->Frame().bottom + K_MARGIN);

    BevelView* sepView3 = new BevelView(BRect(-1, m_innerView->Frame().bottom + K_MARGIN + 1,
                                        Bounds().right - 1.0, m_innerView->Frame().bottom + K_MARGIN + 1 + 1),
                                        "FileJoinerWindow:SepView2", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(sepView3);


    BStringView* noPiecesStr = new BStringView(BRect(2 * K_MARGIN, sepView3->Frame().bottom + 2 * K_MARGIN,
            2 * K_MARGIN + m_backView->StringWidth(B_TRANSLATE("Number of pieces:")) + 3, 0),
            "FileJoinerWindow:noPiecesStr", B_TRANSLATE("Number of pieces:"), B_FOLLOW_LEFT,
            B_WILL_DRAW);
    m_backView->AddChild(noPiecesStr);
    noPiecesStr->ResizeToPreferred();

    m_piecesStr = new BStringView(BRect(m_folderPathView->Frame().left + K_MARGIN, noPiecesStr->Frame().top,
                                        Bounds().right - K_MARGIN, 0), "FileJoinerWindow:PiecesStr", "-", B_FOLLOW_LEFT,
                                  B_WILL_DRAW);
    m_backView->AddChild(m_piecesStr);
    m_piecesStr->ResizeToPreferred();

    BStringView* sizeStr = new BStringView(BRect(noPiecesStr->Frame().left,
                                           noPiecesStr->Frame().bottom + K_MARGIN - 1, 0, 0), "FileJoinerWindow:SizeStr",
                                           B_TRANSLATE("Output file size:"), B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(sizeStr);
    sizeStr->ResizeToPreferred();

    m_sizeStr = new BStringView(BRect(m_piecesStr->Frame().left, sizeStr->Frame().top, 0, 0),
                                "FileJoinerWindow:SizeStr", "-", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_sizeStr->ResizeToPreferred();
    m_backView->AddChild(m_sizeStr);

    m_refreshBtn = new BButton(BRect(m_piecesStr->Frame().right + m_backView->StringWidth("9999999999"),
                                     m_piecesStr->Frame().top + K_MARGIN,
                                     m_piecesStr->Frame().right + m_backView->StringWidth("9999999999") + K_MARGIN +
                                     K_BUTTON_WIDTH, m_piecesStr->Frame().top + K_MARGIN + K_BUTTON_HEIGHT),
                               "FileJoinerWindow:RefreshBtn", B_TRANSLATE("Refresh"), new BMessage(M_REFRESH_INFO),
                               B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(m_refreshBtn);
    m_refreshBtn->SetEnabled(false);

    m_joinBtn = new BButton(BRect(Bounds().right - 2 * K_MARGIN - K_BUTTON_WIDTH - 3,
                                  m_refreshBtn->Frame().top, Bounds().right - 2 * K_MARGIN - 3,
                                  m_refreshBtn->Frame().top + K_BUTTON_HEIGHT),
                            "FileJoinerWindow:JoinBtn", B_TRANSLATE("Join"), new BMessage(M_JOIN_NOW),
                            B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(m_joinBtn);
    m_joinBtn->MakeDefault(true);
    m_joinBtn->SetEnabled(false);

    BevelView* sepView4 = new BevelView(BRect(-1, m_sizeStr->Frame().bottom + K_MARGIN,
                                        Bounds().right - 1.0, m_sizeStr->Frame().bottom + K_MARGIN + 1),
                                        "FileJoinerWindow:SepView4", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(sepView4);
    sepView4->Hide();

    m_statusBar = new BStatusBar(BRect(3 * K_MARGIN, sepView4->Frame().bottom + 2 * K_MARGIN + 1,
                                       Bounds().right - 8 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                 "FileSplitterWindow:StatusBar", NULL, NULL);
    m_statusBar->ResizeToPreferred();
    m_statusBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT);
    m_statusBar->ResizeTo(m_joinBtn->Frame().right - 4 * K_MARGIN, m_statusBar->Frame().Height());
    m_statusBar->SetBarHeight(K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor(K_PROGRESS_COLOR);
    m_backView->AddChild(m_statusBar);

    m_hideProgress = sepView4->Frame().top + K_MARGIN;
    m_showProgress =  m_statusBar->Frame().bottom + K_MARGIN;
    ResizeTo(Frame().Width(), m_hideProgress);

    // Calculate minimum width of window dynamically
    float maxWidth = MAX(m_openChk->Frame().right, m_deleteChk->Frame().right);
    maxWidth = MAX(maxWidth, m_descStr2->Frame().right);
    maxWidth = MAX(maxWidth, m_descStr->Frame().right);
    maxWidth += 2 * K_MARGIN;
    maxWidth = MAX(maxWidth, 520);

    // Center window on-screen
    CenterOnScreen();

    // Constrain window size
    float minH, maxH, minV, maxV;
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    SetSizeLimits(maxWidth, maxH, minV, maxV);
    if (Frame().Width() < maxWidth)
        ResizeTo(maxWidth, Frame().Height());

    // Set focus control
    m_filePathView->MakeFocus(true);
    UpdateRecentMenus();

    m_noOfPieces = m_totalSize = 0;
    m_messenger = new BMessenger(this);
}



FileJoinerWindow::~FileJoinerWindow()
{
    delete m_messenger;
}



void FileJoinerWindow::Quit()
{
    be_app_messenger.SendMessage(M_CLOSE_FILE_JOINER);
    return BWindow::Quit();
}



bool FileJoinerWindow::QuitRequested()
{
    if (m_joinInProgress && m_quitNow == false)    // m_quitNow is checked so this isn't called twice
    {
        suspend_thread(m_thread);

        BAlert* alert = new BAlert("Quit", B_TRANSLATE("Joining is in progress.  Force it to stop?"),
                                   B_TRANSLATE("Cancel"), B_TRANSLATE("Force"), NULL,
                                   B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->SetShortcut(0L, B_ESCAPE);
        alert->SetDefaultButton(alert->ButtonAt(1L));
        int32 index = alert->Go();
        if (index == 1L)
        {
            m_cancel = true;
            m_joinBtn->SetEnabled(false);
        }

        m_quitNow = true;
        resume_thread(m_thread);
        return false;
    }
    else if (m_quitNow)    // Incase QuitRequested is called a second time, still don't do anything silly
        return false;
    else
        return BWindow::QuitRequested();
}



void FileJoinerWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_JOIN_NOW:
        {
            if (m_joinInProgress == false)
            {
                UpdateData();

                if (Frame().Height() == m_hideProgress)
                    ToggleWindowHeight(true);

                m_cancel = false;
                m_thread = spawn_thread(_joiner, "_joiner", B_NORMAL_PRIORITY, (void*)this);
                resume_thread(m_thread);
                m_joinBtn->SetLabel(B_TRANSLATE("Cancel"));
                m_joinInProgress = true;
                m_joinBtn->MakeDefault(false);
            }
            else
            {
                m_cancel = true;
                m_joinBtn->SetEnabled(false);
            }

            break;
        }


        case M_SELECT_JOIN_FILE:
        {
            if (m_filePanel == NULL)
            {
                m_filePanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false,
                                             new BMessage(M_JOIN_FILE_SELECTED), NULL, true, true);
                m_filePanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                m_filePanel->Window()->AddToSubset(this);
                if (m_filePanel->Window()->LockLooper())
                {
                    m_filePanel->Window()->SetTitle(B_TRANSLATE("Select file or folder to join"));
                    m_filePanel->Window()->UnlockLooper();
                }
            }

            m_filePanel->Show();
            break;
        }

        case M_JOIN_FILE_SELECTED: case B_SIMPLE_DATA:
        {
            // Also handle drag 'n drop
            entry_ref ref;
            message->FindRef("refs", &ref);

            BEntry entry(&ref, true);
            BPath filePath;
            entry.GetPath(&filePath);
            m_filePathView->SetText(filePath.Path());
            filePath.GetParent(&filePath);

            m_folderPathView->SetText(filePath.Path());
            UpdateData();
            RefreshInfo();
            break;
        }

        case M_SELECT_JOIN_FOLDER:
        {
            if (m_dirPanel == NULL)
            {
                m_dirPanel = new SelectDirPanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_DIRECTORY_NODE,
                                                false, new BMessage(M_JOIN_FOLDER_SELECTED), NULL, true, false);

                m_dirPanel->SetButtonLabel(B_DEFAULT_BUTTON, B_TRANSLATE("Select"));
                m_dirPanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                m_dirPanel->Window()->AddToSubset(this);
                m_dirPanel->SetCurrentDirButton(B_TRANSLATE("Select"));

                if (m_dirPanel->Window()->LockLooper())
                {
                    m_dirPanel->Window()->SetTitle(B_TRANSLATE("Select folder to create the output (joined) file"));
                    m_dirPanel->Window()->Unlock();
                }
            }
            m_dirPanel->Show();
            break;
        }

        case M_JOIN_FOLDER_SELECTED:
        {
            if (m_dirPanel && m_dirPanel->IsShowing())
                m_dirPanel->Hide();

            entry_ref ref;
            message->FindRef("refs", &ref);

            BPath folderPath(&ref);
            m_folderPathView->SetText(folderPath.Path());
            UpdateData();
            // No need to call refresh info when output file folder is selected
            break;
        }

        case M_UPDATE_DATA:
        {
            UpdateData();
            break;
        }

        case M_REFRESH_INFO:
        {
            RefreshInfo();
            break;
        }

        case M_CALC_COMPLETE:
        {
            m_calcSize = false;

            m_noOfPieces = message->FindInt32(kCount);
            m_totalSize = message->FindInt64(kSize);

            m_sizeStr->SetText(LocaleStringFromBytes(m_totalSize).String());
            m_sizeStr->ResizeToPreferred();

            char countStr[60];
            sprintf(countStr, "%d", m_noOfPieces);
            m_piecesStr->SetText(countStr);
            m_piecesStr->ResizeToPreferred();
            UpdateData();
            break;
        }

        case M_SEPARATOR_CHANGED:
        {
            m_separatorStr = m_separatorView->Text();
            if (m_separatorStr.Length() > 0)
                RefreshInfo();
            break;
        }

        case BZR_UPDATE_PROGRESS:
        {
            char percentStr [100];
            float delta = message->FindFloat("delta");
            int8 percent = (int8)ceil(100 * ((m_statusBar->CurrentValue() + delta) / m_statusBar->MaxValue()));
            sprintf(percentStr, "%d%%", percent);

            BString text = B_TRANSLATE("Joining:");
            text << " " << message->FindString("text");

            m_statusBar->Update(delta, text.String(), percentStr);
            message->SendReply('DUMB');
            break;
        }

        case M_OPERATION_COMPLETE:
        {
            m_joinInProgress = false;
            m_joinBtn->SetEnabled(true);
            m_joinBtn->MakeDefault(true);

            status_t result = message->FindInt32(kResult);
            snooze(10000);
            ToggleWindowHeight(false);
            m_statusBar->Reset();

            BAlert* alert = NULL;
            if (result == BZR_DONE)
            {
                alert = new BAlert("Done", B_TRANSLATE("The file was successfully joined!"),
                                   B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
            }
            else if (result == BZR_CANCEL)
            {
                alert = new BAlert("Cancel", B_TRANSLATE("Joining of the file was cancelled"),
                                   B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
            }
            else
            {
                alert = new BAlert("Error", B_TRANSLATE("An unknown error occured while joining the files."),
                                   B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
            }

            // Incase of no-errors with input files and output dirs, save them to recent lists
            if (result == BZR_DONE || result == BZR_CANCEL)
            {
                m_recentSplitDirs->AddPath(m_dirPathStr.String());
                UpdateRecentMenus();
            }

            // Find and delete source files if needed and only if the join operation was successful!
            if (result == BZR_DONE && m_deleteChk->Value() == B_CONTROL_ON)
                DeleteChunks(m_chunkPathStr.String(), m_separatorStr.String());

            // Disable buttons as needed, etc. after pieces are deleted etc it will be disabled
            // or else enabled
            RefreshInfo();
            UpdateData();

            m_joinBtn->SetLabel(B_TRANSLATE("Join"));
            alert->SetShortcut(0L, B_ESCAPE);
            alert->SetDefaultButton(alert->ButtonAt(0L));
            alert->Go();

            if (m_openChk->Value() == B_CONTROL_ON)
            {
                entry_ref dirRef;
                BEntry destDirEntry(m_dirPathStr.String(), false);
                destDirEntry.GetRef(&dirRef);
                TrackerOpenFolder(&dirRef);
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

        default:
        {
            BWindow::MessageReceived(message);
            break;
        }
    }
}



void FileJoinerWindow::UpdateRecentMenus()
{
    m_recentSplitDirs->UpdateMenu(m_folderMenu, kRecentSplitDir, this);
}



void FileJoinerWindow::UpdateData()
{
    BEntry sourceEntry(m_filePathView->Text(), true);
    m_separatorView->SetEnabled(sourceEntry.IsFile());

    BEntry outputDir(m_folderPathView->Text(), false);

    m_chunkPathStr = m_filePathView->Text();
    m_separatorStr = m_separatorView->Text();
    m_dirPathStr = m_folderPathView->Text();

    m_statusBar->SetMaxValue(m_totalSize);

    if (sourceEntry.Exists())
        m_refreshBtn->SetEnabled(true);
    else
        m_refreshBtn->SetEnabled(false);

    if (outputDir.Exists() && sourceEntry.Exists() && m_noOfPieces > 1)
        m_joinBtn->SetEnabled(true);
    else
        m_joinBtn->SetEnabled(false);
}



void FileJoinerWindow::RefreshInfo()
{
    if (m_calcSize == true)
    {
        m_cancel = true;
        snooze(180000);
        m_calcSize = false;
    }

    entry_ref ref;
    BEntry sourceEntry(m_filePathView->Text(), true);
    sourceEntry.GetRef(&ref);

    m_cancel = false;
    BMessage* msg = new BMessage('xxxx');
    msg->AddRef(kRef, &ref);
    msg->AddPointer(kWindowPtr, (void*)this);
    msg->AddPointer(kCancel, (void*)m_cancel);

    m_thread = spawn_thread(_calcsize, "_calcsize", B_NORMAL_PRIORITY, (void*)msg);
    resume_thread(m_thread);
    m_calcSize = true;
}



void FileJoinerWindow::GetDirectoryInfo(BEntry* srcDir, int32& fileCount, off_t& totalSize,
                                        volatile bool* cancel)
{
    // Customized version of FSUtil's GetDirectoryInfo to not include recursion and counting of directories
    // I did not modify FSUtils one as it is performance critical and checking for bools each time inside
    // the loop etc would slow it down
    BDirectory dir(srcDir);
    BEntry entry;
    off_t size;
    while (dir.GetNextEntry(&entry, false) == B_OK)
    {
        if (cancel && *cancel == true)
            return;

        if (entry.IsFile())
        {
            entry.GetSize(&size);
            fileCount ++;
            totalSize += size;
        }
    }
}



void FileJoinerWindow::DeleteChunks(const char* firstChunkPathStr, const char* separator)
{
    BString firstChunkPath = firstChunkPathStr;
    int32 index = firstChunkPath.FindLast('/');
    BString dirString;
    firstChunkPath.CopyInto(dirString, 0, index);

    int32 index2 = firstChunkPath.FindLast(separator);
    if (index2 <= 0 || index2 < index)
        return;

    BString baseName;
    firstChunkPath.CopyInto(baseName, index + 1, index2 - 1 - index);

    BString numberString;
    firstChunkPath.CopyInto(numberString, index2 + strlen(separator), firstChunkPath.Length() - index2);

    int8 width = numberString.Length();

    BString curFileName = baseName;
    curFileName << separator << numberString;

    BDirectory dir(dirString.String());
    BEntry chunkEntry;
    uint16 start = atoi(numberString.String());     // start from the number they choose eg 2 or 3
    // uint16 start = 1;                         // always start with 1 as the first file number
    // Determine what is to be done, either 1 or the number user chooses
    while (dir.FindEntry(curFileName.String(), &chunkEntry, false) == B_OK)
    {
        chunkEntry.Remove();

        start++;
        char buf[B_PATH_NAME_LENGTH];
        sprintf(buf, "%s%s%0*d", baseName.String(), separator, width, start);
        curFileName = buf;
    }
}



void FileJoinerWindow::ToggleWindowHeight(bool expand)
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
        ResizeTo(Frame().Width(), i);
        if (expand)
            m_backView->Invalidate(BRect(0, i - 10, Bounds().right, i + 10));

        UpdateIfNeeded();
        snooze((int32)snoozeTime);
        Flush();
        UpdateIfNeeded();
    }
}






int32 FileJoinerWindow::_calcsize(void* arg)
{
    // Thread that computes the total size of files/folder that is selected to be joined
    entry_ref ref;
    volatile bool* cancel;
    FileJoinerWindow* wnd(NULL);
    int32 fileCount = 0;
    off_t totalSize = 0;

    BMessage* msg = (BMessage*)arg;
    msg->FindPointer(kWindowPtr, reinterpret_cast<void**>(&wnd));
    msg->FindRef(kRef, &ref);
    msg->FindPointer(kCancel, (void**)&cancel);

    BEntry entry(&ref, true);
    if (entry.IsDirectory())
        wnd->GetDirectoryInfo(&entry, fileCount, totalSize, cancel);
    else
    {
        // Now count should be determined for the moment it's 1
        BPath path;
        entry.GetPath(&path);
        FindChunks(path.Path(), wnd->m_separatorStr.String(), fileCount, totalSize, cancel);
        if (fileCount == 0)
        {
            fileCount = 1;
            entry.GetSize(&totalSize);
        }
    }

    msg->what = M_CALC_COMPLETE;
    msg->AddInt32(kCount, fileCount);
    msg->AddInt64(kSize, totalSize);
    wnd->PostMessage(msg);
    return 0;
}



int32 FileJoinerWindow::_joiner(void* arg)
{
    // The thread that does controls the split process
    FileJoinerWindow* wnd = (FileJoinerWindow*)arg;

    status_t result = JoinFile(wnd->m_chunkPathStr.String(), wnd->m_dirPathStr.String(),
                               wnd->m_separatorStr.String(), wnd->m_messenger, &(wnd->m_cancel));

    BMessage completeMsg(M_OPERATION_COMPLETE);
    completeMsg.AddInt32(kResult, result);
    wnd->m_messenger->SendMessage(&completeMsg);

    return result;
}


