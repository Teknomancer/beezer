/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <Bitmap.h>

#include "StaticBitmapView.h"

//=============================================================================================================//

StaticBitmapView::StaticBitmapView (BRect frame, const char *name, BBitmap *bmp,
                                        uint32 resizeMask, uint32 flags)
    : BView (frame, name, resizeMask, flags),
        m_bitmap (bmp)
{
}

//=============================================================================================================//

StaticBitmapView::~StaticBitmapView ()
{
    delete m_bitmap;
}

//=============================================================================================================//

void StaticBitmapView::AttachedToWindow ()
{
    SetDrawingMode (B_OP_ALPHA);
}

//=============================================================================================================//

void StaticBitmapView::Draw (BRect updateRect)
{
    SetHighColor (ViewColor());
    FillRect (updateRect);
    DrawBitmap (m_bitmap);
}

//=============================================================================================================//
