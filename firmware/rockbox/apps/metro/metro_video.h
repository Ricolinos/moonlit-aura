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
/* /Videos browsing (PLAN_MAESTRO.md S1.2): the device only ever plays
 * MPEG-1/2, so this is a plain flat file list (D-192-class contract:
 * /Videos never gets subfolders) plus the OPTIONAL category index
 * (metro_media_categories.h) -- playback is entirely delegated to the
 * fork's own mpegplayer plugin, no video decoder of Metro's own. */
#ifndef METRO_VIDEO_H
#define METRO_VIDEO_H

#include "metro_fsutil.h"
#include "metro_media_categories.h"

/* Contract cap: the firmware lists up to 100 videos. */
#define METRO_VIDEO_MAX 100

typedef struct {
    char filename[METRO_FSUTIL_NAME_LEN];
    metro_video_cat_t category;
} metro_video_item_t;

/* Re-scans /Videos fresh (natural order, .mpg/.mpeg) and reloads the
 * category index alongside it -- same "refresh on enter" pattern
 * metro_music.c's own lists already use (no persistent cache to
 * invalidate, DESVIACIONES.md F6-1). Call once when the videos page is
 * entered. */
int metro_video_list(metro_video_item_t *out, int max);

/* Launches /Videos/<filename> in the fork's mpegplayer plugin, with
 * open errors silenced (a file this list just found should never fail
 * to open, but the plugin's own error UI is Rockbox chrome Metro never
 * shows). */
void metro_video_play(const char *filename);

#endif /* METRO_VIDEO_H */
