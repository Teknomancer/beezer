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

#include <time.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Button.h>
#include <Entry.h>
#include <GridLayoutBuilder.h>
#include <GroupLayoutBuilder.h>
#include <List.h>
#include <Path.h>
#include <SpaceLayoutItem.h>
#include <StatusBar.h>
#include <String.h>
#include <StringView.h>

#include "AppUtils.h"
#include "Archiver.h"
#include "ArkInfoWindow.h"
#include "HashTable.h"
#include "LangStrings.h"
#include "ListEntry.h"
#include "LocalUtils.h"
#include "Preferences.h"
#include "PrefsFields.h"
#include "StaticBitmapView.h"
#include "UIConstants.h"


ArkInfoWindow::ArkInfoWindow (BWindow *callerWindow, Archiver *archiver, BEntry *entry)
    : BWindow (BRect (30, 30, 440, 280), str (S_ARK_INFO_WINDOW_TITLE), B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
           B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
    m_archiver (archiver),
    m_entry (entry)
{
    m_archiver->GetLists (m_fileList, m_dirList);

    // We could get the calling window by using LooperForThread() but that would not indicate to the caller
    // we are getting a pointer to his window
    if (callerWindow)
    {
        SetFeel (B_MODAL_SUBSET_WINDOW_FEEL);
        AddToSubset (callerWindow);
    }

    SetLayout(new BGroupLayout(B_VERTICAL, 0));

    BBitmap *infoBmp = ResBitmap ("Img:ArchiveInfo");

    StaticBitmapView *infoBmpView = new StaticBitmapView (BRect (K_MARGIN * 5, K_MARGIN * 2,
                         infoBmp->Bounds().Width() + K_MARGIN * 5, infoBmp->Bounds().Height() + K_MARGIN * 2),
                                    "ArkInfoWindow:iconView", infoBmp);
    infoBmpView->SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));

    // Add the file name string view (align it vertically with the icon view)
    m_fileNameStr = new BStringView ("ArkInfoWindow:FileNameView", "");
    m_fileNameStr->SetFont (be_bold_font);

    m_compressRatioBar = new BStatusBar ("ArkInfoWindow:CompressRatioBar", str (S_COMPRESSION_RATIO), NULL);
    m_compressRatioBar->SetBarHeight (K_PROGRESSBAR_HEIGHT);
    m_compressRatioBar->SetBarColor (K_PROGRESS_COLOR);
    m_compressRatioBar->SetMaxValue (100);

    BStringView *compressedSizeStr = new BStringView ("ArkInfoWindow:_CompressedSizeStr", str (S_COMPRESSED_SIZE));
    compressedSizeStr->SetAlignment (B_ALIGN_RIGHT);
    compressedSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_compressedSizeStr = new BStringView ("ArkInfoWindow:CompressedSizeStr", "0 MB (0 bytes)");
    m_compressedSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *originalSizeStr = new BStringView ("ArkInfoWindow:_OriginalSizeStr", str (S_ORIGINAL_SIZE));
    originalSizeStr->SetAlignment (B_ALIGN_RIGHT);
    originalSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_originalSizeStr = new BStringView ("ArkInfoWindow:OriginalSizeStr","0 MB (0 bytes)");
    m_originalSizeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *fileCountStr = new BStringView ("ArkInfoWindow:_FileCountStr", str (S_NUMBER_OF_FILES));
    fileCountStr->SetAlignment (B_ALIGN_RIGHT);
    fileCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_fileCountStr = new BStringView ("ArkInfoWindow:FileCountStr", "0");
    m_fileCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *folderCountStr = new BStringView ("ArkInfoWindow:_FolderCountStr", str (S_NUMBER_OF_FOLDERS));
    folderCountStr->SetAlignment (B_ALIGN_RIGHT);
    folderCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_folderCountStr = new BStringView ("ArkInfoWindow:FolderCountStr", "0");
    m_folderCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *totalCountStr = new BStringView ("ArkInfoWindow:_TotalCountStr", str (S_TOTAL_ENTRIES));
    totalCountStr->SetAlignment (B_ALIGN_RIGHT);
    totalCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_totalCountStr = new BStringView ("ArkInfoWindow:TotalCountStr", "0");
    m_totalCountStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    // Other file infos like path, type, created, modified etc.
    BStringView *typeStr = new BStringView ("ArkInfoWindow:_TypeStr", str (S_TYPE));
    typeStr->SetAlignment (B_ALIGN_RIGHT);
    typeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_typeStr = new BStringView ("ArkInfoWindow:TypeStr", "-");
    m_typeStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *pathStr = new BStringView ("ArkInfoWindow:_PathStr", str (S_PATH));
    pathStr->SetAlignment (B_ALIGN_RIGHT);
    pathStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_pathStr = new BStringView ("ArkInfoWindow:PathStr", "/boot");
    m_pathStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *createdStr = new BStringView ("ArkInfoWindow:_CreatedStr", str (S_CREATED));
    createdStr->SetAlignment (B_ALIGN_RIGHT);
    createdStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_createdStr = new BStringView ("ArkInfoWindow:CreatedStr", "Thursday, 19 June 2003, 03:10:03 PM");
    m_createdStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    BStringView *modifiedStr = new BStringView ("ArkInfoWindow:_ModifiedStr", str (S_MODIFIED));
    modifiedStr->SetAlignment (B_ALIGN_RIGHT);
    modifiedStr->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

    m_modifiedStr = new BStringView ("ArkInfoWindow:ModifiedStr", "Friday, 29 July 2003, 01:10:23 PM");
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
    FillDetails ();

    // Center window on-screen
    CenterOnScreen();

    // Restore position from prefs (not size)
    BPoint pt;
    if (_prefs_windows.FindBoolDef (kPfArkInfoWnd, true))
        if (_prefs_windows.FindPoint (kPfArkInfoWndFrame, &pt) == B_OK)
           MoveTo (pt);

    Show();
}


bool ArkInfoWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef (kPfArkInfoWnd, true))
        _prefs_windows.SetPoint (kPfArkInfoWndFrame, Frame().LeftTop());

    return BWindow::QuitRequested();
}


void ArkInfoWindow::FillDetails ()
{
    char nameBuf[B_FILE_NAME_LENGTH];
    if (m_entry->Exists() == false)
    {
        Hide();
        (new BAlert ("error", str (S_ARCHIVE_MISSING), str (S_OK), NULL, NULL, B_WIDTH_AS_USUAL,
           B_EVEN_SPACING, B_STOP_ALERT))->Go();
        PostMessage (B_QUIT_REQUESTED);
    }

    m_entry->GetName (nameBuf);
    m_fileNameStr->SetText (nameBuf);

    BString buf;
    buf = ""; buf << m_dirList->CountItems();
    m_folderCountStr->SetText (buf.String());

    buf = ""; buf << m_fileList->CountItems();
    m_fileCountStr->SetText (buf.String());

    buf = ""; buf << m_fileList->CountItems() + m_dirList->CountItems();
    m_totalCountStr->SetText (buf.String());

    buf = ""; buf << m_archiver->ArchiveType() << " " << str (S_ARCHIVE);
    m_typeStr->SetText (buf.String());

    BEntry parent; BPath parentPath;
    m_entry->GetParent (&parent);
    parent.GetPath (&parentPath);
    m_pathStr->SetText (parentPath.Path());

    time_t modTime, crTime;
    tm mod_tm, cr_tm;
    m_entry->GetModificationTime (&modTime);
    m_entry->GetCreationTime (&crTime);
    localtime_r (&modTime, &mod_tm);
    localtime_r (&crTime, &cr_tm);

    char dateTimeBuf[256];
    strftime (dateTimeBuf, 256, "%A, %B %d %Y, %I:%M:%S %p", &mod_tm);
    m_modifiedStr->SetText (dateTimeBuf);

    strftime (dateTimeBuf, 256, "%A, %B %d %Y, %I:%M:%S %p", &cr_tm);
    m_createdStr->SetText (dateTimeBuf);

    off_t compressedSize;
    m_entry->GetSize (&compressedSize);
    buf = StringFromBytes (compressedSize);
    if (compressedSize >= 1024LL)
        buf << " (" << CommaFormatString (compressedSize) << " bytes)";

    m_compressedSizeStr->SetText (buf.String());

    // Compute the inflated size from archive entries
    int32 count = m_fileList->CountItems();
    off_t originalSize = 0L;
    for (int32 i = 0; i < count; i++)
    {
        ListEntry *item = ((HashEntry*)m_fileList->ItemAtFast(i))->m_clvItem;
        if (item)
           originalSize += item->m_length;
    }

    buf = StringFromBytes (originalSize);
    if (originalSize >= 1024LL)
        buf << " (" << CommaFormatString (originalSize) << " bytes)";

    m_originalSizeStr->SetText (buf.String());

    // Compute compression ratio
    float ratio;
    ratio = (100 * (originalSize - compressedSize) / (double)originalSize);

    if (ratio < 0)        // For pure .tar files we cannot compute ratio at all, if so don't report -ve
        buf = "n/a";
    else
    {
        char ratioBuf[10];
        sprintf (ratioBuf, "%.1f", ratio);

        buf = "";
        buf << ratioBuf << "%";
    }

    m_compressRatioBar->Update (ratio, NULL, buf.String());
}
