/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _ARCHIVE_ENTRY_H
#define _ARCHIVE_ENTRY_H

class ArchiveEntry
{
	public:
		ArchiveEntry ();
		ArchiveEntry (bool dir, const char *pathStr, const char *sizeStr, const char *packedStr,
					const char *dateStr, time_t timeValue, const char *methodStr, const char *crcStr);
		~ArchiveEntry ();
		
		bool				m_isDir;
		time_t				m_timeValue;
		char				*m_nameStr,
							*m_pathStr,
							*m_sizeStr,
							*m_packedStr,
							*m_ratioStr,
							*m_dateStr,
							*m_methodStr,
							*m_crcStr,
							*m_dirStr;
};

#endif /* _ARCHIVE_ENTRY_H */
