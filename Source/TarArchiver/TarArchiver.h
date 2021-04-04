// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _TAR_ARCHIVER_H
#define _TAR_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class TarArchiver : public Archiver
{
    public:
        TarArchiver(const char *addonImagePath);

        // Abstract Implementations & overridables
        status_t           Open(entry_ref* ref, BMessage* fileList);
        status_t           Extract(entry_ref* dir, BMessage* list, BMessenger* progress, volatile bool* cancel);
        status_t           Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t           Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                               BMessenger* progress, volatile bool* cancel);
        status_t           Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                                  BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);

        bool               IsTarArchive(const char *filePath) const;
        virtual BList      HiddenColumns(BList* columnList) const;
        virtual bool       CanReplaceFiles() const;
        virtual bool       CanPartiallyOpen() const;

    private:
        status_t           ReadOpen(FILE* fp);
        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel);

        status_t           InitBinaryPath();

        char               m_tarPath[B_PATH_NAME_LENGTH];
};

#endif /* _TAR_ARCHIVER_H */
