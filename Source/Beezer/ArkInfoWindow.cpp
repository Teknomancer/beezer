/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#include <time.h>

#include <Button.h>
#include <Alert.h>
#include <Entry.h>
#include <Bitmap.h>
#include <Path.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#include <Screen.h>
#include <StatusBar.h>
#include <String.h>
#include <List.h>

#include "ArkInfoWindow.h"
#include "Archiver.h"
#include "HashTable.h"
#include "UIConstants.h"
#include "BevelView.h"
#include "LangStrings.h"
#include "ListEntry.h"
#include "LocalUtils.h"
#include "AppUtils.h"
#include "StaticBitmapView.h"
#include "Preferences.h"
#include "PrefsFields.h"

//=============================================================================================================//

ArkInfoWindow::ArkInfoWindow (BWindow *callerWindow, Archiver *archiver, BEntry *entry)
    : BWindow (BRect (30, 30, 440, 280), str (S_ARK_INFO_WINDOW_TITLE), B_TITLED_WINDOW,
            B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE, B_CURRENT_WORKSPACE),
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

    m_backView = new BevelView (Bounds(), "ArkInfoWindow:BackView", btOutset, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
    m_backView->SetViewColor (K_BACKGROUND_COLOR);
    AddChild (m_backView);

    BFont font (be_plain_font);
    font_height fntHt;

    font.GetHeight (&fntHt);
    float normFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
    
    font.SetFace (B_BOLD_FACE);
    font.GetHeight (&fntHt);
    float totalFontHeight = fntHt.ascent + fntHt.descent + fntHt.leading + 2.0;
    
    
    BBitmap *infoBmp = ResBitmap ("Img:ArchiveInfo");
    
    BevelView *sepView1 = new BevelView (BRect (-1, infoBmp->Bounds().Height() + 4 * K_MARGIN,
                                    Bounds().right - 1.0, infoBmp->Bounds().Height() + 4 * K_MARGIN + 1),
                                    "ArkInfoWindow:SepView1", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView1);
    
    StaticBitmapView *infoBmpView = new StaticBitmapView (BRect (K_MARGIN * 5, K_MARGIN * 2,
                            infoBmp->Bounds().Width() + K_MARGIN * 5, infoBmp->Bounds().Height() + K_MARGIN * 2),
                                        "ArkInfoWindow:iconView", infoBmp);
    infoBmpView->SetViewColor (m_backView->ViewColor());
    AddChild (infoBmpView);

    // Add the file name string view (align it vertically with the icon view)
    m_fileNameStr = new BStringView (BRect (infoBmpView->Frame().right + K_MARGIN * 3,
                                        infoBmpView->Frame().top, Bounds().right - 1,
                                        infoBmpView->Frame().top + totalFontHeight),
                                        "ArkInfoWindow:FileNameView", "", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_fileNameStr->SetFont (&font);
    m_backView->AddChild (m_fileNameStr);
    m_fileNameStr->MoveTo (m_fileNameStr->Frame().left,
        (infoBmpView->Frame().Height() / 2 - totalFontHeight / 2) + totalFontHeight / 2 + 1);
    m_fileNameStr->ResizeToPreferred ();
    
    
    // Calculate the maximum width of left-side labels, and declare margin widths
    float marginLeft = 6 * K_MARGIN;
    float marginTop = 2 * K_MARGIN;
    float marginMid = 15;        // Width between right-end of left labels and left-end of right labels
    float vGap = 4;
    
    int descStringsCount = 8;
    BString descStrings[] =
    {
        str (S_COMPRESSION_RATIO),
        str (S_COMPRESSED_SIZE),
        str (S_ORIGINAL_SIZE),
        str (S_NUMBER_OF_FILES),
        str (S_NUMBER_OF_FOLDERS),
        str (S_TOTAL_ENTRIES),
        str (S_TYPE),
        str (S_PATH),
        str (S_CREATED),
        str (S_MODIFIED)
    };
    
    float maxWidth = m_backView->StringWidth (descStrings[0].String());
    for (int32 i = 1; i <= descStringsCount; i++)
        maxWidth = MAX (maxWidth, m_backView->StringWidth (descStrings[i].String()));
    
    float dividerWidth = maxWidth + marginLeft + 2;


    // Start rendering the rest of the controls
    m_compressRatioBar = new BStatusBar (BRect (marginLeft, sepView1->Frame().bottom + marginTop - 1,
                                Bounds().Width() - marginLeft,
                                sepView1->Frame().bottom + marginTop - 1 + normFontHeight),
                                "ArkInfoWindow:CompressRatioBar", str (S_COMPRESSION_RATIO), NULL);
    m_backView->AddChild (m_compressRatioBar);
    m_compressRatioBar->SetResizingMode (B_FOLLOW_LEFT_RIGHT);
    m_compressRatioBar->SetBarHeight (K_PROGRESSBAR_HEIGHT);
    m_compressRatioBar->SetBarColor (K_PROGRESS_COLOR);
    m_compressRatioBar->SetMaxValue (100);

    BStringView *compressedSizeStr = new BStringView (BRect (marginLeft,
                                            m_compressRatioBar->Frame().bottom + vGap, dividerWidth,
                                            m_compressRatioBar->Frame().bottom + vGap + normFontHeight),
                                            "ArkInfoWindow:_CompressedSizeStr", str (S_COMPRESSED_SIZE),
                                            B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (compressedSizeStr);
    compressedSizeStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_compressedSizeStr = new BStringView (BRect (dividerWidth + marginMid, compressedSizeStr->Frame().top,
                                    0, compressedSizeStr->Frame().bottom), "ArkInfoWindow:CompressedSizeStr",
                                    "0 MB (0 bytes)", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_compressedSizeStr);

    BStringView *originalSizeStr = new BStringView (BRect (marginLeft,
                                            compressedSizeStr->Frame().bottom + vGap, dividerWidth,
                                            compressedSizeStr->Frame().bottom + vGap + normFontHeight),
                                            "ArkInfoWindow:_OriginalSizeStr", str (S_ORIGINAL_SIZE),
                                            B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (originalSizeStr);
    originalSizeStr->SetAlignment (B_ALIGN_RIGHT);

    m_originalSizeStr = new BStringView (BRect (dividerWidth + marginMid, originalSizeStr->Frame().top, 0,
                                    originalSizeStr->Frame().bottom), "ArkInfoWindow:OriginalSizeStr",
                                    "0 MB (0 bytes)", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_originalSizeStr);

    BStringView *fileCountStr = new BStringView (BRect (marginLeft, originalSizeStr->Frame().bottom + vGap,
                                        dividerWidth, originalSizeStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_FileCountStr", str (S_NUMBER_OF_FILES),
                                        B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (fileCountStr);
    fileCountStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_fileCountStr = new BStringView (BRect (dividerWidth + marginMid, fileCountStr->Frame().top, 0,
                                    fileCountStr->Frame().bottom), "ArkInfoWindow:FileCountStr", "0",
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_fileCountStr);
    
    BStringView *folderCountStr = new BStringView (BRect (marginLeft, fileCountStr->Frame().bottom + vGap,
                                        dividerWidth, fileCountStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_FolderCountStr", str (S_NUMBER_OF_FOLDERS),
                                        B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (folderCountStr);
    folderCountStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_folderCountStr = new BStringView (BRect (dividerWidth + marginMid, folderCountStr->Frame().top, 0,
                                    folderCountStr->Frame().bottom), "ArkInfoWindow:FolderCountStr", "0",
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_folderCountStr);

    BStringView *totalCountStr = new BStringView (BRect (marginLeft, folderCountStr->Frame().bottom + vGap,
                                        dividerWidth, folderCountStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_TotalCountStr", str (S_TOTAL_ENTRIES),
                                        B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (totalCountStr);
    totalCountStr->SetAlignment (B_ALIGN_RIGHT);

    m_totalCountStr = new BStringView (BRect (dividerWidth + marginMid, totalCountStr->Frame().top, 0,
                                    totalCountStr->Frame().bottom), "ArkInfoWindow:TotalCountStr", "0",
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_totalCountStr);


    BevelView *sepView2 = new BevelView (BRect (-1, totalCountStr->Frame().bottom + marginTop + 1,
                                    Bounds().right - 1.0, totalCountStr->Frame().bottom + marginTop + 2),
                                    "ArkInfoWindow:SepView2", btInset, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW);
    m_backView->AddChild (sepView2);

    // Other file infos like path, type, created, modified etc.
    BStringView *typeStr = new BStringView (BRect (marginLeft, sepView2->Frame().bottom + marginTop,
                                        dividerWidth, sepView2->Frame().bottom + marginTop + normFontHeight),
                                        "ArkInfoWindow:_TypeStr", str (S_TYPE), B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (typeStr);
    typeStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_typeStr = new BStringView (BRect (dividerWidth + marginMid, typeStr->Frame().top, 0,
                                    typeStr->Frame().bottom), "ArkInfoWindow:TypeStr", "-",
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_typeStr);

    BStringView *pathStr = new BStringView (BRect (marginLeft, typeStr->Frame().bottom + vGap,
                                        dividerWidth, typeStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_PathStr", str (S_PATH), B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (pathStr);
    pathStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_pathStr = new BStringView (BRect (dividerWidth + marginMid, pathStr->Frame().top, 0,
                                    pathStr->Frame().bottom), "ArkInfoWindow:PathStr", "/boot",
                                    B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_pathStr);

    BStringView *createdStr = new BStringView (BRect (marginLeft, pathStr->Frame().bottom + vGap,
                                        dividerWidth, pathStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_CreatedStr", str (S_CREATED), B_FOLLOW_LEFT,
                                        B_WILL_DRAW);
    m_backView->AddChild (createdStr);
    createdStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_createdStr = new BStringView (BRect (dividerWidth + marginMid, createdStr->Frame().top,
                                    0, createdStr->Frame().bottom), "ArkInfoWindow:CreatedStr",
                                    "Thursday, 19 June 2003, 03:10:03 PM", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_createdStr);
    m_createdStr->ResizeToPreferred();
    
    BStringView *modifiedStr = new BStringView (BRect (marginLeft, createdStr->Frame().bottom + vGap,
                                        dividerWidth, createdStr->Frame().bottom + vGap + normFontHeight),
                                        "ArkInfoWindow:_ModifiedStr", str (S_MODIFIED), B_FOLLOW_LEFT,
                                        B_WILL_DRAW);
    m_backView->AddChild (modifiedStr);
    modifiedStr->SetAlignment (B_ALIGN_RIGHT);
    
    m_modifiedStr = new BStringView (BRect (dividerWidth + marginMid, modifiedStr->Frame().top, 0,
                                    modifiedStr->Frame().bottom), "ArkInfoWindow:ModifiedStr",
                                    "Friday, 29 July 2003, 01:10:23 PM", B_FOLLOW_LEFT, B_WILL_DRAW);
    m_backView->AddChild (m_modifiedStr);

    // Now that we have done w/ placing the cntrls, we will getinfo about the archive & write it to the cntrls
    FillDetails ();

    // Auto-size the width, height of the window to fit the controls rendered above
    AutoSizeWindow (marginLeft, marginMid, maxWidth);
    ResizeTo (Frame().Width(), m_modifiedStr->Frame().bottom + marginTop + m_backView->EdgeThickness() + 1);

    // Center window on-screen
    BRect screen_rect (BScreen().Frame());
    MoveTo (screen_rect.Width() / 2 - Frame().Width() / 2, screen_rect.Height() / 2 - Frame().Height() / 2);

    // Contrain resize
    float minH, maxH, minV, maxV;
    GetSizeLimits (&minH, &maxH, &minV, &maxV);
    SetSizeLimits (Bounds().Width(), maxH, Bounds().Height(), maxV);
    
    // Restore position from prefs (not size)
    BPoint pt;
    if (_prefs_windows.FindBoolDef (kPfArkInfoWnd, true))
        if (_prefs_windows.FindPoint (kPfArkInfoWndFrame, &pt) == B_OK)
            MoveTo (pt);

    Show();    
}

//=============================================================================================================//

bool ArkInfoWindow::QuitRequested()
{
    if (_prefs_windows.FindBoolDef (kPfArkInfoWnd, true))
        _prefs_windows.SetPoint (kPfArkInfoWndFrame, Frame().LeftTop());

    return BWindow::QuitRequested();
}

//=============================================================================================================//

void ArkInfoWindow::AutoSizeWindow (float cornerMargin, float midMargin, float leftSideMaxWidth)
{
    m_compressedSizeStr->ResizeToPreferred();
    m_originalSizeStr->ResizeToPreferred();
    m_folderCountStr->ResizeToPreferred();
    m_fileCountStr->ResizeToPreferred();
    m_totalCountStr->ResizeToPreferred();
    m_typeStr->ResizeToPreferred();
    m_pathStr->ResizeToPreferred();
    m_createdStr->ResizeToPreferred();
    m_modifiedStr->ResizeToPreferred();

    int descRStringsCount = 8;
    BString descRStrings[] =
    {
        m_compressedSizeStr->Text(),
        m_originalSizeStr->Text(),
        m_folderCountStr->Text(),
        m_totalCountStr->Text(),
        m_fileCountStr->Text(),
        m_typeStr->Text(),
        m_pathStr->Text(),
        m_createdStr->Text(),
        m_modifiedStr->Text()
    };
    
    float maxRWidth = m_backView->StringWidth (descRStrings[0].String());
    for (int32 i = 1; i <= descRStringsCount; i++)
        maxRWidth = MAX (maxRWidth, m_backView->StringWidth (descRStrings[i].String()));
    
    maxRWidth += 2 * cornerMargin + midMargin + leftSideMaxWidth;

    // Make sure window accomodates the filename string (which will be in bold so will usually
    // be longer than plain font)    
    maxRWidth = MAX (maxRWidth, m_fileNameStr->Frame().right + cornerMargin + 1);
    ResizeTo (maxRWidth, Frame().Height());
}

//=============================================================================================================//

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
    m_fileNameStr->ResizeToPreferred();

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

//=============================================================================================================//
