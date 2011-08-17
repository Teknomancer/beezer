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
