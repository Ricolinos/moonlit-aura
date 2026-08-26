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
/* moonlit: derived from aura_art.c/.h @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-020, D-042). Alcance identico al
 * de D-020 (formato .pfraw + horneado de esquinas) menos la quinta
 * función original (columna contigua, D-030 fija Marea en fila-
 * contigua): ningún consumidor de moonlit necesita ese otro layout
 * de pictureflow.c.
 *
 * Modulo puro: solo depende de fb_data (lcd.h) y E/S de archivo
 * (file.h) -- nada de aura_settings/apple2026_shell. `theme` reemplaza
 * al global aura_settings.theme de la version original: aqui lo pasa
 * el llamador (moonlit_art_cache.c, D-042) para no atar este archivo a
 * ningun modulo de Rockbox mas alla de file.h/dir.h/lcd.h, y para que
 * apps/metro/test/test_art.c pueda compilarlo y enlazarlo con `cc` de
 * host (test/file.h, test/dir.h, test/lcd.h son los unicos stands-in;
 * dir.h desde D-056, por el barrido de huerfanos). */
#ifndef MOONLIT_ART_H
#define MOONLIT_ART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lcd.h"

/* D-042: cabecera en disco -- mismos 4 campos int32 que
 * aura_art_pfraw_header (16 bytes), pero el 3er/4to campo cambian de
 * sentido: `layout` reemplaza al `theme` original (siempre
 * MOONLIT_ART_LAYOUT_ROW_MAJOR, discrimina un futuro formato
 * transpuesto si alguna vez hiciera falta) y `extra` pasa a llevar el
 * tema activo (night/dawn, D-027) -- la unica llave de invalidacion
 * que Marea necesita (una sola carpeta de albumes, D-023, sin el
 * segundo uso de `extra` que aura_photos.c le daba a mtime). */
#define MOONLIT_ART_LAYOUT_ROW_MAJOR 1

/* D-049: == MAX_PATH (firmware/include/fs_defines.h) -- not included
 * here so this file keeps compiling with a host `cc` (test/). */
#define MOONLIT_ART_PATH_MAX 260

/* Bitmap size x size, fila contigua, con esquinas ya horneadas al
 * radio y fondo pedidos por moonlit_art_mask_corners() -- lee el
 * archivo de `path` a `out` (reservado por el llamador, size*size
 * fb_data) solo si la cabecera coincide con size/radius/theme.
 * Devuelve false en cache-miss (archivo ausente o cabecera distinta),
 * nunca a medio llenar `out`. */
bool moonlit_art_read_pfraw(const char *path, int size, int radius,
                             int32_t theme, fb_data *out);

/* Escribe `data` (size x size fb_data, fila contigua) a `path` con la
 * cabecera de arriba. Sin valor de retorno, igual que
 * aura_art_write_pfraw() -- un fallo de escritura (disco lleno, ruta
 * sin el directorio padre) dega simplemente sin cache, se reintenta
 * en el proximo arranque (mismo criterio que D-224). */
void moonlit_art_write_pfraw(const char *path, int size, int radius,
                              int32_t theme, const fb_data *data);

/* Solo lee la cabecera (sin tocar los pixeles) -- para que
 * moonlit_art_precache() salte rapido los albumes ya cacheados sin
 * pagar el read() completo. */
bool moonlit_art_pfraw_is_cached(const char *path, int size, int radius,
                                  int32_t theme);

/* D-049: producer of the i-th .pfraw path for moonlit_art_count_uncached()
 * -- keeps this module free of metro_music/metro_settings (the caller
 * maps index -> album seek -> path, moonlit_art_cache.c does that). */
typedef void (*moonlit_art_path_fn)(int index, char *out, size_t outsz, void *ctx);

/* D-049: how many of `count` paths are NOT resolved (missing file or
 * header mismatch, and no .none marker -- D-056), header reads only -- the number the "Preparando
 * biblioteca" screen shows as the total, and the reason it can decide
 * to show nothing at all (0 pending == library unchanged, AF pattern
 * aura_music.c:352-358). Pure: host-tested in test/test_art.c. */
int moonlit_art_count_uncached(int count, moonlit_art_path_fn path_fn, void *ctx,
                               int size, int radius, int32_t theme);

/* --- D-056: cache negativa ------------------------------------------ */

/* Ruta del marcador "<clave>.none" a partir de la ruta del .pfraw
 * "<dir>/<clave>-<size>.pfraw" (misma clave estable D-055, sin tamano
 * ni tema: "no hay caratula" no depende de ninguno de los dos). false
 * si `pfraw_path` no tiene esa forma (out queda vacio). */
bool moonlit_art_none_path(const char *pfraw_path, char *out, size_t outsz);

/* Marcador de 0 bytes: "ya se intento decodificar este album y no
 * tiene caratula resoluble" -- el pre-pase lo cuenta como resuelto y
 * Marea cae al monograma sin abrir la pista. Se borra solo cuando la
 * clave cambia (GC) o cuando un decode posterior si produce .pfraw. */
void moonlit_art_write_none(const char *none_path);
bool moonlit_art_none_exists(const char *none_path);

/* .pfraw valido para (size, radius, theme) O marcador .none presente:
 * nada que hacer en la precarga. Cabeceras/open() solamente. */
bool moonlit_art_is_resolved(const char *pfraw_path, int size, int radius,
                             int32_t theme);

/* D-056: barrido de huerfanos host-testable. Borra de `dir` todo
 * archivo que termine en `suffix` cuyo tallo (nombre sin el sufijo) no
 * pase `keep(stem, ctx)`; nombres que empiecen con '.' u otros sufijos
 * se dejan. Devuelve cuantos borro. Antes era gc_sweep() estatica en
 * moonlit_art_cache.c (D-055); aqui para que test_art.c cubra que un
 * .none huerfano cae igual que un .pfraw. */
typedef bool (*moonlit_art_keep_fn)(const char *stem, void *ctx);
int moonlit_art_sweep(const char *dir, const char *suffix,
                      moonlit_art_keep_fn keep, void *ctx);

/* Recorta las 4 esquinas de un bitmap fila-contigua (buf, size x size)
 * al radio pedido, mezclando hacia bg en el borde -- se hornea UNA VEZ
 * antes de cachear (D-020), costo cero en cada cuadro de Marea. */
void moonlit_art_mask_corners(fb_data *buf, int size, int radius, unsigned bg);

#endif /* MOONLIT_ART_H */
