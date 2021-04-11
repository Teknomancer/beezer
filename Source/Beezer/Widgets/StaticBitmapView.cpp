// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2012 Chris Roberts.
// All rights reserved.

#include "StaticBitmapView.h"

#include <Bitmap.h>


StaticBitmapView::StaticBitmapView(BRect frame, const char* name, BBitmap* bmp,
                                   uint32 resizeMask, uint32 flags)
    : BView(frame, name, resizeMask, flags),
      m_bitmap(bmp)
{
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


StaticBitmapView::~StaticBitmapView()
{
    delete m_bitmap;
}


void StaticBitmapView::AttachedToWindow()
{
    SetDrawingMode(B_OP_ALPHA);
}


void StaticBitmapView::Draw(BRect updateRect)
{
    SetHighColor(ViewColor());
    FillRect(updateRect);
    DrawBitmap(m_bitmap);
}
