// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ARK_INFO_WINDOW_H
#define _ARK_INFO_WINDOW_H

#include <Window.h>

class BButton;
class BEntry;
class BList;
class BStatusBar;
class BStringView;

class Archiver;

class ArkInfoWindow : public BWindow
{
    public:
        ArkInfoWindow(BWindow* callerWindow, Archiver* archiver, BEntry* archiveEntry);

        // Inherited hooks
        virtual bool        QuitRequested();

    private:
        void                FillDetails();

        Archiver*           m_archiver;

        BEntry*             m_entry;
        BList*              m_fileList,
                            *m_dirList;
        BButton*            m_closeButton;
        BStatusBar*         m_compressRatioBar;
        BStringView*        m_fileNameStr,
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
