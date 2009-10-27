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

#ifndef _ARCHIVE_REP_H
#define _ARCHIVE_REP_H

#include <SupportDefs.h>

#include "ArchiverMgr.h"

class BPath;
class BDirectory;
class BEntry;

class ArchiveRep
{
    public:
        ArchiveRep ();
        virtual ~ArchiveRep ();
        
        // Additional Hooks
        Archiver           *Ark () const;
        thread_id           ThreadID () const;
        virtual status_t     InitArchiver (entry_ref *ref, char *mimeString);
        virtual status_t     InitArchiver (const char *name, bool popupErrors);
        virtual status_t     Open ();
        virtual status_t     Open (BMessage *message);
        virtual status_t     Test ();
        virtual status_t     Test (BMessage *message);
        virtual status_t     Count (BMessage *message);
        virtual status_t     Create (BMessage *message);
        
    protected:
        // Protected hooks
        const char         *MakeTempDirectory ();
    
        // Protected hooks (thread)
        static int32        _opener (void *arg);
        static int32        _tester (void *arg);
        static int32        _counter (void *arg);
        static int32        _adder (void *arg);
        
    private:
        // Private hooks
        void                CleanMessage (BMessage *message);
        
        // Private members
        Archiver           *m_archiver;
        BPath               m_archivePath;
        BEntry              m_archiveEntry;
        char               *m_tempDirPath;
        BDirectory         *m_tempDir;
        thread_id           m_thread;
};

#endif /* _ARCHIVE_REP_H */
