// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "JoinerWindow.h"
#include "AppConstants.h"
#include "BevelView.h"
#include "Joiner.h"
#include "Shared.h"
#include "UIConstants.h"

#include <Application.h>
#include <StatusBar.h>
#include <Button.h>
#include <Alert.h>
#include <Resources.h>
#include <Roster.h>
#include <Path.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JoinerWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif

const char* JoinerWindow::kJoinResult = "result";


JoinerWindow::JoinerWindow()
    : BWindow(BRect(100, 100, 450, 210), B_TRANSLATE("Beezer: File Joiner"), B_TITLED_WINDOW,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_V_RESIZABLE),
    m_backView(NULL),
    m_statusBar(NULL),
    m_cancelBtn(NULL),
    m_cancel(false),
    m_messenger(new BMessenger(this)),
    m_thread(0)
{
    m_backView = new BevelView(Bounds(), "JoinerWindow:BackView", BevelView::OUTSET,
                               B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    AddChild(m_backView);

    m_statusBar = new BStatusBar(BRect(2 * K_MARGIN, 2 * K_MARGIN, Bounds().right - 2 * K_MARGIN, 0),
                                 "JoinerWindow:StatusBar", B_TRANSLATE("Joining:"), NULL);
    m_statusBar->ResizeToPreferred();
    m_statusBar->ResizeTo(Bounds().right - 4 * K_MARGIN - 1, m_statusBar->Frame().Height());
    m_statusBar->SetResizingMode(B_FOLLOW_LEFT_RIGHT);
    m_statusBar->SetBarHeight(K_PROGRESSBAR_HEIGHT);
    m_statusBar->SetBarColor(K_PROGRESS_COLOR);
    m_backView->AddChild(m_statusBar);

    m_cancelBtn = new BButton(BRect(Bounds().right - 2 * K_MARGIN - K_BUTTON_WIDTH,
                                    m_statusBar->Frame().bottom + K_MARGIN, Bounds().right - 2 * K_MARGIN,
                                    m_statusBar->Frame().bottom + K_MARGIN + K_BUTTON_HEIGHT),
                              "JoinerWindow:CancelBtn", B_TRANSLATE("Cancel"), new BMessage(M_CANCEL),
                              B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(m_cancelBtn);

    ResizeTo(Frame().Width(), m_cancelBtn->Frame().bottom + 2 * K_MARGIN);

    // Center window on-screen
    CenterOnScreen();

    // Constrain window size
    float const minWidth = 350;
    float minH, maxH, minV, maxV;
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    SetSizeLimits(minWidth, maxH, minV, maxV);
    if (Frame().Width() < minWidth)
        ResizeTo(minWidth, Frame().Height());

    status_t const result = ReadSelf();
    if (result == B_OK)
        Show();
    else
    {
        (new BAlert("Error", B_TRANSLATE("An error occurred while reading the stub!"), B_TRANSLATE("OK")))->Go();
        be_app->PostMessage(B_QUIT_REQUESTED);
    }
}


bool JoinerWindow::QuitRequested()
{
    atomic_set((int32 *)&m_cancel, true);
    be_app->PostMessage(B_QUIT_REQUESTED);
    return BWindow::QuitRequested();
}


void JoinerWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_OPERATION_COMPLETE:
        {
            int32 result;
            status_t const resultFound = message->FindInt32(JoinerWindow::kJoinResult, &result);
            if (resultFound == B_OK)
            {
                if (result == BZR_DONE)
                {
                    BAlert* alert = new BAlert("Info", B_TRANSLATE("The file was joined successfully."),
                                               B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
                    alert->Go();
                }
                else if (result == BZR_CANCEL)
                {
                    BAlert* alert = new BAlert("Error", B_TRANSLATE("Joining of the file was cancelled."),
                                               B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
                    alert->Go();
                }
                else
                {
                    BAlert* alert = new BAlert("Error", B_TRANSLATE("An unknown error occurred while joining the files."),
                                               B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
                    alert->Go();
                }
            }
            else
            {
                BAlert* alert = new BAlert("Error", B_TRANSLATE("An internal error occurred while joining the files."),
                                           B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
                alert->Go();
            }
            PostMessage(B_QUIT_REQUESTED);
            break;
        }

        case M_CANCEL:
        {
            atomic_set((int32 *)&m_cancel, true);
            break;
        }

        case BZR_UPDATE_PROGRESS:
        {
            float const delta = message->FindFloat("delta");
            int8 const percent = (int8)ceil(100 * ((m_statusBar->CurrentValue() + delta) / m_statusBar->MaxValue()));

            BString percentStr;
            percentStr << percent << "%";

            BString const text = message->FindString("text");
            m_statusBar->Update(delta, text.String(), percentStr.String());
            message->SendReply('DUMB');
            break;
        }

        default:
            return BWindow::MessageReceived(message);
    }
}


status_t JoinerWindow::ReadSelf()
{
    // TODO: Better error messages for each failure.

    // Reads resource from itself (binary file)
    BResources* res = be_app->AppResources();
    if (res == NULL)
        return B_ERROR;

    size_t fileNameSize;
    const char* fileNameData = (const char*)res->LoadResource(B_STRING_TYPE, K_FILENAME_ATTRIBUTE, &fileNameSize);
    if (fileNameData == NULL || fileNameSize == 0)
        return B_ERROR;

    size_t separatorDataSize;
    const char* separatorData = (const char*)res->LoadResource(B_STRING_TYPE, K_SEPARATOR_ATTRIBUTE, &separatorDataSize);
    if (separatorData == NULL || separatorDataSize == 0)
        return B_ERROR;

    BString fileName(fileNameData, fileNameSize);
    m_separatorStr.SetTo(separatorData, separatorDataSize);

    if (fileName.Length() <= 0 || m_separatorStr.Length() <= 0)
        return B_ERROR;

    // Now read self (resources)
    BEntry appEntry;
    app_info appInfo;

    be_app->GetAppInfo(&appInfo);
    appEntry.SetTo(&(appInfo.ref), true);

    appEntry.GetParent(&appEntry);
    BPath outputDirPath;
    appEntry.GetPath(&outputDirPath);

    m_dirPathStr = outputDirPath.Path();
    m_chunkPathStr = m_dirPathStr;
    m_chunkPathStr << "/" << fileName;

    m_statusBar->Update(0, B_TRANSLATE("Computing file size" B_UTF8_ELLIPSIS));

    // Call findchunks to find out the total size of the joined file to assign to the progress bar
    int32 fileCount = 0;
    off_t totalSize = 0;
    FindChunks(m_chunkPathStr.String(), m_separatorStr.String(), fileCount, totalSize, &m_cancel);
    m_statusBar->SetMaxValue(totalSize);
    m_statusBar->Update(0, B_TRANSLATE("Joining:"));

    // Now we have all we want to start the join process, then what're we waiting for :)
    m_thread = spawn_thread(_joiner, "_joiner", B_NORMAL_PRIORITY, (void*)this);
    resume_thread(m_thread);
    return B_OK;
}


int32 JoinerWindow::_joiner(void* arg)
{
    JoinerWindow* wnd = (JoinerWindow*)arg;

    status_t const result = JoinFile(wnd->m_chunkPathStr.String(), wnd->m_dirPathStr.String(),
                                     wnd->m_separatorStr.String(), wnd->m_messenger, &(wnd->m_cancel));

    BMessage completeMsg(M_OPERATION_COMPLETE);
    completeMsg.AddInt32(JoinerWindow::kJoinResult, result);
    wnd->m_messenger->SendMessage(&completeMsg);

    return result;
}
