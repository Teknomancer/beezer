/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
