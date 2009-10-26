/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _ARCHIVER_MGR_H
#define _ARCHIVER_MGR_H

class BString;
class BList;
class BPopUpMenu;

#include "Archiver.h"
#include "Beezer.h"

const char * const kLoaderFunc =		"load_archiver";

extern BLocker _ark_locker;

Beezer *_bzr();
Archiver *ArchiverForMime (const char *mimeType);
Archiver *ArchiverForType (const char *archiverType);
BList ArchiversInstalled (BList *extensionStrings);
BPopUpMenu *BuildArchiveTypesMenu (BHandler *targetHandler, BList *extensionStrings);
Archiver* NewArchiver (const char *name, bool popupErrors, status_t *returnCode);

#endif /* _ARCHIVER_MGR_H */
