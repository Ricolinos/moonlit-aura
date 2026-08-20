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
/* R3-F2/DD-3 (M-063): synced .lrc lyrics -- own parser, not a port of
 * Aura-Firmware's aura_lrc.c. Aura copies every timestamped line's text
 * into a fixed 128-byte slot (INVESTIGACION-metro-r3.md A.2), ~80 KB
 * permanent RAM for 600 lines. Metro keeps the file's own bytes (it
 * has to read them into *some* buffer anyway) and stores only
 * {ms, offset} per timestamp, cutting each line's text in place with a
 * NUL -- ~8 KB buffer + 600*6 B of entries =~ 11.6 KB. Same contract
 * limits as library-layout-v1.md §3 (which Aura's own limits already
 * match): <=8 KB file, <=600 timestamped lines.
 *
 * Pure C99, no Rockbox includes -- host-testable (apps/metro/test/).
 * File I/O (deriving the sibling path is pure string work and lives
 * here; actually opening/reading it is Rockbox-specific and lives in
 * metro_screen_nowplaying.c, the only caller). */
#ifndef METRO_LRC_H
#define METRO_LRC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define METRO_LRC_MAX_LINES 600
#define METRO_LRC_BUF_SIZE  8192

struct metro_lrc_entry {
    uint32_t ms;
    uint16_t offset; /* into buf -- fits easily, buf is <= 8192 */
};

struct metro_lrc {
    char buf[METRO_LRC_BUF_SIZE];
    struct metro_lrc_entry entries[METRO_LRC_MAX_LINES];
    int count;
};

/* Derives the .lrc sibling path of `audio_path` -- same directory,
 * same base name, ".lrc" extension (library-layout-v1.md §3; same
 * convention Aura-Firmware's own derive_sibling_path() uses,
 * consulted read-only). Pure string work, no filesystem access.
 * Returns false if audio_path has no extension to replace, or the
 * result wouldn't fit in out_len. */
bool metro_lrc_sibling_path(const char *audio_path, char *out, size_t out_len);

/* Parses the first `len` bytes already sitting in lrc->buf (caller
 * fills it -- a raw file read, or a test fixture written in directly)
 * into lrc->entries, cutting each timestamped line's text in place
 * with a NUL. `len` is clamped to METRO_LRC_BUF_SIZE-1 if larger.
 * Multiple timestamps on one line ("[00:12.00][00:45.00]text") each
 * get their own entry pointing at the SAME text offset -- no copy.
 * A line with no valid timestamp is silently dropped (same as Aura's
 * own parser, verified in INVESTIGACION-metro-r3.md A.1). Assumes
 * entries end up non-decreasing in ms (true for any .lrc written in
 * the normal chronological convention); this does not sort. Returns
 * false (count left at 0) if there isn't a single valid timestamped
 * line. */
bool metro_lrc_parse(struct metro_lrc *lrc, size_t len);

/* Index (0..count-1) of the line active at `elapsed_ms`, or -1 if
 * elapsed_ms is before the first timestamp. Binary search. */
int metro_lrc_find_active(const struct metro_lrc *lrc, uint32_t elapsed_ms);

/* Text for entries[index] -- a pointer straight into lrc->buf (valid
 * as long as `lrc` itself is), or NULL if index is out of range. */
const char *metro_lrc_text(const struct metro_lrc *lrc, int index);

#endif /* METRO_LRC_H */
