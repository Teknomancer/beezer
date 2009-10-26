/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _APP_CONSTANTS_H
#define _APP_CONSTANTS_H

#define K_APP_TITLE                            "Beezer"
#define K_APP_AUTHOR                           "Ramshankar"
#define K_APP_AUTHOR_MAIL                      "(v.ramshankar@gmail.com)"
#define K_APP_AUTHOR_MAIL_1                    "v.ramshankar@gmail.com"
#define K_APP_AUTHOR_MAIL_URL_1                "mailto:v.ramshankar@gmail.com"
#define K_APP_AUTHOR_MAIL_2                    "ramthemad@yahoo.co.in"
#define K_APP_AUTHOR_MAIL_URL_2                "mailto:ramthemad@yahoo.co.in"
#define K_APP_WEBSITE                          "www.geocities.com/ram_beos"
#define K_APP_WEBSITE_URL                      "http://www.geocities.com/ram_beos"
#define K_APP_AUTHOR_BESHARE_NICK              "Teknomancer"
#define K_APP_AUTHOR_COUNTRY                   "India"
#define K_APP_AUTHOR_OCCUPATION                "Student"
#define K_APP_AUTHOR_AGE                       "NN"

#define K_APP_COMPILE_STRING                   "__compiled_on__"

#define K_COMPILED_TIME_FILE                   "_bzr_ctstamp.txt"
#define K_RULE_FILE                            "_bzr_rules.txt"
#define K_STUB_JOINER_FILE                     "stub_joiner"
#define K_ARK_DIR_NAME                         "add-ons/"
#define K_BIN_DIR_NAME                         "workers/"
#define K_DOC_DIR_NAME                         "docs/"
#define K_SETTINGS_DIR_NAME                    "settings/"
#define K_STUB_DIR_NAME                        "stubs/"
#ifdef B_ZETA_VERSION
    #define K_LANG_DIR_NAME                    "languages/"
#endif

#define K_SETTINGS_MAINWINDOW                  "mainwindow_settings"
#define K_SETTINGS_COLORS                      "colors_settings"
#define K_SETTINGS_PATHS                       "paths_settings"
#define K_SETTINGS_EXTRACT                     "extract_settings"
#define K_SETTINGS_ADD                         "add_settings"
#define K_SETTINGS_STATE                       "state_settings"
#define K_SETTINGS_WINDOWS                     "windows_settings"
#define K_SETTINGS_RECENT                      "recent_settings"
#define K_SETTINGS_LANG                        "lang_settings"
#define K_SETTINGS_MISC                        "misc_settings"
#define K_SETTINGS_RECENT_ARCHIVES             "recent_archives"
#define K_SETTINGS_RECENT_EXTRACT              "recent_extracts"
#define K_SETTINGS_INTERFACE                   "interface_settings"
#define K_SETTINGS_RECENT_SPLIT_FILES          "recent_split_files"
#define K_SETTINGS_RECENT_SPLIT_DIRS           "recent_split_folders"

#define K_UI_ATTRIBUTE                         "bzr:ui"
#define K_ARK_ATTRIBUTE                        "bzr:ark"
#define K_FILENAME_ATTRIBUTE                   "bzr:file"
#define K_SEPARATOR_ATTRIBUTE                  "bzr:sep"

#ifdef __INTEL__
    #ifdef B_ZETA_VERSION
        #define K_APP_VERSION                  "0.09 (Zeta-x86)"
    #else
        #define K_APP_VERSION                  "0.09 (x86)"
    #endif
#else
    #ifdef B_ZETA_VERSION
        #define K_APP_VERSION                  "0.09 (Zeta-ppc)"
    #else
        #define K_APP_VERSION                  "0.09 (ppc)"
    #endif
#endif

#define K_BARBERPOLE_PULSERATE                 100000
#define K_PROGRESS_DELAY                       70000

#endif /* _APP_CONSTANTS_H */
