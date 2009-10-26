/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _BEEZER_LIST_VIEW_H
#define _BEEZER_LIST_VIEW_H

#include "ColumnListView.h"

class ListEntry;

class BPopUpMenu;
class BString;

class BeezerListView : public ColumnListView
{
    public:
        BeezerListView (BRect frame, CLVContainerView **containerView, const char *name = NULL,
                uint32 resizingMode = B_FOLLOW_LEFT, uint32 flags = B_WILL_DRAW,
                list_view_type type = B_SINGLE_SELECTION_LIST, bool hierarchical = false,
                bool horizontalScroll = true, bool verticalScroll = true, bool scrollViewCorner = true,
                border_style borderStyle = B_NO_BORDER, const BFont *labelFont = be_plain_font);
        virtual ~BeezerListView ();
        
        // Inherited hooks
        virtual void        MakeFocus (bool focused = true);
        virtual void        KeyDown (const char *bytes, int32 numBytes);
        virtual void        MouseDown (BPoint point);
        virtual void        MouseMoved (BPoint point, uint32 status, const BMessage *message);
        virtual void        SelectionChanged ();
        virtual void        DeselectAll ();
        virtual bool        InitiateDrag (BPoint point, int32 index, bool wasSelected);
        virtual void        MessageReceived (BMessage *message);

        // Additional hooks
        void                SelectionChanged (int32 *count, uint32 *bytes);
        void                PostSelectionMessage (int32 *count, uint32 *bytes);
        void                GetState (BMessage &msg) const;
        void                SetState (BMessage *msg);
        void                SendSelectionMessage (bool send);
        void                SelectSubItems (CLVListItem *superItem);
        void                DeselectSubItems (CLVListItem *superItem);
        void                SelectSubItemsOfSelection (bool select);
        void                SetContextMenu (BPopUpMenu *menu);
        void                InvertSelection ();
        void                SelectAll ();
        void                SelectAllEx (bool superItems);
        void                ToggleAllSuperItems (bool expand);
        void                ToggleSelectedSuperItems (bool expand);
        int32                Search (const BMessage *message, const char *&errorString);
        int32                FullListSelectionCount () const;
        int32                SelectionCount () const;
        void                CountSubItemsOf (int32 &subItems, int32 &superItems, CLVListItem *superItem);
        void                CountSelectionSmart (int32 &subItems, int32 &superItems);
        void                CountSelectionDumb (int32 &subItems, int32 &superItems);
        void                CopyToClipboard (char columnSeparator);
        bool                HasSelection () const;
        
        // Static functions
        static int            SortFunction (const CLVListItem *a, const CLVListItem *b, BList *columnList,
                                int32 sortKey);
        static int            SortAsPerName (const char *name1, const char *name2, BList *columnList,
                                int32 sortKey);
            
    private:
        // Private hooks
        void                UpdateWindow () const;
        void                EraseIndicator ();

        // Private members
        BPopUpMenu            *m_contextMenu;
        bool                m_sendSelectionMessage;
        bool                m_dropPossible;
        float                m_dropY;
        ListEntry            *m_dropItem;
        int32                m_cachedCount;
};

#endif /* _BEEZER_LIST_VIEW_H */
