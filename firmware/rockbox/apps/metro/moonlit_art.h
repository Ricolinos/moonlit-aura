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
 * ningun modulo de Rockbox mas alla de file.h/lcd.h, y para que
 * apps/metro/test/test_art.c pueda compilarlo y enlazarlo con `cc` de
 * host (test/file.h, test/lcd.h son los unicos stands-in). */
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

/* D-049: how many of `count` paths are NOT cached (missing file or
 * header mismatch), header reads only -- the number the "Preparando
 * biblioteca" screen shows as the total, and the reason it can decide
 * to show nothing at all (0 pending == library unchanged, AF pattern
 * aura_music.c:352-358). Pure: host-tested in test/test_art.c. */
int moonlit_art_count_uncached(int count, moonlit_art_path_fn path_fn, void *ctx,
                               int size, int radius, int32_t theme);

/* Recorta las 4 esquinas de un bitmap fila-contigua (buf, size x size)
 * al radio pedido, mezclando hacia bg en el borde -- se hornea UNA VEZ
 * antes de cachear (D-020), costo cero en cada cuadro de Marea. */
void moonlit_art_mask_corners(fb_data *buf, int size, int radius, unsigned bg);

#endif /* MOONLIT_ART_H */
