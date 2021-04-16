// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2014 Chris Roberts.
// All rights reserved.

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

#include "PipeMgr.h"
#include "Shared.h"

#include <List.h>
#include <Message.h>
#include <Entry.h>
#include <Path.h>

#include <cstdio>

class HashTable;
class HashEntry;

class BBitmap;
class BMenu;


// ignore the warnings if this is an add-on without a compression menu
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static const char* const kCompressionLevelKey = "bzr:compression_level";
#pragma GCC diagnostic pop


class Archiver
{
    public:
        Archiver();
        Archiver(const char* addonImagePath);
        virtual ~Archiver();

        // Some must-call functions from the main app (to pass us some info)
        void                SetSettingsDirectoryPath(const char* path);
        void                SetIconList(BList* list);
        void                SetFoldingLevel(int8 level);

        status_t            InitCheck() const;
        HashTable*          Table() const;
        BList*              MimeTypeList() const;
        BMessage*           ErrorMessage() const;
        void                GetListFromArchive(BList& fileList, BList& folderList) const;
        void                SetArchiveType(const char* type);
        const char*         ArchiveType() const;
        void                SetArchiveExtension(const char* extension);
        const char*         ArchiveExtension() const;
        void                GetLists(BList*& fileList, BList*& folderList) const;
        void                FillLists(BList* fileList = NULL, BList* dirList = NULL);
        BMenu*              SettingsMenu() const;
        void                SaveSettingsMenu();
        void                LoadSettingsMenu();
        void                SetTempDirectoryPath(const char* path);
        const char*         TempDirectoryPath() const;
        void                SetPassword(const char* password);
        BString             Password() const;

        // Optionally-overridable functions
        virtual status_t    ArchiveSettings(BMessage& message);
        virtual status_t    ArchiveMenu(BMenu* menu, BMessage& message);
        virtual status_t    LoadMetaData(const char* addonImagePath);
        virtual BMessage*   GetRulesMessage();
        virtual void        BuildMenu(BMessage& message);
        virtual BList       HiddenColumns(BList* columnList) const;
        virtual bool        SupportsComment() const;
        virtual bool        SupportsFolderEntity() const;
        virtual bool        CanReplaceFiles() const;
        virtual bool        CanPartiallyOpen() const;
        virtual bool        CanDeleteFiles() const;
        virtual bool        CanAddEmptyFolders() const;
        virtual bool        CanAddFiles() const;
        virtual bool        SupportsPassword() const;
        virtual bool        PasswordRequired() const;
        virtual bool        NeedsTempDirectory() const;
        virtual status_t    GetComment(char*& commentStr);
        virtual status_t    SetComment(char* commentStr, const char* tempDirPath);

        // Helper functions
        virtual status_t    ReadErrStream(FILE* fp, const char* escapeLine = NULL);
        virtual void        ReadStream(FILE* fp, BString& str) const;
        bool                GetBinaryPath(char* destPath, const char* binaryFileName) const;

        // Abstract functions
        virtual status_t    Open(entry_ref* ref, BMessage* fileList = NULL) = 0;
        virtual status_t    Test(char*& output, BMessenger* progressMsngr, volatile bool* cancel) = 0;
        virtual status_t    Create(BPath* archivePath, const char* relativePath, BMessage* fileList,
                                   BMessage* addedPaths, BMessenger* progressMsngr, volatile bool* cancel) = 0;
        virtual status_t    Add(bool craeteMode, const char* relativePath, BMessage* fileList,
                                BMessage* addedPaths, BMessenger* progressMsngr, volatile bool* cancel) = 0;
        virtual status_t    Delete(char*& output, BMessage* files, BMessenger* progress, volatile bool* cancel) = 0;
        virtual status_t    Extract(entry_ref* destDir, BMessage* fileList, BMessenger* progressMsngr,
                                    volatile bool* cancel) = 0;

    protected:
        void                TerminateThread(thread_id tid) const;
        void                FormatDate(char* dateStr, size_t bufSize, struct tm* timeStruct);
        void                MakeTime(struct tm* timeStruct, time_t* timeValue, const char* day, const char* month,
                                     const char* year, const char* hour, const char* min, const char* sec);
        time_t              ArchiveModificationTime() const;
        int32               GetCompressionLevel(BMenu* menu = NULL);
        status_t            SetCompressionLevel(int32 level);

        const char*         m_typeStr,
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
        BMenu*              m_settingsMenu,
                            *m_compressionMenu;
        PipeMgr             m_pipeMgr;

    private:
        void                Init();
        static int          CompareHashEntries(const void* a, const void* b);
        void                AddDirPathToTable(BList* dirList, const char* path);
        HashEntry*          AddFilePathToTable(BList* fileList, const char* path);
        void                ResetCache();
        BBitmap*            BitmapForExtension(const char* str) const;

        BMessage*           m_rulesMsg;
        HashTable*          m_hashTable;
        BString             m_password;
        const char*         m_cachedPath;
        BList               m_fileList,
                            m_folderList,
                           *m_iconList;
        BBitmap*            m_folderBmp,
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

extern "C" _BZR_IMPEXP Archiver* load_archiver(const char* addonImagePath);

#endif /* _ARCHIVER_H */
