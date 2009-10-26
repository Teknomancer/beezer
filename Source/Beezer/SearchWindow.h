/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _SEARCH_WINDOW_H
#define _SEARCH_WINDOW_H

#include <Window.h>

class BMenuField;
class BTextControl;
class BCheckBox;
class BButton;
class BRadioButton;

class CLVColumn;
class BubbleHelper;

class BevelView;
class Archiver;

#define M_ALL_ENTRIES             '_all'
#define M_VISIBLE_ENTRIES         '_vsb'
#define M_SELECTED_ENTRIES        '_sel'
#define M_SEARCH_CLICKED          '_scl'
#define M_SEARCH_TEXT_MODIFIED    'stmf'
#define M_SEARCH_CLOSED           'Xsrc'

class SearchWindow : public BWindow
{
    public:
        SearchWindow (BWindow *caller, BMessage *loadMessage, BubbleHelper *bubbleHelper,
                      const BEntry *entry, const BList *columnList, const Archiver *ark);
        
        // Inherited hooks
        virtual void         Quit ();
        virtual void         MessageReceived (BMessage *message);
        
        // Additional hooks
        int32                ExpressionType () const;
        CLVColumn           *Column () const;
        void                 SetBubbleHelps ();
        void                 GetSettings (BMessage &msg, uint32 msgwhat) const;
        
    private:
        // Private members
        BevelView           *m_backView;
        BList                m_tmpList;
        BWindow             *m_callerWindow;
        BTextControl        *m_searchTextControl;
        BButton             *m_searchBtn;
        BMenuField          *m_columnField,
                            *m_matchField;
        BRadioButton        *m_allEntriesOpt,
                            *m_visibleEntriesOpt,
                            *m_selEntriesOpt;
        BCheckBox           *m_addToSelChk,
                            *m_ignoreCaseChk,
                            *m_invertChk,
                            *m_persistentChk;
        BMessage            *m_loadMessage;
        BubbleHelper        *m_helper;
};

#endif /* _SEARCH_WINDOW_H */
