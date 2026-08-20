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
#include "metro_artist_images_parse.h"

#include <string.h>

static const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    return p;
}

static size_t trim_len(const char *start, size_t len)
{
    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t' ||
                        start[len - 1] == '\r' || start[len - 1] == '\n'))
        len--;
    return len;
}

bool metro_artist_images_parse_line(const char *line,
                                     char *out_filename, size_t filename_cap,
                                     char *out_artist, size_t artist_cap)
{
    const char *p = skip_ws(line);
    const char *colon;
    const char *key_start, *val_start;
    size_t key_len, val_len;

    if (*p == '\0' || *p == '#')
        return false;

    colon = strchr(p, ':');
    if (!colon)
        return false;

    key_start = p;
    key_len = trim_len(key_start, (size_t)(colon - key_start));
    if (key_len == 0 || key_len >= filename_cap)
        return false;

    val_start = skip_ws(colon + 1);
    val_len = trim_len(val_start, strlen(val_start));
    if (val_len == 0 || val_len >= artist_cap)
        return false;

    memcpy(out_filename, key_start, key_len);
    out_filename[key_len] = '\0';
    memcpy(out_artist, val_start, val_len);
    out_artist[val_len] = '\0';
    return true;
}
