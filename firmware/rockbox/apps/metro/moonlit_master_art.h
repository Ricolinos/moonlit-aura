/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* D-059 (contrato v16): shared MASTER art cache under /.aura/art/ --
 * one decode per image, shared by the three firmware families (Aura,
 * Metro, moonlit). Each family derives its own working size from the
 * master when it loads it into RAM (moonlit: Marea 130 -> 120 + baked
 * corners, grids 130 -> 80), never the other way round.
 *
 * Pure module (same criterion as moonlit_art.c, D-042): only fb_data
 * (lcd.h) and file/dir I/O -- no Rockbox headers beyond those, so
 * apps/metro/test/test_master_art.c compiles and links it with a host
 * `cc` against test/file.h, test/dir.h, test/lcd.h. Everything that
 * needs tagcache, crc_32(), metro_settings paths or a thread lives in
 * moonlit_master_art_builder.c / moonlit_art_cache.c.
 *
 * On-disk format (byte-identical in the three families):
 *   16-byte little-endian header { uint32 magic 'MAST', uint16 width,
 *   uint16 height, uint32 flags = 0, uint32 reserved = 0 } followed by
 *   width*height RGB565 LE pixels, row-major, square, NO corners, NO
 *   theme, NO reflection (fill-and-center-crop of the source).
 * Sizes: albums 130, artists 130, photos 80.
 * Names: "<key>.art" where key is
 *   albums  "a-<crc32 hex8 of the representative track path>.<mtime>"
 *           (== metro_music_album_art_key(), D-055),
 *   artists "r-<crc32 hex8 of the image file path>.<mtime>",
 *   photos  "p-<crc32 hex8 of the photo file path>.<mtime>".
 * A 0-byte "<key>.none" with the same key is the shared negative
 * marker (replaces D-056's private moonlitcache/art/<key>.none). */
#ifndef MOONLIT_MASTER_ART_H
#define MOONLIT_MASTER_ART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lcd.h"

#define MOONLIT_MASTER_ART_MAGIC        0x5453414Du /* 'MAST' as LE uint32 */
#define MOONLIT_MASTER_ART_HEADER_SIZE  16

#define MOONLIT_MASTER_ART_ALBUM_SIZE   130
#define MOONLIT_MASTER_ART_ARTIST_SIZE  130
#define MOONLIT_MASTER_ART_PHOTO_SIZE   80

/* Largest master any consumer reads: sizes a scratch of
 * MOONLIT_MASTER_ART_MAX_SIZE^2 fb_data (33 800 B). */
#define MOONLIT_MASTER_ART_MAX_SIZE     130

/* == MAX_PATH (firmware/include/fs_defines.h), not included here so
 * this file keeps compiling with a host `cc` (test/). */
#define MOONLIT_MASTER_ART_PATH_MAX     260

/* "<prefix>-<crc hex8>.<mtime>" -- the stem shared by "<stem>.art" and
 * "<stem>.none". Needs 2 + 8 + 1 + up to 11 + NUL = 23 -> 24. The
 * caller computes `crc` (Rockbox crc_32(path, strlen(path),
 * 0xffffffff), the same call metro_music.c uses for album keys) so
 * this module stays host-buildable without crc32.h. */
#define MOONLIT_MASTER_ART_KEY_LEN 24
void moonlit_master_art_file_key(char prefix, uint32_t crc, long mtime,
                                 char *out, size_t outsz);

/* "<dir>/<stem>.art" -> "<dir>/<stem>.none". false (out emptied) if
 * `art_path` does not end in ".art" or `out` is too small. */
bool moonlit_master_art_none_path(const char *art_path, char *out, size_t outsz);

/* Header-only check (one open() + 16-byte read): a master of exactly
 * `size` x `size` exists at `path`. */
bool moonlit_master_art_exists(const char *path, int size);

/* Reads a `size` x `size` master into `out` (size*size fb_data,
 * caller's). false on missing file / header mismatch / short read --
 * never leaves `out` half-filled as a success. */
bool moonlit_master_art_read(const char *path, int size, fb_data *out);

/* Writes header + pixels. No return value (same criterion as
 * moonlit_art_write_pfraw(): a failed write just means no cache, it is
 * retried on the next builder pass). Writes to "<path>.tmp" first and
 * renames, so a sibling family never reads a half-written master. */
void moonlit_master_art_write(const char *path, int size, const fb_data *data);

void moonlit_master_art_write_none(const char *none_path);
bool moonlit_master_art_none_exists(const char *none_path);

/* Valid master for `size` OR its ".none" marker present: nothing for
 * the builder to do. Headers/open() only. */
bool moonlit_master_art_is_resolved(const char *art_path, int size);

/* Creates every missing ancestor of `dir` and `dir` itself (mkdir -p,
 * same walk metro_thumbs.c's ensure_cache_dir() does for /.aura/thumbs). */
void moonlit_master_art_ensure_dir(const char *dir);

/* --- version de formato de la cache (contrato v18, D-063) --------------- */

/* `/.aura/art/format.txt` lleva un entero decimal: la version de
 * formato de TODO el arbol de caratulas derivadas. Las tres familias lo
 * leen al arrancar; la que encuentre un valor menor que el suyo (o el
 * archivo ausente) purga las cachés derivadas y escribe el suyo. Studio
 * nunca lo toca. Sirve para lo que las claves no pueden arreglar solas:
 * un tile mal derivado por una version anterior del codigo sobrevive
 * para siempre aunque el codigo ya este corregido, porque su clave no
 * cambio. La ruta la compone metro_settings.c (regla de rutas de
 * CLAUDE.md); aqui solo vive el formato del archivo. */
#define MOONLIT_MASTER_ART_FORMAT_VERSION 2

/* Entero del archivo, o 0 si falta, esta vacio o no es un numero. */
int moonlit_master_art_format_read(const char *path);

/* Escribe "<version>\n". false si no pudo. */
bool moonlit_master_art_format_write(const char *path, int version);

/* --- pure pixel ops (integer only, no FPU) ------------------------------ */

/* Fill-and-center-crop resample of a `sw` x `sh` row-major RGB565
 * bitmap into a `size` x `size` one: scale so the SHORT side becomes
 * `size`, crop the long side centred. Each destination pixel averages
 * the source box it covers (integer box filter -- exact area average
 * when the scale is integral, 1-2 px boxes otherwise), so it is both
 * the "cover crop" the master needs from a KEEP_ASPECT decode and a
 * plain box downscale when sw == sh. Upscaling degenerates to nearest
 * neighbour (1 px boxes). `src` and `dst` must not overlap. */
void moonlit_master_art_resample_cover(const fb_data *src, int sw, int sh,
                                       fb_data *dst, int size);

/* Square-to-square box downscale (ssize >= dsize): the per-family
 * derivation 130 -> 120 (Marea) and 130 -> 80 (grids). Same kernel as
 * moonlit_master_art_resample_cover() with sw == sh == ssize. */
void moonlit_master_art_box_downscale(const fb_data *src, int ssize,
                                      fb_data *dst, int dsize);

#endif /* MOONLIT_MASTER_ART_H */
