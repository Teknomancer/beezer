// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "ArchiverMgr.h"
#include "Archiver.h"
#include "BeezerApp.h"
#include "CommonStrings.h"
#include "MsgConstants.h"
#include "RuleMgr.h"

#include <Alert.h>
#include <Autolock.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Resources.h>


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ArchiverMgr"
#else
#define B_TRANSLATE(x) x
#endif

#include <cassert>

BLocker _ark_locker("_ark_mgr_lock", true);

static BMessage* gMetaDataMessage = NULL;


Archiver* InstantiateArchiver(const char* path)
{
    image_id addonID = load_add_on(path);
    if (addonID > 0L)
    {
        // Archiver loaded successfully
        Archiver *(*load_archiver)(const char* addonImagePath);
        if (get_image_symbol(addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
            return (*load_archiver)(path);
    }

    return NULL;
}


Archiver* ArchiverForMime(const char* mimeType)
{
    // Finds an archiver given its name (archiverType and name is the same, eg: zip, tar etc)
    if (gMetaDataMessage == NULL)
        return NULL;

    char* arkPath;
    for (int32 msgIdx = 0; gMetaDataMessage->GetInfo(B_MESSAGE_TYPE, msgIdx, &arkPath, NULL) == B_OK; msgIdx++)
    {
        BMessage arkMsg;
        if (gMetaDataMessage->FindMessage(arkPath, &arkMsg) != B_OK)
            continue;

        BMessage rulesMsg;
        if (arkMsg.FindMessage("rules", &rulesMsg) != B_OK)
            continue;

        char* foundMimeType;
        for (int32 mimeIdx = 0; rulesMsg.GetInfo(B_STRING_TYPE, mimeIdx, &foundMimeType, NULL) == B_OK; mimeIdx++)
        {
            if (strcmp(mimeType, foundMimeType) == 0)
                return InstantiateArchiver(arkPath);
        }
    }

    return NULL;
}


void FreeArchiverMetaData()
{
    delete gMetaDataMessage;
}


status_t LoadArchiverMetaData()
{
    // Load resource metadata from all of the add-ons and store it in metaMsg

    if (gMetaDataMessage != NULL)
        delete gMetaDataMessage;

    gMetaDataMessage = new BMessage();

    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return B_ERROR;

    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    BDirectory* archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();

    BEntry entry;
    while (archiversDir->GetNextEntry(&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath(&path);
        BResources res(path.Path());
        if (res.InitCheck() != B_OK)
            return B_ERROR;

        size_t dataSize;
        const void* resData = res.LoadResource(B_MESSAGE_TYPE, "ArchiverMetaData", &dataSize);
        if (resData == NULL)
            return B_ERROR;

        BMessage resMsg;
        if (resMsg.Unflatten((const char*)resData) != B_OK)
            return B_ERROR;

        gMetaDataMessage->AddMessage(path.Path(), &resMsg);
    }
    return B_OK;
}


status_t ArchiversInstalled(BList& arkTypeList, BList* extensionStrings)
{
    // Finds an archiver given its name (archiverType and name is the same, eg: zip, tar etc)
    if (gMetaDataMessage == NULL)
        return B_ERROR;

    char* arkPath;
    for (int32 msgIdx = 0; gMetaDataMessage->GetInfo(B_MESSAGE_TYPE, msgIdx, &arkPath, NULL) == B_OK; msgIdx++)
    {
        BMessage arkMsg;
        if (gMetaDataMessage->FindMessage(arkPath, &arkMsg) != B_OK)
            continue;

        BString arkDataStr;
        if (arkMsg.FindString("type_name", &arkDataStr) != B_OK)
            continue;

        arkTypeList.AddItem((void*)strdup(arkDataStr.String()));

        if (extensionStrings == NULL || arkMsg.FindString("default_extension", &arkDataStr) != B_OK)
            continue;

        extensionStrings->AddItem((void*)strdup(arkDataStr.String()));
    }

    return B_OK;
}


Archiver* ArchiverForType(const char* archiverType)
{
    // Finds an archiver given its name (archiverType and name is the same, eg: zip, tar etc)
    if (gMetaDataMessage == NULL)
        return NULL;

    char* arkPath;
    for (int32 msgIdx = 0; gMetaDataMessage->GetInfo(B_MESSAGE_TYPE, msgIdx, &arkPath, NULL) == B_OK; msgIdx++)
    {
        BMessage arkMsg;
        if (gMetaDataMessage->FindMessage(arkPath, &arkMsg) != B_OK)
            continue;

        BString arkTypeStr;
        if (arkMsg.FindString("type_name", &arkTypeStr) != B_OK)
            continue;

        if (arkTypeStr == archiverType)
            return InstantiateArchiver(arkPath);
    }

    return NULL;
}


status_t MergeArchiverRules(RuleMgr* ruleMgr)
{
    if (gMetaDataMessage == NULL)
        return B_ERROR;

    char* arkPath;
    for (int32 msgIdx = 0; gMetaDataMessage->GetInfo(B_MESSAGE_TYPE, msgIdx, &arkPath, NULL) == B_OK; msgIdx++)
    {
        BMessage arkMsg;
        if (gMetaDataMessage->FindMessage(arkPath, &arkMsg) != B_OK)
            continue;

        BMessage rulesMsg;
        if (arkMsg.FindMessage("rules", &rulesMsg) != B_OK)
            continue;

        char* mimeType;
        int32 count = 0;
        // iterate our loaded mime rules and add them to the rule manager
        for (int32 idx = 0; rulesMsg.GetInfo(B_STRING_TYPE, idx, &mimeType, NULL, &count) == B_OK; idx++)
        {
            for (int32 subidx = 0; subidx < count; subidx++)
            {
                const char* extension;
                rulesMsg.FindString(mimeType, subidx, &extension);
                ruleMgr->AddMimeRule(strdup(mimeType), strdup(extension));
            }
        }
    }

    return B_OK;
}


BPopUpMenu* BuildArchiveTypesMenu(BHandler* targetHandler, BList* arkExtensions)
{
    // targetHandler is where the message will be sent when an archive type is selected from the menu
    BPopUpMenu* arkTypePopUp = new BPopUpMenu("");
    BList arkTypes;
    ArchiversInstalled(arkTypes, arkExtensions);
    for (int32 i = 0; i < arkTypes.CountItems(); i++)
    {
        BMessage* clickMsg = new BMessage(M_ARK_TYPE_SELECTED);
        clickMsg->AddString(kText, (char*)arkExtensions->ItemAtFast(i));
        BMenuItem* arkTypeItem = new BMenuItem((char*)arkTypes.ItemAtFast(i), clickMsg);
        arkTypeItem->SetTarget(targetHandler);
        arkTypePopUp->AddItem(arkTypeItem);
    }

    return arkTypePopUp;
}


Archiver* NewArchiver(const char* name, bool popupErrors, status_t* returnCode)
{
    Archiver* ark = ArchiverForType(name);
    if (!ark)
    {
        *returnCode = B_ERROR;
        return ark;
    }

    status_t const result = ark->InitCheck();
    switch (result)
    {
        case BZR_BINARY_MISSING:
        {
            if (popupErrors)
            {
                (new BAlert("Error", B_TRANSLATE("Archiver binary missing. Cannot continue"),
                            BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL,
                            B_EVEN_SPACING, B_STOP_ALERT))->Go();
            }
            break;
        }

        case BZR_OPTIONAL_BINARY_MISSING:
        {
            if (popupErrors)
            {
                (new BAlert("Error", B_TRANSLATE("Optional binary missing. Some features may not be available"),
                            BZ_TR(kOKString), NULL, NULL, B_WIDTH_AS_USUAL,
                            B_EVEN_SPACING, B_INFO_ALERT))->Go();
            }
            break;
        }
    }

    *returnCode = result;
    return ark;
}
