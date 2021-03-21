/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011-2021, Chris Roberts
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



BString CommaFormatString(off_t num)
{
    // Taken from OpenTracker iirc.
    // Eg; if num is 2500 then its "2,500" is returned
    char numStr[256];
    sprintf(numStr, "%Ld", num);
    uint32 length = strlen(numStr);

    BString bytes;

    if (length >= 4)
    {
        uint32 charsTillComma = length % 3;
        if (charsTillComma == 0)
            charsTillComma = 3;

        uint32 numberIndex = 0;

        while (numStr[numberIndex])
        {
            bytes += numStr[numberIndex++];
            if (--charsTillComma == 0 && numStr[numberIndex])
            {
                bytes += ',';
                charsTillComma = 3;
            }
        }
    }
    else
        bytes = numStr;

    return bytes;
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

    // Converts string to bytes, "text" needs to be 10.2 KB or 2 MB etc.
    char* end;
    double val;

    char* buffer = new char[strlen(text) + 1];
    strcpy(buffer, text);
    val = strtod(buffer, &end);            // Bytes

    if (strstr(buffer, "KiB"))                // KB
        val *= kKBSize;
    else if (strstr(buffer, "MiB"))            // MB
        val *= kMBSize;
    else if (strstr(buffer, "GiB"))            // GB
        val *= kGBSize;
    else if (strstr(buffer, "TiB"))            // TB
        val *= kTBSize;

    delete[] buffer;
    return (int64)val;
}



BString LocaleStringFromBytes(int64 v)
{
    // Redundant code !! Already exists in AppUtils, but including and using AppUtils from Beezer code-base
    // is ugly, incorrect and may lead to broken implementation when AppUtils change etc, so repeat it here
    // later find a way to merge this function into AppUtils or LocalUtils --

    // OK this has now shifted from FileSplitterWindow to LocalUtils because it is needed in FileJoinerWindow
    // as well, but currently this also exists in AppUtils, maybe it's better to have 2 versions of the same
    // function for the reason mentioed above
    BString str;
    if (v > -1)
    {
        char buf[50];
        if (v > (1024LL * 1024LL * 1024LL * 1024LL))
            sprintf(buf, "%.2f %s", ((double)v) / (1024LL * 1024LL * 1024LL * 1024LL), B_TRANSLATE_COMMENT("TiB", "abbreviation of terrabyte"));
        else if (v > (1024LL * 1024LL * 1024LL))
            sprintf(buf, "%.2f %s", ((double)v) / (1024LL * 1024LL * 1024LL), B_TRANSLATE_COMMENT("GiB", "abbreviation of gigabyte"));
        else if (v > (1024LL * 1024LL))
            sprintf(buf, "%.2f %s", ((double)v) / (1024LL * 1024LL), B_TRANSLATE_COMMENT("MiB", "abbreviation of megabyte"));
        else if (v > (1024LL))
            sprintf(buf, "%.2f %s", ((double)v) / 1024LL, B_TRANSLATE_COMMENT("KiB", "abbreviation of kilobyte"));
        else
            sprintf(buf, "%Li %s", v, B_TRANSLATE("bytes"));

        str = buf;
    }
    else
        str = "-";

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


