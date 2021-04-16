// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Chris Roberts.
// All rights reserved.

#ifndef _KEYED_MENU_ITEM_H
#define _KEYED_MENU_ITEM_H

#include <MenuItem.h>


class KeyedMenuItem : public BMenuItem
{
    public:
        KeyedMenuItem(const char* key, const char* label, BMessage* message = NULL)
            : BMenuItem(label, message),
            m_keyString(new BString(key))
            {};

        ~KeyedMenuItem()
        {
            delete m_keyString;
        }

        const char* Key()
        {
            return m_keyString->String();
        }

        static status_t ArchiveMenu(BMenu* menu, BMessage& message)
        {
            for (int32 x = 0; x < menu->CountItems(); x++)
            {
                BMenu* subMenu = menu->ItemAt(x)->Submenu();
                if (subMenu != NULL)
                    ArchiveMenu(subMenu, message); // recurse
                else
                {
                    KeyedMenuItem* item = dynamic_cast<KeyedMenuItem*>(menu->ItemAt(x));
                    if (item != NULL)
                        message.AddBool(item->Key(), item->IsMarked());
                }
            }

            return B_OK;
        }

        BString* m_keyString;
};

#endif /* _KEYED_MENU_ITEM_H */
