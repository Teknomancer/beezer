// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _SEARCH_WINDOW_H
#define _SEARCH_WINDOW_H

#include <Window.h>

class BButton;
class BCheckBox;
class BEntry;
class BList;
class BMenuField;
class BRadioButton;
class BTextControl;

class CLVColumn;

class Archiver;

class SearchWindow : public BWindow
{
    public:
        SearchWindow(BWindow* caller, BMessage* loadMessage,
                     BEntry const* entry, BList const& columnList, Archiver const* ark);
        virtual ~SearchWindow();

        // Inherited hooks
        virtual void        Quit();
        virtual void        MessageReceived(BMessage* message);

        // Additional hooks
        int32               ExpressionType() const;
        CLVColumn*          Column() const;
        void                SetToolTips();
        void                GetSettings(BMessage& msg, uint32 msgwhat) const;

    private:
        BList*              m_columnList;
        BWindow*            m_callerWindow;
        BTextControl*       m_searchTextControl;
        BButton*            m_searchBtn;
        BMenuField*         m_columnField,
                            *m_matchField;
        BRadioButton*       m_allEntriesOpt,
                            *m_visibleEntriesOpt,
                            *m_selEntriesOpt;
        BCheckBox*          m_addToSelChk,
                            *m_ignoreCaseChk,
                            *m_invertChk,
                            *m_persistentChk;
        BMessage*           m_loadMessage;
};

#endif /* _SEARCH_WINDOW_H */
