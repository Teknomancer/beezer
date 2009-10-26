/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _LIST_SORTER_H
#define _LIST_SORTER_H

class BList;

typedef int (*CompareFunc)(const void* item1, const void* item2);

class ListSorter
{
	public:
		ListSorter (BList *listToSort, CompareFunc func);
		virtual ~ListSorter ();
		
	private:
		// Private hooks
		void				QuickSort (unsigned long *list, int32 n);
		virtual void		SwapItems (unsigned long *list, int32 i, int32 j);
		
		// Private members
		BList				*m_sortList;
		CompareFunc			m_compareFunc;
};

#endif /* _LIST_SORTER_H */
