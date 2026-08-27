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
/* D-059: see moonlit_master_art.h. Pure -- host-tested in
 * apps/metro/test/test_master_art.c. */
#include <stdio.h>
#include <string.h>

#include "moonlit_master_art.h"
#include "file.h"
#include "dir.h"

/* --- names -------------------------------------------------------------- */

void moonlit_master_art_file_key(char prefix, uint32_t crc, long mtime,
                                 char *out, size_t outsz)
{
    snprintf(out, outsz, "%c-%08lx.%ld", prefix, (unsigned long)crc, mtime);
}

bool moonlit_master_art_none_path(const char *art_path, char *out, size_t outsz)
{
    static const char ext[] = ".art";
    size_t len = strlen(art_path);
    size_t stem_len;

    out[0] = '\0';
    if (len <= sizeof(ext) - 1 || strcmp(art_path + len - (sizeof(ext) - 1), ext) != 0)
        return false;
    stem_len = len - (sizeof(ext) - 1);
    if (stem_len + sizeof(".none") > outsz)
        return false;
    memcpy(out, art_path, stem_len);
    memcpy(out + stem_len, ".none", sizeof(".none"));
    return true;
}

/* --- header ------------------------------------------------------------- */

/* Explicit little-endian (de)serialisation: the file is shared with
 * firmwares that may be built with other struct packing rules, and the
 * S5L8702 is LE anyway -- this is documentation as much as safety. */
static void header_pack(unsigned char *h, int size)
{
    uint32_t magic = MOONLIT_MASTER_ART_MAGIC;
    int i;

    for (i = 0; i < 4; i++)
        h[i] = (unsigned char)(magic >> (8 * i));
    h[4] = (unsigned char)(size & 0xff);
    h[5] = (unsigned char)((size >> 8) & 0xff);
    h[6] = h[4];
    h[7] = h[5];
    memset(h + 8, 0, 8); /* flags, reserved */
}

static bool header_valid(const unsigned char *h, int size)
{
    uint32_t magic = (uint32_t)h[0] | ((uint32_t)h[1] << 8)
                   | ((uint32_t)h[2] << 16) | ((uint32_t)h[3] << 24);
    int w = h[4] | (h[5] << 8);
    int ht = h[6] | (h[7] << 8);

    /* flags/reserved deliberately not checked: a sibling family may
     * set a flag this build does not know yet, and the pixels below
     * are still the plain square it promises. */
    return magic == MOONLIT_MASTER_ART_MAGIC && w == size && ht == size;
}

static bool read_header_fd(int fd, int size)
{
    unsigned char h[MOONLIT_MASTER_ART_HEADER_SIZE];
    int n = read(fd, h, sizeof(h));

    return n == (int)sizeof(h) && header_valid(h, size);
}

bool moonlit_master_art_exists(const char *path, int size)
{
    int fd = open(path, O_RDONLY);
    bool ok;

    if (fd < 0)
        return false;
    ok = read_header_fd(fd, size);
    close(fd);
    return ok;
}

bool moonlit_master_art_read(const char *path, int size, fb_data *out)
{
    size_t px_bytes = (size_t)size * size * sizeof(fb_data);
    int fd, n;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return false;
    if (!read_header_fd(fd, size))
    {
        close(fd);
        return false;
    }
    n = read(fd, out, px_bytes);
    close(fd);
    return n == (int)px_bytes;
}

void moonlit_master_art_write(const char *path, int size, const fb_data *data)
{
    unsigned char h[MOONLIT_MASTER_ART_HEADER_SIZE];
    char tmp[MOONLIT_MASTER_ART_PATH_MAX];
    size_t px_bytes = (size_t)size * size * sizeof(fb_data);
    int fd;

    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp))
        return;

    fd = creat(tmp, 0666);
    if (fd < 0)
        return;

    header_pack(h, size);
    if (write(fd, h, sizeof(h)) != (int)sizeof(h)
        || write(fd, data, px_bytes) != (int)px_bytes)
    {
        close(fd);
        remove(tmp);
        return;
    }
    close(fd);
    remove(path); /* rename() over an existing target is not portable */
    rename(tmp, path);
}

void moonlit_master_art_write_none(const char *none_path)
{
    int fd = creat(none_path, 0666);

    if (fd >= 0)
        close(fd);
}

bool moonlit_master_art_none_exists(const char *none_path)
{
    int fd = open(none_path, O_RDONLY);

    if (fd < 0)
        return false;
    close(fd);
    return true;
}

bool moonlit_master_art_is_resolved(const char *art_path, int size)
{
    char none[MOONLIT_MASTER_ART_PATH_MAX];

    if (moonlit_master_art_exists(art_path, size))
        return true;
    return moonlit_master_art_none_path(art_path, none, sizeof(none))
           && moonlit_master_art_none_exists(none);
}

void moonlit_master_art_ensure_dir(const char *dir)
{
    char parent[MOONLIT_MASTER_ART_PATH_MAX];
    char *slash;

    if (strlen(dir) >= sizeof(parent))
        return;
    strcpy(parent, dir);
    for (slash = strchr(parent + 1, '/'); slash; slash = strchr(slash + 1, '/'))
    {
        *slash = '\0';
        if (!dir_exists(parent))
            mkdir(parent);
        *slash = '/';
    }
    if (!dir_exists(dir))
        mkdir(dir);
}

/* --- pixels ------------------------------------------------------------- */

/* Same RGB565 accessors as moonlit_art.c's mask_corners (replicated
 * low bits, so a full-white pixel unpacks to 255/255/255). */
static unsigned unpack_r(unsigned x) { return ((x >> 8) & 0xf8) | ((x >> 13) & 0x07); }
static unsigned unpack_g(unsigned x) { return ((x >> 3) & 0xfc) | ((x >>  9) & 0x03); }
static unsigned unpack_b(unsigned x) { return ((x << 3) & 0xf8) | ((x >>  2) & 0x07); }
static fb_data pack_rgb(unsigned r, unsigned g, unsigned b)
{
    return (fb_data)(((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3));
}

/* Average of the box [x0,x1) x [y0,y1) of `src` (row stride `sw`). */
static fb_data box_average(const fb_data *src, int sw, int x0, int x1, int y0, int y1)
{
    unsigned r = 0, g = 0, b = 0, n = 0;
    int x, y;

    for (y = y0; y < y1; y++)
    {
        const fb_data *row = src + y * sw;
        for (x = x0; x < x1; x++)
        {
            unsigned px = row[x];
            r += unpack_r(px);
            g += unpack_g(px);
            b += unpack_b(px);
            n++;
        }
    }
    if (n == 0)
        return 0;
    return pack_rgb((r + n / 2) / n, (g + n / 2) / n, (b + n / 2) / n);
}

void moonlit_master_art_resample_cover(const fb_data *src, int sw, int sh,
                                       fb_data *dst, int size)
{
    /* Virtual "filled" size: scale the short side to `size`, then the
     * long side is >= size and we crop its centre. All box edges are
     * computed in source coordinates with integer math:
     *   src_x = virt_x * short / size. */
    int short_side = (sw < sh) ? sw : sh;
    int virt_w = sw * size / short_side;
    int virt_h = sh * size / short_side;
    int crop_x = (virt_w - size) / 2;
    int crop_y = (virt_h - size) / 2;
    int ox, oy;

    if (sw <= 0 || sh <= 0 || size <= 0)
        return;

    for (oy = 0; oy < size; oy++)
    {
        int vy = oy + crop_y;
        int y0 = vy * short_side / size;
        int y1 = (vy + 1) * short_side / size;

        if (y1 <= y0) y1 = y0 + 1;
        if (y0 >= sh) y0 = sh - 1;
        if (y1 > sh) y1 = sh;

        for (ox = 0; ox < size; ox++)
        {
            int vx = ox + crop_x;
            int x0 = vx * short_side / size;
            int x1 = (vx + 1) * short_side / size;

            if (x1 <= x0) x1 = x0 + 1;
            if (x0 >= sw) x0 = sw - 1;
            if (x1 > sw) x1 = sw;

            dst[oy * size + ox] = box_average(src, sw, x0, x1, y0, y1);
        }
    }
}

void moonlit_master_art_box_downscale(const fb_data *src, int ssize,
                                      fb_data *dst, int dsize)
{
    if (ssize == dsize)
    {
        memcpy(dst, src, (size_t)ssize * ssize * sizeof(fb_data));
        return;
    }
    moonlit_master_art_resample_cover(src, ssize, ssize, dst, dsize);
}
