// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _ARJ_ARCHIVER_H
#define _ARJ_ARCHIVER_H

#include "Archiver.h"

class BMessenger;

class ArjArchiver : public Archiver
{
    public:
        ArjArchiver(BMessage* metaDataMsg);

        // Overridables
        void                BuildMenu(BMessage& message);

        // Abstract Implementations & overridables
        status_t            Open(entry_ref* ref, BMessage* fileList);
        status_t            Extract(entry_ref* dir, BMessage* list, BMessenger* progress, volatile bool* cancel);
        status_t            Test(char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t            Add(bool createMode, const char* relPath, BMessage* list, BMessage* addedPaths,
                                BMessenger* progress, volatile bool* cancel);
        status_t            Create(BPath* archivePath, const char* relPath, BMessage* fileList,
                                   BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t            Delete(char*& outputStr, BMessage* list, BMessenger* progress, volatile bool* cancel);

        BList               HiddenColumns(BList const& columns) const;
        bool                SupportsFolderEntity() const;
        bool                CanPartiallyOpen() const;
        bool                CanAddEmptyFolders() const;

    private:
        status_t            ReadOpen(FILE* fp);
        status_t            ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel);
        status_t            ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel);
        status_t            ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel);
        status_t            ReadDelete(FILE* fp, char*& outputStr,    BMessenger* progress, volatile bool* cancel);

        void                SetMimeType();

        char                m_arjPath[B_PATH_NAME_LENGTH];
};

#endif /* _ARJ_ARCHIVER_H */
