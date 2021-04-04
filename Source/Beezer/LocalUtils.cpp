// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Application.h>
#include <Autolock.h>
#include <Bitmap.h>
#include <Debug.h>
#include <Entry.h>
#include <Handler.h>
#include <Locker.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Mime.h>
#include <Path.h>
#include <Resources.h>
#include <Roster.h>
#include <String.h>
#include <View.h>

#include <stdio.h>
#include <stdlib.h>

#include "AppConstants.h"
#include "LocalUtils.h"
#include "Shared.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "LocalUtils"
#else
#define B_TRANSLATE(x) x
#define B_TRANSLATE_COMMENT(x, y) x
#endif


BLocker _local_utils_locker("_local_utils_lock", true);



bool OpenEntry(const char* tempDirPath, const char* entryPath, bool openWith)
{
    BString extractedFilePath;
    extractedFilePath << tempDirPath << "/";
    extractedFilePath << entryPath;

    // Construct a ref, since roster/tracker needs refs inorder to launch
    entry_ref ref;
    BEntry entry(extractedFilePath.String());
    if (entry.Exists() == false)
        return false;
    entry.GetRef(&ref);

    // Update mime-type of the entry (sometimes mime isn't correct for newly extracted entries
    // in which case the Open-With operation will produce only the generic list of apps like
    // DiskProbe etc. -- Here we force updating of the mime-type which seems to work fine
    BPath pathOfEntry;
    entry.GetPath(&pathOfEntry);
    update_mime_info(pathOfEntry.Path(), true, true, true);

    // Take appropriate action, either open/open with...
    if (openWith == false)
    {
        status_t result = be_roster->Launch(&ref);

        // 0.06 -- bugfix: Now files are opened forcefully
        // Now viewing archives from Beezer when Beezer is preferred app will work
        if (result == B_BAD_VALUE)
        {
            entry_ref appRef;
            be_roster->FindApp(&ref, &appRef);
            team_id appID = be_roster->TeamFor(&appRef);
            BMessage openMsg(B_REFS_RECEIVED);
            openMsg.AddRef("refs", &ref);
            if (be_roster->IsRunning(&appRef))
                BMessenger(NULL, appID).SendMessage(&openMsg);
        }
    }
    else
        TrackerOpenWith(&ref);

    return true;
}



bool TrackerOpenWith(entry_ref* ref)
{
    // Pass message to Tracker to display the "Open With" dialog for the file
    BEntry entry(ref, true);         // Traverse the entry if it happens to be a link
    if (entry.Exists() == true)
    {
        if (entry.IsDirectory() == false)
        {
            // Update mime-type of the entry (sometimes mime isn't correct for newly extracted entries
            // in which case the Open-With operation will produce only the generic list of apps like
            // DiskProbe etc. -- Here we force updating of the mime-type which seems to work fine
            BPath pathOfEntry;
            entry.GetPath(&pathOfEntry);
            update_mime_info(pathOfEntry.Path(), true, true, true);

            BMessage trakMsg(B_REFS_RECEIVED);
            trakMsg.AddInt32("launchUsingSelector", 1L);
            trakMsg.AddRef("refs", ref);

            if (be_roster->IsRunning(K_TRACKER_SIGNATURE))
                BMessenger(K_TRACKER_SIGNATURE).SendMessage(&trakMsg);
        }
    }
    else
        return false;

    return true;
}



void TrackerOpenFolder(entry_ref* refToDir)
{
    BEntry entry(refToDir);
    if (entry.Exists() == true)
    {
        BMessage trakMsg(B_REFS_RECEIVED);
        trakMsg.AddRef("refs", refToDir);

        if (be_roster->IsRunning(K_TRACKER_SIGNATURE))
            BMessenger(K_TRACKER_SIGNATURE).SendMessage(&trakMsg);
    }
}



BBitmap* ResBitmap(const char* name, uint32 type)
{
    BAutolock autolocker(&_local_utils_locker);
    if (!autolocker.IsLocked())
        return NULL;

    // Only use for archived types, not raw types like PNG etc.
    size_t bmpSize;
    BMessage msg;
    const char* buf = NULL;

    buf = reinterpret_cast<const char*>(be_app->AppResources()->LoadResource(type, name, &bmpSize));
    if (!buf)
    {
        BString errString;
        errString << "error loading application resource: NAME=\"" << name << "\"";
        debugger(errString.String());
        return NULL;
    }

    if (msg.Unflatten(buf) != B_OK)
    {
        BString errString;
        errString << "error unflattening resource message: NAME=\"" << name << "\"";
        debugger(errString.String());
        return NULL;
    }

    return new BBitmap(&msg);
}



int64 BytesFromString(char* text)
{
    if (!text)
        return 0;

    // TODO: Fix this, use AppUtils::StringFromDigitalSize maybe.
    // Converts string to bytes, "text" needs to be 10.2 KB or 2 MB etc.
    char* end;
    double val;

    char* buffer = new char[strlen(text) + 1];
    strcpy(buffer, text);
    val = strtod(buffer, &end);

    if (strstr(buffer, "KiB"))
        val *= kKiBSize;
    else if (strstr(buffer, "MiB"))
        val *= kMiBSize;
    else if (strstr(buffer, "GiB"))
        val *= kGiBSize;
    else if (strstr(buffer, "TiB"))
        val *= kTiBSize;
    else if (strstr(buffer, "PiB"))
        val *= kPiBSize;
    else if (strstr(buffer, "EiB"))
        val *= kEiBSize;

    delete[] buffer;
    return (int64)val;
}



BString LocaleStringFromBytes(uint64 val)
{
    // TODO: Fix Redundant code.
    // Already exists in AppUtils, but including and using AppUtils from Beezer code-base
    // is ugly, incorrect and may lead to broken implementation when AppUtils change etc, so repeat it here
    // later find a way to merge this function into AppUtils or LocalUtils --

    // OK this has now shifted from FileSplitterWindow to LocalUtils because it is needed in FileJoinerWindow
    // as well, but currently this also exists in AppUtils.

    char buf[64];
    if (val < kKiBSize)
        sprintf(buf, "%" B_PRIu64 " bytes");
    else if (val < kMiBSize)
        sprintf(buf, "%.2f %s", (double)val / kKiBSize, B_TRANSLATE_COMMENT("KiB", "abbreviation of kilobyte"));
    else if (val < kGiBSize)
        sprintf(buf, "%.2f %s", (double)val / kMiBSize, B_TRANSLATE_COMMENT("MiB", "abbreviation of megabyte"));
    else if (val < kTiBSize)
        sprintf(buf, "%.2f %s", (double)val / kGiBSize, B_TRANSLATE_COMMENT("GiB", "abbreviation of gigabyte"));
    else if (val < kPiBSize)
        sprintf(buf, "%.2f %s", (double)val / kTiBSize, B_TRANSLATE_COMMENT("TiB", "abbreviation of terrabyte"));
    else if (val < kEiBSize)
        sprintf(buf, "%.2f %s", (double)val / kPiBSize, B_TRANSLATE_COMMENT("PiB", "abbreviation of petabyte"));
    else
        sprintf(buf, "%.2f %s", (double)val / kEiBSize, B_TRANSLATE_COMMENT("EiB", "abbreviation of exabyte"));

    BString str(buf);
    return str;
}



// Currently un-used, will use when we do dynamic column resizes
//float TruncSizeString (BString *result, BView *view, char *text, float width)
//{
//    const int64 kUnknownSize = -1;
//    const char *kSizeFormats[] =
//    {
//        "%.2g %s",
//        "%.1g %s",
//        "%.g %s",
//        "%.g%s",
//        0
//    };
//
//    int64 value = BytesFromString (text);
//    char buffer[1024];
//    if (value == kUnknownSize)
//    {
//        *result = "-";
//        return view->StringWidth ("-");
//    }
//    else if (value < kKBSize)
//    {
//        sprintf (buffer, "%Ld bytes", value);
//        if (view->StringWidth (buffer) > width)
//           sprintf (buffer, "%Ld B", value);
//    }
//    else
//    {
//        const char *suffix;
//        float floatValue;
//        if (value >= kTBSize)
//        {
//           suffix = "TiB";
//           floatValue = (float)value / kTBSize;
//        }
//        else if (value >= kGBSize)
//        {
//           suffix = "GiB";
//           floatValue = (float)value / kGBSize;
//        }
//        else if (value >= kMBSize)
//        {
//           suffix = "MiB";
//           floatValue = (float)value / kMBSize;
//        }
//        else
//        {
//           suffix = "KiB";
//           floatValue = (float)value / kKBSize;
//        }
//
//        for (int32 index = 0; ; index++)
//        {
//           if (!kSizeFormats[index])
//               break;
//
//           sprintf (buffer, kSizeFormats[index], floatValue, suffix);
//
////           // strip off an insignificant zero so we don't get readings such as 1.00
////           char *period = 0;
////           for (char *tmp = buffer; *tmp; tmp++)
////               if (*tmp == '.')
////                  period = tmp;
////
////           // move the rest of the string over the insignificant zero
////           if (period && period[1] && period[2] == '0')
////               for (char *tmp = &period[2]; *tmp; tmp++)
////                  *tmp = tmp[1];
//
//           float resultWidth = view->StringWidth (buffer);
//           if (resultWidth <= width)
//           {
//               *result = buffer;
//               return resultWidth;
//           }
//        }
//    }
//}



void SetTargetForMenuRecursive(BMenu* menu, BHandler* target)
{
    BAutolock autolocker(&_local_utils_locker);
    if (!autolocker.IsLocked())
        return;

    // Recursive descent into sub-menus and set all BMenuItems under "menu" to target "target" BHandler
    menu->SetTargetForItems(target);
    for (int32 i = 0; i < menu->CountItems(); i++)
    {
        BMenuItem* item = menu->ItemAt(i);
        item->SetTarget(target);
        BMenu* subMenu = item->Submenu();
        if (subMenu)
            SetTargetForMenuRecursive(subMenu, target);
    }
}


