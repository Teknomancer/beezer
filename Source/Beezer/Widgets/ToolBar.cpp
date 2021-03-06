// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "ToolBar.h"
#include "ImageButton.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"

#include <Window.h>

static float const kVertSpacing  = 3;
static float const kHorizSpacing = 2;

float const ToolBar::kBorder     = 2;


ToolBar::ToolBar(BRect frame, const char* name, rgb_color backColor)
    : BView(frame, name, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
      m_finalSep(NULL),
      m_finalSepEdge(NULL),
      m_isEnabled(true),
      m_isHidden(false),
      m_ptToDraw(kBorder, kBorder),
      m_nButtons(0L),
      m_nSeparators(0L),
      m_horizGap(kHorizSpacing),
      m_vertGap(kVertSpacing),
      m_backColor(backColor)
{
}


ToolBar::~ToolBar()
{
}


void ToolBar::AttachedToWindow()
{
    SetViewColor(m_backColor);
    m_lightEdge = tint_color(ViewColor(), B_LIGHTEN_2_TINT);
    m_darkEdge1 = tint_color(ViewColor(), B_DARKEN_1_TINT);
    m_darkEdge1.red -= 10; m_darkEdge1.green -= 10; m_darkEdge1.blue -= 10;
    m_darkEdge2 = tint_color(ViewColor(), B_DARKEN_2_TINT);
    m_darkEdge3 = tint_color(ViewColor(), B_DARKEN_3_TINT);
}


void ToolBar::AddItem(ImageButton* button)
{
    if (button)
    {
        m_nButtons++;
        m_buttonList.AddItem((void*)button);
        AddChild(button);
        button->MoveTo(m_ptToDraw);
        m_ptToDraw.x += button->Frame().Width() + m_horizGap;
    }
}


void ToolBar::Draw(BRect updateRect)
{
    // Erase the old border (efficiently)
    BRect rect(Bounds());
    rect.left = rect.right - 2;

    SetHighColor(ViewColor());
    StrokeRect(rect);
    RenderEdges();

    _inherited::Draw(updateRect);
}


inline void ToolBar::RenderEdges()
{
    // Inline function to do the draw (Keeps draw more readable)
    BRect bounds(Bounds());
    SetHighColor(m_lightEdge);
    rgb_color midCol = m_darkEdge1;
    rgb_color midColDark = m_darkEdge2;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;

    BeginLineArray(4);

    float rightLimit;
    if (m_isHidden == false && _prefs_interface.FindBoolDef(kPfFullLengthBars, false) == false)
        rightLimit = m_ptToDraw.x - 3;
    else
        rightLimit = bounds.right;

    // Draw the dark borders first
    AddLine(BPoint(0, bounds.bottom), BPoint(bounds.right, bounds.bottom), midColDark);
    AddLine(BPoint(0, bounds.bottom - 1), BPoint(rightLimit, bounds.bottom - 1), midCol);

    // Draw the light edges
    AddLine(BPoint(0, 0), BPoint(0, bounds.bottom - 1), m_lightEdge);
    AddLine(BPoint(0, 0), BPoint(rightLimit, 0), m_lightEdge);

    EndLineArray();
}


float ToolBar::AddSeparatorItem(bool finalSeparator)
{
    rgb_color midCol = m_darkEdge1;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;

    float xPt = m_ptToDraw.x;
    for (int32 i = 0L; i < 1; i++)
    {
        BView* sepViewEdge1 = new BView(BRect(m_ptToDraw.x, 1, m_ptToDraw.x, Bounds().bottom - 2),
                                        "Toolbar:Separator", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepViewEdge1->SetViewColor(midCol);
        m_ptToDraw.x++;

        BView* sepView = new BView(BRect(m_ptToDraw.x, 0, m_ptToDraw.x, Bounds().bottom - 1),
                                   "ToolBar:Separator", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepView->SetViewColor(m_darkEdge2);
        m_ptToDraw.x++;

        if (finalSeparator == false)
        {
            BView* sepViewEdge2 = new BView(BRect(m_ptToDraw.x, 0, m_ptToDraw.x, Bounds().bottom - 1),
                                            "ToolBar:SeparatorEdge", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
            sepViewEdge2->SetViewColor(m_lightEdge);
            m_ptToDraw.x++;
            AddChild(sepViewEdge2);
            m_separatorList.AddItem((void*)sepViewEdge2);
        }

        AddChild(sepViewEdge1);
        AddChild(sepView);

        if (finalSeparator)
        {
            m_finalSepEdge = sepViewEdge1;
            m_finalSep = sepView;

            // If full length bars are needed by user then hide the final separators
            if (_prefs_interface.FindBoolDef(kPfFullLengthBars, false) == true)
            {
                m_finalSepEdge->Hide();
                m_finalSep->Hide();
            }
        }

        m_separatorList.AddItem((void*)sepView);
        m_separatorList.AddItem((void*)sepViewEdge1);
    }

    m_ptToDraw.x ++;
    return xPt;        // Return the floating-point x co-ordinate
}


BRect ToolBar::InnerFrame()
{
    BRect bounds(Bounds());
    return BRect(bounds.left, bounds.top, bounds.left + m_ptToDraw.x, bounds.bottom);
}


void ToolBar::MouseDown(BPoint point)
{
    // Right click detection
    int32 button;
    BMessage* message = Window()->CurrentMessage();
    if (message->FindInt32("buttons", &button) != B_OK)
        return;

    if (button == B_SECONDARY_MOUSE_BUTTON)
        Toggle();

    _inherited::MouseDown(point);
}


void ToolBar::Toggle()
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

        ResizeBy(0, -unHiddenHeight);
    }
    else
    {
        for (int32 i = 0L; i < buttonCount; i++)
            ((ImageButton*)m_buttonList.ItemAtFast(i))->Show();

        for (int32 i = 0L; i < seperatorCount; i++)
            ((BView*)m_separatorList.ItemAtFast(i))->Show();

        ResizeBy(0, unHiddenHeight);
    }

    m_isHidden = !m_isHidden;
    Invalidate(Bounds());

    // Inform the window about our state change
    BMessage notifierMessage(M_TOOLBAR_TOGGLED);
    notifierMessage.AddBool(kHidden, m_isHidden);
    notifierMessage.AddFloat(kBarHeight, unHiddenHeight);
    Window()->PostMessage(&notifierMessage);
}


bool ToolBar::IsShown() const
{
    return !m_isHidden;
}


void ToolBar::SetEnabled(bool enable)
{
    if (m_isEnabled == enable)
        return;

    m_isEnabled = enable;
    int32 buttonCount = m_buttonList.CountItems();
    for (int32 i = 0L; i < buttonCount; i++)
        ((ImageButton*)m_buttonList.ItemAtFast(i))->SetEnabled(m_isEnabled);
}


bool ToolBar::IsEnabled() const
{
    return m_isEnabled;
}


float ToolBar::Height() const
{
    if (m_isHidden)
        return Frame().Height() - m_vertGap;
    else
        return Frame().Height();
}


void ToolBar::Redraw()
{
    // Called when preferences have changed
    if (m_finalSepEdge == NULL || m_finalSep == NULL)
        return;

    // If full length bars are needed by user then hide the final separators
    if (_prefs_interface.FindBoolDef(kPfFullLengthBars, false) == true)
    {
        m_finalSepEdge->Hide();
        m_finalSep->Hide();
    }
    else
    {
        m_finalSepEdge->Show();
        m_finalSep->Show();
    }

    Invalidate();
}
