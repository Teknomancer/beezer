// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _RECENT_MGR_H
#define _RECENT_MGR_H

#include <SupportDefs.h>

class BHandler;
class BList;
class BLocker;
class BMenu;
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
        RecentMgr(int32 maxNumPaths, Preferences* pref, RecentItemType itemType, bool showFullPath);
        virtual ~RecentMgr();

        void                AddPath(const char* path);
        void                RemovePath(const char* path);
        void                SavePrefs();
        void                LoadPrefs();
        void                SetCommand(uint32 command);
        void                SetMaxPaths(int32 maxNumPaths);
        void                SetShowFullPath(bool showFullPath);

        BMenu*              BuildMenu(const char* menuName, const char* fieldName, BHandler* targetForItems);
        BPopUpMenu*         BuildPopUpMenu(const char* menuName, const char* fieldName,
                                           BHandler* targetForItems);
        void                UpdateMenu(BMenu* recentMenu, const char* fieldName, BHandler* targetForItems);

    private:
        void                FillMenu(BMenu* menu, const char* fieldName, BHandler* targetForItems);

        static int32        m_maxInternalCount;

        Preferences*        m_prefs;
        BList*              m_paths;
        bool                m_showFullPath;
        int32               m_maxNumPaths;
        uint32              m_command;
        RecentItemType      m_type;
};

#endif /* _RECENT_MGR_H */
