// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "FileSplitterWindow.h"
#include "AppConstants.h"
#include "BeezerApp.h"
#include "BevelView.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "FSUtils.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "RecentMgr.h"
#include "SelectDirPanel.h"
#include "Shared.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"

#include <Alert.h>
#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <NumberFormat.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <StatusBar.h>
#include <TextControl.h>
#include <StringView.h>

#include <stdio.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FileSplitterWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_COMMENT(x, y) x
#define B_TRANSLATE_CONTEXT(x, y) x
#endif

const uint32 M_SPLIT_NOW             = 'splt';
const uint32 M_SELECT_SPLIT_FILE     = 'file';
const uint32 M_SELECT_SPLIT_FOLDER   = 'fold';
const uint32 M_SPLIT_FILE_SELECTED   = 'fise';
const uint32 M_SPLIT_FOLDER_SELECTED = 'flse';
const uint32 M_CUSTOM_SIZE           = 'cuss';
const uint32 M_PREDEFINED_SIZE       = 'pres';
const uint32 M_UPDATE_DATA           = 'updd';
const uint32 M_OPERATION_COMPLETE    = 'opcc';
const uint32 M_SEPARATOR_CHANGED     = 'spch';


FileSplitterWindow::FileSplitterWindow(RecentMgr* files, RecentMgr* dirs)
    : BWindow(BRect(10, 10, 540, 350), B_TRANSLATE("File Splitter"), B_TITLED_WINDOW,
              B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
    m_backView(NULL),
    m_fileMenu(NULL),
    m_folderMenu(NULL),
    m_prefixField(NULL),
    m_sizePopUp(NULL),
    m_prefixPopUp(NULL),
    m_filePathView(NULL),
    m_folderPathView(NULL),
    m_customSizeView(NULL),
    m_separatorView(NULL),
    m_splitBtn(NULL),
    m_descStr(NULL),
    m_descStr2(NULL),
    m_piecesStr(NULL),
    m_sizeStr(NULL),
    m_createChk(NULL),
    m_openDirChk(NULL),
    m_closeChk(NULL),
    m_statusBar(NULL),
    m_dirPanel(NULL),
    m_filePanel(NULL),
    m_hideProgress(0.0f),
    m_showProgress(0.0f),
    m_fragmentSize(0),
    m_fragmentCount(0),
    m_sepString(NULL),
    m_cancel(false),
    m_splitInProgress(false),
    m_quitNow(false),
    m_messenger(NULL),
    //m_threadId
    m_recentSplitFiles(files),
    m_recentSplitDirs(dirs)
{
    BFont font(be_plain_font);
    font_height fntHt;
    font.GetHeight(&fntHt);
    float const fontHeight = fntHt.ascent + fntHt.descent + fntHt.leading;

    // If you change the above BRect in the BWindow call, change accordingly the code at the end
    // of this constructor to match, i.e. current 550 - 10 = 540 is used below, if you change them
    // change the code below (see "Constrain size of window" comment)
    m_backView = new BevelView(Bounds(), "FileSplitterWindow:BackView", BevelView::bevel_type::OUTSET,
                               B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    AddChild(m_backView);

    BBitmap* splitBmp = BitmapPool::LoadAppVector("Img:FileSplitter", 32, 32);

    BevelView* sepView1 = new BevelView(BRect(-1, splitBmp->Bounds().Height() + 4 * K_MARGIN,
                                              Bounds().right - 1.0, splitBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                        "FileSplitterWindow:SepView1", BevelView::bevel_type::INSET,
                                        B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(sepView1);

    StaticBitmapView* splitBmpView = new StaticBitmapView(BRect(K_MARGIN * 5, K_MARGIN * 2,
                                                                splitBmp->Bounds().Width() + K_MARGIN * 5,
                                                                splitBmp->Bounds().Height() + K_MARGIN * 2),
                                                          "FileSplitterWindow:splitBmpView", splitBmp);
    splitBmpView->SetViewColor(m_backView->ViewColor());
    AddChild(splitBmpView);

    // Add the file name string view (align it vertically with the icon view)
    m_descStr = new BStringView(BRect(splitBmpView->Frame().right + K_MARGIN * 3,
                                      splitBmpView->Frame().top, Bounds().right - 1,
                                      splitBmpView->Frame().top + fontHeight),
                                "FileSplitterWindow:DescStr",
                                B_TRANSLATE("This tool lets you split a file into several small pieces."),
                                B_FOLLOW_LEFT, B_WILL_DRAW);

    //m_descStr->SetFont (&font);
    m_backView->AddChild(m_descStr);
    m_descStr->MoveTo(m_descStr->Frame().left,
                      (splitBmpView->Frame().Height() / 2 - fontHeight / 2) + fontHeight / 2 + 1);
    m_descStr->MoveBy(0, -fontHeight / 2 - 1);
    m_descStr->ResizeToPreferred();

    m_descStr2 = new BStringView(BRect(m_descStr->Frame().left,
                                       m_descStr->Frame().bottom + 1, Bounds().right - 1, 0),
                                 "FileSplitterWindow:DescStr2",
                                 B_TRANSLATE("The pieces can later be rejoined to recreate the original file."),
                                 B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(m_descStr2);
    m_descStr2->ResizeToPreferred();

    BevelView *innerView = new BevelView(BRect(K_MARGIN, sepView1->Frame().bottom + K_MARGIN,
                                      Bounds().right - K_MARGIN,
                                      Bounds().bottom - K_MARGIN), "FileSplitterWindow:InnerView",
                                BevelView::bevel_type::NO_BEVEL, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(innerView);

    m_fileMenu = new BMenu(B_TRANSLATE("File to split:"));
    BMenuField *fileField = new BMenuField(BRect(K_MARGIN, K_MARGIN, K_MARGIN + m_backView->StringWidth(B_TRANSLATE("File to split:")) + 40,
                                                 0), "FileSplitterWindow:FileField", NULL, m_fileMenu);
    innerView->AddChild(fileField);

    m_folderMenu = new BMenu(B_TRANSLATE("Folder for pieces:"));
    BMenuField *folderField = new BMenuField(BRect(K_MARGIN, fileField->Frame().bottom + K_MARGIN - 1,
                                                   K_MARGIN + m_backView->StringWidth(B_TRANSLATE("Folder for pieces:")) + 40, 0),
                                             "FileSplitterWindow:FolderField", NULL, m_folderMenu);
    float maxWidth = MAX(folderField->Frame().Width(), fileField->Frame().Width());
    maxWidth = MAX(maxWidth, m_backView->StringWidth(B_TRANSLATE("Size of pieces:")));
    maxWidth = MAX(maxWidth, m_backView->StringWidth(B_TRANSLATE("Custom size:")));
    maxWidth = MAX(maxWidth, m_backView->StringWidth(B_TRANSLATE_CONTEXT("Number of pieces:", "FileJoiner/Splitter")));
    maxWidth = MAX(maxWidth, m_backView->StringWidth(B_TRANSLATE("File size:")));
    maxWidth = MAX(maxWidth, m_backView->StringWidth(B_TRANSLATE_CONTEXT("File number separator:", "FileJoiner/Splitter")));
    maxWidth += 2 * K_MARGIN;

    m_filePathView = new BTextControl(BRect(maxWidth, fileField->Frame().top + 2,
                                            innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                      "FileSplitterView:FilePathView", NULL, NULL, NULL,
                                      B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_filePathView->SetDivider(0);
    m_filePathView->ResizeToPreferred();
    m_filePathView->SetModificationMessage(new BMessage(M_UPDATE_DATA));

    m_folderPathView = new BTextControl(BRect(maxWidth, folderField->Frame().top + 2,
                                        innerView->Frame().Width() - 2 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                        "FileSplitterView:FolderPathView", NULL, NULL, NULL,
                                        B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_folderPathView->SetDivider(0);
    m_folderPathView->ResizeToPreferred();
    m_folderPathView->SetModificationMessage(new BMessage(M_UPDATE_DATA));

    // Re-use this string here
    BButton *selectFileBtn = new BButton(BRect(m_filePathView->Frame().right + K_MARGIN,
                                               m_filePathView->Frame().top - 4, m_filePathView->Frame().right + K_MARGIN + K_BUTTON_WIDTH,                                        m_filePathView->Frame().top - 4 + K_BUTTON_HEIGHT),
                                         "FileSplitterWindow:SelectFileBtn", B_TRANSLATE_CONTEXT("Select" B_UTF8_ELLIPSIS, "FileJoiner/Splitter"),
                                         new BMessage(M_SELECT_SPLIT_FILE), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    innerView->AddChild(m_filePathView);
    innerView->AddChild(selectFileBtn);

    BButton *selectFolderBtn = new BButton(BRect(m_folderPathView->Frame().right + K_MARGIN, m_folderPathView->Frame().top - 4,
                                                 m_folderPathView->Frame().right + K_MARGIN + K_BUTTON_WIDTH,
                                                 m_folderPathView->Frame().top - 4 + K_BUTTON_HEIGHT),
                                           "FileSplitterWindow:SelectFolderBtn", B_TRANSLATE("Select" B_UTF8_ELLIPSIS),
                                           new BMessage(M_SELECT_SPLIT_FOLDER), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    innerView->AddChild(folderField);
    innerView->AddChild(m_folderPathView);
    innerView->AddChild(selectFolderBtn);

    m_sizePopUp = new BPopUpMenu("Sizes", true, true);
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("Custom size" B_UTF8_ELLIPSIS), new BMessage(M_CUSTOM_SIZE)));
    m_sizePopUp->AddSeparatorItem();
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("1.44 MiB - (floppy)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("1.20 MiB - (floppy)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("100 MiB - (zip disk)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("250 MiB - (zip disk)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("650 MiB - (CD)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("700 MiB - (CD)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("800 MiB - (CD)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("1 GiB - (jaz disk)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->AddItem(new BMenuItem(B_TRANSLATE("2 GiB - (jaz disk)"), new BMessage(M_PREDEFINED_SIZE)));
    m_sizePopUp->ItemAt(0L)->SetMarked(true);

    BMenuField *sizeField = new BMenuField(BRect(K_MARGIN, folderField->Frame().bottom + K_MARGIN - 1,
                                                innerView->Frame().Width() - K_MARGIN, 0),
                                           "FileSplitterWindow:SizeField", B_TRANSLATE("Size of pieces:"),
                                           (BMenu*)m_sizePopUp, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    sizeField->SetDivider(m_filePathView->Frame().left - K_MARGIN - 1);
    innerView->AddChild(sizeField);

    m_customSizeView = new BTextControl(BRect(K_MARGIN, sizeField->Frame().bottom + K_MARGIN - 1,
                                              K_MARGIN + 182, 0),
                                        "FileSplitterWindow:CustomSizeView", B_TRANSLATE("Custom size:"), NULL, NULL,
                                        B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_customSizeView->SetDivider(sizeField->Divider() - 2);
    m_customSizeView->SetModificationMessage(new BMessage(M_UPDATE_DATA));
    innerView->AddChild(m_customSizeView);

    // !! IMPORTANT !! Order is very critical, if the order below changes, the change must also be reflected
    // in "UpdateData()" function
    m_prefixPopUp = new BPopUpMenu("SizePrefix", true, true);
    m_prefixPopUp->AddItem(new BMenuItem(BZ_TR(kBytesString), new BMessage(M_UPDATE_DATA)));
    m_prefixPopUp->AddItem(new BMenuItem(BZ_TR(kKilobyteString), new BMessage(M_UPDATE_DATA)));
    m_prefixPopUp->AddItem(new BMenuItem(BZ_TR(kMegabyteString), new BMessage(M_UPDATE_DATA)));
    m_prefixPopUp->AddItem(new BMenuItem(BZ_TR(kGigabyteString), new BMessage(M_UPDATE_DATA)));
    m_prefixPopUp->ItemAt(2L)->SetMarked(true);
    m_prefixField = new BMenuField(BRect(m_customSizeView->Frame().right + K_MARGIN,
                                         m_customSizeView->Frame().top - 2, innerView->Frame().right - K_MARGIN, 0),
                                   "FileSplitterWindow:PrefixView", NULL, (BMenu*)m_prefixPopUp,
                                   B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    innerView->AddChild(m_prefixField);

    float maxPrefixWidth = 0.0f;
    for (int32 i = 0; i < m_sizePopUp->CountItems(); i++)
        maxPrefixWidth = MAX(maxPrefixWidth, 36 + m_backView->StringWidth(m_sizePopUp->ItemAt(i)->Label()));

    for (int32 i = 0; i < m_prefixPopUp->CountItems(); i++)
    {
        maxPrefixWidth = MAX(maxPrefixWidth,
                             m_customSizeView->Frame().Width() - m_customSizeView->Divider() + 36 +
                             m_backView->StringWidth(m_prefixPopUp->ItemAt(i)->Label()));
    }

    maxPrefixWidth += m_customSizeView->Divider() + K_MARGIN;

    BevelView* sepView4 = new BevelView(BRect(maxPrefixWidth, sizeField->Frame().top, maxPrefixWidth + 1,
                                        m_prefixField->Frame().bottom), "FileSplitterWindow:sepView4",
                                        BevelView::bevel_type::INSET, B_FOLLOW_LEFT, B_WILL_DRAW);
    innerView->AddChild(sepView4);

    m_openDirChk = new BCheckBox(BRect(sepView4->Frame().left + 3 * K_MARGIN, sizeField->Frame().top + 2, 0, 0),
                                 "FileSplitterWindow:OpenDirChk", B_TRANSLATE("Open pieces folder after splitting"), NULL,
                                 B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_openDirChk->ResizeToPreferred();
    m_openDirChk->SetValue(B_CONTROL_ON);

    m_closeChk = new BCheckBox(BRect(m_openDirChk->Frame().left, m_openDirChk->Frame().bottom + 1, 0, 0),
                               "FileSplitterWindow:CloseChk", B_TRANSLATE("Close window after splitting"), NULL, B_FOLLOW_LEFT,
                               B_WILL_DRAW | B_NAVIGABLE);
    m_closeChk->ResizeToPreferred();
    m_closeChk->SetValue(B_CONTROL_ON);

    m_createChk = new BCheckBox(BRect(m_closeChk->Frame().left, m_closeChk->Frame().bottom + 1, 0, 0),
                                "FileSplitterWindow:CreateChk", B_TRANSLATE("Create self-joining executable"), NULL,
                                B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_createChk->ResizeToPreferred();

    m_separatorView = new BTextControl(BRect(m_customSizeView->Frame().left, m_customSizeView->Frame().bottom + K_MARGIN,
                                             m_customSizeView->Frame().right, 0), "FileSplitter:SeparatorView",
                                       B_TRANSLATE("File number separator:"), "_", NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_separatorView->SetDivider(m_customSizeView->Divider());
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
    innerView->AddChild(m_separatorView);
    innerView->AddChild(m_openDirChk);            // For tab ordering!
    innerView->AddChild(m_closeChk);
    innerView->AddChild(m_createChk);

    innerView->ResizeTo(innerView->Frame().Width(), m_separatorView->Frame().bottom + K_MARGIN);

    // Extend the line to the above text controls' bottom co-ordinate
    sepView4->ResizeBy(0, m_customSizeView->Frame().Height() + K_MARGIN + 1);


    // Add the next level of controls
    BevelView* sepView2 = new BevelView(BRect(-1, innerView->Frame().bottom + K_MARGIN + 1, Bounds().right - 1.0,
                                              innerView->Frame().bottom + K_MARGIN + 1 + 1),
                                        "FileSplitterWindow:SepView2", BevelView::bevel_type::INSET,
                                        B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild(sepView2);

    BStringView* noPiecesStr = new BStringView(BRect(2 * K_MARGIN, sepView2->Frame().bottom + 2 * K_MARGIN,
                                                     2 * K_MARGIN + m_backView->StringWidth(B_TRANSLATE("Number of pieces:")) + 3, 0),
                                               "FileSplitterWindow:noPiecesStr", B_TRANSLATE("Number of pieces:"),
                                               B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(noPiecesStr);
    noPiecesStr->ResizeToPreferred();

    m_piecesStr = new BStringView(BRect(m_filePathView->Frame().left + K_MARGIN, noPiecesStr->Frame().top, Bounds().right - K_MARGIN, 0),
                                  "FileSplitterWindow:PiecesStr", "-", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild(m_piecesStr);
    m_piecesStr->ResizeToPreferred();

    BStringView* sizeStr = new BStringView(BRect(noPiecesStr->Frame().left, noPiecesStr->Frame().bottom + K_MARGIN - 1, 0, 0),
                                           "FileSplitterWindow:sizeStr", B_TRANSLATE("File size:"), B_FOLLOW_LEFT, B_WILL_DRAW);
    sizeStr->ResizeToPreferred();
    m_backView->AddChild(sizeStr);

    m_sizeStr = new BStringView(BRect(m_piecesStr->Frame().left, sizeStr->Frame().top, 0, 0),
                                "FileSplitterWindow:SizeStr", "-", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_sizeStr->ResizeToPreferred();
    m_backView->AddChild(m_sizeStr);

    BevelView* sepView3 = new BevelView(BRect(-1, m_sizeStr->Frame().bottom + K_MARGIN, Bounds().right - 1.0,
                                              m_sizeStr->Frame().bottom + K_MARGIN + 1),
                                        "FileSplitterWindow:SepView3", BevelView::bevel_type::INSET,
                                        B_FOLLOW_LEFT_RIGHT,B_WILL_DRAW);
    m_backView->AddChild(sepView3);
    sepView3->Hide();

    m_splitBtn = new BButton(BRect(Bounds().right - 2 * K_MARGIN - K_BUTTON_WIDTH - 3,
                                   sepView3->Frame().bottom - K_MARGIN - 6 - K_BUTTON_HEIGHT,
                                   Bounds().right - 2 * K_MARGIN - 3, sepView3->Frame().bottom - K_MARGIN - 6),
                             "FileSplitterWindow:SplitBtn", B_TRANSLATE("Split"), new BMessage(M_SPLIT_NOW),
                             B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_splitBtn->MakeDefault(true);
    m_backView->AddChild(m_splitBtn);
    m_splitBtn->SetEnabled(false);

    // Add the status bar and buttons
    m_statusBar = new BStatusBar(BRect(3 * K_MARGIN, sepView3->Frame().bottom + 2 * K_MARGIN + 1,
                                       Bounds().right - 8 * K_MARGIN - K_BUTTON_WIDTH, 0),
                                 "FileSplitterWindow:StatusBar", NULL, NULL);
    m_statusBar->ResizeToPreferred();
    m_statusBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT);
    m_statusBar->ResizeTo(m_splitBtn->Frame().right - 4 * K_MARGIN, m_statusBar->Frame().Height());
    m_statusBar->SetBarHeight(K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor(K_PROGRESS_COLOR);
    m_backView->AddChild(m_statusBar);

    // Resize window as needed
    m_hideProgress = sepView3->Frame().top + K_MARGIN;
    m_showProgress =  m_statusBar->Frame().bottom + K_MARGIN;
    ResizeTo(Frame().Width(), m_hideProgress);

    CenterOnScreen();

    // Calculatate maximum desc label size
    float maxLabelLen = MAX(m_descStr->StringWidth(m_descStr->Text()),
                            m_descStr2->StringWidth(m_descStr2->Text()));
    maxLabelLen += splitBmpView->Frame().right + 2 * K_MARGIN * 5;
    maxLabelLen = MAX(maxLabelLen, m_openDirChk->Frame().right);
    maxLabelLen = MAX(maxLabelLen, m_createChk->Frame().right);
    maxLabelLen += 2 * K_MARGIN;
    maxLabelLen = MAX(maxLabelLen, 540);

    // Constrain size of window
    float minH, maxH, minV, maxV;
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    SetSizeLimits(maxLabelLen, maxH, minV, maxV);
    if (Frame().Width() < maxLabelLen)
        ResizeTo(maxLabelLen, Frame().Height());

    m_filePathView->MakeFocus(true);
    m_messenger = new BMessenger(this);

    UpdateRecentMenus();

    m_splitInProgress = false;
    m_quitNow = false;
}


FileSplitterWindow::~FileSplitterWindow()
{
    if (m_dirPanel)
        delete m_dirPanel;

    if (m_filePanel);
    delete m_filePanel;

    if (m_sepString)
        free(m_sepString);

    delete m_messenger;
}


bool FileSplitterWindow::QuitRequested()
{
    if (m_splitInProgress && m_quitNow == false)    // m_quitNow is checked so this isn't called twice
    {
        suspend_thread(m_thread);

        BAlert* alert = new BAlert("Quit", B_TRANSLATE("Splitting is in progress. Force it to stop?"),
                                   BZ_TR(kCancelString), BZ_TR(kForceStopString),
                                   NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->SetShortcut(0L, B_ESCAPE);
        alert->SetDefaultButton(alert->ButtonAt(1L));
        int32 index = alert->Go();
        if (index == 1)
        {
            m_cancel = true;
            m_splitBtn->SetEnabled(false);
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


void FileSplitterWindow::Quit()
{
    be_app_messenger.SendMessage(M_CLOSE_FILE_SPLITTER);
    return BWindow::Quit();
}


void FileSplitterWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_SPLIT_NOW:
        {
            if (m_splitInProgress == false)
            {
                UpdateData();

                if (Frame().Height() == m_hideProgress)
                    ToggleWindowHeight(true);

                m_cancel = false;
                m_thread = spawn_thread(_splitter, "_splitter", B_NORMAL_PRIORITY, (void*)this);
                resume_thread(m_thread);
                m_splitBtn->SetLabel(BZ_TR(kCancelString));
                m_splitInProgress = true;
                m_splitBtn->MakeDefault(false);
            }
            else
            {
                m_cancel = true;
                m_splitBtn->SetEnabled(false);
            }

            break;
        }

        case BZR_UPDATE_PROGRESS:
        {
            char percentStr [100];
            float const delta = message->FindFloat("delta");
            int8 const percent = (int8)ceil(100 * ((m_statusBar->CurrentValue() + delta) / m_statusBar->MaxValue()));
            sprintf(percentStr, "%d%%", percent);

            BString text = B_TRANSLATE("Creating:");
            text << " " << message->FindString("text");

            m_statusBar->Update(delta, text.String(), percentStr);
            message->SendReply('DUMB');
            break;
        }

        case M_OPERATION_COMPLETE:
        {
            m_splitInProgress = false;
            m_splitBtn->SetEnabled(true);
            m_splitBtn->MakeDefault(true);

            status_t const result = message->FindInt32(kResult);
            snooze(10000);
            ToggleWindowHeight(false);
            m_statusBar->Reset();

            BAlert* alert = NULL;
            if (result == BZR_DONE)
            {
                alert = new BAlert("Done", B_TRANSLATE("The file has been successfully split!"),
                                   BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
            }
            else if (result == BZR_CANCEL)
            {
                alert = new BAlert("Cancel", B_TRANSLATE("Splitting of the file was cancelled"),
                                   BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
            }
            else
            {
                alert = new BAlert("Error", B_TRANSLATE("An unknown error occured while splitting the file."),
                                   BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
            }

            if (result == BZR_DONE && m_createChk->Value() == B_CONTROL_ON)
                CreateSelfJoiner();

            // Incase of no-errors with input files and output dirs, save them to recent lists
            if (result == BZR_DONE || result == BZR_CANCEL)
            {
                BPath path;
                BEntry temp;
                m_fileEntry.GetPath(&path);
                m_recentSplitFiles->AddPath(path.Path());

                m_destDir.GetEntry(&temp);
                temp.GetPath(&path);
                m_recentSplitDirs->AddPath(path.Path());

                UpdateRecentMenus();
            }

            m_splitBtn->SetLabel(B_TRANSLATE("Split"));
            alert->SetShortcut(0L, B_ESCAPE);
            alert->SetDefaultButton(alert->ButtonAt(0L));
            alert->Go();

            if (m_openDirChk->Value() == B_CONTROL_ON)
            {
                entry_ref dirRef;
                BEntry destDirEntry;
                m_destDir.GetEntry(&destDirEntry);

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

        case M_SELECT_SPLIT_FILE:
        {
            if (m_filePanel == NULL)
            {
                m_filePanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
                                             B_FILE_NODE, false, new BMessage(M_SPLIT_FILE_SELECTED), NULL,
                                             true, true);
                m_filePanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                m_filePanel->Window()->AddToSubset(this);
                if (m_filePanel->Window()->LockLooper())
                {
                    m_filePanel->Window()->SetTitle(B_TRANSLATE("Select folder for pieces:"));
                    m_filePanel->Window()->UnlockLooper();
                }
            }

            m_filePanel->Show();
            break;
        }

        case M_SPLIT_FILE_SELECTED: case M_TOOLS_FILE_SPLITTER:
        {
            entry_ref ref;
            message->FindRef("refs", &ref);

            BPath filePath(&ref);
            m_filePathView->SetText(filePath.Path());

            BPath parentPath;
            filePath.GetParent(&parentPath);
            m_folderPathView->SetText(parentPath.Path());
            UpdateData();

            if (message->what == M_TOOLS_FILE_SPLITTER && m_customSizeView->IsEnabled())
                m_customSizeView->MakeFocus(true);

            break;
        }

        case M_SELECT_SPLIT_FOLDER:
        {
            if (m_dirPanel == NULL)
            {
                m_dirPanel = new SelectDirPanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_DIRECTORY_NODE,
                                                false, new BMessage(M_SPLIT_FOLDER_SELECTED), NULL, true, false);

                m_dirPanel->SetButtonLabel(B_DEFAULT_BUTTON, BZ_TR(kSelectString));
                m_dirPanel->Window()->SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
                m_dirPanel->Window()->AddToSubset(this);
                m_dirPanel->SetCurrentDirButton(BZ_TR(kSelectString));

                if (m_dirPanel->Window()->LockLooper())
                {
                    m_dirPanel->Window()->SetTitle(B_TRANSLATE("Select folder for pieces:"));
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
            message->FindRef("refs", &ref);

            BPath folderPath(&ref);
            m_folderPathView->SetText(folderPath.Path());
            break;
        }

        case M_CUSTOM_SIZE:
        {
            m_customSizeView->SetEnabled(true);
            m_prefixField->SetEnabled(true);
            UpdateData();
            break;
        }

        case M_PREDEFINED_SIZE:
        {
            m_customSizeView->SetText(NULL);
            m_customSizeView->SetEnabled(false);
            m_prefixField->SetEnabled(false);
            UpdateData();
            break;
        }

        case M_UPDATE_DATA:
        {
            UpdateData();
            break;
        }

        case M_RECENT_SPLIT_FILE:
        {
            // Don't load from label of BMenuItem as if that changes to display only filename than path
            // we would have errors, hence load from the Path field that "RecentMgr" adds to each recent item
            BPath path;
            message->FindFlat(kRecentSplitFile, &path);
            m_filePathView->SetText(path.Path());
            break;
        }

        case M_RECENT_SPLIT_DIR:
        {
            // Load from Path object -- even though it's not a file item
            BPath path;
            message->FindFlat(kRecentSplitDir, &path);
            m_folderPathView->SetText(path.Path());
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
            message->FindRef("refs", &ref);

            BEntry entry(&ref, true);
            entry.GetPath(&path);
            if (entry.IsFile())                             // File is dropped
                m_filePathView->SetText(path.Path());
            else if (entry.IsDirectory())                  // Folder is dropped
                m_folderPathView->SetText(path.Path());

            break;
        }

        default:
            BWindow::MessageReceived(message);
            break;
    }
}


void FileSplitterWindow::ToggleWindowHeight(bool expand)
{
    float start = m_hideProgress;
    float finish = m_showProgress + 1;
    float step = 1;
    float const snoozeTime = (m_showProgress - m_hideProgress) * 5.0f;
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


void FileSplitterWindow::UpdateData()
{
    // Updates the filename entry, folder entry, size and number of pieces display etc
    if (m_filePathView->Text() == NULL)
        return;

    m_fragmentCount = m_fragmentSize = 0;

    m_fileEntry.SetTo(m_filePathView->Text(), true);
    m_destDir.SetTo(m_folderPathView->Text());

    if (m_sepString)
    {
        free(m_sepString);
        m_sepString = NULL;
    }

    if (m_separatorView->Text())
        m_sepString = strdup(m_separatorView->Text());
    else
        m_sepString = strdup("_");

    if (m_fileEntry.Exists() && m_fileEntry.IsFile())
    {
        off_t size;
        m_fileEntry.GetSize(&size);

        BString sizeStr = LocaleStringFromBytes(size);
        if (size > 1024L)
        {
            BString fmtStr;
            if (BNumberFormat().Format(fmtStr, (double)size) != B_OK)
                fmtStr = "???";

            sizeStr << "  " << "(" << fmtStr << " " << BZ_TR(kbytesString) << ")";
        }

        m_sizeStr->SetText(sizeStr.String());

        // Determine the size of each piece that the user has selected in bytes
        uint64 fragmentSize = 0;
        if (m_customSizeView->IsEnabled())
        {
            BString val;
            val << m_customSizeView->Text();
            val << " ";

            switch (m_prefixPopUp->IndexOf(m_prefixPopUp->FindMarked()))
            {
                case 0: val << BZ_TR(kbytesString); break;
                case 1: val << BZ_TR(kKilobyteString); break;
                case 2: val << BZ_TR(kMegabyteString); break;
                case 3: val << BZ_TR(kGigabyteString); break;
            }

            fragmentSize = BytesFromString((char*)val.String());
            m_fragmentSize = fragmentSize;
        }
        else               // Predefined size
        {
            BString sizeLabel = m_sizePopUp->FindMarked()->Label();
            int32 const index = sizeLabel.FindFirst("-");
            sizeLabel.Truncate(index - 1);

            fragmentSize = BytesFromString((char*)sizeLabel.String());
            m_fragmentSize = fragmentSize;
        }

        m_piecesStr->SetHighColor((rgb_color)
        {
            0, 0, 0, 255
        });
        if (fragmentSize > 0)
        {
            int32 const noOfPieces = (size / fragmentSize);
            if (noOfPieces <= 0 && m_customSizeView->Text())
            {
                m_piecesStr->SetText(B_TRANSLATE("Incorrect split size!"));
                m_piecesStr->SetHighColor((rgb_color)
                {
                    210, 0, 0, 255
                });
            }
            else if (noOfPieces > kMaxFragmentCount)
            {
                m_piecesStr->SetText(B_TRANSLATE("Too many pieces!"));
                m_piecesStr->SetHighColor((rgb_color)
                {
                    0, 0, 210, 255
                });
            }
            else
            {
                m_fragmentCount = noOfPieces;
                if ((int64)(m_fragmentCount * m_fragmentSize) < size)
                    m_fragmentCount++;

                BString temp;
                temp << m_fragmentCount;
                m_piecesStr->SetText(temp.String());
            }
        }
        else if (fragmentSize < 0)
        {
            m_piecesStr->SetText(B_TRANSLATE("Incorrect split size!"));
            m_piecesStr->SetHighColor((rgb_color)
            {
                210, 0, 0, 255
            });
        }
        else
            m_piecesStr->SetText("-");

        m_statusBar->SetMaxValue(size);
    }
    else
    {
        m_sizeStr->SetText("-");
        m_piecesStr->SetText("-");
    }

    m_sizeStr->ResizeToPreferred();
    m_piecesStr->ResizeToPreferred();
    if (m_fragmentCount > 0 && m_fragmentSize > 0 && m_destDir.IsDirectory() &&
            m_separatorView->TextView()->TextLength() > 0)
        m_splitBtn->SetEnabled(true);
    else
        m_splitBtn->SetEnabled(false);
}


void FileSplitterWindow::UpdateRecentMenus()
{
    m_recentSplitFiles->UpdateMenu(m_fileMenu, kRecentSplitFile, this);
    m_recentSplitDirs->UpdateMenu(m_folderMenu, kRecentSplitDir, this);
}


void FileSplitterWindow::CreateSelfJoiner()
{
    // Rather a copy self joiner to outputdir etc
    BDirectory* stubDir = &(_bzr()->m_stubDir);
    if (stubDir->IsDirectory() == false)
    {
        BAlert* err = new BAlert("Error", B_TRANSLATE("Stub folder not found.  Cannot create self-joining executable."),
                                 BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
        err->Go();
        return;
    }

    stubDir->Rewind();
    BEntry stubEntry;
    stubDir->FindEntry(K_STUB_JOINER_FILE, &stubEntry, true);

    char* stubName = new char[B_FILE_NAME_LENGTH + 1];      // if symlink was traversed, name might change
    stubEntry.GetName(stubName);                            // hence read the name from the entry again

    CopyFile(&stubEntry, &m_destDir, NULL, &m_cancel);      // search the destination dir for the name
    m_destDir.Rewind();                                     // and then we have our file that we want to
    m_destDir.FindEntry(stubName, &stubEntry, true);        // add the resources to
    delete[] stubName;

    BString name = B_TRANSLATE_COMMENT("_Create_", "Prefix for stub files. The underscore at the beginning is to change the sort order");
    name << m_fileEntry.Name();

    stubEntry.Rename(name.String(), true);                  // Rename to "create_filename"

    BFile stubFile(&stubEntry, B_READ_WRITE);

    BResources res(&stubFile, false);
    res.AddResource(B_STRING_TYPE, 1, m_firstChunkName.String(), m_firstChunkName.Length(),
                    K_FILENAME_ATTRIBUTE);
    res.AddResource(B_STRING_TYPE, 2, m_sepString, strlen(m_sepString), K_SEPARATOR_ATTRIBUTE);

    res.Sync();
    stubFile.Sync();
    stubFile.Unset();
}


int32 FileSplitterWindow::_splitter(void* arg)
{
    // The thread that does controls the split process
    FileSplitterWindow* wnd = (FileSplitterWindow*)arg;

    status_t const result = SplitFile(&(wnd->m_fileEntry), &(wnd->m_destDir), wnd->m_messenger,
                                      wnd->m_fragmentSize, wnd->m_fragmentCount, wnd->m_sepString, wnd->m_firstChunkName,
                                      &(wnd->m_cancel));

    BMessage completeMsg(M_OPERATION_COMPLETE);
    completeMsg.AddInt32(kResult, result);
    wnd->m_messenger->SendMessage(&completeMsg);

    return result;
}
