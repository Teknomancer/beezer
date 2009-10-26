/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _BITMAP_POOL_H
#define _BITMAP_POOL_H

#include <Bitmap.h>
#include <List.h>

class BitmapPool
{
	public:
		BitmapPool ();
		~BitmapPool ();
		
		BList				m_iconList;
		BBitmap				*m_folderBmp,
							*m_executableBmp,
							*m_audioBmp,
							*m_htmlBmp,
							*m_textBmp,
							*m_archiveBmp,
							*m_packageBmp,
							*m_imageBmp,
							*m_pdfBmp,
							*m_sourceBmp,
							*m_tbarNewBmp,
							*m_tbarOpenBmp,
							*m_tbarOpenRecentBmp,
							*m_tbarPrefsBmp,
							*m_tbarToolsBmp,
							*m_tbarCloseBmp,
							*m_tbarSearchDisabledBmp,
							*m_tbarSearchBmp,
							*m_tbarExtractDisabledBmp,
							*m_tbarExtractBmp,
							*m_tbarViewDisabledBmp,
							*m_tbarViewBmp,
							*m_tbarAddDisabledBmp,
							*m_tbarAddBmp,
							*m_tbarDeleteDisabledBmp,
							*m_tbarDeleteBmp,
							*m_smallAppIcon,
							*m_largeAppIcon;
	
	private:
		static int32		m_runCount;
};

extern BitmapPool *_glob_bitmap_pool;

#endif /* _BITMAP_POOL_H */
