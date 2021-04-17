// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _RULE_MGR_H
#define _RULE_MGR_H

#include <SupportDefs.h>

class BEntry;
class BList;
class BDirectory;
class BPath;

class RuleMgr
{
    public:
        RuleMgr(BDirectory* ruleDir, const char* ruleFile);
        ~RuleMgr();

        char*               ValidateFileType(BPath* filePath) const;
        status_t            AddMimeRule(const char* mime, const char* extension);

    private:
        void                ReadRules(BEntry* rulesEntry);

        static int32        m_runCount;

        BList*              m_ruleList;
};

#endif /* _RULE_MGR_H */
