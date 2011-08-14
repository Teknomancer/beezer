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

#include <Window.h>
#include <ClassInfo.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Clipboard.h>
#include <Path.h>
#include <Entry.h>

#include <stdlib.h>

#include "CLVEasyItem.h"

#include "BeezerListView.h"
#include "MsgConstants.h"
#include "RegExString.h"
#include "AppUtils.h"
#include "LocalUtils.h"
#include "ListEntry.h"
#include "BitmapPool.h"
#include "LangStrings.h"
#include "AppConstants.h"

const char *const kPfSpecialField = "beezer_special_field_magix!";

//=============================================================================================================//

BeezerListView::BeezerListView (BRect frame, CLVContainerView **containerView, const char *name,
                  uint32 resizingMode, uint32 flags,
                  list_view_type type, bool hierarchical,
                  bool horizontalScroll, bool verticalScroll, bool scrollViewCorner,
                  border_style borderStyle, const BFont *labelFont)
    : ColumnListView (frame, containerView, name, resizingMode, flags, type, hierarchical, horizontalScroll,
                  verticalScroll, scrollViewCorner, borderStyle, labelFont)
{
    m_contextMenu = NULL;
    m_cachedCount = -1L;
    m_sendSelectionMessage = true;

    // The following are related to drag-drop from outside sources (like Tracker)
    // used mainly in MouseMoved() event
    m_dropPossible = false;
    m_dropY = -1;
    m_dropItem = NULL;
}

//=============================================================================================================//

BeezerListView::~BeezerListView ()
{
    // Delete all list items here as CLV doesn't do it for us in its destructor
    int32 itemCount = FullListCountItems();
    for (int32 i = 0; i < itemCount; i++)
        delete (ListEntry*)ColumnListView::RemoveItem(0L);
}

//=============================================================================================================//

void BeezerListView::MessageReceived (BMessage *message)
{
    switch (message->what)
    {
        case B_COPY_TARGET:
        {
           BMessage dropDoneMessage (*message);
           dropDoneMessage.what = M_DROP_MESSAGE;
           
           // Remove the temporary file (dropped object) created by Tracker
           entry_ref dirRef;
           message->FindRef ("directory", &dirRef);
           BPath path (&dirRef);
           path.Append (kDropClipping);

           BEntry entry (path.Path(), false);
           entry.Remove();
           
           // MainWindow must handle the message
           Window()->PostMessage (&dropDoneMessage);
           break;
        }
        
        case B_SIMPLE_DATA:
        {
           // We are getting files dropped on our listview FROM tracker
           // Make sure it has "refs" field and is NOT the beezer special field
           if (message->HasRef ("refs") && message->HasBool (kPfSpecialField) == false)
           {
               // Grab the directory ref from the ref in the message
               entry_ref dirRef;
               entry_ref ref;
               message->FindRef ("refs", 0, &ref);
               BEntry entry (&ref, false);        // Don't traverse links here
               entry.GetParent (&entry);
               entry.GetRef (&dirRef);
               
               BMessage *dropMsg = new BMessage (*message);
               dropMsg->what = M_ADD;
               if (m_dropItem == NULL)
                  dropMsg->AddBool (kRoot, true);
               else
                  dropMsg->AddPointer (kListItem, m_dropItem);
               
               dropMsg->AddRef (kDirectoryRef, &dirRef);
               EraseIndicator ();
               Window()->PostMessage (dropMsg);
               delete dropMsg;
           }
           break;
        }
        
        case B_MODIFIERS_CHANGED:
        {
           // New from 0.05 -- now pressing SHIFT while adding files without moving the mouse
           // correctly updates the DropLine colour!
           if (m_dropPossible)
           {
               BPoint pt;
               uint32 buttons;
               GetMouse (&pt, &buttons, true);
               MouseMoved (pt, B_INSIDE_VIEW, message);
           }
           break;
        }
        
        default:
           ColumnListView::MessageReceived (message);
           break;
    }
}

//=============================================================================================================//

void BeezerListView::KeyDown (const char *bytes, int32 numBytes)
{
    if ((modifiers() & B_SHIFT_KEY) == 0)           // When shift key is not held down
    {
        // Remove selection when user presses left/right without SHIFT when all items are selected
        if (bytes[0] == B_ESCAPE || bytes[0] == B_LEFT_ARROW || bytes[0] == B_RIGHT_ARROW)
        {
           int32 i = 0;
           while (FullListCurrentSelection (i) >= 0)
               i++;
           
           if ((i == FullListCountItems() || i == CountItems()) && i > 1)
           {
               DeselectAll();
               return ColumnListView::KeyDown(bytes, numBytes);
           }
        }
        else
        {
           // Don't mind the repeat of the above code as this is for performance, by duplicating
           // (almost if (i>1) is extra) we avoid recounting items when user presses up/down which
           // will be the MOST frequently used keys we count only on "rarer" keys like ESCAPE,LEFT,RIGHT
           int32 i = 0;
           while (FullListCurrentSelection (i) >= 0)
           {
               i++;
               if (i > 1)
                  break;
           }
           
           if (i > 1)
           {
               // User is allowed to move up/down when everything is selected - left/right deselects though
                BScrollBar *vScrollBar = ScrollBar (B_VERTICAL);
               float smallStep, largeStep;
               vScrollBar->GetSteps (&smallStep, &largeStep);
               
               switch (bytes[0])
               {
                  case B_DOWN_ARROW: vScrollBar->SetValue (vScrollBar->Value() + smallStep); break;
                  case B_UP_ARROW: vScrollBar->SetValue (vScrollBar->Value() - smallStep); break;
                  case B_PAGE_DOWN: vScrollBar->SetValue (vScrollBar->Value() + largeStep); break;
                  case B_PAGE_UP: vScrollBar->SetValue (vScrollBar->Value() - largeStep); break;
               }
           }
        }
    }
    
    if (numBytes != 1)
        return ColumnListView::KeyDown (bytes, numBytes);
        
    // Expand/Collapse items using arrows and enter
    switch (bytes[0])
    {
        case B_RIGHT_ARROW: case B_LEFT_ARROW: case B_ENTER:
        {
           // Calculate selection again - speed boost as we calculate (count) selection only
           // when user presses LEFT/RIGHT ARROW/ENTER not up/down which will be used all the time
           int32 i = 0;
           while (FullListCurrentSelection (i) >= 0)    // No need to count fully here as we only check
           {                                        // if more than one selection is present
               i++;
               if (i > 1)
                  break;
           }

           // Don't collapse folders when multi-selection is there and user pressed LEFT arrow
           if (i > 1 && (bytes[0] == B_LEFT_ARROW || bytes[0] == B_RIGHT_ARROW))
               break;
           
           // When shift is NOT held down, pretty confusing?    
           if ((modifiers() & B_SHIFT_KEY) == 0)        // removed i == 1 && 
           {
               CLVListItem *item = FullListItemAt (FullListCurrentSelection (0L));
               if (!item)
                  return;
               
               // If superitem expand using right arrow or enter and collapse using left arrow or enter
               if (item->IsSuperItem() == true)
               {
                  if (item->IsExpanded() == true && (bytes[0] == B_LEFT_ARROW || bytes[0] == B_ENTER))
                      Collapse (item);
                  else if (item->IsExpanded() == false && (bytes[0] == B_RIGHT_ARROW || bytes[0] == B_ENTER))
                      Expand (item);
               }
               else
               {
                  // If normal item collapse its superitem (if any) using left arrow
                  if (bytes[0] == B_LEFT_ARROW)
                  {
                      CLVListItem *superItem = Superitem (item);
                      if (superItem)
                      {
                         Collapse (superItem);
                         Select (IndexOf (superItem));
                         ScrollToSelection();
                      }
                  }
               }
               
               if (bytes[0] == B_ENTER)
               {
                  BMessage msg (M_ENTER);
                  msg.AddBool (kSuperItem, item->IsSuperItem());
                  Window()->PostMessage (&msg);
               }
           }
           break;
        }
        
        case B_DOWN_ARROW: case B_UP_ARROW:
        {
           // When user expands selection (adds new items to the selection) using keyboard report only
           // added item to the selection to the MainWindow so that it can update its display
           if ((modifiers() & B_SHIFT_KEY) != 0)    // When shift is held down
           {
               int32 count = m_cachedCount != -1 ? m_cachedCount : SelectionCount();
               int32 index = CurrentSelection (bytes[0] == B_DOWN_ARROW ? count - 1 : 0L);
               CLVListItem *item = (CLVListItem*)ItemAt (bytes[0] == B_DOWN_ARROW ? index + 1 : index - 1);
               
               // If final item make sure we recount it
               if (item && item->IsSelected() == false)
               {
                  BMessage msg (M_SELECTION_ADDED);
                  msg.AddPointer (kListItem, item);
                  m_sendSelectionMessage = false;
                  m_cachedCount++;
                  Window()->PostMessage (&msg);
               }
           }
           else
               m_sendSelectionMessage = true;
           
           break;
        }
        
        case B_DELETE:
        {
           if (HasSelection())
               Window()->PostMessage (M_ACTIONS_DELETE);
           
           break;
        }
    }

    ColumnListView::KeyDown (bytes, numBytes);
}

//=============================================================================================================//

void BeezerListView::MouseMoved (BPoint point, uint32 status, const BMessage *message)
{
    BPoint pt;
    uint32 buttons;
    GetMouse (&pt, &buttons, true);

    switch (status)
    {
        case B_ENTERED_VIEW:
        {
           // Means a drop is about to take place (user is dragging something from outside)
           if (message && buttons != 0)
               m_dropPossible = true;
           else
               m_dropPossible = false;
           
           break;
        }
        
        case B_EXITED_VIEW:
        {
           if (m_dropPossible == true)
           {
               EraseIndicator();
               m_dropY = -1;
           }
           
           m_dropPossible = false;
           break;
        }
        
        case B_INSIDE_VIEW:
        {
           // Erase previous indicator if any
           if (m_dropY != -1)
           {
               EraseIndicator ();
               m_dropY = -1;    // Very important we reset this to prevent further erasals
           }
           
           if (m_dropPossible && message && buttons != 0)
           {
               if (message->HasBool (kPfSpecialField))
                  break;
               
               // Draw the indicator (a vertical line)
               int32 index = IndexOf (point);
               if (index < 0)
                  index = CountItems() - 1;
               
               BRect itemFrame = ItemFrame (index);
               BRect firstFrame = ItemFrame (0);
               if (point.y > itemFrame.bottom)
                  m_dropY = itemFrame.bottom;
               else
               {
                  m_dropY = itemFrame.top;
                  if (index > 0)
                      index--;
               }
               
               bool addAtRoot = false;
               if ((modifiers() & B_SHIFT_KEY) != 0 || ((m_dropY == firstFrame.top) && (index == 0)))
                  addAtRoot = true;
               
               if (!addAtRoot)
                  SetHighColor (0, 0, 200, 255);
               else
                  SetHighColor (200, 0, 0, 255);

               SetPenSize(2);
               StrokeLine (BPoint (0, m_dropY), BPoint (Bounds().right, m_dropY));
               
               if (addAtRoot)
                  m_dropItem = NULL;
               else
                  m_dropItem = (ListEntry*)ItemAt (index);
           }

           break;
        }
    }

    return ColumnListView::MouseMoved (point, status, message);
}

//=============================================================================================================//

void BeezerListView::EraseIndicator ()
{
    rgb_color eraseColor = ViewColor();
    if (m_dropItem)
        if (m_dropItem->IsSelected())
           eraseColor = ItemSelectColor (true);
           
    SetHighColor (eraseColor);
    SetPenSize(2);
    StrokeLine (BPoint (0, m_dropY), BPoint (Bounds().right, m_dropY));
}

//=============================================================================================================//

void BeezerListView::MouseDown (BPoint point)
{
    MakeFocus (true);
    m_sendSelectionMessage = true;    // KeyDown() might have turned this off, we turn it back ON
    
    // Check for double clicks
    BMessage *msg = Window()->CurrentMessage();
    int32 clicks = msg->FindInt32 ("clicks");
    int32 button = msg->FindInt32 ("buttons");
    
    static int32 previousButton = button;
    static int32 clickCount = 0;
    static BPoint previousPoint = point;
    
    if (previousButton == button && clicks > 1 && button == B_PRIMARY_MOUSE_BUTTON)
        clickCount++;
    else
        clickCount = 0L;
    
    previousButton = button;

    if (clickCount == 1L && (point.y >= previousPoint.y - 3 && point.y <= previousPoint.y + 3)
        && FullListCurrentSelection() == FullListIndexOf (point))        // Make sure mouse is over selection
    {
        clickCount = 0;
        BMessage msg2 (M_ENTER);
        
        CLVListItem *item = FullListItemAt (FullListIndexOf(point));
        if (!item)        // In case we have no items at all or something like that -- Bug fix
           return;
        
        // If a superitem is dbl-clicked, select all its sub-items (visible subitems)
        if (item->IsSuperItem())
        {
           if (item->IsExpanded() == true)
           {
               msg2.AddBool (kSuperItem, true);
               SelectSubItems (item);
           }
           else
           {
               Expand (item);
               return;
           }
        }

        Window()->PostMessage (&msg2);
        return;
    }
    
    previousPoint = point;

    // Detect right click for context menu popup
    if (button == B_SECONDARY_MOUSE_BUTTON && m_contextMenu != NULL)
    {
        CLVListItem *clickedItem = FullListItemAt (FullListIndexOf(point));
        if (!clickedItem)
           return;
                      
        if (clickedItem->IsSelected() == false)
           Select (IndexOf (clickedItem), false);
        
        BPoint screenPt = point;
        BRect openRect (point.x - 2, point.y - 2, point.x + 2, point.y + 2);
        
        ConvertToScreen (&screenPt);
        ConvertToScreen (&openRect);
        
        // If item is clicked disable "View" and "Open With" in case only directory items are in 
        // selection, even if one file exist in the selection enable it
        bool enabler = false;
        CLVEasyItem *selItem = NULL;
        register int32 i = 0L;
    
        while ((selItem = (CLVEasyItem*)FullListItemAt (FullListCurrentSelection (i++))) != NULL)
        {
           if (selItem->IsSuperItem() == false)
           {
               enabler = true;
               break;
           }
        }
        
        BMenuItem *viewItem = m_contextMenu->FindItem (M_ACTIONS_VIEW);
        BMenuItem *openWithItem = m_contextMenu->FindItem (M_ACTIONS_OPEN_WITH);

        if (viewItem)
           viewItem->SetEnabled (enabler);
           
        if (openWithItem)
           openWithItem->SetEnabled (enabler);

        m_contextMenu->SetAsyncAutoDestruct (true);
        BMenuItem *selectedItem = m_contextMenu->Go (screenPt, true, true, openRect, false);
        if (selectedItem && Window())
           Window()->PostMessage (selectedItem->Message());
    }
    
    ColumnListView::MouseDown (point);
}

//=============================================================================================================//

void BeezerListView::PostSelectionMessage (int32 *count, uint32 *bytes)
{
    BMessage msg (M_SELECTION_CHANGED);
    msg.AddInt32 (kCount, *count);
    msg.AddInt64 (kBytes, (int64)(*bytes));
    Window()->PostMessage (&msg);
}

//=============================================================================================================//

void BeezerListView::SelectionChanged (int32 *count, uint32 *bytes)
{
    // An extra SelectionChanged that doesn't recount bytes - for performance
    // We leave the OLD SelectionChanged() as IT IS (it will recount bytes whenever called)
    // as it is a base class member (in BListView I think)
    if (m_sendSelectionMessage)
    {
        PostSelectionMessage (count, bytes);
        m_cachedCount = *count;
    }
}

//=============================================================================================================//

void BeezerListView::SelectionChanged ()
{
    // Notify our parent window about changes with the selection
    if (m_sendSelectionMessage)
    {
        ListEntry *item = NULL;
        int32 i (0);
        uint32 val (0);
        while ((item = (ListEntry*)FullListItemAt(FullListCurrentSelection(i++))) != NULL)
           val += item->m_length;

        int32 temp = i - 1;
        PostSelectionMessage (&temp, &val);
        
        // For speed cache the number of selected items
        // Also send the kCount and kBytes so that MainWindow's UpdateInfoBar()
        // need not calculate it again
        m_cachedCount = temp;
    }           
    ColumnListView::SelectionChanged ();
}

//=============================================================================================================//

void BeezerListView::SelectSubItems (CLVListItem *superItem)
{
    // Turn off sending of selection change message (for large folders if we keep sending selection
    //    change message it takes too long...)
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;
    
    uint32 parentLevel = superItem->OutlineLevel();
    int32 itemPos = FullListIndexOf (superItem);
    int32 i = 0L;
    bool selected = false;
    if (itemPos >= 0)
    {
        for (i = itemPos + 1; i >= 1; i++)
        {
           CLVListItem *subItem = (CLVListItem*)FullListItemAt (i);
           if (subItem == NULL || subItem->OutlineLevel() <= parentLevel)
               break;
           
           if (subItem->IsSelected() == false)
           {
               selected = true;
               Select (IndexOf (subItem), true);
               UpdateWindow();
           }
        }
    }
    
    m_sendSelectionMessage = t;

    // Optimize sending of messages (faster), send only if the selection was actually CHANGED where
    // selected will be set to true
    if (selected == true)
        SelectionChanged();
}

//=============================================================================================================//

void BeezerListView::DeselectSubItems (CLVListItem *superItem)
{
    // Turn off sending of selection change message (for large folders if we keep sending selection
    //    change message it takes too long...)
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;
    
    uint32 parentLevel = superItem->OutlineLevel();
    int32 itemPos = FullListIndexOf (superItem);
    int32 i = 0L;
    if (itemPos >= 0)
    {
        for (i = itemPos + 1; i >= 1; i++)
        {
           CLVListItem *subItem = (CLVListItem*)FullListItemAt (i);
           if (subItem == NULL || subItem->OutlineLevel() <= parentLevel)
               break;

           Deselect (IndexOf (subItem));
           UpdateWindow();
        }
    }

    m_sendSelectionMessage = t;
    SelectionChanged();
}

//=============================================================================================================//

void BeezerListView::SetContextMenu (BPopUpMenu *menu)
{
    m_contextMenu = menu;
}

//=============================================================================================================//

void BeezerListView::InvertSelection ()
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;
    
    int32 count = CountItems();
    int32 selCount = 0L;
    uint32 bytes = 0L;
    for (int32 i = 0; i < count; i++)
    {
        if (ItemAt(i)->IsSelected())
           Deselect (i);
        else
        {
           ListEntry *item = reinterpret_cast<ListEntry*>(ItemAt(i));
           bytes += item->m_length;
           selCount++;
           Select (i, true);
        }

        UpdateWindow();
    }

    m_sendSelectionMessage = t;
    // Optimized counting here too - for those terribly large archives
    PostSelectionMessage (&selCount, &bytes);
}

//=============================================================================================================//

void BeezerListView::SelectAll ()
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;
    
    int32 count = CountItems();
    uint32 bytes = 0L;
    for (int32 i = 0; i < count; i++)
    {
        // Later maybe count the bytes here itself and pass it as a parameter to a new
        // SelectionChanged() function maybe?? It will improve speed drastically as
        // SelectionChanged() need not reloop to recount bytes
        ListEntry *item = reinterpret_cast<ListEntry*>(ItemAt (i));
        bytes += item->m_length;
        
        Select (i, true);
        UpdateWindow();
    }
    
    m_sendSelectionMessage = t;
    PostSelectionMessage (&count, &bytes);
}

//=============================================================================================================//

void BeezerListView::DeselectAll ()
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;

    ColumnListView::DeselectAll();
    
    m_sendSelectionMessage = t;
    SelectionChanged();
}

//=============================================================================================================//

void BeezerListView::ToggleAllSuperItems (bool expand)
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;

    void (BeezerListView::*toggleFunc)(CLVListItem*);
    if (expand == true)
        toggleFunc = &BeezerListView::Expand;
    else
        toggleFunc = &BeezerListView::Collapse;

    int32 count = FullListCountItems();
    for (int32 i = 0; i < count; i++)
    {
        CLVListItem *iThItem = FullListItemAt (i);
        if (iThItem->IsSuperItem())
           (this->*toggleFunc)(iThItem);

        UpdateWindow();
    }

    m_sendSelectionMessage = t;
    SelectionChanged();
    Invalidate();
}

//=============================================================================================================//

void BeezerListView::ToggleSelectedSuperItems (bool expand)
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;

    void (BeezerListView::*toggleFunc)(CLVListItem*);
    if (expand == true)
        toggleFunc = &BeezerListView::Expand;
    else
        toggleFunc = &BeezerListView::Collapse;

    CLVListItem *selEntry = NULL;
    int32 i = 0L;
    while ((selEntry = FullListItemAt (FullListCurrentSelection (i++))) != NULL)
    {
        if (selEntry->IsSuperItem())
           (this->*toggleFunc)(selEntry);

        UpdateWindow();
    }

    m_sendSelectionMessage = t;
    SelectionChanged();
}

//=============================================================================================================//

int32 BeezerListView::Search (const BMessage *message, const char *&errorString)
{
    // Turn off sending of selection change message
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;

    RegExStringExpressionType expressionType;
    BString expression;
    int8 columnIndex;
    CLVColumn *columnPtr;
    const char *expressionPtr;
    bool invertSelection, ignoreCase, addToSelection, searchSelection, allFiles;

    message->FindInt32 (kExprType, (int32*)&expressionType);
    message->FindString (kExpr, &expressionPtr);
    message->FindBool (kInvertSelection, &invertSelection);
    message->FindBool (kIgnoreCase, &ignoreCase);
    message->FindBool (kAddToSelection, &addToSelection);
    message->FindBool (kSearchSelection, &searchSelection);
    message->FindBool (kAllFiles, &allFiles);
    message->FindPointer (kColumnPtr, reinterpret_cast<void**>(&columnPtr));
    
    errorString = NULL;
    columnIndex = IndexOfColumn (columnPtr);
    
    expression = expressionPtr;
    if (addToSelection == false && searchSelection == false)
        DeselectAll();
    
    RegExp regExpr;
    if (expressionType == kRegexpMatch)
    {
        regExpr.SetTo (expression);
        if (regExpr.InitCheck() != B_OK)
        {
           errorString = strdup (regExpr.ErrorString());

           // Restore selection message bool before returning!        
           m_sendSelectionMessage = t;
           SelectionChanged();
           
           return -1;
        }
    }
    
    BList selectedItemsList;
    if (allFiles)
    {
        // We need to preserve selection in this case as expanding superitems will blow away current
        // selection
        if (addToSelection == true)
        {
           CLVEasyItem *selItem = NULL;
           int32 i = 0L;
        
           while ((selItem = (CLVEasyItem*)FullListItemAt (FullListCurrentSelection (i++))) != NULL)
               selectedItemsList.AddItem ((void*)selItem);
        }
        
        ToggleAllSuperItems (true);
        UpdateWindow ();
    }

    int32 found = 0L;
    
    // No need to use FullListxxxx functions as we expanded the list if need be, so
    // CountItems() and FullListCount() items will return the same - besides we can
    // have a faster search for collapsed lists since CountItems() will be < than FullListCountItems()
    if (searchSelection == false)
    {
        int32 count = CountItems();
        
        for (int32 index = 0; index < count; index++)
        {
           CLVEasyItem *item = (CLVEasyItem*)ItemAt (index);
           const char *columnText = item->GetColumnContentText (columnIndex);
           
           if (!columnText)
               continue;
           
           RegExString regExStr (columnText);
           if (regExStr.Matches (expression.String(), !ignoreCase, expressionType) ^ invertSelection)
           {
               found++;
               Select (index, true);
           }
        }
        
        // Now select the items that were selected before expanding (stored in the list)
        if (addToSelection == true)
        {
           count = selectedItemsList.CountItems();
           for (int32 index = 0; index < count; index++)
               Select (FullListIndexOf ((CLVListItem*)selectedItemsList.ItemAtFast(index)), true);
        }
    }
    else
    {
        BList unmatchedItems;
        CLVEasyItem *selectedItem = NULL;
        int32 i = 0L;
        
        // Search through selection without modifying the selections
        while ((selectedItem = (CLVEasyItem*)FullListItemAt (FullListCurrentSelection (i++))) != NULL)
        {
           const char *columnText = selectedItem->GetColumnContentText (columnIndex);
           if (!columnText)
               continue;
               
           RegExString regExStr (columnText);
           if (regExStr.Matches (expression.String(), !ignoreCase, expressionType) ^ invertSelection)
               found++;
           else
               unmatchedItems.AddItem ((void*)selectedItem);
        }
        
        // addToSelection works as "Deselect unmatched entries" in "Search selection" mode
        if (addToSelection == true)
        {
           // Now deselect all unmatched items we have got from searching the current selection
           int32 count = unmatchedItems.CountItems();
           for (int32 index = 0; index < count; index++)
           {
               CLVEasyItem *item = reinterpret_cast<CLVEasyItem*>(unmatchedItems.ItemAtFast(index));
               Deselect (IndexOf (item));
           }
        }
    }

    ScrollToSelection();

    // Restore selection message bool
    m_sendSelectionMessage = t;
    SelectionChanged();
    
    return found;
}

//=============================================================================================================//

void BeezerListView::CopyToClipboard (char columnSeparator)
{
    BList visibleColumnList;
    int32 columnCount = CountColumns();
    
    // Copy according to the display order by adding items to the visibleColumnList as per the display order
    int32 *displayOrderArray = new int32[columnCount];
    GetDisplayOrder (displayOrderArray);

    // Form the ordered visible column list now
    for (int32 k = 2; k < columnCount; k++)
    {
        CLVColumn *column = (CLVColumn*)ColumnAt (displayOrderArray[k]);
        if (column->IsShown())
           visibleColumnList.AddItem ((void*)displayOrderArray[k]);
    }
    columnCount = visibleColumnList.CountItems();


    // Now get all the selected items in the visible columns
    BString buf;
    CLVEasyItem *selectedItem = NULL;
    int32 i = 0L;
    
    while ((selectedItem = (CLVEasyItem*)FullListItemAt (FullListCurrentSelection (i++))) != NULL)
    {
        for (int32 j = 0; j < columnCount; j++)
        {
           const char *columnText = selectedItem->GetColumnContentText ((int32)visibleColumnList.ItemAt(j));
           if (columnText && !(strcmp (columnText, "") == 0))
               buf << columnText;

           buf << columnSeparator;
        }
        buf << '\n';
        
        UpdateWindow();
    }
    
    if (buf.Length() > 0L)
    {
        // Now copy the buffer to the clipboard
        BMessage *clip = NULL;
        if (be_clipboard->Lock())
        {
           be_clipboard->Clear();
           if ((clip = be_clipboard->Data()))
           {
               clip->AddData ("text/plain", B_MIME_TYPE, buf.String(), buf.Length());
               be_clipboard->Commit();
           }
           be_clipboard->Unlock();
        }
    }

    delete[] displayOrderArray;
}

//=============================================================================================================//

void BeezerListView::UpdateWindow () const
{
    Window()->UpdateIfNeeded();
}

//=============================================================================================================//

void BeezerListView::MakeFocus (bool focused)
{
    Window()->PostMessage (M_GOT_FOCUS);
    
    ColumnListView::MakeFocus (focused);
}

//=============================================================================================================//

int BeezerListView::SortAsPerName (const char *name1, const char *name2, BList *columnList, int32 sortKey)
{
    // Intelligent sort, if name is sorted by ascending order do NOT disturb it, this
    // checks how Name column is sorted and sort accordingly - very good - this is how it is
    // supposed to be
    if (((CLVColumn*)columnList->ItemAt(2))->SortMode() == NoSort)
    {
        if (((CLVColumn*)columnList->ItemAt(sortKey))->SortMode() == Descending)
           return strcasecmp (name1, name2);
        else
           return -strcasecmp (name1, name2);
    }
    else if (((CLVColumn*)columnList->ItemAt(2))->SortMode() ==
               ((CLVColumn*)columnList->ItemAt(sortKey))->SortMode())
        return strcasecmp (name1, name2);
    else
        return -strcasecmp (name1, name2);
}

//=============================================================================================================//

int BeezerListView::SortFunction (const CLVListItem *a, const CLVListItem *b, BList *columnList,
                      int32 sortKey)
{
    // If neither ascending/descending just don't sort!
    if (sortKey == -1)
        return 0;
    
    const ListEntry *x = dynamic_cast<const ListEntry*>(a);
    const ListEntry *y = dynamic_cast<const ListEntry*>(b);
    
    // Bug-fix -- This case will come when no sorting is selected ie not ascending/descending
    if (x == NULL || y == NULL)
        return 0;
    
    const char *name1 = (const_cast<ListEntry*>(x))->GetColumnContentText (2);
    const char *name2 = (const_cast<ListEntry*>(y))->GetColumnContentText (2);
    const char *s1 = (const_cast<ListEntry*>(x))->GetColumnContentText (sortKey);
    const char *s2 = (const_cast<ListEntry*>(y))->GetColumnContentText (sortKey);
    bool xSuper = x->IsSuperItem();
    bool ySuper = y->IsSuperItem();

    // Don't compare a folder and file item as we ALWAYS SORT FOLDERS FIRST
    if (xSuper == true && ySuper == false)
        return -1;
    else if (ySuper == true && xSuper == false)
        return 1;

    // If both are folders -- we do smart sorting, don't do this smart sorting
    // for the name column
    if (xSuper == true && ySuper == true && sortKey != 2)
        return SortAsPerName (name1, name2, columnList, sortKey);
    
    int32 retValue = 0;
    switch (sortKey)
    {
        // Name column
        case 2:
        {
           retValue = strcasecmp (s1, s2);
           break;
        }
        
        // Size and Packed Columns
        case 3: case 4:
        {
           int64 valx = BytesFromString ((char*)s1);
           int64 valy = BytesFromString ((char*)s2);
           
           if (valx < valy)
               retValue = -1;
           else if (valx > valy)
               retValue = 1;
           else    // If same size sort intelligently by their name
               retValue = SortAsPerName (name1, name2, columnList, sortKey);
           
           break;
        }
        
        // Ratio column
        case 5:
        {
           double vs1, vs2;
           vs1 = s1 == NULL ? 0 : atof (s1);
           vs2 = s2 == NULL ? 0 : atof (s2);

           if (vs1 < vs2)
               retValue = -1;
           else if (vs1 > vs2)
               retValue = 1;
           else    // If same ratio sort intelligently by their name
               retValue = SortAsPerName (name1, name2, columnList, sortKey);
           
           break;
        }
        
        // Date column
        case 7:
        {
           if (x->m_timeValue < y->m_timeValue)
               retValue = -1;
           else if (x->m_timeValue > y->m_timeValue)
               retValue = 1;
           else
               retValue = SortAsPerName (name1, name2, columnList, sortKey);
           
           break;
        }
        
        // Path, Method, CRC column
        case 6: case 8: case 9:
        {
           retValue = strcasecmp (s1, s2);
           if (retValue == 0)        // If same sort intelligently by names
               retValue = SortAsPerName (name1, name2, columnList, sortKey);
           
           break;
        }
    }

    return retValue;
}

//=============================================================================================================//

void BeezerListView::SelectSubItemsOfSelection (bool select)
{
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;

    // Select the entire folder for all selected items:
    // Eg:
    //        > A
    //           B               if C is selected, then select B, C, D. Do this for all
    //           C               selected items like C. (Note A is not selected)
    //           D               if "select" is false - then deselect all instead of selecting.

    register int32 i = 0L;
    BList originalSelectionList;
    CLVListItem *item;

    // Hold currently selected items in a list, then modify the selection, if we do both in the same
    // loop we may end up in an infinite loop as adding selections will keep the loop going
    while ((item = FullListItemAt(FullListCurrentSelection (i++))) != NULL)
        originalSelectionList.AddItem ((void*)item);
    
    // Select the folder level of the selected items
    int32 count = originalSelectionList.CountItems();
    for (i = 0; i < count; i++)
    {
        item = (CLVListItem*)originalSelectionList.ItemAtFast (i);
        if (!item->IsSuperItem())
           item = Superitem (item);
        
        if (item)
        {
           if (select)
               SelectSubItems (item);
           else
               DeselectSubItems (item);
        }
    }

    m_sendSelectionMessage = t;
    SelectionChanged();
}

//=============================================================================================================//

int32 BeezerListView::FullListSelectionCount () const
{
    // Return the number of items selected in full list
    int32 count = 0L, i = 0L;
    while (FullListCurrentSelection(i++) >= 0L)
        count++;

    return count;
}

//=============================================================================================================//

int32 BeezerListView::SelectionCount () const
{
    int32 count = 0L, i = 0L;
    while (CurrentSelection(i++) >= 0L)
        count++;
    
    return count;
}

//=============================================================================================================//

bool BeezerListView::HasSelection () const
{
    int32 i = 0L;
    while (FullListCurrentSelection (i++) >= 0L)
        return true;
    return false;
}

//=============================================================================================================//

void BeezerListView::SendSelectionMessage (bool send)
{
    m_sendSelectionMessage = send;
}

//=============================================================================================================//

bool BeezerListView::InitiateDrag (BPoint point, int32 index, bool wasSelected)
{
    BMessage *dragMessage = new BMessage (B_SIMPLE_DATA);
    
    int32 selCount = FullListSelectionCount();
    bool fullArchiveExtract = (selCount == FullListCountItems() ? true : false);
    
    BPoint location;
    uint32 buttons;
    GetMouse(&location, &buttons, false);
    
    dragMessage->AddBool (kFieldFull, fullArchiveExtract);
    dragMessage->AddString ("be:types", "application/octet-stream");
    dragMessage->AddString ("be:filetypes", "application/octet-stream");
    dragMessage->AddString ("be:clip_name", kDropClipping);
    dragMessage->AddInt32 ("buttons", buttons);                         // For future use?
    dragMessage->AddBool (kPfSpecialField, true);                      // To identify in MouseMoved()
    dragMessage->AddInt32 ("be:actions", B_COPY_TARGET);
    
    // Get app icon and construct the text to display while dragging
    BBitmap *appSmallIcon = _glob_bitmap_pool->m_smallAppIcon;
    BString dragStr;
    if (fullArchiveExtract)
        dragStr = str (S_DRAG_EXTRACT_ARCHIVE);
    else
    {
        //    Later use this to count folders, files -- call Dumb or Smart version
        //    depending on the prefs (whether extraction of empty folders is allowed)
        int32 fileCount, folderCount;
        CountSelectionSmart (fileCount, folderCount);
    
        if (fileCount == 0)
           dragStr << folderCount << " " << (folderCount > 1 ? str (S_DRAG_FOLDERS) : str (S_DRAG_FOLDER));
        else if (folderCount == 0)
           dragStr << fileCount << " " << (fileCount > 1 ? str (S_DRAG_FILES) : str (S_DRAG_FILE));
        else
        {
           dragStr << folderCount << " " << (folderCount > 1 ? str (S_DRAG_FOLDERS) : str (S_DRAG_FOLDER))
               << " (" << fileCount << " " << (fileCount > 1 ? str (S_DRAG_FILES) : str (S_DRAG_FILE)) << ")";
        }
        dragStr << " " << str (S_DRAG_TO_EXTRACT);        
    }
    
    // Autosize the height to fit icon and text height
    font_height fnt_ht;
    be_plain_font->GetHeight (&fnt_ht);
    float ht = fnt_ht.ascent + fnt_ht.descent;
    float iconSize = appSmallIcon->Bounds().Height();
    ht = ht < iconSize ? iconSize + 2 : ht + 2;

    // Find the overall rectangular region for the dragged visual    
    BRect rect;
    rect.top = rect.left = 0;
    rect.right = be_plain_font->StringWidth (dragStr.String()) + 4 + iconSize;
    rect.bottom = ht;

    // Construct the drag bitmap
    BBitmap *dragBitmap = new BBitmap (rect, B_RGBA32, true, false);
    BView *dragView = new BView (rect, "bzr_dragView", B_FOLLOW_LEFT, B_WILL_DRAW);
    dragBitmap->Lock();
    dragBitmap->AddChild (dragView);
    
    // Set the transparent background by filling with transparent color
    BRect frameRect = dragView->Frame();
    dragView->SetHighColor (B_TRANSPARENT_COLOR);
    dragView->FillRect (frameRect);

    // Find the vertical center of the view so we can vertically center everything
    int centerY = (int)((frameRect.bottom - frameRect.top) / 2);

    // Draw mode set to half opaque
    dragView->SetDrawingMode (B_OP_ALPHA);
    dragView->SetHighColor (0, 0, 0, 138);
    dragView->SetBlendingMode (B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);

    // Center the icon in the view
    dragView->MovePenTo (BPoint (frameRect.left, centerY - (iconSize / 2)));
    dragView->DrawBitmap (appSmallIcon);
    
    // Draw the text
    dragView->SetFont (be_plain_font);
    dragView->MovePenTo (frameRect.left + iconSize + 4,    frameRect.bottom - fnt_ht.descent - 1);
    dragView->DrawString (dragStr.String());
    
    // Since offscreen bitmap, flush to make sure draws are complete
    dragView->Flush();
    dragBitmap->Unlock();
    
    // Finally! Initiate drag and drop action
    DragMessage (dragMessage, dragBitmap, B_OP_ALPHA, BPoint (frameRect.left + iconSize,frameRect.bottom - 2),
                  this);
    delete dragMessage;
    return true;
}

//=============================================================================================================//

void BeezerListView::CountSelectionDumb (int32 &subItems, int32 &superItems)
{
    // Doesn't recurse into folders
    ListEntry *selEntry (NULL);
    int32 i = 0L, fileCount = 0L, folderCount = 0L;
    
    while ((selEntry = (ListEntry*)FullListItemAt (FullListCurrentSelection (i))) != NULL)
    {
        if (selEntry->IsSuperItem() == false)
           fileCount++;
        else
           folderCount++;

        i++;
    }
    
    subItems = fileCount;
    superItems = folderCount;
}

//=============================================================================================================//

void BeezerListView::CountSelectionSmart (int32 &subItems, int32 &superItems)
{
    // Recurse into folders only when folder items are collapsed
    ListEntry *selEntry (NULL);
    int32 i = 0L, fileCount = 0L, folderCount = 0L;
    
    while ((selEntry = (ListEntry*)FullListItemAt (FullListCurrentSelection (i))) != NULL)
    {
        if (selEntry->IsSuperItem() == false)
           fileCount++;
        else
        {
           folderCount++;

           // If a folder is collapsed and selected add all its subitems to our message
           // this is done in "Smart" version of this counting function
           //if (selEntry->IsExpanded() == false)
           //    CountSubItemsOf (fileCount, folderCount, (CLVListItem*)selEntry);
        }
        i++;
    }
    
    subItems = fileCount;
    superItems = folderCount;
}

//=============================================================================================================//

void BeezerListView::CountSubItemsOf (int32 &subItems, int32 &superItems, CLVListItem *item)
{
    // Count all the subitems of the given superitem (don't recurse here, as calling function
    // -- CountSelectionSmart for example already does recursion into folders using this function)
    uint32 parentLevel = item->OutlineLevel();
    int32 fileCount = 0L, folderCount = 0L, i = 0L;
    int32 itemPos = FullListIndexOf (item);
    
    if (itemPos >= 0)
    {
        for (i = itemPos + 1; i >= 1; i++)
        {
           ListEntry *subItem = (ListEntry*)FullListItemAt (i);
           if (subItem == NULL || subItem->OutlineLevel() <= parentLevel)
               break;

           if (subItem->IsSuperItem() == false)
               fileCount++;
           else
               folderCount++;
        }
    }
    
    subItems += fileCount;
    superItems += folderCount;
}

//=============================================================================================================//

void BeezerListView::SelectAllEx (bool superItems)
{
    // Turn off sending of selection change message
    // Select all folders/files as per "superItems" value
    bool t = m_sendSelectionMessage;
    m_sendSelectionMessage = false;
    DeselectAll ();
    
    int32 count = CountItems();
    int32 selCount = 0L;
    uint32 bytes = 0L;
    
    for (int32 i = 0; i < count; i++)
    {
        ListEntry *item = reinterpret_cast<ListEntry*>(ItemAt(i));
        
        if (item->IsSuperItem() == superItems)
        {
           selCount++;
           bytes += item->m_length;
           Select (i, true);
        }
        
        UpdateWindow();
    }
    
    m_sendSelectionMessage = t;
    
    // Optimized counting for large archives
    PostSelectionMessage (&selCount, &bytes);
}

//=============================================================================================================//

void BeezerListView::GetState (BMessage &msg) const
{
    int8 nColumns = CountColumns();

    // Add width AND visibility of columns
    // This saves space -- we add minus width if the column is hidden instead of adding a bool for each
    // column's visibility - nice optimization in terms of space we reduce a good 16 bytes
    for (int8 i = 0; i < nColumns; i++)
        if (ColumnAt(i)->IsShown() == true)
           msg.AddFloat (kColumnWidth, ColumnAt(i)->Width());
        else
           msg.AddFloat (kColumnWidth, -ColumnAt(i)->Width());    

    // Add order of columns
    int32 *displayOrder = new int32[nColumns];
    GetDisplayOrder (displayOrder);
    for (int8 i = 0; i < nColumns; i++)
        msg.AddInt8 (kColumnOrder, (int8)displayOrder[i]);
    delete[] displayOrder;
    
    // Add sort settings
    int32 *sortKeys  = new int32[nColumns];
    CLVSortMode *sortModes = new CLVSortMode[nColumns];
    int32 nSortKeys = GetSorting (sortKeys, sortModes);
    for (int32 i = 0; i < nSortKeys; i++)
    {
        msg.AddInt8 (kSortKey, (int8)sortKeys[i]);
        msg.AddInt8 (kSortMode, (int8)sortModes[i]);
    }

    delete[] sortKeys;
    delete[] sortModes;
}

//=============================================================================================================//

void BeezerListView::SetState (BMessage *msg)
{
    int8 nColumns = CountColumns();
    float width;
    int8 dummy;
    
    // Restore width AND visibility of columns
    for (int8 i = 0; i < nColumns; i++)
    {
        if (msg->FindFloat (kColumnWidth, i, &width) == B_OK)
        {
           if (width > 0)
               ColumnAt(i)->SetWidth (width);
           else
           {
               ColumnAt(i)->SetShown (false);
               ColumnAt(i)->SetWidth (-width);
           }
        }
    }

    // Restore order of columns
    int32 *displayOrder = new int32[nColumns];
    for (int8 i = 0; i < nColumns; i++)
        if (msg->FindInt8 (kColumnOrder, i, &dummy) != B_OK)
           displayOrder[i] = i;
        else    
           displayOrder[i] = (int32)dummy;
    
    SetDisplayOrder (displayOrder);
    delete[] displayOrder;
    
    // Restore sort settings
    int32 *sortKeys = new int32[nColumns];
    CLVSortMode *sortModes = new CLVSortMode[nColumns];
    int8 nSortKeys = 0;
    for (int32 i = 0; (msg->FindInt8 (kSortKey, i, &dummy) == B_OK); i++)
    {
        sortKeys[i] = (int32)dummy;
        int8 temp;
        sortModes[i] = (msg->FindInt8 (kSortMode, i, &temp) == B_OK) ? (CLVSortMode)temp : NoSort;
        nSortKeys++;
    }

    if (nSortKeys > 0)
        SetSorting (nSortKeys, sortKeys, sortModes);

    delete[] sortKeys;
    delete[] sortModes;
}

//=============================================================================================================//
