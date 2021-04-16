// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Chris Roberts.
// All rights reserved.

#ifndef _COMMON_STRINGS_H
#define _COMMON_STRINGS_H

#include <InterfaceDefs.h>

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

// These strings are shared across multiple source files.  Most of them are
// here to reduce duplicates and make it easier to translate.  Some are also
// used as labels for menus/menuitems and must be consistent across files for
// BMenu::FindItem() and similar methods to work properly
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static const char
       *const kAboutString            = B_TRANSLATE_MARK("About"),
       *const kAddString              = B_TRANSLATE_MARK("Add"),
       *const kbytesString            = B_TRANSLATE_MARK("bytes"),
       *const kBytesString            = B_TRANSLATE_MARK("Bytes"),
       *const kCancelString           = B_TRANSLATE_MARK("Cancel"),
       *const kCloseWindowString      = B_TRANSLATE_MARK("Close window"),
       *const kCommentString          = B_TRANSLATE_MARK("Comment"),
       *const kCRCString              = B_TRANSLATE_MARK("CRC"),
       *const kCreateString           = B_TRANSLATE_MARK("Create"),
       *const kDateString             = B_TRANSLATE_MARK("Date"),
       *const kDeleteString           = B_TRANSLATE_MARK("Delete"),
       *const kExabyteString          = B_TRANSLATE_MARK("EiB"),
       *const kExtractSelectedString  = B_TRANSLATE_MARK("Extract selected" B_UTF8_ELLIPSIS),
       *const kExtractString          = B_TRANSLATE_MARK("Extract"),
       *const kExtractToString        = B_TRANSLATE_MARK("Extract to" B_UTF8_ELLIPSIS),
       *const kForceStopString        = B_TRANSLATE_MARK("Force stop"),
       *const kGigabyteString         = B_TRANSLATE_MARK("GiB"),
       *const kHelpString             = B_TRANSLATE_MARK("Help"),
       *const kKilobyteString         = B_TRANSLATE_MARK("KiB"),
       *const kMethodString           = B_TRANSLATE_MARK("Method"),
       *const kMegabyteString         = B_TRANSLATE_MARK("MiB"),
       *const kNameString             = B_TRANSLATE_MARK("Name"),
       *const kNewString              = B_TRANSLATE_MARK("New"),
       *const kOKString               = B_TRANSLATE_MARK("OK"),
       *const kOpenExistingString     = B_TRANSLATE_MARK("Open an existing archive"),
       *const kOpenString             = B_TRANSLATE_MARK("Open"),
       *const kOpenMenuString         = B_TRANSLATE_MARK("Open" B_UTF8_ELLIPSIS),
       *const kPackedString           = B_TRANSLATE_MARK("Packed"),
       *const kPasswordString         = B_TRANSLATE_MARK("Password:"),
       *const kPathString             = B_TRANSLATE_MARK("Path"),
       *const kPetabyteString         = B_TRANSLATE_MARK("PiB"),
       *const kRatioString            = B_TRANSLATE_MARK("Ratio"),
       *const kResetToDefaultsString  = B_TRANSLATE_MARK("Reset to defaults"),
       *const kRecentString           = B_TRANSLATE_MARK("Recent"),
       *const kSaveAsDefaultsString   = B_TRANSLATE_MARK("Save as defaults"),
       *const kSaveToArchiveString    = B_TRANSLATE_MARK("Save to archive"),
       *const kSaveString             = B_TRANSLATE_MARK("Save"),
       *const kSearchString           = B_TRANSLATE_MARK("Search"),
       *const kSelectAllString        = B_TRANSLATE_MARK("Select all"),
       *const kSelectString           = B_TRANSLATE_MARK("Select"),
       *const kSettingsString         = B_TRANSLATE_MARK("Settings"),
       *const kAllFoldedString        = B_TRANSLATE_MARK("Show all items folded"),
       *const kAllUnfoldedString      = B_TRANSLATE_MARK("Show all items unfolded"),
       *const kTwoUnfoldedString      = B_TRANSLATE_MARK("Show first 2 levels unfolded"),
       *const kFirstUnfoldedString    = B_TRANSLATE_MARK("Show first level unfolded"),
       *const kSizeString             = B_TRANSLATE_MARK("Size"),
       *const kTerrabyteString        = B_TRANSLATE_MARK("TiB"),
       *const kToolsString            = B_TRANSLATE_MARK("Tools"),
       *const kViewString             = B_TRANSLATE_MARK("View"),
       *const kWindowsString          = B_TRANSLATE_MARK("Windows");

#pragma GCC diagnostic pop
#endif  /* __GNUC__ */

#endif /* _COMMON_STRINGS_H */
