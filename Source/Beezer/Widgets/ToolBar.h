// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include <View.h>

class ImageButton;

class ToolBar : public BView
{
    public:
        ToolBar(BRect frame, const char* name, rgb_color backColor);
        virtual ~ToolBar();

        // Inherited hooks
        virtual void        AttachedToWindow();
        virtual void        Draw(BRect updateRect);
        virtual void        MouseDown(BPoint point);

        // Additional hooks
        virtual void        Redraw();
        virtual void        AddItem(ImageButton* button);
        virtual float       AddSeparatorItem(bool finalSeparator = false);
        virtual BRect       InnerFrame();
        virtual void        Toggle();
        virtual bool        IsShown() const;
        virtual bool        IsEnabled() const;
        virtual void        SetEnabled(bool enable);
        virtual float       Height() const;

        // Static constants
        static float const  kBorder;

    protected:
        // Protected members
        virtual void        RenderEdges();

        // Protected members
        BView*             m_finalSep,
                           *m_finalSepEdge;
        bool               m_isEnabled;
        bool               m_isHidden;
        BPoint             m_ptToDraw;
        int32              m_nButtons,
                           m_nSeparators;
        float              m_horizGap,
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
