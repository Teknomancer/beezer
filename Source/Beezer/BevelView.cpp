/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include "BevelView.h"
#include "UIConstants.h"

//=============================================================================================================//

BevelView::BevelView (BRect frame, const char *name, BevelType bevelMode, uint32 resizeMask, uint32 flags)
	: BView (frame, name, resizeMask, flags | B_FRAME_EVENTS)
{
	// Set up colors, edges and cache the Bounds() rectangle
	m_bevelType = bevelMode;
	rgb_color backColor;
	
	if (Parent())
		backColor = ViewColor();
	else
		backColor = K_BACKGROUND_COLOR;
	
	switch (m_bevelType)
	{
		case btDeep: case btInset:
			m_darkEdge1 = tint_color (backColor, B_DARKEN_2_TINT);
			m_darkEdge2 = tint_color (backColor, B_DARKEN_3_TINT);
			m_lightEdge = K_WHITE_COLOR;
			m_edgeThickness = m_bevelType == btInset ? btInsetThickness : btDeepThickness;
			break;

		case btOutset:
			m_darkEdge1 = K_WHITE_COLOR;
			m_darkEdge2 = tint_color (backColor, B_DARKEN_3_TINT);
			m_lightEdge = tint_color (backColor, B_DARKEN_2_TINT);
			m_edgeThickness = btOutsetThickness;
			break;
		
		case btBulge:
			m_lightEdge = tint_color (backColor, B_DARKEN_3_TINT);
			m_darkEdge2 = tint_color (backColor, B_DARKEN_2_TINT);
			m_darkEdge1 = tint_color (backColor, B_LIGHTEN_1_TINT);
			m_edgeThickness = btBulgeThickness;
			break;
		
		case btNoBevel:
			break;
	}
	
	m_cachedRect = Bounds();
}

//=============================================================================================================//

void BevelView::Draw (BRect updateRect)
{
	// Draw the edges based on the type of edge specified
	switch (m_bevelType)
	{
		case btNoBevel:
			break;
		
		case btDeep: case btBulge:
		{
			SetHighColor (m_darkEdge2);
			StrokeRect (BRect (m_cachedRect.left + 1, m_cachedRect.top + 1, m_cachedRect.right - 1,
							m_cachedRect.bottom - 1));
			
			BeginLineArray (4L);
			AddLine (m_cachedRect.LeftTop(), m_cachedRect.RightTop(), m_darkEdge1);
			AddLine (m_cachedRect.LeftTop(), m_cachedRect.LeftBottom(), m_darkEdge1);
			
			AddLine (m_cachedRect.RightTop(), m_cachedRect.RightBottom(), m_lightEdge);
			AddLine (m_cachedRect.RightBottom(), m_cachedRect.LeftBottom(), m_lightEdge);
			EndLineArray ();
			break;
		}
		
		case btInset: case btOutset:
		{
			rgb_color c = m_lightEdge;
			c.red += 30; c.green += 30; c.blue += 30;
			SetHighColor (m_bevelType == btInset ? m_lightEdge : c);
			StrokeRect (Bounds());
			
			SetHighColor (m_darkEdge1);
			StrokeLine (m_cachedRect.LeftTop(), m_cachedRect.RightTop());
			StrokeLine (m_cachedRect.LeftTop(), m_cachedRect.LeftBottom());
			break;
		}	
	}
	
	BView::Draw (updateRect);
}

//=============================================================================================================//

void BevelView::FrameResized (float newWidth, float newHeight)
{
	// Cached drawing. Draw only when the "extra" area
	BRect newRect (Bounds());
	float minX, maxX, minY, maxY;
	
	// Capture the new co-ords of the "extra" rect
	minX = newRect.right > m_cachedRect.right ? m_cachedRect.right : newRect.right;
	maxX = newRect.right < m_cachedRect.right ? m_cachedRect.right : newRect.right;
	minY = newRect.bottom > m_cachedRect.bottom ? m_cachedRect.bottom : newRect.bottom;
	maxY = newRect.bottom < m_cachedRect.bottom ? m_cachedRect.bottom : newRect.bottom;
	
	// Draw if the rectangle is really valid
	m_cachedRect = newRect;
	if (minX != maxX)
		Invalidate (BRect (minX - 1, newRect.top, maxX, maxY));

	if (minY != maxY)
		Invalidate (BRect (newRect.left, minY - 1, maxX, maxY));
	
	BView::FrameResized (newWidth, newHeight);
}

//=============================================================================================================//

float BevelView::EdgeThickness() const
{
	return m_edgeThickness;
}

//=============================================================================================================//

void BevelView::AttachedToWindow ()
{
	if (Parent())
		SetViewColor (Parent()->ViewColor());

	BView::AttachedToWindow ();
}

//=============================================================================================================//
