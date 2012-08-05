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

#ifndef _UI_CONSTANTS_H
#define _UI_CONSTANTS_H

const rgb_color
K_BACKGROUND_COLOR =         {216, 216, 216, 255},
                   K_TOOLBAR_BACK_COLOR =        {222, 222, 222, 255},
                   K_HIGHLIGHT_GREY =           {225, 225, 225, 255},
                   K_DARK_GREY_COLOR =          {107, 107, 107, 255},
                   K_ACTIVE_SELECT_COLOR =       {215, 230, 255, 255},
                   K_ACTIVE_FORE_COLOR =        {  0,   0,   0, 255},
                   K_INACTIVE_SELECT_COLOR =     {215, 230, 255, 255},
                   K_INACTIVE_FORE_COLOR =       {  0,   0,   0, 255},
                   K_WHITE_COLOR =              {255, 255, 255, 255},
                   K_BLACK_COLOR =              {  0,   0,   0, 255},
                   K_STARTUP_MAIN_HEADING =      { 60,  80, 180, 255},
                   K_ABOUT_MAIN_HEADING =        {  0,   0, 192, 255},
                   K_ABOUT_SUB_HEADING =        {152,   0,   0, 255},
                   K_PROGRESS_COLOR =           { 95, 140, 206, 255},
                   K_DEEP_BLUE_COLOR =          {  0,   0, 180, 255},
                   K_ORANGE_COLOR =             {255, 190,   0, 255},
                   K_DEEP_RED_COLOR =           {180,   0,   0, 255};

const float
K_MARGIN =                  5,
         K_TOOLBAR_WIDTH =            44,
         K_INFOBAR_HEIGHT =           25,
         K_BUTTON_HEIGHT =            25,
         K_BUTTON_WIDTH =             76,
         K_PROGRESSBAR_HEIGHT =        15;

#endif /* _UI_CONSTANTS_H */
