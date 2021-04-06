// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Chris Roberts.
// All rights reserved.

#ifndef _COMMON_STRINGS_H
#define _COMMON_STRINGS_H

#include "AppConstants.h"

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Common"
#else
#define B_TRANSLATE_MARK(x) x
#endif

/*
*    These strings are shared across multiple source files.  Most of them are
*    here to reduce duplicates and make it easier to translate.  Some are also
*    used as labels for menus/menuitems and must be consistent across files for
*    BMenu::FindItem() and similar methods to work properly
*/

static const char* skAboutString = B_TRANSLATE_MARK("About");
static const char* skAddString = B_TRANSLATE_MARK("Add");
static const char* skbytesString = B_TRANSLATE_MARK("bytes");
static const char* skBytesString = B_TRANSLATE_MARK("Bytes");
static const char* skCancelString = B_TRANSLATE_MARK("Cancel");
static const char* skCloseWindowString = B_TRANSLATE_MARK("Close window");
static const char* skCommentString = B_TRANSLATE_MARK("Comment");
static const char* skCRCString = B_TRANSLATE_MARK("CRC");
static const char* skCreateString = B_TRANSLATE_MARK("Create");
static const char* skDateString = B_TRANSLATE_MARK("Date");
static const char* skDeleteString = B_TRANSLATE_MARK("Delete");
static const char* skExabyteString = B_TRANSLATE_MARK("EiB");
static const char* skExtractSelectedString = B_TRANSLATE_MARK("Extract selected" B_UTF8_ELLIPSIS);
static const char* skExtractString = B_TRANSLATE_MARK("Extract");
static const char* skExtractToString = B_TRANSLATE_MARK("Extract to" B_UTF8_ELLIPSIS);
static const char* skForceStopString = B_TRANSLATE_MARK("Force stop");
static const char* skGigabyteString = B_TRANSLATE_MARK("GiB");
static const char* skHelpString = B_TRANSLATE_MARK("Help");
static const char* skKilobyteString = B_TRANSLATE_MARK("KiB");
static const char* skMethodString = B_TRANSLATE_MARK("Method");
static const char* skMegabyteString = B_TRANSLATE_MARK("MiB");
static const char* skNameString = B_TRANSLATE_MARK("Name");
static const char* skNewString = B_TRANSLATE_MARK("New");
static const char* skOKString = B_TRANSLATE_MARK("OK");
static const char* skOpenExistingString = B_TRANSLATE_MARK("Open an existing archive");
static const char* skOpenString = B_TRANSLATE_MARK("Open");
static const char* skOpenMenuString = B_TRANSLATE_MARK("Open" B_UTF8_ELLIPSIS);
static const char* skPackedString = B_TRANSLATE_MARK("Packed");
static const char* skPasswordString = B_TRANSLATE_MARK("Password:");
static const char* skPathString = B_TRANSLATE_MARK("Path");
static const char* skPetabyteString = B_TRANSLATE_MARK("PiB");
static const char* skRatioString = B_TRANSLATE_MARK("Ratio");
static const char* skRecentString = B_TRANSLATE_MARK("Recent");
static const char* skSaveAsDefaultsString = B_TRANSLATE_MARK("Save as defaults");
static const char* skSaveToArchiveString = B_TRANSLATE_MARK("Save to archive");
static const char* skSaveString = B_TRANSLATE_MARK("Save");
static const char* skSearchString = B_TRANSLATE_MARK("Search");
static const char* skSelectAllString = B_TRANSLATE_MARK("Select all");
static const char* skSelectString = B_TRANSLATE_MARK("Select");
static const char* skSettingsString = B_TRANSLATE_MARK("Settings");
static const char* skAllFoldedString = B_TRANSLATE_MARK("Show all items folded");
static const char* skAllUnfoldedString = B_TRANSLATE_MARK("Show all items unfolded");
static const char* skTwoUnfoldedString = B_TRANSLATE_MARK("Show first 2 levels unfolded");
static const char* skFirstUnfoldedString = B_TRANSLATE_MARK("Show first level unfolded");
static const char* skSizeString = B_TRANSLATE_MARK("Size");
static const char* skTerrabyteString = B_TRANSLATE_MARK("TiB");
static const char* skToolsString = B_TRANSLATE_MARK("Tools");
static const char* skViewString = B_TRANSLATE_MARK("View");
static const char* skWindowsString = B_TRANSLATE_MARK("Windows");

#endif /* _COMMON_STRINGS_H */
