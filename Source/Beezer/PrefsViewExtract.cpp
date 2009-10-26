/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <String.h>
#include <CheckBox.h>
#include <Debug.h>

#include "PrefsViewExtract.h"
#include "LangStrings.h"
#include "UIConstants.h"
#include "AppConstants.h"
#include "Preferences.h"

//=============================================================================================================//

PrefsViewExtract::PrefsViewExtract (BRect frame)
    : PrefsView (frame, str (S_PREFS_TITLE_EXTRACT), str (S_PREFS_DESC_EXTRACT))
{
    SetBitmap (ResBitmap ("Img:Prefs_Extract"));
    Render();
}

//=============================================================================================================//

void PrefsViewExtract::Render ()
{
    m_openFolderChk = new BCheckBox (BRect (m_margin, m_margin, 0, 0), "PrefsViewExtract:openFolderChk",
                         str (S_PREFS_EXTRACT_OPEN_DIR), NULL);
    m_openFolderChk->ResizeToPreferred();

    m_closeChk = new BCheckBox (BRect (m_margin, m_openFolderChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewExtract:closeChk", str (S_PREFS_EXTRACT_CLOSE_WINDOW), NULL);
    m_closeChk->ResizeToPreferred();

    BString s = str (S_PREFS_EXTRACT_QUIT_APP);
    s.ReplaceAll ("%s", K_APP_TITLE);
    m_quitChk = new BCheckBox (BRect (m_margin, m_closeChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewExtract:closeChk", s.String(), NULL);
    m_quitChk->ResizeToPreferred();

    m_dragChk = new BCheckBox (BRect (m_margin, m_quitChk->Frame().bottom + m_vGap, 0, 0),
                             "PrefsViewExtract:dragChk", str (S_PREFS_EXTRACT_DRAG_PATH), NULL);
    m_dragChk->ResizeToPreferred();

    AddChild (m_openFolderChk);
    AddChild (m_closeChk);
    AddChild (m_quitChk);
    AddChild (m_dragChk);
    AddRevertButton();
}

//=============================================================================================================//

void PrefsViewExtract::Save ()
{
    _prefs_extract.SetBool (kPfOpen, IsChecked (m_openFolderChk));
    _prefs_extract.SetBool (kPfClose, IsChecked (m_closeChk));
    _prefs_extract.SetBool (kPfQuit, IsChecked (m_quitChk));
    _prefs_extract.SetBool (kPfDrag, IsChecked (m_dragChk));

    _prefs_extract.WritePrefs();
}

//=============================================================================================================//

void PrefsViewExtract::Load ()
{
    m_openFolderChk->SetValue (_prefs_extract.FindBoolDef (kPfOpen, true));
    m_closeChk->SetValue (_prefs_extract.FindBoolDef (kPfClose, false));
    m_quitChk->SetValue (_prefs_extract.FindBoolDef (kPfQuit, false));
    m_dragChk->SetValue (_prefs_extract.FindBoolDef (kPfDrag, true));
}

//=============================================================================================================//
