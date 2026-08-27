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
#include <stddef.h>
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

/* moonlit (D-059): raw decodes shared by the master-art builder thread
 * (own scratch, own mp3entry) and the UI (the *_ui variants below,
 * this module's own s_scratch). Every one of them decodes with
 * FORMAT_KEEP_ASPECT into a METRO_ALBUMART_SIZE box -- the same
 * already-proven-safe target metro_albumart_load_current() uses, never
 * a second JPEG decode at the final size (R3-F4/DD-5, docs/DESVIACIONES.md
 * R3-3) -- and reports the real decoded `w` x `h` (one side ==
 * METRO_ALBUMART_SIZE, the other <= it) so the caller can
 * fill-and-center-crop the ALREADY-DECODED pixels
 * (moonlit_master_art_resample_cover()). `scratch` must be at least
 * METRO_ALBUMART_SCRATCH_SIZE: FORMAT_RESIZE needs real working room
 * beyond the final bitmap (JPEG_DECODE_OVERHEAD) -- undersizing it
 * corrupts whatever the linker placed next, it does not fail loudly.
 * Any thread: the JPEG decoder's single static state is serialised
 * inside via moonlit_master_art_lock(). */
#define METRO_ALBUMART_SCRATCH_SIZE \
    (METRO_ALBUMART_SIZE * METRO_ALBUMART_SIZE * 2 * 2)

/* Any image file (.jpg/.jpeg/.bmp): artist photos, /Photos. */
bool metro_albumart_decode_file_raw(const char *art_path, unsigned char *scratch,
                                    size_t scratch_size, int *w, int *h);

/* Art for `track_path` (any real track file, not necessarily the one
 * playing): folder art via find_albumart(), embedded JPEG otherwise.
 * Reads the track's own tags via get_metadata() into `id3` (folder art
 * needs id3->path/album, embedded art needs the real
 * has_embedded_albumart/albumart.* fields -- a hand-built stand-in
 * mp3entry with just a path would silently never find embedded art).
 * false if the track has no readable metadata or no art at all. */
struct mp3entry;
bool metro_albumart_decode_track_cover_raw(const char *track_path, struct mp3entry *id3,
                                           unsigned char *scratch, size_t scratch_size,
                                           int *w, int *h);

/* UI-thread variants into this module's own scratch: the returned
 * pointer (row-major `w` x `h`, native LCD format) is valid until the
 * next call into this module. Both invalidate
 * metro_albumart_load_current()'s cache-of-1 (same buffer). NULL on
 * failure. */
const fb_data *metro_albumart_decode_track_cover_ui(const char *track_path, int *w, int *h);
const fb_data *metro_albumart_decode_file_ui(const char *art_path, int *w, int *h);

#endif /* METRO_ALBUMART_H */
