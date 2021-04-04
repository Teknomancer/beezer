// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _BEVEL_VIEW_H
#define _BEVEL_VIEW_H

#include <View.h>

enum BevelThickness
{
    btInsetThickness = 1,
    btOutsetThickness = 1,
    btBulgeThickness = 2,
    btDeepThickness = 2,
    btNoBevelThickness = 0
};

enum BevelType
{
    btInset,
    btDeep,
    btOutset,
    btBulge,
    btNoBevel
};

class BevelView : public BView
{
    public:
        BevelView(BRect frame, const char* name, BevelType bevelMode, uint32 resizeMask = B_FOLLOW_LEFT,
                  uint32 flags = B_WILL_DRAW);

        // Inherited hooks
        virtual void        Draw(BRect updateRect);
        virtual void        FrameResized(float newWidth, float newHeight);
        virtual void        AttachedToWindow();

        // Additional hooks
        float                EdgeThickness() const;

    private:
        // Private members
        BRect               m_cachedRect;
        BevelType           m_bevelType;
        rgb_color           m_darkEdge1,
                            m_darkEdge2,
                            m_lightEdge;
        float               m_edgeThickness;
};

#endif /* _BEVEL_VIEW_H */
