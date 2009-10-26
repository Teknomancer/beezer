/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _ARK_INFO_WINDOW_H
#define _ARK_INFO_WINDOW_H

#include <Window.h>

class BButton;
class BEntry;
class BList;
class BStringView;

class BevelView;
class Archiver;

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
        BevelView            *m_backView;
        Archiver            *m_archiver;
        
        BEntry                *m_entry;
        BList                *m_fileList,
                            *m_dirList;
        BButton                *m_closeButton;
        BStatusBar            *m_compressRatioBar;
        BStringView            *m_fileNameStr,
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
