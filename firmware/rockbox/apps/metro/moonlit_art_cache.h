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
/* D-042: resolucion album -> pista -> pixeles + pasada de precarga
 * (patron D-224, AF/aura_music.c:221-300) para el cache .pfraw de
 * moonlit_art.h. Separado de moonlit_art.c a proposito -- ese archivo
 * es el formato .pfraw puro (D-020, cero dependencias de Rockbox mas
 * alla de file.h/lcd.h, compilable y enlazable con `cc` de host); este
 * necesita metro_music.h/metro_albumart.h/metro_settings.h/
 * moonlit_palette.h reales y por lo tanto NO es host-testable (mismo
 * criterio que metro_thumbs.c, que tampoco lo es). */
#ifndef MOONLIT_ART_CACHE_H
#define MOONLIT_ART_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lcd.h"

/* D-030: tapa central de Marea, 120 px; D-020/plan M7: mismo radio de
 * esquina que design-system/tokens.json shape.corner_s (8). */
#define MOONLIT_ART_CACHE_SIZE   120
#define MOONLIT_ART_CACHE_RADIUS 8

/* Ruta del .pfraw de `seek` a `size` px bajo
 * metro_settings_metro_cache_dir("art", ...) (= .../aura/moonlitcache/art/,
 * D-023) -- expuesta (no `static`) porque 05-plan-correctivo.md §M7 la
 * pide como función propia, no solo un detalle interno de
 * moonlit_art_load_for_album(). */
void moonlit_art_pfraw_path(int32_t seek, int size, char *out, size_t outsz);

/* Resuelve la carátula de `album_seek` a `out` (MOONLIT_ART_CACHE_SIZE
 * x MOONLIT_ART_CACHE_SIZE, reservado por el llamador): cache-hit ->
 * solo lee el .pfraw; cache-miss -> decodifica vía
 * metro_albumart_decode_track_cover_sized(), hornea esquinas contra
 * moonlit_color(MROLE_SURFACE) y escribe el .pfraw para la próxima
 * vez. false si el álbum no tiene ninguna pista resoluble o ninguna
 * carátula real (folder ni embebida) -- el llamador cae al monograma
 * (M8). Nunca decodifica JPEG dentro de un bucle de animación: Marea
 * solo debe llamar esto cuando ya sabe que hubo un cache-miss (o
 * durante moonlit_art_precache(), antes de que la pantalla exista). */
bool moonlit_art_load_for_album(int32_t album_seek, fb_data *out);

typedef void (*moonlit_art_progress_fn)(int done, int total);

/* D-224: recorre metro_music_albums() una vez por biblioteca lista
 * (moonlit_art_cache_on_db_ready() más abajo la llama una sola vez por
 * arranque) y llama moonlit_art_load_for_album() por CADA álbum --
 * incluidos los que ya tienen .pfraw: esa llamada es la que decide
 * hit/miss y dejar constancia de cuál fue (DEBUGF "moonlit_art: hit"/
 * "moonlit_art: decode", 05-plan-correctivo.md M7 def-de-hecho) es su
 * trabajo, no el de esta función -- un hit es solo un open()+read() de
 * 16 bytes de cabecera, barato incluso recorriendo toda la biblioteca
 * en cada arranque. yield() tras cada álbum. `progress_cb` es opcional
 * (NULL = sin progreso visible); M7 no conecta ninguna pantalla a
 * esto todavía (moonlit_art_cache.c no incluye ningún metro_screen_*,
 * ver 05-plan-correctivo.md M7 "NO tocar: pantallas") -- queda como
 * infraestructura para cuando M8 (Marea) o una revisión posterior
 * decida mostrar progreso. */
void moonlit_art_precache(moonlit_art_progress_fn progress_cb);

/* Enganche de una sola vez por arranque -- llamar desde
 * metro_music_db_ready() (capa de datos, no una pantalla) justo cuando
 * la base de datos se vuelve usable por primera vez, mismo punto que
 * aura_music_db_ready() dispara aura_music_precache_album_art()
 * (AF/aura_music.c:471-473). Idempotente: no hace nada si ya corrió en
 * este arranque. */
void moonlit_art_cache_on_db_ready(void);

#endif /* MOONLIT_ART_CACHE_H */
