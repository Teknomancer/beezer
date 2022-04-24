// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Chris Roberts.
// All rights reserved.

#include <FilePanel.h>


class DirRefFilter : public BRefFilter
{
public:
    DirRefFilter();
    virtual bool Filter(const entry_ref* ref, BNode* node, struct stat_beos* st, const char* filetype);
};
