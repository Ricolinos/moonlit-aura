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
/* Album art for the currently playing track (PLAN_MAESTRO.md S1.4,
 * S1.2 "cache de 1"), plus (R3-F4/DD-5, M-065) a standalone resolver
 * for an ARBITRARY track's art -- Quickplay needs a representative
 * cover per album, not just the one audio_current_track() points at.
 * Still narrower than Aura-Firmware's aura_albumart.c: no disk-cached
 * .pfraw, no precache pass -- metro_thumbs.c's own RAM window + disk
 * cache (DD-1) already covers that for whichever tiles are on screen,
 * there's no reason to duplicate it here.
 */
#ifndef METRO_ALBUMART_H
#define METRO_ALBUMART_H

#include <stdbool.h>
#include "lcd.h"

#define METRO_ALBUMART_SIZE 136

/* Loads (or reuses the cached decode of) the art for
 * audio_current_track() -- folder art first (find_albumart(), cover.jpg
 * next to the track or in its parent dir), embedded JPEG (ID3 APIC)
 * otherwise. False if nothing is playing or the track has no art at
 * all -- draw metro_draw_tile() instead. */
bool metro_albumart_load_current(void);

/* Valid only right after metro_albumart_load_current() returned true --
 * METRO_ALBUMART_SIZE x METRO_ALBUMART_SIZE, row-major, native LCD
 * format (ready for lcd_bitmap()). */
const fb_data *metro_albumart_bitmap(void);

/* F12: same track as metro_albumart_load_current(), scaled to fill
 * the whole screen (LCD_WIDTH x LCD_HEIGHT) instead of the small NP
 * tile -- for the dimmed background behind Now Playing
 * (PLAN_MAESTRO.md S3.3, graphics=full only -- the caller decides
 * whether to call this at all, this module doesn't read
 * metro_settings itself). Cached independently of
 * metro_albumart_load_current()'s own tile-sized cache -- calling one
 * has no effect on the other. */
bool metro_albumart_load_background(void);

/* R4/FA-7 (M-078): mismo destino y misma caché-de-1 que
 * metro_albumart_load_background(), pero desde un archivo de imagen
 * ARBITRARIO en vez de la carátula de la pista en reproducción -- el
 * caso real es una foto de artista bajo `.rockbox/aura/artists/`.
 *
 * **Este módulo no decide cuál usar.** La política (foto de artista si
 * existe, si no la carátula, si no fondo plano) vive en el llamador,
 * igual que la decisión de dibujar fondo o no ya vivía ahí -- ver la
 * cabecera de este archivo y metro_screen_nowplaying.c.
 *
 * Las dos funciones comparten búfer y clave de caché, así que llamar a
 * una invalida lo que la otra hubiera dejado: es un solo fondo en
 * pantalla a la vez, por construcción. */
bool metro_albumart_load_background_file(const char *path);

/* Valid only right after metro_albumart_load_background() -- or
 * metro_albumart_load_background_file() -- returned true:
 * LCD_WIDTH x LCD_HEIGHT, row-major, native LCD format. */
const fb_data *metro_albumart_background_bitmap(void);

/* R3-F4/DD-5 (M-065): resolves and decodes art for `track_path` (any
 * real track file, not necessarily the one playing) straight into
 * `out` -- METRO_TILE_SIZE x METRO_TILE_SIZE (metro_draw.h), ready for
 * metro_thumbs.c's "albums" source. Reads the track's own tags via
 * get_metadata() (folder art needs id3->path/album, embedded art needs
 * the real has_embedded_albumart/albumart.* fields -- a hand-built
 * stand-in mp3entry with just a path would silently never find
 * embedded art). Decodes at METRO_ALBUMART_SIZE first (the same
 * already-proven-safe target metro_albumart_load_current() uses for
 * covers of any real-world size) and downscales the ALREADY-DECODED
 * PIXELS to the tile size -- not a second JPEG decode at 80px, which
 * would risk the exact JPEG_DECODE_OVERHEAD gap R3-F3 hit for artist
 * photos (docs/DESVIACIONES.md R3-3) for any cover landing near that
 * size. Returns false if the track has no metadata Rockbox can read,
 * or no art at all -- caller falls back to the usual accent tile. */
bool metro_albumart_decode_track_cover(const char *track_path, fb_data *out);

#endif /* METRO_ALBUMART_H */
