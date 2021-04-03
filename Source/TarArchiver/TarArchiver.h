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

#ifndef _TAR_ARCHIVER_H
#define _TAR_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class TarArchiver : public Archiver
{
    public:
        TarArchiver();
        TarArchiver(bool isBeingDerived);

        // Abstract Implementations & overridables
        status_t           Open(entry_ref* ref, BMessage* fileList);
        status_t           ReadOpen(FILE* fp);

        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           Extract(entry_ref* dir, BMessage* list, BMessenger* progress, volatile bool* cancel);

        status_t           Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);

        status_t           ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                               BMessenger* progress, volatile bool* cancel);

        status_t           Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                                  BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);

        status_t           Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);
        status_t           ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel);

        bool               IsTarArchive(const char *filePath) const;

        virtual BList      HiddenColumns(BList* columnList) const;
        virtual bool       CanReplaceFiles() const;
        virtual bool       CanPartiallyOpen() const;

    private:
        status_t           InitBinaryPath();

        char               m_tarPath[B_PATH_NAME_LENGTH];
};

#endif /* _TAR_ARCHIVER_H */
