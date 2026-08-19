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
#include <stdlib.h>

#include "file.h"
#include "rbpaths.h"
#include "misc.h"

#include "metro_manifest.h"

#define METRO_DIR           ROCKBOX_DIR "/aura"
#define MANIFEST_PATH        METRO_DIR "/sync_summary.cfg"

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
            out->music_count = atoi(value);
        else if (!strcmp(name, "video_count"))
            out->video_count = atoi(value);
        else if (!strcmp(name, "photo_count"))
            out->photo_count = atoi(value);
        /* Byte totals, playlist_count, per-category breakdown: not
         * read here, see the module comment in metro_manifest.h. */
    }
    close(fd);
    return true;
}
