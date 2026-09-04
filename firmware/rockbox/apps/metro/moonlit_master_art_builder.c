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
/* D-059: see moonlit_master_art_builder.h. */
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "kernel.h"   /* sleep(), yield(), struct mutex */
#include "thread.h"   /* create_thread() */
#include "file.h"
#include "dir.h"
#include "debug.h"
#include "crc32.h"
#include "string-extra.h"
#include "tagcache.h"
#include "metadata.h" /* struct mp3entry */

#include "moonlit_master_art_builder.h"
#include "moonlit_master_art.h"
#include "moonlit_art.h"       /* moonlit_art_sweep() -- gc */
#include "moonlit_art_cache.h" /* gc flag */
#include "metro_music.h"
#include "metro_albumart.h"
#include "metro_settings.h"
#include "metro_photos.h"
#include "metro_fsutil.h"

/* --- memory: all static, all this thread's ------------------------------ */

/* Below PRIORITY_BACKGROUND (tagcache's own thread): the database scan
 * and the codec buffering thread (PRIORITY_BUFFERING) both win over
 * us at every scheduling point -- the closest this kernel offers to
 * "suspended while audio is buffering" (there is no audio_status()
 * bit for buffering; see DECISIONS.md D-059). */
#define BUILDER_PRIORITY   (PRIORITY_BACKGROUND + 2)
#define BUILDER_STACK_SIZE (DEFAULT_STACK_SIZE + 0x2000)
#define BUILDER_ELEMENT_PACE (HZ / 20)  /* contract: sleep(HZ/20) between elements */
#define BUILDER_PAUSE_POLL   (HZ / 10)
#define BUILDER_IDLE_POLL    (HZ / 2)
#define HINT_RING_N 8

static long s_stack[BUILDER_STACK_SIZE / sizeof(long)];
static struct mutex s_lock;
static unsigned int s_thread_id;
static bool s_thread_running;

static volatile bool s_kick;
static volatile bool s_paused;
static volatile bool s_active;
static volatile unsigned s_generation = 1;
/* D-061: progreso de la pasada. Lo escribe solo el hilo del constructor
 * y lo lee solo el de UI; enteros de palabra y planificacion cooperativa
 * (el cambio de contexto solo ocurre en yield/sleep/bloqueo), asi que
 * sin candado -- misma disciplina que s_paused/s_active. */
static volatile moonlit_master_art_phase_t s_phase = MOONLIT_MASTER_ART_PHASE_IDLE;
static volatile int  s_phase_done = 0;
static volatile int  s_phase_total = 0;   /* 0 = desconocido (streaming) */
static volatile bool s_pass_done = false;
static volatile bool s_foreground = false;
static bool s_legacy_migrated;

static int32_t s_hints[HINT_RING_N];
static int s_hints_n;

/* Album seeks of the pass -- 8 KB instead of the 432 KB
 * metro_music_item_t array the old precache kept (labels are never
 * needed here). */
static int32_t s_seeks[METRO_MUSIC_MAX_GROUPS];
static int s_seeks_n;

/* Decode scratch (136 px KEEP_ASPECT box + JPEG working room) and the
 * finished master. Never metro_albumart.c's s_scratch (UI-only, and
 * Now Playing keeps a pointer into it). Doubles as the gc key table. */
static unsigned char s_decode[METRO_ALBUMART_SCRATCH_SIZE];
static fb_data s_master[MOONLIT_MASTER_ART_MAX_SIZE * MOONLIT_MASTER_ART_MAX_SIZE];
static struct mp3entry s_id3;

/* --- lock / flags --------------------------------------------------------- */

void moonlit_master_art_lock(void)
{
    mutex_lock(&s_lock);
}

void moonlit_master_art_unlock(void)
{
    mutex_unlock(&s_lock);
}

void moonlit_master_art_builder_init(void)
{
    mutex_init(&s_lock);
}

void moonlit_master_art_builder_kick(void)
{
    s_kick = true;
}

void moonlit_master_art_builder_pause(bool paused)
{
    s_paused = paused;
}

bool moonlit_master_art_builder_active(void)
{
    return s_active;
}

unsigned moonlit_master_art_builder_generation(void)
{
    return s_generation;
}

void moonlit_master_art_builder_hint_album(int32_t album_seek)
{
    int i;

    for (i = 0; i < s_hints_n; i++)
        if (s_hints[i] == album_seek)
            return;
    if (s_hints_n < HINT_RING_N)
        s_hints[s_hints_n++] = album_seek;
}

static bool db_usable(void)
{
    return tagcache_is_usable() && tagcache_is_fully_initialized();
}

/* Between elements: honour a pause, then the contract's pace. */
static void pace(bool did_work)
{
    /* D-061: en primer plano el usuario esta MIRANDO esta pasada -- ni la
     * pausa de animacion (la levanta la cadencia de Marea, que no esta en
     * pantalla) ni el paso de HZ/20 tienen sentido. Se cede el turno para
     * que la pantalla se redibuje, y nada mas. */
    if (s_foreground)
    {
        yield();
        return;
    }
    while (s_paused)
        sleep(BUILDER_PAUSE_POLL);
    if (did_work)
        sleep(BUILDER_ELEMENT_PACE);
    else
        yield();
}

/* --- one element ---------------------------------------------------------- */

static void master_path_for(const char *subdir, const char *key, char *out, size_t outsz)
{
    char dir[MAX_PATH];

    metro_settings_shared_art_dir(subdir, dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s.art", dir, key);
}

/* Writes `master` (or the .none marker when `ok` is false) for `art`
 * and bumps the generation. Runs under the lock: the pixels in
 * s_master came from a decode that must not be interleaved either. */
static void commit_element(const char *art, int size, bool ok)
{
    char none[MAX_PATH];
    char dir[MAX_PATH];

    strlcpy(dir, art, sizeof(dir));
    if (strrchr(dir, '/'))
        *strrchr(dir, '/') = '\0';
    moonlit_master_art_ensure_dir(dir);

    if (ok)
    {
        moonlit_master_art_write(art, size, s_master);
        if (moonlit_master_art_none_path(art, none, sizeof(none)))
            remove(none);
    }
    else if (moonlit_master_art_none_path(art, none, sizeof(none)))
        moonlit_master_art_write_none(none);
    s_generation++;
}

/* true if it did disk work (decode/write), false if already resolved. */
static bool build_album(int32_t seek)
{
    char key[METRO_MUSIC_ART_KEY_LEN];
    char track[MAX_PATH];
    char art[MAX_PATH];
    int w, h;
    bool ok;

    if (!metro_music_album_art_source(seek, key, sizeof(key), track, sizeof(track)))
        return false; /* no resolvable track: nothing to cache, never pending */
    master_path_for("albums", key, art, sizeof(art));
    if (moonlit_master_art_is_resolved(art, MOONLIT_MASTER_ART_ALBUM_SIZE))
        return false;

    moonlit_master_art_lock();
    ok = metro_albumart_decode_track_cover_raw(track, &s_id3, s_decode, sizeof(s_decode),
                                               &w, &h);
    if (ok)
        moonlit_master_art_resample_cover((const fb_data *)s_decode, w, h,
                                          s_master, MOONLIT_MASTER_ART_ALBUM_SIZE);
    commit_element(art, MOONLIT_MASTER_ART_ALBUM_SIZE, ok);
    moonlit_master_art_unlock();
    DEBUGF("master_art: album %s %s\n", key, ok ? "built" : "none");
    return true;
}

static bool build_file(char prefix, const char *subdir, const char *path, long mtime,
                       int size)
{
    char key[MOONLIT_MASTER_ART_KEY_LEN];
    char art[MAX_PATH];
    int w, h;
    bool ok;

    moonlit_master_art_file_key(prefix, crc_32(path, strlen(path), 0xffffffff), mtime,
                                key, sizeof(key));
    master_path_for(subdir, key, art, sizeof(art));
    if (moonlit_master_art_is_resolved(art, size))
        return false;

    moonlit_master_art_lock();
    ok = metro_albumart_decode_file_raw(path, s_decode, sizeof(s_decode), &w, &h);
    if (ok)
        moonlit_master_art_resample_cover((const fb_data *)s_decode, w, h, s_master, size);
    commit_element(art, size, ok);
    moonlit_master_art_unlock();
    DEBUGF("master_art: %s %s %s\n", subdir, key, ok ? "built" : "none");
    return true;
}

static void service_hints(void)
{
    while (s_hints_n > 0)
    {
        int32_t seek = s_hints[0];
        int i;

        for (i = 1; i < s_hints_n; i++)
            s_hints[i - 1] = s_hints[i];
        s_hints_n--;
        pace(build_album(seek));
    }
}

/* --- enumeration ---------------------------------------------------------- */

/* Album seeks straight from the tag_album index (each album once, no
 * uniqbuf needed without a filter) -- no labels, no sorting. */
static bool enumerate_albums(void)
{
    struct tagcache_search tcs;
    char buf[TAGCACHE_BUFSZ];

    s_seeks_n = 0;
    if (!tagcache_search(&tcs, tag_album))
        return false;
    while (s_seeks_n < METRO_MUSIC_MAX_GROUPS && tagcache_get_next(&tcs, buf, sizeof(buf)))
        s_seeks[s_seeks_n++] = tcs.result_seek;
    tagcache_search_finish(&tcs);
    return true;
}

static bool has_ext(const char *name, const char *ext)
{
    size_t len = strlen(name), elen = strlen(ext);

    return len > elen && !strcasecmp(name + len - elen, ext);
}

/* Streams a directory (no arrays: the hub's 500-entry photo lists are
 * UI-side statics we must not share) calling `fn(path, mtime, ctx)`
 * for every non-hidden regular file with one of `exts`. Returns false
 * if a kick/abort happened mid-walk. */
typedef bool (*walk_fn)(const char *path, long mtime, void *ctx);

static bool walk_dir(const char *dir, const char *const *exts, int n_exts,
                     walk_fn fn, void *ctx)
{
    DIR *d = opendir(dir);
    struct DIRENT *entry;
    char path[MAX_PATH];
    int i;

    if (!d)
        return true;
    while ((entry = readdir(d)) != NULL)
    {
        struct dirinfo info;
        bool match = false;

        if (metro_fsutil_is_hidden_name(entry->d_name))
            continue;
        for (i = 0; i < n_exts && !match; i++)
            match = has_ext(entry->d_name, exts[i]);
        if (!match)
            continue;
        info = dir_get_info(d, entry);
        if (info.attribute & ATTR_DIRECTORY)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (!fn(path, (long)info.mtime, ctx))
        {
            closedir(d);
            return false;
        }
    }
    closedir(d);
    return true;
}

static const char *const k_artist_exts[] = { ".jpg" };
static const char *const k_photo_exts[] = { METRO_PHOTOS_EXT_JPG, METRO_PHOTOS_EXT_JPEG };

static bool build_artist_cb(const char *path, long mtime, void *ctx)
{
    (void)ctx;
    service_hints();
    pace(build_file('r', "artists", path, mtime, MOONLIT_MASTER_ART_ARTIST_SIZE));
    s_phase_done++; /* D-061 */
    return !s_kick;
}

static bool build_photo_cb(const char *path, long mtime, void *ctx)
{
    (void)ctx;
    service_hints();
    pace(build_file('p', "photos", path, mtime, MOONLIT_MASTER_ART_PHOTO_SIZE));
    s_phase_done++; /* D-061 */
    return !s_kick;
}

/* --- legacy (pre-D-059) private cache ------------------------------------- */

/* moonlitcache/art/ used to hold "<key>-120.pfraw" (theme-baked Marea
 * covers, D-042) and "<key>.none" (D-056). The .none markers carry the
 * SAME key as the master and move to /.aura/art/albums/; the .pfraw
 * files are dropped (the master replaces them -- deriving 130 -> 120
 * on load is ~1-2 ms, cheaper than a second 28.8 KB file per album in
 * a flat FAT directory, DECISIONS.md D-059). Only the ".gc-pending"
 * flag stays there. Once per boot. */
static void migrate_legacy_once(void)
{
    char src_dir[MAX_PATH], dst_dir[MAX_PATH];
    char from[MAX_PATH], to[MAX_PATH];
    DIR *d;
    struct DIRENT *entry;

    if (s_legacy_migrated)
        return;
    s_legacy_migrated = true;

    metro_settings_metro_cache_dir("art", src_dir, sizeof(src_dir));
    d = opendir(src_dir);
    if (!d)
        return;
    metro_settings_shared_art_dir("albums", dst_dir, sizeof(dst_dir));
    moonlit_master_art_ensure_dir(dst_dir);

    while ((entry = readdir(d)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        snprintf(from, sizeof(from), "%s/%s", src_dir, entry->d_name);
        if (has_ext(entry->d_name, ".pfraw"))
            remove(from);
        else if (has_ext(entry->d_name, ".none"))
        {
            snprintf(to, sizeof(to), "%s/%s", dst_dir, entry->d_name);
            if (moonlit_master_art_none_exists(to) || rename(from, to) != 0)
                remove(from);
        }
        yield();
    }
    closedir(d);
    DEBUGF("master_art: legacy moonlitcache/art migrated\n");
}

/* --- gc (D-055/D-056, now here) ------------------------------------------- */

#define GC_TABLE_CAP ((int)(sizeof(s_decode) / sizeof(uint32_t)))

struct gc_ctx {
    uint32_t *table;
    int n;
};

static uint32_t stem_crc(const char *stem)
{
    return crc_32(stem, strlen(stem), 0xffffffff);
}

static void gc_add(struct gc_ctx *g, const char *stem)
{
    if (g->n < GC_TABLE_CAP)
        g->table[g->n++] = stem_crc(stem);
}

static bool gc_keep(const char *stem, void *ctx)
{
    const struct gc_ctx *g = ctx;
    uint32_t crc = stem_crc(stem);
    int i;

    for (i = 0; i < g->n; i++)
        if (g->table[i] == crc)
            return true;
    return false;
}

/* Live stems for one file source: the master's "<p>-<crc>.<mtime>"
 * and the .mth's "<file>.<mtime>" (metro_thumbs.c's own key). */
struct gc_file_ctx {
    struct gc_ctx *g;
    char prefix;
};

static bool gc_file_cb(const char *path, long mtime, void *ctx)
{
    struct gc_file_ctx *c = ctx;
    char stem[MAX_PATH];
    const char *name = strrchr(path, '/');

    moonlit_master_art_file_key(c->prefix, crc_32(path, strlen(path), 0xffffffff), mtime,
                                stem, sizeof(stem));
    gc_add(c->g, stem);
    snprintf(stem, sizeof(stem), "%s.%ld", name ? name + 1 : path, mtime);
    gc_add(c->g, stem);
    return true;
}

static void gc_sweep_source(const char *subdir, struct gc_ctx *g)
{
    char dir[MAX_PATH];

    metro_settings_shared_art_dir(subdir, dir, sizeof(dir));
    moonlit_art_sweep(dir, ".art", gc_keep, g);
    moonlit_art_sweep(dir, ".none", gc_keep, g);
    metro_settings_shared_thumbs_dir(subdir, dir, sizeof(dir));
    moonlit_art_sweep(dir, ".mth", gc_keep, g);
}

static void run_gc(void)
{
    struct gc_ctx g = { (uint32_t *)s_decode, 0 };
    struct gc_file_ctx fc;
    char key[METRO_MUSIC_ART_KEY_LEN];
    char path[MAX_PATH];
    int i;

    /* albums: same stems for /.aura/art/albums (.art/.none) and
     * /.aura/thumbs/albums (.mth) */
    for (i = 0; i < s_seeks_n; i++)
    {
        if (metro_music_album_art_source(s_seeks[i], key, sizeof(key), path, sizeof(path)))
            gc_add(&g, key);
        if ((i & 31) == 31)
            yield();
    }
    gc_sweep_source("albums", &g);

    g.n = 0;
    fc.g = &g;
    fc.prefix = 'r';
    metro_settings_artists_dir(path, sizeof(path));
    walk_dir(path, k_artist_exts, 1, gc_file_cb, &fc);
    gc_sweep_source("artists", &g);

    g.n = 0;
    fc.prefix = 'p';
    walk_dir(METRO_PHOTOS_DIR, k_photo_exts, 2, gc_file_cb, &fc);
    gc_sweep_source("photos", &g);

    moonlit_art_gc_clear();
    DEBUGF("master_art: gc done\n");
}

/* --- the pass ------------------------------------------------------------- */

/* false = interrupted (kick mid-pass, database went away): the thread
 * runs it again from the top, everything already written stays. */
static bool run_pass(void)
{
    char dir[MAX_PATH];
    int i;

    migrate_legacy_once();

    if (!enumerate_albums())
        return false;
    DEBUGF("master_art: pass start, %d albums\n", s_seeks_n);
    s_phase = MOONLIT_MASTER_ART_PHASE_ALBUMS;
    s_phase_done = 0;
    s_phase_total = s_seeks_n;
    for (i = 0; i < s_seeks_n; i++)
    {
        service_hints();
        if (s_kick || !db_usable())
            return false;
        /* D-061: se cuenta AQUI y no dentro de pace(), que tambien corre
         * desde service_hints() -- un hint de Marea no es un elemento del
         * recorrido y no debe mover el contador de la pantalla. */
        s_phase_done = i;
        pace(build_album(s_seeks[i]));
    }

    s_phase = MOONLIT_MASTER_ART_PHASE_ARTISTS;
    s_phase_done = 0;
    s_phase_total = 0; /* recorrido en streaming: sin conteo previo */
    metro_settings_artists_dir(dir, sizeof(dir));
    if (!walk_dir(dir, k_artist_exts, 1, build_artist_cb, NULL))
        return false;
    s_phase = MOONLIT_MASTER_ART_PHASE_PHOTOS;
    s_phase_done = 0;
    s_phase_total = 0;
    if (!walk_dir(METRO_PHOTOS_DIR, k_photo_exts, 2, build_photo_cb, NULL))
        return false;

    if (moonlit_art_gc_pending())
        run_gc();

    DEBUGF("master_art: pass complete\n");
    return true;
}

static void builder_thread(void)
{
    for (;;)
    {
        bool complete;

        if (!s_kick)
        {
            sleep(BUILDER_IDLE_POLL);
            continue;
        }
        if (!db_usable())
        {
            sleep(HZ);
            continue;
        }
        s_kick = false;
        s_active = true;
        complete = run_pass();
        s_active = false;
        s_phase = MOONLIT_MASTER_ART_PHASE_IDLE;
        s_generation++;
        if (complete)
            s_pass_done = true; /* D-061 */
        else
        {
            s_kick = true;
            sleep(HZ);
        }
    }
}

void moonlit_master_art_builder_poll(void)
{
    if (s_thread_running)
        return;
    if (!metro_music_db_ready() || !tagcache_is_fully_initialized())
        return;

    s_kick = true;
    s_thread_id = create_thread(builder_thread, s_stack, sizeof(s_stack), 0,
                                "master art"
                                IF_PRIO(, BUILDER_PRIORITY)
                                IF_COP(, CPU));
    s_thread_running = (s_thread_id != 0);
    DEBUGF("master_art: builder thread %s\n", s_thread_running ? "started" : "FAILED");
}

/* -- D-061: preparacion explicita --------------------------------------- */

bool moonlit_master_art_builder_progress(moonlit_master_art_phase_t *phase,
                                          int *done, int *total)
{
    if (phase)
        *phase = s_phase;
    if (done)
        *done = s_phase_done;
    if (total)
        *total = s_phase_total;
    return s_thread_running && s_phase != MOONLIT_MASTER_ART_PHASE_IDLE;
}

bool moonlit_master_art_builder_pass_done(void)
{
    return s_pass_done;
}

bool moonlit_master_art_builder_is_running(void)
{
    return s_thread_running;
}

void moonlit_master_art_builder_set_foreground(bool foreground)
{
    s_foreground = foreground;
}

void moonlit_master_art_builder_begin_full_pass(void)
{
    s_pass_done = false;
    /* El hilo lo crea poll() la primera vez que la base esta lista. Si el
     * usuario pidio la preparacion sin haber entrado nunca a Musica, el
     * hilo puede no existir todavia -- sin esto, s_kick se quedaria
     * puesto y la pantalla saldria por is_running() sin preparar nada. */
    moonlit_master_art_builder_poll();
    s_kick = true;
}
