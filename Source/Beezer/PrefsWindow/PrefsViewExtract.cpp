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

#include <CheckBox.h>
#include <Debug.h>
#include <String.h>

#include "AppConstants.h"
#include "BitmapPool.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsViewExtract.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsViewExtract"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_SYSTEM_NAME(x) x
#endif


PrefsViewExtract::PrefsViewExtract(BRect frame)
    : PrefsView(frame, B_TRANSLATE("Extract"), B_TRANSLATE("Extraction related settings"))
{
    SetBitmap(BitmapPool::LoadAppVector("Img:Prefs_Extract", 20, 20));
    Render();
}



void PrefsViewExtract::Render()
{
    m_openFolderChk = new BCheckBox(BRect(m_margin, m_margin, 0, 0), "PrefsViewExtract:openFolderChk",
                                    B_TRANSLATE("Open destination folder after extracting"), NULL);
    m_openFolderChk->ResizeToPreferred();

    m_closeChk = new BCheckBox(BRect(m_margin, m_openFolderChk->Frame().bottom + m_vGap, 0, 0),
                               "PrefsViewExtract:closeChk", B_TRANSLATE("Close window after extracting"), NULL);
    m_closeChk->ResizeToPreferred();

    BString bufStr(B_TRANSLATE("Quit %appname% after extracting"));
    bufStr.ReplaceAll("%appname%", B_TRANSLATE_SYSTEM_NAME(K_APP_TITLE));
    m_quitChk = new BCheckBox(BRect(m_margin, m_closeChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewExtract:closeChk", bufStr, NULL);
    m_quitChk->ResizeToPreferred();

    m_dragChk = new BCheckBox(BRect(m_margin, m_quitChk->Frame().bottom + m_vGap, 0, 0),
                              "PrefsViewExtract:dragChk", B_TRANSLATE("Extract with full path during drag 'n drop"), NULL);
    m_dragChk->ResizeToPreferred();

    AddChild(m_openFolderChk);
    AddChild(m_closeChk);
    AddChild(m_quitChk);
    AddChild(m_dragChk);
    AddRevertButton();
}



void PrefsViewExtract::Save()
{
    _prefs_extract.SetBool(kPfOpen, IsChecked(m_openFolderChk));
    _prefs_extract.SetBool(kPfClose, IsChecked(m_closeChk));
    _prefs_extract.SetBool(kPfQuit, IsChecked(m_quitChk));
    _prefs_extract.SetBool(kPfDrag, IsChecked(m_dragChk));

    _prefs_extract.WritePrefs();
}



void PrefsViewExtract::Load()
{
    m_openFolderChk->SetValue(_prefs_extract.FindBoolDef(kPfOpen, true));
    m_closeChk->SetValue(_prefs_extract.FindBoolDef(kPfClose, false));
    m_quitChk->SetValue(_prefs_extract.FindBoolDef(kPfQuit, false));
    m_dragChk->SetValue(_prefs_extract.FindBoolDef(kPfDrag, true));
}


