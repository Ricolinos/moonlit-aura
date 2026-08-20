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
#include "metadata.h" /* get_metadata() -- R3-F4/DD-5 */

#include "metro_albumart.h"
#include "metro_draw.h" /* METRO_TILE_SIZE */

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

bool metro_albumart_load_background_file(const char *path)
{
    if (!path || !path[0])
        return false;

    /* Misma caché-de-1 que load_background(), clavada a la RUTA REAL
     * de origen y no al track: así una foto de artista y una carátula
     * nunca se confunden entre sí, y volver a la misma pista con la
     * misma fuente no vuelve a decodificar. */
    if (s_bg_loaded && !strcmp(s_bg_loaded_path, path))
        return true;

    /* Sin FORMAT_KEEP_ASPECT, igual que load_background(): llena los
     * 320x240 y recorta, que es el punto de un fondo. Una foto de
     * artista viene a lo mucho de 128px (tope del contrato), así que
     * aquí se AGRANDA -- se ve suave, y a 30% de opacidad detrás del
     * texto eso no es un defecto sino lo deseable. */
    if (!decode_file_into(path, s_bg_scratch, sizeof(s_bg_scratch),
                           LCD_WIDTH, LCD_HEIGHT, FORMAT_NATIVE | FORMAT_RESIZE))
    {
        s_bg_loaded = false;
        s_bg_loaded_path[0] = '\0';
        return false;
    }

    strlcpy(s_bg_loaded_path, path, sizeof(s_bg_loaded_path));
    s_bg_loaded = true;
    return true;
}

const fb_data *metro_albumart_background_bitmap(void)
{
    return (const fb_data *)s_bg_scratch;
}

/* R3-F4/DD-5 (M-065): nearest-neighbour downscale of an ALREADY-DECODED
 * METRO_ALBUMART_SIZE x METRO_ALBUMART_SIZE bitmap to a
 * METRO_TILE_SIZE x METRO_TILE_SIZE one -- pure pixel resampling, no
 * JPEG involved, so it can't hit the JPEG_DECODE_OVERHEAD gap a second
 * from-JPEG decode at 80px would risk for a cover near that size
 * (docs/DESVIACIONES.md R3-3). Both sizes are square, so this is a
 * plain scale, no cover-crop math needed. */
static void downscale_to_tile(const fb_data *src, fb_data *out)
{
    int oy, ox;

    for (oy = 0; oy < METRO_TILE_SIZE; oy++)
    {
        int sy = oy * METRO_ALBUMART_SIZE / METRO_TILE_SIZE;
        for (ox = 0; ox < METRO_TILE_SIZE; ox++)
        {
            int sx = ox * METRO_ALBUMART_SIZE / METRO_TILE_SIZE;
            out[oy * METRO_TILE_SIZE + ox] = src[sy * METRO_ALBUMART_SIZE + sx];
        }
    }
}

/* Own static mp3entry -- struct mp3entry is ~1.5-2KB (ID3V2_BUF_SIZE
 * alone is up to 1800B, lib/rbcodec/metadata/metadata.h), too big for
 * a comfortable stack frame on Rockbox's small per-thread stacks (same
 * "D-226 stack concern" class of buffer metro_music.c already avoids
 * putting on the stack). Deliberately NOT get_temp_mp3entry()
 * (playback.h): that one is playback-engine scratch memory with its
 * own locking, for a different purpose (peeking at the next track) --
 * using it here would mean contending with the audio thread for
 * something that has nothing to do with it. get_metadata() itself is
 * a standalone utility, not tied to that machinery -- tagcache.c calls
 * it the same way, for the same reason (reading tags from an arbitrary
 * file, independent of what's playing). */
static struct mp3entry s_track_id3;

bool metro_albumart_decode_track_cover(const char *track_path, fb_data *out)
{
    char art_path[MAX_PATH];
    struct dim dim = { METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE };
    bool ok;

    /* Shares s_scratch with metro_albumart_load_current()'s own "cache
     * of 1" (Now Playing's screens and Quickplay's grid are never on
     * screen at once, so there's no *concurrent* use) -- but its
     * bookkeeping (s_loaded/s_loaded_path) has no way to know this
     * function just overwrote the buffer's CONTENTS for an unrelated
     * track. Without this, returning to Now Playing on the same track
     * that was already cached before a Quickplay visit would trust the
     * stale flag and serve whatever album cover Quickplay decoded
     * last, not the real one -- force a real redecode next time
     * instead. */
    s_loaded = false;

    if (!get_metadata(&s_track_id3, -1, track_path))
        return false;

    if (find_albumart(&s_track_id3, art_path, sizeof(art_path), &dim))
        ok = decode_file_into(art_path, s_scratch, sizeof(s_scratch),
                              METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE,
                              FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT);
    else
        ok = decode_embedded_into(&s_track_id3, s_scratch, sizeof(s_scratch),
                                  METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE,
                                  FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT);

    if (!ok)
        return false;

    downscale_to_tile((const fb_data *)s_scratch, out);
    return true;
}
