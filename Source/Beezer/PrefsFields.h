/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _PREFS_FIELDS_H
#define _PREFS_FIELDS_H

// IMPORTANT NOTE:: Everytime you add a new constant to this file,
// make sure it is NOT already defined by doing a text search. It 
// would be extremely difficult to trace bugs arising from duplicate
// constants (as compilers can't pick them up)

const char
    *const kPfRecentPath =                "pfRecent",                  // Recentpaths
    
    *const kPfOpen =                     "pfOpen",                  // Extract
    *const kPfClose =                    "pfClose",
    *const kPfQuit =                     "pfQuit",
    *const kPfDrag =                     "pfDrag",
    
    *const kPfStoreUI =                  "pfStoreUI",               // State
    *const kPfStoreArk =                 "pfStoreArk",
    *const kPfRestoreUI =                "pfRestoreUI",
    *const kPfRestoreArk =                "pfRestoreArk",
    
    *const kPfSearchWnd =                "pfSearchWnd",               // Windows
    *const kPfSearchWndFrame =            "pfSearchWndFrame",
    *const kPfCommentWnd =                "pfCommentWnd",
    *const kPfCommentWndFrame =           "pfCommentWndFrame",
    *const kPfLogWnd =                   "pfLogWnd",
    *const kPfLogWndFrame =               "pfLogWndFrame",
    *const kPfArkInfoWnd =                "pfArkInfoWnd",
    *const kPfArkInfoWndFrame =           "pfArkInfoWndFrame",
    *const kPfWelcomeWnd =                "pfWelcomeWnd",
    *const kPfWelcomeWndFrame =           "pfWelcomeWndFrame",
    *const kPfPrefsWnd =                 "pfPrefsWnd",
    *const kPfPrefsWndFrame =             "pfPrefsWndFrame",
    
    *const kPfDefOpenPath =               "pfDefOpenPath",           // Paths
    *const kPfDefAddPath =                "pfDefAddPath",
    *const kPfDefExtractPath =            "pfDefExtractPath",
    *const kPfUseArkDir =                "pfUseArkDir",
    *const kPfFavPathsMsg =               "pfFavPathsMsg",
    *const kPfGenPath =                  "pfGenPath",
    
    *const kPfShowPathInRecent =          "pfShowPath",               // Recent
    *const kPfNumRecentArk =              "pfnArchives",
    *const kPfNumRecentExt =              "pfnExtracts",
    
    *const kPfReplaceFiles =              "pfReplace",               // Add
    *const kPfWarnBeforeAdd =             "pfWarn",
    *const kPfWarnAmount =                "pfWarnMB",
    *const kPfSortAfterAdd =              "pfSortAfterAdd",
    *const kPfConfirmDropAdd =            "pfConfirmDropAdd",
    
    *const kPfFullLengthBars =            "pfFullLengthBars",           // Interface
    *const kPfActFore =                  "pfActFore",
    *const kPfActBack =                  "pfActBack",
    *const kPfColorIndex =                "pfColIndex",
    
    *const kPfPrefPanelIndex =            "pfPanelIndex",               // Miscellaneous
    *const kPfStartup =                  "pfStartupAction",
    *const kPfWelcomeOnQuit =             "pfWelcomeOnQuit",
    *const kPfShowCommentOnOpen =         "pfShowCommentOpen",
    *const kPfMimeOnStartup =             "pfRegMimeStartup",
    *const kPfDefaultArk =                "pfDefaultArk";

#endif /* _PREFS_FIELDS_H */
