// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ARCHIVER_MGR_H
#define _ARCHIVER_MGR_H

class BList;
class BLocker;
class BPopUpMenu;
class BString;

class Archiver;
class RuleMgr;


const char* const kLoaderFunc =        "load_archiver";

extern BLocker _ark_locker;

Archiver* ArchiverForMime(const char* mimeType);
Archiver* ArchiverForType(const char* archiverType);
BList ArchiversInstalled(BList* extensionStrings);
BPopUpMenu* BuildArchiveTypesMenu(BHandler* targetHandler, BList* extensionStrings);
status_t MergeArchiverRules(RuleMgr* ruleMgr);
Archiver* NewArchiver(const char* name, bool popupErrors, status_t* returnCode);

#endif /* _ARCHIVER_MGR_H */
