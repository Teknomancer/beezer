// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _FS_UTILS_H
#define _FS_UTILS_H

#include <String.h>

#if __GNUC__ > 2
#define _FS_IMPEXP
#else
#if _FS_BUILD
#define _FS_IMPEXP __declspec (dllexport)
#else
#define _FS_IMPEXP __declspec (dllimport)
#endif
#endif

class BMessenger;
class BEntry;
class BDirectory;
class BNode;

extern "C" _FS_IMPEXP status_t CopyDirectory(BEntry* srcDir, BDirectory* destDir, BMessenger* progress,
                                             volatile bool* cancel);

extern "C" _FS_IMPEXP status_t CopyFile(BEntry* src, BDirectory* destDir, BMessenger* progress, volatile bool* cancel);

extern "C" _FS_IMPEXP void GetDirectoryInfo(BEntry* srcDir, int32& fileCount, int32& folderCount, off_t& totalSize,
                                            volatile bool* cancel);

extern "C" _FS_IMPEXP BString CreateTempDirectory(const char* prefix, BDirectory** createdDir, bool createNow);

extern "C" _FS_IMPEXP void CopyAttributes(BNode* srcNode, BNode* destNode, void* buffer, size_t bufSize);

extern "C" _FS_IMPEXP void RemoveDirectory(BDirectory* dir);

extern "C" _FS_IMPEXP status_t SplitFile(BEntry* src, BDirectory* destDir, BMessenger* progress, uint64 fragmentSize,
                                         uint16 fragmentCount, char* sepChar, BString& firstChunkName,
                                         volatile bool* cancel);

#endif /* _FS_UTILS_H */
