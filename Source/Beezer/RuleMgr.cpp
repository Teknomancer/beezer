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

#include <List.h>
#include <String.h>
#include <Debug.h>
#include <Path.h>
#include <NodeInfo.h>

#include <fstream>

#include "RuleMgr.h"

//=============================================================================================================//

MimeRule::MimeRule (const char *mime, const char *extension)
{
    m_mime = mime;
    m_extension = extension;
}

//=============================================================================================================//
//=============================================================================================================//
//=============================================================================================================//

int32 RuleMgr::m_runCount = 0;

RuleMgr::RuleMgr (const char *dir, const char *ruleFile)
{
    // You only need one instance per application
    if (atomic_add (&m_runCount, 1) == 0)
        m_ruleList = new BList (10L);
    else
        debugger ("only one RuleMgr instance allowed/necessary");
    
    ReadRules (dir, ruleFile);
}

//=============================================================================================================//

RuleMgr::~RuleMgr ()
{
    delete m_ruleList;
}

//=============================================================================================================//

void RuleMgr::ReadRules (const char *dir, const char *ruleFile)
{
    int32 len = B_MIME_TYPE_LENGTH+30;    // we don't care for extensions more than 30 characters long ;-P
    char buffer[len];
    BString fullPath = dir;
    fullPath << '/' << ruleFile;
    fullPath.ReplaceAll ("//", "/");    // Incase of missing dirs etc
    
    std::fstream f (fullPath.String(), std::ios::in);
    if (!f)
        return;
    
    while (!f.eof())
    {
        f.getline (buffer, len, '\n');
        
        // skip comments and blank lines
        if (buffer[0] == '#' || buffer[0] == '\0')
           continue;

        BString tempBuf = buffer;
        BString mime, extension;
        int32 equalIndex = tempBuf.FindFirst ('=');
        int32 lineLen = tempBuf.Length();
        
        if (equalIndex > 0 && equalIndex < lineLen)
        {
           // abcd=defg           assume "abcd" is mimetype and "defg" is extension
           // 012345678
           // Segregate mime type into mime and extension strings
           tempBuf.CopyInto (mime, 0L, equalIndex);
           tempBuf.CopyInto (extension, equalIndex + 1, lineLen - equalIndex);
           m_ruleList->AddItem (new MimeRule (mime.String(), extension.String()));
        }
    }
    
    f.close();
}

//=============================================================================================================//

char* RuleMgr::ValidateFileType (BPath *filePath) const
{
    char type[B_MIME_TYPE_LENGTH+1];
    char *mime = new char [B_MIME_TYPE_LENGTH+1];
    BString fileName = filePath->Leaf();
    BNode node (filePath->Path());
    BNodeInfo nodeInfo (&node);
    nodeInfo.GetType (type);

    int32 extensionIndex = -1;
    for (int32 i = 0; i < m_ruleList->CountItems(); i++)
    {
        MimeRule *rule = (MimeRule*)m_ruleList->ItemAtFast(i);
        int32 foundIndex = fileName.IFindLast (rule->m_extension.String());

        // xyz.zip           .zip
        // 0123457           0123
        //    (len7)           (len4)
        
        // Check if extension matches
        if (foundIndex > 0 && foundIndex == fileName.Length() - rule->m_extension.Length())
        {
           // check if mime-type matches, if so everything is okay no need for any corrections
           if (strcmp (rule->m_mime.String(), type) == 0)
               return NULL;
           else
               extensionIndex = i;
        }
    }

    // Check if mime matches when no extension matches -- bug fix
    if (extensionIndex == -1)
    {
        for (int32 i = 0; i < m_ruleList->CountItems(); i++)
        {
           MimeRule *rule = (MimeRule*)m_ruleList->ItemAtFast(i);
    
           // Like say a .zip named "test" without any extension but with correct mime
           if (strcmp (rule->m_mime.String(), type) == 0)
           {
               strcpy (mime, rule->m_mime.String());
               return mime;
           }
        }
    }
        
    // No rules matched the extension for the mime type,
    // remove mime type and ask BeOS to set the correct type
    // This will also take place in case the rules file could not be opened (deleted,renamed or moved etc)
    // as CountItems() would be zero, the loop wouldn't have entered
    status_t result = nodeInfo.SetType ("application/octet-stream");
    update_mime_info (filePath->Path(), false, true, true);
    
    if (result != B_OK && extensionIndex >= 0L)
    {
        // If extension matched but not mime type return a mime type from the rule's list
        // This is for archives on , say, a CD without mime-types but with extensions
        strcpy (mime, ((MimeRule*)m_ruleList->ItemAtFast (extensionIndex))->m_mime.String());
        return mime;
    }
    
    // If no mime-type or extension matched, return NULL
    return NULL;
}

//=============================================================================================================//
