/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _LOG_TEXT_VIEW_H
#define _LOG_TEXT_VIEW_H

#include <TextView.h>

class BPopUpMenu;

const BRect kInternalTextRect (2, 2, 6000, 0);

class LogTextView : public BTextView
{
    public:
        LogTextView (BRect frame, const char *name, uint32 resizeMask, uint32 flags = B_WILL_DRAW);
        LogTextView (BRect frame, const char *name, const BFont *initialFont,
                const rgb_color *initialColor, uint32 resizeMask, uint32 flags);
        
        // Inherited hooks
        virtual void         MakeFocus (bool focused = true);
        virtual void         MouseDown (BPoint point);
        
        // Additional hooks
        void                 Copy ();
        void                 SetContextMenu (BPopUpMenu *contextMenu);
        void                 AddText (const char *text, bool newLine = true,
                                bool capitalFirstLetter = false, bool trimLeadingSpaces = true);

    private:
        // Private hooks
        void                 InitSelf ();
        
        // Private members
        BPopUpMenu           *m_contextMenu;
};

#endif /* _LOG_TEXT_VIEW_H */
