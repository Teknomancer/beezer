/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
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
        Archiver            *Ark () const;
        thread_id            ThreadID () const;
        virtual status_t    InitArchiver (entry_ref *ref, char *mimeString);
        virtual status_t    InitArchiver (const char *name, bool popupErrors);
        virtual status_t    Open ();
        virtual status_t    Open (BMessage *message);
        virtual status_t    Test ();
        virtual status_t    Test (BMessage *message);
        virtual status_t    Count (BMessage *message);
        virtual status_t    Create (BMessage *message);
        
    protected:
        // Protected hooks
        const char            *MakeTempDirectory ();
    
        // Protected hooks (thread)
        static int32        _opener (void *arg);
        static int32        _tester (void *arg);
        static int32        _counter (void *arg);
        static int32        _adder (void *arg);
        
    private:
        // Private hooks
        void                CleanMessage (BMessage *message);
        
        // Private members
        Archiver            *m_archiver;
        BPath                m_archivePath;
        BEntry                m_archiveEntry;
        char                *m_tempDirPath;
        BDirectory            *m_tempDir;
        thread_id            m_thread;
};

#endif /* _ARCHIVE_REP_H */
