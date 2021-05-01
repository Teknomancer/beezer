// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _APP_UTILS_H
#define _APP_UTILS_H

#include <String.h>

extern BString        StringFromBytes(uint64 bytes);
extern BString        StringFromDigitalSize(char *size, char *unit);
extern int8           MonthStrToNum(const char* month);
extern int32          CountCharsInFront(char* str, char whatChar);
extern int32          LastOccurrence(const char* buffer, char whatChar);
extern char*          Extension(const char* fileName, int extLen);
extern char*          ParentPath(const char* pathStr, bool truncateSlash);
extern const char*    LeafFromPath(const char* path);
extern const char*    FinalPathComponent(const char* path);
extern bool           StrEndsWith(const char* str, const char* end);
extern BString        SupressWildcards(const char* str);
extern BString        SupressWildcardSet(const char* str);
extern bool           IsPermString(const char *str, size_t len);
extern void           StrReverse(char *str, size_t len);
extern BString        RootPathAtDepth(const char *pathStr, size_t pathStrLen, size_t depth);

#endif /* _APP_UTILS_H */
