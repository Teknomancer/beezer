// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _ZIP_ARCHIVER_H
#define _ZIP_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class ZipArchiver : public Archiver
{
    public:
        ZipArchiver(BMessage* metaDataMsg);

        // Overridables
        void               BuildMenu(BMessage& message);

        // Abstract Implementations & overridables
        status_t           Open(entry_ref* ref, BMessage* fileList);
        status_t           Extract(entry_ref* dir, BMessage* list, BMessenger* progress, volatile bool* cancel);
        status_t           Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t           Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                               BMessenger* progress, volatile bool* cancel);
        status_t           Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                                  BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);

        status_t           GetComment(char*& commentStr);
        status_t           SetComment(char* commentStr, const char* tempDirPath);
        bool               SupportsComment() const;
        bool               SupportsFolderEntity() const;

    private:
        status_t           ReadOpen(FILE* fp);
        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t           ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           ReadDelete(FILE* fp, char*& outputStr,    BMessenger* progress, volatile bool* cancel);

        char               m_unzipPath[B_PATH_NAME_LENGTH];
        char               m_zipPath[B_PATH_NAME_LENGTH];
        char               m_zipnotePath[B_PATH_NAME_LENGTH];
};

#endif /* _ZIP_ARCHIVER_H */
