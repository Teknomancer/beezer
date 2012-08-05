/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#include <Alert.h>
#include <Application.h>
#include <Autolock.h>
#include <Debug.h>
#include <List.h>
#include <MenuItem.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <image.h>

#include "Archiver.h"
#include "ArchiverMgr.h"
#include "Beezer.h"
#include "LangStrings.h"
#include "MsgConstants.h"

BLocker _ark_locker("_ark_mgr_lock", true);



Beezer* _bzr()
{
    return (Beezer*)be_app;
}



Archiver* ArchiverForMime(const char* mimeType)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory* archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();

    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry(&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath(&path);

        image_id addonID = load_add_on(path.Path());
        if (addonID > 0L)
        {
            // Archiver loaded successfully, now check if it supports the mimetype
            Archiver *(*load_archiver)();
            if (get_image_symbol(addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
            {
                Archiver* ark = (*load_archiver)();

                BList* mimeList = ark->MimeTypeList();
                int32 supportedMimeCount = mimeList->CountItems();

                for (int32 i = 0; i < supportedMimeCount; i++)
                {
                    const char* mimeString = reinterpret_cast<const char*>(mimeList->ItemAtFast(i));
                    if (strcmp(mimeString, mimeType) == 0)
                        return ark;
                }
            }

            unload_add_on(addonID);
        }
    }

    return NULL;
}



BList ArchiversInstalled(BList* extensionStrings)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BList installedArkList;
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return installedArkList;

    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory* archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();

    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry(&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath(&path);

        image_id addonID = load_add_on(path.Path());
        if (addonID > 0L)
        {
            // Archiver loaded successfully, now check if it supports the mimetype
            Archiver *(*load_archiver)();
            if (get_image_symbol(addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
            {
                Archiver* ark = (*load_archiver)();
                installedArkList.AddItem((void*)strdup(ark->ArchiveType()));
                if (extensionStrings)
                    extensionStrings->AddItem((void*)strdup(ark->ArchiveExtension()));
            }

            unload_add_on(addonID);
        }
    }

    return installedArkList;
}



Archiver* ArchiverForType(const char* archiverType)
{
    // Finds an archiver given its name (archiverType and name is the same, eg: zip, tar etc)

    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    // Get the Archiver dir and the path of the Binaries dir (both from _bzr())
    // Bug Fix: we dont ask the Window to pass these details to us anymore
    BDirectory* archiversDir = &(_bzr()->m_addonsDir);
    archiversDir->Rewind();

    // Load/Unload all the add-ons and check which archiver supports the type
    BEntry entry;
    while (archiversDir->GetNextEntry(&entry, true) == B_OK)
    {
        BPath path;
        entry.GetPath(&path);

        image_id addonID = load_add_on(path.Path());
        if (addonID > 0L)
        {
            // Archiver loaded successfully, now check if it supports the mimetype
            Archiver *(*load_archiver)();
            if (get_image_symbol(addonID, kLoaderFunc, B_SYMBOL_TYPE_TEXT, (void**)&load_archiver) == B_OK)
            {
                Archiver* ark = (*load_archiver)();
                if (strcmp(ark->ArchiveType(), archiverType) == 0)
                    return ark;
            }

            unload_add_on(addonID);
        }
    }
    return NULL;
}



BPopUpMenu* BuildArchiveTypesMenu(BHandler* targetHandler, BList* arkExtensions)
{
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    // targetHandler is where the message will be sent when an archive type is selected from the menu
    BPopUpMenu* arkTypePopUp = new BPopUpMenu("");
    BList arkTypes = ArchiversInstalled(arkExtensions);
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
    // Operate in a critical section as we access global data like BDirectory of be_app
    BAutolock autoLocker(_ark_locker);
    if (autoLocker.IsLocked() == false)
        return NULL;

    Archiver* ark = ArchiverForType(name);
    if (!ark)
    {
        *returnCode = B_ERROR;
        return ark;
    }

    status_t result = ark->InitCheck();
    switch (result)
    {
        case BZR_BINARY_MISSING:
        {
            if (popupErrors)
            {
                (new BAlert("error", str(S_BINARY_MISSING), str(S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
                            B_EVEN_SPACING, B_STOP_ALERT))->Go();
            }
            break;
        }

        case BZR_OPTIONAL_BINARY_MISSING:
        {
            if (popupErrors)
            {
                (new BAlert("error", str(S_OPTIONAL_BINARY_MISSING), str(S_OK), NULL, NULL,
                            B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT))->Go();
            }
            break;
        }
    }

    *returnCode = result;
    return ark;
}


