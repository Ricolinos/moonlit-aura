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
 *
 * R2-F1/DD-5 (M-055): full 13-key parse, same field set as
 * Aura-Firmware's aura_manifest_t (consulted read-only) -- Studio
 * writes the identical sync_summary.cfg format for either firmware
 * family, so there is no reason for Metro to understand fewer of its
 * keys than Aura does. */
#ifndef METRO_MANIFEST_H
#define METRO_MANIFEST_H

#include <stdbool.h>

typedef struct {
    int music_count;
    long long music_bytes;
    int video_count;
    long long video_bytes;
    int photo_count;
    long long photo_bytes;
    int playlist_count;
    /* Per-category breakdown within video/photo -- Studio classifies
     * each item on import (Rockbox has no video database or EXIF
     * parser of its own). has_video_categories/has_photo_categories
     * are true only if the corresponding keys were actually present
     * in the file (a manifest written before Studio added category
     * breakdown never had them) -- lets the About screen show "sync
     * to see the detail" instead of a misleading 0. */
    int video_movies_count;
    int video_series_count;
    int video_clips_count;
    bool has_video_categories;
    int photo_images_count;
    int photo_photos_count;
    int photo_ai_count;
    bool has_photo_categories;
} metro_manifest_t;

/* False if sync_summary.cfg doesn't exist yet (device never synced
 * from Studio) -- *out is zeroed either way, never left uninitialized. */
bool metro_manifest_load(metro_manifest_t *out);

#endif /* METRO_MANIFEST_H */
