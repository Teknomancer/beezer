/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>

#include <string.h>
#include <malloc.h>

#include "LangStrings.h"
#include "PrefsView.h"
#include "UIConstants.h"

//=============================================================================================================//

PrefsView::PrefsView (BRect frame, const char *title, const char *description)
    : BevelView (frame, NULL, btInset, B_FOLLOW_LEFT, B_WILL_DRAW)
{
    m_descriptionStr = strdup (description);
    m_titleStr = strdup (title);
    m_bitmap = NULL;

    // For use in inherited classes - to provide consistency among them we give these
    // member functions which they are supposed to use when needed
    m_margin = K_MARGIN + 2;
    m_vGap = 1;
    SetViewColor (K_BACKGROUND_COLOR);
    m_sectionFont = be_plain_font;
    m_sectionFont.SetFace (B_BOLD_FACE);
    // We are un-necessarily storing a BFont object for each PrefView but there isn't
    // any easy way out (static BFont didn't worked and crashed on initialization)
    // we can do one thing though - add a BFont parameter to ever PrefView derived
    // class and make it pass that to us, but thats a bore too
}

//=============================================================================================================//

PrefsView::~PrefsView ()
{
    if (m_descriptionStr)
        free ((char*)m_descriptionStr);

    if (m_titleStr)
        free ((char*)m_titleStr);

    DeleteBitmap();
}

//=============================================================================================================//

const char* PrefsView::Description () const
{
    return m_descriptionStr;
}

//=============================================================================================================//

const char* PrefsView::Title () const
{
    return m_titleStr;
}

//=============================================================================================================//

bool PrefsView::IsChecked (BCheckBox *chkBox) const
{
    // Convert checkbox's value as a bool
    if (chkBox->Value() == B_CONTROL_ON)
        return true;
    else
        return false;
}

//=============================================================================================================//

void PrefsView::AddRevertButton ()
{
    m_revertBtn = new BButton (BRect (Bounds().right - m_margin - K_BUTTON_WIDTH,
                             Bounds().bottom - m_margin - K_BUTTON_HEIGHT - 1, Bounds().right - m_margin,
                             Bounds().bottom - m_margin - 1), "PrefsView:defaultBtn", str (S_REVERT),
                             new BMessage (M_REVERT), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    AddChild (m_revertBtn);
    m_revertBtn->SetTarget (this);
}

//=============================================================================================================//

void PrefsView::DeleteBitmap ()
{
    if (m_bitmap !=NULL)
    {
        delete m_bitmap;
        m_bitmap = NULL;
    }
}

//=============================================================================================================//

void PrefsView::SetBitmap (BBitmap *bmp)
{
    DeleteBitmap();
    m_bitmap = bmp;
}

//=============================================================================================================//

BBitmap *PrefsView::Bitmap () const
{
    return m_bitmap;
}

//=============================================================================================================//

void PrefsView::Render ()
{
    // Derived class will override this, deliberately not made abstract
}

//=============================================================================================================//

void PrefsView::Save ()
{
    // Derived class will override this, deliberately not made abstract
}

//=============================================================================================================//

void PrefsView::Load ()
{
    // Derived class will override this, deliberately not made abstract
}

//=============================================================================================================//
