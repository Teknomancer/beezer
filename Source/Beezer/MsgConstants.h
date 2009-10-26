/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _MSG_CONSTANTS_H
#define _MSG_CONSTANTS_H

// Do NOT put constants that need to be used in Archiver here,
// Put only Beezer specific constants, for common ones that are used
// both by Beezer & Archiver (add-ons) use Shared.h

// IMPORTANT NOTE:: Everytime you add a new constant to this file,
// make sure it is NOT already defined by doing a text search. It 
// would be extremely difficult to trace bugs arising from duplicate
// constants (as compilers can't pick them up)

const char *const kSuperItem =               "sitem",
    *const kDynamic =                       "dynamic",
    *const kSuperItemPath =                 "sitem_path",
    *const kFiles =                        "files",
    *const kFolders =                       "folders",
    *const kSize =                         "size",
    *const kCancel =                        "cancel_ptr",
    *const kHidden =                        "hidden",
    *const kBarHeight =                     "bar_height",
    *const kTempPath =                      "tmp_path",
    *const kLaunchDir =                     "launch_dir",
    *const kCount =                        "count",
    *const kRef =                          "ref",
    *const kDirectoryRef =                  "dirref",
    *const kRoot =                         "rootzoot",
    *const kProgressMessenger =              "progress_msgr",
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
    *const kErroneousArchive =               "erroneous_archive",
    *const kErrorString =                   "error_string",
    *const kTestResult =                    "test_result",
    *const kTestPassed =                    "test_passed",
    *const kText =                         "text",
    *const kNewLine =                       "new_line",
    *const kCapitalize =                    "capitalize",
    *const kLeftTrim =                      "trim_leading",
    *const kItemPointer =                   "item_pointer",
    *const kContextMenuPoint =               "context_point",
    *const kExprType =                      "expr_type",
    *const kExpr =                         "expr",
    *const kInvertSelection =                "invert_sel",
    *const kIgnoreCase =                    "ignore_case",
    *const kColumnPtr =                     "column_ptr",
    *const kAddToSelection =                "add_to_sel",
    *const kSearchSelection =                "search_sel",
    *const kPersistent =                    "persistent",
    *const kWindowRect =                    "window_rect",
    *const kCommentContent =                "comment_content",
    *const kFailOnNull =                    "fail_on_null",
    *const kListItem =                      "list_item",
    *const kBytes =                        "sel_bytes",
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
    
    *const kColumnWidth =                   "cw",
    *const kColumnOrder =                   "co",
    *const kSortKey =                       "sk",
    *const kSortMode =                      "sm";

// Constant aliases -- used to not break backward compatibility but still
//                give proper meaning to the message fields
const char *const kLooperPtr =               kWindowPtr;


const char* const kRecentSplitFile =         "recent_split_file",
*const kRecentSplitDir =                    "recent_split_dir";

const uint32
    M_ARK_TYPE_SELECTED =            'atsl',
    M_STOP_OPERATION =               'stop',
    M_REPLY =                       'repl',
    M_ADD_ITEMS =                   'adds',
    M_ADD_FOLDERS =                 'adir',
    M_ADD_ITEMS_LIST =               'affl',
    M_ADD_FOLDERS_LIST =             'adfl',
    M_COPY_LISTS =                  'cpyl',
    M_COUNT_COMPLETE =               'cocm',
    M_SKIPPED =                     'skip',

    M_UPDATE_RECENT =                'udrc',
    M_BROADCAST_STATUS =             'brst',
    M_SWITCH_WINDOW =                'swwn',
    M_NOTIFY_STATUS =                'ntst',
    M_SEARCH =                      'srch',

    M_FILE_NEW =                    'flnw',
    M_FILE_OPEN =                   'flop',
    M_FILE_OPEN_RECENT =             'flor',
    M_FILE_CLOSE =                  'flcs',
    M_FILE_ARCHIVE_INFO =            'flai',
    M_FILE_PASSWORD =                'flpw',
    M_FILE_DELETE =                 'fldl',
    M_FILE_HELP =                   'flhl',
    M_FILE_ABOUT =                  'flab',
    M_FILE_DEVELOPER_INFO =          'fldi',
    M_FILE_QUIT =                   'flqt',
    
    M_EDIT_COPY =                   'edcp',
    M_EDIT_SELECT_ALL =              'edsa',
    M_EDIT_SELECT_ALL_DIRS =         'edad',
    M_EDIT_SELECT_ALL_FILES =        'edsf',
    M_EDIT_DESELECT_ALL =            'edda',
    M_EDIT_DESELECT_ALL_DIRS =        'eddd',
    M_EDIT_DESELECT_ALL_FILES =       'eddf',
    M_EDIT_INVERT_SELECTION =        'edis',
    M_EDIT_EXPAND_ALL =              'edea',
    M_EDIT_EXPAND_SELECTED =         'edes',
    M_EDIT_COLLAPSE_ALL =            'edca',
    M_EDIT_COLLAPSE_SELECTED =        'edcs',
    M_EDIT_PREFERENCES =             'edpf',
    
    M_ACTIONS_EXTRACT =              'acxt',
    M_ACTIONS_EXTRACT_TO =           'actt',
    M_ACTIONS_EXTRACT_SELECTED =      'acxs',
    M_ACTIONS_VIEW =                'acvw',
    M_ACTIONS_OPEN_WITH =            'acow',
    M_ACTIONS_TEST =                'acta',
    M_ACTIONS_SEARCH_ARCHIVE =        'acfn',
    M_ACTIONS_DEEP_SEARCH =          'acds',
    M_ACTIONS_COMMENT =              'accm',
    M_ACTIONS_ADD =                 'acad',
    M_ACTIONS_DELETE =               'acdl',
    M_ACTIONS_RENAME =               'acrn',
    M_ACTIONS_CREATE_FOLDER =        'acmk',
    
    M_TOOLS_LIST =                  'tlls',
    M_TOOLS_FILE_SPLITTER =          'tlfs',
    M_CLOSE_FILE_SPLITTER =          'clsp',
    M_TOOLS_FILE_JOINER =            'tlfj',
    M_CLOSE_FILE_JOINER =            'clfj',
    
    M_TOGGLE_LOG =                  'lgtg',
    M_TOGGLE_TOOLBAR =               'tgtb',
    M_TOGGLE_INFOBAR =               'tgib',
    M_TOGGLE_COLUMN_NAME =           'tgnm',
    M_TOGGLE_COLUMN_SIZE =           'tgsz',
    M_TOGGLE_COLUMN_PACKED =         'tgpc',
    M_TOGGLE_COLUMN_RATIO =          'tgrt',
    M_TOGGLE_COLUMN_PATH =           'tgpt',
    M_TOGGLE_COLUMN_DATE =           'tgdt',
    M_TOGGLE_COLUMN_METHOD =         'tgmt',
    M_TOGGLE_COLUMN_CRC =            'tgcc',
    M_SORT_COLUMN_NAME =             'srnm',
    M_SORT_COLUMN_SIZE =             'srsz',
    M_SORT_COLUMN_PACKED =           'srpc',
    M_SORT_COLUMN_RATIO =            'srrt',
    M_SORT_COLUMN_PATH =             'srpt',
    M_SORT_COLUMN_DATE =             'srdt',
    M_SORT_COLUMN_METHOD =           'srmt',
    M_SORT_COLUMN_CRC =              'srcc',
    M_SORT_NONE =                   'sorn',
    M_SORT_ASCENDING =               'sora',
    M_SORT_DESCENDING =              'sord',
    
    M_SAVE_AS_DEFAULT =              'sadf',    
    M_SAVE_ARK_AS_DEFAULT =          'saad',
    M_SAVE_TO_ARCHIVE =              'stoa',
    M_SAVE_ARK_TO_ARCHIVE =          'sata',
    
    M_SELECTION_CHANGED =            'slch',
    M_TOOLBAR_TOGGLED =              'tbtg',
    M_INFOBAR_TOGGLED =              'ibtg',
    
    M_UNREG_WINDOW =                'unrw',
    M_OPEN_REQUESTED =               'oprq',
    M_OPEN_PART_TWO =                'opp2',
    M_CREATE_REQUESTED =             'crrq',
    M_ENTER =                       'entr',
    M_GOT_FOCUS =                   'gotf',
    M_CONTEXT_MENU =                'cntx',
    M_OPEN_FINISHED =                'opfn',
    M_EXTRACT_TO =                  'exto',
    M_EXTRACT_DONE =                'exdo',
    M_ADD =                        'addd',
    M_READY_TO_ADD =                'rdta',
    M_ADD_DONE =                    'addn',
    M_EXTRACT_SELECTED_TO =          'exst',
    M_DELETE_SELECTED =              'dlst',
    M_DELETE_DONE =                 'dldn',
    M_UPDATE_PROGRESS =              'upst',
    M_CLOSE =                       'cls!',
    M_EXTRACT_DIR_INIT_ERROR =        'exir',
    M_ARCHIVE_PATH_INIT_ERROR =       'apir',
    M_TEST_DONE =                   'tsdn',
    
    M_CONTEXT_COPY =                'cxcp',
    M_CONTEXT_SELECT =               'cxsl',
    M_CONTEXT_DESELECT =             'cxds',
    
    M_LOG_CONTEXT_CLEAR =            'lccl',
    M_LOG_CONTEXT_COPY =             'lccp',
    M_LOG_CONTEXT_SAVE =             'lcsv',
    
    M_SAVE_COMMENT =                'svcm',
        
    M_DROP_MESSAGE =                'drmg',
    M_RECENT_ITEM =                 'rick',
    M_RECENT_EXTRACT_ITEM =          'reim',
    
    M_RECENT_SPLIT_FILE =            'rcsf',
    M_RECENT_SPLIT_DIR =             'rcsd',
    
    M_SELECTION_ADDED =              'sela',
    M_REGISTER_TYPES =               'regt',
    
    M_UPDATE_INTERFACE =             'upif',
    
    M_ADDON_HELP =                  'ahlp',
    M_ADDON_CREATE =                'adca',
    
    M_PREFS_HELP =                  'pffh';

#endif /* _MSG_CONSTANTS_H */
