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
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "dir.h"
#include "jpeg_load.h"
#include "bmp.h"
#include "string-extra.h"

#include "metro_photo_thumbs.h"
#include "metro_settings.h"
#include "metro_draw.h"
#include "metro_fsutil.h"

#define PHOTOS_DIR "/Photos"
#define THUMB_PX (METRO_TILE_SIZE * METRO_TILE_SIZE)

/* R2-F2/DD-9: 16 thumbnails (~2 grid screens) resident in RAM at once
 * -- 16 * 80*80*sizeof(fb_data) = 16 * 12800 = 204800 bytes, matching
 * the plan's own estimate. Plain ring eviction (not true LRU): simple,
 * and with a window this much bigger than one screen (8 tiles) it
 * takes real back-and-forth scrolling to evict something still on
 * screen. */
#define WINDOW_N 16
#define PENDING_MAX 16

struct thumb_slot {
    char filename[METRO_FSUTIL_NAME_LEN];
    bool valid;
    fb_data pixels[THUMB_PX];
};

static struct thumb_slot s_window[WINDOW_N];
static int s_window_ring = 0;

static char s_pending[PENDING_MAX][METRO_FSUTIL_NAME_LEN];
static long s_pending_mtime[PENDING_MAX];
static int s_pending_n = 0;

/* Oversized scratch for read_jpeg_file()'s own decode overhead + the
 * pre-crop KEEP_ASPECT decode -- same x2 margin rule as metro_albumart.c
 * (DECISIONS.md M-033): undersizing this doesn't fail loudly, it
 * writes past the buffer before the decoder's own bounds check
 * catches it. */
#define SCRATCH_SIZE (METRO_TILE_SIZE * METRO_TILE_SIZE * 2 * 2)
static unsigned char s_scratch[SCRATCH_SIZE];

static struct thumb_slot *find_slot(const char *filename)
{
    int i;
    for (i = 0; i < WINDOW_N; i++)
        if (s_window[i].valid && !strcmp(s_window[i].filename, filename))
            return &s_window[i];
    return NULL;
}

static void cache_dir(char *out, size_t outsz)
{
    metro_settings_metro_cache_dir(out, outsz);
}

static void cache_path(const char *filename, long mtime, char *out, size_t outsz)
{
    char dir[MAX_PATH];
    cache_dir(dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s.%ld.mth", dir, filename, mtime);
}

static void ensure_cache_dir(void)
{
    char dir[MAX_PATH], parent[MAX_PATH];
    char *slash;

    cache_dir(dir, sizeof(dir));
    strlcpy(parent, dir, sizeof(parent));
    slash = strrchr(parent, '/');
    if (slash)
        *slash = '\0';

    /* .../aura already exists (metro_settings_save() creates it on
     * first boot, C1) -- only "metrocache" and "metrocache/photos"
     * are ever missing here. */
    if (!dir_exists(parent))
        mkdir(parent);
    if (!dir_exists(dir))
        mkdir(dir);
}

/* R2-F2/DD-9: drops any other cached .mth for this same source
 * filename -- the mtime-in-the-name scheme means a re-synced photo
 * with new content picks a new cache filename, leaving the old one an
 * orphan pointing at content nobody will ever ask for by that exact
 * (filename, mtime) pair again. Cheap: one directory scan, only run on
 * an actual decode (already the slow path). */
static void remove_stale(const char *filename, const char *keep_path)
{
    char dir[MAX_PATH];
    DIR *d;
    struct DIRENT *entry;
    size_t name_len = strlen(filename);

    cache_dir(dir, sizeof(dir));
    d = opendir(dir);
    if (!d)
        return;

    while ((entry = readdir(d)) != NULL)
    {
        char full[MAX_PATH];

        if (strncmp(entry->d_name, filename, name_len) != 0 ||
            entry->d_name[name_len] != '.')
            continue; /* not "<filename>.<mtime>.mth" for THIS filename */

        snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
        if (strcmp(full, keep_path) != 0)
            remove(full);
    }
    closedir(d);
}

/* Nearest-neighbour "cover" crop from a single FORMAT_KEEP_ASPECT
 * decode -- no second (unscaled) decode and no JPEG-dimension probe
 * needed. `src`/`sw`/`sh` is the KEEP_ASPECT result (one dimension
 * already == METRO_TILE_SIZE, the other <= it); this conceptually
 * upscales that result until BOTH dimensions reach METRO_TILE_SIZE,
 * then samples the centered METRO_TILE_SIZE x METRO_TILE_SIZE crop
 * straight out of `src` (never materializes the upscaled bitmap).
 * Trades a little sharpness on the cropped axis for staying a single
 * cheap decode -- acceptable at 80x80. DD-10's own "cubrir" (the
 * photo viewer, full 320x240) needs real precision instead, hence
 * that one *does* read Aura's Q16.16 algorithm as reference; a
 * thumbnail this small doesn't call for the same machinery. */
static void cover_crop(const fb_data *src, int sw, int sh, fb_data *out)
{
    int scale_den = (sw < sh) ? sw : sh; /* the dimension short of METRO_TILE_SIZE */
    int upscaled_w = sw * METRO_TILE_SIZE / scale_den;
    int upscaled_h = sh * METRO_TILE_SIZE / scale_den;
    int crop_x = (upscaled_w - METRO_TILE_SIZE) / 2;
    int crop_y = (upscaled_h - METRO_TILE_SIZE) / 2;
    int ox, oy;

    for (oy = 0; oy < METRO_TILE_SIZE; oy++)
    {
        int up_y = oy + crop_y;
        int sy = up_y * scale_den / METRO_TILE_SIZE;

        if (sy < 0) sy = 0;
        if (sy >= sh) sy = sh - 1;

        for (ox = 0; ox < METRO_TILE_SIZE; ox++)
        {
            int up_x = ox + crop_x;
            int sx = up_x * scale_den / METRO_TILE_SIZE;

            if (sx < 0) sx = 0;
            if (sx >= sw) sx = sw - 1;

            out[oy * METRO_TILE_SIZE + ox] = src[sy * sw + sx];
        }
    }
}

static bool decode_thumb(const char *filename, fb_data *out)
{
    char path[MAX_PATH];
    struct bitmap bm;
    int ret;

    snprintf(path, sizeof(path), "%s/%s", PHOTOS_DIR, filename);

    bm.width = METRO_TILE_SIZE;
    bm.height = METRO_TILE_SIZE;
    bm.data = (char *)s_scratch;
#if (LCD_DEPTH > 1)
    bm.maskdata = NULL;
#endif
    ret = read_jpeg_file(path, &bm, sizeof(s_scratch),
                          FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT, NULL);
    if (ret <= 0)
        return false;

    cover_crop((const fb_data *)s_scratch, bm.width, bm.height, out);
    return true;
}

const fb_data *metro_photo_thumbs_get(const char *filename, long mtime)
{
    struct thumb_slot *s;
    char path[MAX_PATH];
    int fd;
    ssize_t got;
    int i;

    s = find_slot(filename);
    if (s)
        return s->pixels;

    /* Disk cache is a raw read (no decode) -- cheap enough to try
     * synchronously, unlike an actual JPEG decode. */
    cache_path(filename, mtime, path, sizeof(path));
    fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        got = read(fd, s_window[s_window_ring].pixels, THUMB_PX * sizeof(fb_data));
        close(fd);
        if (got == (ssize_t)(THUMB_PX * sizeof(fb_data)))
        {
            s = &s_window[s_window_ring];
            strlcpy(s->filename, filename, sizeof(s->filename));
            s->valid = true;
            s_window_ring = (s_window_ring + 1) % WINDOW_N;
            return s->pixels;
        }
    }

    for (i = 0; i < s_pending_n; i++)
        if (!strcmp(s_pending[i], filename))
            return NULL; /* already queued */

    if (s_pending_n < PENDING_MAX)
    {
        strlcpy(s_pending[s_pending_n], filename, sizeof(s_pending[0]));
        s_pending_mtime[s_pending_n] = mtime;
        s_pending_n++;
    }
    return NULL;
}

bool metro_photo_thumbs_tick(void)
{
    char filename[METRO_FSUTIL_NAME_LEN];
    long mtime;
    char path[MAX_PATH];
    struct thumb_slot *s;
    int fd;
    int i;

    if (s_pending_n == 0)
        return false;

    strlcpy(filename, s_pending[0], sizeof(filename));
    mtime = s_pending_mtime[0];
    for (i = 1; i < s_pending_n; i++)
    {
        strlcpy(s_pending[i - 1], s_pending[i], sizeof(s_pending[0]));
        s_pending_mtime[i - 1] = s_pending_mtime[i];
    }
    s_pending_n--;

    s = &s_window[s_window_ring];
    if (!decode_thumb(filename, s->pixels))
        return true; /* budget spent either way -- don't retry this tick */

    strlcpy(s->filename, filename, sizeof(s->filename));
    s->valid = true;
    s_window_ring = (s_window_ring + 1) % WINDOW_N;

    ensure_cache_dir();
    cache_path(filename, mtime, path, sizeof(path));
    remove_stale(filename, path);
    fd = creat(path, 0666);
    if (fd >= 0)
    {
        write(fd, s->pixels, THUMB_PX * sizeof(fb_data));
        close(fd);
    }

    return true;
}

void metro_photo_thumbs_reset(void)
{
    int i;
    for (i = 0; i < WINDOW_N; i++)
        s_window[i].valid = false;
    s_pending_n = 0;
}
