/*
 * SplitPane (C)
 *
 * SplitPane is a useful UI component. It allows the
 * use to ajust two view Horizontally or Vertaclly so
 * that they are a desired size. This type of Pane
 * shows up most comonly in Mail/News Readers.
 *
 * Author  YNOP (ynop@acm.org)
 * Version beta
 * Date     Dec. 10 1999
 *
 * (note:    modified by jaf@lcsaudio.com 20/06/00)
 * (note:    modified by ramshankar@bemail.org 17/10/03)
 *
 */

#ifndef _SPLIT_PANE_VIEW_H
#define _SPLIT_PANE_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#define SPLITPANE_STATE 'spst'

// jaf: All BPoint arguments represent the value to use when in    B_VERTICAL alignment followed by
//        the value to use when in B_HORIZONTAL alignment.
// ram: Reformated/aligned this header file to suit the rest of my project.

class SplitPane : public BView
{
    public:
        SplitPane (BRect, BView*, BView*, uint32);

        // Inherited hooks
        virtual void        Draw (BRect);
        virtual void        AttachedToWindow ();
        virtual void        FrameResized (float, float);
        virtual void        MouseDown (BPoint);
        virtual void        MouseUp (BPoint);
        virtual void        MouseMoved (BPoint, uint32, const BMessage*);
        virtual void        MessageReceived (BMessage*);

        // Additional hooks
        void               AddChildOne (BView *);
        void               AddChildTwo (BView *);
        void               SetAlignment (uint);
        void               SetBarPosition(BPoint pos);
        void               SetBarThickness (BPoint);
        void               SetJump (BPoint);
        bool               HasViewOne () const;
        bool               HasViewTwo () const;
        void               SetViewOneDetachable (bool);
        void               SetViewTwoDetachable (bool);
        bool               IsViewOneDetachable () const;
        bool               IsViewTwoDetachable () const;
        void               SetEditable (bool);
        bool               IsEditable () const;
        void               SetViewInsetBy (BPoint);
        void               SetMinSizeOne (const BPoint &size);
        void               SetMinSizeTwo (const BPoint &size);
        void               SetBarLocked (bool);
        bool               IsBarLocked () const;
        void               SetBarAlignmentLocked (bool);
        bool               IsBarAlignmentLocked () const;
        void               SetState (BMessage*);
        void               SetResizeViewOne (bool whileInVertAlign, bool whileInHorizAlign);
        void               StoreBarPosition ();        // By Ram

        uint               GetAlignment () const;
        BPoint             GetBarPosition () const;
        BPoint             GetBarThickness () const;
        BPoint             GetJump () const;
        BPoint             GetViewInsetBy () const;
        BPoint             GetMinSizeOne () const;
        BPoint             GetMinSizeTwo () const;
        void               GetState (BMessage &writeTo) const;
        void               GetResizeViewOne (bool &returnWhileInVertAlign, bool &returnWhileInHorizAlign) const;
        BView*             ViewAt (int32 num);           // By Ram
        void               RestoreBarPosition ();        // By Ram

    private:
        // Private hooks
        bool               IsInDraggerBounds (BPoint pt) const;
        void               Update ();

        // Private members
        BView             *PaneOne,
                        *PaneTwo;
        BPoint             vertpos,         // added by Ram; stores bar position in vertical mode
                         horizpos,        // added by Ram; stores bar position in horizontal mode
                         storedvpos,        // The above 2 are used to store/restore while toggling splitter
                         storedhpos;        // added by Ram; used by StoreBarPosition() it saves the bar
                                        // position from vertpos, horizpos & used in RestoreBarPosition()

        uint storedalign;                  // stores the alignment to be used while restoring
        bool               Draggin,
                         attached;
        BPoint             here,
                         prevSize;
        BWindow           *WinOne,
                        *WinTwo;

        // State info members
        bool               VOneDetachable,
                         VTwoDetachable;
        uint               align;
        BPoint             pos,
                         thickness,
                         jump,
                         pad,
                         MinSizeOne,
                         MinSizeTwo;
        bool               poslocked,
                         alignlocked,
                         resizeOneX,  // added by jaf; determines which child view gets
                         resizeOneY;  // resized when SplitPane view is resized
};

#endif /* _SPLIT_PANE_VIEW_H */
