/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _LOCAL_UTILS_H
#define _LOCAL_UTILS_H

const int64    kHalfKBSize = 512;
const int64    kKBSize = 1024;
const int64    kMBSize = 1048576;
const int64    kGBSize = 1073741824;
const int64    kTBSize = kGBSize * kKBSize;

class BString;
class BMenu;
class BHandler;

bool                OpenEntry (const char *tempDirPath, const char *entryPath, bool openWith);
bool                TrackerOpenWith (entry_ref *ref);
void                TrackerOpenFolder (entry_ref *refToDir);
BString             CommaFormatString (off_t num);
BString             LocaleStringFromBytes (int64 val);
BBitmap            *ResBitmap (const char *name, uint32 type = 'BBMP');
int64               BytesFromString (char *text);
void                SetTargetForMenuRecursive (BMenu *menu, BHandler *target);

#endif /* _LOCAL_UTILS_H */
