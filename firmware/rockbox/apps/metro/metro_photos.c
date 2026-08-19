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

#include "metro_photos.h"

#define PHOTOS_DIR "/Photos"

static const char *const k_exts[] = { ".jpg", ".jpeg" };

int metro_photos_list(metro_photo_item_t *out, int max)
{
    static char names[METRO_PHOTOS_MAX][METRO_FSUTIL_NAME_LEN];
    int n, i;

    if (max > METRO_PHOTOS_MAX)
        max = METRO_PHOTOS_MAX;

    metro_media_categories_load_photo();
    n = metro_fsutil_list_by_ext(PHOTOS_DIR, k_exts, 2, names, max);

    for (i = 0; i < n; i++)
    {
        strlcpy(out[i].filename, names[i], sizeof(out[i].filename));
        out[i].category = metro_media_categories_photo_lookup(names[i]);
    }
    return n;
}

void metro_photos_view(const char *filename)
{
    char path[MAX_PATH];

    snprintf(path, sizeof(path), "%s/%s", PHOTOS_DIR, filename);
    plugin_set_silent_open_errors(true);
    plugin_load(VIEWERS_DIR "/imageviewer.rock", path);
}
