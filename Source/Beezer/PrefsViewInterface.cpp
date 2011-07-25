/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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

#include <String.h>
#include <CheckBox.h>
#include <Debug.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ColorControl.h>
#include <Window.h>

#include "PrefsViewInterface.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "AppConstants.h"
#include "Preferences.h"
#include "MsgConstants.h"
#include "ArchiverMgr.h"

#define M_COLOR_CHANGE        'clch'
#define M_ITEM_CHANGE        'itch'

//=============================================================================================================//

PrefsViewInterface::PrefsViewInterface (BRect frame)
    : PrefsView (frame, str (S_PREFS_TITLE_INTERFACE), str (S_PREFS_DESC_INTERFACE))
{
    SetBitmap (ResBitmap ("Img:Prefs_Interface"));
    Render();
}

//=============================================================================================================//

void PrefsViewInterface::Render ()
{
    m_fullLengthBarsChk = new BCheckBox (BRect (m_margin, m_margin, 0, 0), "PrefsViewInterface:fullLenBars",
                         str (S_PREFS_INTERFACE_FULLBARS), NULL);
    m_fullLengthBarsChk->ResizeToPreferred();

    BStringView *colorStrView = new BStringView (BRect (m_margin, m_fullLengthBarsChk->Frame().bottom + 
                                    m_margin + 2 * m_vGap, 0, 0), NULL, str (S_PREFS_INTERFACE_COLORS));
    colorStrView->SetFont (&m_sectionFont);
    colorStrView->ResizeToPreferred();
    colorStrView->SetLowColor (ViewColor());

    BevelView *outerView =     new BevelView (BRect (3 * m_margin, colorStrView->Frame().bottom + m_margin,
                                3 * m_margin + 30, colorStrView->Frame().bottom + m_margin + 30),
                                "PrefsViewInterface:outerView", btDeep, B_FOLLOW_LEFT, B_WILL_DRAW);
    float boundary = outerView->EdgeThickness();
    
    m_colorWell = new BView (BRect (boundary, boundary, outerView->Frame().Width() - boundary,
                             outerView->Frame().Height() - boundary), "PrefsViewInterface:colorWell",
                             B_FOLLOW_LEFT, B_WILL_DRAW);
    outerView->AddChild (m_colorWell);
    m_colorWell->SetViewColor (0,0,0,255);
    
    m_colorPopUp = new BPopUpMenu ("");
    m_colorField = new BMenuField (BRect (outerView->Frame().right + 3 * m_margin,
                         outerView->Frame().top + 2, Frame().Width(), 0),
                         "PrefsViewInterface:colorField", NULL, (BMenu*)m_colorPopUp, B_FOLLOW_LEFT,
                         B_WILL_DRAW);
    m_colorPopUp->AddItem (new BMenuItem (str (S_PREFS_INTERFACE_ACTFORE), new BMessage (M_ITEM_CHANGE)));
    m_colorPopUp->AddItem (new BMenuItem (str (S_PREFS_INTERFACE_ACTBACK), new BMessage (M_ITEM_CHANGE)));
    m_colorPopUp->ResizeToPreferred();
    
    m_colorControl = new BColorControl (BPoint (3 * m_margin,
                         MAX (m_colorPopUp->Frame().bottom, outerView->Frame().bottom) + m_margin + 2),
                         B_CELLS_32x8, 8, "PrefsViewInteface:colorControl", new BMessage (M_COLOR_CHANGE));


    BStringView *defStrView = new BStringView (BRect (m_margin, m_colorControl->Frame().bottom + 
                                    2*m_margin + 2 * m_vGap, 0, 0), NULL, str (S_PREFS_INTERFACE_DEFAULTS));
    defStrView->SetFont (&m_sectionFont);
    defStrView->ResizeToPreferred();
    defStrView->SetLowColor (ViewColor());

    m_toolbarChk = new BCheckBox (BRect (3 * m_margin, defStrView->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewInterface:toolbarChk", str (S_PREFS_INTERFACE_TOOLBAR), NULL);
    m_toolbarChk->ResizeToPreferred();

    m_infobarChk = new BCheckBox (BRect (3 * m_margin, m_toolbarChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewInterface:infoBarChk", str (S_PREFS_INTERFACE_INFOBAR), NULL);
    m_infobarChk->ResizeToPreferred();

    m_actionLogChk = new BCheckBox (BRect (3 * m_margin, m_infobarChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewInterface:actionLogChk", str (S_PREFS_INTERFACE_ACTION_LOG), NULL);
    m_actionLogChk->ResizeToPreferred();

    m_foldingPopUp = new BPopUpMenu ("");
    m_foldingPopUp->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_NONE), NULL));
    m_foldingPopUp->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_ONE), NULL));
    m_foldingPopUp->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_TWO), NULL));
    m_foldingPopUp->AddItem (new BMenuItem (str (S_SETTINGS_FOLDING_ALL), NULL));
    m_foldingPopUp->ItemAt(3)->SetMarked (true);

    float maxW = MAX (StringWidth (str (S_PREFS_INTERFACE_INFOBAR)), StringWidth(str(S_PREFS_INTERFACE_TOOLBAR)));
    maxW = MAX (maxW, StringWidth (str (S_PREFS_INTERFACE_ACTION_LOG)));
    maxW += 5 * m_margin + 30;

    m_foldingField = new BMenuField (BRect (m_toolbarChk->Frame().left + maxW,
                         m_toolbarChk->Frame().top, Bounds().right - m_margin, 0),
                         "PrefsViewInterface:foldingField", str (S_PREFS_INTERFACE_FOLDING),
                         (BMenu*)m_foldingPopUp, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);
    float div = m_foldingField->StringWidth (m_foldingField->Label()) + 10;                      
    m_foldingField->SetDivider (div);

    font_height fntHt;
    m_sectionFont.GetHeight (&fntHt);

    AddChild (m_fullLengthBarsChk);
    AddChild (colorStrView);
    AddChild (outerView);
    AddChild (m_colorField);
    AddChild (m_colorControl);
    AddChild (defStrView);
    AddChild (m_toolbarChk);
    AddChild (m_infobarChk);
    AddChild (m_actionLogChk);
    AddChild (m_foldingField);
    AddRevertButton();
}

//=============================================================================================================//

void PrefsViewInterface::Save ()
{
    _prefs_interface.SetBool (kPfFullLengthBars, IsChecked (m_fullLengthBarsChk));
    _prefs_interface.SetColor (kPfActFore, m_actFore);
    _prefs_interface.SetColor (kPfActBack, m_actBack);
    _prefs_interface.SetInt16 (kPfColorIndex, (int16)m_colorPopUp->IndexOf (m_colorPopUp->FindMarked()));
    _prefs_interface.WritePrefs();

    // A sort of hack, What we do is load the settings file from the hard-disk and replace
    // the settings
    BMessage msg;
    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;

    BFile file (path.String(), B_READ_ONLY);
    msg.Unflatten (&file);                      // may fail (if settings file is missing, but doesn't matter

    // Change only the fields we need, rest (such as Window size and position, columns etc. remain unchanged
    // and will be preserved)
    msg.RemoveName (kToolBar);
    msg.RemoveName (kInfoBar);
    msg.RemoveName (kSplitter);
    msg.RemoveName (kFolding);
    msg.AddBool (kToolBar, IsChecked (m_toolbarChk));
    msg.AddBool (kInfoBar, IsChecked (m_infobarChk));
    msg.AddBool (kSplitter, IsChecked (m_actionLogChk));
    msg.AddInt8 (kFolding, m_foldingPopUp->IndexOf (m_foldingPopUp->FindMarked()));
    
    // Re-write message (using a NEW FILE and erase older one)
    // Don't worry all the settings have been retreived in "msg" (using above Unflatten) so other settings
    // like window position and size are still retained!
    BFile writeFile (path.String(), B_ERASE_FILE | B_CREATE_FILE | B_READ_WRITE);
    msg.Flatten (&writeFile);
}

//=============================================================================================================//

void PrefsViewInterface::Load ()
{
    m_fullLengthBarsChk->SetValue (_prefs_interface.FindBoolDef (kPfFullLengthBars, false));
    
    m_colorPopUp->ItemAt(_prefs_interface.FindInt16Def (kPfColorIndex, 0))->SetMarked (true);
    m_actFore = _prefs_interface.FindColorDef (kPfActFore, K_ACTIVE_FORE_COLOR);
    m_actBack = _prefs_interface.FindColorDef (kPfActBack, K_ACTIVE_SELECT_COLOR);
    UpdateColorControl (m_colorPopUp->FindMarked());
    
    // Load MainWindow settings file to retrieve the appropriate settings (semi-hack!)
    BMessage msg;
    BString path = _bzr()->m_settingsPathStr;
    path << "/" << K_SETTINGS_MAINWINDOW;

    BFile file (path.String(), B_READ_ONLY);
    
    // restore defaults or load from file
    msg.Unflatten (&file);        // may fail (if settings file is missing)

    m_toolbarChk->SetValue (FindBoolDef (&msg, kToolBar, true));
    m_infobarChk->SetValue (FindBoolDef (&msg, kInfoBar, true));    
    m_actionLogChk->SetValue (FindBoolDef (&msg, kSplitter, true));

    // Restore folding level if present or else the default one
    int8 v;
    status_t result = msg.FindInt8 (kFolding, &v);
    if (result == B_OK)
        m_foldingPopUp->ItemAt(v)->SetMarked (true);
    else
        m_foldingPopUp->ItemAt(3)->SetMarked (true);
}

//=============================================================================================================//

void PrefsViewInterface::AttachedToWindow()
{
    m_colorControl->SetTarget (this);
    m_colorPopUp->SetTargetForItems (this);
    return PrefsView::AttachedToWindow();
}

//=============================================================================================================//

void PrefsViewInterface::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case M_COLOR_CHANGE:
        {
           BMenuItem *item = m_colorPopUp->FindMarked();
           if (!item)
               break;
               
           BString itemText = item->Label();
           if (itemText == str (S_PREFS_INTERFACE_ACTFORE))
               m_actFore = m_colorControl->ValueAsColor();
           else if (itemText == str (S_PREFS_INTERFACE_ACTBACK))
               m_actBack = m_colorControl->ValueAsColor();
           
           UpdateColorWell();
           break;
        }
        
        case M_ITEM_CHANGE:
        {
           BMenuItem *item = NULL;
           message->FindPointer ("source", (void**)&item);
           if (!item)
               break;
           UpdateColorControl (item);
           break;
        }
    }
    
    return PrefsView::MessageReceived (message);
}

//=============================================================================================================//

void PrefsViewInterface::UpdateColorWell ()
{
    m_colorWell->SetViewColor (m_colorControl->ValueAsColor());
    m_colorWell->Invalidate();
}

//=============================================================================================================//

void PrefsViewInterface::UpdateColorControl (BMenuItem *item)
{
    BString itemText = item->Label();
    if (itemText == str (S_PREFS_INTERFACE_ACTFORE))
        m_colorControl->SetValue (m_actFore);
    else if (itemText == str (S_PREFS_INTERFACE_ACTBACK))
        m_colorControl->SetValue (m_actBack);
    
    UpdateColorWell();
}

//=============================================================================================================//

bool PrefsViewInterface::FindBoolDef (BMessage *msg, const char *name, bool defaultValue)
{
    // Because we are loading from ANOTHER settings file (via a BMessage and not Preferences class)!!
    // This supports the semi-hack we do in Load and Save in this View
    bool v;
    status_t result = msg->FindBool (name, &v);
    if (result == B_OK)
        return v;
    else
        return defaultValue;
}

//=============================================================================================================//
