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

#include <Clipboard.h>
#include <Bitmap.h>
#include <Window.h>
#include <Screen.h>

#include <malloc.h>
#include <string.h>
#include <ctype.h>

#include "BeezerStringView.h"

//=============================================================================================================//

BeezerStringView::BeezerStringView (BRect frame, const char *name, const char *label,
               uint32 rmask, uint32 flags)
    : BView (frame, name, rmask, flags),
    m_mouseTargetView (NULL),
    m_text (strdup (label)),
    m_oldText (NULL),
    m_foreground ((rgb_color){0,0,0,255}),
    m_background ((rgb_color){219,219,219,255})
{
    m_font = *be_plain_font;
}

//=============================================================================================================//

BeezerStringView::~BeezerStringView ()
{
    if (m_text)
        free (m_text);

    if (m_oldText)
        free (m_oldText);
}

//=============================================================================================================//

void BeezerStringView::Draw (BRect frame)
{
    RenderView ();
    BView::Draw (frame);
}

//=============================================================================================================//

void BeezerStringView::Invalidate (BRect invalRect)
{
    RenderView ();
    BView::Invalidate (invalRect);
}

//=============================================================================================================//

void BeezerStringView::RenderView (bool firstTime)
{
    // Buffered draw, lock the bitmap, draw to the bitmap owned view, then render bitmap finally
    m_backBitmap->Lock();
    
    BRect bounds (m_backView->Bounds());
    m_backView->SetDrawingMode (B_OP_COPY);
    
    if (firstTime == true)
    {
        // Fill up the background and draw the borders -- i.e. complete redraw
        SetViewColor (B_TRANSPARENT_COLOR);
        m_backView->SetHighColor (m_backView->ViewColor());
        m_backView->FillRect (bounds);
    }

    // Erase old text using backcolor 
    m_backView->SetFont (&m_font, B_FONT_ALL);
    if (m_oldText != NULL && firstTime == false)    // firstTime check is CRITICAL (bugfix)
    {
        m_backView->MovePenTo (3, bounds.bottom - 3);
        m_backView->SetHighColor (m_backView->ViewColor());
        m_backView->DrawString (m_oldText);
    }
    
    // Write new text
    m_backView->MovePenTo (3, bounds.bottom - 3);
    m_backView->SetHighColor (m_foreground);
    m_backView->DrawString (m_text);
    
    // Sync the view, its important we do this    
    m_backView->Sync();
    DrawBitmap (m_backBitmap, Bounds(), Bounds());

    m_backBitmap->Unlock();
}

//=============================================================================================================//

void BeezerStringView::SetText (const char *text)
{
    // Delete the previous text if any
    if (m_oldText)
        free (m_oldText);
    
    // Hold old text into m_oldText, re-allocate m_text for the new text
    m_oldText = m_text;
    m_text = strdup (text);
    
    RenderView ();
}

//=============================================================================================================//

const char* BeezerStringView::Text () const
{
    return const_cast<const char*>(m_text);
}

//=============================================================================================================//

void BeezerStringView::AttachedToWindow ()
{
    // BugFix: for the first-time flicker problem - set the parent's view color here
    // and in Draw() use B_TRANSPARENT_COLOR
    SetViewColor (Parent() ? Parent()->ViewColor() : B_TRANSPARENT_COLOR);
    m_backBitmap = new BBitmap (Bounds(), B_RGB32, true);
    m_backView = new BView (Bounds(), "_2x_bufview_", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

    m_background = Parent() ? Parent()->ViewColor() : m_background;
    
    m_backBitmap->Lock();
    m_backBitmap->AddChild (m_backView);
    m_backView->SetViewColor (m_background);
    m_backView->SetLowColor (m_background);
    m_backView->SetHighColor (m_foreground);
    RenderView (true);
    m_backBitmap->Unlock();
    
    BView::AttachedToWindow();
}

//=============================================================================================================//

void BeezerStringView::SetHighColor (rgb_color a_color)
{
    m_foreground = a_color;
    BView::SetHighColor(a_color);
}

//=============================================================================================================//

void BeezerStringView::SetHighColor (uchar r, uchar g, uchar b, uchar a)
{
    m_foreground.red = r;
    m_foreground.green = g;
    m_foreground.blue = b;
    m_foreground.alpha = a;
    BView::SetHighColor (r, g, b, a);
}

//=============================================================================================================//

void BeezerStringView::SetFont (const BFont *font, uint32 mask)
{
    m_font = *font;
    BView::SetFont (font, mask);
}

//=============================================================================================================//

void BeezerStringView::GetPreferredSize (float *width, float *height)
{
    font_height fntHt;
    m_font.GetHeight(&fntHt);
    *height = 6 + fntHt.ascent + fntHt.descent;
    *width = 6 + m_font.StringWidth (m_text);
}

//=============================================================================================================//

void BeezerStringView::SendMouseEventsTo (BView *view)
{
    m_mouseTargetView = view;
}

//=============================================================================================================//

void BeezerStringView::MouseDown (BPoint point)
{
    if (m_mouseTargetView)
        m_mouseTargetView->MouseDown (point);
    
    return BView::MouseDown (point);
}

//=============================================================================================================//
