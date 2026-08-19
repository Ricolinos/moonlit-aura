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
#include <string.h>

#include "config.h" /* HAVE_ALBUMART/HAVE_JPEG, same ordering gotcha as
                        metro_music.c -- see DECISIONS.md M-030. */
#include "audio.h"
#include "albumart.h"
#include "jpeg_load.h"
#include "bmp.h"
#include "string-extra.h"

#include "metro_albumart.h"

/* NOT just width*height*sizeof(fb_data) -- FORMAT_RESIZE needs real
 * working room beyond the final bitmap (JPEG_DECODE_OVERHEAD,
 * recorder/jpeg_load.h, is ~39KB on its own) for the intermediate
 * decode before it downscales to METRO_ALBUMART_SIZE. Same formula and
 * same margin Aura-Firmware's aura_albumart.c settled on after finding
 * this the hard way: undersizing this doesn't fail loudly, it either
 * degrades silently to the no-art tile, or -- verified here, the
 * actual failure hit while building this file with a 1x buffer --
 * read_jpeg_file()/clip_jpeg_file() write past the end of an
 * undersized buffer before their own bounds check gives up, corrupting
 * whatever static data the linker placed next. */
#define METRO_ALBUMART_SCRATCH_SIZE \
    (METRO_ALBUMART_SIZE * METRO_ALBUMART_SIZE * 2 * 2)

static unsigned char s_scratch[METRO_ALBUMART_SCRATCH_SIZE];
static char s_loaded_path[MAX_PATH];
static bool s_loaded = false;

/* F12: same shape as s_scratch above, just full-screen instead of the
 * small NP tile -- the dimmed background (metro_screen_nowplaying.c,
 * graphics=full only). Same oversizing rule as METRO_ALBUMART_SCRATCH_SIZE
 * (M-033): FORMAT_RESIZE needs room for the full native decode before
 * it downscales, not just the final LCD_WIDTH*LCD_HEIGHT bitmap. */
#define METRO_ALBUMART_BG_SCRATCH_SIZE \
    (LCD_WIDTH * LCD_HEIGHT * 2 * 2)

static unsigned char s_bg_scratch[METRO_ALBUMART_BG_SCRATCH_SIZE];
static char s_bg_loaded_path[MAX_PATH];
static bool s_bg_loaded = false;

/* Shared by both sizes (the small NP tile and F12's full-screen
 * background) -- only the destination width/height/buffer and the
 * FORMAT_* flags differ between them. */
static bool decode_file_into(const char *art_path, unsigned char *scratch,
                              size_t scratch_size, int width, int height, int format)
{
    struct bitmap bm;
    size_t len = strlen(art_path);

    bm.width = width;
    bm.height = height;
    bm.data = scratch;
#if (LCD_DEPTH > 1)
    bm.maskdata = NULL;
#endif

    if (len > 4 && !strcasecmp(art_path + len - 4, ".bmp"))
        return read_bmp_file(art_path, &bm, scratch_size, format, NULL) > 0;

    return read_jpeg_file(art_path, &bm, scratch_size, format, NULL) > 0;
}

static bool decode_embedded_into(struct mp3entry *id3, unsigned char *scratch,
                                  size_t scratch_size, int width, int height, int format)
{
    struct bitmap bm;

    if (!id3->has_embedded_albumart ||
        (id3->albumart.type & AA_CLEAR_FLAGS_MASK) != AA_TYPE_JPG)
        return false;

    bm.width = width;
    bm.height = height;
    bm.data = scratch;
#if (LCD_DEPTH > 1)
    bm.maskdata = NULL;
#endif

    return clip_jpeg_file(id3->path, id3->albumart.pos, id3->albumart.size,
                           &bm, scratch_size, format, NULL) > 0;
}

/* "cache of 1" (module doc comment): reloads only when the track's
 * path changed since the last call for THIS particular loaded_path/
 * loaded_flag pair -- the tile and the background cache independently
 * of each other, since a caller might only ever need one of the two. */
static bool load_art(char *loaded_path, size_t loaded_path_sz, bool *loaded_flag,
                      unsigned char *scratch, size_t scratch_size,
                      int width, int height, int format)
{
    struct mp3entry *id3;
    char art_path[MAX_PATH];
    struct dim dim = { width, height };
    bool ok;

    if (!(audio_status() & AUDIO_STATUS_PLAY))
        return false;

    id3 = audio_current_track();
    if (!id3)
        return false;

    if (*loaded_flag && !strcmp(loaded_path, id3->path))
        return true;

    if (find_albumart(id3, art_path, sizeof(art_path), &dim))
        ok = decode_file_into(art_path, scratch, scratch_size, width, height, format);
    else
        ok = decode_embedded_into(id3, scratch, scratch_size, width, height, format);

    if (!ok)
    {
        *loaded_flag = false;
        loaded_path[0] = '\0';
        return false;
    }

    strlcpy(loaded_path, id3->path, loaded_path_sz);
    *loaded_flag = true;
    return true;
}

bool metro_albumart_load_current(void)
{
    return load_art(s_loaded_path, sizeof(s_loaded_path), &s_loaded,
                     s_scratch, sizeof(s_scratch),
                     METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE,
                     FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT);
}

const fb_data *metro_albumart_bitmap(void)
{
    return (const fb_data *)s_scratch;
}

bool metro_albumart_load_background(void)
{
    /* No FORMAT_KEEP_ASPECT -- fills the whole 320x240 screen, cropped
     * corners are the point (PLAN_MAESTRO.md S3.3's "carátula escalada
     * a 320x240"), not letterboxed. */
    return load_art(s_bg_loaded_path, sizeof(s_bg_loaded_path), &s_bg_loaded,
                     s_bg_scratch, sizeof(s_bg_scratch),
                     LCD_WIDTH, LCD_HEIGHT, FORMAT_NATIVE | FORMAT_RESIZE);
}

const fb_data *metro_albumart_background_bitmap(void)
{
    return (const fb_data *)s_bg_scratch;
}
