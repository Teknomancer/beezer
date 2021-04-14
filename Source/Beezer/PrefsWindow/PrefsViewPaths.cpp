// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsViewPaths.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "ImageButton.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsListItem.h"
#include "Shared.h"

#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Path.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewPaths"
#else
#define B_TRANSLATE(x) x
#endif

static const uint32 M_PATH_SELECTED       = 'pths';
static const uint32 M_SELECT_OPEN_PATH    = 'sopp';
static const uint32 M_SELECT_ADD_PATH     = 'sadp';
static const uint32 M_SELECT_EXTRACT_PATH = 'setp';
static const uint32 M_ARK_DIR             = 'arkd';
static const uint32 M_ADD_CLICKED         = 'addd';
static const uint32 M_REMOVE_CLICKED      = 'remc';

static const char* const kTextCtrlPtr = "txtctrptr";
static const char* const kListCtrlPtr = "lstctrptr";
static const char* const kArkDir      = ":arkdir:";


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
    BStringView* defaultStrView = new BStringView("PrefsViewPath:defaultStrView", B_TRANSLATE("Default paths"), B_WILL_DRAW);
    defaultStrView->SetFont(&m_sectionFont);

    m_openPathView = new BTextControl("PrefsViewPaths:openPathView", B_TRANSLATE("Open path:"), NULL, NULL,
                                      B_WILL_DRAW | B_NAVIGABLE);
    m_openPathView->TextView()->DisallowChar(B_INSERT);
    m_openPathView->TextView()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, m_openPathView->TextView()->MinSize().Height()));

    BString const selectBtnText = BZ_TR(kSelectString);
    m_openPathBtn = new BButton("PrefsViewPaths:openPathBtn", selectBtnText.String(),
                                new BMessage(M_SELECT_OPEN_PATH), B_WILL_DRAW | B_NAVIGABLE);

    m_addPathView = new BTextControl("PrefsViewPaths:addPathView", B_TRANSLATE("Add path:"), NULL, NULL,
                                     B_WILL_DRAW | B_NAVIGABLE);
    m_addPathView->TextView()->DisallowChar(B_INSERT);
    m_addPathView->TextView()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, m_addPathView->TextView()->MinSize().Height()));

    m_addPathBtn = new BButton("PrefsViewPaths:addPathBtn", selectBtnText.String(), new BMessage(M_SELECT_ADD_PATH),
                               B_WILL_DRAW | B_NAVIGABLE);

    m_extractPathView = new BTextControl("PrefsViewPaths:extractPathView", B_TRANSLATE("Extract path:"), NULL, NULL,
                                         B_WILL_DRAW | B_NAVIGABLE);
    m_extractPathView->TextView()->DisallowChar(B_INSERT);
    m_extractPathView->TextView()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, m_extractPathView->TextView()->MinSize().Height()));

    m_extractPathBtn = new BButton("PrefsViewPaths:extractPathBtn", selectBtnText.String(),
                                   new BMessage(M_SELECT_EXTRACT_PATH), B_WILL_DRAW | B_NAVIGABLE);

    m_arkDirChk = new BCheckBox("PrefsViewPaths:arkDirOpt", B_TRANSLATE("Extract to the same folder as the source archive"), new BMessage(M_ARK_DIR));

    BStringView* favStrView = new BStringView("PrefsViewPaths:favStrView", B_TRANSLATE("Favorite extract paths"),
                                              B_WILL_DRAW);
    favStrView->SetFont(&m_sectionFont);

    m_genChk = new BCheckBox("PrefsViewPaths:genChk", B_TRANSLATE("List more paths (using archive name)"),
                             NULL, B_WILL_DRAW | B_NAVIGABLE);

    m_favListView = new BListView("PrefsViewPaths:favListView", B_SINGLE_SELECTION_LIST,
                                  B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);

    m_scrollView = new BScrollView("PrefsViewPaths:scrollView", m_favListView,
                                   B_WILL_DRAW | B_FRAME_EVENTS, true /* horiz */, true /* vert */);

    m_addBmp = BitmapPool::LoadAppVector("Img:PlusSign", 20, 20);
    m_removeBmp = BitmapPool::LoadAppVector("Img:MinusSign", 20, 20);

    m_addBtn = new ImageButton("PrefsViewPaths:addBnt", NULL, m_addBmp, NULL, new BMessage(M_ADD_CLICKED),false, ViewColor());
    m_addBtn->SetExplicitSize(BSize(30, 30));

    m_removeBtn = new ImageButton("PrefsViewPaths:removeBtn", NULL, m_removeBmp, NULL, new BMessage(M_REMOVE_CLICKED), false,
                                  ViewColor());
    m_removeBtn->SetExplicitSize(BSize(30, 30));

    // Using 0 spacing in main vertical layout below to pack certain items (like
    // the select buttons) tightly as otherwise the layout gets weird.
    // We'll explicitly add half-item spacing instead.
    BLayoutBuilder::Group<> builder = BLayoutBuilder::Group<>(this, B_VERTICAL, 0);
    builder
        .SetInsets(B_USE_DEFAULT_SPACING)
        .Add(defaultStrView)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(B_USE_HALF_ITEM_SPACING)
            .AddGrid(0.0, B_USE_HALF_ITEM_SPACING)
                .AddTextControl(m_openPathView, 0, 0, B_ALIGN_RIGHT)
                .Add(m_openPathBtn, 3, 0)
                .AddTextControl(m_addPathView, 0, 1, B_ALIGN_RIGHT)
                .Add(m_addPathBtn, 3, 1)
                .AddTextControl(m_extractPathView, 0, 2, B_ALIGN_RIGHT)
                .Add(m_extractPathBtn, 3, 2)
                .AddGroup(B_HORIZONTAL, 0.0, 1, 4, 3, 1)
                    .Add(m_arkDirChk)
                    .AddGlue()
                .End()
                .Add(BSpaceLayoutItem::CreateHorizontalStrut(5), 2, 0, 1, 3) // 3 row spacer between inputs and select buttons
            .End()
        .End()
        .AddStrut(B_USE_ITEM_SPACING)  // extra space before starting next logical group
        .Add(favStrView)
        .AddStrut(B_USE_HALF_ITEM_SPACING)
        .AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
            .AddStrut(B_USE_ITEM_SPACING)
            .Add(m_scrollView)
            .AddGroup(B_VERTICAL, 0)
                .Add(m_addBtn)
                .Add(m_removeBtn)
                .AddGlue()
            .End()
        .End()
        .AddStrut(B_USE_HALF_ITEM_SPACING)
        .AddGroup(B_HORIZONTAL)
            .AddStrut(B_USE_ITEM_SPACING)
            .Add(m_genChk)
            .AddGlue()
        .End()        // don't add glue below so m_scrollView stretches to fill up vertical space
        .End();

    AddRevertButton(builder);
}


void PrefsViewPaths::Save()
{
    _prefs_paths.SetString(kPfDefOpenPath, m_openPathView->Text());
    _prefs_paths.SetString(kPfDefAddPath, m_addPathView->Text());
    _prefs_paths.SetString(kPfDefExtractPath, m_extractPathView->Text());
    _prefs_paths.SetBool(kPfUseArkDir, m_arkDirChk->Value() == B_CONTROL_ON ? true : false);

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
            m_arkDirChk->SetValue(B_CONTROL_ON);
        else
            m_arkDirChk->SetValue(B_CONTROL_OFF);

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
    m_arkDirChk->SetTarget(this);
    m_addBtn->SetTarget(this);
    m_removeBtn->SetTarget(this);
    PrefsView::AttachedToWindow();
}


void PrefsViewPaths::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case M_SELECT_OPEN_PATH:
        case M_SELECT_ADD_PATH:
        case M_SELECT_EXTRACT_PATH:
        case M_ADD_CLICKED:
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
                        BAlert* errAlert = new BAlert("Error", errString.String(), BZ_TR(kOKString),
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

        case M_ARK_DIR:
        {
            ToggleExtractPathView(m_arkDirChk->Value() == B_CONTROL_ON ? false : true);
            break;
        }

        case M_REMOVE_CLICKED:
        {
            int32 const selIndex = m_favListView->CurrentSelection(0L);
            if (selIndex >= 0)
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
