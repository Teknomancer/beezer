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

#ifndef _PREFS_VIEW_Interface_H
#define _PREFS_VIEW_Interface_H

#include "PrefsView.h"

class BColorControl;
class BMenuItem;

class PrefsViewInterface : public PrefsView
{
    public:
        PrefsViewInterface(BRect frame);

        // Inherited hooks
        virtual void        MessageReceived(BMessage* message);
        virtual void        AttachedToWindow();

        virtual void        Render();
        virtual void        Save();
        virtual void        Load();

    private:
        // Private hooks
        void                UpdateColorWell();
        void                UpdateColorControl(BMenuItem* item);
        bool                FindBoolDef(BMessage* msg, const char* name, bool defaultValue);

        // Private members
        BCheckBox*          m_fullLengthBarsChk,
                            *m_toolbarChk,
                            *m_infobarChk,
                            *m_actionLogChk;
        BView*              m_colorWell;
        BPopUpMenu*         m_colorPopUp,
                            *m_foldingPopUp;
        BMenuField*         m_colorField,
                            *m_foldingField;
        BColorControl*       m_colorControl;
        rgb_color           m_actFore,
                            m_actBack;
};

#endif /* _PREFS_VIEW_EXTRACT_H */
