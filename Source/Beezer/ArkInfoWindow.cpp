// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <time.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Button.h>
#include <DateTimeFormat.h>
#include <Entry.h>
#include <GridLayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#include <List.h>
#include <NumberFormat.h>
#include <Path.h>
#include <StatusBar.h>
#include <String.h>
#include <StringView.h>

#include "AppConstants.h"
#include "AppUtils.h"
#include "Archiver.h"
#include "ArkInfoWindow.h"
#include "BitmapPool.h"
#include "CommonStrings.h"
#include "HashTable.h"
#include "ListEntry.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ArkInfoWindow"
#else
#define B_TRANSLATE(x) x
#endif


ArkInfoWindow::ArkInfoWindow(BWindow* callerWindow, Archiver* archiver, BEntry* entry)
    : BWindow(BRect(30, 30, 440, 280), B_TRANSLATE("Archive Information"), B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
              B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
    m_archiver(archiver),
    m_entry(entry)
{
    m_archiver->GetLists(m_fileList, m_dirList);

    // We could get the calling window by using LooperForThread() but that would not indicate to the caller
    // we are getting a pointer to his window
    if (callerWindow)
    {
        SetFeel(B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset(callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap* infoBmp = BitmapPool::LoadAppVector("Img:ArchiveInfo", 32, 32);

    StaticBitmapView* infoBmpView = new StaticBitmapView(BRect(K_MARGIN * 5, K_MARGIN * 2,
            infoBmp->Bounds().Width() + K_MARGIN * 5, infoBmp->Bounds().Height() + K_MARGIN * 2),
            "ArkInfoWindow:iconView", infoBmp);

    // Add the file name string view (align it vertically with the icon view)
    m_fileNameStr = new BStringView("ArkInfoWindow:FileNameView", "");
    BFont font(be_bold_font);
    font.SetSize(font.Size()+4);
    m_fileNameStr->SetFont(&font);

    m_compressRatioBar = new BStatusBar("ArkInfoWindow:CompressRatioBar", B_TRANSLATE("Compression ratio:"), NULL);
    m_compressRatioBar->SetBarHeight(K_PROGRESSBAR_HEIGHT);
    m_compressRatioBar->SetBarColor(K_PROGRESS_COLOR);
    m_compressRatioBar->SetMaxValue(100);

    BStringView* compressedSizeStr = new BStringView("ArkInfoWindow:_CompressedSizeStr", B_TRANSLATE("Compressed size:"));
    compressedSizeStr->SetAlignment(B_ALIGN_RIGHT);
    compressedSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_compressedSizeStr = new BStringView("ArkInfoWindow:CompressedSizeStr", "0 MiB (0 bytes)");
    m_compressedSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* originalSizeStr = new BStringView("ArkInfoWindow:_OriginalSizeStr", B_TRANSLATE("Original size:"));
    originalSizeStr->SetAlignment(B_ALIGN_RIGHT);
    originalSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_originalSizeStr = new BStringView("ArkInfoWindow:OriginalSizeStr", "0 MiB (0 bytes)");
    m_originalSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* fileCountStr = new BStringView("ArkInfoWindow:_FileCountStr", B_TRANSLATE("Number of files:"));
    fileCountStr->SetAlignment(B_ALIGN_RIGHT);
    fileCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_fileCountStr = new BStringView("ArkInfoWindow:FileCountStr", "0");
    m_fileCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* folderCountStr = new BStringView("ArkInfoWindow:_FolderCountStr", B_TRANSLATE("Number of folders:"));
    folderCountStr->SetAlignment(B_ALIGN_RIGHT);
    folderCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_folderCountStr = new BStringView("ArkInfoWindow:FolderCountStr", "0");
    m_folderCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* totalCountStr = new BStringView("ArkInfoWindow:_TotalCountStr", B_TRANSLATE("Total entries:"));
    totalCountStr->SetAlignment(B_ALIGN_RIGHT);
    totalCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_totalCountStr = new BStringView("ArkInfoWindow:TotalCountStr", "0");
    m_totalCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    // Other file infos like path, type, created, modified etc.
    BStringView* typeStr = new BStringView("ArkInfoWindow:_TypeStr", B_TRANSLATE("Type:"));
    typeStr->SetAlignment(B_ALIGN_RIGHT);
    typeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_typeStr = new BStringView("ArkInfoWindow:TypeStr", "-");
    m_typeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* pathStr = new BStringView("ArkInfoWindow:_PathStr", B_TRANSLATE("Path:"));
    pathStr->SetAlignment(B_ALIGN_RIGHT);
    pathStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_pathStr = new BStringView("ArkInfoWindow:PathStr", "/boot");
    m_pathStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* createdStr = new BStringView("ArkInfoWindow:_CreatedStr", B_TRANSLATE("Created on:"));
    createdStr->SetAlignment(B_ALIGN_RIGHT);
    createdStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_createdStr = new BStringView("ArkInfoWindow:CreatedStr", "");
    m_createdStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView* modifiedStr = new BStringView("ArkInfoWindow:_ModifiedStr", B_TRANSLATE("Last modified:"));
    modifiedStr->SetAlignment(B_ALIGN_RIGHT);
    modifiedStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_modifiedStr = new BStringView("ArkInfoWindow:ModifiedStr", "");
    m_modifiedStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BGridLayout* infoLayout = NULL;
    BGridLayout* detailLayout = NULL;

    AddChild(BGroupLayoutBuilder(B_VERTICAL)
             .AddGroup(B_HORIZONTAL)
             .Add(infoBmpView, 0.0f)
             .Add(m_fileNameStr, 1.0f)
             .AddGlue()
             .SetInsets(K_MARGIN, K_MARGIN, K_MARGIN, K_MARGIN)
             .End()
             .Add(m_compressRatioBar)
             .Add(infoLayout = BGridLayoutBuilder(B_USE_DEFAULT_SPACING, 5)
                  .Add(compressedSizeStr, 0, 0)
                  .Add(m_compressedSizeStr, 1, 0)
                  .Add(originalSizeStr, 0, 1)
                  .Add(m_originalSizeStr, 1, 1)
                  .Add(fileCountStr, 0, 2)
                  .Add(m_fileCountStr, 1, 2)
                  .Add(folderCountStr, 0, 3)
                  .Add(m_folderCountStr, 1, 3)
                  .Add(totalCountStr, 0, 4)
                  .Add(m_totalCountStr, 1, 4)
                  .SetColumnWeight(0, 0)
                 )
             .AddStrut(10)
             .Add(detailLayout = BGridLayoutBuilder(B_USE_DEFAULT_SPACING, 5)
                  .Add(typeStr, 0, 0)
                  .Add(m_typeStr, 1, 0)
                  .Add(pathStr, 0, 1)
                  .Add(m_pathStr, 1, 1)
                  .Add(createdStr, 0, 2)
                  .Add(m_createdStr, 1, 2)
                  .Add(modifiedStr, 0, 3)
                  .Add(m_modifiedStr, 1, 3)
                  .SetColumnWeight(0, 0)
                 )
             .SetInsets(4 * K_MARGIN, 2 * K_MARGIN, 4 * K_MARGIN, 2 * K_MARGIN)
            );

    detailLayout->AlignLayoutWith(infoLayout, B_HORIZONTAL);

    // Now that we have done w/ placing the cntrls, we will getinfo about the archive & write it to the cntrls
    FillDetails();

    if (callerWindow != NULL)
    {
        UpdateSizeLimits();
        BRect callerRect(callerWindow->Frame());
        BPoint windowPoint(callerRect.left + callerRect.Width()/2 - Bounds().Width()/2, callerRect.top + callerRect.Height()/2 - Bounds().Height()/2);
        MoveTo(windowPoint);
    }
    else
        CenterOnScreen();

    // Restore position from prefs (not size)
    BPoint pt;
    if (_prefs_windows.FindBoolDef(kPfArkInfoWnd, true))
        if (_prefs_windows.FindPoint(kPfArkInfoWndFrame, &pt) == B_OK)
            MoveTo(pt);

    Show();
}


bool ArkInfoWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef(kPfArkInfoWnd, true))
        _prefs_windows.SetPoint(kPfArkInfoWndFrame, Frame().LeftTop());

    return BWindow::QuitRequested();
}


void ArkInfoWindow::FillDetails()
{
    char nameBuf[B_FILE_NAME_LENGTH];
    if (m_entry->Exists() == false)
    {
        Hide();
        (new BAlert("Error", B_TRANSLATE("Operation failed. The archive is missing."), B_TRANSLATE(skCloseWindowString),
                    NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT))->Go();
        PostMessage(B_QUIT_REQUESTED);
    }

    m_entry->GetName(nameBuf);
    m_fileNameStr->SetText(nameBuf);

    BString buf;
    if (BNumberFormat().Format(buf, m_dirList->CountItems()) != B_OK)
        buf = "???";
    m_folderCountStr->SetText(buf);

    if (BNumberFormat().Format(buf, m_fileList->CountItems()) != B_OK)
        buf = "???";
    m_fileCountStr->SetText(buf);

    if (BNumberFormat().Format(buf, m_fileList->CountItems() + m_dirList->CountItems()) != B_OK)
        buf = "???";
    m_totalCountStr->SetText(buf);

    buf = ""; buf << m_archiver->ArchiveType() << " " << B_TRANSLATE_COMMENT("archive", "ex: zip archive, tar archive, ...");
    m_typeStr->SetText(buf);

    BEntry parent; BPath parentPath;
    m_entry->GetParent(&parent);
    parent.GetPath(&parentPath);
    m_pathStr->SetText(parentPath.Path());

    time_t modTime, crTime;
    m_entry->GetModificationTime(&modTime);
    m_entry->GetCreationTime(&crTime);

    BString dateStr;
    if (BDateTimeFormat().Format(dateStr, modTime, B_FULL_DATE_FORMAT, B_LONG_TIME_FORMAT) != B_OK)
        dateStr = "<Error formatting date>";
    m_modifiedStr->SetText(dateStr);

    if (BDateTimeFormat().Format(dateStr, crTime, B_FULL_DATE_FORMAT, B_LONG_TIME_FORMAT) != B_OK)
        dateStr = "<Error formatting date>";
    m_createdStr->SetText(dateStr);

    off_t compressedSize;
    m_entry->GetSize(&compressedSize);
    BString bytesStr;
    if (BNumberFormat().Format(bytesStr, (double)compressedSize) != B_OK)
        bytesStr = "???";
    buf = StringFromBytes(compressedSize);
    if (compressedSize >= 1024LL)
        buf << " (" << bytesStr << " " << B_TRANSLATE(skbytesString) << ")";

    m_compressedSizeStr->SetText(buf);

    // Compute the inflated size from archive entries
    int32 count = m_fileList->CountItems();
    off_t originalSize = 0L;
    for (int32 i = 0; i < count; i++)
    {
        ListEntry* item = ((HashEntry*)m_fileList->ItemAtFast(i))->m_clvItem;
        if (item)
            originalSize += item->m_length;
    }

    if (BNumberFormat().Format(bytesStr, (double)originalSize) != B_OK)
        bytesStr = "???";
    buf = StringFromBytes(originalSize);
    if (originalSize >= 1024LL)
        buf << " (" << bytesStr << " " << B_TRANSLATE(skbytesString) << ")";

    m_originalSizeStr->SetText(buf);

    // Compute compression ratio
    float ratio;
    ratio = (100 * (originalSize - compressedSize) / (double)originalSize);

    if (ratio < 0)        // For pure .tar files we cannot compute ratio at all, if so don't report -ve
        buf = "n/a";
    else
        buf.SetToFormat("%.1f%%", ratio);


    m_compressRatioBar->Update(ratio, NULL, buf);
}
