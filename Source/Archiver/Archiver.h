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
 * -> Neither the name of the author nor the names of its contributors may
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

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#if __GNUC__ > 2
    #define _BZR_IMPEXP
#else
    #ifdef _BZR_BUILD
        #define _BZR_IMPEXP __declspec (dllexport)
    #else
        #define _BZR_IMPEXP __declspec (dllimport)
    #endif
#endif

#include <List.h>
#include <Message.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>

#include "PipeMgr.h"
#include "Shared.h"

class BBitmap;
class BMenu;

class HashTable;
class HashEntry;

class Archiver
{
    public:
        Archiver ();
        virtual ~Archiver ();

        // Some must-call functions from the main app (to pass us some info)
        void                SetSettingsDirectoryPath (const char *path);
        void                SetIconList (BList *list);
        void                SetFoldingLevel (int8 level);
        
        status_t            InitCheck () const;
        HashTable          *Table () const;
        BList              *MimeTypeList () const;
        BMessage           *ErrorMessage () const;
        void                GetListFromArchive (BList &fileList, BList &folderList) const;
        void                SetArchiveType (const char *type);
        const char         *ArchiveType () const;
        void                SetArchiveExtension (const char *extension);
        const char         *ArchiveExtension () const;
        void                GetLists (BList *&fileList, BList *&folderList) const;
        void                FillLists (BList *fileList = NULL, BList *dirList = NULL);
        BMenu              *SettingsMenu () const;
        void                SetSettingsMenu (BMenu *menu);
        void                SaveSettingsMenu ();
        void                LoadSettingsMenu ();
        void                SetTempDirectoryPath (const char *path);
        const char         *TempDirectoryPath () const;
        void                SetPassword (const char *password);
        BString             Password() const;
        
        // Optionally-overridable functions
        virtual void        BuildDefaultMenu ();
        virtual BList        HiddenColumns (BList *columnList) const;
        virtual bool        SupportsComment () const;
        virtual bool        SupportsFolderEntity () const;
        virtual bool        CanReplaceFiles () const;
        virtual bool        CanPartiallyOpen () const;
        virtual bool        CanDeleteFiles () const;
        virtual bool        CanAddEmptyFolders () const;
        virtual bool        CanAddFiles () const;
        virtual bool        SupportsPassword () const;
        virtual bool        PasswordRequired () const;
        virtual bool        NeedsTempDirectory () const;
        virtual status_t     GetComment (char *&commentStr);
        virtual status_t     SetComment (char *commentStr, const char *tempDirPath);
        
        // Helper functions
        virtual status_t     ReadErrStream (FILE *fp, const char *escapeLine = NULL);
        virtual void        ReadStream (FILE *fp, BString &str) const;
        bool                IsBinaryFound (char *path, const char *binaryFileName) const;
        
        // Abstract functions
        virtual status_t     Open (entry_ref *ref, BMessage *fileList = NULL) = 0;
        virtual status_t     Test (char *&output, BMessenger *progressMsngr, volatile bool *cancel) = 0;
        virtual status_t     Create (BPath *archivePath, const char *relativePath, BMessage *fileList,
                                 BMessage *addedPaths, BMessenger *progressMsngr, volatile bool *cancel) = 0;
        virtual status_t     Add (bool craeteMode, const char *relativePath, BMessage *fileList,
                               BMessage *addedPaths, BMessenger *progressMsngr, volatile bool *cancel) = 0;
        virtual status_t     Delete (char *&output, BMessage *files, BMessenger *progress, volatile bool *cancel) = 0;
        virtual status_t     Extract (entry_ref *destDir, BMessage *fileList, BMessenger *progressMsngr,
                                  volatile bool *cancel) = 0;
        
    protected:
        // Protected functions
        void                TerminateThread (thread_id tid) const;
        void                FormatDate (char *dateStr, size_t bufSize, struct tm *timeStruct);
        void                MakeTime (struct tm *timeStruct, time_t *timeValue, const char *day, const char *month,
                                   const char *year, const char *hour, const char *min, const char *sec);
        
        // Protected members
        const char         *m_typeStr,
                         *m_extensionStr,
                         *m_settingsLangStr,
                         *m_settingsDirectoryPath,
                         *m_tempDirPath;
        bool                m_passwordRequired;
        BList               m_entriesList,
                          m_mimeList;
        status_t            m_error;
        entry_ref           m_archiveRef;
        BPath               m_archivePath;
        BMessage            m_errorDetails;
        BMenu              *m_settingsMenu;
        PipeMgr             m_pipeMgr;
        
    private:
        // Private functions
        static int          CompareHashEntries (const void *a, const void *b);
        void                AddDirPathToTable (BList *dirList, const char *path);
        HashEntry          *AddFilePathToTable (BList *fileList, const char *path);
        void                ResetCache ();
        BBitmap            *BitmapForExtension (const char *str) const;
        
        // Private members
        HashTable          *m_hashTable;
        BString             m_password;
        const char         *m_cachedPath;
        BList               m_fileList,
                          m_folderList,
                         *m_iconList;
        BBitmap            *m_folderBmp,
                         *m_binaryBmp,
                         *m_htmlBmp,
                         *m_textBmp,
                         *m_sourceBmp,
                         *m_audioBmp,
                         *m_archiveBmp,
                         *m_packageBmp,
                         *m_pdfBmp,
                         *m_imageBmp;
        int8                m_foldingLevel;
};

extern "C" _BZR_IMPEXP Archiver* load_archiver ();

#endif /* _ARCHIVER_H */
