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

#include "rbpaths.h"
#include "file.h"
#include "plugin.h"
#include "string-extra.h"

#include "metro_video.h"
#include "metro_transitions.h"

/* NOT metro_screen_list.h: that header pulls in metro_page.h ->
 * metro_lang.h, whose bare LANG_* enumerators collide with the
 * apps/plugin.h -> lang_enum.h Rockbox already needs in this TU
 * (Metro's own lang table is deliberately separate from Rockbox's --
 * project CLAUDE.md -- so the two must never be visible together). A
 * one-line forward declaration of the single function this file
 * actually calls avoids the whole header. */
void metro_screen_list_show(void);

#define VIDEOS_DIR "/Videos"

static const char *const k_exts[] = { ".mpg", ".mpeg" };

int metro_video_list(metro_video_item_t *out, int max)
{
    static char names[METRO_VIDEO_MAX][METRO_FSUTIL_NAME_LEN];
    int n, i;

    if (max > METRO_VIDEO_MAX)
        max = METRO_VIDEO_MAX;

    metro_media_categories_load_video();
    n = metro_fsutil_list_by_ext(VIDEOS_DIR, k_exts, 2, names, max);

    for (i = 0; i < n; i++)
    {
        strlcpy(out[i].filename, names[i], sizeof(out[i].filename));
        out[i].category = metro_media_categories_video_lookup(names[i]);
    }
    return n;
}

void metro_video_play(const char *filename)
{
    char path[MAX_PATH];

    snprintf(path, sizeof(path), "%s/%s", VIDEOS_DIR, filename);
    plugin_set_silent_open_errors(true);
    plugin_load(VIEWERS_DIR "/mpegplayer.rock", path);

    /* F11: FADE back into the videos list (PLAN_MAESTRO.md S3.3, "a/desde
     * plugins") -- the nav stack itself never changed while the plugin
     * ran, so metro_main.c's push/pop/twist diff has nothing to see
     * here on its own; this is the one explicit transition call
     * outside that diff. */
    metro_transitions_fade(metro_screen_list_show);
}
