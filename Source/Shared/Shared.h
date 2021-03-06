// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _SHARED_H
#define _SHARED_H

// NOTE:
// Everytime you add a new constant to this file,
// make sure it is NOT already defined by doing a text search. It
// would be extremely difficult to trace bugs arising from duplicate
// constants (as compilers can't pick them up)

#define BZR_CANCEL                    'acna'
#define BZR_NOT_SUPPORTED             'anso'
#define BZR_DONE                      'aard'
#define BZR_ERROR                     'aerr'
#define BZR_BINARY_MISSING            'axnf'
#define BZR_OPTIONAL_BINARY_MISSING   'obfm'
#define BZR_ERRSTREAM_FOUND           'errf'
#define BZR_UNKNOWN                   'eruk'
#define BZR_PASSWORD_ERROR            'pwer'

#define BZR_EXTRACT_DIR_INIT_ERROR    'edie'
#define BZR_ARCHIVE_PATH_INIT_ERROR   'apie'
#define BZR_CANCEL_ARCHIVER           'cana'
#define BZR_UPDATE_PROGRESS           'upda'

#define BZR_MENUITEM_SELECTED         'amis'

#define M_LAUNCH_TRACKER_ADDON        'ltad'

const char* const kPath =             "path";
const int64 kMaxFragmentCount =       32767;

#define K_TRACKER_SIGNATURE           "application/x-vnd.Be-TRAK"
#define K_APP_SIGNATURE               "application/x-vnd.Ram-Beezer"
#define K_JOINER_STUB_SIGNATURE       "application/x-vnd.Ram-FileJoiner"

// Digital unit multiplier constants, move elsewhere if needed.
const uint64 kEiBSize = 0x1000000000000000ULL;
const uint64 kPiBSize = 0x4000000000000ULL;
const uint64 kTiBSize = 0x10000000000ULL;
const uint64 kGiBSize = 0x40000000ULL;
const uint64 kMiBSize = 0x100000ULL;
const uint64 kKiBSize = 0x400ULL;

#endif /* _SHARED_H */
