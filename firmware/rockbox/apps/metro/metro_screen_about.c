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
#include <stddef.h>
#include <string.h>

#include "metro_screen_about.h"
#include "metro_device.h"
#include "metro_manifest.h"
#include "metro_lang.h"

/* moonlit H1 (D-002): credits block after "based on rockbox" -- one
 * About row per '\n'-separated line of LANG_ABOUT_CREDITS_BODY (the
 * AURA_STR_ABOUT_CREDITS_BODY pattern, cut into rows because Metro's
 * About is a pivot list, not a text body). GPL v2 section 3: source
 * URL; font/icon licenses; Apple non-affiliation. */
static int credits_line_count(void)
{
    const char *s = metro_lang_str(LANG_ABOUT_CREDITS_BODY);
    int n = 1;
    for (; *s; s++)
        if (*s == '\n')
            n++;
    return n;
}

/* Copies line `line` (0-based) of the credits body into buf. */
static const char *credits_line(int line, char *buf, size_t bufsz)
{
    const char *s = metro_lang_str(LANG_ABOUT_CREDITS_BODY);
    const char *end;
    size_t len;

    while (line > 0 && *s)
    {
        if (*s == '\n')
            line--;
        s++;
    }
    end = strchr(s, '\n');
    len = end ? (size_t)(end - s) : strlen(s);
    if (len >= bufsz)
        len = bufsz - 1;
    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

/* Row layout: device name, then either "based on rockbox" straight
 * after 1 "not synced yet" row, or (if sync_summary.cfg exists, R2-F1/
 * DD-5, M-055) music/video/photo/playlist counts followed by whichever
 * category breakdown rows the manifest actually carries (0, 3, or 6
 * extra rows depending on has_video_categories/has_photo_categories --
 * a manifest written before Studio added category breakdown has
 * neither), then "based on rockbox", then the credits rows (H1). */

static int synced_row_count(const metro_manifest_t *m)
{
    int n = 4; /* music, video, photo, playlists */
    if (m->has_video_categories)
        n += 3; /* movies, series, clips */
    if (m->has_photo_categories)
        n += 3; /* images, photos, ai */
    return n;
}

/* R5-F1 (M-081): both providers run per FRAME (the pivot slide redraws
 * every row each tick), so they read the RAM copy that
 * metro_disk_handoff() refreshes -- never the disk. The previous
 * metro_manifest_load() here (open + parse + close of sync_summary.cfg,
 * once per row per frame) is what wedged the real iPod on entering
 * About while the simulator, backed by the host filesystem, showed
 * nothing wrong. */
static int about_count(void *ctx)
{
    const metro_manifest_t *m = metro_manifest_cached();
    (void)ctx;
    return 2 + (m ? synced_row_count(m) : 1) + credits_line_count();
}

static void about_get_row(void *ctx, int index, struct metro_row *out)
{
    static char buf[64];
    const metro_manifest_t *mp = metro_manifest_cached();
    metro_manifest_t m;
    bool synced = (mp != NULL);
    const char *name;

    (void)ctx;
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;

    if (index == 0)
    {
        name = metro_device_name();
        out->title = name ? name : metro_lang_str(LANG_ABOUT_DEVICE_DEFAULT);
        return;
    }

    if (synced)
        m = *mp;

    if (!synced)
    {
        if (index == 1)
        {
            out->title = metro_lang_str(LANG_ABOUT_NOT_SYNCED);
            return;
        }
    }
    else
    {
        /* R2-F1/DD-5 (M-055): row order mirrors Aura-Firmware's own
         * About screen (consulted read-only, aura_screens.c) -- top-level
         * counts, then playlists, then video breakdown, then photo
         * breakdown, each breakdown only if the manifest actually
         * carries it. */
        int row = 1;

        if (index == row++)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.music_count, metro_lang_str(LANG_ABOUT_SONGS));
            out->title = buf;
            return;
        }
        if (index == row++)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.video_count, metro_lang_str(LANG_HUB_VIDEOS));
            out->title = buf;
            return;
        }
        if (index == row++)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.photo_count, metro_lang_str(LANG_HUB_PHOTOS));
            out->title = buf;
            return;
        }
        if (index == row++)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.playlist_count, metro_lang_str(LANG_ABOUT_PLAYLISTS));
            out->title = buf;
            return;
        }
        if (m.has_video_categories)
        {
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.video_movies_count, metro_lang_str(LANG_ABOUT_MOVIES));
                out->title = buf;
                return;
            }
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.video_series_count, metro_lang_str(LANG_ABOUT_SERIES));
                out->title = buf;
                return;
            }
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.video_clips_count, metro_lang_str(LANG_ABOUT_CLIPS));
                out->title = buf;
                return;
            }
        }
        if (m.has_photo_categories)
        {
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.photo_images_count, metro_lang_str(LANG_ABOUT_IMAGES));
                out->title = buf;
                return;
            }
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.photo_photos_count, metro_lang_str(LANG_ABOUT_PHOTOS_TAKEN));
                out->title = buf;
                return;
            }
            if (index == row++)
            {
                snprintf(buf, sizeof(buf), "%d %s", m.photo_ai_count, metro_lang_str(LANG_ABOUT_AI));
                out->title = buf;
                return;
            }
        }
    }

    {
        int base = 1 + (synced ? synced_row_count(&m) : 1);
        if (index == base)
        {
            out->title = metro_lang_str(LANG_ABOUT_BASED_ON_ROCKBOX);
            return;
        }
        out->title = credits_line(index - base - 1, buf, sizeof(buf));
    }
}

static void about_on_select(void *ctx, int index)
{
    (void)ctx;
    (void)index;
}

const struct metro_pivot metro_screen_about_pivot = {
    LANG_PIVOT_ABOUT, about_count, about_get_row, about_on_select, NULL
};
