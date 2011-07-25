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

#include <List.h>
#include <Window.h>
#include <Message.h>

#include "ToolBar.h"
#include "UIConstants.h"
#include "ImageButton.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"

//=============================================================================================================//

ToolBar::ToolBar (BRect frame, const char *name, rgb_color backColor)
    : BView (frame, name, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
        m_finalSep (NULL),
        m_finalSepEdge (NULL),
        m_isEnabled (true),
        m_isHidden (false),
        m_ptToDraw (mk_Border, mk_Border),
        m_nButtons (0),
        m_nSeparators (0L),
        m_horizGap ((int32)static_cast<float>(mk_horizSpacing)),
        m_vertGap ((int32)static_cast<float>(mk_vertSpacing)),
        m_backColor (backColor)
{
}

//=============================================================================================================//

ToolBar::~ToolBar ()
{
}

//=============================================================================================================//

void ToolBar::AttachedToWindow ()
{
    SetViewColor (m_backColor);
    m_lightEdge = K_WHITE_COLOR;
    m_darkEdge1 = tint_color (ViewColor(), B_DARKEN_1_TINT);
    m_darkEdge1.red -= 10; m_darkEdge1.green -= 10; m_darkEdge1.blue -= 10;
    m_darkEdge2 = tint_color (ViewColor(), B_DARKEN_2_TINT);
    m_darkEdge3 = tint_color (ViewColor(), B_DARKEN_3_TINT);
}

//=============================================================================================================//

void ToolBar::AddItem (ImageButton *button)
{
    if (button)
    {
        m_nButtons++;
        m_buttonList.AddItem ((void*)button);
        AddChild (button);
        button->MoveTo (m_ptToDraw);
        m_ptToDraw.x += button->Frame().Width() + m_horizGap;
    }
}

//=============================================================================================================//

void ToolBar::Draw (BRect updateRect)
{
    // Erase the old border (efficiently)
    BRect rect (Bounds());
    rect.left = rect.right - 2;

    SetHighColor (ViewColor());
    StrokeRect (rect);
    RenderEdges ();
    
    _inherited::Draw (updateRect);
}

//=============================================================================================================//

inline void ToolBar::RenderEdges ()
{
    // Inline function to do the draw (Keeps draw more readable)
    BRect bounds (Bounds());
    SetHighColor (m_lightEdge);
    rgb_color midCol = m_darkEdge1;
    rgb_color midColDark = m_darkEdge2;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;

    BeginLineArray (4);

    float rightLimit;
    if (m_isHidden == false && _prefs_interface.FindBoolDef (kPfFullLengthBars, false) == false)
        rightLimit = m_ptToDraw.x - 3;
    else
        rightLimit = bounds.right;
    
    // Draw the dark borders first
    AddLine (BPoint (0, bounds.bottom), BPoint (bounds.right, bounds.bottom), midColDark);
    AddLine (BPoint (0, bounds.bottom - 1), BPoint (rightLimit, bounds.bottom - 1), midCol);

    // Draw the light edges
    AddLine (BPoint (0, 0), BPoint (0, bounds.bottom - 1), m_lightEdge);
    AddLine (BPoint (0, 0), BPoint (rightLimit, 0), m_lightEdge);
    
    EndLineArray();
}

//=============================================================================================================//

float ToolBar::AddSeparatorItem (bool finalSeparator)
{
    rgb_color midCol = m_darkEdge1;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;
    
    float xPt = m_ptToDraw.x;
    for (int32 i = 0L; i < 1; i++)
    {
        BView *sepViewEdge1 = new BView (BRect (m_ptToDraw.x, 1, m_ptToDraw.x, Bounds().bottom - 2),
                             "Toolbar:Separator", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepViewEdge1->SetViewColor (midCol);
        m_ptToDraw.x++;
        
        BView *sepView = new BView (BRect (m_ptToDraw.x, 0, m_ptToDraw.x, Bounds().bottom - 1),
                             "ToolBar:Separator", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepView->SetViewColor (m_darkEdge2);
        m_ptToDraw.x++;

        if (finalSeparator == false)
        {
           BView *sepViewEdge2 = new BView (BRect (m_ptToDraw.x, 0, m_ptToDraw.x, Bounds().bottom - 1),
                                "ToolBar:SeparatorEdge", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
           sepViewEdge2->SetViewColor (m_lightEdge);
           m_ptToDraw.x++;
           AddChild (sepViewEdge2);
           m_separatorList.AddItem ((void*)sepViewEdge2);
        }
        
        AddChild (sepViewEdge1);
        AddChild (sepView);
        
        if (finalSeparator)
        {
           m_finalSepEdge = sepViewEdge1;
           m_finalSep = sepView;

           // If full length bars are needed by user then hide the final separators
           if (_prefs_interface.FindBoolDef (kPfFullLengthBars, false) == true)
           {
               m_finalSepEdge->Hide();
               m_finalSep->Hide();
           }
        }
        
        m_separatorList.AddItem ((void*)sepView);
        m_separatorList.AddItem ((void*)sepViewEdge1);
    }
    
    m_ptToDraw.x ++;
    return xPt;        // Return the floating-point x co-ordinate
}

//=============================================================================================================//

BRect ToolBar::InnerFrame ()
{
    BRect bounds (Bounds());
    return BRect (bounds.left, bounds.top, bounds.left + m_ptToDraw.x, bounds.bottom);
}

//=============================================================================================================//

void ToolBar::MouseDown (BPoint point)
{
    // Right click detection
    int32 button;
    BMessage *message = Window()->CurrentMessage();
    if (message->FindInt32 ("buttons", &button) != B_OK)
        return;
    
    if (button == B_SECONDARY_MOUSE_BUTTON)
        Toggle();
    
    _inherited::MouseDown (point);
}

//=============================================================================================================//

void ToolBar::Toggle ()
{
    static float unHiddenHeight = Frame().Height() - m_vertGap;
    int32 buttonCount = m_buttonList.CountItems();
    int32 seperatorCount = m_separatorList.CountItems();
    
    if (!m_isHidden)
    {
        for (int32 i = 0L; i < buttonCount; i++)
           ((ImageButton*)m_buttonList.ItemAtFast(i))->Hide();
        
        for (int32 i = 0L; i < seperatorCount; i++)
           ((BView*)m_separatorList.ItemAtFast(i))->Hide();
        
        ResizeBy (0, -unHiddenHeight);
    }
    else
    {
        for (int32 i = 0L; i < buttonCount; i++)
           ((ImageButton*)m_buttonList.ItemAtFast(i))->Show();
        
        for (int32 i = 0L; i < seperatorCount; i++)
           ((BView*)m_separatorList.ItemAtFast(i))->Show();
    
        ResizeBy (0, unHiddenHeight);
    }

    m_isHidden = !m_isHidden;
    Invalidate (Bounds());

    // Inform the window about our state change
    BMessage notifierMessage (M_TOOLBAR_TOGGLED);
    notifierMessage.AddBool (kHidden, m_isHidden);
    notifierMessage.AddFloat (kBarHeight, unHiddenHeight);
    Window()->PostMessage (&notifierMessage);
}

//=============================================================================================================//

bool ToolBar::IsShown () const
{
    return !m_isHidden;
}

//=============================================================================================================//

void ToolBar::SetEnabled (bool enable)
{
    if (m_isEnabled == enable)
        return;
    
    m_isEnabled = enable;
    int32 buttonCount = m_buttonList.CountItems();
    for (int32 i = 0L; i < buttonCount; i++)
        ((ImageButton*)m_buttonList.ItemAtFast(i))->SetEnabled (m_isEnabled);
}

//=============================================================================================================//

bool ToolBar::IsEnabled () const
{
    return m_isEnabled;
}

//=============================================================================================================//

float ToolBar::Height () const
{
    if (m_isHidden)
        return Frame().Height() - m_vertGap;
    else
        return Frame().Height();
}

//=============================================================================================================//

void ToolBar::Redraw ()
{
    // Called when preferences have changed
    if (m_finalSepEdge == NULL || m_finalSep == NULL)
        return;
    
    // If full length bars are needed by user then hide the final separators
    if (_prefs_interface.FindBoolDef (kPfFullLengthBars, false) == true)
    {
        m_finalSepEdge->Hide();
        m_finalSep->Hide();
    }
    else
    {
        m_finalSepEdge->Show();
        m_finalSep->Show();
    }

    Invalidate ();
}

//=============================================================================================================//
