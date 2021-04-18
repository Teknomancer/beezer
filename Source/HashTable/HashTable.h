// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <SupportDefs.h>

class BList;
class BMessage;

class ListEntry;

class HashEntry
{
    public:
        HashEntry();
        ~HashEntry();

        HashEntry*          m_next;
        char*               m_pathStr;
        ListEntry*          m_clvItem;
};

class HashTable
{
    public:
        HashTable(int32 sizeOfTable);
        ~HashTable();

        void                DeleteTable();
        bool                Delete(HashEntry* entry);
        bool                Delete(const char* str);
        int32               CountItems() const;
        int32               TableSize() const;
        HashEntry*          Find(const char* str);
        HashEntry*          Insert(const char* str, bool *added);
        HashEntry*          Add(const char* str);
        void                FindUnder(const char* directoryPath, BList& fileList, BList& folderList) const;

        static int32        OptimalSize(int32 minSize);
        static int32        MaxCapacity();

    private:
        HashEntry*          LookUp(const char* str) const;
        int32               Hash(const char* str) const;
        void                InitializeTable();
        void                ResetCache(HashEntry* element);
        HashEntry*          ItemAt(int32 bucket) const;

        HashEntry*          m_lastFoundEntry;
        int32               m_tableSize,
                            m_itemCount;
        HashEntry**         m_table;
};

#endif /* _HASH_TABLE_H */
