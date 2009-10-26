/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <SupportDefs.h>
#include <Message.h>

class BList;

class ListEntry;

// Predefined table sizes - prime numbers
const int32 kTableSizes[] =
{
    1021, 1597, 2039, 4093, 8191, 16381, 32749, 65521, 131071, 262139,
    524287, 1048573, 2097143, 4194301, 8388593, 16777213, 33554393, 67108859,
    134217689, 268435399, 536870909, 0
};

class HashEntry
{
    public:
        HashEntry();
        ~HashEntry();
        
        const char         *m_pathStr;
        ListEntry          *m_clvItem;
        HashEntry          *m_next;
};

class HashTable
{
    public:
        HashTable (int32 sizeOfTable);
        ~HashTable ();
        
        void                DeleteTable ();
        bool                IsFound (const char *str);
        bool                Delete (HashEntry *entry);
        bool                Delete (char *str);
        int32               CountItems () const;
        int32               TableSize () const;
        HashEntry          *Insert (char *str, bool *wasFound = NULL, bool copyInput = true);
        HashEntry          *Find (const char *str);
        HashEntry          *ItemAt (int32 bucket) const;
        HashEntry          *LastAddedEntry () const;
        HashEntry          *ForceInsert (const char *str, bool copyInput = true);
        int32               FindUnder (BMessage *message, const char *fieldName, const char *directoryPath,
                                    BList *fileList, BList *folderList);
        static int32        OptimalSize (int32 minSize);
        
    protected:
        HashEntry          *LookUp (const char *str, bool insert, bool *wasFound = NULL, bool copyInput = true);
        int32               Hash (const char *str) const;
        void                InitializeTable ();
        void                ResetCache (HashEntry *element);

        int32               m_tableSize,
                          m_itemCount;

        HashEntry         **m_table,
                         *m_lastAddedEntry,
                         *m_lastFoundEntry;
};

#endif /* _HASH_TABLE_H */
