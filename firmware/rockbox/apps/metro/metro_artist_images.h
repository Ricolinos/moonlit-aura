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
/* R3-F3/DD-6 (M-064): the in-RAM index built from artist_images.cfg --
 * pure C99, no Rockbox includes, no file I/O (host-testable). Actually
 * reading the file (open/read_line/close, Rockbox-only) lives in
 * metro_music.c, the same split metro_lrc.c/metro_screen_nowplaying.c
 * already established in R3-F2: this module never touches a
 * filesystem, its caller feeds it lines one at a time. */
#ifndef METRO_ARTIST_IMAGES_H
#define METRO_ARTIST_IMAGES_H

#include <stdbool.h>

#include "metro_artist_images_parse.h"

/* 300 entries, matching both the contract's own cap (B.1) and Metro's
 * existing METRO_MUSIC_MAX_ITEMS (metro_music.h) -- an entry only
 * matters for an artist that could actually show up in that same
 * list, so it can never need to hold more distinct artists than that. */
#define METRO_ARTIST_IMAGES_MAX 300

struct metro_artist_image_entry {
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];
};

struct metro_artist_images {
    struct metro_artist_image_entry entries[METRO_ARTIST_IMAGES_MAX];
    int count;
};

void metro_artist_images_init(struct metro_artist_images *idx);

/* Parses one line (metro_artist_images_parse_line()) and adds it to
 * idx if valid, enforcing the contract's two index-level rules: a
 * repeated artist VALUE keeps only its FIRST line ("valor duplicado ->
 * gana la primera línea", B.1 -- several filenames can legitimately
 * point at variant spellings of the same artist tag, but only the
 * first-seen mapping for a given tag is kept), and idx never grows
 * past METRO_ARTIST_IMAGES_MAX entries (further valid lines are
 * silently ignored once full, not an error). Pure, no I/O -- safe to
 * drive with synthetic lines from a test. Returns true if the line was
 * syntactically valid (whether or not it actually got added -- a
 * legitimate duplicate-value or index-full skip still counts as
 * "handled", not a parse failure), false only if
 * metro_artist_images_parse_line() itself rejected the line. */
bool metro_artist_images_add_line(struct metro_artist_images *idx, const char *line);

/* Filename (relative to the artists/ source directory -- NOT a full
 * path) for `artist_tag`: an exact byte-for-byte match against the raw
 * tag_artist string, no accent normalization (B.1 -- the same tag
 * metro_music_artists() already exposes as metro_music_item_t.label).
 * Returns NULL if no entry matches (no image for this artist, or the
 * index is empty). */
const char *metro_artist_images_lookup(const struct metro_artist_images *idx,
                                        const char *artist_tag);

#endif /* METRO_ARTIST_IMAGES_H */
