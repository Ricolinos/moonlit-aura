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
#include "lcd.h"

#include "moonlit_elevation.h"
#include "metro_fb.h"

/* Mismo entero-sqrt de metro_widgets.c (distancias en 8.8 al cuadrado,
 * < 2^24 para un radio de esquina de a lo sumo unas pocas decenas de
 * px) -- privado a proposito, no vale la pena exportarlo por una
 * funcion de tres lineas. */
static unsigned isqrt32(unsigned v)
{
    unsigned res = 0, bit = 1u << 30;

    while (bit > v)
        bit >>= 2;
    while (bit)
    {
        if (v >= res + bit)
        {
            v -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

/* Repinta hacia `outside` el cuadrante de una esquina que cae fuera del
 * radio, por cobertura (0 = adentro, sigue siendo `base`; 256 = del
 * todo afuera). `cx`,`cy` ya son la esquina superior-izquierda del
 * cuadrante -- cada llamada cubre las 4 esquinas de la tarjeta a la vez. */
static void round_corners(int x, int y, int w, int h, int r, unsigned outside)
{
    int dx, dy;
    int r256 = r << 8;

    for (dy = 0; dy < r; dy++)
    {
        for (dx = 0; dx < r; dx++)
        {
            int cx = r - 1 - dx, cy = r - 1 - dy;
            unsigned d256 = isqrt32((unsigned)(cx * cx + cy * cy) << 16);
            int alpha = (int)d256 - r256 + 256;

            if (alpha <= 0)
                continue;
            if (alpha > 256)
                alpha = 256;

            metro_fb_plot_alpha(x + dx,         y + dy,         outside, alpha);
            metro_fb_plot_alpha(x + w - 1 - dx, y + dy,         outside, alpha);
            metro_fb_plot_alpha(x + dx,         y + h - 1 - dy, outside, alpha);
            metro_fb_plot_alpha(x + w - 1 - dx, y + h - 1 - dy, outside, alpha);
        }
    }
}

void moonlit_draw_surface(int x, int y, int w, int h,
                           enum moonlit_surface_level level, int radius)
{
    unsigned base, light, shadow, outside;
    int r = radius;

    if (w <= 0 || h <= 0)
        return;
    if (r > w / 2)
        r = w / 2;
    if (r > h / 2)
        r = h / 2;
    if (r < 0)
        r = 0;

    base = moonlit_surface(level, MEDGE_NONE);
    light = moonlit_surface(level, MEDGE_LIGHT);
    shadow = moonlit_surface(level, MEDGE_SHADOW);

    lcd_set_foreground(base);
    lcd_fillrect(x, y, w, h);

    if (r > 0)
    {
        outside = moonlit_color(MROLE_SURFACE);
        round_corners(x, y, w, h, r, outside);
    }

    /* D-012: 1px, luz arriba-izquierda / sombra abajo-derecha, cortado
     * antes de las esquinas redondeadas para no pelearse con el
     * antialias de round_corners(). */
    lcd_set_foreground(light);
    lcd_hline(x + r, x + w - 1 - r, y);
    lcd_vline(x, y + r, y + h - 1 - r);

    lcd_set_foreground(shadow);
    lcd_hline(x + r, x + w - 1 - r, y + h - 1);
    lcd_vline(x + w - 1, y + r, y + h - 1 - r);
}
