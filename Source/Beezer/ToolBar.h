/*
 * Copyright (c) 2011, Ramshankar (aka Teknomancer)
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

#ifndef _TOOL_BAR_H
#define _TOOL_BAR_H

#include <View.h>

class ImageButton;

class ToolBar : public BView
{
    public:
        ToolBar (BRect frame, const char *name, rgb_color backColor);
        virtual ~ToolBar ();
        
        // Inherited hooks
        virtual void        AttachedToWindow ();
        virtual void        Draw (BRect updateRect);
        virtual void        MouseDown (BPoint point);
        
        // Additional hooks
        virtual void        Redraw ();
        virtual void        AddItem (ImageButton *button);
        virtual float       AddSeparatorItem (bool finalSeparator = false);
        virtual BRect       InnerFrame ();
        virtual void        Toggle();
        virtual bool        IsShown () const;
        virtual bool        IsEnabled () const;
        virtual void        SetEnabled (bool enable);
        virtual float       Height () const;

        // Static constants
        static const float  mk_vertSpacing = 3;
        static const float  mk_horizSpacing = 2;
        static const float  mk_Border = 2;
        
    protected:
        // Protected members
        virtual void        RenderEdges ();

        // Protected members
        BView             *m_finalSep,
                        *m_finalSepEdge;
        bool               m_isEnabled;
        bool               m_isHidden;
        BPoint             m_ptToDraw;
        int32              m_nButtons,
                         m_nSeparators,
                         m_horizGap,
                         m_vertGap;
        BList              m_buttonList,
                         m_separatorList;
        rgb_color          m_backColor,
                         m_lightEdge,
                         m_darkEdge1,
                         m_darkEdge2,
                         m_darkEdge3;
        
        typedef BView       _inherited;
};

#endif /* _TOOLBAR_H */
