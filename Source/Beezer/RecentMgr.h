/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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

#ifndef _RECENT_MGR_H
#define _RECENT_MGR_H


class BLocker;
class BPopUpMenu;

class Preferences;

enum RecentItemType
{
    ritFile,
    ritFolder,
    ritAny,
};

extern BLocker _recent_locker;

class RecentMgr
{
    public:
        RecentMgr (int32 maxNumPaths, Preferences *pref, RecentItemType itemType, bool showFullPath);
        virtual ~RecentMgr ();

        // Public hooks
        void                AddPath (const char *path);
        void                RemovePath (const char *path);
        void                SavePrefs ();
        void                LoadPrefs ();
        void                SetCommand (uint32 command);
        void                SetMaxPaths (int32 maxNumPaths);
        void                SetShowFullPath (bool showFullPath);

        BMenu              *BuildMenu (const char *menuName, const char *fieldName, BHandler *targetForItems);
        BPopUpMenu         *BuildPopUpMenu (const char *menuName, const char *fieldName,
                                        BHandler *targetForItems);
        void                UpdateMenu (BMenu *recentMenu, const char *fieldName, BHandler *targetForItems);

    private:
        // Private hooks
        void                FillMenu (BMenu *menu, const char *fieldName, BHandler *targetForItems);

        // Static members
        static int32        m_maxInternalCount;

        // Private members
        Preferences        *m_prefs;
        BList               m_paths;
        bool                m_showFullPath;
        int32               m_maxNumPaths;
        uint32              m_command;
        RecentItemType       m_type;
};

#endif /* _RECENT_MGR_H */
