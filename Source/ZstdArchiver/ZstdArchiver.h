// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _ZSTD_ARCHIVER_H
#define _ZSTD_ARCHIVER_H

#include "TarArchiver.h"

class BMessenger;

class ZstdArchiver : public TarArchiver
{
    public:
        ZstdArchiver(BMessage* metaDataMsg);

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

        bool               NeedsTempDirectory() const;
        bool               CanAddFiles() const;
        BList              HiddenColumns(BList const& columns) const;
        BString            OutputFileName(const char* fullFileName) const;

    private:
        status_t           ReadOpen(FILE* fp);
        status_t           ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t           ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t           ReadDelete(FILE* fp, char*& outputStr,    BMessenger* progress, volatile bool* cancel);

        void               CompressFromTemp();
        BString            InitTarFilePath(char* fileName);
        void               SendProgressMessage(BMessenger* progress) const;

        char               m_zstdPath[B_PATH_NAME_LENGTH];
        char               m_tarFilePath[B_PATH_NAME_LENGTH];
        char               m_arkFilePath[B_PATH_NAME_LENGTH];
        bool               m_tarArk;
};

#endif /* _ZSTD_ARCHIVER_H */
