/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _RAR_ARCHIVER_H
#define _RAR_ARCHIVER_H

#include <stdio.h>

#include "Archiver.h"

#define BZR_ARK                        "rar"
#define BZR_UNARK                      "unrar"

class BMessenger;

class RarArchiver : public Archiver
{
    public:
        RarArchiver (const char *binPath);
        
        // Overridables
        void               BuildDefaultMenu ();
        
        // Abstract Implementations & overridables
        status_t           Open (entry_ref *ref, BMessage *fileList);
        status_t           ReadOpen (FILE *fp);

        status_t           ReadExtract (FILE *fp, BMessenger *progress, volatile bool *cancel);
        status_t           Extract (entry_ref *dir, BMessage *list, BMessenger *progress, volatile bool *cancel);
        
        status_t           Test (char *&outputStr, BMessenger *progress, volatile bool *cancel);
        status_t           ReadTest (FILE *fp, char *&outputStr, BMessenger *progress, volatile bool *cancel);
        
        status_t           ReadAdd (FILE *fp, BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
        status_t           Add (bool createMode, const char *relPath, BMessage *list, BMessage *addedPaths,
                             BMessenger *progress, volatile bool *cancel);
    
        status_t           Create (BPath *archivePath, const char *relPath, BMessage *fileList,
                                BMessage *addedPaths, BMessenger *progress, volatile bool *cancel);
    
        status_t           Delete (char *&outputStr, BMessage *list, BMessenger *progress, volatile bool *cancel);

        status_t           GetComment (char *&commentStr);
        status_t           SetComment (char *commentStr, const char *tempDirPath);
        bool               SupportsComment () const;
        bool               CanPartiallyOpen () const;
        bool               NeedsTempDirectory () const;
        bool               SupportsPassword () const;
        bool               CanDeleteFiles () const;
        
    private:    
        void               SetMimeType();

        char               m_rarPath[B_PATH_NAME_LENGTH];
        char               m_unrarPath[B_PATH_NAME_LENGTH];
};

#endif /* _RAR_ARCHIVER_H */
