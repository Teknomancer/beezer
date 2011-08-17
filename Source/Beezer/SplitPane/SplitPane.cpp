/*
 * SplitPane (C)
 *
 * SplitPane is a useful UI component. It allows the
 * use to ajust two view Horizontally or Vertically so
 * that they are a desired size. This type of Pane
 * shows up most comonly in Mail/News Readers.
 *
 * Author  YNOP (ynop@acm.org)
 * Version beta
 * Date    Dec. 10 1999
 *
 * (note:    modified by jaf@lcsaudio.com 6/20/00)
 * (note:    modified by ramshankar@bemail.org 4/07/03)
 *
 */

#include <AppKit.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <String.h>
#include <Path.h>
#include <TranslationKit.h>
#include <TranslationUtils.h>

#include <stdio.h>

#include "SplitPane.h"

//=============================================================================================================//

SplitPane::SplitPane (BRect frame, BView *one, BView *two,uint32 Mode)
    : BView (frame, "", Mode, B_WILL_DRAW | B_FRAME_EVENTS)
{
    //SetViewColor(216,216,216);  // This is default get from parent if exist
    //SetViewColor(B_TRANSPARENT_32_BIT); // go tran so we have control over drawing
    BRect b;
    b = Bounds();

    PaneOne = one;
    PaneTwo = two;

    prevSize.Set (0.0f,0.0f);                       // used for resizing when resizeOne(X|Y) is true --jaf
    resizeOneX = resizeOneY = false;
    align = B_VERTICAL;                             // Most people use it this way
    pos.Set (b.Width() / 2.0f, b.Height() / 2.0f);    // Center is a good start place
    thickness.Set (10.0f, 10.0f);
    jump.Set (1.0f, 1.0f);                         // 1 makes a smother slide
    VOneDetachable = false;
    VTwoDetachable = false;
    pad.Set (3.0f, 3.0f);
    MinSizeOne.Set (0, 0);
    MinSizeTwo.Set (0, 0);
    poslocked = false;                             // free movement
    alignlocked = false;                         // free alignment
    Draggin = false;
    attached = false;

    vertpos.Set (-1, -1);                         // nothing is stored yet
    horizpos.Set (-1, -1);                         // nothing is stored yet

    WinOne = NULL;
    WinTwo = NULL;
    //   ConfigWindow = NULL;

    AddChild (one);
    AddChild (two);
}

//=============================================================================================================//

void SplitPane::AttachedToWindow()
{
   BView::AttachedToWindow ();
   //SetViewColor(Parent()->ViewColor());
   attached = true;
   Update();
}

//=============================================================================================================//

void SplitPane::FrameResized (float w, float h)
{
//   if bar is on the left side follow left
//   else if it is on the right side follow the right
//   Need to implements smart follow still

    BView::FrameResized(w,h);
    Update();
    Invalidate();
}

//=============================================================================================================//

void SplitPane::Draw (BRect updateRect)
{
    SetHighColor(160,160,160);

    if (align == B_VERTICAL)
    {
        SetHighColor(145,145,145);
        FillRect(BRect(pos.x,Bounds().top+pad.x+1,pos.x,Bounds().bottom-pad.x-1));

        SetHighColor(255,255,255);
        FillRect(BRect(pos.x+1,Bounds().top+pad.x+1,pos.x+2,Bounds().bottom-pad.x-1));

        SetHighColor(216,216,216);
        FillRect(BRect(pos.x+2,Bounds().top+pad.x+1,pos.x+thickness.x-2,Bounds().bottom-pad.x-1));

        SetHighColor(145,145,145);
        FillRect(BRect(pos.x+thickness.x-2,Bounds().top+pad.x+1,pos.x+thickness.x-2,Bounds().bottom-pad.x-1));

        SetHighColor(96,96,96);
        FillRect(BRect(pos.x+thickness.x-1,Bounds().top+pad.x+1,pos.x+thickness.x-1,Bounds().bottom-pad.x-1));

        // Ram -- draw a gripper
        rgb_color lite = { 245, 245, 245, 255}, dark = { 118, 118, 118, 255 };
        float x = (pos.x + pos.x + thickness.x) / 2.0 - 1;
        for (float i = 3; i < Bounds().bottom - pad.x - 1 - 2; i += 3)
        {
           SetHighColor (dark);
           StrokeLine (BPoint (x,i), BPoint (x,i));
           SetHighColor (lite);
           StrokeLine (BPoint (x+1, i+1), BPoint (x+1,i+1));
        }
    }
    else
    {
        SetHighColor(145,145,145);
        FillRect(BRect(Bounds().left+pad.y+1,pos.y,Bounds().right-pad.y-1,pos.y));

        SetHighColor(255,255,255);
        FillRect(BRect(Bounds().left+pad.y+1,pos.y+1,Bounds().right-pad.y-1,pos.y+2));

        SetHighColor(Parent()->ViewColor());
        FillRect(BRect(Bounds().left+pad.y+1,pos.y+2,Bounds().right-pad.y-1,pos.y+thickness.y-2));

        SetHighColor(145,145,145);
        FillRect(BRect(Bounds().left+pad.y+1,pos.y+thickness.y-2,Bounds().right-pad.y-1,pos.y+thickness.y-2));

        SetHighColor(96,96,96);
        FillRect(BRect(Bounds().left+pad.y+1,pos.y+thickness.y-1,Bounds().right-pad.y-1,pos.y+thickness.y-1)); // 96

        // Ram -- draw a gripper
        rgb_color lite = { 245, 245, 245, 255}, dark = { 118, 118, 118, 255 };
        float y = (pos.y + pos.y + thickness.y) / 2.0 - 1;
        for (float i = 3; i < Bounds().right - pad.y - 1 - 2; i += 3)
        {
           SetHighColor (dark);
           StrokeLine (BPoint (i,y), BPoint (i,y));
           SetHighColor (lite);
           StrokeLine (BPoint (i+1, y+1), BPoint (i+1,y+1));
        }
    }
}

//=============================================================================================================//

void SplitPane::Update ()
{
    Window()->Lock();

    if (align == B_VERTICAL)
    {
        if ((resizeOneX) && (prevSize.x > 0))
           pos.x += Bounds().Width()-prevSize.x;

        PaneOne->SetResizingMode ((resizeOneX ? B_FOLLOW_LEFT_RIGHT : B_FOLLOW_LEFT) | B_FOLLOW_TOP_BOTTOM);
        PaneTwo->SetResizingMode ((resizeOneX ? B_FOLLOW_RIGHT : B_FOLLOW_LEFT_RIGHT) | B_FOLLOW_TOP_BOTTOM);

        if (pos.x > (Bounds().Width() - thickness.x - MinSizeTwo.x))
           if(!poslocked)
               pos.x = Bounds().Width() - thickness.x - MinSizeTwo.x;

        if(pos.x < MinSizeOne.x)
           if(!poslocked)
               pos.x = MinSizeOne.x;

        vertpos = pos;
    }
    else
    {
        if ((resizeOneY) && (prevSize.y > 0))
           pos.y += Bounds().Height() - prevSize.y;

        PaneOne->SetResizingMode ((resizeOneY ? B_FOLLOW_TOP_BOTTOM : B_FOLLOW_TOP) | B_FOLLOW_LEFT_RIGHT);
        PaneTwo->SetResizingMode ((resizeOneY ? B_FOLLOW_BOTTOM : B_FOLLOW_TOP_BOTTOM) | B_FOLLOW_LEFT_RIGHT);

        if (pos.y > (Bounds().Height() - thickness.y - MinSizeTwo.y))
           if(!poslocked)
               pos.y = Bounds().Height() - thickness.y - MinSizeTwo.y;

        if (pos.y < MinSizeOne.y)
           if(!poslocked)
               pos.y = MinSizeOne.y;

        horizpos = pos;
    }

    // store our new size so we can do a diff next time we're called
    prevSize = BPoint (Bounds().Width(), Bounds().Height());

    if (PaneOne)
    {
        if (!WinOne)
        {
           if(align == B_VERTICAL)
           {
               PaneOne->MoveTo (pad.x, Bounds().top + pad.x);
               PaneOne->ResizeTo (pos.x - pad.x, Bounds().Height() - pad.x - pad.x); // width x height
           }
           else
           {
               PaneOne->MoveTo (pad.y,Bounds().top + pad.y);
               PaneOne->ResizeTo (Bounds().Width() - pad.y - pad.y, pos.y - pad.y - pad.y); // width x height
           }
        }
    }

    if (PaneTwo)
    {
        if (!WinTwo)
        {
           if (align == B_VERTICAL)
           {
               PaneTwo->MoveTo (pos.x+thickness.x,Bounds().top+pad.x);
               PaneTwo->ResizeTo (Bounds().Width() - (pos.x + thickness.x) - pad.x,
                             Bounds().Height() - pad.x - pad.x);
           }
           else
           {
               PaneTwo->MoveTo (Bounds().left+pad.y,pos.y+thickness.y);
               PaneTwo->ResizeTo (Bounds().Width() - pad.y - pad.y,
                         Bounds().Height() - pos.y - pad.y - thickness.y);
           }
        }
    }

    Window()->Unlock();
}

//=============================================================================================================//

void SplitPane::MouseDown (BPoint where)
{
    Window()->Lock();
    BMessage *currentMsg = Window()->CurrentMessage();

    if (currentMsg->what == B_MOUSE_DOWN)
    {
        uint32 buttons (0), modifiers (0), clicks (0);
        currentMsg->FindInt32("buttons", (int32*)&buttons);
        currentMsg->FindInt32("modifiers", (int32*)&modifiers);
        currentMsg->FindInt32("clicks", (int32*)&clicks);

        if (buttons & B_SECONDARY_MOUSE_BUTTON)
        {
           if (!alignlocked)
           {
               switch (align)
               {
                  case B_VERTICAL: align = B_HORIZONTAL; break;
                  case B_HORIZONTAL: align = B_VERTICAL; break;
               }

               Update();
               Invalidate();

               // ONLY FOR THIS APP WE NEED THE BELOW LINE -- BUG FIX -- Don't Implement for other apps
               // Unless needed -- Ram (Custom bug fix/workaround for project)
               Window()->FrameResized (Window()->Bounds().Width(), Window()->Bounds().Height());
           }
        }

        if ((buttons & B_PRIMARY_MOUSE_BUTTON) && (!Draggin) && (IsInDraggerBounds(where)))
        {
           if (!poslocked)
           {
               Draggin = true; // this is so we can drag
               here = where;
           }

           SetMouseEventMask (B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS);
        }
    }

    Window()->Unlock();
}

//=============================================================================================================//

bool SplitPane::IsInDraggerBounds (BPoint pt) const
{
    // this block should go in FrameResized .. think about it
    return (align == B_VERTICAL)
        ? ((pt.x > PaneOne->Frame().right) && (pt.x < PaneTwo->Frame().left))
        : ((pt.y > PaneOne->Frame().bottom) && (pt.y < PaneTwo->Frame().top));
}

//=============================================================================================================//

void SplitPane::MouseUp (BPoint point)
{
    Draggin = false; // stop following mouse
}

//=============================================================================================================//

void SplitPane::MouseMoved (BPoint where, uint32 info, const BMessage *message)
{
    if (Draggin)
    {
        float minVal;
        switch (align)
        {
           case B_HORIZONTAL:
               pos.y = (where.y)-(thickness.y/2);
               minVal = MinSizeOne.y;
               break;

           case B_VERTICAL:
               pos.x = (where.x)-(thickness.x/2);
               minVal = MinSizeOne.x;
               break;
        }

        switch (align)
        {
           case B_HORIZONTAL:
               if (pos.y < MinSizeOne.y)
                  pos.y = MinSizeOne.y;
               break;

           case B_VERTICAL:
               if (pos.x < MinSizeOne.x)
                  pos.x = MinSizeOne.x;
               break;
        }

        if (align == B_VERTICAL)
        {
           if (pos.x > (Bounds().Width() - thickness.x - MinSizeTwo.x))
               pos.x = (Bounds().Width() - thickness.x - MinSizeTwo.x + 1);

           vertpos = pos;
        }
        else
        {
           if (pos.y > (Bounds().Height() - thickness.y - MinSizeTwo.y))
               pos.y = (Bounds().Height() - thickness.y - MinSizeTwo.y + 1);

           horizpos = pos;
        }

        Update();
        Invalidate();
    }
}

//=============================================================================================================//

void SplitPane::AddChildOne (BView *v)
{
    RemoveChild (PaneOne);
    PaneOne = v;
    AddChild (PaneOne);
}

//=============================================================================================================//

void SplitPane::AddChildTwo (BView *v)
{
    RemoveChild (PaneTwo);
    PaneTwo = v;
    AddChild (PaneTwo);
}

//=============================================================================================================//

void SplitPane::SetAlignment (uint a)
{
   align = a;
   if(attached)
        Update();

   Invalidate();
}

//=============================================================================================================//

uint SplitPane::GetAlignment () const
{
    return align;
}

//=============================================================================================================//

void SplitPane::SetBarPosition (BPoint p)
{
    pos = p;
    if (attached)
        Update();

    Invalidate();
}

//=============================================================================================================//

BPoint SplitPane::GetBarPosition () const
{
    return pos;
}

//=============================================================================================================//

void SplitPane::SetBarThickness (BPoint t)
{
    thickness = t;
    if (attached)
        Update();

    Invalidate();
}

//=============================================================================================================//

BPoint SplitPane::GetBarThickness () const
{
    return thickness;
}

//=============================================================================================================//

void SplitPane::SetJump (BPoint j)
{
    jump = j;
    if (attached)
        Update();
}

//=============================================================================================================//

BPoint SplitPane::GetJump () const
{
    return jump;
}

//=============================================================================================================//

bool SplitPane::HasViewOne () const
{
    if (PaneOne)
        return true;

    return false;
}

//=============================================================================================================//

bool SplitPane::HasViewTwo () const
{
    if (PaneTwo)
        return true;

    return false;
}

//=============================================================================================================//

void SplitPane::SetViewOneDetachable (bool b)
{
    VOneDetachable = b;
}

//=============================================================================================================//

void SplitPane::SetViewTwoDetachable (bool b)
{
    VTwoDetachable = b;
}

//=============================================================================================================//

bool SplitPane::IsViewOneDetachable () const
{
    return VOneDetachable;
}

//=============================================================================================================//

bool SplitPane::IsViewTwoDetachable () const
{
    return VTwoDetachable;
}

//=============================================================================================================//

void SplitPane::SetEditable (bool /*b*/)
{
   //ADD CODE HERE YNOP
}

//=============================================================================================================//

bool SplitPane::IsEditable () const
{
    return true; //ADD SOME MORE CODE HERE
}

//=============================================================================================================//

void SplitPane::SetViewInsetBy (BPoint p)
{
    pad = p;
    if (attached)
        Update();

    Invalidate();
}

//=============================================================================================================//

BPoint SplitPane::GetViewInsetBy () const
{
    return pad;
}

//=============================================================================================================//

void SplitPane::SetMinSizeOne (const BPoint & p)
{
    MinSizeOne = p;
}

//=============================================================================================================//

BPoint SplitPane::GetMinSizeOne () const
{
    return MinSizeOne;
}

//=============================================================================================================//

void SplitPane::SetMinSizeTwo (const BPoint & p)
{
    MinSizeTwo = p;
}

//=============================================================================================================//

BPoint SplitPane::GetMinSizeTwo () const
{
    return MinSizeTwo;
}

//=============================================================================================================//

void SplitPane::SetBarLocked (bool b)
{
    poslocked = b;
}

//=============================================================================================================//

bool SplitPane::IsBarLocked () const
{
    return poslocked;
}

//=============================================================================================================//

void SplitPane::SetBarAlignmentLocked (bool b)
{
    alignlocked = b;
}

//=============================================================================================================//

bool SplitPane::IsBarAlignmentLocked () const
{
    return alignlocked;
}

//=============================================================================================================//

void SplitPane::SetResizeViewOne (bool x, bool y)
{
    resizeOneX = x;
    resizeOneY = y;
    if (attached)
       Update();

    Invalidate();
}

//=============================================================================================================//

void SplitPane::GetResizeViewOne (bool & rx, bool & ry) const
{
    rx = resizeOneX;
    ry = resizeOneY;
}

//=============================================================================================================//

void SplitPane::GetState (BMessage& state) const
{
    // Changed names of fields (shortened) so that it results in smaller flattened files
    // Ram
    state.AddBool ("1d", VOneDetachable);
    state.AddBool ("2d", VTwoDetachable);
    state.AddInt32 ("an", align);
    state.AddPoint ("ps", pos);
    state.AddPoint ("th", thickness);
    state.AddPoint ("jm", jump);
    state.AddPoint ("pd", pad);
    state.AddPoint ("m1", MinSizeOne);
    state.AddPoint ("m2", MinSizeTwo);
    state.AddBool ("pl", poslocked);
    state.AddBool ("al", alignlocked);
    state.AddBool ("rx", resizeOneX);
    state.AddBool ("ry", resizeOneY);
}

//=============================================================================================================//

void SplitPane::SetState (BMessage *state)
{
    BPoint pt;
    int32 i;

    if (state->FindBool ("1d", &VOneDetachable) != B_NO_ERROR)
        VOneDetachable = false;

    if (state->FindBool ("2d", &VTwoDetachable) != B_NO_ERROR)
        VTwoDetachable = false;

    if (state->FindInt32 ("an",&i) == B_NO_ERROR)
        align = i;

    if (state->FindPoint ("ps",&pt) == B_NO_ERROR)
        pos = pt;

    if (state->FindPoint ("th",&pt) == B_NO_ERROR)
        thickness = pt;

    if (state->FindPoint ("jm", &pt) == B_NO_ERROR)
        jump = pt;

    if (state->FindPoint ("pd",&pt) == B_NO_ERROR)
        pad = pt;

    if (state->FindPoint ("m1", &pt) == B_NO_ERROR)
        MinSizeOne = pt;

    if (state->FindPoint ("m2", &pt) == B_NO_ERROR)
        MinSizeTwo = pt;

    if (state->FindBool ("pl", &poslocked) != B_NO_ERROR)
        poslocked = false;

    if (state->FindBool ("al", &alignlocked) != B_NO_ERROR)
        alignlocked = false;

    if (state->FindBool ("rx", &resizeOneX) != B_NO_ERROR)
        resizeOneX = false;

    if (state->FindBool ("ry", &resizeOneY) != B_NO_ERROR)
        resizeOneY = false;

    // We MIGHT be calling SetState() even before attached is set,
    // that is why we have disabled it
    //if (attached)
    //{
        SetBarPosition (pos);
        Update();
        Invalidate();
    //}
}

//=============================================================================================================//

void SplitPane::MessageReceived (BMessage *msg)
{
    switch (msg->what)
    {
        case SPLITPANE_STATE:
           SetState (msg);
           break;

        default:
           BView::MessageReceived (msg);
           break;
    }
}

//=============================================================================================================//

BView* SplitPane::ViewAt (int32 num)
{
    // Added by Ram -- returns the views 'owned' by SplitPane
    if (num == 1L)
        return PaneOne;
    else if (num == 2L)
        return PaneTwo;

    return NULL;
}

//=============================================================================================================//

void SplitPane::RestoreBarPosition ()
{
    if (storedvpos.x != -1)
    {
        if (storedalign == B_VERTICAL)
           pos = storedvpos;
        else
           pos = storedhpos;

        if (attached)
           Update();

        Invalidate();
    }
}

//=============================================================================================================//

void SplitPane::StoreBarPosition()
{
    storedvpos = vertpos;
    storedhpos = horizpos;
    storedalign = align;
}

//=============================================================================================================//
