/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <String.h>

#include <image.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "PipeMgr.h"

//=============================================================================================================//

PipeMgr::PipeMgr ()
{
}

//=============================================================================================================//

PipeMgr::~PipeMgr ()
{
    FlushArgs();
}

//=============================================================================================================//

void PipeMgr::FlushArgs ()
{
    int32 count = m_argList.CountItems();
    for (int32 i = 0; i < count; i++)
        free (m_argList.RemoveItem (0L));
    m_argList.MakeEmpty();
}

//=============================================================================================================//

status_t PipeMgr::AddArg (const char *arg)
{
    return (m_argList.AddItem (reinterpret_cast<void*>(strdup (arg))) == true ? B_OK : B_ERROR);
}

//=============================================================================================================//

thread_id PipeMgr::Pipe (int *outdes, int *errdes) const
{
    int oldstdout;
    int oldstderr;
    pipe (outdes);
    pipe (errdes);
    oldstdout = dup (STDOUT_FILENO);
    oldstderr = dup (STDERR_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);
    dup2 (outdes[1], STDOUT_FILENO);
    dup2 (errdes[1], STDERR_FILENO);
    
    // Construct the argv vector
    int32 argc = m_argList.CountItems();
    const char **argv = (const char**)malloc ((argc + 1) * sizeof(char*));
    for (int32 i = 0; i < argc; i++)
        argv[i] = (const char*)m_argList.ItemAtFast(i);
    argv[argc] = NULL;
    
    // Load the app image...
    thread_id appThread = load_image ((int)argc, argv, const_cast<const char**>(environ));

    dup2 (oldstdout, STDOUT_FILENO);
    dup2 (oldstderr, STDERR_FILENO);
    close (oldstdout);
    close (oldstderr);

    return appThread;
}

//=============================================================================================================//

status_t PipeMgr::Pipe (int *outdes) const
{
    int errdes[2];
    status_t tid = Pipe (outdes, errdes);
    close (errdes[0]);
    close (errdes[1]);
    return tid;
}

//=============================================================================================================//

void PipeMgr::Pipe () const
{
    int outdes[2], errdes[2];
    status_t exitCode;
    wait_for_thread (Pipe (outdes, errdes), &exitCode);

    close (outdes[0]);
    close (outdes[1]);
    close (errdes[0]);
    close (errdes[1]);
}

//=============================================================================================================//

PipeMgr& PipeMgr::operator << (const char *arg)
{
    AddArg (arg);
    return *this;
}

//=============================================================================================================//

PipeMgr& PipeMgr::operator << (BString arg)
{
    AddArg (arg.String());
    return *this;
}

//=============================================================================================================//

void PipeMgr::PrintToStream () const
{
    for (int32 i = 0L; i < m_argList.CountItems(); i++)
        printf ("%s ", (char*)m_argList.ItemAtFast(i));
    printf ("\n");
}

//=============================================================================================================//
