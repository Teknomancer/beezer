// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _INFO_BAR_H
#define _INFO_BAR_H

#include <View.h>

class BarberPole;
class BeezerStringView;

class InfoBar : public BView
{
    public:
        InfoBar(BRect frame, BList* slotPositions, const char* name, rgb_color backColor);
        virtual ~InfoBar();

        // Inherited hooks
        virtual void        Draw(BRect updateRect);
        virtual void        AttachedToWindow();
        virtual void        MouseDown(BPoint point);

        // Additional hooks
        virtual void        Redraw();
        virtual void        UpdateFilesDisplay(int32 selectedCount, int32 totalCount, bool setTotalCount);
        virtual void        UpdateBytesDisplay(uint32 selectedCount, uint32 totalBytes, bool setTotalBytes);
        virtual void        UpdateBy(int32 countBy, uint32 bytesBy);
        virtual void        Toggle();
        virtual bool        IsShown() const;
        virtual float       Height() const;
        BarberPole*         LoadIndicator() const;

        // Static constants
        static const float  mk_vertSpacing;
        static const float  mk_horizSpacing;

    protected:
        // Protected members
        void                RenderEdges();
        void                AddSeparatorItem(float wherex, bool finalSeparator);

        BView*              m_finalSep,
                            *m_finalSepEdge;
        BeezerStringView*   m_filesStr,
                            *m_bytesStr;
        BarberPole*         m_barberPole;
        bool                m_isHidden;
        int32               m_horizGap,
                            m_vertGap,
                            m_filesTotal,
                            m_selectedFiles;
        float               m_finalX;
        BList               m_separatorList,
                            *m_slotPositions;
        off_t               m_totalBytes,
                            m_selectedBytes;
        rgb_color           m_backColor,
                            m_lightEdge,
                            m_darkEdge1,
                            m_darkEdge2,
                            m_darkEdge3;
};

#endif /* _INFO_BAR_H */
