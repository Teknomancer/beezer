// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _BEVEL_VIEW_H
#define _BEVEL_VIEW_H

#include <View.h>

class BevelView : public BView
{
    public:
        enum BevelType
        {
            INSET,
            OUTSET,
            BULGE,
            DEEP,
            NO_BEVEL
        };

        BevelView(BRect frame, const char* name, BevelType bevelMode, uint32 resizeMask = B_FOLLOW_LEFT,
                  uint32 flags = B_WILL_DRAW);

        // Inherited hooks
        virtual void        Draw(BRect updateRect);
        virtual void        FrameResized(float newWidth, float newHeight);
        virtual void        AttachedToWindow();

        // Additional hooks
        float               EdgeThickness() const;

        static const float  kInsetThickness;
        static const float  kOutsetThickness;
        static const float  kBulgeThickness;
        static const float  kDeepThickness;
        static const float  kNoBevelThickness;

    private:
        BRect               m_cachedRect;
        BevelType           m_bevelType;
        rgb_color           m_darkEdge1,
                            m_darkEdge2,
                            m_lightEdge;
        float               m_edgeThickness;
};

#endif /* _BEVEL_VIEW_H */
