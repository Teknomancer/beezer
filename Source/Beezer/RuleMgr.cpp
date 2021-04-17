// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "RuleMgr.h"
#include "RuleDefaults.h"

#include <List.h>
#include <NodeInfo.h>
#include <Path.h>
#include <String.h>

#include <fstream>

struct MimeRule
{
    BString m_mime;
    BString m_extension;
};

int32 RuleMgr::m_runCount = 0;


RuleMgr::RuleMgr(BDirectory* ruleDir, const char* ruleFile)
{
    // You only need one instance per application
    if (atomic_add(&m_runCount, 1) == 0)
        m_ruleList = new BList(10L);
    else
        debugger("only one RuleMgr instance allowed/necessary");

    BEntry rulesEntry(ruleDir, ruleFile);
    // If we can't find our rules file then create a default one
    if (!rulesEntry.Exists())
    {
        BFile defaultRulesFile(&rulesEntry, B_WRITE_ONLY | B_CREATE_FILE);
        if (defaultRulesFile.InitCheck() != B_OK)
            return;

        defaultRulesFile.Write(kDefaultRules, strlen(kDefaultRules));
        defaultRulesFile.Unset();
    }

    ReadRules(&rulesEntry);
}


RuleMgr::~RuleMgr()
{
    for (int32 i = 0; i < m_ruleList->CountItems(); i++)
    {
        MimeRule* rule = (MimeRule*)m_ruleList->ItemAtFast(i);
        delete rule;
    }
    m_ruleList->MakeEmpty();
    delete m_ruleList;
}


void RuleMgr::ReadRules(BEntry* rulesEntry)
{
    char buffer[B_MIME_TYPE_LENGTH + 30];     // we don't care for extensions more than 30 characters long ;-P
    int32 const len = sizeof(buffer);
    BPath rulePath(rulesEntry);

    std::fstream f(rulePath.Path(), std::ios::in);
    if (!f)
        return;

    BString tempBuf;
    while (!f.eof())
    {
        f.getline(buffer, len, '\n');

        // skip comments and blank lines
        if (buffer[0] == '#' || buffer[0] == '\0')
            continue;

        tempBuf = buffer;
        int32 const equalIndex = tempBuf.FindFirst('=');
        int32 const lineLen = tempBuf.Length();

        if (equalIndex > 0 && equalIndex < lineLen)
        {
            // Segregate mime type into mime and extension strings
            MimeRule *rule = new MimeRule;
            if (rule)
            {
                tempBuf.CopyInto(rule->m_mime, 0L, equalIndex);
                tempBuf.CopyInto(rule->m_extension, equalIndex + 1, lineLen - equalIndex);
                m_ruleList->AddItem(rule);
            }
            else
                break;
        }
    }

    f.close();
}


status_t RuleMgr::AddMimeRule(const char *mime, const char *extension)
{
    if (mime != NULL && extension != NULL)
    {
        MimeRule *rule = new MimeRule;
        if (rule)
        {
            rule->m_mime = mime;
            rule->m_extension = extension;
            m_ruleList->AddItem(rule);

            return B_OK;
        }
    }
    return B_ERROR;
}


char* RuleMgr::ValidateFileType(BPath* filePath) const
{
    char type[B_MIME_TYPE_LENGTH + 1];
    char* mime = new char [B_MIME_TYPE_LENGTH + 1];
    BString fileName = filePath->Leaf();
    BNode node(filePath->Path());
    BNodeInfo nodeInfo(&node);
    nodeInfo.GetType(type);

    int32 extensionIndex = -1;
    for (int32 i = 0; i < m_ruleList->CountItems(); i++)
    {
        MimeRule* const rule = (MimeRule*)m_ruleList->ItemAtFast(i);
        int32 const foundIndex = fileName.IFindLast(rule->m_extension.String());

        // Check if extension matches
        if (foundIndex > 0 && foundIndex == fileName.Length() - rule->m_extension.Length())
        {
            // check if mime-type matches, if so everything is okay no need for any corrections
            if (strcmp(rule->m_mime.String(), type) == 0)
                return NULL;

            extensionIndex = i;
        }
    }

    // Check if mime matches when no extension matches -- bug fix
    if (extensionIndex == -1)
    {
        for (int32 i = 0; i < m_ruleList->CountItems(); i++)
        {
            MimeRule* const rule = (MimeRule*)m_ruleList->ItemAtFast(i);

            // Like say a .zip named "test" without any extension but with correct mime
            if (strcmp(rule->m_mime.String(), type) == 0)
            {
                strcpy(mime, rule->m_mime.String());
                return mime;
            }
        }
    }

    // No rules matched the extension for the mime type,
    // remove mime type and ask BeOS to set the correct type
    // This will also take place in case the rules file could not be opened (deleted,renamed or moved etc)
    // as CountItems() would be zero, the loop wouldn't have entered
    status_t const result = nodeInfo.SetType("application/octet-stream");
    update_mime_info(filePath->Path(), false, true, B_UPDATE_MIME_INFO_FORCE_UPDATE_ALL);

    if (result == B_OK && extensionIndex >= 0L)
    {
        // If extension matched but not mime type return a mime type from the rule's list
        // This is for archives on , say, a CD without mime-types but with extensions
        strcpy(mime, ((MimeRule*)m_ruleList->ItemAtFast(extensionIndex))->m_mime.String());
        return mime;
    }

    // If no mime-type or extension matched, return NULL
    return NULL;
}
