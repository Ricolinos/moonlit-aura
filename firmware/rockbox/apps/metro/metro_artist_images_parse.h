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
/* R3-F3/DD-6 (M-064): artist_images.cfg's line format is INVERTED from
 * the usual "key: value" convention on purpose -- the FILENAME is the
 * key (FAT-safe, guaranteed colon-free by construction) and the ARTIST
 * TAG is the value (arbitrary, may itself contain ':') --
 * CONTRATO-firmware-studio.md §D.3. Splitting at the first ':' works
 * cleanly either way, but only because the unpredictable field is the
 * one that never needs splitting itself.
 *
 * Pure C99, no Rockbox includes -- host-testable (apps/metro/test/),
 * same split as Aura-Firmware's own aura_artist_images_parse.c
 * (consulted read-only, INVESTIGACION-metro-r3.md B.2) between "parse
 * one line" (here) and "the index built from many lines"
 * (metro_artist_images.h). */
#ifndef METRO_ARTIST_IMAGES_PARSE_H
#define METRO_ARTIST_IMAGES_PARSE_H

#include <stdbool.h>
#include <stddef.h>

/* Contract's own limits (CONTRATO-firmware-studio.md §D.3): 128 B
 * filename, 64 B artist tag -- the latter matches Metro's own
 * METRO_MUSIC_ITEM_LEN (metro_music.h), the length metro_music_artists()
 * already caps tag_artist strings to. */
#define METRO_ARTIST_IMAGES_FILE_LEN   128
#define METRO_ARTIST_IMAGES_ARTIST_LEN 64

/* Parses one "<filename>.jpg: <artist tag>" line -- filename and
 * artist tag land trimmed (leading/trailing whitespace stripped) in
 * out_filename/out_artist. Blank lines and comments ('#' as the first
 * non-whitespace character) return false, same as a line with no ':'
 * at all. A field that would reach its own cap (128 B filename / 64 B
 * artist) drops the whole line -- "tope ... excedido = línea
 * ignorada" (B.1). */
bool metro_artist_images_parse_line(const char *line,
                                     char *out_filename, size_t filename_cap,
                                     char *out_artist, size_t artist_cap);

#endif /* METRO_ARTIST_IMAGES_PARSE_H */
