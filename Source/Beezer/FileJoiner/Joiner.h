/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _JOINER_H
#define _JOINER_H

#ifdef _JOINER_BUILD
    #define _JOINER_IMPEXP __declspec (dllexport)
#else
    #define _JOINER_IMPEXP __declspec (dllimport)
#endif

class BMessenger;
class BEntry;
class BDirectory;
class BNode;
class BString;

extern "C" _JOINER_IMPEXP status_t JoinFile (const char *firstChunkPathStr, const char *outputDir,
                                    const char *separator, BMessenger *progress, volatile bool *cancel);

extern "C" _JOINER_IMPEXP void JoinCopyAttributes (BNode *srcNode, BNode *destNode, void *buffer, size_t bufSize);

extern "C" _JOINER_IMPEXP void FindChunks (const char *firstChunkPathStr, const char *separator, int32 &fileCount,
            off_t &totalSize, volatile bool *cancel);


#endif /* _JOINER_H */
