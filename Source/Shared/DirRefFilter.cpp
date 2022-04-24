// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Chris Roberts.
// All rights reserved.

#include "DirRefFilter.h"


// this is used to workaround Haiku bug #13721 where a file is sent if the user double clicks it
// we'll force the BFilePanel to only show directories and symlinks to them

DirRefFilter::DirRefFilter() {};


bool DirRefFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* /*st*/, const char* /*filetype*/)
{
    if (node->IsDirectory())
        return true;

    if (node->IsSymLink())
    {
        BEntry entry(ref, true);
        if (entry.InitCheck() == B_OK && entry.IsDirectory())
            return true;
    }

    return false;
}
