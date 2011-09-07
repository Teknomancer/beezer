/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _SEARCH_WINDOW_H
#define _SEARCH_WINDOW_H

#include <Window.h>

class BButton;
class BCheckBox;
class BMenuField;
class BRadioButton;
class BTextControl;

class CLVColumn;

class Archiver;
class BevelView;

#define M_ALL_ENTRIES            '_all'
#define M_VISIBLE_ENTRIES        '_vsb'
#define M_SELECTED_ENTRIES        '_sel'
#define M_SEARCH_CLICKED         '_scl'
#define M_SEARCH_TEXT_MODIFIED    'stmf'
#define M_SEARCH_CLOSED          'Xsrc'

class SearchWindow : public BWindow
{
    public:
        SearchWindow (BWindow *caller, BMessage *loadMessage,
                    const BEntry *entry, const BList *columnList, const Archiver *ark);

        // Inherited hooks
        virtual void        Quit ();
        virtual void        MessageReceived (BMessage *message);

        // Additional hooks
        int32               ExpressionType () const;
        CLVColumn          *Column () const;
        void                SetToolTips ();
        void                GetSettings (BMessage &msg, uint32 msgwhat) const;

    private:
        // Private members
        BevelView          *m_backView;
        BList               m_tmpList;
        BWindow            *m_callerWindow;
        BTextControl        *m_searchTextControl;
        BButton            *m_searchBtn;
        BMenuField         *m_columnField,
                         *m_matchField;
        BRadioButton        *m_allEntriesOpt,
                         *m_visibleEntriesOpt,
                         *m_selEntriesOpt;
        BCheckBox          *m_addToSelChk,
                         *m_ignoreCaseChk,
                         *m_invertChk,
                         *m_persistentChk;
        BMessage           *m_loadMessage;
};

#endif /* _SEARCH_WINDOW_H */
