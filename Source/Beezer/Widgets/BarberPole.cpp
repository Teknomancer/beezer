// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "BarberPole.h"
#include "AppConstants.h"
#include "LocalUtils.h"

#include <Bitmap.h>
#include <Window.h>


BarberPole::BarberPole(BRect frame, const char* name)
    : BevelView(frame, name, BevelView::BevelType::INSET, B_FOLLOW_LEFT, B_WILL_DRAW | B_PULSE_NEEDED)
{
    m_poleImage = ResBitmap("Img:BarberPole");
    m_imageHeight = m_poleImage->Bounds().Height();
    m_edgeThickness = EdgeThickness();
    m_y = m_edgeThickness;

    m_animate = false;
    Hide();
}


void BarberPole::Draw(BRect updateRect)
{
    DrawBitmap(m_poleImage, BPoint(m_edgeThickness, -m_y));
    BevelView::Draw(updateRect);
}


void BarberPole::Pulse()
{
    Animate();
}


void BarberPole::GetPreferredSize(float* width, float* height)
{
    // Auto size the control (according to the bitmap we got)
    // The height is caller controllable - if zero we resize to half of bitmap height
    *width = m_poleImage->Bounds().Width() + 2 * m_edgeThickness;
    if (Bounds().Height() == 0)
        *height = (m_poleImage->Bounds().Height() / 2.0) + m_edgeThickness;
    else
        *height = Bounds().Height();
}


float BarberPole::Width() const
{
    return m_poleImage->Bounds().Width() + 2 * m_edgeThickness;
}


void BarberPole::AttachedToWindow()
{
    ResizeToPreferred();
    BevelView::AttachedToWindow();
}


void BarberPole::SetValue(bool animate, bool adjustPulse)
{
    m_animate = animate;
    if (m_animate)
    {
        if (adjustPulse)
            Window()->SetPulseRate(Window()->PulseRate() > 0 ? Window()->PulseRate() : K_BARBERPOLE_PULSERATE);
        Show();
    }
    else
    {
        if (adjustPulse)
            Window()->SetPulseRate(0);
        Hide();
    }
}


bool BarberPole::IsAnimating() const
{
    return m_animate;
}


void BarberPole::Animate()
{
    if (m_y <= Bounds().bottom - 2 * m_edgeThickness)
        m_y ++;
    else
        m_y = m_edgeThickness + 1;

    if (m_animate)
        Invalidate();
}
