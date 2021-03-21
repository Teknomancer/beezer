/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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

#include <stdio.h>
#include <string.h>

#include "AppUtils.h"
#include "BarberPole.h"
#include "BeezerStringView.h"
#include "ImageButton.h"
#include "InfoBar.h"
#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "UIConstants.h"


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "InfoBar"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_COMMENT(x) x
#endif


// Initialize static non-integral vars here to make c++11 happy
// while avoiding the use of constexpr which would break gcc2
const float InfoBar::mk_vertSpacing = 3;
const float InfoBar::mk_horizSpacing = 2;


InfoBar::InfoBar(BRect frame, BList* slotPositions, const char* name, rgb_color backColor)
    : BView(frame, name, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
      m_finalSep(NULL),
      m_finalSepEdge(NULL),
      m_isHidden(false),
      m_horizGap((int32)static_cast<float>(mk_horizSpacing)),
      m_vertGap((int32)static_cast<float>(mk_vertSpacing)),
      m_filesTotal(0L),
      m_totalBytes(0),
      m_backColor(backColor)
{
	// @todo get rid of slotPositions, they aren't currently used
    m_slotPositions = slotPositions;
}



InfoBar::~InfoBar()
{
    delete m_slotPositions;
}



void InfoBar::AttachedToWindow()
{
    SetViewColor(m_backColor);
    m_lightEdge = K_WHITE_COLOR;
    m_darkEdge1 = tint_color(ViewColor(), B_DARKEN_1_TINT);
    m_darkEdge1.red -= 10; m_darkEdge1.green -= 10; m_darkEdge1.blue -= 10;
    m_darkEdge2 = tint_color(ViewColor(), B_DARKEN_2_TINT);
    m_darkEdge3 = tint_color(ViewColor(), B_DARKEN_3_TINT);

    m_barberPole = new BarberPole(BRect(m_horizGap + 6, m_vertGap, 0, Bounds().Height() - m_vertGap),
                                  "InfoBar:BarberPole");
    AddChild(m_barberPole);

    font_height fntHt;
    GetFontHeight(&fntHt);

    float normFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;

	// Use realistic maximums for now, later figure a way to resize this dynamically
    float oneX = StringWidth(B_TRANSLATE("Entries: ")) + StringWidth(B_TRANSLATE_COMMENT("of", "ex: 7 of 9"))
    					+ 2 * StringWidth(" 9999999");	// 99 million files
    float twoX = StringWidth(B_TRANSLATE("Bytes: ")) + StringWidth(B_TRANSLATE_COMMENT("of", "ex: 7 of 9")) + StringWidth("(100%)")
    					+ 2 * StringWidth(" 1099511627776"); // 1 TB
    m_filesStr = new BeezerStringView(BRect(m_barberPole->Frame().right + m_horizGap + 6,
                                            Bounds().Height() / 2 - normFontHeight / 2 - 1,
                                            m_barberPole->Frame().right + m_horizGap + 6 + oneX - 1,
                                            Bounds().Height() / 2 - normFontHeight / 2 + normFontHeight),
                                            "InfoBar:FilesStr", B_TRANSLATE("Entries: "));
    AddChild(m_filesStr);
    m_filesStr->SendMouseEventsTo(this);
    UpdateFilesDisplay(0L, 0L, true);
    AddSeparatorItem(m_filesStr->Frame().right + 1, false);

    m_bytesStr = new BeezerStringView(BRect(m_filesStr->Frame().right + m_horizGap + 6,
                                            Bounds().Height() / 2 - normFontHeight / 2 - 1,
                                            m_filesStr->Frame().right + m_horizGap + 6 + twoX - 1, Bounds().Height() / 2 -
                                            normFontHeight / 2 + normFontHeight), "InfoBar:BytesStr", B_TRANSLATE("Bytes: "));
    AddChild(m_bytesStr);
    m_bytesStr->SendMouseEventsTo(this);
    UpdateBytesDisplay(0L, 0L, true);
    AddSeparatorItem(m_filesStr->Frame().right + m_horizGap + 6 + twoX, true);
}



void InfoBar::UpdateFilesDisplay(int32 selectedCount, int32 totalCount, bool setTotalCount)
{
    if (setTotalCount == true)
        m_filesTotal = totalCount;

    char buf[strlen(B_TRANSLATE("Entries: ")) + 50];
    sprintf(buf, "%s%ld %s %ld", B_TRANSLATE("Entries: "), selectedCount, B_TRANSLATE_COMMENT("of", "ex: 7 of 9"), m_filesTotal);
    m_filesStr->SetText(buf);
    m_selectedFiles = selectedCount;
}



void InfoBar::UpdateBytesDisplay(uint32 selectedBytes, uint32 totalBytes, bool setTotalBytes)
{
    if (setTotalBytes == true)
        m_totalBytes = totalBytes;

    int8 percent = m_totalBytes > 0 ? (int8)(selectedBytes / (float)m_totalBytes * 100) : 0;
    char buf[strlen(B_TRANSLATE("Bytes: ")) + 50];
    sprintf(buf, "%s%ld %s %Ld (%d%%)", B_TRANSLATE("Bytes: "), selectedBytes, B_TRANSLATE_COMMENT("of", "ex: 7 of 9"), m_totalBytes, percent);
    m_bytesStr->SetText(buf);
    m_selectedBytes = selectedBytes;
}



void InfoBar::UpdateBy(int32 countBy, uint32 bytesBy)
{
    m_selectedFiles += countBy;
    char buf[strlen(B_TRANSLATE("Entries: ")) + 50];
    sprintf(buf, "%s%ld %s %ld", B_TRANSLATE("Entries: "), m_selectedFiles, B_TRANSLATE_COMMENT("of", "ex: 7 of 9"), m_filesTotal);
    m_filesStr->SetText(buf);

    m_selectedBytes += bytesBy;
    int8 percent = m_totalBytes > 0 ? (int8)(m_selectedBytes / (float)m_totalBytes * 100) : 0;
    char buf2[strlen(B_TRANSLATE("Bytes: ")) + 50];
    sprintf(buf2, "%s%Ld %s %Ld (%d%%)", B_TRANSLATE("Bytes: "), m_selectedBytes, B_TRANSLATE_COMMENT("of", "ex: 7 of 9"), m_totalBytes, percent);
    m_bytesStr->SetText(buf2);
}



void InfoBar::Draw(BRect updateRect)
{
    // Erase the old border (efficiently)
    BRect rect(Bounds());
    rect.left = rect.right - 2;
    SetHighColor(ViewColor());
    StrokeRect(rect);

    RenderEdges();

    BView::Draw(updateRect);
}



inline void InfoBar::RenderEdges()
{
    BRect bounds(Bounds());
    SetHighColor(m_lightEdge);
    rgb_color midCol = m_darkEdge1;
    rgb_color midColDark = m_darkEdge2;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;

    BeginLineArray(4);

    float rightLimit;
    if (m_isHidden == false && _prefs_interface.FindBoolDef(kPfFullLengthBars, false) == false)
        rightLimit = m_finalX - 2;
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



void InfoBar::MouseDown(BPoint point)
{
    // Right click detection
    int32 button;
    BMessage* message = Window()->CurrentMessage();
    if (message->FindInt32("buttons", &button) != B_OK)
        return;

    if (button == B_SECONDARY_MOUSE_BUTTON)
        Toggle();

    BView::MouseDown(point);
}



void InfoBar::Toggle()
{
    static float unHiddenHeight = Frame().Height() - m_vertGap;
    int32 separatorCount = m_separatorList.CountItems();

    if (!m_isHidden)
    {
        m_barberPole->Hide();
        m_filesStr->Hide();
        m_bytesStr->Hide();

        for (int32 i = 0L; i < separatorCount; i++)
            ((BView*)m_separatorList.ItemAtFast(i))->Hide();

        ResizeBy(0, -unHiddenHeight);
    }
    else
    {
        if (m_barberPole->IsAnimating())
            m_barberPole->Show();
            m_filesStr->Show();
            m_bytesStr->Show();

        for (int32 i = 0L; i < separatorCount; i++)
            ((BView*)m_separatorList.ItemAtFast(i))->Show();

        ResizeBy(0, unHiddenHeight);
    }

    m_isHidden = !m_isHidden;
    Invalidate(Bounds());

    // Inform the window about our state change
    BMessage notifierMessage(M_INFOBAR_TOGGLED);
    notifierMessage.AddBool(kHidden, m_isHidden);
    notifierMessage.AddFloat(kBarHeight, unHiddenHeight);
    Window()->PostMessage(&notifierMessage);
}



void InfoBar::AddSeparatorItem(float x, bool finalSeparator)
{
    rgb_color midCol = m_darkEdge1;
    midCol.red += 20; midCol.green += 20; midCol.blue += 20;

    for (int32 i = 0L; i < 1; i++)
    {
        BView* sepViewEdge1 = new BView(BRect(x, 1, x, Bounds().bottom - 2), "Infobar:Separator",
                                        B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepViewEdge1->SetViewColor(midCol);
        x++;

        BView* sepView = new BView(BRect(x, 0, x, Bounds().bottom - 1), "InfoBar:Separator",
                                   B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
        sepView->SetViewColor(m_darkEdge2);
        x++;

        if (finalSeparator == false)
        {
            BView* sepViewEdge2 = new BView(BRect(x, 0, x, Bounds().bottom - 1), "InfoBar:SeparatorEdge",
                                            B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
            sepViewEdge2->SetViewColor(m_lightEdge);
            x++;
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

    x ++;
    if (finalSeparator)
        m_finalX = x;
}



bool InfoBar::IsShown() const
{
    return !m_isHidden;
}



BarberPole* InfoBar::LoadIndicator() const
{
    return m_barberPole;
}



float InfoBar::Height() const
{
    if (m_isHidden)
        return Frame().Height() - m_vertGap;
    else
        return Frame().Height();
}



void InfoBar::Redraw()
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


