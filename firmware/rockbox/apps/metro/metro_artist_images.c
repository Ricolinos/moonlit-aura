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
#include "metro_artist_images.h"

#include <string.h>

void metro_artist_images_init(struct metro_artist_images *idx)
{
    idx->count = 0;
}

bool metro_artist_images_add_line(struct metro_artist_images *idx, const char *line)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];
    int i;

    if (!metro_artist_images_parse_line(line, filename, sizeof(filename),
                                         artist, sizeof(artist)))
        return false;

    for (i = 0; i < idx->count; i++)
        if (!strcmp(idx->entries[i].artist, artist))
            return true; /* duplicate artist value -- first line already won */

    if (idx->count >= METRO_ARTIST_IMAGES_MAX)
        return true; /* index full -- syntactically valid, silently dropped */

    memcpy(idx->entries[idx->count].filename, filename, sizeof(filename));
    memcpy(idx->entries[idx->count].artist, artist, sizeof(artist));
    idx->count++;
    return true;
}

const char *metro_artist_images_lookup(const struct metro_artist_images *idx,
                                        const char *artist_tag)
{
    int i;
    for (i = 0; i < idx->count; i++)
        if (!strcmp(idx->entries[i].artist, artist_tag))
            return idx->entries[i].filename;
    return NULL;
}
