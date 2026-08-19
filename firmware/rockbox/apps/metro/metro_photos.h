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
/* /Photos browsing (PLAN_MAESTRO.md S1.2, Aura-Firmware's contract
 * S D.1): flat JPEG list (D-192-class contract: /Photos never gets
 * subfolders) plus the OPTIONAL category index
 * (metro_media_categories.h) -- viewing is entirely delegated to the
 * fork's own imageviewer plugin, no image decoder of Metro's own. */
#ifndef METRO_PHOTOS_H
#define METRO_PHOTOS_H

#include "metro_fsutil.h"
#include "metro_media_categories.h"

/* Contract cap: the firmware lists up to 500 photos. */
#define METRO_PHOTOS_MAX 500

typedef struct {
    char filename[METRO_FSUTIL_NAME_LEN];
    metro_photo_cat_t category;
} metro_photo_item_t;

/* Re-scans /Photos fresh (natural order, .jpg/.jpeg) and reloads the
 * category index alongside it -- same "refresh on enter" pattern as
 * metro_video_list() / metro_music.c's own lists (DESVIACIONES.md
 * F6-1). Call once when the photos page is entered. */
int metro_photos_list(metro_photo_item_t *out, int max);

/* Launches /Photos/<filename> in the fork's imageviewer plugin, open
 * errors silenced (same reasoning as metro_video_play()). */
void metro_photos_view(const char *filename);

#endif /* METRO_PHOTOS_H */
