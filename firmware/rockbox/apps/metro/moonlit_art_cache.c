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
#include "dir.h"    /* dir_exists()/mkdir() */
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

/* D-056: "<dir>/<key>.none" next to "<dir>/<key>-120.pfraw". */
static bool none_path_for(const char *pfraw_path, char *out, size_t outsz)
{
    return moonlit_art_none_path(pfraw_path, out, outsz);
}

/* D-056: the album was looked at and has no resolvable cover -- leave
 * the 0-byte marker so neither the pre-pass nor Marea's tick tries the
 * decode again. Same key as the .pfraw (D-055, includes the track's
 * mtime): a cover added by rewriting the track re-keys the album and
 * retries by itself; a cover.jpg dropped next to an untouched track
 * does NOT (documented limitation, DECISIONS.md D-056). */
static bool give_up(const char *pfraw_path, int32_t album_seek)
{
    char none[MAX_PATH];

    (void)album_seek; /* DEBUGF() only */
    if (none_path_for(pfraw_path, none, sizeof(none)))
    {
        ensure_cache_dir();
        moonlit_art_write_none(none);
        DEBUGF("moonlit_art: none %ld\n", (long)album_seek);
    }
    return false;
}

bool moonlit_art_load_for_album(int32_t album_seek, fb_data *out)
{
    char path[MAX_PATH];
    char none[MAX_PATH];
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

    /* D-056: negative hit -- monogram, no track open, no JPEG decode. */
    if (none_path_for(path, none, sizeof(none)) && moonlit_art_none_exists(none))
        return false;

    if (metro_music_songs_of_album(album_seek, &track, 1) < 1)
        return give_up(path, album_seek);
    if (!metro_music_track_path(track.seek, track_path, sizeof(track_path)))
        return give_up(path, album_seek);
    if (!metro_albumart_decode_track_cover_sized(track_path, out, MOONLIT_ART_CACHE_SIZE))
        return give_up(path, album_seek);

    DEBUGF("moonlit_art: decode %ld\n", (long)album_seek);

    moonlit_art_mask_corners(out, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS,
                              moonlit_color(MROLE_SURFACE));

    ensure_cache_dir();
    moonlit_art_write_pfraw(path, MOONLIT_ART_CACHE_SIZE, MOONLIT_ART_CACHE_RADIUS, theme, out);
    /* a stale marker under the same key cannot normally coexist with a
     * decode (it would have short-circuited above) -- cheap to be sure */
    if (none[0])
        remove(none);
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

/* D-055/D-056: remember the pre-pass answer per (tagcache
 * total_entries, theme, generation) -- ALWAYS, not only when it is 0.
 * With stable keys plus the negative cache every album ends up either
 * .pfraw or .none after one complete pass, so the count only changes
 * when the library does; before D-056 an album whose cover could not
 * be decoded stayed pending forever and, since only 0 was memoized,
 * every entry into Música re-walked tagcache, showed the screen and
 * re-failed the same 57 decodes on the owner's iPod. The generation
 * bumps on moonlit_art_pending_invalidate(): sync finish_ok()
 * (via moonlit_art_request_gc()), the bootstrap seal in
 * metro_music_db_ready(), and an aborted precache. A completed
 * precache stores 0 directly. */
static int s_pending_memo_entries = -1;
static int32_t s_pending_memo_theme = -1;
static unsigned s_pending_memo_gen_seen = 0;
static unsigned s_pending_gen = 1;
static int s_pending_memo_value = 0;

void moonlit_art_pending_invalidate(void)
{
    s_pending_gen++;
}

static void remember_pending(int entries, int32_t theme, int value)
{
    s_pending_memo_entries = entries;
    s_pending_memo_theme = theme;
    s_pending_memo_gen_seen = s_pending_gen;
    s_pending_memo_value = value;
}

static int count_uncached_now(int count, int32_t theme)
{
    int i, n = 0;
    char path[MAX_PATH];

    for (i = 0; i < count; i++)
    {
        precache_path_at(i, path, sizeof(path), NULL);
        if (!path[0])
            continue; /* no track -> nothing to cache, never pending */
        if (!moonlit_art_is_resolved(path, MOONLIT_ART_CACHE_SIZE,
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

    if (entries == s_pending_memo_entries && theme == s_pending_memo_theme
        && s_pending_memo_gen_seen == s_pending_gen)
        return s_pending_memo_value;

    count = load_albums();
    if (count <= 0)
        return 0;
    pending = count_uncached_now(count, theme);
    remember_pending(entries, theme, pending);
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
    moonlit_art_pending_invalidate();

    for (i = 0; i < count; i++)
    {
        /* D-049: header-only check before the full open()+read() that
         * moonlit_art_load_for_album() would do on a hit -- measured on
         * the owner's iPod, the old "load everything, let the hit path
         * decide" pass cost 264 ms per album. */
        if (!moonlit_art_pfraw_path(s_precache_albums[i].seek, MOONLIT_ART_CACHE_SIZE,
                                    path, sizeof(path)))
            continue;
        if (moonlit_art_is_resolved(path, MOONLIT_ART_CACHE_SIZE,
                                    MOONLIT_ART_CACHE_RADIUS, theme))
            continue;

        /* moonlit_art_load_for_album() decide hit vs. decode. D-056: un
         * álbum sin carátula resoluble deja su "<clave>.none" en este
         * mismo pase (antes, sin caché negativa -- criterio heredado de
         * aura_music_precache_album_art(), AF/aura_music.c:221-300 --
         * volvía a contar como pendiente y a fallar el decode en cada
         * entrada a Música). */
        moonlit_art_load_for_album(s_precache_albums[i].seek, s_precache_cover);
        done++;
        if (progress_cb)
            progress_cb(done, pending);
        yield();

        /* D-049: between albums only -- never mid-decode, so an abort
         * leaves the cache consistent (every .pfraw on disk complete). */
        if (should_abort && should_abort())
        {
            moonlit_art_pending_invalidate();
            return false;
        }
    }
    /* D-056: every album is now .pfraw or .none -- the next entry into
     * Música must not walk tagcache again to learn that. */
    remember_pending(tagcache_get_stat()->total_entries, theme, 0);
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

    moonlit_art_pending_invalidate();
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

/* D-056: keep = the stem's crc32 is in the live-key table. Lo que
 * antes era gc_sweep() estática vive ahora en moonlit_art.c
 * (moonlit_art_sweep(), host-testable). */
struct gc_ctx {
    const uint32_t *table;
    int n;
};

static bool gc_keep(const char *stem, void *ctx)
{
    const struct gc_ctx *c = ctx;

    return gc_table_has(c->table, c->n, key_crc(stem));
}

void moonlit_art_gc(void)
{
    uint32_t *table = (uint32_t *)s_precache_cover;
    char key[METRO_MUSIC_ART_KEY_LEN];
    char dir[MAX_PATH], path[MAX_PATH];
    int count = load_albums();
    int i, n = 0;
    struct gc_ctx ctx;

    for (i = 0; i < count && n < GC_TABLE_CAP; i++)
    {
        if (metro_music_album_art_key(s_precache_albums[i].seek, key, sizeof(key)))
            table[n++] = key_crc(key);
        if ((i & 31) == 31)
            yield();
    }

    ctx.table = table;
    ctx.n = n;

    metro_settings_metro_cache_dir("art", dir, sizeof(dir));
    moonlit_art_sweep(dir, "-" MOONLIT_ART_STR(MOONLIT_ART_CACHE_SIZE) ".pfraw", gc_keep, &ctx);
    /* D-056: an orphan .none (album gone / re-keyed) goes the same way */
    moonlit_art_sweep(dir, ".none", gc_keep, &ctx);

    metro_settings_shared_thumbs_dir("albums", dir, sizeof(dir));
    moonlit_art_sweep(dir, ".mth", gc_keep, &ctx);

    gc_flag_path(path, sizeof(path));
    remove(path);
}
