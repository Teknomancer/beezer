// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2024 Oscar Lesta.
// All rights reserved.

#ifndef _SQUASHFS_ARCHIVER_H
#define _SQUASHFS_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class SquashFSArchiver: public Archiver
{
    public:
                    SquashFSArchiver(BMessage* metaDataMsg);

        void        BuildMenu(BMessage& message);
        BList       HiddenColumns(BList const& columnList) const;

        status_t    Open(entry_ref* ref, BMessage* fileList);
        status_t    Extract(entry_ref* dir, BMessage* list, BMessenger* progress,
                            volatile bool* cancel);

        status_t    Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t    Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                        BMessenger* progress, volatile bool* cancel);
        status_t    Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                           BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t    Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);

        bool        CanAddFiles() const;
        bool        CanDeleteFiles() const;

    private:
        status_t    ReadOpen(FILE* fp);
        status_t    ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);

        char        m_unsquashfsPath[B_PATH_NAME_LENGTH];
};

#endif
