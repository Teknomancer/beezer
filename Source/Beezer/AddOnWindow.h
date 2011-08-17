/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _ADDON_WINDOW_H
#define _ADDON_WINDOW_H

#include <Window.h>

class BButton;
class BStringView;
class BString;
class ArchiveRep;

class BevelView;
class BarberPole;

#define M_CLOSE_ADDON            'adtx'
#define M_FILENAME_CHANGED        'fnch'

const rgb_color
    SC_READY =                  { 0, 128, 0, 255 },
    SC_NOT_READY =               { 0, 0, 0, 255 },
    SC_DIR_MISSING =           { 182, 0, 0, 255 },
    SC_OVERWRITE =                { 128, 0, 0, 255 },
    SC_BUSY =                  { 0,  0, 128, 255 };

class AddOnWindow : public BWindow
{
    public:
        AddOnWindow (BMessage *refsMessage);
        
        // Inherited hooks
        virtual bool        QuitRequested ();
        virtual void        MessageReceived (BMessage *message);
        virtual void        Quit ();

    private:
        // Private hooks
        bool                ReplaceExtensionWith (const char *newExt);
        void                UpdateStatus (const char *text);
        void                ValidateData ();
        void                CreateArchiveRepAndMenus ();
        void                EnableControls (bool enable);
        void                RefsReceived (BMessage *message);
        
        // Private members
        BevelView          *m_backView,
                         *m_backViewMain,
                         *m_backViewAlt,
                         *m_addView;
        BTextControl        *m_fileName,
                         *m_password;
        BMenuField         *m_arkTypeField,
                         *m_arkSettingsMenuField;
        BPopUpMenu         *m_arkTypePopUp;
        BList               m_arkTypes,
                          m_arkExtensions;
        BButton            *m_helpBtn,
                         *m_createBtn;
        BStringView        *m_statusStr,
                         *m_addingFileStr;
        BarberPole         *m_barberPole;
        BString             m_archiveDirPath;
        ArchiveRep         *m_archive;
        BMessage            m_refsMessage;
        
        bool                m_readyMode,
                          m_inProgress,
                          m_quitNow;
        rgb_color           m_statusColor;
        volatile bool        m_cancel;
        float               m_strWidthOfArkTypes;
        thread_id           m_thread;
};

#endif /* _ADDON_WINDOW_H */
