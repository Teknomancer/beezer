// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _BEVEL_VIEW_H
#define _BEVEL_VIEW_H

#include <View.h>

class BevelView : public BView
{
    public:
        enum bevel_type
        {
            INSET,
            OUTSET,
            BULGE,
            DEEP,
            NO_BEVEL
        };

        BevelView(BRect frame, const char* name, bevel_type bevelMode, uint32 resizeMask = B_FOLLOW_LEFT,
                  uint32 flags = B_WILL_DRAW);

        // Inherited hooks
        virtual void        Draw(BRect updateRect);
        virtual void        FrameResized(float newWidth, float newHeight);
        virtual void        AttachedToWindow();

        // Additional hooks
        float               EdgeThickness() const;

        static float const  kInsetThickness;
        static float const  kOutsetThickness;
        static float const  kBulgeThickness;
        static float const  kDeepThickness;
        static float const  kNoBevelThickness;

    private:
        BRect               m_cachedRect;
        bevel_type          m_bevelType;
        rgb_color           m_darkEdge1,
                            m_darkEdge2,
                            m_lightEdge;
        float               m_edgeThickness;
};

#endif /* _BEVEL_VIEW_H */
