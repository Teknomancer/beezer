/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
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

#ifndef _PREFS_FIELDS_H
#define _PREFS_FIELDS_H

// IMPORTANT NOTE:: Everytime you add a new constant to this file,
// make sure it is NOT already defined by doing a text search. It 
// would be extremely difficult to trace bugs arising from duplicate
// constants (as compilers can't pick them up)

const char
    *const kPfRecentPath =               "pfRecent",                  // Recentpaths
    
    *const kPfOpen =                     "pfOpen",                  // Extract
    *const kPfClose =                    "pfClose",
    *const kPfQuit =                     "pfQuit",
    *const kPfDrag =                     "pfDrag",
    
    *const kPfStoreUI =                  "pfStoreUI",               // State
    *const kPfStoreArk =                 "pfStoreArk",
    *const kPfRestoreUI =                "pfRestoreUI",
    *const kPfRestoreArk =               "pfRestoreArk",
    
    *const kPfSearchWnd =                "pfSearchWnd",               // Windows
    *const kPfSearchWndFrame =           "pfSearchWndFrame",
    *const kPfCommentWnd =               "pfCommentWnd",
    *const kPfCommentWndFrame =          "pfCommentWndFrame",
    *const kPfLogWnd =                   "pfLogWnd",
    *const kPfLogWndFrame =              "pfLogWndFrame",
    *const kPfArkInfoWnd =               "pfArkInfoWnd",
    *const kPfArkInfoWndFrame =          "pfArkInfoWndFrame",
    *const kPfWelcomeWnd =               "pfWelcomeWnd",
    *const kPfWelcomeWndFrame =          "pfWelcomeWndFrame",
    *const kPfPrefsWnd =                 "pfPrefsWnd",
    *const kPfPrefsWndFrame =            "pfPrefsWndFrame",
    
    *const kPfDefOpenPath =              "pfDefOpenPath",           // Paths
    *const kPfDefAddPath =               "pfDefAddPath",
    *const kPfDefExtractPath =           "pfDefExtractPath",
    *const kPfUseArkDir =                "pfUseArkDir",
    *const kPfFavPathsMsg =              "pfFavPathsMsg",
    *const kPfGenPath =                  "pfGenPath",
    
    *const kPfShowPathInRecent =         "pfShowPath",               // Recent
    *const kPfNumRecentArk =             "pfnArchives",
    *const kPfNumRecentExt =             "pfnExtracts",
    
    *const kPfReplaceFiles =             "pfReplace",               // Add
    *const kPfWarnBeforeAdd =            "pfWarn",
    *const kPfWarnAmount =               "pfWarnMB",
    *const kPfSortAfterAdd =             "pfSortAfterAdd",
    *const kPfConfirmDropAdd =           "pfConfirmDropAdd",
   
    *const kPfFullLengthBars =           "pfFullLengthBars",           // Interface
    *const kPfActFore =                  "pfActFore",
    *const kPfActBack =                  "pfActBack",
    *const kPfColorIndex =               "pfColIndex",
    
    *const kPfPrefPanelIndex =           "pfPanelIndex",               // Miscellaneous
    *const kPfStartup =                  "pfStartupAction",
    *const kPfWelcomeOnQuit =            "pfWelcomeOnQuit",
    *const kPfShowCommentOnOpen =        "pfShowCommentOpen",
    *const kPfMimeOnStartup =            "pfRegMimeStartup",
    *const kPfDefaultArk =               "pfDefaultArk";

#endif /* _PREFS_FIELDS_H */
