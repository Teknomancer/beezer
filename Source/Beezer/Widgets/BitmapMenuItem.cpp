// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "BitmapMenuItem.h"

#include <Bitmap.h>


BitmapMenuItem::BitmapMenuItem(BMenu* subMenu, BBitmap* bitmap)
    : BMenuItem(subMenu),
      m_bitmap(bitmap)
{
}


void BitmapMenuItem::DrawContent()
{
    BMenu* menu = Menu();
    menu->SetDrawingMode(B_OP_OVER);
    menu->DrawBitmap(m_bitmap, ContentLocation());
    menu->SetDrawingMode(B_OP_COPY);
}


void BitmapMenuItem::GetContentSize(float* width, float* height)
{
    *width = m_bitmap->Bounds().Width();
    *height = m_bitmap->Bounds().Height();
}
