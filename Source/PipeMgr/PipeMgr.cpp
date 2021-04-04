// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "PipeMgr.h"

#include <image.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>



PipeMgr::PipeMgr()
{
}



PipeMgr::~PipeMgr()
{
    FlushArgs();
}



void PipeMgr::FlushArgs()
{
    int32 count = m_argList.CountItems();
    for (int32 i = 0; i < count; i++)
        free(m_argList.RemoveItem((int32)0));
    m_argList.MakeEmpty();
}



status_t PipeMgr::AddArg(const char* arg)
{
    return (m_argList.AddItem(reinterpret_cast<void*>(strdup(arg))) == true ? B_OK : B_ERROR);
}



thread_id PipeMgr::Pipe(int* outdes, int* errdes) const
{
    int oldstdout;
    int oldstderr;
    pipe(outdes);
    pipe(errdes);
    oldstdout = dup(STDOUT_FILENO);
    oldstderr = dup(STDERR_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    dup2(outdes[1], STDOUT_FILENO);
    dup2(errdes[1], STDERR_FILENO);

    // Construct the argv vector
    int32 argc = m_argList.CountItems();
    const char** argv = (const char**)malloc((argc + 1) * sizeof(char*));
    for (int32 i = 0; i < argc; i++)
        argv[i] = (const char*)m_argList.ItemAtFast(i);
    argv[argc] = NULL;

    // Load the app image...
    thread_id appThread = load_image((int)argc, argv, const_cast<const char**>(environ));

    dup2(oldstdout, STDOUT_FILENO);
    dup2(oldstderr, STDERR_FILENO);
    close(oldstdout);
    close(oldstderr);

    return appThread;
}



status_t PipeMgr::Pipe(int* outdes) const
{
    int errdes[2];
    status_t tid = Pipe(outdes, errdes);
    close(errdes[0]);
    close(errdes[1]);
    return tid;
}



void PipeMgr::Pipe() const
{
    int outdes[2], errdes[2];
    status_t exitCode;
    wait_for_thread(Pipe(outdes, errdes), &exitCode);

    close(outdes[0]);
    close(outdes[1]);
    close(errdes[0]);
    close(errdes[1]);
}



PipeMgr& PipeMgr::operator << (const char* arg)
{
    AddArg(arg);
    return *this;
}



PipeMgr& PipeMgr::operator << (BString arg)
{
    AddArg(arg.String());
    return *this;
}



void PipeMgr::PrintToStream() const
{
    for (int32 i = 0L; i < m_argList.CountItems(); i++)
        printf("%s ", (char*)m_argList.ItemAtFast(i));
    printf("\n");
}


