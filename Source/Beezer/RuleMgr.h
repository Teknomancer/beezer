// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _RULE_MGR_H
#define _RULE_MGR_H

class BList;
class BString;

class MimeRule
{
    public:
        MimeRule(const char* mime, const char* extension);

        // Public members
        BString            m_mime,
                           m_extension;
};

class RuleMgr
{
    public:
        RuleMgr(BDirectory* ruleDir, const char* ruleFile);
        ~RuleMgr();

        // Public hooks
        char*               ValidateFileType(BPath* filePath) const;
        status_t            AddMimeRule(const char* mime, const char* extension);

    private:
        // Private hooks
        void               ReadRules(BEntry* rulesEntry);

        // Private members
        BList*             m_ruleList;

        static int32        m_runCount;
};

#endif /* _RULE_MGR_H */
