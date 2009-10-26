/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <View.h>
#include <Region.h>
#include <Bitmap.h>

#include <string.h>
#include <malloc.h>

#include "PrefsListItem.h"
#include "UIConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"

//=============================================================================================================//

PrefsListItem::PrefsListItem (const char *text, BBitmap *bmp, bool makeBold)
    : BStringItem (text)
{
    font_height fntHt;
    be_plain_font->GetHeight (&fntHt);
    m_fontDelta = fntHt.ascent / 2;
    m_makeBold = makeBold;
    m_bitmap = bmp;
    if (m_makeBold == false)
        m_fontDelta = 3;
    
    rgb_color actFore = _prefs_interface.FindColorDef (kPfActFore, K_ACTIVE_FORE_COLOR);
    rgb_color actBack = _prefs_interface.FindColorDef (kPfActBack, K_ACTIVE_SELECT_COLOR);
    
    m_selTextColor = actFore;
    m_selBackColor = actBack;
}

//=============================================================================================================//

PrefsListItem::~PrefsListItem()
{
    // and NO we don't delete m_bitmap as its allocated/de-allocated by calling side
}

//=============================================================================================================//

void PrefsListItem::DrawItem (BView *owner, BRect frame, bool complete)
{
    if (IsSelected() || complete)
    {
        rgb_color color;
        color = IsSelected() ? m_selBackColor : owner->ViewColor();
        owner->SetHighColor (color);
        owner->FillRect (frame);
        owner->SetLowColor (color);
        
        // lighten top 2 lines
        owner->BeginLineArray (4);
        owner->AddLine (BPoint (frame.left, frame.top), BPoint (frame.right, frame.top),
                         tint_color (owner->HighColor(), B_LIGHTEN_2_TINT));
        owner->AddLine (BPoint (frame.left, frame.top + 1), BPoint (frame.right, frame.top + 1),
                         tint_color (owner->HighColor(), B_LIGHTEN_1_TINT));

        // Darken bottom 2 liness (cool effect!!)
        rgb_color specialDark1 = tint_color (owner->HighColor(), B_DARKEN_1_TINT);
        specialDark1.red += 10; specialDark1.green += 10; specialDark1.blue += 10;
        owner->AddLine (BPoint (frame.left, frame.bottom - 1), BPoint (frame.right, frame.bottom - 1),
                         specialDark1);
        rgb_color specialDark = specialDark1;
        specialDark.red -= 20; specialDark.green -= 20; specialDark.blue -= 20;
        owner->AddLine (BPoint (frame.left, frame.bottom), BPoint (frame.right, frame.bottom), specialDark);
        owner->EndLineArray();
    }
    else
        owner->SetLowColor (owner->ViewColor());
    
    if (IsEnabled())
    {
        if (IsSelected())
        {
           owner->SetHighColor (m_selTextColor);
           if (m_makeBold)
               owner->SetFont (be_bold_font);
        }
        else
        {
           owner->SetHighColor (owner->ViewColor());
           owner->FillRect (frame);
           owner->SetHighColor (0,0,0,255);
           if (m_makeBold)
               owner->SetFont (be_plain_font);
        }
    }
    else
        owner->SetHighColor (182,182,182,255);
    
    if (m_bitmap)
    {
        float bmpWidth = m_bitmap->Bounds().Width();
        float bmpHeight = m_bitmap->Bounds().Height();
        float itemWidth = (frame.right - frame.left);
        
        // Draw bitmap at center of item
        owner->SetDrawingMode (B_OP_ALPHA);
        owner->MovePenTo (6, frame.bottom - m_fontDelta / 2 - bmpHeight - 1);
        owner->DrawBitmapAsync (m_bitmap);

        // Draw string at right of item
        owner->SetDrawingMode (B_OP_COPY);
        owner->MovePenTo (6 + bmpWidth + 6, frame.bottom - bmpHeight / 2 + 1);
        owner->DrawString (Text());
    }
    else
    {
        owner->SetDrawingMode (B_OP_COPY);
        owner->MovePenTo (frame.left + 6, frame.bottom - m_fontDelta);
        owner->DrawString (Text());
    }
}

//=============================================================================================================//

float PrefsListItem::FontHeight () const
{
    return m_fontDelta * 2;
}

//=============================================================================================================//
