// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "HashTable.h"
#include "ListEntry.h"

#include <Message.h>

#include <string.h>



HashEntry::HashEntry()
{
    m_pathStr = NULL;

    m_clvItem = NULL;
    m_next = NULL;
}



HashEntry::~HashEntry()
{
    if (m_pathStr != NULL)
        delete[] m_pathStr;
    // m_clvItem will be deleted by MainWindow
}




HashTable::HashTable(int32 sizeOfTable)
{
    m_tableSize = sizeOfTable;
    m_table = new HashEntry*[m_tableSize];
    m_itemCount = 0L;

    InitializeTable();
}



HashTable::~HashTable()
{
    DeleteTable();
}



int32 HashTable::OptimalSize(int32 minSize)
{
    // Static function that returns an optimal prime-size given a desired size
    for (int32 i = 0; kTableSizes[i] != 0; i++)
        if (kTableSizes[i] >= minSize)
            return kTableSizes[i];

    return 0;
}



void HashTable::InitializeTable()
{
    // Very important we initialize NULL (zero) pointers so our "for" loops works
    memset((void*)m_table, 0, m_tableSize * sizeof(HashEntry*));
    m_lastAddedEntry = NULL;
    m_lastFoundEntry = NULL;
}



void HashTable::DeleteTable()
{
    // Delete all the hash table elements
    for (int64 bucket = 0LL; bucket < m_tableSize; bucket++)
        for (HashEntry * element = m_table[bucket]; element != NULL;)
        {
            // Don't simply delete element as we will lose it's m_next field, store m_next then delete
            HashEntry* next = element->m_next;

            // Reset cached entries
            ResetCache(element);
            delete element;
            element = next;
        }

    m_itemCount = 0L;
    delete[] m_table;
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



HashEntry* HashTable::ForceInsert(const char* str, bool copyInput)
{
    // Never check if str exists already in the hashtable, simply insert whats given
    int32 hashValue = Hash(str);
    HashEntry* bucket = new HashEntry();
    if (copyInput == true)
    {
        bucket->m_pathStr = new char [strlen(str) + 1];
        strcpy(const_cast<char*>(bucket->m_pathStr), str);
    }
    else
        bucket->m_pathStr = str;

    m_lastAddedEntry = bucket;

    bucket->m_next = m_table[hashValue];
    m_table[hashValue] = bucket;
    m_itemCount++;

    return bucket;
}



HashEntry* HashTable::LookUp(const char* str, bool insert, bool* wasFound, bool copyInput)
{
    int32 hashValue = Hash(str);
    HashEntry* bucket = NULL;
    if (wasFound != NULL)
        *wasFound = false;

    // Lookup node in table
    for (bucket = m_table[hashValue]; bucket != NULL; bucket = bucket->m_next)
    {
        if (strcmp(bucket->m_pathStr, str) == 0)
        {
            if (wasFound != NULL)
                *wasFound = true;

            return bucket;
        }
    }

    // "str" is not found, if insert is needed then add it to the table
    if (insert == false)
        return NULL;

    return ForceInsert(str, copyInput);
}



int32 HashTable::FindUnder(BMessage* message, const char* fieldName, const char* directoryPath,
                           BList* fileList, BList* folderList)
{
    // Add all hashitems which is under the specified directoryPath,
    // eg: if directory path is be/book, then add be/book/* (everything under it)
    // Could be an expensive operation since entire table is scanned
    int32 count = 0L;
    message->AddString(fieldName, directoryPath);
    count++;
    for (int64 bucket = 0LL; bucket < m_tableSize; bucket++)
        for (HashEntry * element = m_table[bucket]; element != NULL;)
        {
            BString buf = element->m_pathStr;
            if (buf.FindFirst(directoryPath) >= 0L)
            {
                BString buf = element->m_pathStr;
                buf.ReplaceAll("*", "\\*");
                // Don't add filenames - this is because tar will get stuck up when there are
                // duplicate entries (same filenames) as samenames must be supplied to tar only
                // once
                //message->AddString (fieldName, buf.String());
                //count++;
                if (element->m_clvItem->IsSuperItem())
                    folderList->AddItem((void*)element->m_clvItem);
                else
                    fileList->AddItem((void*)element->m_clvItem);
            }

            element = element->m_next;
        }
    return count;
}



bool HashTable::IsFound(const char* str)
{
    // Just find if "str" is found, don't add
    bool isFound;
    LookUp(str, false, &isFound, false);

    return isFound;
}



HashEntry* HashTable::Find(const char* str)
{
    // Cached find - performance gain
    if (m_lastFoundEntry)
        if (strcmp(m_lastFoundEntry->m_pathStr, str) == 0)
            return m_lastFoundEntry;

    // Cache result of current find, return what is found
    m_lastFoundEntry = LookUp(const_cast<char*>(str), false, NULL, false);
    return m_lastFoundEntry;
}



HashEntry* HashTable::Insert(char* str, bool* wasFound, bool copyItem)
{
    return LookUp(str, true, wasFound, copyItem);
}



bool HashTable::Delete(HashEntry* entry)
{
    // Delete the given entry without destroying links
    return Delete((char*)entry->m_pathStr);
}



bool HashTable::Delete(char* str)
{
    // Find the item with the given "str" & remove not destroying the links between the elements
    HashEntry* targetElement = NULL, *prevElement = NULL;
    int32 hashValue = Hash(str);

    for (targetElement = m_table[hashValue]; targetElement != NULL; targetElement = targetElement->m_next)
    {
        if (strcmp(targetElement->m_pathStr, str) == 0)
        {
            // If there is a prev element connect it to the next one, else make next element as top of bucket
            if (prevElement != NULL)
                prevElement->m_next = targetElement->m_next;
            else
                m_table[hashValue] = targetElement->m_next;

            // Reset cached entries -- very important
            ResetCache(targetElement);
            delete targetElement;
            targetElement = NULL;

            m_itemCount--;
            return true;
        }
        //prevElement = m_table[hashValue];
        prevElement = targetElement;           // bug-fix
    }

    return false;
}



HashEntry* HashTable::LastAddedEntry() const
{
    return m_lastAddedEntry;
}



HashEntry* HashTable::ItemAt(int32 bucket) const
{
    return m_table[bucket];
}



int32 HashTable::TableSize() const
{
    return m_tableSize;
}



void HashTable::ResetCache(HashEntry* element)
{
    if (element == m_lastAddedEntry)
        m_lastAddedEntry = NULL;
    if (element == m_lastFoundEntry)
        m_lastFoundEntry = NULL;
}
