/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Somebody)
 *    See "License.txt" for licensing info.
*/

#include <List.h>

#include <stdlib.h>
#include <string.h>

#include "ListSorter.h"
#include "HashTable.h"
#include "ArchiveEntry.h"

//=============================================================================================================//

ListSorter::ListSorter (BList *list, CompareFunc func)
{
    m_sortList = list;
    m_compareFunc = func;
    
    int32 nItems = m_sortList->CountItems();
    unsigned long *ptrOfList = (unsigned long*)m_sortList->Items();

    QuickSort (ptrOfList, nItems);
}

//=============================================================================================================//

ListSorter::~ListSorter ()
{
}

//=============================================================================================================//

void ListSorter::QuickSort (unsigned long *list, int32 n)
{
    // Custom QuickSort algorithm: Blazing fast :)
    // Written to sort a list of pointers basically.
    // The QuickSort algorithm requires continuous memory (since it is based on indices). So we
    // cannot directly sort a BList whose pointers point to non-continuous memory (as in the case
    // of using "new") because BList.SortItems() will pass the pointers to those "new" locations.
    // So this sort, rather than sorting the pointers held by the BList, it sorts the pointers of
    // the pointers of the BList and only during the actual compare statement does it
    // double-reference the pointer for the data.
    if (n <= 1)
        return;

    SwapItems (list, 0L, n / 2);
    unsigned long pivot = list[0];
    int32 i = 1, last = 0;
    
    pivot = list[0];
    HashEntry *pvt = reinterpret_cast<HashEntry*>(pivot);
    
    for (; i < n; ++i)
        if (strcasecmp (((HashEntry*)list[i])->m_pathStr, pvt->m_pathStr) < 0)
            SwapItems (list, ++last, i);
    
    SwapItems (list, 0L, last);

    QuickSort (list, last);
    QuickSort (list + last + 1, n - last - 1);
}

//=============================================================================================================//

inline void ListSorter::SwapItems (unsigned long *list, int32 i, int32 j)
{
    unsigned long temp;
    temp = list[i];
    list[i] = list[j];
    list[j] = temp;
}

//=============================================================================================================//
