/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
