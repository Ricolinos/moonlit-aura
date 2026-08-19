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
#include <string.h>

#include "file.h"
#include "rbpaths.h"
#include "misc.h"

#include "metro_manifest.h"

#define METRO_DIR           ROCKBOX_DIR "/aura"
#define MANIFEST_PATH        METRO_DIR "/sync_summary.cfg"

/* Same reasoning as Aura-Firmware's aura_manifest.c (consulted
 * read-only): atoll()/strtoll() aren't available on every target in
 * this tree, and every value here is a non-negative decimal integer
 * written by CatalogSummaryWriter (Aura Studio), so a small parser of
 * its own is enough without pulling in extended libc. */
static long long parse_i64(const char *s)
{
    long long value = 0;

    while (*s >= '0' && *s <= '9')
    {
        value = value * 10 + (*s - '0');
        s++;
    }
    return value;
}

bool metro_manifest_load(metro_manifest_t *out)
{
    int fd;
    char line[64];

    memset(out, 0, sizeof(*out));

    fd = open(MANIFEST_PATH, O_RDONLY);
    if (fd < 0)
        return false;

    while (read_line(fd, line, sizeof(line)) > 0)
    {
        char *name, *value;

        if (!settings_parseline(line, &name, &value))
            continue;

        if (!strcmp(name, "music_count"))
            out->music_count = (int)parse_i64(value);
        else if (!strcmp(name, "music_bytes"))
            out->music_bytes = parse_i64(value);
        else if (!strcmp(name, "video_count"))
            out->video_count = (int)parse_i64(value);
        else if (!strcmp(name, "video_bytes"))
            out->video_bytes = parse_i64(value);
        else if (!strcmp(name, "photo_count"))
            out->photo_count = (int)parse_i64(value);
        else if (!strcmp(name, "photo_bytes"))
            out->photo_bytes = parse_i64(value);
        else if (!strcmp(name, "playlist_count"))
            out->playlist_count = (int)parse_i64(value);
        else if (!strcmp(name, "video_movies_count"))
        { out->video_movies_count = (int)parse_i64(value); out->has_video_categories = true; }
        else if (!strcmp(name, "video_series_count"))
        { out->video_series_count = (int)parse_i64(value); out->has_video_categories = true; }
        else if (!strcmp(name, "video_clips_count"))
        { out->video_clips_count = (int)parse_i64(value); out->has_video_categories = true; }
        else if (!strcmp(name, "photo_images_count"))
        { out->photo_images_count = (int)parse_i64(value); out->has_photo_categories = true; }
        else if (!strcmp(name, "photo_photos_count"))
        { out->photo_photos_count = (int)parse_i64(value); out->has_photo_categories = true; }
        else if (!strcmp(name, "photo_ai_count"))
        { out->photo_ai_count = (int)parse_i64(value); out->has_photo_categories = true; }
    }
    close(fd);
    return true;
}
