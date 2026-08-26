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
/* D-042 (M7): ver moonlit_art_cache.h para por qué esto vive separado
 * de moonlit_art.c. */
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "dir.h"
#include "kernel.h" /* yield() */
#include "debug.h"  /* DEBUGF() */

#include "moonlit_art_cache.h"
#include "moonlit_art.h"
#include "moonlit_palette.h"
#include "metro_theme.h"
#include "metro_music.h"
#include "metro_albumart.h"
#include "metro_settings.h"
#include "metro_fsutil.h"

static void pfraw_path(int32_t album_seek, char *out, size_t outsz)
{
    char dir[MAX_PATH];

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    snprintf(out, outsz, "%s/%ld-%d.pfraw", dir, (long)album_seek, MOONLIT_ART_CACHE_SIZE);
}

/* Mismo patrón que ensure_cache_dir() de metro_thumbs.c: solo
 * "moonlitcache" (padre) y "moonlitcache/art" (dir) pueden faltar --
 * ".../aura" ya existe desde el primer metro_settings_save(). */
static void ensure_cache_dir(void)
{
    char dir[MAX_PATH], parent[MAX_PATH];
    char *slash;

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    strlcpy(parent, dir, sizeof(parent));
    slash = strrchr(parent, '/');
    if (slash)
        *slash = '\0';

    if (!dir_exists(parent))
        mkdir(parent);
    if (!dir_exists(dir))
        mkdir(dir);
}

bool moonlit_art_load_for_album(int32_t album_seek, fb_data *out)
{
    char path[MAX_PATH];
    metro_music_item_t track;
    char track_path[MAX_PATH];
    int32_t theme = (int32_t)metro_theme_get();

    pfraw_path(album_seek, path, sizeof(path));

    if (moonlit_art_read_pfraw(path, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS,
                                theme, out))
    {
        DEBUGF("moonlit_art: hit %ld\n", (long)album_seek);
        return true;
    }

    if (metro_music_songs_of_album(album_seek, &track, 1) < 1)
        return false;
    if (!metro_music_track_path(track.seek, track_path, sizeof(track_path)))
        return false;
    if (!metro_albumart_decode_track_cover_sized(track_path, out, MOONLIT_ART_CACHE_SIZE))
        return false;

    DEBUGF("moonlit_art: decode %ld\n", (long)album_seek);

    moonlit_art_mask_corners(out, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS,
                              moonlit_color(MROLE_SURFACE));

    ensure_cache_dir();
    moonlit_art_write_pfraw(path, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS, theme, out);
    return true;
}

/* D-224 (patrón AF/aura_music.c:221-300): scratch estático (no en el
 * stack del hilo de UI, mismo criterio que metro_music.c/D-226) para
 * la tapa que cada vuelta de la pasada va a llenar -- se descarta
 * después de escribirse a disco, el llamador real de Marea nunca la
 * lee de aquí. */
static metro_music_item_t s_precache_albums[METRO_MUSIC_MAX_GROUPS];
static fb_data s_precache_cover[MOONLIT_ART_CACHE_SIZE * MOONLIT_ART_CACHE_SIZE];

void moonlit_art_precache(moonlit_art_progress_fn progress_cb)
{
    int count, i;

    count = metro_music_albums(s_precache_albums, METRO_MUSIC_MAX_GROUPS);
    if (count <= 0)
        return;

    for (i = 0; i < count; i++)
    {
        /* moonlit_art_load_for_album() decide hit vs. decode -- un
         * álbum sin carátula resoluble nunca escribe .pfraw (devuelve
         * false) y por lo tanto vuelve a intentar el decode en cada
         * arranque, sin caché negativa a propósito (mismo criterio
         * que aura_music_precache_album_art(), AF/aura_music.c:221-300). */
        moonlit_art_load_for_album(s_precache_albums[i].seek, s_precache_cover);
        if (progress_cb)
            progress_cb(i + 1, count);
        yield();
    }
}

static bool s_precached = false;

void moonlit_art_cache_on_db_ready(void)
{
    if (s_precached)
        return;
    s_precached = true;
    moonlit_art_precache(NULL);
}
