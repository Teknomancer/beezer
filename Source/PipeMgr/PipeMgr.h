// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _PIPE_MGR_H
#define _PIPE_MGR_H

#include <OS.h>
#include <List.h>
#include <String.h>
#include <SupportDefs.h>

class PipeMgr
{
    public:
        PipeMgr();
        virtual ~PipeMgr();

        // Additional hooks
        void               FlushArgs();
        status_t           AddArg(const char* argv);
        void               Pipe() const;
        thread_id          Pipe(int* outdes) const;
        thread_id          Pipe(int* outdes, int* errdes) const;
        void               PrintToStream() const;

        // Custom operators
        PipeMgr& operator   << (const char* arg);
        PipeMgr& operator   << (BString arg);

    protected:
        // Protected members
        BList              m_argList;
};

#endif /* _PIPE_MGR_H */
