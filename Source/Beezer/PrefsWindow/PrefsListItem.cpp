// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "PrefsListItem.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "UIConstants.h"

#include <Bitmap.h>
#include <View.h>


PrefsListItem::PrefsListItem(const char* text, BBitmap* bmp, bool makeBold)
    : BStringItem(text),
    m_fontDelta(0),
    m_makeBold(makeBold),
    m_bitmap(bmp),
    m_selBackColor((rgb_color){0,0,0,0}), // gcc2 requires the initializers like this
    m_selTextColor((rgb_color){0,0,0,0})
{
    font_height fntHt;
    if (makeBold == true)
    {
        // Don't use be_bold_font as that's used for window titles,
        // we just want the existing font to be bold.
        BFont boldFont(be_plain_font);
        boldFont.SetFace(B_BOLD_FACE);
        boldFont.GetHeight(&fntHt);
    }
    else
        be_plain_font->GetHeight(&fntHt);
    m_fontDelta = fntHt.ascent / 2 ;
    m_selTextColor = _prefs_interface.FindColorDef(kPfActFore, K_ACTIVE_FORE_COLOR);
    m_selBackColor = _prefs_interface.FindColorDef(kPfActBack, K_ACTIVE_SELECT_COLOR);
}


PrefsListItem::~PrefsListItem()
{
    // and NO we don't delete m_bitmap as its allocated/de-allocated by the caller.
}


void PrefsListItem::DrawItem(BView* owner, BRect frame, bool complete)
{
    if (IsSelected() || complete)
    {
        rgb_color const color =  IsSelected() ? m_selBackColor : owner->ViewColor();
        owner->SetHighColor(color);
        owner->FillRect(frame);
        owner->SetLowColor(color);

        // lighten top 2 lines
        owner->BeginLineArray(4);
        owner->AddLine(BPoint(frame.left, frame.top), BPoint(frame.right, frame.top),
                       tint_color(owner->HighColor(), B_LIGHTEN_2_TINT));
        owner->AddLine(BPoint(frame.left, frame.top + 1), BPoint(frame.right, frame.top + 1),
                       tint_color(owner->HighColor(), B_LIGHTEN_1_TINT));

        // Darken bottom 2 liness (cool effect!!)
        rgb_color specialDark1 = tint_color(owner->HighColor(), B_DARKEN_1_TINT);
        specialDark1.red += 10; specialDark1.green += 10; specialDark1.blue += 10;
        owner->AddLine(BPoint(frame.left, frame.bottom - 1), BPoint(frame.right, frame.bottom - 1),
                       specialDark1);
        rgb_color specialDark = specialDark1;
        specialDark.red -= 20; specialDark.green -= 20; specialDark.blue -= 20;
        owner->AddLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom), specialDark);
        owner->EndLineArray();
    }
    else
        owner->SetLowColor(owner->ViewColor());

    if (IsEnabled())
    {
        if (IsSelected())
        {
            owner->SetHighColor(m_selTextColor);
            if (m_makeBold)
            {
                BFont boldFont(be_plain_font);
                boldFont.SetFace(B_BOLD_FACE);
                owner->SetFont(&boldFont);
            }
        }
        else
        {
            owner->SetHighColor(owner->ViewColor());
            owner->FillRect(frame);
            owner->SetHighUIColor(B_CONTROL_TEXT_COLOR);
            if (m_makeBold)            // Reset to regular font when not selected
                owner->SetFont(be_plain_font);
        }
    }
    else
        owner->SetHighUIColor(B_CONTROL_TEXT_COLOR, B_DISABLED_LABEL_TINT);

    if (m_bitmap)
    {
        float const bmpWidth = m_bitmap->Bounds().Width();
        float const bmpHeight = m_bitmap->Bounds().Height();

        // Draw bitmap at center of item
        owner->SetDrawingMode(B_OP_ALPHA);
        owner->MovePenTo(6, frame.bottom - m_fontDelta / 2 - bmpHeight - 1);
        owner->DrawBitmapAsync(m_bitmap);

        // Draw string at right of item
        owner->SetDrawingMode(B_OP_COPY);
        owner->MovePenTo(6 + bmpWidth + 6, frame.bottom - bmpHeight / 2 + 1);
        owner->DrawString(Text());
    }
    else
    {
        owner->SetDrawingMode(B_OP_COPY);
        owner->MovePenTo(frame.left + 6, frame.bottom - m_fontDelta);
        owner->DrawString(Text());
    }
}


float PrefsListItem::FontHeight() const
{
    return m_fontDelta * 2;
}
