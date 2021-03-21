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

#include <Alert.h>
#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <ListView.h>
#include <Path.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

#include "AppConstants.h"
#include "BitmapPool.h"
#include "ImageButton.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsListItem.h"
#include "PrefsViewPaths.h"
#include "Shared.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewPaths"
#else
#define B_TRANSLATE(x) x
#endif


#define M_PATH_SELECTED               'pths'
#define M_SELECT_OPEN_PATH           'sopp'
#define M_SELECT_ADD_PATH           'sadp'
#define M_SELECT_EXTRACT_PATH        'setp'
#define M_USE_DIR                  'used'
#define M_ARK_DIR                  'arkd'
#define M_ADD_CLICKED               'addd'
#define M_REMOVE_CLICKED           'remc'

const char* const kTextCtrlPtr =    "txtctrptr";
const char* const kListCtrlPtr =    "lstctrptr";
const char* const kArkDir =           ":arkdir:";



PrefsViewPaths::PrefsViewPaths(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Paths"), B_TRANSLATE("Configure default & favorite paths"))
{
    m_messenger = NULL;
    m_message = NULL;
    m_openPanel = NULL;
    SetBitmap(BitmapPool::LoadSystemVector("application/x-vnd.Be-directory", 20, 20));
    Render();
}



PrefsViewPaths::~PrefsViewPaths()
{
    delete m_messenger;
    if (m_openPanel)
        delete m_openPanel;

    delete m_addBmp;
    delete m_removeBmp;
}



void PrefsViewPaths::Render()
{
    BStringView* defaultStrView = new BStringView(BRect(m_margin, m_margin, 0, 0), NULL,
            B_TRANSLATE("Default paths:"));
    defaultStrView->SetFont(&m_sectionFont);
    defaultStrView->ResizeToPreferred();
    defaultStrView->SetLowColor(ViewColor());

    int8 dividerStrCount = 3;
    BString dividerStrings[] =
    {
        B_TRANSLATE("Open path: "),
        B_TRANSLATE("Add path: "),
        B_TRANSLATE("Extract path: ")
    };

    float divider = -1;
    for (int8 i = 0; i < dividerStrCount - 1; i++)
        divider = MAX(StringWidth(dividerStrings[i].String()), StringWidth(dividerStrings[i+1].String()));

    divider += 8;

    m_openPathView = new BTextControl(BRect(3 * m_margin, defaultStrView->Frame().bottom + m_vGap + 6,
                                            Bounds().right - (2 * m_margin) - K_BUTTON_WIDTH, 0), "PrefsViewPaths:openPathView",
                                      B_TRANSLATE("Open path: "), NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP,
                                      B_WILL_DRAW | B_NAVIGABLE);
    m_openPathView->SetDivider(divider);
    m_openPathView->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
    m_openPathView->TextView()->DisallowChar(B_INSERT);

    BString buttonText = B_TRANSLATE("Select");
    m_openPathBtn = new BButton(BRect(m_openPathView->Frame().right + m_margin, m_openPathView->Frame().top - 4,
                                      m_openPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
                                      m_openPathView->Frame().top - 4 + K_BUTTON_HEIGHT), "PrefsViewPaths:openPathBtn",
                                buttonText.String(), new BMessage(M_SELECT_OPEN_PATH),
                                B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);

    m_addPathView = new BTextControl(BRect(3 * m_margin, m_openPathView->Frame().bottom + m_vGap + 6,
                                           m_openPathView->Frame().right, 0), "PrefsViewPaths:addPathView", B_TRANSLATE("Add path: "),
                                     NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_addPathView->SetDivider(divider);
    m_addPathView->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
    m_addPathView->TextView()->DisallowChar(B_INSERT);

    m_addPathBtn = new BButton(BRect(m_addPathView->Frame().right + m_margin, m_addPathView->Frame().top - 4,
                                     m_addPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
                                     m_addPathView->Frame().top - 4 + K_BUTTON_HEIGHT), "PrefsViewPaths:addPathBtn",
                               buttonText.String(), new BMessage(M_SELECT_ADD_PATH),
                               B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);

    BStringView* extractStrView = new BStringView(BRect(m_addPathView->Frame().left,
            m_addPathView->Frame().bottom + 2 * m_vGap + 10,
            m_addPathView->Frame().left + StringWidth(B_TRANSLATE("Extract path: ")), 0),
            "PrefsViewPaths:extractStrView", B_TRANSLATE("Extract path: "),
            B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
    extractStrView->ResizeToPreferred();

    m_arkDirOpt = new BRadioButton(BRect(extractStrView->Frame().left + 3 * m_margin,
                                         extractStrView->Frame().bottom + m_vGap, 0, 0), "PrefsViewPaths:arkDirOpt",
                                   B_TRANSLATE("Same folder as source (archive) file"), new BMessage(M_ARK_DIR), B_FOLLOW_LEFT | B_FOLLOW_TOP,
                                   B_WILL_DRAW | B_NAVIGABLE);
    m_arkDirOpt->ResizeToPreferred();

    m_useDirOpt = new BRadioButton(BRect(m_arkDirOpt->Frame().left, m_arkDirOpt->Frame().bottom + m_vGap + 1,
                                         0, 0), "PrefsViewPaths:useDirOpt", B_TRANSLATE("Use: "),
                                   new BMessage(M_USE_DIR), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_useDirOpt->ResizeToPreferred();
    m_useDirOpt->SetValue(B_CONTROL_ON);

    float strW = m_useDirOpt->Frame().right;
    strW = MAX(m_useDirOpt->Frame().right + 4, 3 * m_margin + divider);

    m_extractPathView = new BTextControl(BRect(strW, m_arkDirOpt->Frame().bottom + m_vGap,
                                         m_addPathView->Frame().right, 0), "PrefsViewPaths:extractPathView", NULL,
                                         NULL, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    m_extractPathView->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
    m_extractPathView->TextView()->DisallowChar(B_INSERT);

    m_extractPathBtn = new BButton(BRect(m_extractPathView->Frame().right + m_margin,
                                         m_extractPathView->Frame().top - 4,
                                         m_extractPathView->Frame().right + m_margin + K_BUTTON_WIDTH,
                                         m_extractPathView->Frame().top - 4 + K_BUTTON_HEIGHT),
                                   "PrefsViewPaths:extractPathBtn", buttonText.String(),
                                   new BMessage(M_SELECT_EXTRACT_PATH), B_FOLLOW_LEFT | B_FOLLOW_TOP,
                                   B_WILL_DRAW | B_NAVIGABLE);

    BStringView* favStrView = new BStringView(BRect(defaultStrView->Frame().left,
            m_extractPathView->Frame().bottom + m_margin + m_vGap + 8, 0, 0),
            "PrefsViewPaths:favStrView", B_TRANSLATE("Favorite extract paths:"),
            B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
    favStrView->SetFont(&m_sectionFont);
    favStrView->ResizeToPreferred();
    favStrView->SetLowColor(ViewColor());

    m_genChk = new BCheckBox(BRect(3 * m_margin, favStrView->Frame().bottom, 0, 0),
                             "PrefsViewPaths:genChk", B_TRANSLATE("List more paths (using archive name)"), NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP,
                             B_WILL_DRAW | B_NAVIGABLE);
    m_genChk->ResizeToPreferred();

    m_favListView = new BListView(BRect(5  * m_margin, favStrView->Frame().bottom + m_vGap + m_margin,
                                        m_openPathView->Frame().right - B_V_SCROLL_BAR_WIDTH - m_margin,
                                        Bounds().bottom - m_margin - B_H_SCROLL_BAR_HEIGHT - m_genChk->Frame().Height() - 6),
                                  "PrefsViewPaths:favListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT,
                                  B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);

    m_scrollView = new BScrollView("PrefsViewPaths:scrollView", m_favListView,
                                   B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS, true, true,
                                   B_FANCY_BORDER);

    m_genChk->MoveTo(m_genChk->Frame().left, m_scrollView->Frame().bottom + m_vGap + 4);

    m_addBmp = BitmapPool::LoadAppVector("Img:PlusSign", 20, 20);
    m_removeBmp = BitmapPool::LoadAppVector("Img:MinusSign", 20, 20);

    m_addBtn = new ImageButton(BRect(m_scrollView->Frame().right + m_margin, m_scrollView->Frame().top + 1,
                                     m_scrollView->Frame().right + 20, m_scrollView->Frame().top + 21), "PrefsViewPaths:addBnt",
                               NULL, m_addBmp, NULL, new BMessage(M_ADD_CLICKED), false, ViewColor(),
                               kBelowIcon, true, true, true, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);

    m_removeBtn = new ImageButton(BRect(m_addBtn->Frame().left, m_addBtn->Frame().bottom + 2 * m_margin - 3,
                                        m_addBtn->Frame().right, m_addBtn->Frame().bottom + m_margin + 21),
                                  "PrefsViewPaths:removeBtn", NULL, m_removeBmp, NULL, new BMessage(M_REMOVE_CLICKED), false,
                                  ViewColor(), kBelowIcon, true, true, true, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);

    AddChild(defaultStrView);
    AddChild(m_openPathView);
    AddChild(m_openPathBtn);
    AddChild(m_addPathView);
    AddChild(m_addPathBtn);
    AddChild(extractStrView);
    AddChild(m_arkDirOpt);
    AddChild(m_useDirOpt);
    AddChild(m_extractPathView);
    AddChild(m_extractPathBtn);
    AddChild(favStrView);
    AddChild(m_scrollView);
    AddChild(m_genChk);
    AddChild(m_addBtn);
    AddChild(m_removeBtn);
    AddRevertButton();
}



void PrefsViewPaths::Save()
{
    _prefs_paths.SetString(kPfDefOpenPath, m_openPathView->Text());
    _prefs_paths.SetString(kPfDefAddPath, m_addPathView->Text());
    _prefs_paths.SetString(kPfDefExtractPath, m_extractPathView->Text());
    _prefs_paths.SetBool(kPfUseArkDir, m_arkDirOpt->Value() == B_CONTROL_ON ? true : false);

    BMessage favPathMsg('fav!');
    for (int32 i = 0; i < m_favListView->CountItems(); i++)
        favPathMsg.AddString(kPath, ((PrefsListItem*)m_favListView->ItemAt(i))->Text());

    _prefs_paths.SetMessage(kPfFavPathsMsg, &favPathMsg);
    _prefs_paths.SetBool(kPfGenPath, IsChecked(m_genChk));
    _prefs_paths.WritePrefs();
}



void PrefsViewPaths::Load()
{
    const char* foundPath;
    if (_prefs_paths.FindString(kPfDefOpenPath, &foundPath) == B_OK)
        m_openPathView->SetText(foundPath);

    if (_prefs_paths.FindString(kPfDefAddPath, &foundPath) == B_OK)
        m_addPathView->SetText(foundPath);

    if (_prefs_paths.FindString(kPfDefExtractPath, &foundPath) == B_OK)
        m_extractPathView->SetText(foundPath);

    bool useArkDir;
    if (_prefs_paths.FindBool(kPfUseArkDir, &useArkDir) == B_OK)
    {
        if (useArkDir == true)
            m_arkDirOpt->SetValue(B_CONTROL_ON);
        else
            m_useDirOpt->SetValue(B_CONTROL_ON);

        // m_useDirOpt->Invoke()won't work because SetTarget() in AttchdToWnd() would not yet have been exec
        ToggleExtractPathView(!useArkDir);
    }

    BMessage favPathMsg;
    if (_prefs_paths.FindMessage(kPfFavPathsMsg, &favPathMsg) == B_OK)
    {
        m_favListView->MakeEmpty();
        int32 i = 0L;
        while (favPathMsg.FindString(kPath, i++, &foundPath) == B_OK)
            m_favListView->AddItem(new PrefsListItem(foundPath, NULL, false));
    }

    m_genChk->SetValue(_prefs_paths.FindBoolDef(kPfGenPath, true));
}



void PrefsViewPaths::AttachedToWindow()
{
    m_messenger = new BMessenger(this);
    m_openPathBtn->SetTarget(this);
    m_addPathBtn->SetTarget(this);
    m_extractPathBtn->SetTarget(this);
    m_arkDirOpt->SetTarget(this);
    m_useDirOpt->SetTarget(this);
    m_addBtn->SetTarget(this);
    m_removeBtn->SetTarget(this);
    PrefsView::AttachedToWindow();
}



void PrefsViewPaths::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_SELECT_OPEN_PATH: case M_SELECT_ADD_PATH: case M_SELECT_EXTRACT_PATH: case M_ADD_CLICKED:
        {
            if (m_openPanel == NULL)
            {
                m_openPanel = new BFilePanel(B_OPEN_PANEL, m_messenger, NULL, B_DIRECTORY_NODE, false, NULL,
                                             NULL, true, true);
                m_openPanel->SetButtonLabel(B_DEFAULT_BUTTON, B_TRANSLATE("Select"));
            }

            if (m_message == NULL)
                m_message = new BMessage(M_PATH_SELECTED);
            else
                m_message->RemoveName(kTextCtrlPtr);

            BString panelWindowTitle;
            if (message->what == M_SELECT_OPEN_PATH)
            {
                m_message->AddPointer(kTextCtrlPtr, m_openPathView);
                panelWindowTitle = B_TRANSLATE("Select default open path");
            }
            else if (message->what == M_SELECT_ADD_PATH)
            {
                m_message->AddPointer(kTextCtrlPtr, m_addPathView);
                panelWindowTitle = B_TRANSLATE("Select default add path");
            }
            else if (message->what == M_SELECT_EXTRACT_PATH)
            {
                m_message->AddPointer(kTextCtrlPtr, m_extractPathView);
                panelWindowTitle = B_TRANSLATE("Select default extract path");
            }
            else if (message->what == M_ADD_CLICKED)
            {
                m_message->AddPointer(kListCtrlPtr, m_favListView);
                panelWindowTitle = B_TRANSLATE("Add a favorite extract path");
            }

            m_openPanel->Window()->SetTitle(panelWindowTitle.String());
            m_openPanel->SetMessage(m_message);
            m_openPanel->Show();
            break;
        }

        case M_PATH_SELECTED:
        {
            BTextControl* textControl(NULL);
            BListView* listControl(NULL);
            entry_ref ref;
            message->FindRef("refs", &ref);
            message->FindPointer(kTextCtrlPtr, reinterpret_cast<void**>(&textControl));
            message->FindPointer(kListCtrlPtr, reinterpret_cast<void**>(&listControl));

            BPath pathOfRef(&ref);
            if (textControl)
                textControl->SetText(pathOfRef.Path());
            else
            {
                bool isUnique = true;
                // check if the path is already not there in the list, only then add it
                for (int32 i = 0; i < listControl->CountItems(); i++)
                {
                    BString existingPath = ((PrefsListItem*)listControl->ItemAt(i))->Text();

                    // don't add if same path is being added
                    if (strcmp(existingPath.String(), pathOfRef.Path()) == 0)
                    {
                        BString errString = B_TRANSLATE("%filepath% is already present in your favorites");
                        errString.ReplaceAll("%filepath%", existingPath.String());
                        BAlert* errAlert = new BAlert("error", errString.String(), B_TRANSLATE("OK"),
                                                      NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
                        errAlert->SetShortcut(0L, B_ESCAPE);
                        errAlert->Go();
                        isUnique = false;
                        break;
                    }
                }

                // Add only paths that are already not present
                if (isUnique)
                {
                    int32 selIndex = listControl->CurrentSelection(0L);
                    if (selIndex >= 0)
                        listControl->AddItem(new PrefsListItem(pathOfRef.Path(), NULL, false), selIndex);
                    else
                        listControl->AddItem(new PrefsListItem(pathOfRef.Path(), NULL, false));
                }
            }

            break;
        }


        case M_USE_DIR:
        {
            ToggleExtractPathView(true);
            break;
        }

        case M_ARK_DIR:
        {
            ToggleExtractPathView(false);
            break;
        }

        case M_REMOVE_CLICKED:
        {
            int32 selIndex = m_favListView->CurrentSelection(0L);
            if (selIndex >= 0L)
            {
                m_favListView->RemoveItem(selIndex);
                if (m_favListView->CountItems() - 1 >= selIndex)
                    m_favListView->Select(selIndex, false);
                else if (m_favListView->CountItems() > 0)
                    m_favListView->Select(m_favListView->CountItems() - 1, false);
            }

            break;
        }

        default:
            PrefsView::MessageReceived(message);
            break;
    }
}



void PrefsViewPaths::ToggleExtractPathView(bool enable)
{
    m_extractPathView->SetEnabled(enable);
    m_extractPathBtn->SetEnabled(enable);
}


