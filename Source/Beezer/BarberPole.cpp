/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#include <Bitmap.h>
#include <Message.h>
#include <Window.h>

#include "AppConstants.h"
#include "BarberPole.h"
#include "LocalUtils.h"



BarberPole::BarberPole(BRect frame, const char* name)
    : BevelView(frame, name, btInset, B_FOLLOW_LEFT, B_WILL_DRAW | B_PULSE_NEEDED)
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


