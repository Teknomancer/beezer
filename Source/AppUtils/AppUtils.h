/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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
 * -> Neither the name of the author nor the names of its contributors may
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

#ifndef _APP_UTILS_H
#define _APP_UTILS_H

#include <Entry.h>
#include <String.h>

class BBitmap;
class BResources;
class BDirectory;
class BLocker;

// Global data
const char *const kTrackerSignature = "application/x-vnd.Be-TRAK";

extern BLocker        _apputils_locker;

// Global functions (these should never access data/resources defined outside them)
extern BString        StringFromBytes (int64 bytes);
extern int32         CountCharsInFront (char *str, char whatChar);
extern int32         LastOccurrence (const char *buffer, char whatChar);
extern char         *Extension (const char *fileName, int extLen);
extern char         *ParentPath (const char *pathStr, bool truncateSlash);
extern const char    *LeafFromPath (const char *path);
extern const char    *FinalPathComponent (const char *path);
extern bool          StrEndsWith (char *str, char *end);
extern BString        SupressWildcards (const char *str);
extern BString        SupressWildcardSet (const char *str);

#endif /* _APP_UTILS_H */
