// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _UI_CONSTANTS_H
#define _UI_CONSTANTS_H

const rgb_color  K_HIGHLIGHT_GREY =           {225, 225, 225, 255},
                 K_DARK_GREY_COLOR =          {107, 107, 107, 255},
                 K_ACTIVE_SELECT_COLOR =      {215, 230, 255, 255},
                 K_ACTIVE_FORE_COLOR =        {  0,   0,   0, 255},
                 K_INACTIVE_SELECT_COLOR =    {215, 230, 255, 255},
                 K_INACTIVE_FORE_COLOR =      {  0,   0,   0, 255},
                 K_STARTUP_MAIN_HEADING =     { 60,  80, 180, 255},
                 K_ABOUT_MAIN_HEADING =       {  0,   0, 192, 255},
                 K_ABOUT_SUB_HEADING =        {152,   0,   0, 255},
                 K_PROGRESS_COLOR =           { 95, 140, 206, 255},
                 K_DEEP_BLUE_COLOR =          {  0,   0, 180, 255},
                 K_ORANGE_COLOR =             {255, 190,   0, 255},
                 K_DEEP_RED_COLOR =           {180,   0,   0, 255};

const float K_MARGIN =                   5,
            K_TOOLBAR_WIDTH =            44,
            K_INFOBAR_HEIGHT =           25,
            K_BUTTON_HEIGHT =            25,
            K_BUTTON_WIDTH =             76,
            K_PROGRESSBAR_HEIGHT =       15;

#endif /* _UI_CONSTANTS_H */
