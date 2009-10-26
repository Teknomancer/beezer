/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _RULE_MGR_H
#define _RULE_MGR_H

class BList;
class BString;

class MimeRule
{
    public:
        MimeRule (const char *mime, const char *extension);
        
        // Public members
        BString                m_mime,
                            m_extension;
};

class RuleMgr
{
    public:
        RuleMgr (const char *dir, const char *ruleFile);
        ~RuleMgr ();
        
        // Public hooks
        char                *ValidateFileType (BPath *filePath) const;

    private:
        // Private hooks
        void                ReadRules (const char *dir, const char *ruleFile);

        // Private members
        BList                *m_ruleList;
        
        static int32        m_runCount;
};

#endif /* _RULE_MGR_H */
