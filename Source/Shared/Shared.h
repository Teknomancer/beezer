/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _SHARED_H
#define _SHARED_H

// IMPORTANT NOTE:: Everytime you add a new constant to this file,
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
#define K_JOINER_STUB_SIGNATURE       "application/x-vnd.Ram-Joiner"


#endif /* _SHARED_H */
