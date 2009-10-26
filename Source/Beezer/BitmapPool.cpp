/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <Message.h>
#include <Roster.h>
#include <Application.h>
#include <Resources.h>

#include "LocalUtils.h"
#include "BitmapPool.h"

BitmapPool *_glob_bitmap_pool;
int32 BitmapPool::m_runCount = 0;

//=============================================================================================================//

BitmapPool::BitmapPool ()
{
	// The purpose of this BitmapPool is to maintain a global list of bitmap images, inorder 
	// to avoid bitmap redundancy i.e. holding multiple copies of bitmaps for, say, each window.
	// Now all bitmaps are allocated on the heap - once - and only one copy is present in memory at any time
	// for an instance of the application. The application is not intended for multiple launches...
	// and thus we won't take effort to create "area"s in memory and try sharing, a waste of time
	
	// The BitmapPool object is a global object referenced through "_glob_bitmap_pool" pointer.
	// _glob_bitmap_pool is created on heap by Beezer.cpp and freed by the same. Other clases/files
	// simply access (but don't modify) the bitmaps from _glob_bitmap_pool
	
	// The reason why we alloc bitmaps here is because IF we make AppUtils (globally)
	// make these allocs the app crashes as BBitmap requires be_app to be running BEFORE use of BBitmap
	// whereas AppUtils is a seperate class initialized before be_app, it is also used in Archiver
	
	if (atomic_add (&m_runCount, 1) != 0)
	{
		debugger ("only one BitmapPool instance allowed/necessary");
		return;
	}
	
	m_folderBmp = ResBitmap ("Img:Folder");
	m_executableBmp = ResBitmap ("Img:Executable");
	m_htmlBmp = ResBitmap ("Img:HTML");
	m_textBmp = ResBitmap ("Img:Text");
	m_sourceBmp = ResBitmap ("Img:Source");
	m_audioBmp = ResBitmap ("Img:Audio");
	m_archiveBmp = ResBitmap ("Img:Archive");
	m_packageBmp = ResBitmap ("Img:Package");
	m_pdfBmp = ResBitmap ("Img:PDF");
	m_imageBmp = ResBitmap ("Img:Image");

	// The **ORDER** in which the bmps are added to the BList is critical
	m_iconList.AddItem ((void*)m_folderBmp);
	m_iconList.AddItem ((void*)m_executableBmp);
	m_iconList.AddItem ((void*)m_htmlBmp);
	m_iconList.AddItem ((void*)m_textBmp);
	m_iconList.AddItem ((void*)m_sourceBmp);
	m_iconList.AddItem ((void*)m_audioBmp);
	m_iconList.AddItem ((void*)m_archiveBmp);
	m_iconList.AddItem ((void*)m_packageBmp);
	m_iconList.AddItem ((void*)m_pdfBmp);
	m_iconList.AddItem ((void*)m_imageBmp);
	
	m_tbarNewBmp = ResBitmap ("Img:New");
	m_tbarOpenBmp = ResBitmap ("Img:Open");
	m_tbarOpenRecentBmp = ResBitmap ("Img:OpenRecent");
	m_tbarPrefsBmp = ResBitmap ("Img:Prefs");
	m_tbarToolsBmp = ResBitmap ("Img:Tools");
	m_tbarCloseBmp = ResBitmap ("Img:Close");
	m_tbarSearchDisabledBmp = ResBitmap ("Img:SearchDisabled");
	m_tbarSearchBmp = ResBitmap ("Img:Search");
	m_tbarExtractDisabledBmp = ResBitmap ("Img:ExtractDisabled");
	m_tbarExtractBmp = ResBitmap ("Img:Extract");
	m_tbarViewDisabledBmp = ResBitmap ("Img:ViewDisabled");
	m_tbarViewBmp = ResBitmap ("Img:View");
	m_tbarAddDisabledBmp = ResBitmap ("Img:AddDisabled");
	m_tbarAddBmp = ResBitmap ("Img:Add");
	m_tbarDeleteDisabledBmp = ResBitmap ("Img:DeleteDisabled");
	m_tbarDeleteBmp = ResBitmap ("Img:Delete");

	// Archived bitmap (MICN - mini icon) get from resource
	size_t size;
	BMessage msg;
	const char *buf = NULL;
	buf = reinterpret_cast<const char*>(be_app->AppResources()->LoadResource ('MICN', "BEOS:M:STD_ICON", &size));
	m_smallAppIcon = new BBitmap (BRect (0, 0, 15, 15), B_CMAP8);
	m_smallAppIcon->SetBits (buf, 16*16, 0, B_CMAP8);
	delete[] buf;
	
	buf = reinterpret_cast<const char*>(be_app->AppResources()->LoadResource ('ICON', "BEOS:L:STD_ICON", &size));
	m_largeAppIcon = new BBitmap (BRect (0, 0, 31, 31), B_CMAP8);
	m_largeAppIcon->SetBits (buf, 32*32, 0, B_CMAP8);
	delete[] buf;
}

//=============================================================================================================//

BitmapPool::~BitmapPool ()
{
	// This is fun :)
	delete m_folderBmp;
	delete m_executableBmp;
	delete m_audioBmp;
	delete m_htmlBmp;
	delete m_textBmp;
	delete m_archiveBmp;
	delete m_packageBmp;
	delete m_imageBmp;
	delete m_pdfBmp;
	delete m_sourceBmp;
	
	delete m_tbarNewBmp;
	delete m_tbarOpenBmp;
	delete m_tbarOpenRecentBmp;
	delete m_tbarPrefsBmp;
	delete m_tbarToolsBmp;
	delete m_tbarCloseBmp;
	delete m_tbarSearchDisabledBmp;
	delete m_tbarSearchBmp;
	delete m_tbarExtractDisabledBmp;
	delete m_tbarExtractBmp;
	delete m_tbarViewDisabledBmp;
	delete m_tbarViewBmp;
	delete m_tbarAddDisabledBmp;
	delete m_tbarAddBmp;
	delete m_tbarDeleteDisabledBmp;
	delete m_tbarDeleteBmp;
	
	delete m_smallAppIcon;
	delete m_largeAppIcon;
}

//=============================================================================================================//
