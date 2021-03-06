// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "BevelView.h"
#include "UIConstants.h"

const float BevelView::kInsetThickness   = 1.0;
const float BevelView::kOutsetThickness  = 1.0;
const float BevelView::kBulgeThickness   = 2.0;
const float BevelView::kDeepThickness    = 2.0;
const float BevelView::kNoBevelThickness = 0.0;

// TODO: Make layout-kit compatible constructor etc.
BevelView::BevelView(BRect frame, const char* name, bevel_type bevelMode, uint32 resizeMask, uint32 flags)
    : BView(frame, name, resizeMask, flags | B_FRAME_EVENTS)
{
    // Set up colors, edges and cache the Bounds() rectangle
    m_bevelType = bevelMode;
    rgb_color backColor;

    if (Parent())
        backColor = ViewColor();
    else
        backColor = ui_color(B_PANEL_BACKGROUND_COLOR);

    switch (m_bevelType)
    {
        case DEEP: case INSET:
            m_darkEdge1 = tint_color(backColor, B_DARKEN_2_TINT);
            m_darkEdge2 = tint_color(backColor, B_DARKEN_3_TINT);
            m_lightEdge = tint_color(backColor, B_LIGHTEN_MAX_TINT);
            m_edgeThickness = m_bevelType == INSET ? kInsetThickness : kDeepThickness;
            break;

        case OUTSET:
            m_darkEdge1 = tint_color(backColor, B_LIGHTEN_MAX_TINT);
            m_darkEdge2 = tint_color(backColor, B_DARKEN_3_TINT);
            m_lightEdge = tint_color(backColor, B_DARKEN_2_TINT);
            m_edgeThickness = kOutsetThickness;
            break;

        case BULGE:
            m_lightEdge = tint_color(backColor, B_DARKEN_3_TINT);
            m_darkEdge2 = tint_color(backColor, B_DARKEN_2_TINT);
            m_darkEdge1 = tint_color(backColor, B_LIGHTEN_1_TINT);
            m_edgeThickness = kBulgeThickness;
            break;

        case NO_BEVEL:
            break;
    }

    m_cachedRect = Bounds();
}


void BevelView::Draw(BRect updateRect)
{
    // Draw the edges based on the type of edge specified
    switch (m_bevelType)
    {
        case NO_BEVEL:
            break;

        case DEEP:
        case BULGE:
        {
            SetHighColor(m_darkEdge2);
            StrokeRect(BRect(m_cachedRect.left + 1, m_cachedRect.top + 1, m_cachedRect.right - 1,
                             m_cachedRect.bottom - 1));

            BeginLineArray(4L);
            AddLine(m_cachedRect.LeftTop(), m_cachedRect.RightTop(), m_darkEdge1);
            AddLine(m_cachedRect.LeftTop(), m_cachedRect.LeftBottom(), m_darkEdge1);

            AddLine(m_cachedRect.RightTop(), m_cachedRect.RightBottom(), m_lightEdge);
            AddLine(m_cachedRect.RightBottom(), m_cachedRect.LeftBottom(), m_lightEdge);
            EndLineArray();
            break;
        }

        case INSET:
        case OUTSET:
        {
            rgb_color c = m_lightEdge;
            c.red += 30; c.green += 30; c.blue += 30;
            SetHighColor(m_bevelType == INSET ? m_lightEdge : c);
            StrokeRect(Bounds());

            SetHighColor(m_darkEdge1);
            StrokeLine(m_cachedRect.LeftTop(), m_cachedRect.RightTop());
            StrokeLine(m_cachedRect.LeftTop(), m_cachedRect.LeftBottom());
            break;
        }
    }

    BView::Draw(updateRect);
}


void BevelView::FrameResized(float newWidth, float newHeight)
{
    // Cached drawing. Draw only when the "extra" area
    BRect newRect(Bounds());
    float minX, maxX, minY, maxY;

    // Capture the new co-ords of the "extra" rect
    minX = newRect.right > m_cachedRect.right ? m_cachedRect.right : newRect.right;
    maxX = newRect.right < m_cachedRect.right ? m_cachedRect.right : newRect.right;
    minY = newRect.bottom > m_cachedRect.bottom ? m_cachedRect.bottom : newRect.bottom;
    maxY = newRect.bottom < m_cachedRect.bottom ? m_cachedRect.bottom : newRect.bottom;

    // Draw if the rectangle is really valid
    m_cachedRect = newRect;
    if (minX != maxX)
        Invalidate(BRect(minX - 1, newRect.top, maxX, maxY));

    if (minY != maxY)
        Invalidate(BRect(newRect.left, minY - 1, maxX, maxY));

    BView::FrameResized(newWidth, newHeight);
}


float BevelView::EdgeThickness() const
{
    return m_edgeThickness;
}


void BevelView::AttachedToWindow()
{
    if (Parent())
        SetViewColor(Parent()->ViewColor());

    BView::AttachedToWindow();
}
