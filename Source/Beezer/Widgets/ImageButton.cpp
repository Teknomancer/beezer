// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "ImageButton.h"

#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Window.h>

#include <string.h>     // required for gcc2?

static const float kContextWidth = 14.0;


ImageButton::ImageButton(BRect frame, const char* name, const char* text, BBitmap* smallIcon,
                         BBitmap* disabled, BMessage* message, bool popUpMenu, const rgb_color bgColor,
                         text_position textPos, bool borders, bool smallFont, bool hoverHighlight, uint32 resizeMask,
                         uint32 flags)
    : BView(frame, name, resizeMask, flags),
      m_handler(NULL),
      m_clickBitmap(smallIcon),
      m_disabledBitmap(disabled),
      m_clickMessage(message),
      m_contextMenu(NULL),
      m_backColor(bgColor),
      m_drawingTriangle(false),
      m_popUpMenu(popUpMenu),
      m_isPushed(false),
      m_isEnabled(true),
      m_isClickable(true),
      m_borders(borders),
      m_smallFont(smallFont),
      m_hoverHighlight(hoverHighlight),
      m_mouseInside(false),
      m_textPosition(textPos)
{
    Init(text);
}


ImageButton::ImageButton(const char* name, const char* text, BBitmap* smallIcon,
                         BBitmap* disabled, BMessage* message, bool popUpMenu, const rgb_color bgColor,
                         text_position textPos, bool borders, bool smallFont, bool hoverHighlight,
                         uint32 flags)
    : BView(name, flags),
      m_handler(NULL),
      m_clickBitmap(smallIcon),
      m_disabledBitmap(disabled),
      m_clickMessage(message),
      m_contextMenu(NULL),
      m_backColor(bgColor),
      m_drawingTriangle(false),
      m_popUpMenu(popUpMenu),
      m_isPushed(false),
      m_isEnabled(true),
      m_isClickable(true),
      m_borders(borders),
      m_smallFont(smallFont),
      m_hoverHighlight(hoverHighlight),
      m_mouseInside(false),
      m_textPosition(textPos)
{
    Init(text);
}


void ImageButton::Init(const char* text)
{
	if (text != NULL)
    {
        m_buttonText = new char [strlen(text) + 1];
        strcpy(const_cast<char*>(m_buttonText), text);
    }
    else
        m_buttonText = NULL;

    m_marginWidth = 5;
    m_marginHeight = 4;

    BFont font(be_plain_font);
    font.SetSize(font.Size() > 10.0 ? 10 : font.Size());
    font_height fontHeight;
    font.GetHeight(&fontHeight);
    SetFont(&font);
    m_fontPlacement = fontHeight.ascent + fontHeight.descent - 1;

    // Calculate the tinted colours for our edges
    m_lightEdge = tint_color(m_backColor, B_LIGHTEN_2_TINT);
    m_lightEdge2 = tint_color(m_backColor, B_DARKEN_1_TINT);
    m_darkEdge1 = tint_color(m_backColor, B_DARKEN_2_TINT);
    m_darkEdge2 = tint_color(m_backColor, B_DARKEN_4_TINT);
}


ImageButton::~ImageButton()
{
    // Do NOT delete the bitmaps
    if (m_clickMessage) delete m_clickMessage;
    if (m_buttonText) delete[] m_buttonText;
    if (m_contextMenu) delete m_contextMenu;
}


void ImageButton::SetContextMenu(BPopUpMenu* menu)
{
    if (m_contextMenu)
        delete m_contextMenu;

    m_contextMenu = menu;
    m_popUpMenu = true;
    Invalidate();
}


void ImageButton::Draw(BRect updateRect)
{
    SetViewColor(B_TRANSPARENT_COLOR);

    if (m_isPushed && m_mouseInside == true)
    {
        PushButton(updateRect);
        return BView::Draw(updateRect);
    }

    // Erase everything & redraw borders
    MovePenTo(0, 0);
    SetHighColor(m_backColor);
    FillRect(updateRect);
    if (!m_hoverHighlight || m_mouseInside)
    {
        DrawOutsideEdge(updateRect);
        DrawShinyEdge(updateRect, false);
    }

    // Draw the picture & render the text on its left, check for 'activeness'
    // and for missing data as well (in case we have NULL bmps or text)
    SetDrawingMode(B_OP_ALPHA);
    if (m_textPosition == RIGHT_OF_ICON)
        MovePenTo(m_marginWidth, m_marginHeight);
    else
    {
        if (m_buttonText == NULL)
            MovePenTo(m_marginWidth, m_marginHeight);
        else
        {
            MovePenTo(Bounds().Width() / 2.0 - 10.0, m_marginHeight);
            if (m_popUpMenu)
                MovePenBy(-kContextWidth / 2.0, 0);
        }
    }

    if (m_isEnabled)
    {
        if (m_clickBitmap)
            DrawBitmapAsync(m_clickBitmap);
        else if (m_disabledBitmap)
            DrawBitmapAsync(m_disabledBitmap);
    }
    else
    {
        if (m_disabledBitmap)
            DrawBitmapAsync(m_disabledBitmap);
        else if (m_clickBitmap)
            DrawBitmapAsync(m_clickBitmap);
    }

    SetDrawingMode(B_OP_COPY);
    if (m_textPosition == RIGHT_OF_ICON)
    {
        if (m_clickBitmap)
            MovePenTo(2 * m_marginWidth + 20, m_marginHeight + m_fontPlacement);
        else
            MovePenTo(m_marginWidth, m_marginHeight + m_fontPlacement);
    }
    else
    {
        float strWidth = m_buttonText != NULL ? StringWidth(m_buttonText) : 0;
        float viewWidth = Bounds().Width();
        if (m_clickBitmap || m_disabledBitmap)
            MovePenTo(viewWidth / 2.0 - strWidth / 2.0, 20 + m_marginHeight + 1 + m_fontPlacement - 1);
        else
            MovePenTo(viewWidth / 2.0 - strWidth / 2.0, m_marginHeight + 1 + m_fontPlacement - 1);
        if (m_popUpMenu)
            MovePenBy(-kContextWidth / 2.0, 0);
    }

    // SetHighColor (foreground) and LowColor (to background color) for proper anti-aliasing
    SetLowColor(m_backColor);
    if (m_isEnabled == false)
    {
        BPoint origPt = PenLocation();
        MovePenBy(1, 1);
        SetHighUIColor(B_CONTROL_TEXT_COLOR, B_DISABLED_MARK_TINT);
        if (m_buttonText != NULL)
            DrawString(m_buttonText);
        MovePenTo(origPt);
    }

    SetHighColor(m_isEnabled == true ? ui_color(B_CONTROL_TEXT_COLOR) : tint_color(m_backColor, B_DISABLED_LABEL_TINT));
    if (m_buttonText)
        DrawString(m_buttonText);

    if (m_popUpMenu)
    {
        BRect bounds(Bounds());
        DrawContextMenuTriangle(BPoint(bounds.right - kContextWidth + 1, bounds.bottom / 2.0 - 5));
    }

    BView::Draw(updateRect);
}


void ImageButton::DrawContextMenuTriangle(BPoint topLeftPoint)
{
    // Draws the down-pointing triangle
    rgb_color slightlyDarker = m_darkEdge1;
    slightlyDarker.red -= 30; slightlyDarker.green -= 30; slightlyDarker.blue -= 30;

    SetHighColor(slightlyDarker);
    BPoint topRightPoint = topLeftPoint;
    topRightPoint.x += 8;
    BPoint bottomMiddlePoint = topLeftPoint;
    bottomMiddlePoint.x += 4;
    bottomMiddlePoint.y += 9;
    StrokeTriangle(topLeftPoint, topRightPoint, bottomMiddlePoint);
    topLeftPoint.x ++; topLeftPoint.y++;
    bottomMiddlePoint.y--;

    SetHighColor(m_lightEdge);
    topRightPoint.x--;
    topRightPoint.y++;
    StrokeLine(topLeftPoint, topRightPoint);
    topLeftPoint.y++; topLeftPoint.x++;
    bottomMiddlePoint.y--;
    StrokeLine(topLeftPoint, bottomMiddlePoint);

    rgb_color darker = m_lightEdge2;
    darker.red -= 18; darker.green -= 18; darker.blue -= 18;
    SetHighColor(darker);
    topRightPoint.x--;
    topRightPoint.y++;
    bottomMiddlePoint.y -= 2;
    bottomMiddlePoint.x ++;
    StrokeLine(topRightPoint, bottomMiddlePoint);
}


void ImageButton::MouseMoved(BPoint point, uint32 status, const BMessage* dragInfo)
{
    // Handle the mouse -- Don't alter this unless you know what you are doing
    int32 buttons = Window()->CurrentMessage()->FindInt32("buttons");

    switch (status)
    {
        case B_ENTERED_VIEW:
        {
            if (!Window()->IsActive())
                break;

            m_firstClick = false;
            m_mouseInside = true;

            if (buttons != 0 && m_isPushed == true && m_firstClick == false && m_isEnabled)
            {
                BRect rect = Bounds();
                if (m_popUpMenu && m_borders)
                {
                    if (m_drawingTriangle)
                        rect.left = rect.right - kContextWidth - 1;
                    else
                        rect.right = rect.right - kContextWidth - 4;
                }

                if (rect.Contains(point))
                    Invalidate(rect);
            }
            else if (m_hoverHighlight == true && buttons == 0 && m_borders == true)
                HighlightNow(false);

            break;
        }

        case B_INSIDE_VIEW:
        {
            if (!Window()->IsActive())
                break;

            if (m_hoverHighlight && buttons == 0)
                HighlightNow(false);

            m_mouseInside = true;

            break;
        }

        case B_EXITED_VIEW:
        {
            m_mouseInside = false;
            if (m_isPushed == true && m_isEnabled == true)
            {
                if (m_hoverHighlight)
                {
                    Draw(Bounds());                // Invalidate() doesn't work!
                    HighlightNow(false);
                }
                else
                {
                    if (m_borders == true)           // Saves unnecessary re-draws when in no border mode
                        Draw(Bounds());
                }
            }
            else
            {
                if (m_borders == true)               // Saves unncessesary re-draws when in no border mode
                    Draw(Bounds());
            }
            break;
        }
    }

    // No need to call base class as BView's MouseMoved() is empty - we can avoid the function
    // call overhead
    //return BView::MouseMoved (point, status, dragInfo);
}


void ImageButton::DrawOutsideEdge(BRect bounds)
{
    if (m_borders == false)
        return;

    SetHighColor(m_darkEdge2);
    StrokeRect(bounds);

    SetHighColor(m_darkEdge1);
    StrokeLine(BPoint(0, 0), BPoint(bounds.right, 0));
    StrokeLine(BPoint(0, 0), BPoint(0, bounds.bottom));
}


void ImageButton::DrawShinyEdge(BRect bounds, bool isPressing)
{
    // Draw the four edges. isPressing determines which colour goes to which edge (to give pressed effect)
    if (m_borders == false)
        return;

    if (isPressing == false)
    {
        int8 lineCount = 4L;
        if (m_popUpMenu)
            lineCount += 3;

        BeginLineArray(lineCount);
        AddLine(BPoint(1, 1), BPoint(bounds.right - 1, 1), m_lightEdge);
        AddLine(BPoint(1, 1), BPoint(1, bounds.bottom - 1), m_lightEdge);
        AddLine(BPoint(2, bounds.bottom - 1), BPoint(bounds.right - 1, bounds.bottom - 1), m_lightEdge2);
        AddLine(BPoint(bounds.right - 1 , 1), BPoint(bounds.right - 1, bounds.bottom - 1), m_lightEdge2);

        // Draw popup menu separator line
        if (m_popUpMenu == true)
        {
            BPoint sepPtTop(bounds.right - kContextWidth - 3, bounds.top + 1);
            BPoint sepPtBtm(bounds.right - kContextWidth - 3, bounds.bottom - 1);
            AddLine(sepPtTop, sepPtBtm, m_lightEdge2);
            sepPtTop.y -= 1;
            sepPtTop.x++;
            sepPtBtm.x++;
            AddLine(sepPtTop, sepPtBtm, m_darkEdge1);
            sepPtTop.y+=1;
            sepPtTop.x++;
            sepPtBtm.x++;
            AddLine(sepPtTop, sepPtBtm, m_lightEdge);
        }
    }
    else
    {
        BeginLineArray(6L);
        AddLine(BPoint(1, 1), BPoint(bounds.right - 1, 1), m_darkEdge2);
        AddLine(BPoint(1, 1), BPoint(1, bounds.bottom - 1), m_darkEdge2);
        AddLine(BPoint(1, bounds.bottom), BPoint(bounds.right, bounds.bottom), m_lightEdge);
        AddLine(BPoint(bounds.right, 1), BPoint(bounds.right, bounds.bottom - 1), m_lightEdge);

        AddLine(BPoint(2, bounds.bottom - 1), BPoint(bounds.right - 1, bounds.bottom - 1), m_backColor);
        AddLine(BPoint(bounds.right - 1 , 1), BPoint(bounds.right - 1, bounds.bottom - 1), m_backColor);
    }

    EndLineArray();
}


void ImageButton::MouseDown(BPoint point)
{
    SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS | B_SUSPEND_VIEW_FOCUS);

    // Render a pushed button if the view is currently an enabled button
    m_drawingTriangle = false;
    BRect bounds;
    if (m_popUpMenu && m_borders)
    {
        bounds.Set(0, 0, Bounds().right - kContextWidth - 4, Bounds().bottom);
        if (bounds.Contains(point) == false && Bounds().Contains(point))
        {
            bounds.Set(Bounds().right - kContextWidth - 1, 0, Bounds().right, Bounds().bottom);
            m_drawingTriangle = true;
        }
    }
    else
        bounds = Bounds();

    if (bounds.Contains(point) == true && m_isClickable == true && m_isEnabled == true)
    {
        m_isPushed = true;
        Invalidate(bounds);

        if (m_drawingTriangle && m_contextMenu)
        {
            // Bug-fix: now clicking several times in the triangle region doesn't show context menu
            // again and again - this happened because of SetMouseEventMask which sends us mouseDOWNs
            // even though we try and ignore it in ShowContextMenu()
            BPoint curPoint; uint32 buttons;
            GetMouse(&curPoint, &buttons, false);     // Important that current state of mouse is here
            if (bounds.Contains(curPoint))
                ShowContextMenu(point);
        }
    }

    return;
}


void ImageButton::MouseUp(BPoint point)
{
    if (m_isEnabled == false || Window()->IsActive() == false)
        return;

    BRect bounds(Bounds());
    if (bounds.Contains(point) == false)
    {
        m_isPushed = false;
        m_drawingTriangle = false;
        Invalidate();
        return;
    }

    if (m_popUpMenu && m_borders && m_firstClick == false && m_isPushed == true)
    {
        BRect triBounds = bounds;
        triBounds.left = bounds.right - kContextWidth - 1;
        if (triBounds.Contains(point))
        {
            if (!m_contextMenu)
            {
                m_isPushed = false;
                Draw(triBounds);
                Draw(bounds);
            }
            return;
        }

        if (m_drawingTriangle == true)
        {
            m_drawingTriangle = false;
            m_isPushed = false;
            Invalidate();
            return;
        }
    }

    Draw(bounds);
    DrawOutsideEdge(bounds);
    DrawShinyEdge(bounds, false);

    if (bounds.Contains(point) == true)
    {
        if (m_firstClick == false && m_isPushed == true)
        {
            // Critical order, don't change
            m_isPushed = false;
            Draw(bounds);
            if (m_clickMessage)
            {
                if (m_handler == NULL)
                    Window()->PostMessage(m_clickMessage);
                else
                    m_handler->MessageReceived(m_clickMessage);
            }
        }
        else
        {
            m_isPushed = false;
            m_firstClick = false;
        }
        //else    // It's not nice to call MouseMoved() so lets do something better hmm :)
        //    MouseMoved (point, B_ENTERED_VIEW, NULL);
    }

    return BView::MouseUp(point);
}


void ImageButton::PushButton(BRect rect)
{
    // Give a pushed button effect
    if (m_firstClick == true || m_isEnabled == false || m_isClickable == false)
        return;

    BRect bounds(rect);
    DrawOutsideEdge(bounds);
    DrawShinyEdge(bounds, true);

    // Erase view (leave border alone) then re-draw
    SetHighColor(m_backColor);
    if (m_borders)
        FillRect(bounds.InsetByCopy(2, 2));
    else
        FillRect(bounds.InsetByCopy(1, 1));

    if (m_popUpMenu && m_borders && m_drawingTriangle == true)
    {
        DrawContextMenuTriangle(BPoint(bounds.right - kContextWidth + 2, bounds.bottom / 2.0 - 4 + 1));
        SetHighColor(m_darkEdge2);
        StrokeLine(BPoint(bounds.left, bounds.bottom - 1), BPoint(bounds.left, bounds.bottom));
        SetDrawingMode(B_OP_SUBTRACT);
        SetHighColor(25, 25, 25, 255);
        SetLowColor(m_backColor);
        FillRect(bounds.InsetByCopy(2, 2));

        SetDrawingMode(B_OP_COPY);
        return;
    }

    // Draw with transparency the picture and then the text
    SetDrawingMode(B_OP_ALPHA);
    if (m_textPosition == RIGHT_OF_ICON)
        MovePenTo(m_marginWidth + 1, m_marginHeight + 1);
    else
    {
        if (m_buttonText == NULL)
            MovePenTo(m_marginWidth + 1, m_marginHeight + 1);
        else
            MovePenTo(bounds.Width() / 2.0 - 10.0 + 1, m_marginHeight + 1);

        if (m_popUpMenu)
            MovePenBy(1, 0);
    }

    if (m_clickBitmap)
        DrawBitmapAsync(m_clickBitmap);
    else if (m_disabledBitmap)
        DrawBitmapAsync(m_disabledBitmap);

    SetDrawingMode(B_OP_COPY);
    SetHighUIColor(B_CONTROL_TEXT_COLOR);
    SetLowColor(m_backColor);
    if (m_textPosition == RIGHT_OF_ICON)
    {
        if (m_clickBitmap || m_disabledBitmap)
            MovePenTo(2 * (m_marginWidth) + 20 + 1, m_marginHeight + 1 + m_fontPlacement);
        else
            MovePenTo(m_marginWidth + 1, m_marginHeight + 1 + m_fontPlacement);
    }
    else
    {
        float strWidth = m_buttonText != NULL ? StringWidth(m_buttonText) : 0;
        float viewWidth = bounds.Width();
        if (m_clickBitmap || m_disabledBitmap)
            MovePenTo(viewWidth / 2.0 - strWidth / 2.0 + 1, 20 + m_marginHeight + 1 + m_fontPlacement);
        else
            MovePenTo(viewWidth / 2.0 - strWidth / 2.0 + 1, m_marginHeight + 1 + m_fontPlacement);

        if (m_popUpMenu)
            MovePenBy(1, 0);
    }

    if (m_buttonText)
        DrawString(m_buttonText);

    // Darkening the pushed down button (only when borders are on)
    if (m_borders)
    {
        SetDrawingMode(B_OP_SUBTRACT);
        SetHighColor(25, 25, 25, 255);
        SetLowColor(m_backColor);
        FillRect(bounds.InsetByCopy(2, 2));

        SetDrawingMode(B_OP_COPY);
    }
}


void ImageButton::GetPreferredSize(float* width, float* height)
{
    if (m_buttonText == NULL)
    {
        if (m_clickBitmap || m_disabledBitmap)
            *height = m_marginHeight + 20 + m_marginHeight - 1;
        else
            *height = 2 * m_marginHeight + m_fontPlacement + 2;

        *width = m_marginWidth + 20 + m_marginWidth - 1;
        return;
    }

    // Calculate the height of the view, leave width as it is
    if (m_textPosition == RIGHT_OF_ICON)
    {
        if (m_clickBitmap || m_disabledBitmap)
            *height = m_marginHeight + 20 + m_marginHeight - 1;
        else
            *height = 2 * m_marginHeight + m_fontPlacement + 2;

        *width = 2 * m_marginWidth + 20 + m_marginWidth + StringWidth(m_buttonText);
        return;
    }
    else
    {
        if (m_clickBitmap || m_disabledBitmap)
            *height = 2 * m_marginHeight + 20 + (m_buttonText != NULL ? m_fontPlacement : 0) + 2;
        else
            *height = 2 * m_marginHeight + m_fontPlacement + 2;
    }

    if (m_borders == false)
        *height -= 4;

    *width = Bounds().Width();
    if (m_popUpMenu)
        *width += kContextWidth;

    if (m_buttonText)
        if (StringWidth(m_buttonText) > *width)
            *width += (StringWidth(m_buttonText) - *width) + 2 * m_marginWidth;
}


void ImageButton::AttachedToWindow()
{
    // BugFix: for the first-time flicker problem - set the parent's view color here
    // and in Draw() use B_TRANSPARENT_COLOR
    SetViewColor(Parent() ? Parent()->ViewColor() : B_TRANSPARENT_COLOR);
    ResizeToPreferred();
    BView::AttachedToWindow();
}


status_t ImageButton::SetMargin(float width, float height)
{
    // Set the margin width and height. If width or height is -1 then ignore setting them
    status_t retVal = B_OK;
    if (width != -1)
    {
        if (width > 0 && width <= 10)
            m_marginWidth = width;
        else
            retVal = B_ERROR;
    }

    if (height != -1)
    {
        if (height > 0 && height <= 10)
            m_marginHeight = height;
        else
            retVal = B_ERROR;
    }

    Invalidate();
    return retVal;
}


void ImageButton::GetMargin(float* width, float* height)
{
    // Write the current margin values to width and height
    *width = m_marginWidth;
    *height = m_marginHeight;
}


void ImageButton::SetEnabled(bool enable)
{
    // Change enabled state only when its different from the current one, this way we prevent
    // un-necessary invalidating (re-draws)
    if (enable != m_isEnabled)
    {
        m_isEnabled = enable;
        Invalidate(Bounds());
    }
}


bool ImageButton::IsEnabled() const
{
    return m_isEnabled;
}


void ImageButton::SetMessage(BMessage* msg)
{
    if (m_clickMessage)
        delete msg;

    m_clickMessage = msg;
}


BMessage* ImageButton::Message() const
{
    return m_clickMessage;
}


void ImageButton::HighlightNow(bool isPressing)
{
    if (m_isEnabled == true)
    {
        BRect rect(Bounds());
        DrawOutsideEdge(rect);
        DrawShinyEdge(rect, isPressing);
    }
}


void ImageButton::WindowActivated(bool state)
{
    // Make the first click (which is usually used to give focus to the window) be ignored
    // by our mouse handling functions by setting m_firstClick to true
    m_firstClick = !state;    // bug-fix: 0.05

    BPoint pt;
    uint32 buttons;
    GetMouse(&pt, &buttons, false);

    if (Bounds().Contains(pt) && state == true)
        m_mouseInside = true;
    else
        m_mouseInside = false;

    Invalidate();

    BView::WindowActivated(state);
}


void ImageButton::SetBorderPadding(float horizontal, float vertical)
{
    m_marginWidth = horizontal >= 0 && horizontal <= 6 ? horizontal + 1 : m_marginWidth;
    m_marginHeight = vertical >= 0 && vertical <= 6 ? vertical + 1 : m_marginHeight;
    Invalidate(Bounds());
}


void ImageButton::GetBorderPadding(float* horizontal, float* vertical)
{
    *horizontal = m_marginWidth;
    *vertical = m_marginHeight;
}


void ImageButton::SetClickable(bool clickable)
{
    m_isClickable = clickable;
}


bool ImageButton::IsClickable() const
{
    return m_isClickable;
}


void ImageButton::ShowContextMenu(BPoint point)
{
    if (m_contextMenu == NULL)
        return;

    point.x = 1;
    point.y = Bounds().Height() + 1;
    BPoint screenPt = point;

    // This ignoreClickRect means when the menu is being shown if the user clicks anywhere inside the
    // rect, the menu is not dismissed, it is very useful as we are now blocking the user from clicking
    // this button when the menu is being shown, thereby avoiding un-necessary hassles, avoiding blinking
    // nonsense and lots of other annoying stuff :)
    // Thank god Be, Inc. gave the BRect argument in Go() of BPopUpMenu class :)
    BRect ignoreClickRect(point.x, point.y - Bounds().Height() - 1, point.x + Bounds().Width(),
                          point.y + Bounds().Height());

    ConvertToScreen(&screenPt);
    ConvertToScreen(&ignoreClickRect);

    m_contextMenu->SetAsyncAutoDestruct(true);
    BMenuItem* selectedItem = m_contextMenu->Go(screenPt, true, true, ignoreClickRect, false);
    if (selectedItem && Window())
    {
        // Some times the caller might need the source (see MainWindow:M_DYNEXTRACT_ITEM it needs source)
        BMessage* msg = selectedItem->Message();
        msg->AddPointer("source", (void*)selectedItem);
        if (!m_handler)
            Window()->PostMessage(msg);
        else
        {
            BMessenger messenger(m_handler);
            messenger.SendMessage(msg);
        }
    }
    else    // else-part added :: BugFix
    {
        m_isPushed = false;
        m_drawingTriangle = false;
        // Invalidate ();
    }
}


void ImageButton::SetTarget(const BHandler* handler)
{
    m_handler = const_cast<BHandler*>(handler);
}
