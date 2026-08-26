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

#include "file.h"

struct moonlit_art_pfraw_header {
    int32_t size;
    int32_t radius;
    int32_t layout;
    int32_t extra;
};

static bool pfraw_header_valid(int fd, int size, int radius, int32_t theme)
{
    struct moonlit_art_pfraw_header hdr;
    int n = read(fd, &hdr, sizeof(hdr));

    return n == (int)sizeof(hdr) && hdr.size == size && hdr.radius == radius
           && hdr.layout == MOONLIT_ART_LAYOUT_ROW_MAJOR && hdr.extra == theme;
}

bool moonlit_art_read_pfraw(const char *path, int size, int radius,
                             int32_t theme, fb_data *out)
{
    size_t px_bytes = (size_t)size * size * sizeof(fb_data);
    int fd, n;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return false;

    if (!pfraw_header_valid(fd, size, radius, theme))
    {
        close(fd);
        return false;
    }

    n = read(fd, out, px_bytes);
    close(fd);
    return n == (int)px_bytes;
}

bool moonlit_art_pfraw_is_cached(const char *path, int size, int radius,
                                  int32_t theme)
{
    int fd;
    bool ok;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return false;

    ok = pfraw_header_valid(fd, size, radius, theme);
    close(fd);
    return ok;
}

int moonlit_art_count_uncached(int count, moonlit_art_path_fn path_fn, void *ctx,
                               int size, int radius, int32_t theme)
{
    char path[MOONLIT_ART_PATH_MAX];
    int i, pending = 0;

    for (i = 0; i < count; i++)
    {
        path_fn(i, path, sizeof(path), ctx);
        if (!moonlit_art_pfraw_is_cached(path, size, radius, theme))
            pending++;
    }
    return pending;
}

void moonlit_art_write_pfraw(const char *path, int size, int radius,
                              int32_t theme, const fb_data *data)
{
    struct moonlit_art_pfraw_header hdr;
    int fd;

    hdr.size = size;
    hdr.radius = radius;
    hdr.layout = MOONLIT_ART_LAYOUT_ROW_MAJOR;
    hdr.extra = theme;

    fd = creat(path, 0666);
    if (fd < 0)
        return;

    write(fd, &hdr, sizeof(hdr));
    write(fd, data, (size_t)size * size * sizeof(fb_data));
    close(fd);
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
