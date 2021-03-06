// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "HashTable.h"
#include "ListEntry.h"

#include <cassert>
#include <cstdlib>

// Predefined table sizes of prime numbers.
// Must be in ascending order.
static int32 const kTableSizes[] =
{
    1021, 1597, 2039, 4093, 8191, 16381, 32749,
    65521, 131071, 262139, 524287, 1048573
};


HashEntry::HashEntry()
    : m_next(NULL),
    m_pathStr(NULL),
    m_clvItem(NULL)
{
}


HashEntry::~HashEntry()
{
    free(m_pathStr);
    m_pathStr = NULL;
    m_clvItem = NULL;  // will be deleted by MainWindow
    m_next = NULL;
}


HashTable::HashTable(int32 sizeOfTable)
    : m_lastFoundEntry(NULL),
    m_tableSize(sizeOfTable),
    m_itemCount(0),
    m_table(new HashEntry*[m_tableSize])
{
    Init();
}


HashTable::~HashTable()
{
    DeleteAll();
    delete[] m_table;
    m_table = NULL;
}


void HashTable::Init()
{
    // Important we initialize table with NULL pointers
    memset(m_table, 0, m_tableSize * sizeof(HashEntry*));
    assert(m_itemCount == 0);
}


int32 HashTable::MaxCapacity()
{
    return kTableSizes[B_COUNT_OF(kTableSizes) - 1];
}


int32 HashTable::OptimalSize(int32 minSize)
{
    for (int32 i = 0; i < (int32)B_COUNT_OF(kTableSizes); i++)
        if (kTableSizes[i] >= minSize)
            return kTableSizes[i];
    return HashTable::MaxCapacity();
}


void HashTable::DeleteAll()
{
    // Delete all the hash table items
    for (int64 bucket = 0LL; bucket < m_tableSize; bucket++)
        for (HashEntry* item = m_table[bucket]; item != NULL; )
        {
            // Don't simply delete item as we will lose it's m_next field, store m_next then delete
            HashEntry* next = item->m_next;

            // Reset cache so we don't point to stale/deleted item.
            ResetCache(item);

            delete item;
            item = next;
        }

    m_itemCount = 0L;
    Init();
}


int32 HashTable::CountItems() const
{
    return m_itemCount;
}


int32 HashTable::Hash(const char* str) const
{
    // Hash function that is used to divides strings into "m_tableSize" buckets
    unsigned int h = 0;
    while (*str)
        h = (h << 1) ^ *str++;

    return (h % m_tableSize);
}


HashEntry* HashTable::Add(const char* str)
{
    // Adds 'str' to the hash table without checking if it already exists.
    int32 const hashValue = Hash(str);
    HashEntry* item = new HashEntry();
    item->m_pathStr = strdup(str);
    item->m_next = m_table[hashValue];
    m_table[hashValue] = item;
    ++m_itemCount;

    return item;
}


HashEntry* HashTable::LookUp(const char* str) const
{
    int32 const hashValue = Hash(str);
    for (HashEntry* item = m_table[hashValue]; item != NULL; item = item->m_next)
        if (strcmp(item->m_pathStr, str) == 0)
            return item;
    return NULL;
}


void HashTable::FindUnder(const char* directoryPath, BList& fileList, BList& folderList) const
{
    // Add all hashitems which is under the specified directoryPath,
    // eg: if directory path is be/book, then add be/book/* (everything under it)
    // Could be an expensive operation since entire table is scanned
    for (int64 bucket = 0LL; bucket < m_tableSize; bucket++)
        for (HashEntry* item = m_table[bucket]; item != NULL; item = item->m_next)
        {
            BString buf = item->m_pathStr;
            if (buf.FindFirst(directoryPath) >= 0L)
            {
                buf.ReplaceAll("*", "\\*");
                // Don't add filenames - this is because tar will get stuck up when there are
                // duplicate entries (same filenames) as samenames must be supplied to tar only once
                if (item->m_clvItem->IsSuperItem())
                    folderList.AddItem((void*)item->m_clvItem);
                else
                    fileList.AddItem((void*)item->m_clvItem);
            }
        }
}


HashEntry* HashTable::Find(const char* str)
{
    // Cached find - slight performance gain
    if (m_lastFoundEntry && strcmp(m_lastFoundEntry->m_pathStr, str) == 0)
        return m_lastFoundEntry;

    HashEntry* found = LookUp(str);
    if (found != NULL)
        m_lastFoundEntry = found;
    return found;
}


HashEntry* HashTable::Insert(const char* str, bool *added)
{
    if (Find(str) == NULL)
    {
        *added = true;
        return Add(str);
    }

    *added = false;
    return NULL;
}


bool HashTable::Delete(HashEntry* item)
{
    return Delete(item->m_pathStr);
}


bool HashTable::Delete(const char* str)
{
    int32 const hashValue = Hash(str);

    HashEntry* prevItem = NULL;
    for (HashEntry* item = m_table[hashValue]; item != NULL; item = item->m_next)
    {
        if (strcmp(item->m_pathStr, str) == 0)
        {
            if (prevItem!= NULL)
                prevItem->m_next = item->m_next;
            else
                m_table[hashValue] = item->m_next;

            // Reset the cache (so we don't point to stale/deleted items).
            ResetCache(item);

            delete item;
            item = NULL;
            --m_itemCount;
            return true;
        }
        prevItem = item;
    }

    return false;
}


HashEntry* HashTable::ItemAt(int32 bucket) const
{
    return m_table[bucket];
}


int32 HashTable::TableSize() const
{
    return m_tableSize;
}


void HashTable::ResetCache(HashEntry* item)
{
    if (m_lastFoundEntry == item)
        m_lastFoundEntry = NULL;
}
