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
/* Album art for the currently playing track only (PLAN_MAESTRO.md S1.4,
 * S1.2 "cache de 1"). Deliberately narrower than Aura-Firmware's
 * aura_albumart.c: no disk-cached .pfraw, no precache pass, no
 * per-album lookups from tagcache seeks -- Metro only ever needs the
 * ONE track audio_current_track() is pointing at, so there is nothing
 * to precompute ahead of time and no cache-invalidation problem beyond
 * "did the path change".
 */
#ifndef METRO_ALBUMART_H
#define METRO_ALBUMART_H

#include <stdbool.h>
#include "lcd.h"

#define METRO_ALBUMART_SIZE 136

/* Loads (or reuses the cached decode of) the art for
 * audio_current_track() -- folder art first (find_albumart(), cover.jpg
 * next to the track or in its parent dir), embedded JPEG (ID3 APIC)
 * otherwise. False if nothing is playing or the track has no art at
 * all -- draw metro_draw_tile() instead. */
bool metro_albumart_load_current(void);

/* Valid only right after metro_albumart_load_current() returned true --
 * METRO_ALBUMART_SIZE x METRO_ALBUMART_SIZE, row-major, native LCD
 * format (ready for lcd_bitmap()). */
const fb_data *metro_albumart_bitmap(void);

/* F12: same track as metro_albumart_load_current(), scaled to fill
 * the whole screen (LCD_WIDTH x LCD_HEIGHT) instead of the small NP
 * tile -- for the dimmed background behind Now Playing
 * (PLAN_MAESTRO.md S3.3, graphics=full only -- the caller decides
 * whether to call this at all, this module doesn't read
 * metro_settings itself). Cached independently of
 * metro_albumart_load_current()'s own tile-sized cache -- calling one
 * has no effect on the other. */
bool metro_albumart_load_background(void);

/* Valid only right after metro_albumart_load_background() returned
 * true -- LCD_WIDTH x LCD_HEIGHT, row-major, native LCD format. */
const fb_data *metro_albumart_background_bitmap(void);

#endif /* METRO_ALBUMART_H */
