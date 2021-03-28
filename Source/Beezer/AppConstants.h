/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _APP_CONSTANTS_H
#define _APP_CONSTANTS_H

#define K_APP_TITLE                          "Beezer"
#define K_APP_AUTHOR                         "Ramshankar"
#define K_APP_AUTHOR_MAIL                    "<v." "ramshankar" "@gmail.com>"
#define K_APP_AUTHOR_2                       "Chris Roberts"
#define K_APP_AUTHOR_2_MAIL                  "<mail>"   // @todo

#define K_APP_WEBSITE                        "ramshankar.org/software/beezer/"
#define K_APP_WEBSITE_URL                    "https://ramshankar.org/software/beezer/"

#define K_APP_GITHUB_URL                     "https://github.com/Teknomancer/beezer"

#define K_APP_COMPILE_STRING                 "__compiled_on__"

#define K_RULE_FILE                          "_bzr_rules.txt"
#define K_STUB_JOINER_FILE                   "stub_joiner"
#define K_ARK_DIR_NAME                       "add-ons/"
#define K_DOC_DIR_NAME                       "docs/"
#define K_SETTINGS_DIR_NAME                  "Beezer"
#define K_STUB_DIR_NAME                      "stubs/"

#define K_SETTINGS_MAINWINDOW                "mainwindow_settings"
#define K_SETTINGS_COLORS                    "colors_settings"
#define K_SETTINGS_PATHS                     "paths_settings"
#define K_SETTINGS_EXTRACT                   "extract_settings"
#define K_SETTINGS_ADD                       "add_settings"
#define K_SETTINGS_STATE                     "state_settings"
#define K_SETTINGS_WINDOWS                   "windows_settings"
#define K_SETTINGS_RECENT                    "recent_settings"
#define K_SETTINGS_LANG                      "lang_settings"
#define K_SETTINGS_MISC                      "misc_settings"
#define K_SETTINGS_RECENT_ARCHIVES            "recent_archives"
#define K_SETTINGS_RECENT_EXTRACT             "recent_extracts"
#define K_SETTINGS_INTERFACE                 "interface_settings"
#define K_SETTINGS_RECENT_SPLIT_FILES         "recent_split_files"
#define K_SETTINGS_RECENT_SPLIT_DIRS          "recent_split_folders"

#define K_UI_ATTRIBUTE                       "bzr:ui"
#define K_ARK_ATTRIBUTE                      "bzr:ark"
#define K_FILENAME_ATTRIBUTE                 "bzr:file"
#define K_SEPARATOR_ATTRIBUTE                "bzr:sep"

#ifdef __i386__
#define K_APP_VERSION                "0.99 (x86)"
#elif __x86_64__
#define K_APP_VERSION                "0.99 (x86_64)"
#else
#define K_APP_VERSION                "0.99 (ppc)"
#endif

#define K_BARBERPOLE_PULSERATE                100000
#define K_PROGRESS_DELAY                      70000

#endif /* _APP_CONSTANTS_H */
