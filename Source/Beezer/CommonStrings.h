// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Chris Roberts.
// All rights reserved.

#ifndef _COMMON_STRINGS_H
#define _COMMON_STRINGS_H


#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Common"
// for some reason Haiku doesn't have a B_TRANSLATE_NOCOLLECT_CONTEXT variant of the macro
// so we use the ALL version with an empty comment
#define BZ_TR(x) B_TRANSLATE_NOCOLLECT_ALL(x, "Common", "")
#else
#define B_TRANSLATE_MARK(x) x
#define BZ_TR(x) x
#endif


/*
*    These strings are shared across multiple source files.  Most of them are
*    here to reduce duplicates and make it easier to translate.  Some are also
*    used as labels for menus/menuitems and must be consistent across files for
*    BMenu::FindItem() and similar methods to work properly
*/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static const char
            *kAboutString           = B_TRANSLATE_MARK("About"),
            *kAddString             = B_TRANSLATE_MARK("Add"),
            *kbytesString           = B_TRANSLATE_MARK("bytes"),
            *kBytesString           = B_TRANSLATE_MARK("Bytes"),
            *kCancelString          = B_TRANSLATE_MARK("Cancel"),
            *kCloseWindowString     = B_TRANSLATE_MARK("Close window"),
            *kCommentString         = B_TRANSLATE_MARK("Comment"),
            *kCRCString             = B_TRANSLATE_MARK("CRC"),
            *kCreateString          = B_TRANSLATE_MARK("Create"),
            *kDateString            = B_TRANSLATE_MARK("Date"),
            *kDeleteString          = B_TRANSLATE_MARK("Delete"),
            *kExabyteString         = B_TRANSLATE_MARK("EiB"),
            *kExtractSelectedString = B_TRANSLATE_MARK("Extract selected" B_UTF8_ELLIPSIS),
            *kExtractString         = B_TRANSLATE_MARK("Extract"),
            *kExtractToString       = B_TRANSLATE_MARK("Extract to" B_UTF8_ELLIPSIS),
            *kForceStopString       = B_TRANSLATE_MARK("Force stop"),
            *kGigabyteString        = B_TRANSLATE_MARK("GiB"),
            *kHelpString            = B_TRANSLATE_MARK("Help"),
            *kKilobyteString        = B_TRANSLATE_MARK("KiB"),
            *kMethodString          = B_TRANSLATE_MARK("Method"),
            *kMegabyteString        = B_TRANSLATE_MARK("MiB"),
            *kNameString            = B_TRANSLATE_MARK("Name"),
            *kNewString             = B_TRANSLATE_MARK("New"),
            *kOKString              = B_TRANSLATE_MARK("OK"),
            *kOpenExistingString    = B_TRANSLATE_MARK("Open an existing archive"),
            *kOpenString            = B_TRANSLATE_MARK("Open"),
            *kOpenMenuString        = B_TRANSLATE_MARK("Open" B_UTF8_ELLIPSIS),
            *kPackedString          = B_TRANSLATE_MARK("Packed"),
            *kPasswordString        = B_TRANSLATE_MARK("Password:"),
            *kPathString            = B_TRANSLATE_MARK("Path"),
            *kPetabyteString        = B_TRANSLATE_MARK("PiB"),
            *kRatioString           = B_TRANSLATE_MARK("Ratio"),
            *kRecentString          = B_TRANSLATE_MARK("Recent"),
            *kSaveAsDefaultsString  = B_TRANSLATE_MARK("Save as defaults"),
            *kSaveToArchiveString   = B_TRANSLATE_MARK("Save to archive"),
            *kSaveString            = B_TRANSLATE_MARK("Save"),
            *kSearchString          = B_TRANSLATE_MARK("Search"),
            *kSelectAllString       = B_TRANSLATE_MARK("Select all"),
            *kSelectString          = B_TRANSLATE_MARK("Select"),
            *kSettingsString        = B_TRANSLATE_MARK("Settings"),
            *kAllFoldedString       = B_TRANSLATE_MARK("Show all items folded"),
            *kAllUnfoldedString     = B_TRANSLATE_MARK("Show all items unfolded"),
            *kTwoUnfoldedString     = B_TRANSLATE_MARK("Show first 2 levels unfolded"),
            *kFirstUnfoldedString   = B_TRANSLATE_MARK("Show first level unfolded"),
            *kSizeString            = B_TRANSLATE_MARK("Size"),
            *kTerrabyteString       = B_TRANSLATE_MARK("TiB"),
            *kToolsString           = B_TRANSLATE_MARK("Tools"),
            *kViewString            = B_TRANSLATE_MARK("View"),
            *kWindowsString         = B_TRANSLATE_MARK("Windows");
#pragma GCC diagnostic pop
#endif  /* __GNUC__ */

#endif /* _COMMON_STRINGS_H */
