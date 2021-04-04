// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _LOCAL_UTILS_H
#define _LOCAL_UTILS_H

const int64    kHalfKBSize = 512;
const int64    kKBSize = 1024;
const int64    kMBSize = 1048576;
const int64    kGBSize = 1073741824;
const int64    kTBSize = kGBSize* kKBSize;

class BHandler;
class BMenu;
class BString;

bool               OpenEntry(const char* tempDirPath, const char* entryPath, bool openWith);
bool               TrackerOpenWith(entry_ref* ref);
void               TrackerOpenFolder(entry_ref* refToDir);
BString            LocaleStringFromBytes(int64 val);
BBitmap*           ResBitmap(const char* name, uint32 type = 'BBMP');
int64              BytesFromString(char* text);
void               SetTargetForMenuRecursive(BMenu* menu, BHandler* target);

#endif /* _LOCAL_UTILS_H */
