// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsView.h"
#include "UIConstants.h"

#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsView"
#else
#define B_TRANSLATE(x) x
#endif


PrefsView::PrefsView(BRect frame, const char* title, const char* description)
    : BevelView(frame, NULL, BevelView::INSET, B_FOLLOW_LEFT, B_WILL_DRAW),
    m_revertBtn(NULL),
    m_margin(K_MARGIN + 2),         // For use in inherited classes - to provide consistency among them we give these
    m_vGap(1),                      // member functions which they are supposed to use when needed
    m_sectionFont(be_plain_font),
    m_titleStr(title),
    m_descriptionStr(description),
    m_bitmap(NULL)
{
    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

    // We are un-necessarily storing a BFont object for each PrefView but there isn't
    // any easy way out (static BFont didn't worked and crashed on initialization)
    // we can do one thing though - add a BFont parameter to ever PrefView derived
    // class and make it pass that to us, but thats a bore too
    m_sectionFont.SetFace(B_BOLD_FACE);
}


PrefsView::~PrefsView()
{
    DeleteBitmap();
}


const char* PrefsView::Description() const
{
    return m_descriptionStr.String();
}


const char* PrefsView::Title() const
{
    return m_titleStr.String();
}


bool PrefsView::IsChecked(BCheckBox* chkBox) const
{
    // Convert checkbox's value as a bool
    if (chkBox->Value() == B_CONTROL_ON)
        return true;
    else
        return false;
}


void PrefsView::AddRevertButton()
{
    m_revertBtn = new BButton(BRect(Bounds().right - m_margin - K_BUTTON_WIDTH,
                                    Bounds().bottom - m_margin - K_BUTTON_HEIGHT - 1, Bounds().right - m_margin,
                                    Bounds().bottom - m_margin - 1), "PrefsView:defaultBtn", B_TRANSLATE("Revert"),
                              new BMessage(M_REVERT), B_FOLLOW_RIGHT, B_WILL_DRAW | B_NAVIGABLE);
    AddChild(m_revertBtn);
    m_revertBtn->SetTarget(this);
}


void PrefsView::AddRevertButton(BLayoutBuilder::Group<>& builder)
{
    m_revertBtn = new BButton("PrefsView:defaultBtn", B_TRANSLATE("Revert"), new BMessage(M_REVERT),
                              B_WILL_DRAW | B_NAVIGABLE);

    builder
        .AddGroup(B_HORIZONTAL)
            .AddGlue()  // glue first adds padding and moves the button to the right
            .Add(m_revertBtn)
        .End();

    m_revertBtn->SetTarget(this);
}


void PrefsView::DeleteBitmap()
{
    if (m_bitmap != NULL)
    {
        delete m_bitmap;
        m_bitmap = NULL;
    }
}


void PrefsView::SetBitmap(BBitmap* bmp)
{
    DeleteBitmap();
    m_bitmap = bmp;
}


BBitmap* PrefsView::Bitmap() const
{
    return m_bitmap;
}


void PrefsView::Render()
{
    // Derived class will override this, deliberately not made abstract
}


void PrefsView::Save()
{
    // Derived class will override this, deliberately not made abstract
}


void PrefsView::Load()
{
    // Derived class will override this, deliberately not made abstract
}
