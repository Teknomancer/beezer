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

#ifndef _ARK_INFO_WINDOW_H
#define _ARK_INFO_WINDOW_H

#include <Window.h>

class BButton;
class BEntry;
class BList;
class BStatusBar;
class BStringView;

class Archiver;
class BevelView;

class ArkInfoWindow : public BWindow
{
    public:
        ArkInfoWindow (BWindow *callerWindow, Archiver *archiver, BEntry *archiveEntry);

        // Inherited hooks
        virtual bool        QuitRequested ();

    private:
        // Private hooks
        void                FillDetails ();
        void                AutoSizeWindow (float cornerMargin, float midMargin, float leftSideMaxWidth);

        // Private members
        BevelView          *m_backView;
        Archiver           *m_archiver;

        BEntry             *m_entry;
        BList              *m_fileList,
                         *m_dirList;
        BButton            *m_closeButton;
        BStatusBar         *m_compressRatioBar;
        BStringView        *m_fileNameStr,
                         *m_compressedSizeStr,
                         *m_originalSizeStr,
                         *m_fileCountStr,
                         *m_folderCountStr,
                         *m_totalCountStr,
                         *m_typeStr,
                         *m_pathStr,
                         *m_createdStr,
                         *m_modifiedStr;
};

#endif /*_ARK_INFO_WINDOW_H */
