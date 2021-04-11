// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Alert.h>
#include <Directory.h>
#include <Entry.h>
#include <Mime.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Window.h>

#include <malloc.h>

#include "ArchiveRep.h"
#include "Archiver.h"
#include "ArchiverMgr.h"
#include "BeezerApp.h"
#include "BitmapPool.h"
#include "FSUtils.h"
#include "MsgConstants.h"


ArchiveRep::ArchiveRep()
{
    m_tempDir = NULL;
    m_archiver = NULL;
}


ArchiveRep::~ArchiveRep()
{
    if (m_archiver)
        delete m_archiver;

    if (m_tempDir)
    {
        RemoveDirectory(m_tempDir);
        delete m_tempDir;
        free((char*)m_tempDirPath);
    }
}


status_t ArchiveRep::InitArchiver(const char* name, bool popupErrors)
{
    status_t result;
    m_archiver = NewArchiver(name, popupErrors, &result);

    if (m_archiver)
    {
        m_archiver->SetIconList(&(_glob_bitmap_pool->m_iconList));
        m_archiver->SetSettingsDirectoryPath((_bzr()->m_settingsPathStr).String());
        if (m_archiver->NeedsTempDirectory())
            m_archiver->SetTempDirectoryPath(MakeTempDirectory());

        m_archiver->LoadSettingsMenu();
    }
    return result;
}


status_t ArchiveRep::InitArchiver(entry_ref* ref, char* mimeString)
{
    // Initialise archiver based either on ref, or on the passed-in mimeString
    char type[B_MIME_TYPE_LENGTH];
    if (ref)
    {
        m_archivePath.SetTo(ref);
        m_archiveEntry.SetTo(ref);

        if (!mimeString)
        {
            update_mime_info(m_archivePath.Path(), false, true, false);
            BNode node(&m_archiveEntry);
            BNodeInfo nodeInfo(&node);
            nodeInfo.GetType(type);
        }
        else
            strcpy(type, mimeString);
    }
    else if (mimeString)
    {
        strcpy(type, mimeString);
    }

    status_t errCode = BZR_ERROR;
    m_archiver = ArchiverForMime(type);

    if (m_archiver == NULL)        // Handle unsupported types
        return errCode;

    if ((errCode = m_archiver->InitCheck()) != B_OK)    // Type is supported,
        return errCode;

    if (m_archiver)
    {
        m_archiver->SetIconList(&(_glob_bitmap_pool->m_iconList));
        m_archiver->SetSettingsDirectoryPath((_bzr()->m_settingsPathStr).String());
        if (m_archiver->NeedsTempDirectory())
            m_archiver->SetTempDirectoryPath(MakeTempDirectory());

        m_archiver->LoadSettingsMenu();
    }

    return errCode;
}


const char* ArchiveRep::MakeTempDirectory()
{
    if (m_tempDir == NULL)
        m_tempDirPath = strdup(CreateTempDirectory(NULL, &m_tempDir, true).String());

    return m_tempDirPath;
}


Archiver* ArchiveRep::Ark() const
{
    return m_archiver;
}


thread_id ArchiveRep::ThreadID() const
{
    return m_thread;
}


void ArchiveRep::CleanMessage(BMessage* message)
{
    // Backward compatibility: remove old fields and add-in our new ones
    message->RemoveName(kArchiverPtr);
    message->AddPointer(kArchiverPtr, (void*)m_archiver);
}


status_t ArchiveRep::Open(BMessage* message)
{
    CleanMessage(message);
    m_thread = spawn_thread(_opener, "_opener", B_NORMAL_PRIORITY, (void*)message);
    resume_thread(m_thread);
    return B_OK;
}


status_t ArchiveRep::Open()
{
    entry_ref ref;
    m_archiveEntry.GetRef(&ref);
    return m_archiver->Open(&ref);
}


status_t ArchiveRep::Test()
{
    char* output = NULL;
    return m_archiver->Test(output, NULL, NULL);
}


status_t ArchiveRep::Test(BMessage* message)
{
    CleanMessage(message);
    m_thread = spawn_thread(_tester, "_tester", B_NORMAL_PRIORITY, (void*)message);
    resume_thread(m_thread);
    return B_OK;
}


status_t ArchiveRep::Count(BMessage* message)
{
    CleanMessage(message);
    m_thread = spawn_thread(_counter, "_counter", B_NORMAL_PRIORITY, (void*)message);
    resume_thread(m_thread);

    return B_OK;
}


status_t ArchiveRep::Create(BMessage* message)
{
    CleanMessage(message);
    message->RemoveName(kCreateMode);
    message->AddBool(kCreateMode, true);
    m_thread = spawn_thread(_adder, "_creator", B_NORMAL_PRIORITY, (void*)message);
    resume_thread(m_thread);
    return B_OK;
}


int32 ArchiveRep::_opener(void* arg)
{
    // Open the archive and post message to calling looper
    BLooper* looper = NULL;
    Archiver* ark = NULL;
    entry_ref ref;

    BMessage* msg = reinterpret_cast<BMessage*>(arg);
    msg->FindPointer(kLooperPtr, reinterpret_cast<void**>(&looper));
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindRef(kRef, &ref);

    status_t result = ark->Open(&ref);
    delete msg;

    BMessage backMessage(M_OPEN_PART_TWO);
    backMessage.AddInt32(kResult, result);
    looper->PostMessage(&backMessage);
    return result;
}


int32 ArchiveRep::_tester(void* arg)
{
    // Test the archive against errors, report back to calling looper
    volatile bool* cancel;
    BLooper* looper = NULL;
    Archiver* ark = NULL;
    BMessenger messenger;
    BMessage* msg = reinterpret_cast<BMessage*>(arg);

    msg->FindPointer(kLooperPtr, reinterpret_cast<void**>(&looper));
    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindMessenger(kProgressMessenger, &messenger);

    char* outputStr = NULL;
    status_t result = ark->Test(outputStr, &messenger, cancel);

    messenger.SendMessage(M_CLOSE);
    BMessage backMessage(M_TEST_DONE);
    backMessage.AddInt32(kResult, result);
    if (outputStr)
        backMessage.AddString(kText, outputStr);

    delete msg;
    looper->PostMessage(&backMessage);

    return result;
}


int32 ArchiveRep::_counter(void* arg)
{
    // Thread that does getting information about a directory (files, folders...)
    BLooper* looper(NULL);
    BWindow* statusWnd(NULL);
    volatile bool* cancel;

    BMessage* message = reinterpret_cast<BMessage*>(arg);
    message->FindPointer(kLooperPtr, reinterpret_cast<void**>(&looper));
    message->FindPointer(kCancel, (void**)&cancel);
    message->FindPointer(kStatusPtr, reinterpret_cast<void**>(&statusWnd));

    uint32 type;
    int32 count;
    entry_ref ref;
    message->GetInfo("refs", &type, &count);
    if (type != B_REF_TYPE)
        return B_ERROR;

    int32 fileCount(0), folderCount(0);
    off_t totalSize(0);
    for (int32 i = --count; i >= 0; i--)
        if (message->FindRef("refs", i, &ref) == B_OK)
        {
            BEntry entry(&ref, false);         // Do NOT traverse links
            if (entry.IsDirectory() == true)
                GetDirectoryInfo(&entry, fileCount, folderCount, totalSize, cancel);
            else
            {
                off_t size;
                entry.GetSize(&size);
                totalSize += size;
                fileCount++;
            }
        }

    // Remove the following fields from message as they will be re-add (when message is re-used)
    // and that time the Add... WILL fail unless we remove it
    message->RemoveName(kLooperPtr);
    message->RemoveName(kCancel);

    // Don't delete message here as it doesn't belong to us! SPECIAL CASE
    message->what = M_COUNT_COMPLETE;                  // See we are reusing the message
    message->AddInt32(kFiles, fileCount);                // Add the critical fields as that is what
    message->AddInt32(kFolders, folderCount);            // we are here for in the first place
    message->AddInt64(kSize, totalSize);
    if (statusWnd)
        statusWnd->PostMessage(M_CLOSE);

    if (*cancel == false)
        looper->PostMessage(message);

    return BZR_DONE;
}


int32 ArchiveRep::_adder(void* arg)
{
    // Thread that does adding of files to archive
    volatile bool* cancel;
    status_t result;
    BMessenger messenger;
    BMessage filesToAdd;
    BLooper* looper(NULL);
    Archiver* ark(NULL);
    BMessage* msg = reinterpret_cast<BMessage*>(arg);
    const char* relativePath(NULL);
    const char* archivePath(NULL);
    bool createMode;

    msg->FindPointer(kArchiverPtr, reinterpret_cast<void**>(&ark));
    msg->FindPointer(kLooperPtr, reinterpret_cast<void**>(&looper));
    msg->FindMessenger(kProgressMessenger, &messenger);
    msg->FindPointer(kCancel, (void**)&cancel);
    msg->FindString(kLaunchDir, &relativePath);
    msg->FindString(kArchivePath, &archivePath);

    BPath path(archivePath);

    if (msg->FindBool(kCreateMode, &createMode) != B_OK)
        createMode = false;

    BMessage newlyAddedPaths;
    if (createMode == true)
        result = ark->Create(&path, relativePath, msg, &newlyAddedPaths, &messenger, cancel);
    else
        result = ark->Add(false, relativePath, msg, &newlyAddedPaths, &messenger, cancel);

    msg->what = M_ADD_DONE;
    msg->RemoveName(kResult);
    msg->AddInt32(kResult, result);
    msg->RemoveName(kPath);
    msg->AddMessage(kFileList, &newlyAddedPaths);
    messenger.SendMessage(M_CLOSE);
    looper->PostMessage(msg);

    return result;
}
