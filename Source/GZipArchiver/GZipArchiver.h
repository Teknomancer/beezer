/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _GZIP_ARCHIVER_H
#define _GZIP_ARCHIVER_H

#include <stdio.h>

#include "TarArchiver.h"

#define BZR_ARK                            "gzip"

class BMessenger;

class GZipArchiver : public TarArchiver
{
    public:
        GZipArchiver (const char *binPath);
        
        // Overridables
        void                BuildDefaultMenu ();
        
        // Abstract Implementations & overridables
        status_t            Open (entry_ref *ref, BMessage *fileList);
        status_t            ReadOpen (FILE *fp);

        status_t            ReadExtract (FILE *fp, BMessenger *progress, volatile bool *cancel);
        status_t            Extract (entry_ref *dir, BMessage *list, BMessenger *progress, volatile bool *cancel);
        
        status_t            Test (char *&outputStr, BMessenger *progress, volatile bool *cancel);
        
        status_t            ReadAdd (FILE *fp, BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
        status_t            Add (bool createMode, const char *relPath, BMessage *list, BMessage *addedPaths,
                                BMessenger *progress, volatile bool *cancel);
    
        status_t            Create (BPath *archivePath, const char *relPath, BMessage *fileList,
                                BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
    
        status_t            Delete (char *&outputStr, BMessage *list, BMessenger *progress, volatile bool *cancel);
        status_t            ReadDelete (FILE *fp, char *&outputStr,    BMessenger *progress, volatile bool *cancel);
        
        bool                NeedsTempDirectory () const;
        bool                CanAddFiles () const;
        BList                HiddenColumns (BList *columns) const;
        BString                OutputFileName (const char *fullFileName) const;
        
    private:
        // Private helper functions
        void                CompressFromTemp ();
        BString                InitTarFilePath (char *fileName);
        void                SendProgressMessage (BMessenger *progress) const;
        
        char                m_gzipPath[B_PATH_NAME_LENGTH];
        char                m_tarFilePath[B_PATH_NAME_LENGTH];
        char                m_arkFilePath[B_PATH_NAME_LENGTH];
        bool                m_tarArk;
};

#endif /* _ZIP_ARCHIVER_H */
