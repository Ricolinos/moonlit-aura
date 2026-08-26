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
#include "crc32.h"  /* crc_32() -- D-055 gc table */
#include "string-extra.h" /* strlcpy()/strlcat() */
#include "tagcache.h" /* tagcache_get_stat() -- D-055 pending-count memo */

#include "moonlit_art_cache.h"
#include "moonlit_art.h"
#include "moonlit_palette.h"
#include "metro_theme.h"
#include "metro_music.h"
#include "metro_albumart.h"
#include "metro_settings.h"
#include "metro_fsutil.h"

bool moonlit_art_pfraw_path(int32_t seek, int size, char *out, size_t outsz)
{
    char dir[MAX_PATH];
    char key[METRO_MUSIC_ART_KEY_LEN];

    if (!metro_music_album_art_key(seek, key, sizeof(key)))
    {
        out[0] = '\0';
        return false;
    }
    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s-%d.pfraw", dir, key, size);
    return true;
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

    if (!moonlit_art_pfraw_path(album_seek, MOONLIT_ART_CACHE_SIZE, path, sizeof(path)))
        return false;

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

/* D-049: index -> album seek -> .pfraw path, for the pure counter in
 * moonlit_art.c (moonlit_art_count_uncached()). */
static void precache_path_at(int index, char *out, size_t outsz, void *ctx)
{
    (void)ctx;
    /* D-055: an album with no resolvable track has no cache file and
     * nothing to decode -- empty path, count_uncached_now() skips it. */
    if (!moonlit_art_pfraw_path(s_precache_albums[index].seek, MOONLIT_ART_CACHE_SIZE,
                                out, outsz))
        out[0] = '\0';
}

static int load_albums(void)
{
    return metro_music_albums(s_precache_albums, METRO_MUSIC_MAX_GROUPS);
}

/* D-055: with stable keys the expensive answer ("nothing pending") is
 * also the stable one -- remember it per (tagcache total_entries,
 * theme) so re-entering Música doesn't pay one tagcache lookup + one
 * header read per album every time (D-049 made that pass cheap in
 * disk terms; the key lookup adds a tagcache search per album). A
 * decode pass or a sync (moonlit_art_request_gc()) forgets it. */
static int s_nothing_pending_entries = -1;
static int32_t s_nothing_pending_theme = -1;

static int count_uncached_now(int count, int32_t theme)
{
    int i, n = 0;
    char path[MAX_PATH];

    for (i = 0; i < count; i++)
    {
        precache_path_at(i, path, sizeof(path), NULL);
        if (!path[0])
            continue; /* no track -> nothing to cache, never pending */
        if (!moonlit_art_pfraw_is_cached(path, MOONLIT_ART_CACHE_SIZE,
                                         MOONLIT_ART_CACHE_RADIUS, theme))
            n++;
        if ((i & 31) == 31)
            yield();
    }
    return n;
}

int moonlit_art_pending_count(void)
{
    int32_t theme = (int32_t)metro_theme_get();
    int entries = tagcache_get_stat()->total_entries;
    int count, pending;

    if (entries == s_nothing_pending_entries && theme == s_nothing_pending_theme)
        return 0;

    count = load_albums();
    if (count <= 0)
        return 0;
    pending = count_uncached_now(count, theme);
    if (pending == 0)
    {
        s_nothing_pending_entries = entries;
        s_nothing_pending_theme = theme;
    }
    return pending;
}

bool moonlit_art_precache(moonlit_art_progress_fn progress_cb,
                          moonlit_art_abort_fn should_abort)
{
    char path[MAX_PATH];
    int32_t theme = (int32_t)metro_theme_get();
    int count, i, pending, done = 0;

    count = load_albums();
    if (count <= 0)
        return true;

    /* D-049 (AF/aura_music.c:352-358): count first so the progress the
     * screen shows is "N of the ones actually missing", not "N of the
     * whole library" -- and so a library with nothing missing costs
     * `count` header reads, never a screen. */
    pending = count_uncached_now(count, theme);
    if (pending == 0)
        return true;
    s_nothing_pending_entries = -1;

    for (i = 0; i < count; i++)
    {
        /* D-049: header-only check before the full open()+read() that
         * moonlit_art_load_for_album() would do on a hit -- measured on
         * the owner's iPod, the old "load everything, let the hit path
         * decide" pass cost 264 ms per album. */
        if (!moonlit_art_pfraw_path(s_precache_albums[i].seek, MOONLIT_ART_CACHE_SIZE,
                                    path, sizeof(path)))
            continue;
        if (moonlit_art_pfraw_is_cached(path, MOONLIT_ART_CACHE_SIZE,
                                        MOONLIT_ART_CACHE_RADIUS, theme))
            continue;

        /* moonlit_art_load_for_album() decide hit vs. decode -- un
         * álbum sin carátula resoluble nunca escribe .pfraw (devuelve
         * false) y por lo tanto vuelve a intentar el decode en cada
         * pasada, sin caché negativa a propósito (mismo criterio
         * que aura_music_precache_album_art(), AF/aura_music.c:221-300). */
        moonlit_art_load_for_album(s_precache_albums[i].seek, s_precache_cover);
        done++;
        if (progress_cb)
            progress_cb(done, pending);
        yield();

        /* D-049: between albums only -- never mid-decode, so an abort
         * leaves the cache consistent (every .pfraw on disk complete). */
        if (should_abort && should_abort())
            return false;
    }
    return true;
}

/* --- D-055: limpieza de huérfanos ------------------------------------ */

#define GC_FLAG_NAME ".gc-pending"
#define MOONLIT_ART_STR_(x) #x
#define MOONLIT_ART_STR(x)  MOONLIT_ART_STR_(x)

static void gc_flag_path(char *out, size_t outsz)
{
    char dir[MAX_PATH];

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    strlcpy(out, dir, outsz);
    strlcat(out, "/" GC_FLAG_NAME, outsz);
}

void moonlit_art_request_gc(void)
{
    char path[MAX_PATH];
    int fd;

    s_nothing_pending_entries = -1;
    metro_music_album_art_key_reset();
    ensure_cache_dir();
    gc_flag_path(path, sizeof(path));
    fd = creat(path, 0666);
    if (fd >= 0)
        close(fd);
}

bool moonlit_art_gc_pending(void)
{
    char path[MAX_PATH];

    gc_flag_path(path, sizeof(path));
    return file_exists(path);
}

/* crc32 de las claves vigentes en el scratch de la precarga (28 800 B
 * = 7 200 entradas, > METRO_MUSIC_MAX_GROUPS). */
#define GC_TABLE_CAP ((int)(sizeof(s_precache_cover) / sizeof(uint32_t)))

static uint32_t key_crc(const char *key)
{
    return crc_32(key, strlen(key), 0xffffffff);
}

static bool gc_table_has(const uint32_t *table, int n, uint32_t crc)
{
    int i;

    for (i = 0; i < n; i++)
        if (table[i] == crc)
            return true;
    return false;
}

/* Borra de `dir` todo archivo que termine en `suffix` cuyo tallo
 * (nombre sin el sufijo) no esté en la tabla. Un nombre que no
 * termine en `suffix` (la bandera, otros tamaños) se deja. */
static void gc_sweep(const char *dir, const char *suffix, const uint32_t *table, int n)
{
    DIR *d = opendir(dir);
    struct DIRENT *entry;
    size_t suffix_len = strlen(suffix);
    char stem[METRO_FSUTIL_NAME_LEN];
    char full[MAX_PATH];

    if (!d)
        return;
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
        if (gc_table_has(table, n, key_crc(stem)))
            continue;
        strlcpy(full, dir, sizeof(full));
        strlcat(full, "/", sizeof(full));
        strlcat(full, name, sizeof(full));
        remove(full);
        DEBUGF("moonlit_art: gc %s\n", name);
    }
    closedir(d);
}

void moonlit_art_gc(void)
{
    uint32_t *table = (uint32_t *)s_precache_cover;
    char key[METRO_MUSIC_ART_KEY_LEN];
    char dir[MAX_PATH], path[MAX_PATH];
    int count = load_albums();
    int i, n = 0;

    for (i = 0; i < count && n < GC_TABLE_CAP; i++)
    {
        if (metro_music_album_art_key(s_precache_albums[i].seek, key, sizeof(key)))
            table[n++] = key_crc(key);
        if ((i & 31) == 31)
            yield();
    }

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    gc_sweep(dir, "-" MOONLIT_ART_STR(MOONLIT_ART_CACHE_SIZE) ".pfraw", table, n);

    metro_settings_shared_thumbs_dir("albums", dir, sizeof(dir));
    gc_sweep(dir, ".mth", table, n);

    gc_flag_path(path, sizeof(path));
    remove(path);
}
