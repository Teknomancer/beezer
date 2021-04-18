// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PIPE_MGR_H
#define _PIPE_MGR_H

#include <OS.h>
#include <List.h>

class BString;

class PipeMgr
{
    public:
        PipeMgr();
        virtual ~PipeMgr();

        void               FlushArgs();
        status_t           AddArg(const char* argv);
        void               Pipe() const;
        thread_id          Pipe(int* outdes) const;
        thread_id          Pipe(int* outdes, int* errdes) const;
        void               PrintToStream() const;

        PipeMgr& operator  << (const char* arg);
        PipeMgr& operator  << (BString const& arg);

    protected:
        BList              m_argList;
};

#endif /* _PIPE_MGR_H */
