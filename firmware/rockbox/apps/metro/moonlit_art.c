/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* moonlit: derived from aura_art.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-020, D-042). */
#include "moonlit_art.h"

#include <string.h>
#include <stdio.h>  /* snprintf(), remove() */
#include "file.h"
#include "dir.h"    /* D-056: moonlit_art_sweep() */

int moonlit_art_sweep(const char *dir, const char *suffix,
                      moonlit_art_keep_fn keep, void *ctx)
{
    DIR *d = opendir(dir);
    struct DIRENT *entry;
    size_t suffix_len = strlen(suffix);
    char stem[MOONLIT_ART_PATH_MAX];
    char full[MOONLIT_ART_PATH_MAX];
    int removed = 0;

    if (!d)
        return 0;
    while ((entry = readdir(d)) != NULL)
    {
        const char *name = entry->d_name;
        size_t len = strlen(name);

        if (name[0] == '.' || len <= suffix_len ||
            strcmp(name + len - suffix_len, suffix) != 0)
            continue;
        if (len - suffix_len >= sizeof(stem))
            continue;
        memcpy(stem, name, len - suffix_len);
        stem[len - suffix_len] = '\0';
        if (keep(stem, ctx))
            continue;
        snprintf(full, sizeof(full), "%s/%s", dir, name);
        if (remove(full) == 0)
            removed++;
    }
    closedir(d);
    return removed;
}

/* D-042: raiz entera copiada localmente (mismo algoritmo que
 * a26_shell_isqrt256/apple2026_shell.c y que isqrt32() en
 * metro_widgets.c/moonlit_elevation.c) en vez de compartirla con esos
 * modulos -- moonlit_art.c no puede incluir metro_widgets.h/metro_fb.h
 * (arrastran lcd.h real, no compilable con `cc` de host) y seguir
 * enlazando standalone en apps/metro/test/test_art.c. Entradas aqui
 * son (radius-1-fila)^2 + (radius-1-columna)^2 con radius <= 12
 * (design-system/tokens.json shape.corner_m), asi que v < 288: v<<16
 * nunca desborda 32 bits, no hace falta el ajuste de escala variable
 * de a26_shell_isqrt256. */
static unsigned isqrt256(unsigned v)
{
    unsigned res = 0, bit = 1u << 30;

    v <<= 16;
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

/* D-042: mismo formato RGB565 plano de ipod6g (LCD_PIXELFORMAT RGB565,
 * config/ipod6g.h:85 -- no RGB565SWAPPED) que RGB_UNPACK_xxx y
 * LCD_RGBPACK de lcd.h, copiado localmente por el mismo motivo que
 * isqrt256() de arriba: metro_fb_blend_color() (metro_fb.c) haria
 * falta incluir metro_fb.h, que tira de lcd.h real. */
static unsigned unpack_r(unsigned x) { return ((x >> 8) & 0xf8) | ((x >> 13) & 0x07); }
static unsigned unpack_g(unsigned x) { return ((x >> 3) & 0xfc) | ((x >>  9) & 0x03); }
static unsigned unpack_b(unsigned x) { return ((x << 3) & 0xf8) | ((x >>  2) & 0x07); }
static unsigned pack_rgb(unsigned r, unsigned g, unsigned b)
{
    return (((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3);
}

static unsigned blend(unsigned from, unsigned to, int alpha256)
{
    unsigned r = unpack_r(from) + (((int)unpack_r(to) - (int)unpack_r(from)) * alpha256) / 256;
    unsigned g = unpack_g(from) + (((int)unpack_g(to) - (int)unpack_g(from)) * alpha256) / 256;
    unsigned b = unpack_b(from) + (((int)unpack_b(to) - (int)unpack_b(from)) * alpha256) / 256;

    return pack_rgb(r, g, b);
}

void moonlit_art_mask_corners(fb_data *buf, int size, int radius, unsigned bg)
{
    int r256 = radius * 256;
    int row, col;

    for (row = 0; row < radius; row++)
    {
        for (col = 0; col < radius; col++)
        {
            int rr = radius - 1 - row;
            int rc = radius - 1 - col;
            int dist256 = (int)isqrt256((unsigned)(rr * rr + rc * rc));
            size_t idx[4];
            int k, t;

            if (dist256 <= r256 - 128)
                continue;

            idx[0] = (size_t)row * size + col;
            idx[1] = (size_t)row * size + (size - 1 - col);
            idx[2] = (size_t)(size - 1 - row) * size + col;
            idx[3] = (size_t)(size - 1 - row) * size + (size - 1 - col);

            if (dist256 >= r256 + 128)
            {
                for (k = 0; k < 4; k++)
                    buf[idx[k]] = (fb_data)bg;
                continue;
            }

            t = dist256 - (r256 - 128);
            for (k = 0; k < 4; k++)
                buf[idx[k]] = (fb_data)blend(buf[idx[k]], bg, t);
        }
    }
}
