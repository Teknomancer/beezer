// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Chris Roberts.
// All rights reserved.

#ifndef _ARCHIVER_MENU_ITEM_H
#define _ARCHIVER_MENU_ITEM_H

#include <MenuItem.h>


class ArchiverMenuItem : public BMenuItem
{
    public:
		ArchiverMenuItem(const char* key, const char* label, BMessage* message = NULL)
			: BMenuItem(label, message),
			m_keyString(new BString(key))
			{};

        ~ArchiverMenuItem()
        {
            delete m_keyString;
        }

        const char* Key()
        {
            return m_keyString->String();
        }

        BString* m_keyString;
};

#endif /* _ARCHIVER_MENU_ITEM_H */
