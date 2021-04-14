// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _BITMAP_POOL_H
#define _BITMAP_POOL_H

#include <List.h>

class BBitmap;

class BitmapPool
{
    public:
        BitmapPool();
        ~BitmapPool();

        static BBitmap* LoadSystemVector(const char* mimestring, int width, int height);
        static BBitmap* LoadAppVector(const char* resource, int width, int height);

        BList               m_iconList;
        BBitmap*            m_folderBmp,
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
                            *m_tbarHelpBmp,
                            *m_smallAppIcon,
                            *m_largeAppIcon;

    private:
        static int32        m_runCount;
};

extern BitmapPool* _glob_bitmap_pool;

#endif /* _BITMAP_POOL_H */
