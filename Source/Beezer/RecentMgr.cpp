// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Autolock.h>
#include <Debug.h>
#include <Entry.h>
#include <Locker.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Path.h>
#include <PopUpMenu.h>

#include <malloc.h>
#include <string.h>

#include "MsgConstants.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "RecentMgr.h"

BLocker _recent_locker("_recent_mgr_lock", true);
int32 RecentMgr::m_maxInternalCount = 99;



RecentMgr::RecentMgr(int32 maxNumPaths, Preferences* pref, RecentItemType allowedPathType, bool showFullPath)
{
    m_maxNumPaths = maxNumPaths;
    m_prefs = pref;
    m_type = allowedPathType;
    m_showFullPath = showFullPath;
    m_command = M_RECENT_ITEM;
    LoadPrefs();
}



RecentMgr::~RecentMgr()
{
    SavePrefs();

    for (int32 i = 0; i < m_paths.CountItems(); i++)
        free((char*)m_paths.RemoveItem((int32)0));
}



void RecentMgr::SetMaxPaths(int32 maxNumPaths)
{
    m_maxNumPaths = maxNumPaths;
}



void RecentMgr::SetShowFullPath(bool showFullPath)
{
    m_showFullPath = showFullPath;
}



void RecentMgr::SetCommand(uint32 command)
{
    m_command = command;
}



void RecentMgr::AddPath(const char* path)
{
    BAutolock codeLock(_recent_locker);
    if (!codeLock.IsLocked())
        return;

    for (int32 i = 0; i < m_paths.CountItems(); i++)
    {
        const char* existingPath = (const char*)m_paths.ItemAtFast(i);
        if (strcmp(existingPath, path) == 0)
            free((char*)m_paths.RemoveItem(i));
    }

    // Clip away any paths that are more than m_maxInternalCount, we store
    // m_maxInternalCount paths maximum, and display "m_maxNumPaths" (see FillMenu())
    if (m_paths.CountItems() > RecentMgr::m_maxInternalCount)
    {
        for (int32 i = m_maxNumPaths; i < m_paths.CountItems(); i++)
            free((char*)m_paths.RemoveItem(m_maxNumPaths));
    }

    m_paths.AddItem((void*)strdup(path), 0L);
}



void RecentMgr::RemovePath(const char* path)
{
    BAutolock codeLock(_recent_locker);
    if (!codeLock.IsLocked())
        return;

    m_paths.RemoveItem((void*)path);
}



BMenu* RecentMgr::BuildMenu(const char* menuName, const char* fieldName, BHandler* targetForItems)
{
    BMenu* recentMenu = new BMenu(menuName);
    FillMenu(recentMenu, fieldName, targetForItems);
    return recentMenu;
}



BPopUpMenu* RecentMgr::BuildPopUpMenu(const char* menuName, const char* fieldName, BHandler* targetForItems)
{
    BPopUpMenu* recentMenu = new BPopUpMenu(menuName, false, false);
    FillMenu(recentMenu, fieldName, targetForItems);
    return recentMenu;
}



void RecentMgr::UpdateMenu(BMenu* recentMenu, const char* fieldName, BHandler* targetForItems)
{
    // Non-destructive way to update an existing menu (without deleting because for a BMenu that is part of
    // a BMenuField, you cannot delete and re-assign a new BMenu to a BMenuField because BMenuField does not
    // allow that, so this can be used to remove all items from a menu, and update with new items
    for (int32 i = recentMenu->CountItems() - 1; i >= 0; i--)
        delete recentMenu->RemoveItem(i);

    FillMenu(recentMenu, fieldName, targetForItems);
}



void RecentMgr::FillMenu(BMenu* menu, const char* fieldName, BHandler* target)
{
    // "menu" must be allocated before calling this function
    int32 addedCount = 0L;
    for (int32 i = 0; i < m_paths.CountItems(); i++)
    {
        if (addedCount >= m_maxNumPaths || addedCount >= RecentMgr::m_maxInternalCount)
            break;

        BPath path = (const char*)m_paths.ItemAtFast(i);
        bool canAddInMenu = true;

        // For a file item, the file MUST exist AND must be a file
        // For a folder item, either it must exist AND be a folder or it must not exist
        BEntry dummyEntry(path.Path(), true);
        if (dummyEntry.Exists() == true)
        {
            if (m_type == ritFile && dummyEntry.IsFile() == false)
                canAddInMenu = false;
            else if (m_type == ritFolder && dummyEntry.IsDirectory() == false)
                canAddInMenu = false;
        }
        else if (m_type != ritFolder)    // MainWindow will create folder if needed
            canAddInMenu = false;

        if (canAddInMenu)
        {
            const char* displayPath = path.Path();
            if (m_showFullPath == false)
                displayPath = path.Leaf();

            BMessage* targetMessage = new BMessage(m_command);
            // FIXME  the Haiku version of AddFlat seems to be broken and is sending bad data
            // send the refs manually for now and reenable once it's fixed
            entry_ref ref;
            if (get_ref_for_path(path.Path(), &ref) == B_OK)
	            targetMessage->AddRef(fieldName, &ref);
            //targetMessage->AddFlat(fieldName, &path);

            BMenuItem* item = new BMenuItem(displayPath, targetMessage);
            menu->AddItem(item);
            addedCount++;
        }
    }

    if (target)
        menu->SetTargetForItems(target);
}



void RecentMgr::SavePrefs()
{
    // Save the recent path to preferences
    if (!m_prefs)
        return;

    m_prefs->MakeEmpty();

    for (int32 i = 0; i < m_paths.CountItems(); i++)
    {
        if (i >= RecentMgr::m_maxInternalCount)
            break;

        m_prefs->AddString(kPfRecentPath, (const char*)m_paths.ItemAtFast(i));
    }

    m_prefs->WritePrefs();
}



void RecentMgr::LoadPrefs()
{
    if (!m_prefs)
        return;

    const char* path;
    int32 i = 0L;
    while (m_prefs->FindString(kPfRecentPath, i++, &path) == B_OK)
        m_paths.AddItem((void*)strdup(path));
}


