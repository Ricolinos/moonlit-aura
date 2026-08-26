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
/* moonlit (D-008, D-033): mismo cuerpo que metro_widgets_draw_glyph()
 * (metro_widgets.c, M-089) -- blend pixel a pixel de la mascara de
 * cobertura contra el framebuffer via metro_fb_plot_alpha(). Un solo
 * dibujante para los tres tamanos, en vez de una funcion por talla. */
#include "moonlit_icons.h"
#include "metro_fb.h"

void moonlit_icon_draw(enum moonlit_icon_id id, int size_px, int x, int y, unsigned color)
{
    const struct moonlit_icon_mask *mask;
    int size_index;
    int row, col;

    if ((unsigned)id >= MOONLIT_ICON_COUNT)
        return;

    switch (size_px)
    {
        case MOONLIT_ICON_SIZE_16: size_index = 0; break;
        case MOONLIT_ICON_SIZE_24: size_index = 1; break;
        case MOONLIT_ICON_SIZE_40: size_index = 2; break;
        default: return;
    }

    mask = &moonlit_icons[id][size_index];

    for (row = 0; row < mask->height; row++)
        for (col = 0; col < mask->width; col++)
        {
            int a = mask->cov[row * mask->width + col];
            if (a)
                metro_fb_plot_alpha(x + col, y + row, color, a * 256 / 255);
        }
}
