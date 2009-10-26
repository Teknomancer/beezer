/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include "BitmapMenuItem.h"

//=============================================================================================================//

BitmapMenuItem::BitmapMenuItem (BMenu *subMenu, BBitmap *bitmap)
	: BMenuItem (subMenu),
	m_bitmap (bitmap)
{
}

//=============================================================================================================//

void BitmapMenuItem::DrawContent ()
{
	BMenu *menu = Menu();
	menu->SetDrawingMode (B_OP_OVER);
	menu->DrawBitmap (m_bitmap, ContentLocation());
	menu->SetDrawingMode (B_OP_COPY);
}

//=============================================================================================================//

void BitmapMenuItem::GetContentSize (float *width, float *height)
{
	*width = m_bitmap->Bounds().Width() + 4;
	*height = m_bitmap->Bounds().Height() + 1;
}

//=============================================================================================================//
