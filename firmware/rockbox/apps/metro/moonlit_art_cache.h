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

/* Ruta del .pfraw del álbum `seek` a `size` px bajo
 * metro_settings_metro_cache_dir("art", ...) (= .../aura/moonlitcache/art/,
 * D-023): "<clave>-<size>.pfraw" con la clave estable de
 * metro_music_album_art_key() (D-055: "a-<crc32 ruta>.<mtime>", no el
 * seek, que tagcache renumera en cada rebuild). false si el álbum no
 * tiene pista resoluble (sin archivo de caché posible). Expuesta (no
 * `static`) porque 05-plan-correctivo.md §M7 la pide como función
 * propia. Toca tagcache (búsqueda + retrieve, memoizada): nunca desde
 * un cuadro de animación -- Marea ya no la llama (D-053). */
bool moonlit_art_pfraw_path(int32_t seek, int size, char *out, size_t outsz);

/* D-057: como moonlit_art_pfraw_path() pero nunca calcula la clave si
 * todavia no esta memoizada (metro_music_album_art_key_peek()) -- la
 * unica variante segura de llamar DENTRO de un cuadro de animacion de
 * Marea (D-053): clave desconocida -> false, sin tocar tagcache. */
bool moonlit_art_pfraw_path_peek(int32_t seek, int size, char *out, size_t outsz);

/* D-055: limpieza de huérfanos. Las claves son estables, así que un
 * huérfano solo aparece cuando un álbum desaparece o cambia de pista
 * representativa -- y cuando cambió el ESQUEMA de clave (los
 * "<seek>-120.pfraw"/"album-<seek>.mth" anteriores a D-055). No corre
 * sola: metro_sync.c la PIDE al terminar bien un sync con música
 * (bandera en disco, sobrevive reinicios) y la pantalla "preparando
 * biblioteca" la EJECUTA tras la precarga, con la pantalla puesta.
 * moonlit_art_gc(): una pasada -- tabla de crc32 de las claves de
 * todos los álbumes (en el scratch estático de la precarga, cero .bss
 * nuevo) y un barrido de moonlitcache/art/ (.pfraw y, D-056, .none) y
 * /.aura/thumbs/albums/ (.mth) borrando lo que no esté en la tabla. */
void moonlit_art_request_gc(void);
bool moonlit_art_gc_pending(void);
void moonlit_art_gc(void);

/* Resuelve la carátula de `album_seek` a `out` (MOONLIT_ART_CACHE_SIZE
 * x MOONLIT_ART_CACHE_SIZE, reservado por el llamador): cache-hit ->
 * solo lee el .pfraw; cache-miss -> decodifica vía
 * metro_albumart_decode_track_cover_sized(), hornea esquinas contra
 * moonlit_color(MROLE_SURFACE) y escribe el .pfraw para la próxima
 * vez. false si el álbum no tiene ninguna pista resoluble o ninguna
 * carátula real (folder ni embebida) -- el llamador cae al monograma
 * (M8); en ese caso deja "<clave>.none" (0 bytes, D-056) junto al
 * .pfraw que no pudo escribir, y la siguiente llamada devuelve false
 * sin abrir la pista ni decodificar nada. Nunca decodifica JPEG dentro de un bucle de animación: Marea
 * solo debe llamar esto cuando ya sabe que hubo un cache-miss (o
 * durante moonlit_art_precache(), antes de que la pantalla exista). */
bool moonlit_art_load_for_album(int32_t album_seek, fb_data *out);

typedef void (*moonlit_art_progress_fn)(int done, int total);
typedef bool (*moonlit_art_abort_fn)(void);

/* D-049: albums whose MOONLIT_ART_CACHE_SIZE .pfraw for the active
 * theme is missing or stale AND have no "<clave>.none" marker (D-056)
 * -- header reads only (one open() per album, no pixels). 0 means
 * moonlit_art_precache() has nothing to do and the "Preparando
 * biblioteca" screen never draws its phase 2. D-056: the answer is
 * memoized per (tagcache total_entries, theme, generation) whatever
 * its value; moonlit_art_pending_invalidate() bumps the generation
 * (sync finish_ok() via moonlit_art_request_gc(), the bootstrap seal
 * in metro_music_db_ready(), an aborted precache). */
int moonlit_art_pending_count(void);
void moonlit_art_pending_invalidate(void);

/* D-224/D-049: recorre metro_music_albums() una vez y llama
 * moonlit_art_load_for_album() SOLO por los álbumes sin .pfraw válido
 * (moonlit_art_pfraw_is_cached() antes, cabecera de 16 bytes, nunca el
 * read() completo del hit -- 264 ms/álbum medidos en el iPod del
 * dueño con el pase anterior, 4 min 18 s para 979 álbumes). `progress_cb`
 * recibe (hechos, pendientes) tras cada decode; `should_abort` se
 * consulta entre álbumes (nunca a mitad de un decode) y, si devuelve
 * true, la pasada se corta y esto devuelve false -- lo que falte queda
 * para la próxima llamada, idempotente. Ambos opcionales (NULL).
 * Único llamador: moonlit_screen_library.c (D-049); ya no corre dentro
 * de metro_music_db_ready(), que bloqueaba el hub sin pantalla ni
 * botones. */
bool moonlit_art_precache(moonlit_art_progress_fn progress_cb,
                          moonlit_art_abort_fn should_abort);

#endif /* MOONLIT_ART_CACHE_H */
