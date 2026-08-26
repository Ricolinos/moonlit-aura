/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gomez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
/* moonlit (D-016, D-044): mismo cuerpo que moonlit_icon_draw()
 * (moonlit_icons.c, D-033) -- blend pixel a pixel de la mascara de
 * cobertura contra el framebuffer via metro_fb_plot_alpha(). */
#include "moonlit_logo.h"
#include "metro_fb.h"

static void blend_mask(const struct moonlit_logo_mask *mask, int x, int y, unsigned color)
{
    int row, col;

    for (row = 0; row < mask->height; row++)
        for (col = 0; col < mask->width; col++)
        {
            int a = mask->cov[row * mask->width + col];
            if (a)
                metro_fb_plot_alpha(x + col, y + row, color, a * 256 / 255);
        }
}

void moonlit_logo_draw_crescent(int size_px, int x, int y, unsigned color)
{
    int index;

    switch (size_px)
    {
        case MOONLIT_LOGO_CRESCENT_SIZE_16: index = 0; break;
        case MOONLIT_LOGO_CRESCENT_SIZE_24: index = 1; break;
        case MOONLIT_LOGO_CRESCENT_SIZE_40: index = 2; break;
        case MOONLIT_LOGO_CRESCENT_SIZE_64: index = 3; break;
        default: return;
    }

    blend_mask(&moonlit_logo_crescent[index], x, y, color);
}

void moonlit_logo_draw_wordmark(int x, int y, unsigned color)
{
    blend_mask(&moonlit_logo_wordmark, x, y, color);
}
