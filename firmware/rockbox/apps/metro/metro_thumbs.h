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
/* R3-F1/DD-1 (M-062): generalized from R2-F2's photo-only
 * metro_photo_thumbs.c -- same two-layer design (RAM window + on-disk
 * cache under metro_settings_metro_cache_dir()), now parametrized by a
 * metro_thumb_source so R3-F3 (artist photos) and R3-F4 (quickplay
 * album art) each plug in their own "what's the key"/"how do I decode
 * this" without a second/third copy of the window+cache bookkeeping.
 * Only one grid is ever on screen at a time, so there's exactly ONE
 * shared RAM window/pending queue for all sources -- switching source
 * (leaving Photos for Artists, say) is a metro_thumbs_reset(), same as
 * before. See DECISIONS.md M-062 for why this shape (index-based
 * pending entries, not filename-based) instead of a thinner rename. */
#ifndef METRO_THUMBS_H
#define METRO_THUMBS_H

#include <stdbool.h>
#include <stddef.h>
#include "lcd.h"

struct metro_thumb_source {
    /* Subdirectory of metro_settings_metro_cache_dir(), e.g. "photos"/
     * "artists"/"albums" -- keeps each source's .mth files apart on
     * disk (and apart from Aura's own unrelated photocache/). */
    const char *cache_subdir;

    /* Writes this item's cache-key stem (no ".mth") into out -- by
     * convention "<stable-name>.<mtime-or-equivalent>", same scheme
     * R2-F2 used for photos, so a re-synced/changed source item
     * invalidates cleanly: metro_thumbs_tick() drops any other cached
     * file that shares the part before the *last* '.' once it writes
     * a fresh one. Returns false if this index has nothing cacheable
     * (item vanished, index out of range) -- caller gets NULL/no-op
     * instead of a garbage key. */
    bool (*cache_key)(void *ctx, int index, char *out, size_t out_len);

    /* Produces this item's METRO_TILE_SIZE x METRO_TILE_SIZE pixels
     * into dst -- since moonlit D-059 always via the shared master
     * cache (metro_thumbs_decode_via_master() below): read the master
     * and downscale, or decode the JPEG once, write the master, then
     * downscale. Returns METRO_THUMB_OK on success, METRO_THUMB_FAIL
     * on a definitive failure (missing file, decode error, shared
     * .none marker -- the caller keeps the accent-tile placeholder),
     * METRO_THUMB_WAITING when the master doesn't exist yet and the
     * background builder is mid-pass (placeholder now; metro_main.c
     * repaints when the builder's generation moves and the grid asks
     * again). */
    int (*decode)(void *ctx, int index, fb_data *dst);
};

#define METRO_THUMB_FAIL    0
#define METRO_THUMB_OK      1
#define METRO_THUMB_WAITING (-1)

/* Returns the decoded METRO_TILE_SIZE x METRO_TILE_SIZE thumbnail for
 * (source, ctx, index) if it's ready -- from the RAM window, or found
 * on disk and loaded into it (a raw read, cheap enough to not need the
 * decode budget). Returns NULL if not ready yet, which also queues the
 * item for metro_thumbs_tick() (deduplicated by cache key). Caller (a
 * pivot's get_tile()) falls back to metro_draw_tile()'s
 * accent-tile-with-initial placeholder in that case. */
const fb_data *metro_thumbs_get(const struct metro_thumb_source *source,
                                 void *ctx, int index);

/* Decodes and caches (RAM + disk) at most one pending thumbnail from
 * the queue metro_thumbs_get() built up -- regardless of which
 * source(s) queued it. Returns true if it did the work for one (caller
 * should force a redraw so the freshly-ready tile replaces its
 * placeholder); false if the queue was empty (idle -- nothing to do
 * this tick). Meant to be called once per metro_main() idle-loop
 * iteration, same poll as before. */
bool metro_thumbs_tick(void);

/* Clears the RAM window and the pending queue -- call when leaving a
 * grid, or switching source within the same screen, so a later visit
 * doesn't keep serving or decoding for a subset the user isn't looking
 * at anymore. Never touches the on-disk cache -- that stays valid
 * across visits. */
void metro_thumbs_reset(void);

/* moonlit (D-059): the shared "via master" pipeline every source's
 * decode() now uses. `master_path` is the /.aura/art/<subdir>/<key>.art
 * file for this item (moonlit_art_master_path()/
 * moonlit_art_master_file_path()), `master_size` its side (130 albums/
 * artists, 80 photos). Order: master exists -> box-downscale to the
 * 80 px tile (one plain read, no JPEG); shared .none ->
 * METRO_THUMB_FAIL; builder mid-pass -> METRO_THUMB_WAITING (never a
 * UI-thread JPEG decode while it works); builder idle -> `raw_decode`
 * (a metro_albumart *_ui decode into its own scratch; returns the
 * decoded pixels and their real w x h, or NULL) runs here, the master
 * is WRITTEN (always -- contract v16) and the tile derived from it.
 * Owns the only 130 px master scratch this module needs. */
typedef const fb_data *(*metro_thumbs_raw_decode_fn)(void *ctx, int *w, int *h);
int metro_thumbs_decode_via_master(const char *master_path, int master_size,
                                   metro_thumbs_raw_decode_fn raw_decode, void *ctx,
                                   fb_data *out);

/* moonlit (D-059): true (and self-clears) if some decode() returned
 * METRO_THUMB_WAITING since the last call -- metro_main.c pairs it
 * with the builder's generation to repaint the grid when the awaited
 * masters start landing. */
bool metro_thumbs_take_waiting(void);

#endif /* METRO_THUMBS_H */
