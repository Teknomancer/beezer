// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _BZIP_ARCHIVER_H
#define _BZIP_ARCHIVER_H

#include "TarArchiver.h"

class BMessenger;

class BZipArchiver : public TarArchiver
{
    public:
        BZipArchiver(const char* addonImagePath);

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

        bool               CanAddFiles() const;
        bool               NeedsTempDirectory() const;
        BList              HiddenColumns(BList* columns) const;
        BString            OutputFileName(const char* fullFileName) const;

    private:
        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           ReadDelete(FILE* fp, char*& outputStr,    BMessenger* progress, volatile bool* cancel);

        void               CompressFromTemp();
        BString            InitTarFilePath(char* fileName);
        void               SendProgressMessage(BMessenger* progress) const;

        char               m_bzipPath[B_PATH_NAME_LENGTH];
        char               m_tarFilePath[B_PATH_NAME_LENGTH];
        char               m_arkFilePath[B_PATH_NAME_LENGTH];
        bool               m_tarArk;
};

#endif /* _BZIP_ARCHIVER_H */
