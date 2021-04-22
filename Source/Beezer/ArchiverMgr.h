// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ARCHIVER_MGR_H
#define _ARCHIVER_MGR_H

#include <SupportDefs.h>

class BHandler;
class BList;
class BLocker;
class BMessage;
class BPath;
class BPopUpMenu;

class Archiver;
class RuleMgr;

const char* const kLoaderFunc = "load_archiver";

extern BLocker _ark_locker;

class ArchiverMgr
{
    public:
                        ArchiverMgr();
                        ~ArchiverMgr();

        Archiver*       ArchiverForMime(const char* mimeType);
        Archiver*       ArchiverForType(const char* archiverType);
        status_t        ArchiversInstalled(BList& arkTypeList, BList* extensionStrings);
        BPopUpMenu*     BuildArchiveTypesMenu(BHandler* targetHandler, BList* extensionStrings);
        Archiver*       NewArchiver(const char* name, bool popupErrors, status_t* returnCode);
        Archiver*       InstantiateArchiver(const char* path);
        char*           ValidateFileType(BPath* path);

    private:
        status_t        MergeArchiverRules();

        BMessage*       m_fullMetaDataMsg;
        RuleMgr*        m_ruleMgr;

};

ArchiverMgr* _archiverMgr();

#endif /* _ARCHIVER_MGR_H */
