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
 * -> Neither the name of the RAMSHANKAR nor the names of its contributors may
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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <SupportDefs.h>

#include "ArchiveEntry.h"
#include "AppUtils.h"

//=============================================================================================================//

ArchiveEntry::ArchiveEntry()
{
}

//=============================================================================================================//

ArchiveEntry::ArchiveEntry (bool dir, const char *pathStr, const char *sizeStr, const char *packedStr,
                  const char *dateStr, time_t timeValue, const char *methodStr, const char *crcStr)
{
    m_isDir = dir;
    m_nameStr = strdup (LeafFromPath (pathStr));        // Never call FinalPathComponent here - only use
    m_pathStr = strdup (pathStr);                      // LeafFromPath() bug fixed
    
    // Get path of parent directory
    int32 len = strlen(pathStr) - strlen(m_nameStr);
    m_dirStr = (char*)malloc((len +1) * sizeof(char));
    strncpy (m_dirStr, pathStr, len);
    m_dirStr[len] = 0;
    
    m_dateStr = strdup (dateStr);
    m_timeValue = timeValue;
    m_sizeStr = strdup (sizeStr);
    m_packedStr = strdup (packedStr);

    float ratio = 100 * (atof (sizeStr) - atof (packedStr));
    if (ratio > 0)    // to prevent 0 bytes 0 packed files (like those in BeBookmarks.zip)
        ratio /= atof (sizeStr);
    else if (ratio < 0)
        ratio = 0;
    
    m_ratioStr = (char*)malloc (8 * sizeof(char));
    sprintf (m_ratioStr, "%.1f%%", ratio);
    
    if (methodStr)
        m_methodStr = strdup (methodStr);
    else
        m_methodStr = NULL;
    
    if (crcStr)
        m_crcStr = strdup (crcStr);
    else
        m_crcStr = NULL;
}

//=============================================================================================================//

ArchiveEntry::~ArchiveEntry()
{
    if (m_nameStr) free (m_nameStr);
    if (m_pathStr) free (m_pathStr);
    if (m_dirStr) free (m_dirStr);
    if (m_sizeStr) free (m_sizeStr);
    if (m_packedStr) free (m_packedStr);
    if (m_ratioStr) free (m_ratioStr);
    if (m_dateStr) free (m_dateStr);
    if (m_methodStr) free (m_methodStr);
    if (m_crcStr) free (m_crcStr);
}

//=============================================================================================================//
