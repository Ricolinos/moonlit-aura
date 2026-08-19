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
#include "string-extra.h"

#include "metro_media_categories.h"

#define METRO_DIR                ROCKBOX_DIR "/aura"
#define VIDEO_CATEGORIES_PATH    METRO_DIR "/video_categories.cfg"
#define PHOTO_CATEGORIES_PATH    METRO_DIR "/photo_categories.cfg"

/* Matches the contract's own scan caps (100 videos / 500 photos) with
 * headroom -- an index entry only matters for a file that's actually
 * listed, so it can never need to hold more distinct names than that. */
#define METRO_CATEGORIES_MAX 512

/* Local copy of metro_fsutil.h's name length to avoid pulling that
 * header in just for one constant -- filenames here come straight from
 * settings_parseline(), not from metro_fsutil_list_by_ext(). */
#define METRO_FSUTIL_NAME_LEN_LOCAL 96

typedef struct {
    char filename[METRO_FSUTIL_NAME_LEN_LOCAL];
    int  code; /* metro_video_cat_t or metro_photo_cat_t, whichever table */
} category_entry_t;

static category_entry_t s_video[METRO_CATEGORIES_MAX];
static int s_video_n;
static category_entry_t s_photo[METRO_CATEGORIES_MAX];
static int s_photo_n;

static int video_code_for(const char *v)
{
    if (!strcmp(v, "movie"))  return METRO_VIDEO_CAT_MOVIE;
    if (!strcmp(v, "series")) return METRO_VIDEO_CAT_SERIES;
    if (!strcmp(v, "clip"))   return METRO_VIDEO_CAT_CLIP;
    return METRO_VIDEO_CAT_NONE; /* unknown code: forward-compat, ignored */
}

static int photo_code_for(const char *v)
{
    if (!strcmp(v, "photo")) return METRO_PHOTO_CAT_PHOTO;
    if (!strcmp(v, "image")) return METRO_PHOTO_CAT_IMAGE;
    if (!strcmp(v, "ai"))    return METRO_PHOTO_CAT_AI;
    return METRO_PHOTO_CAT_NONE;
}

static int load_index(const char *path, category_entry_t *out, int max,
                       int (*code_for)(const char *))
{
    int fd, n = 0;
    char line[160]; /* filename up to 95 bytes + ": " + code word */

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;

    while (n < max && read_line(fd, line, sizeof(line)) > 0)
    {
        char *name, *value;
        int code;

        if (!settings_parseline(line, &name, &value))
            continue;
        code = code_for(value);
        if (code == 0)
            continue; /* unknown/empty code: ignored, forward-compat */

        strlcpy(out[n].filename, name, sizeof(out[n].filename));
        out[n].code = code;
        n++;
    }
    close(fd);
    return n;
}

void metro_media_categories_load_video(void)
{
    s_video_n = load_index(VIDEO_CATEGORIES_PATH, s_video, METRO_CATEGORIES_MAX, video_code_for);
}

void metro_media_categories_load_photo(void)
{
    s_photo_n = load_index(PHOTO_CATEGORIES_PATH, s_photo, METRO_CATEGORIES_MAX, photo_code_for);
}

metro_video_cat_t metro_media_categories_video_lookup(const char *filename)
{
    int i;
    for (i = 0; i < s_video_n; i++)
        if (!strcmp(s_video[i].filename, filename))
            return (metro_video_cat_t)s_video[i].code;
    return METRO_VIDEO_CAT_NONE;
}

metro_photo_cat_t metro_media_categories_photo_lookup(const char *filename)
{
    int i;
    for (i = 0; i < s_photo_n; i++)
        if (!strcmp(s_photo[i].filename, filename))
            return (metro_photo_cat_t)s_photo[i].code;
    return METRO_PHOTO_CAT_NONE;
}
