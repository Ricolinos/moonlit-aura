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
/* R2-F2/DD-9 (M-057): Metro's own photo thumbnail cache -- 80x80
 * fb_data bitmaps ("cubrir" cropped, see metro_photo_thumbs.c) for the
 * Photos grid's tiles (metro_draw_tiles()'s get_tile() callback wires
 * straight to metro_photo_thumbs_get()). Two layers, neither ever
 * blocks the UI thread for a visible amount of time:
 *
 *   1. An in-RAM window of the METRO_PHOTO_THUMB_WINDOW most recently
 *      requested thumbnails (decoded pixels, ready to blit).
 *   2. An on-disk cache under metro_settings_metro_cache_dir()
 *      (.../aura/metrocache/photos/, NOT Aura's photocache/ -- format
 *      and size differ, see DECISIONS.md M-057) -- one raw file per
 *      thumbnail, filename keyed by source name + mtime so a re-synced
 *      file with new content never serves a stale thumbnail.
 *
 * Actually decoding a JPEG is the one part of this that's slow enough
 * to matter -- metro_photo_thumbs_tick() budgets that to at most one
 * decode per call, meant to be called once per metro_main() idle-loop
 * iteration (same poll as F10's index-letter redraw), so scrolling
 * into a grid full of uncached photos never freezes: the grid draws
 * accent-tile placeholders for whatever isn't ready yet and fills in
 * for real, one tile at a time, over the next several idle ticks. */
#ifndef METRO_PHOTO_THUMBS_H
#define METRO_PHOTO_THUMBS_H

#include <stdbool.h>
#include "lcd.h"

/* Returns the decoded 80x80 thumbnail (METRO_TILE_SIZE x METRO_TILE_SIZE,
 * see metro_draw.h) for /Photos/<filename> if it's ready -- either
 * already in the RAM window, or found on disk and loaded into it (a
 * raw file read, cheap enough to not need the decode budget). `mtime`
 * is the source file's current mtime (metro_photo_item_t.mtime) --
 * used to validate a disk-cache hit and to key a fresh one.
 *
 * Returns NULL if no valid thumbnail exists yet -- this also queues
 * `filename` for metro_photo_thumbs_tick() to decode (deduplicated;
 * a filename already pending isn't queued twice). Caller (the pivot's
 * get_tile()) falls back to metro_draw_tile()'s accent-tile-with-initial
 * placeholder in that case. */
const fb_data *metro_photo_thumbs_get(const char *filename, long mtime);

/* Decodes and caches (RAM + disk) at most one pending thumbnail from
 * the queue metro_photo_thumbs_get() built up. Returns true if it
 * actually did the work for one (caller should force a redraw so the
 * freshly-ready tile replaces its placeholder); false if the queue was
 * empty (idle -- nothing to do this tick). */
bool metro_photo_thumbs_tick(void);

/* Clears the RAM window and the pending queue -- call when leaving the
 * Photos grid (metro_screen_hub.c) so a later visit to a different
 * pivot/category doesn't keep serving thumbnails from, or spending its
 * decode budget on, a subset the user isn't looking at anymore. Never
 * touches the on-disk cache -- that stays valid across visits. */
void metro_photo_thumbs_reset(void);

#endif /* METRO_PHOTO_THUMBS_H */
