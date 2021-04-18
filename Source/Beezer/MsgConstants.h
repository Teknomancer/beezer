// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _MSG_CONSTANTS_H
#define _MSG_CONSTANTS_H

// Do NOT put constants that need to be used in Archiver here,
// Put only Beezer specific constants, for common ones that are used
// both by Beezer & Archiver (add-ons) use Shared.h

// IMPORTANT NOTE:: Everytime you add a new constant to this file,
// make sure it is NOT already defined by doing a text search. It
// would be extremely difficult to trace bugs arising from duplicate
// constants (as compilers can't pick them up)

// TODO: Move constants that are file-static to their respective files.
const char* const kSuperItem =          "sitem",
*const kDynamic =                       "dynamic",
*const kSuperItemPath =                 "sitem_path",
*const kFiles =                         "files",
*const kFolders =                       "folders",
*const kSize =                          "size",
*const kCancel =                        "cancel_ptr",
*const kHidden =                        "hidden",
*const kBarHeight =                     "bar_height",
*const kTempPath =                      "tmp_path",
*const kLaunchDir =                     "launch_dir",
*const kCount =                         "count",
*const kRef =                           "ref",
*const kDirectoryRef =                  "dirref",
*const kRoot =                          "rootzoot",
*const kProgressMessenger =             "progress_msgr",
*const kArchiverPtr =                   "archiver_ptr",
*const kCreateMode =                    "create_mode",
*const kAllFiles =                      "all_files",
*const kFileList =                      "file_list",
*const kResult =                        "result",
*const kProgressAction =                "progress_action",
*const kPreparing =                     "preparing",
*const kWindowPtr =                     "window_ptr",
*const kStatusPtr =                     "status_ptr",
*const kWindowList =                    "window_list",
*const kClosingWindow =                 "closing_window",
*const kErroneousArchive =              "erroneous_archive",
*const kErrorString =                   "error_string",
*const kTestResult =                    "test_result",
*const kTestPassed =                    "test_passed",
*const kText =                          "text",
*const kNewLine =                       "new_line",
*const kCapitalize =                    "capitalize",
*const kLeftTrim =                      "trim_leading",
*const kItemPointer =                   "item_pointer",
*const kContextMenuPoint =              "context_point",
*const kExprType =                      "expr_type",
*const kExpr =                          "expr",
*const kInvertSelection =               "invert_sel",
*const kIgnoreCase =                    "ignore_case",
*const kColumnPtr =                     "column_ptr",
*const kAddToSelection =                "add_to_sel",
*const kSearchSelection =               "search_sel",
*const kPersistent =                    "persistent",
*const kWindowRect =                    "window_rect",
*const kCommentContent =                "comment_content",
*const kFailOnNull =                    "fail_on_null",
*const kListItem =                      "list_item",
*const kBytes =                         "sel_bytes",
*const kArchivePath =                   "archive_path",

// Drag and drop constants
*const kFieldFull =                     "bzr:full",
*const kDropClipping =                  "bzr-will_magically_disappear",

// Save constant fields (as small as possible to reduce size while storing)
*const kToolBar =                       "tb",
*const kInfoBar =                       "ib",
*const kSplitter =                      "sp",
*const kSplitterState =                 "ss",
*const kWindowSize =                    "ws",
*const kFolding =                       "fd",
*const kListViewState =                 "ls",

// "2" suffix is used to deliberately break compatibility with old state
// TODO: Why can't we move these inside BeezerListView.cpp ?
*const kColumnCount =                   "cc2",
*const kColumnVisible =                 "cv2",
*const kColumnWidth =                   "cw2",
*const kColumnOrder =                   "co2",
*const kSortKeyCount =                  "sc2",
*const kSortKey =                       "sk2",
*const kSortMode =                      "sm2";

// Constant aliases -- used to not break backward compatibility but still
//                give proper meaning to the message fields
const char* const kLooperPtr =          kWindowPtr;


const char* const kRecentSplitFile =    "recent_split_file",
*const kRecentSplitDir =                "recent_split_dir";

enum
{
    M_ARK_TYPE_SELECTED = 0xbe05,
    M_STOP_OPERATION,
    M_REPLY,
    M_ADD_ITEMS,
    M_ADD_FOLDERS,
    M_ADD_ITEMS_LIST,
    M_ADD_FOLDERS_LIST,
    M_COPY_LISTS,
    M_COUNT_COMPLETE,
    M_SKIPPED,

    M_UPDATE_RECENT,
    M_BROADCAST_STATUS,
    M_SWITCH_WINDOW,
    M_NOTIFY_STATUS,
    M_SEARCH,

    M_FILE_NEW,
    M_FILE_OPEN,
    M_FILE_OPEN_RECENT,
    M_FILE_CLOSE,
    M_FILE_ARCHIVE_INFO,
    M_FILE_PASSWORD,
    M_FILE_DELETE,
    M_FILE_QUIT,

    M_EDIT_COPY,
    M_EDIT_SELECT_ALL,
    M_EDIT_SELECT_ALL_DIRS,
    M_EDIT_SELECT_ALL_FILES,
    M_EDIT_DESELECT_ALL,
    M_EDIT_DESELECT_ALL_DIRS,
    M_EDIT_DESELECT_ALL_FILES,
    M_EDIT_INVERT_SELECTION,
    M_EDIT_EXPAND_ALL,
    M_EDIT_EXPAND_SELECTED,
    M_EDIT_COLLAPSE_ALL,
    M_EDIT_COLLAPSE_SELECTED,
    M_EDIT_PREFERENCES,

    M_ACTIONS_EXTRACT,
    M_ACTIONS_EXTRACT_TO,
    M_ACTIONS_EXTRACT_SELECTED,
    M_ACTIONS_VIEW,
    M_ACTIONS_OPEN_WITH,
    M_ACTIONS_TEST,
    M_ACTIONS_SEARCH_ARCHIVE,
    M_ACTIONS_DEEP_SEARCH,
    M_ACTIONS_COMMENT,
    M_ACTIONS_ADD,
    M_ACTIONS_DELETE,
    M_ACTIONS_RENAME,
    M_ACTIONS_CREATE_FOLDER,

    M_TOOLS_LIST,
    M_TOOLS_FILE_SPLITTER,
    M_CLOSE_FILE_SPLITTER,
    M_TOOLS_FILE_JOINER,
    M_CLOSE_FILE_JOINER,

    M_TOGGLE_LOG,
    M_TOGGLE_TOOLBAR,
    M_TOGGLE_INFOBAR,
    M_TOGGLE_COLUMN_NAME,
    M_TOGGLE_COLUMN_SIZE,
    M_TOGGLE_COLUMN_PACKED,
    M_TOGGLE_COLUMN_RATIO,
    M_TOGGLE_COLUMN_PATH,
    M_TOGGLE_COLUMN_DATE,
    M_TOGGLE_COLUMN_METHOD,
    M_TOGGLE_COLUMN_CRC,
    M_SORT_COLUMN_NAME,
    M_SORT_COLUMN_SIZE,
    M_SORT_COLUMN_PACKED,
    M_SORT_COLUMN_RATIO,
    M_SORT_COLUMN_PATH,
    M_SORT_COLUMN_DATE,
    M_SORT_COLUMN_METHOD,
    M_SORT_COLUMN_CRC,
    M_SORT_NONE,
    M_SORT_ASCENDING,
    M_SORT_DESCENDING,

    M_RESET_TO_DEFAULT,
    M_RESET_ARK_TO_DEFAULT,
    M_SAVE_AS_DEFAULT,
    M_SAVE_ARK_AS_DEFAULT,
    M_SAVE_TO_ARCHIVE,
    M_SAVE_ARK_TO_ARCHIVE,

    M_SELECTION_CHANGED,
    M_TOOLBAR_TOGGLED,
    M_INFOBAR_TOGGLED,

    M_UNREG_WINDOW,
    M_OPEN_REQUESTED,
    M_OPEN_PART_TWO,
    M_CREATE_REQUESTED,
    M_ENTER,
    M_GOT_FOCUS,
    M_CONTEXT_MENU,
    M_OPEN_FINISHED,
    M_EXTRACT_TO,
    M_EXTRACT_DONE,
    M_ADD,
    M_READY_TO_ADD,
    M_ADD_DONE,
    M_EXTRACT_SELECTED_TO,
    M_DELETE_SELECTED,
    M_DELETE_DONE,
    M_UPDATE_PROGRESS,
    M_CLOSE,
    M_EXTRACT_DIR_INIT_ERROR,
    M_ARCHIVE_PATH_INIT_ERROR,
    M_TEST_DONE,

    M_CONTEXT_COPY,
    M_CONTEXT_SELECT,
    M_CONTEXT_DESELECT,

    M_LOG_CONTEXT_CLEAR,
    M_LOG_CONTEXT_COPY,
    M_LOG_CONTEXT_SAVE,

    M_SAVE_COMMENT,

    M_DROP_MESSAGE,
    M_RECENT_ITEM,
    M_RECENT_EXTRACT_ITEM,

    M_RECENT_SPLIT_FILE,
    M_RECENT_SPLIT_DIR,

    M_SELECTION_ADDED,
    M_REGISTER_TYPES,

    M_UPDATE_INTERFACE,

    M_ADDON_HELP,
    M_ADDON_CREATE,

    M_PREFS_HELP,

    M_HELP_MANUAL,
    M_HELP_WEBSITE,
    M_HELP_GITHUB,

    M_CLOSE_ABOUT,

    M_SEARCH_ALL_ENTRIES,
    M_SEARCH_VISIBLE_ENTRIES,
    M_SEARCH_SELECTED_ENTRIES,
    M_SEARCH_CLICKED,
    M_SEARCH_TEXT_MODIFIED,
    M_SEARCH_CLOSED,

    M_CLOSE_STARTUP
};

#endif /* _MSG_CONSTANTS_H */
