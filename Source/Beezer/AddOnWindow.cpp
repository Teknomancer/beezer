/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <Screen.h>
#include <Application.h>
#include <TextControl.h>
#include <Alert.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <Font.h>
#include <List.h>
#include <MenuItem.h>
#include <interface/StringView.h>
#include <Button.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <String.h>
#include <Debug.h>

#include "AddOnWindow.h"
#include "BevelView.h"
#include "AppConstants.h"
#include "UIConstants.h"
#include "LangStrings.h"
#include "ArchiverMgr.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "MsgConstants.h"
#include "BarberPole.h"
#include "ArchiveRep.h"
#include "LocalUtils.h"

//=============================================================================================================//

AddOnWindow::AddOnWindow (BMessage *refsMessage)
    : BWindow (BRect (10, 10, 420, 475), str (S_TA_TITLE), B_TITLED_WINDOW,
        B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE),
    m_arkSettingsMenuField (NULL),
    m_archive (NULL),
    m_readyMode (true),
    m_inProgress (false),
    m_quitNow (false)
{
    m_backView = new BevelView (Bounds(), "AddOnWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    AddChild (m_backView);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);

    // backViewMain will hold all controls that are to be shown when files/folders are really selected
    // otherwise it will be hidden and a message like "Drop files here to archive" will be shown using 
    // the alternate Back View    
    m_backViewMain = new BevelView (Bounds().InsetByCopy (4, 2),
                         "AddOnWindow:BackViewMain", btNoBevel, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->AddChild (m_backViewMain);
    m_backViewMain->SetViewColor (K_BACKGROUND_COLOR);

    float divider = 0;
    divider = MAX (divider, m_backViewMain->StringWidth (str (S_TA_ARCHIVE_NAME)));
    divider = MAX (divider, m_backViewMain->StringWidth (str (S_TA_ARCHIVE_TYPE)));
    divider = MAX (divider, m_backViewMain->StringWidth (str (S_TA_PASSWORD)));
    divider += 10;

    m_fileName = new BTextControl (BRect (K_MARGIN, 2 * K_MARGIN, 0, 0), "AddOnWindow:fileName",
                             str (S_TA_ARCHIVE_NAME), NULL, NULL, B_FOLLOW_LEFT_RIGHT,
                             B_WILL_DRAW);
    m_backViewMain->AddChild (m_fileName);
    m_fileName->SetModificationMessage (new BMessage (M_FILENAME_CHANGED));
    m_fileName->ResizeToPreferred ();
    m_fileName->ResizeTo (m_backViewMain->Frame().Width() - 2 * K_MARGIN, m_fileName->Frame().Height());
    m_fileName->SetDivider (divider);
    m_fileName->TextView()->DisallowChar (':');
    m_fileName->TextView()->DisallowChar ('*');
    m_fileName->TextView()->DisallowChar ('?');

    m_arkTypes = ArchiversInstalled (&m_arkExtensions);
    m_arkTypePopUp = BuildArchiveTypesMenu (this, &m_arkExtensions);
    m_arkTypeField = new BMenuField (BRect (K_MARGIN, m_fileName->Frame().bottom + K_MARGIN,
                         m_backViewMain->Frame().right, 0), "AddOnWindow:ArkTypeField",
                         str (S_TA_ARCHIVE_TYPE), (BMenu*)m_arkTypePopUp, B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backViewMain->AddChild (m_arkTypeField);
    m_arkTypeField->ResizeToPreferred();
    m_arkTypeField->SetDivider (divider + 2);
    
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
        m_fileName->SetText ((char*)m_arkExtensions.ItemAtFast (m_arkTypePopUp->IndexOf (item)));

        // Interface headache! Calculate maximum width of labels of each archiver listed in the
        // ark types field, grr!! 
        m_strWidthOfArkTypes = 0.0f;
        for (int32 i = 0; i < m_arkTypes.CountItems(); i++)
        {
           const char *name = (const char*)m_arkTypes.ItemAtFast(i);
           m_strWidthOfArkTypes = MAX (m_strWidthOfArkTypes, m_backViewMain->StringWidth (name));
        }
        m_strWidthOfArkTypes += m_arkTypeField->Divider() + 2 * K_MARGIN + 35;
    }
    else
    {
        // We've seen enough there are no add-ons installed, no use proceeding!!! Call it quits NOW
        (new BAlert ("error", str (S_TA_FATAL), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
        PostMessage (B_QUIT_REQUESTED);
        Show();
        return;
    }

    m_password = new BTextControl (BRect (K_MARGIN, m_arkTypeField->Frame().bottom + K_MARGIN,
                      divider + 2 * K_MARGIN + 80, 0), "AddOnWindow:Password", str (S_TA_PASSWORD), NULL,
                      NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    m_backViewMain->AddChild (m_password);
    m_password->TextView()->HideTyping (true);
    m_password->SetDivider (divider);
    
    m_backViewMain->ResizeTo (m_backViewMain->Frame().Width(), m_password->Frame().bottom + K_MARGIN);

    BFont font;
    m_backView->GetFont (&font);
    font_height fntHt;
    font.GetHeight (&fntHt);
    float fontHeight = fntHt.ascent + fntHt.descent + 1;
    BevelView *sepView2 = new BevelView (BRect (-1, Bounds().bottom - fontHeight - 4, Bounds().right,
                             Bounds().bottom - fontHeight - 3),
                             "AddOnWindow:sepView2", btInset, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
                             B_WILL_DRAW);
    m_backView->AddChild (sepView2);

    BevelView *sepView = new BevelView (BRect (-1, sepView2->Frame().bottom - 2 * K_MARGIN - K_BUTTON_HEIGHT -
                             4 - 8, Bounds().right, sepView2->Frame().bottom - 2 * K_MARGIN
                             - K_BUTTON_HEIGHT - 3 - 8),    "AddOnWindow:sepView", btInset,
                             B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backViewMain->AddChild (sepView);
    sepView->Hide();

    m_helpBtn = new BButton (BRect (K_MARGIN, sepView2->Frame().bottom - K_MARGIN - K_BUTTON_HEIGHT,
                                K_MARGIN + K_BUTTON_WIDTH, sepView2->Frame().bottom - K_MARGIN),
                                "AddOnWindow:HelpButton", str (S_TA_HELP), new BMessage (M_ADDON_HELP),
                                B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild (m_helpBtn);

    m_createBtn = new BButton (BRect (Bounds().right - K_MARGIN - 4 - K_BUTTON_WIDTH,
                      sepView2->Frame().bottom - K_MARGIN - K_BUTTON_HEIGHT-4,
                      Bounds().right - K_MARGIN - 4, sepView2->Frame().bottom - K_MARGIN - 4),
                      "AddOnWindow:CreateBtn", str (S_TA_CREATE), new BMessage (M_ADDON_CREATE), 
                      B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
    m_backView->AddChild (m_createBtn);
    m_createBtn->MakeDefault (true);
    m_createBtn->SetEnabled (false);

    m_helpBtn->MoveBy (0, -4);

    if (refsMessage->CountNames (B_REF_TYPE) == 0)           // No refs were selected
    {
        m_readyMode = false;
        m_backViewMain->Hide();
        
        // Add a control to ask for dropping of files
        m_backViewAlt = new BevelView (Bounds().InsetByCopy (2,2),
                             "AddOnWindow:BackViewAlt", btInset, B_FOLLOW_LEFT, B_WILL_DRAW);
        m_backViewAlt->ResizeBy (0, -(2 * K_MARGIN + K_BUTTON_HEIGHT + 4 + fontHeight + 2));
        m_backViewAlt->SetResizingMode (B_FOLLOW_ALL_SIDES);
        m_backViewAlt->SetViewColor (0, 0, 0, 255);
        
        BStringView *shadowStr = new BStringView (BRect (2 * K_MARGIN, 4 * K_MARGIN, 0, 0),
                                    "AddOnWindow:DropStr", str (S_TA_PROMPT_DROP), B_FOLLOW_LEFT,
                                    B_WILL_DRAW);
        shadowStr->SetFont (be_bold_font);
        shadowStr->SetFontSize (font.Size() + 4);
        shadowStr->SetHighColor (145, 149, 183, 255);
        shadowStr->ResizeToPreferred();
        m_backViewAlt->AddChild (shadowStr);

        BStringView *dropStr = new BStringView (BRect (2 * K_MARGIN, 4 * K_MARGIN, 0, 0), "AddOnWindow:DropStr",
                                    str (S_TA_PROMPT_DROP), B_FOLLOW_LEFT, B_WILL_DRAW);
        dropStr->SetFont (be_bold_font);
        dropStr->SetFontSize (font.Size() + 4);
        dropStr->SetHighColor (K_STARTUP_MAIN_HEADING);
        dropStr->ResizeToPreferred();
        m_backViewAlt->AddChild (dropStr);
        
        m_backView->AddChild (m_backViewAlt);

        BBitmap *bmp = BTranslationUtils::GetBitmap ('PNG ', "Img:Background");
        m_backViewAlt->SetViewBitmap (bmp);
        dropStr->SetViewBitmap (bmp);
        #ifndef B_ZETA_VERSION
        shadowStr->SetViewBitmap (bmp);
        #endif
        delete bmp;
        
        // Resize window
        ResizeTo (MAX (350, dropStr->Frame().right + 2 * K_MARGIN + 2), Frame().Height());
    }

    BString statusString = str (S_TA_READY);
    m_statusColor = SC_READY;
    if (m_readyMode == false)
    {
        statusString = str (S_TA_NOT_READY);
        m_statusColor = SC_NOT_READY;
    }
        
    m_statusStr = new BStringView (BRect (K_MARGIN, Bounds().bottom - 2 - fontHeight, 0, 0),
                         "AddOnWindow:StatusStr", statusString.String(),
                         B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW);
    m_backView->AddChild (m_statusStr);
    m_statusStr->SetLowColor (m_backView->ViewColor());
    m_statusStr->SetHighColor (m_statusColor);
    m_statusStr->SetFontSize (MIN (10, be_plain_font->Size() - 1));
    m_statusStr->ResizeToPreferred ();
    
    m_addView = new BevelView (BRect (K_MARGIN, m_password->Frame().bottom + K_MARGIN,
                      m_backViewMain->Bounds().right - K_MARGIN, sepView->Frame().top - K_MARGIN),
                      "AddOnWindow:AddView", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backViewMain->AddChild (m_addView);
    
    m_barberPole = new BarberPole (BRect (K_MARGIN, K_MARGIN, 0, K_MARGIN + fontHeight + K_MARGIN),
                         "AddOnWindow:BarberPole");
    m_barberPole->ResizeToPreferred();
    m_addView->AddChild (m_barberPole);

    m_addingFileStr = new BStringView (BRect (m_barberPole->Frame().right + K_MARGIN, K_MARGIN + 1, 0, 0),
                             "AddOnWindow:AddingFileStr", str (S_TA_ADDING), B_FOLLOW_LEFT,
                             B_WILL_DRAW);
    m_addingFileStr->ResizeToPreferred();
    m_addView->AddChild (m_addingFileStr);
    
    m_addView->ResizeTo (m_addView->Frame().Width(), m_barberPole->Frame().bottom + K_MARGIN);
    m_addView->Hide();

    ResizeTo (Frame().Width(), m_password->Frame().bottom + 3 * K_MARGIN + K_BUTTON_HEIGHT +
               m_statusStr->Frame().Height() + 8);
    m_backViewMain->ResizeTo (m_backViewMain->Frame().Width(), m_helpBtn->Frame().top - K_MARGIN);
    
    // Center window on-screen
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);

    // Constrain the size of the window
    float minH, maxH, minV, maxV;
    float w = 3 * K_MARGIN + 2 * K_BUTTON_WIDTH + 16;
    w = MAX (w, Bounds().Width());
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (w, maxH, Bounds().Height(), maxV);
    
    if (m_readyMode == true)
        RefsReceived (refsMessage);
    
    // Show could also be called ABOVE!!!! (when no addons are found)
    CreateArchiveRepAndMenus();
    Show();
}

//=============================================================================================================//

bool AddOnWindow::QuitRequested ()
{
    if (m_inProgress == true && m_quitNow == false)    // m_quitNow is checked so this isn't called twice
    {
        // We cannot pause the creation thread as we cannot get "zip", "gzip"s thread ID, atbest we can
        // only get thread ID of the thread that spawned zip etc., so forget it for the time being
        BAlert *alert = new BAlert ("Quit", str (S_FORCE_ADDON_CLOSE_WARNING), str (S_DONT_FORCE_CLOSE),
                             str (S_FORCE_CLOSE), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->SetShortcut (0L, B_ESCAPE);
        alert->SetDefaultButton (alert->ButtonAt (1L));
        int32 index = alert->Go();
        if (index == 1L)
        {
           m_cancel = true;
           m_createBtn->SetEnabled (false);
        }

        m_quitNow = true;
        return false;
    }
    else if (m_quitNow)    // Incase QuitRequested is called a second time, still don't do anything silly
        return false;
    else
        return BWindow::QuitRequested();
}

//=============================================================================================================//

void AddOnWindow::Quit ()
{
    m_cancel = true;
    for (int32 i = 0; i < m_arkExtensions.CountItems(); i++)
        free ((char*)m_arkExtensions.ItemAtFast(i));

    be_app_messenger.SendMessage (M_CLOSE_ADDON);
    return BWindow::Quit();
}

//=============================================================================================================//

void AddOnWindow::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case BZR_UPDATE_PROGRESS:
        {
           const char *mainText;
           if (message->FindString ("text", &mainText) != B_OK)
               mainText = "";
           
           BString buf = str (S_TA_ADDING);
           buf << mainText;
           m_addingFileStr->SetText (buf.String());
           m_addingFileStr->ResizeToPreferred ();
           break;
        }
        
        case M_CLOSE:
        {
           m_inProgress = false;
           Quit ();
           break;
        }
        
        case M_ADDON_CREATE:
        {
           if (m_inProgress == false)
           {
               if (m_archive)
               {
                  m_inProgress = true;
                  UpdateStatus (str (S_TA_BUSY));
                  m_createBtn->SetLabel (str (S_CANCEL));
                  m_createBtn->MakeDefault (false);
                  m_cancel = false;
                  EnableControls (false);
                  
                  BMessage *addMessage = new BMessage ('addd');    // no need to delete as it will be posted to
                                                             // us as M_ADD_DONE
                  
                  // Pull the first ref's directory from the list of refs
                  entry_ref ref;
                  m_refsMessage.FindRef ("refs", 0, &ref);
                  BEntry entry (&ref, false);
                  entry.GetParent (&entry);        // strange, but legal
                  BPath launchDirPath;
                  entry.GetPath (&launchDirPath);
                  
                  BMessenger messenger (this);
                  addMessage->AddMessenger (kProgressMessenger, messenger);
                  addMessage->AddString (kArchivePath, m_fileName->Text());
                  addMessage->AddPointer (kLooperPtr, this);
                  addMessage->AddPointer (kCancel, (void*)&m_cancel);
                  addMessage->AddString (kLaunchDir, launchDirPath.Path());
                  if (m_password->TextView()->TextLength() > 0)
                      m_archive->Ark()->SetPassword (m_password->Text());
                  
                  uint32 type;
                  int32 count;
                  entry_ref tmpRef;
                  m_refsMessage.GetInfo ("refs", &type, &count);
                  for (int32 i = --count; i >= 0; i--)
                      if (m_refsMessage.FindRef ("refs", i, &tmpRef) == B_OK)
                         addMessage->AddString (kPath, tmpRef.name);
                  
                  // Deleting existing file if any
                  BEntry temp (m_fileName->Text());
                  if (temp.Exists())
                      temp.Remove();
                  
                  ResizeBy (0, m_addView->Frame().Height());
                  m_barberPole->SetValue (true, false);
                  m_addView->Show();
                  
                  m_archive->Create (addMessage);
               }
           }
           else
           {
               m_cancel = true;
               m_inProgress = false;
               m_createBtn->MakeDefault (true);
               m_createBtn->SetLabel (str (S_TA_CREATE));
               
               EnableControls (true);
               PostMessage (B_QUIT_REQUESTED);
           }
                  
           break;
        }
        
        case B_REFS_RECEIVED:
        {
           RefsReceived (message);
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
           be_app->PostMessage (message);
           break;
        }
        
        case M_ARK_TYPE_SELECTED:
        {
           ReplaceExtensionWith (message->FindString (kText));
           CreateArchiveRepAndMenus ();
           break;
        }

        case BZR_MENUITEM_SELECTED:
        {
           // An archiver (add-on)'s menu item has been toggled (we should toggle it to be precise)
           // Get the source and toggle it
           BMenuItem *source_item = NULL;
           message->FindPointer ("source", reinterpret_cast<void**>(&source_item));
           if (source_item)
               source_item->SetMarked (!(source_item->IsMarked()));

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
               PostMessage (message);
           }
           
           break;
        }

        default:
           return BWindow::MessageReceived (message);
           break;
    }
}

//=============================================================================================================//

void AddOnWindow::EnableControls (bool enable)
{
    m_fileName->SetEnabled (enable);
    m_arkTypeField->SetEnabled (enable);

    if (m_arkSettingsMenuField)
        m_arkSettingsMenuField->SetEnabled (enable);

    m_password->SetEnabled (enable);
}

//=============================================================================================================//

void AddOnWindow::CreateArchiveRepAndMenus ()
{
    if (m_readyMode == false)
        return;

    if (m_arkSettingsMenuField)
    {
        m_backViewMain->RemoveChild (m_arkSettingsMenuField);
        delete m_arkSettingsMenuField;
        m_arkSettingsMenuField = NULL;
    }
    
    if (m_archive != NULL)
    {
        delete m_archive;
        m_archive = NULL;
    }
    
    m_archive = new ArchiveRep();
    status_t result = m_archive->InitArchiver (m_arkTypePopUp->FindMarked()->Label(), true);
    if (result == BZR_DONE)
    {
        // **** Later move this to the place where all archivers are added to the m_arkType menu!!!!!!
        if (m_archive->Ark()->SettingsMenu())
        {
           m_arkSettingsMenuField = new BMenuField (BRect (m_strWidthOfArkTypes,
                                        m_arkTypeField->Frame().top, m_backViewMain->Frame().right, 0),
                                        "AddOnWindow:ArkSettingsMenuField", str (S_TA_SETTINGS),
                                        m_archive->Ark()->SettingsMenu(), B_FOLLOW_LEFT,
                                        B_WILL_DRAW | B_NAVIGABLE);
           m_backViewMain->AddChild (m_arkSettingsMenuField);
           m_arkSettingsMenuField->ResizeToPreferred();
           m_arkSettingsMenuField->SetDivider (m_backViewMain->StringWidth (m_arkSettingsMenuField->Label()) + 10);
           SetTargetForMenuRecursive (m_archive->Ark()->SettingsMenu(), this);
        }
        
        if (m_archive && m_archive->Ark()->SupportsPassword() == true)
           m_password->SetEnabled (true);
        else
        {
           m_password->SetEnabled (false);
           m_password->SetText (NULL);
        }
    }
    else
    {
        delete m_archive;
        m_archive = NULL;
    }
}

//=============================================================================================================//

bool AddOnWindow::ReplaceExtensionWith (const char *newExt)
{
    int32 start, end;
    BTextView *fileNameView = m_fileName->TextView();
    fileNameView->GetSelection (&start, &end);
    
    BString existingName = m_fileName->Text();
    const char *newExtStr = newExt;
    bool replacedExt = false;

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
           replacedExt = true;
           break;
        }
        
    if (replacedExt == false)
    {
        BString buf = existingName;
        buf << newExtStr;
        fileNameView->SetText (buf.String());
        fileNameView->Select (start, end);
    }
    
    return replacedExt;
}

//=============================================================================================================//

void AddOnWindow::UpdateStatus (const char *text)
{
    if (strcmp (text, str (S_TA_OVERWRITE)) == 0)
        m_statusStr->SetHighColor (SC_OVERWRITE);
    else if (strcmp (text, str (S_TA_READY)) == 0)
        m_statusStr->SetHighColor (SC_READY);
    else if (strcmp (text, str (S_TA_DIR_MISSING)) == 0)
        m_statusStr->SetHighColor (SC_DIR_MISSING);
    else if (strcmp (text, str (S_TA_BUSY)) == 0)
        m_statusStr->SetHighColor (SC_BUSY);
    else
        m_statusStr->SetHighColor (0, 0, 0, 255);

    m_statusStr->SetText (text);
    m_statusStr->ResizeToPreferred ();
}

//=============================================================================================================//

void AddOnWindow::ValidateData ()
{
    if (m_inProgress)
        return;
    
    bool valid = true;
    if (m_readyMode == true)
    {
        BEntry entry (m_fileName->Text(), false);
        if (entry.Exists() == true)
           UpdateStatus (str (S_TA_OVERWRITE));
        else
           UpdateStatus (str (S_TA_READY));
        
        BString buf = m_fileName->Text();
        int32 found = buf.FindLast ('/');
        buf.Remove (found, buf.Length() - found);
        
        m_archiveDirPath = buf;
        bool doubleSlash = false;
        if (m_archiveDirPath[m_archiveDirPath.Length() - 1] == '/')    // error as it is like /boot//a.zip
           doubleSlash = true;
           
        BEntry dirEntry (m_archiveDirPath.String(), false);
        
        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false || doubleSlash == true)
        {
           UpdateStatus (str (S_TA_DIR_MISSING));
           valid = false;
        }
    }
    else
    {
        valid = false;
        UpdateStatus (str (S_TA_NOT_READY));
    }
    
    if (m_readyMode && valid == true)
        m_createBtn->SetEnabled (true);
    else
        m_createBtn->SetEnabled (false);
}

//=============================================================================================================//

void AddOnWindow::RefsReceived (BMessage *message)
{
    // It is now in ready mode
    uint32 type;
    int32 count;
    entry_ref ref;
    
    message->GetInfo ("refs", &type, &count);
    if (type != B_REF_TYPE)
        return;
    
    // Pull the first ref's directory from the list of refs
    message->FindRef ("refs", 0, &ref);
    BEntry entry (&ref, false);
    entry.GetParent (&entry);        // strange, but legal
    BPath path;
    entry.GetPath (&path);
    m_archiveDirPath = path.Path();
               
    // OKAY get name of ref and append extension to it
    if (count == 1)
    {
        if (message->FindRef ("refs", &ref) == B_OK)
        {
           BEntry entry (&ref, false);        // No traversal link
           BPath path;
           entry.GetPath (&path);
           m_fileName->SetText (path.Path());
        }
    }
    else if (count > 1)
    {
        // Get the first ref and pull the directory name from it
        BString buf = m_archiveDirPath;
        buf << "/" << "Archive";
        m_fileName->SetText (buf.String());
    }
    
    BMenuItem *marked = m_arkTypePopUp->FindMarked();
    if (marked)
        ReplaceExtensionWith ((const char*)m_arkExtensions.ItemAt (m_arkTypePopUp->IndexOf (marked)));
    else
        ReplaceExtensionWith ((const char*)m_arkExtensions.ItemAt (0L));
        
    m_refsMessage = *message;
        m_readyMode = true;
}

//=============================================================================================================//
