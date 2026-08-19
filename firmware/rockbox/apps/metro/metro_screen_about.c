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

#include "metro_screen_about.h"
#include "metro_device.h"
#include "metro_manifest.h"
#include "metro_lang.h"

/* Row layout: device name, then either 3 count rows (music/video/photo,
 * if sync_summary.cfg exists) or 1 "not synced yet" row, then "based
 * on rockbox". */

static int about_count(void *ctx)
{
    metro_manifest_t m;
    (void)ctx;
    return 2 + (metro_manifest_load(&m) ? 3 : 1);
}

static void about_get_row(void *ctx, int index, struct metro_row *out)
{
    static char buf[64];
    metro_manifest_t m;
    bool synced;
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

    synced = metro_manifest_load(&m);

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
        if (index == 1)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.music_count, metro_lang_str(LANG_ABOUT_SONGS));
            out->title = buf;
            return;
        }
        if (index == 2)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.video_count, metro_lang_str(LANG_HUB_VIDEOS));
            out->title = buf;
            return;
        }
        if (index == 3)
        {
            snprintf(buf, sizeof(buf), "%d %s", m.photo_count, metro_lang_str(LANG_HUB_PHOTOS));
            out->title = buf;
            return;
        }
    }

    out->title = metro_lang_str(LANG_ABOUT_BASED_ON_ROCKBOX);
}

static void about_on_select(void *ctx, int index)
{
    (void)ctx;
    (void)index;
}

const struct metro_pivot metro_screen_about_pivot = {
    LANG_PIVOT_ABOUT, about_count, about_get_row, about_on_select, NULL
};
