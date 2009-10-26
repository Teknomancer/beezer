/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <CheckBox.h>
#include <Button.h>
#include <Bitmap.h>

#include <string.h>
#include <malloc.h>

#include "PrefsView.h"
#include "LangStrings.h"
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
