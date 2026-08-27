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

/* D-049: == MAX_PATH (firmware/include/fs_defines.h) -- not included
 * here so this file keeps compiling with a host `cc` (test/). */
#define MOONLIT_ART_PATH_MAX 260

/* D-059: the .pfraw format that used to live here (D-020/D-042 --
 * moonlit's private, theme-baked 120 px Marea cover on disk) is gone:
 * the shared master (moonlit_master_art.h, /.aura/art/) replaced it
 * and the 120 px cover is now derived at load. What remains is what
 * still has no other home: the corner bake (applied to the DERIVED
 * cover, once per load) and the host-testable orphan sweep. */

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
 * al radio pedido, mezclando hacia bg en el borde -- se hornea al
 * derivar la tapa de 120 px desde la maestra (D-059; antes, una vez
 * antes de cachear el .pfraw, D-020), costo cero en cada cuadro. */
void moonlit_art_mask_corners(fb_data *buf, int size, int radius, unsigned bg);

#endif /* MOONLIT_ART_H */
