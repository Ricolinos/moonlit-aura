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
#include "string-extra.h"

#include "metro_thumbs.h"
#include "metro_settings.h"
#include "metro_draw.h"
#include "metro_fsutil.h"
#include "moonlit_master_art.h"         /* moonlit (D-059) */
#include "moonlit_master_art_builder.h" /* moonlit (D-059) */

#define THUMB_PX (METRO_TILE_SIZE * METRO_TILE_SIZE)

/* R2-F2/DD-9: 16 thumbnails (~2 grid screens) resident in RAM at once
 * -- 16 * 80*80*sizeof(fb_data) = 16 * 12800 = 204800 bytes. Plain ring
 * eviction (not true LRU): simple, and with a window this much bigger
 * than one screen (8 tiles) it takes real back-and-forth scrolling to
 * evict something still on screen. R3-F1/DD-1: this window is now
 * SHARED across every source (photos/artists/albums) -- only one grid
 * is ever on screen, so there's no reason to pay for three windows. */
#define WINDOW_N 16
#define PENDING_MAX 16

/* Cache-key stems are filename-shaped ("<name>.<mtime>") -- reuse the
 * same length budget metro_fsutil.c already uses for source names. */
#define KEY_LEN METRO_FSUTIL_NAME_LEN

struct thumb_slot {
    char key[KEY_LEN];
    bool valid;
    fb_data pixels[THUMB_PX];
};

static struct thumb_slot s_window[WINDOW_N];
static int s_window_ring = 0;

/* R3-F1/DD-1: a pending entry remembers WHICH source queued it (not
 * just a filename+mtime, R2-F2's original shape) -- metro_thumbs_tick()
 * needs (source, ctx, index) to call back into that source's own
 * decode(). The cache key is captured once, at metro_thumbs_get() time,
 * and carried along so tick() never has to recompute it (and so a
 * dedup check against the pending queue is a plain string compare). */
struct pending_entry {
    const struct metro_thumb_source *source;
    void *ctx;
    int index;
    char key[KEY_LEN];
};

static struct pending_entry s_pending[PENDING_MAX];
static int s_pending_n = 0;

/* moonlit (D-059): the JPEG scratch that lived here (SCRATCH_MAX_SRC_PX,
 * docs/DESVIACIONES.md R3-3) moved to metro_albumart.c's shared raw
 * decodes -- this module only ever touches masters now. One master is
 * at most 130 x 130 fb_data (33 800 B). */
static fb_data s_master[MOONLIT_MASTER_ART_MAX_SIZE * MOONLIT_MASTER_ART_MAX_SIZE];
static bool s_waiting;

/* moonlit (D-072): presupuesto de lecturas de maestra por cuadro, mismo
 * criterio que la lectura acotada de Marea (D-057) -- el dibujo de la
 * rejilla no puede convertirse en ocho lecturas de disco seguidas. */
#define THUMB_MASTER_READS_PER_FRAME 4
static int s_master_reads;

void metro_thumbs_begin_frame(void)
{
    s_master_reads = 0;
}

static struct thumb_slot *find_slot(const char *key)
{
    int i;
    for (i = 0; i < WINDOW_N; i++)
        if (s_window[i].valid && !strcmp(s_window[i].key, key))
            return &s_window[i];
    return NULL;
}

static void cache_dir_for(const struct metro_thumb_source *source, char *out, size_t outsz)
{
    /* moonlit (D-055): shared with Metro under /.aura/thumbs/<subdir>/. */
    metro_settings_shared_thumbs_dir(source->cache_subdir, out, outsz);
}

static void cache_path(const struct metro_thumb_source *source, const char *key,
                        char *out, size_t outsz)
{
    char dir[MAX_PATH];
    cache_dir_for(source, dir, sizeof(dir));
    snprintf(out, outsz, "%s/%s.mth", dir, key);
}

static void ensure_cache_dir(const struct metro_thumb_source *source)
{
    char dir[MAX_PATH], parent[MAX_PATH];
    char *slash;

    cache_dir_for(source, dir, sizeof(dir));

    /* moonlit (D-055): /.aura/thumbs/<subdir> -- every ancestor may be
     * missing on a fresh volume, so walk the path creating each one. */
    strlcpy(parent, dir, sizeof(parent));
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

/* R2-F2/DD-9, generalized R3-F1: drops any other cached .mth that
 * shares this key's stem (the part before the *last* '.', i.e. the
 * "<stable-name>" half of "<stable-name>.<mtime>") -- the mtime-in-
 * the-key scheme means a changed source item picks a new cache
 * filename, leaving the old one an orphan pointing at content nobody
 * will ever ask for by that exact key again. Cheap: one directory
 * scan, only run on an actual decode (already the slow path). Generic
 * over all three sources because all three follow the same
 * "<name>.<mtime>" key convention (metro_thumb_source.cache_key's own
 * contract). */
static void remove_stale(const struct metro_thumb_source *source, const char *key,
                          const char *keep_path)
{
    char dir[MAX_PATH];
    char prefix[KEY_LEN];
    char *dot;
    size_t prefix_len;
    DIR *d;
    struct DIRENT *entry;

    strlcpy(prefix, key, sizeof(prefix));
    dot = strrchr(prefix, '.');
    if (dot)
        *dot = '\0';
    prefix_len = strlen(prefix);

    cache_dir_for(source, dir, sizeof(dir));
    d = opendir(dir);
    if (!d)
        return;

    while ((entry = readdir(d)) != NULL)
    {
        char full[MAX_PATH];

        if (strncmp(entry->d_name, prefix, prefix_len) != 0 ||
            entry->d_name[prefix_len] != '.')
            continue; /* not "<prefix>.<...>.mth" for THIS key's stem */

        snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
        if (strcmp(full, keep_path) != 0)
            remove(full);
    }
    closedir(d);
}

/* moonlit (D-059): see metro_thumbs.h. The nearest-neighbour
 * cover_crop() that lived here (R2-F2/M-057) is superseded by
 * moonlit_master_art_resample_cover()'s integer box filter -- one
 * resampler for the master and for every derivation. */
int metro_thumbs_decode_via_master(const char *master_path, int master_size,
                                   metro_thumbs_raw_decode_fn raw_decode, void *ctx,
                                   fb_data *out)
{
    char none[MAX_PATH];
    char dir[MAX_PATH];
    const fb_data *px;
    int w, h;

    if (moonlit_master_art_read(master_path, master_size, s_master))
    {
        moonlit_master_art_box_downscale(s_master, master_size, out, METRO_TILE_SIZE);
        return METRO_THUMB_OK;
    }

    if (!moonlit_master_art_none_path(master_path, none, sizeof(none)))
        return METRO_THUMB_FAIL;
    if (moonlit_master_art_none_exists(none))
        return METRO_THUMB_FAIL;

    if (moonlit_master_art_builder_active())
    {
        s_waiting = true;
        return METRO_THUMB_WAITING;
    }

    px = raw_decode(ctx, &w, &h);

    strlcpy(dir, master_path, sizeof(dir));
    if (strrchr(dir, '/'))
        *strrchr(dir, '/') = '\0';
    moonlit_master_art_ensure_dir(dir);

    if (!px)
    {
        /* Contract v16 (shared .none, D-056): a definitive "no art"
         * is recorded once, for the three families. */
        moonlit_master_art_write_none(none);
        return METRO_THUMB_FAIL;
    }

    moonlit_master_art_resample_cover(px, w, h, s_master, master_size);
    moonlit_master_art_write(master_path, master_size, s_master);
    moonlit_master_art_box_downscale(s_master, master_size, out, METRO_TILE_SIZE);
    return METRO_THUMB_OK;
}

bool metro_thumbs_take_waiting(void)
{
    bool was = s_waiting;

    s_waiting = false;
    return was;
}

const fb_data *metro_thumbs_get(const struct metro_thumb_source *source,
                                 void *ctx, int index)
{
    struct thumb_slot *s;
    char key[KEY_LEN];
    char path[MAX_PATH];
    int fd;
    ssize_t got;
    int i;

    if (!source->cache_key(ctx, index, key, sizeof(key)))
        return NULL;

    s = find_slot(key);
    if (s)
        return s->pixels;

    /* moonlit (D-072): fuente cuya maestra YA es del tamano del tile
     * (Fotos). Se lee la maestra directo, sin `.mth` de por medio y sin
     * pasar por la cola -- pero con presupuesto: como mucho
     * THUMB_MASTER_READS_PER_FRAME lecturas por cuadro, para que una
     * rejilla llena no meta ocho lecturas de disco en el mismo cuadro.
     * Lo que no alcance en este cuadro entra en el siguiente. */
    if (source->master_path && s_master_reads < THUMB_MASTER_READS_PER_FRAME)
    {
        char mpath[MAX_PATH];

        if (source->master_path(ctx, index, mpath, sizeof(mpath)))
        {
            s_master_reads++;
            if (moonlit_master_art_read(mpath, METRO_TILE_SIZE,
                                         s_window[s_window_ring].pixels))
            {
                s = &s_window[s_window_ring];
                strlcpy(s->key, key, sizeof(s->key));
                s->valid = true;
                s_window_ring = (s_window_ring + 1) % WINDOW_N;
                return s->pixels;
            }
        }
        /* Sin maestra todavia: el constructor de fondo la escribira
         * (D-059). Cae a la cola de siempre. */
    }

    /* Agotado el presupuesto del cuadro, NO se sale con las manos
     * vacias: el item cae a la cola de siempre y metro_thumbs_tick() lo
     * resuelve en la vuelta ociosa, que es quien fuerza el repintado.
     * Salir aqui dejaba la segunda fila de la rejilla en monograma
     * hasta que el usuario la moviera -- se vio en la primera captura
     * de la rejilla de fotos, con las cuatro de arriba cargadas y las
     * cuatro de abajo no. */

    /* Disk cache is a raw read (no decode) -- cheap enough to try
     * synchronously, unlike an actual decode. */
    cache_path(source, key, path, sizeof(path));
    fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        got = read(fd, s_window[s_window_ring].pixels, THUMB_PX * sizeof(fb_data));
        close(fd);
        if (got == (ssize_t)(THUMB_PX * sizeof(fb_data)))
        {
            s = &s_window[s_window_ring];
            strlcpy(s->key, key, sizeof(s->key));
            s->valid = true;
            s_window_ring = (s_window_ring + 1) % WINDOW_N;
            return s->pixels;
        }
    }

    for (i = 0; i < s_pending_n; i++)
        if (!strcmp(s_pending[i].key, key))
            return NULL; /* already queued */

    if (s_pending_n < PENDING_MAX)
    {
        s_pending[s_pending_n].source = source;
        s_pending[s_pending_n].ctx = ctx;
        s_pending[s_pending_n].index = index;
        strlcpy(s_pending[s_pending_n].key, key, sizeof(s_pending[0].key));
        s_pending_n++;
    }
    return NULL;
}

bool metro_thumbs_tick(void)
{
    struct pending_entry entry;
    char path[MAX_PATH];
    struct thumb_slot *s;
    int fd;
    int i;

    if (s_pending_n == 0)
        return false;

    entry = s_pending[0];
    for (i = 1; i < s_pending_n; i++)
        s_pending[i - 1] = s_pending[i];
    s_pending_n--;

    s = &s_window[s_window_ring];
    if (entry.source->decode(entry.ctx, entry.index, s->pixels) != METRO_THUMB_OK)
        return true; /* budget spent either way (a WAITING item is
                      * re-queued by the next metro_thumbs_get() once
                      * the builder's generation moves, metro_main.c) */

    strlcpy(s->key, entry.key, sizeof(s->key));
    s->valid = true;
    s_window_ring = (s_window_ring + 1) % WINDOW_N;

    /* moonlit (D-072): una fuente que lee la maestra directo no escribe
     * `.mth` -- seria una copia byte a byte de un archivo que ya esta en
     * disco. `/.aura/thumbs/photos` deja de crecer. */
    if (entry.source->master_path)
        return true;

    ensure_cache_dir(entry.source);
    cache_path(entry.source, entry.key, path, sizeof(path));
    remove_stale(entry.source, entry.key, path);
    fd = creat(path, 0666);
    if (fd >= 0)
    {
        write(fd, s->pixels, THUMB_PX * sizeof(fb_data));
        close(fd);
    }

    return true;
}

void metro_thumbs_reset(void)
{
    int i;
    for (i = 0; i < WINDOW_N; i++)
        s_window[i].valid = false;
    s_pending_n = 0;
    s_waiting = false;
}
