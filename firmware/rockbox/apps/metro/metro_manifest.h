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
/* Reads /.rockbox/aura/sync_summary.cfg -- Aura Studio writes this on
 * every sync so About can show real counts (contract: this file is
 * write-only from Studio's side, the firmware never writes it).
 * Narrower than Aura-Firmware's aura_manifest.c: no per-category
 * breakdown (video_movies_count/etc, Aura's "Estado 2") or byte
 * totals -- Metro's About screen (F8) only needs the three top-level
 * counts. */
#ifndef METRO_MANIFEST_H
#define METRO_MANIFEST_H

#include <stdbool.h>

typedef struct {
    int music_count;
    int video_count;
    int photo_count;
} metro_manifest_t;

/* False if sync_summary.cfg doesn't exist yet (device never synced
 * from Studio) -- *out is zeroed either way, never left uninitialized. */
bool metro_manifest_load(metro_manifest_t *out);

#endif /* METRO_MANIFEST_H */
