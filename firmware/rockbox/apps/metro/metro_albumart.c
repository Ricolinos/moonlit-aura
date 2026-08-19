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

static bool decode_into_scratch(const char *art_path, int format)
{
    struct bitmap bm;
    size_t len = strlen(art_path);

    bm.width = METRO_ALBUMART_SIZE;
    bm.height = METRO_ALBUMART_SIZE;
    bm.data = s_scratch;
#if (LCD_DEPTH > 1)
    bm.maskdata = NULL;
#endif

    if (len > 4 && !strcasecmp(art_path + len - 4, ".bmp"))
        return read_bmp_file(art_path, &bm, sizeof(s_scratch), format, NULL) > 0;

    return read_jpeg_file(art_path, &bm, sizeof(s_scratch), format, NULL) > 0;
}

static bool decode_embedded(struct mp3entry *id3, int format)
{
    struct bitmap bm;

    if (!id3->has_embedded_albumart ||
        (id3->albumart.type & AA_CLEAR_FLAGS_MASK) != AA_TYPE_JPG)
        return false;

    bm.width = METRO_ALBUMART_SIZE;
    bm.height = METRO_ALBUMART_SIZE;
    bm.data = s_scratch;
#if (LCD_DEPTH > 1)
    bm.maskdata = NULL;
#endif

    return clip_jpeg_file(id3->path, id3->albumart.pos, id3->albumart.size,
                           &bm, sizeof(s_scratch), format, NULL) > 0;
}

bool metro_albumart_load_current(void)
{
    struct mp3entry *id3;
    char art_path[MAX_PATH];
    struct dim dim = { METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE };
    int format = FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT;
    bool ok;

    if (!(audio_status() & AUDIO_STATUS_PLAY))
        return false;

    id3 = audio_current_track();
    if (!id3)
        return false;

    if (s_loaded && !strcmp(s_loaded_path, id3->path))
        return true;

    if (find_albumart(id3, art_path, sizeof(art_path), &dim))
        ok = decode_into_scratch(art_path, format);
    else
        ok = decode_embedded(id3, format);

    if (!ok)
    {
        s_loaded = false;
        s_loaded_path[0] = '\0';
        return false;
    }

    strlcpy(s_loaded_path, id3->path, sizeof(s_loaded_path));
    s_loaded = true;
    return true;
}

const fb_data *metro_albumart_bitmap(void)
{
    return (const fb_data *)s_scratch;
}
