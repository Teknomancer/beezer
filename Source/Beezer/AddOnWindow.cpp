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
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <Debug.h>
#include <Entry.h>
#include <Font.h>
#include <List.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <String.h>
#include <TextControl.h>
#include <TranslationUtils.h>
#include <interface/StringView.h>

#include "AddOnWindow.h"
#include "AppConstants.h"
#include "ArchiveRep.h"
#include "Archiver.h"
#include "ArchiverMgr.h"
#include "BarberPole.h"
#include "BevelView.h"
#include "LocalUtils.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AddOnWindow"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


AddOnWindow::AddOnWindow(BMessage* refsMessage)
    : BWindow(BRect(10, 10, 420, 475), B_TRANSLATE("QuickCreate archive"), B_TITLED_WINDOW,
              B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
    m_arkSettingsMenuField(NULL),
    m_archive(NULL),
    m_readyMode(true),
    m_inProgress(false),
    m_quitNow(false)
{
    m_backView = new BevelView(Bounds(), "AddOnWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    AddChild(m_backView);
    m_backView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    // backViewMain will hold all controls that are to be shown when files/folders are really selected
    // otherwise it will be hidden and a message like "Drop files here to archive" will be shown using
    // the alternate Back View
    m_backViewMain = new BevelView(Bounds().InsetByCopy(4, 2),
                                   "AddOnWindow:BackViewMain", btNoBevel, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->AddChild(m_backViewMain);
    m_backViewMain->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    float divider = 0;
    divider = MAX(divider, m_backViewMain->StringWidth(B_TRANSLATE("Archive name:")));
    divider = MAX(divider, m_backViewMain->StringWidth(B_TRANSLATE("Archive type:")));
    divider = MAX(divider, m_backViewMain->StringWidth(B_TRANSLATE("Password:")));
    divider += 10;

    m_fileName = new BTextControl(BRect(K_MARGIN, 2 * K_MARGIN, 0, 0), "AddOnWindow:fileName",
                                  B_TRANSLATE("Archive name:"), NULL, NULL, B_FOLLOW_LEFT_RIGHT,
                                  B_WILL_DRAW);
    m_backViewMain->AddChild(m_fileName);
    m_fileName->SetModificationMessage(new BMessage(M_FILENAME_CHANGED));
    m_fileName->ResizeToPreferred();
    m_fileName->ResizeTo(m_backViewMain->Frame().Width() - 2 * K_MARGIN, m_fileName->Frame().Height());
    m_fileName->SetDivider(divider);
    m_fileName->TextView()->DisallowChar(':');
    m_fileName->TextView()->DisallowChar('*');
    m_fileName->TextView()->DisallowChar('?');

    m_arkTypes = ArchiversInstalled(&m_arkExtensions);
    m_arkTypePopUp = BuildArchiveTypesMenu(this, &m_arkExtensions);
    m_arkTypeField = new BMenuField(BRect(K_MARGIN, m_fileName->Frame().bottom + K_MARGIN,
                                          m_backViewMain->Frame().right, 0), "AddOnWindow:ArkTypeField",
                                    B_TRANSLATE("Archive type:"), (BMenu*)m_arkTypePopUp, B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backViewMain->AddChild(m_arkTypeField);
    m_arkTypeField->ResizeToPreferred();
    m_arkTypeField->SetDivider(divider + 2);

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
        m_fileName->SetText((char*)m_arkExtensions.ItemAtFast(m_arkTypePopUp->IndexOf(item)));

        // Interface headache! Calculate maximum width of labels of each archiver listed in the
        // ark types field, grr!!
        m_strWidthOfArkTypes = 0.0f;
        for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
        {
            const char* name = (const char*)m_arkTypes.ItemAtFast(i);
            m_strWidthOfArkTypes = MAX(m_strWidthOfArkTypes, m_backViewMain->StringWidth(name));
        }
        m_strWidthOfArkTypes += m_arkTypeField->Divider() + 2 * K_MARGIN + 35;
    }
    else
    {
        // We've seen enough there are no add-ons installed, no use proceeding!!! Call it quits NOW
        BString alertStr(B_TRANSLATE("Fatal error, no add-ons found! You cannot create any archives using %appname%."));
        alertStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
        (new BAlert("Error", alertStr, B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
        PostMessage(B_QUIT_REQUESTED);
        Show();
        return;
    }

    m_password = new BTextControl(BRect(K_MARGIN, m_arkTypeField->Frame().bottom + K_MARGIN,
                                        divider + 2 * K_MARGIN + 80, 0), "AddOnWindow:Password", B_TRANSLATE("Password:"), NULL,
                                  NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_backViewMain->AddChild(m_password);
    m_password->TextView()->HideTyping(true);
    m_password->SetDivider(divider);

    m_backViewMain->ResizeTo(m_backViewMain->Frame().Width(), m_password->Frame().bottom + K_MARGIN);

    BFont font;
    m_backView->GetFont(&font);
    font_height fntHt;
    font.GetHeight(&fntHt);
    float fontHeight = fntHt.ascent + fntHt.descent + 1;
    BevelView* sepView2 = new BevelView(BRect(-1, Bounds().bottom - fontHeight - 4, Bounds().right,
                                        Bounds().bottom - fontHeight - 3),
                                        "AddOnWindow:sepView2", btInset, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
                                        B_WILL_DRAW);
    m_backView->AddChild(sepView2);

    BevelView* sepView = new BevelView(BRect(-1, sepView2->Frame().bottom - 2 * K_MARGIN - K_BUTTON_HEIGHT -
                                       4 - 8, Bounds().right, sepView2->Frame().bottom - 2 * K_MARGIN
                                       - K_BUTTON_HEIGHT - 3 - 8),    "AddOnWindow:sepView", btInset,
                                       B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backViewMain->AddChild(sepView);
    sepView->Hide();

    m_helpBtn = new BButton(BRect(K_MARGIN, sepView2->Frame().bottom - K_MARGIN - K_BUTTON_HEIGHT,
                                  K_MARGIN + K_BUTTON_WIDTH, sepView2->Frame().bottom - K_MARGIN),
                            "AddOnWindow:HelpButton", B_TRANSLATE("Help"), new BMessage(M_ADDON_HELP),
                            B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(m_helpBtn);

    m_createBtn = new BButton(BRect(Bounds().right - K_MARGIN - 4 - K_BUTTON_WIDTH,
                                    sepView2->Frame().bottom - K_MARGIN - K_BUTTON_HEIGHT - 4,
                                    Bounds().right - K_MARGIN - 4, sepView2->Frame().bottom - K_MARGIN - 4),
                              "AddOnWindow:CreateBtn", B_TRANSLATE("Create"), new BMessage(M_ADDON_CREATE),
                              B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild(m_createBtn);
    m_createBtn->MakeDefault(true);
    m_createBtn->SetEnabled(false);

    m_helpBtn->MoveBy(0, -4);

    if (refsMessage->CountNames(B_REF_TYPE) == 0)            // No refs were selected
    {
        m_readyMode = false;
        m_backViewMain->Hide();

        // Add a control to ask for dropping of files
        m_backViewAlt = new BevelView(Bounds().InsetByCopy(2, 2),
                                      "AddOnWindow:BackViewAlt", btInset, B_FOLLOW_LEFT, B_WILL_DRAW);
        m_backViewAlt->ResizeBy(0, -(2 * K_MARGIN + K_BUTTON_HEIGHT + 4 + fontHeight + 2));
        m_backViewAlt->SetResizingMode(B_FOLLOW_ALL_SIDES);
        m_backViewAlt->SetViewColor(0, 0, 0, 255);

        BStringView* shadowStr = new BStringView(BRect(2 * K_MARGIN, 4 * K_MARGIN, 0, 0),
                "AddOnWindow:DropStr", B_TRANSLATE("Drop files to create an archive."), B_FOLLOW_LEFT,
                B_WILL_DRAW);
        shadowStr->SetFont(be_bold_font);
        shadowStr->SetFontSize(font.Size() + 4);
        shadowStr->SetHighColor(145, 149, 183, 255);
        shadowStr->ResizeToPreferred();
        m_backViewAlt->AddChild(shadowStr);

        BStringView* dropStr = new BStringView(BRect(2 * K_MARGIN, 4 * K_MARGIN, 0, 0), "AddOnWindow:DropStr",
                                               B_TRANSLATE("Drop files to create an archive."), B_FOLLOW_LEFT, B_WILL_DRAW);
        dropStr->SetFont(be_bold_font);
        dropStr->SetFontSize(font.Size() + 4);
        dropStr->SetHighColor(K_STARTUP_MAIN_HEADING);
        dropStr->ResizeToPreferred();
        m_backViewAlt->AddChild(dropStr);

        m_backView->AddChild(m_backViewAlt);

        BBitmap* bmp = BTranslationUtils::GetBitmap('PNG ', "Img:Background");
        m_backViewAlt->SetViewBitmap(bmp);
        dropStr->SetViewBitmap(bmp);
        shadowStr->SetViewBitmap(bmp);
        delete bmp;

        // Resize window
        ResizeTo(MAX(350, dropStr->Frame().right + 2 * K_MARGIN + 2), Frame().Height());
    }

    BString statusString = B_TRANSLATE("Ready to create archive.");
    m_statusColor = SC_READY;
    if (m_readyMode == false)
    {
        statusString = B_TRANSLATE("Waiting for input files.");
        m_statusColor = SC_NOT_READY;
    }

    m_statusStr = new BStringView(BRect(K_MARGIN, Bounds().bottom - 2 - fontHeight, 0, 0),
                                  "AddOnWindow:StatusStr", statusString.String(),
                                  B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild(m_statusStr);
    m_statusStr->SetLowColor(m_backView->ViewColor());
    m_statusStr->SetHighColor(m_statusColor);
    m_statusStr->SetFontSize(MIN(10, be_plain_font->Size() - 1));
    m_statusStr->ResizeToPreferred();

    m_addView = new BevelView(BRect(K_MARGIN, m_password->Frame().bottom + K_MARGIN,
                                    m_backViewMain->Bounds().right - K_MARGIN, sepView->Frame().top - K_MARGIN),
                              "AddOnWindow:AddView", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backViewMain->AddChild(m_addView);

    m_barberPole = new BarberPole(BRect(K_MARGIN, K_MARGIN, 0, K_MARGIN + fontHeight + K_MARGIN),
                                  "AddOnWindow:BarberPole");
    m_barberPole->ResizeToPreferred();
    m_addView->AddChild(m_barberPole);

    m_addingFileStr = new BStringView(BRect(m_barberPole->Frame().right + K_MARGIN, K_MARGIN + 1, 0, 0),
                                      "AddOnWindow:AddingFileStr", B_TRANSLATE("Adding:"), B_FOLLOW_LEFT,
                                      B_WILL_DRAW);
    m_addingFileStr->ResizeToPreferred();
    m_addView->AddChild(m_addingFileStr);

    m_addView->ResizeTo(m_addView->Frame().Width(), m_barberPole->Frame().bottom + K_MARGIN);
    m_addView->Hide();

    ResizeTo(Frame().Width(), m_password->Frame().bottom + 3 * K_MARGIN + K_BUTTON_HEIGHT +
             m_statusStr->Frame().Height() + 8);
    m_backViewMain->ResizeTo(m_backViewMain->Frame().Width(), m_helpBtn->Frame().top - K_MARGIN);

    // Center window on-screen
    CenterOnScreen();

    // Constrain the size of the window
    float minH, maxH, minV, maxV;
    float w = 3 * K_MARGIN + 2 * K_BUTTON_WIDTH + 16;
    w = MAX(w, Bounds().Width());
    GetSizeLimits(&minH, &maxH, &minV, &maxV);
    SetSizeLimits(w, maxH, Bounds().Height(), maxV);

    if (m_readyMode == true)
        RefsReceived(refsMessage);

    // Show could also be called ABOVE!!!! (when no addons are found)
    CreateArchiveRepAndMenus();
    Show();
}



bool AddOnWindow::QuitRequested()
{
    if (m_inProgress == true && m_quitNow == false)    // m_quitNow is checked so this isn't called twice
    {
        // We cannot pause the creation thread as we cannot get "zip", "gzip"s thread ID, atbest we can
        // only get thread ID of the thread that spawned zip etc., so forget it for the time being
        BAlert* alert = new BAlert("Quit", B_TRANSLATE("Operation is in progress, force it to stop?"),
                                   B_TRANSLATE("Cancel"), B_TRANSLATE("Force"), NULL,
                                   B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->SetShortcut(0L, B_ESCAPE);
        alert->SetDefaultButton(alert->ButtonAt(1L));
        int32 index = alert->Go();
        if (index == 1L)
        {
            m_cancel = true;
            m_createBtn->SetEnabled(false);
        }

        m_quitNow = true;
        return false;
    }
    else if (m_quitNow)    // Incase QuitRequested is called a second time, still don't do anything silly
        return false;
    else
        return BWindow::QuitRequested();
}



void AddOnWindow::Quit()
{
    m_cancel = true;
    for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
        free((char*)m_arkExtensions.ItemAtFast(i));

    be_app_messenger.SendMessage(M_CLOSE_ADDON);
    return BWindow::Quit();
}



void AddOnWindow::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case BZR_UPDATE_PROGRESS:
        {
            const char* mainText;
            if (message->FindString("text", &mainText) != B_OK)
                mainText = "";

            BString buf = B_TRANSLATE("Adding:");
            buf << " " << mainText;
            m_addingFileStr->SetText(buf.String());
            m_addingFileStr->ResizeToPreferred();
            break;
        }

        case M_CLOSE:
        {
            m_inProgress = false;
            Quit();
            break;
        }

        case M_ADDON_CREATE:
        {
            if (m_inProgress == false)
            {
                if (m_archive)
                {
                    m_inProgress = true;
                    UpdateStatus(B_TRANSLATE("Adding files to the archive"));
                    m_createBtn->SetLabel(B_TRANSLATE("Cancel"));
                    m_createBtn->MakeDefault(false);
                    m_cancel = false;
                    EnableControls(false);

                    BMessage* addMessage = new BMessage('addd');     // no need to delete as it will be posted to
                    // us as M_ADD_DONE

                    // Pull the first ref's directory from the list of refs
                    entry_ref ref;
                    m_refsMessage.FindRef("refs", 0, &ref);
                    BEntry entry(&ref, false);
                    entry.GetParent(&entry);         // strange, but legal
                    BPath launchDirPath;
                    entry.GetPath(&launchDirPath);

                    BMessenger messenger(this);
                    addMessage->AddMessenger(kProgressMessenger, messenger);
                    addMessage->AddString(kArchivePath, m_fileName->Text());
                    addMessage->AddPointer(kLooperPtr, this);
                    addMessage->AddPointer(kCancel, (void*)&m_cancel);
                    addMessage->AddString(kLaunchDir, launchDirPath.Path());
                    if (m_password->TextView()->TextLength() > 0)
                        m_archive->Ark()->SetPassword(m_password->Text());

                    uint32 type;
                    int32 count;
                    entry_ref tmpRef;
                    m_refsMessage.GetInfo("refs", &type, &count);
                    for (int32 i = --count; i >= 0; i--)
                        if (m_refsMessage.FindRef("refs", i, &tmpRef) == B_OK)
                            addMessage->AddString(kPath, tmpRef.name);

                    // Deleting existing file if any
                    BEntry temp(m_fileName->Text());
                    if (temp.Exists())
                        temp.Remove();

                    ResizeBy(0, m_addView->Frame().Height());
                    m_barberPole->SetValue(true, false);
                    m_addView->Show();

                    m_archive->Create(addMessage);
                }
            }
            else
            {
                m_cancel = true;
                m_inProgress = false;
                m_createBtn->MakeDefault(true);
                m_createBtn->SetLabel(B_TRANSLATE("Create"));

                EnableControls(true);
                PostMessage(B_QUIT_REQUESTED);
            }

            break;
        }

        case B_REFS_RECEIVED:
        {
            RefsReceived(message);
            break;
        }

        case M_FILENAME_CHANGED:
        {
            ValidateData();
            break;
        }

        case M_ADDON_HELP:
        {
            // A cry for help! we aren't very helpful but atleast we can pass on the message :)
            be_app->PostMessage(message);
            break;
        }

        case M_ARK_TYPE_SELECTED:
        {
            ReplaceExtensionWith(message->FindString(kText));
            CreateArchiveRepAndMenus();
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

        case B_SIMPLE_DATA:
        {
            if (m_readyMode == false && m_inProgress == false)
            {
                m_backViewAlt->Hide();
                m_backViewMain->Show();
                message->what = B_REFS_RECEIVED;
                m_readyMode = true;
                CreateArchiveRepAndMenus();
                PostMessage(message);
            }

            break;
        }

        default:
            return BWindow::MessageReceived(message);
            break;
    }
}



void AddOnWindow::EnableControls(bool enable)
{
    m_fileName->SetEnabled(enable);
    m_arkTypeField->SetEnabled(enable);

    if (m_arkSettingsMenuField)
        m_arkSettingsMenuField->SetEnabled(enable);

    m_password->SetEnabled(enable);
}



void AddOnWindow::CreateArchiveRepAndMenus()
{
    if (m_readyMode == false)
        return;

    if (m_arkSettingsMenuField)
    {
        m_backViewMain->RemoveChild(m_arkSettingsMenuField);
        delete m_arkSettingsMenuField;
        m_arkSettingsMenuField = NULL;
    }

    if (m_archive != NULL)
    {
        delete m_archive;
        m_archive = NULL;
    }

    m_archive = new ArchiveRep();
    status_t result = m_archive->InitArchiver(m_arkTypePopUp->FindMarked()->Label(), true);
    if (result == BZR_DONE)
    {
        // **** Later move this to the place where all archivers are added to the m_arkType menu!!!!!!
        if (m_archive->Ark()->SettingsMenu())
        {
            m_arkSettingsMenuField = new BMenuField(BRect(m_strWidthOfArkTypes,
                                                    m_arkTypeField->Frame().top, m_backViewMain->Frame().right, 0),
                                                    "AddOnWindow:ArkSettingsMenuField", B_TRANSLATE("Settings:"),
                                                    m_archive->Ark()->SettingsMenu(), B_FOLLOW_LEFT,
                                                    B_WILL_DRAW | B_NAVIGABLE);
            m_backViewMain->AddChild(m_arkSettingsMenuField);
            m_arkSettingsMenuField->ResizeToPreferred();
            m_arkSettingsMenuField->SetDivider(m_backViewMain->StringWidth(m_arkSettingsMenuField->Label()) + 10);
            SetTargetForMenuRecursive(m_archive->Ark()->SettingsMenu(), this);
        }

        if (m_archive && m_archive->Ark()->SupportsPassword() == true)
            m_password->SetEnabled(true);
        else
        {
            m_password->SetEnabled(false);
            m_password->SetText(NULL);
        }
    }
    else
    {
        delete m_archive;
        m_archive = NULL;
    }
}



bool AddOnWindow::ReplaceExtensionWith(const char* newExt)
{
    int32 start, end;
    BTextView* fileNameView = m_fileName->TextView();
    fileNameView->GetSelection(&start, &end);

    BString existingName = m_fileName->Text();
    const char* newExtStr = newExt;
    bool replacedExt = false;

    for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
    {
        int32 extLocation = existingName.FindLast((char*)m_arkExtensions.ItemAtFast(i));
        if (extLocation >= 0L
                // make sure it's exactly the same. otherwise extensions like tar get found before tar.xz
                && (existingName.Length() - extLocation == strlen((char*)m_arkExtensions.ItemAtFast(i)))
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
            replacedExt = true;
            break;
        }
    }

    if (replacedExt == false)
    {
        BString buf = existingName;
        buf << newExtStr;
        fileNameView->SetText(buf.String());
        fileNameView->Select(start, end);
    }

    return replacedExt;
}



void AddOnWindow::UpdateStatus(const char* text)
{
    if (strcmp(text, B_TRANSLATE("Warning! File already exists and will be overwritten!")) == 0)
        m_statusStr->SetHighColor(SC_OVERWRITE);
    else if (strcmp(text, B_TRANSLATE("Ready to create archive.")) == 0)
        m_statusStr->SetHighColor(SC_READY);
    else if (strcmp(text, B_TRANSLATE("Error! Destination folder is incorrect.")) == 0)
        m_statusStr->SetHighColor(SC_DIR_MISSING);
    else if (strcmp(text, B_TRANSLATE("Adding files to the archive")) == 0)
        m_statusStr->SetHighColor(SC_BUSY);
    else
        m_statusStr->SetHighColor(0, 0, 0, 255);

    m_statusStr->SetText(text);
    m_statusStr->ResizeToPreferred();
}



void AddOnWindow::ValidateData()
{
    if (m_inProgress)
        return;

    bool valid = true;
    if (m_readyMode == true)
    {
        BEntry entry(m_fileName->Text(), false);
        if (entry.Exists() == true)
            UpdateStatus(B_TRANSLATE("Warning! File already exists and will be overwritten!"));
        else
            UpdateStatus(B_TRANSLATE("Ready to create archive."));

        BString buf = m_fileName->Text();
        int32 found = buf.FindLast('/');
        buf.Remove(found, buf.Length() - found);

        m_archiveDirPath = buf;
        bool doubleSlash = false;
        if (m_archiveDirPath[m_archiveDirPath.Length() - 1] == '/')    // error as it is like /boot//a.zip
            doubleSlash = true;

        BEntry dirEntry(m_archiveDirPath.String(), false);

        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false || doubleSlash == true)
        {
            UpdateStatus(B_TRANSLATE("Error! Destination folder is incorrect."));
            valid = false;
        }
    }
    else
    {
        valid = false;
        UpdateStatus(B_TRANSLATE("Waiting for input files."));
    }

    if (m_readyMode && valid == true)
        m_createBtn->SetEnabled(true);
    else
        m_createBtn->SetEnabled(false);
}



void AddOnWindow::RefsReceived(BMessage* message)
{
    // It is now in ready mode
    uint32 type;
    int32 count;
    entry_ref ref;

    message->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return;

    // Pull the first ref's directory from the list of refs
    message->FindRef("refs", 0, &ref);
    BEntry entry(&ref, false);
    entry.GetParent(&entry);         // strange, but legal
    BPath path;
    entry.GetPath(&path);
    m_archiveDirPath = path.Path();

    // OKAY get name of ref and append extension to it
    if (count == 1)
    {
        if (message->FindRef("refs", &ref) == B_OK)
        {
            BEntry entry(&ref, false);         // No traversal link
            BPath path;
            entry.GetPath(&path);
            m_fileName->SetText(path.Path());
        }
    }
    else if (count > 1)
    {
        // Get the first ref and pull the directory name from it
        BString buf = m_archiveDirPath;
        buf << "/" << "Archive";
        m_fileName->SetText(buf.String());
    }

    BMenuItem* marked = m_arkTypePopUp->FindMarked();
    if (marked)
        ReplaceExtensionWith((const char*)m_arkExtensions.ItemAt(m_arkTypePopUp->IndexOf(marked)));
    else
        ReplaceExtensionWith((const char*)m_arkExtensions.ItemAt(0L));

    m_refsMessage = *message;
    m_readyMode = true;
}


