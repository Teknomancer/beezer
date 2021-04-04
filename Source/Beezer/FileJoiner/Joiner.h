// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _JOINER_H
#define _JOINER_H

#if __GNUC__ > 2
#define _JOINER_IMPEXP
#else
#ifdef _JOINER_BUILD
#define _JOINER_IMPEXP __declspec (dllexport)
#else
#define _JOINER_IMPEXP __declspec (dllimport)
#endif
#endif

class BMessenger;
class BEntry;
class BDirectory;
class BNode;
class BString;

extern "C" _JOINER_IMPEXP status_t JoinFile(const char* firstChunkPathStr, const char* outputDir,
        const char* separator, BMessenger* progress, volatile bool* cancel);

extern "C" _JOINER_IMPEXP void JoinCopyAttributes(BNode* srcNode, BNode* destNode, void* buffer, size_t bufSize);

extern "C" _JOINER_IMPEXP void FindChunks(const char* firstChunkPathStr, const char* separator, int32& fileCount,
        off_t& totalSize, volatile bool* cancel);


#endif /* _JOINER_H */
