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
/* D-042/D-059: see moonlit_art_cache.h. */
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "dir.h"
#include "debug.h"
#include "crc32.h"
#include "string-extra.h"

#include "moonlit_art_cache.h"
#include "moonlit_art.h"
#include "moonlit_master_art.h"
#include "moonlit_master_art_builder.h"
#include "moonlit_palette.h"
#include "metro_music.h"
#include "metro_albumart.h"
#include "metro_settings.h"

/* D-226: never on the 8 KB UI stack. 33 800 B, UI thread only. */
static fb_data s_master_scratch[MOONLIT_MASTER_ART_MAX_SIZE * MOONLIT_MASTER_ART_MAX_SIZE];

static void album_master_path(const char *key, char *out, size_t outsz)
{
    char dir[MAX_PATH];

    metro_settings_shared_art_dir("albums", dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s.art", dir, key);
}

bool moonlit_art_master_path(int32_t seek, char *out, size_t outsz)
{
    char key[METRO_MUSIC_ART_KEY_LEN];

    if (!metro_music_album_art_key(seek, key, sizeof(key)))
    {
        out[0] = '\0';
        return false;
    }
    album_master_path(key, out, outsz);
    return true;
}

bool moonlit_art_master_path_peek(int32_t seek, char *out, size_t outsz)
{
    char key[METRO_MUSIC_ART_KEY_LEN];

    if (!metro_music_album_art_key_peek(seek, key, sizeof(key)))
    {
        out[0] = '\0';
        return false;
    }
    album_master_path(key, out, outsz);
    return true;
}

void moonlit_art_master_file_path(char prefix, const char *subdir, const char *file_path,
                                  long mtime, char *out, size_t outsz)
{
    char dir[MAX_PATH];
    char key[MOONLIT_MASTER_ART_KEY_LEN];

    moonlit_master_art_file_key(prefix, crc_32(file_path, strlen(file_path), 0xffffffff),
                                mtime, key, sizeof(key));
    metro_settings_shared_art_dir(subdir, dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s.art", dir, key);
}

static void derive_marea_cover(const fb_data *master, fb_data *out)
{
    moonlit_master_art_box_downscale(master, MOONLIT_MASTER_ART_ALBUM_SIZE,
                                     out, MOONLIT_ART_CACHE_SIZE);
    moonlit_art_mask_corners(out, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS,
                              moonlit_color(MROLE_SURFACE));
}

bool moonlit_art_derive_from_master(const char *master_path, fb_data *out)
{
    if (!moonlit_master_art_read(master_path, MOONLIT_MASTER_ART_ALBUM_SIZE, s_master_scratch))
        return false;
    derive_marea_cover(s_master_scratch, out);
    return true;
}

/* D-056/D-059: the album was looked at and has no resolvable cover --
 * leave the SHARED 0-byte marker so neither the builder nor any
 * family tries the decode again. Same key as the master (includes the
 * track's mtime): a cover added by rewriting the track re-keys the
 * album and retries by itself; a cover.jpg dropped next to an
 * untouched track does NOT (documented limitation, D-056). */
static enum moonlit_art_result give_up(const char *master_path, int32_t album_seek)
{
    char none[MAX_PATH];
    char dir[MAX_PATH];

    (void)album_seek; /* DEBUGF() only */
    if (moonlit_master_art_none_path(master_path, none, sizeof(none)))
    {
        metro_settings_shared_art_dir("albums", dir, sizeof(dir));
        moonlit_master_art_ensure_dir(dir);
        moonlit_master_art_write_none(none);
        DEBUGF("moonlit_art: none %ld\n", (long)album_seek);
    }
    return MOONLIT_ART_NONE;
}

enum moonlit_art_result moonlit_art_load_for_album(int32_t album_seek, fb_data *out)
{
    char path[MAX_PATH];
    char none[MAX_PATH];
    char dir[MAX_PATH];
    metro_music_item_t track;
    char track_path[MAX_PATH];
    const fb_data *px;
    int w, h;

    if (!moonlit_art_master_path(album_seek, path, sizeof(path)))
        return MOONLIT_ART_NONE;

    if (moonlit_art_derive_from_master(path, out))
    {
        DEBUGF("moonlit_art: hit %ld\n", (long)album_seek);
        return MOONLIT_ART_LOADED;
    }

    if (moonlit_master_art_none_path(path, none, sizeof(none))
        && moonlit_master_art_none_exists(none))
        return MOONLIT_ART_NONE;

    /* D-059: the builder is walking the library -- it will get here;
     * asking it to come sooner is all the UI thread does. */
    if (moonlit_master_art_builder_active())
    {
        moonlit_master_art_builder_hint_album(album_seek);
        return MOONLIT_ART_WAITING;
    }

    if (metro_music_songs_of_album(album_seek, &track, 1) < 1)
        return give_up(path, album_seek);
    if (!metro_music_track_path(track.seek, track_path, sizeof(track_path)))
        return give_up(path, album_seek);
    px = metro_albumart_decode_track_cover_ui(track_path, &w, &h);
    if (!px)
        return give_up(path, album_seek);

    DEBUGF("moonlit_art: decode %ld\n", (long)album_seek);

    /* Contract v16: always write the master when a JPEG was decoded. */
    moonlit_master_art_resample_cover(px, w, h, s_master_scratch,
                                      MOONLIT_MASTER_ART_ALBUM_SIZE);
    metro_settings_shared_art_dir("albums", dir, sizeof(dir));
    moonlit_master_art_ensure_dir(dir);
    moonlit_master_art_write(path, MOONLIT_MASTER_ART_ALBUM_SIZE, s_master_scratch);
    if (none[0])
        remove(none);
    derive_marea_cover(s_master_scratch, out);
    return MOONLIT_ART_LOADED;
}

void moonlit_art_library_changed(void)
{
    moonlit_master_art_builder_kick();
}

/* --- D-055: orphan sweep request flag ---------------------------------- */

#define GC_FLAG_NAME ".gc-pending"

/* Only the flag lives under moonlitcache/art/ since D-059 (the
 * .pfraw/.none files it used to hold moved/went away with the master
 * cache, moonlit_master_art_builder.c migrate_legacy_once()). */
static void gc_flag_path(char *out, size_t outsz)
{
    char dir[MAX_PATH];

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    strlcpy(out, dir, outsz);
    strlcat(out, "/" GC_FLAG_NAME, outsz);
}

void moonlit_art_request_gc(void)
{
    char path[MAX_PATH], dir[MAX_PATH];
    int fd;

    metro_music_album_art_key_reset();
    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    moonlit_master_art_ensure_dir(dir);
    gc_flag_path(path, sizeof(path));
    fd = creat(path, 0666);
    if (fd >= 0)
        close(fd);
    moonlit_master_art_builder_kick();
}

bool moonlit_art_gc_pending(void)
{
    char path[MAX_PATH];

    gc_flag_path(path, sizeof(path));
    return file_exists(path);
}

void moonlit_art_gc_clear(void)
{
    char path[MAX_PATH];

    gc_flag_path(path, sizeof(path));
    remove(path);
}
