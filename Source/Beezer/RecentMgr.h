/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _RECENT_MGR_H
#define _RECENT_MGR_H

#include <List.h>

class BLocker;
class BPopUpMenu;

class Preferences;

enum RecentItemType
{
    ritFile,
    ritFolder,
    ritAny,
};

extern BLocker _recent_locker;

class RecentMgr
{
    public:
        RecentMgr (int32 maxNumPaths, Preferences *pref, RecentItemType itemType, bool showFullPath);
        virtual ~RecentMgr ();
        
        // Public hooks
        void                 AddPath (const char *path);
        void                 RemovePath (const char *path);
        void                 SavePrefs ();
        void                 LoadPrefs ();
        void                 SetCommand (uint32 command);
        void                 SetMaxPaths (int32 maxNumPaths);
        void                 SetShowFullPath (bool showFullPath);

        BMenu               *BuildMenu (const char *menuName, const char *fieldName, BHandler *targetForItems);
        BPopUpMenu          *BuildPopUpMenu (const char *menuName, const char *fieldName,
                                            BHandler *targetForItems);
        void                 UpdateMenu (BMenu *recentMenu, const char *fieldName, BHandler *targetForItems);
        
    private:
        // Private hooks
        void                 FillMenu (BMenu *menu, const char *fieldName, BHandler *targetForItems);
        
        // Static members
        static int32         m_maxInternalCount;
        
        // Private members
        Preferences         *m_prefs;
        BList                m_paths;
        bool                 m_showFullPath;
        int32                m_maxNumPaths;
        uint32               m_command;
        RecentItemType       m_type;
};

#endif /* _RECENT_MGR_H */
