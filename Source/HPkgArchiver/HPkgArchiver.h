// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _HPKG_ARCHIVER_H
#define _HPKG_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class HPkgArchiver : public Archiver
{
    public:
        HPkgArchiver(const char* addonImagePath);

        // Overridables
        void               BuildDefaultMenu();

        // Abstract Implementations & overridables
        status_t           Open(entry_ref* ref, BMessage* fileList);
        status_t           ReadOpen(FILE* fp, const char *metaInfo);

        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           Extract(entry_ref* dir, BMessage* list, BMessenger* progress, volatile bool* cancel);

        status_t           Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t           ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel);

        status_t           Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                               BMessenger* progress, volatile bool* cancel);

        status_t           Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                                  BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);

        status_t           Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);

    private:
        char               m_hpkgPath[B_PATH_NAME_LENGTH];
};

#endif /* _HPKG_ARCHIVER_H */
