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
/* Marea (D-029, D-030, M8): pantalla completa de Cover Flow vertical --
 * portadas en columna a la izquierda (x en [0,152)), información del
 * álbum enfocado a la derecha (x en [160,312), D-030). Motor de
 * proyección: moonlit_flow.h (D-014/D-041, ya portado y probado en
 * host por M6). Carátulas: moonlit_art.h/moonlit_art_cache.h (D-020/
 * D-042, ya cacheadas en .pfraw por M7).
 *
 * Mismo patrón de página centinela que Now Playing/el visor de fotos
 * (metro_screen_nowplaying.h, metro_screen_photo_viewer.h): un
 * metro_page real y vacío se empuja a la pila de navegación para que
 * la contabilidad de profundidad/pop de metro_screen_list.c siga
 * siendo genérica; metro_main.c despacha dibujo/entrada aquí en vez de
 * a la lista genérica mientras esta página esté en la cima.
 *
 * Experimental hasta la medición en hardware real de M12 (ver
 * DECISIONS.md) -- MOONLIT_FLOW_CAM_DIST sigue siendo una HIPÓTESIS
 * (D-041) sin retunear todavía contra el dispositivo. */
#ifndef MOONLIT_SCREEN_MAREA_H
#define MOONLIT_SCREEN_MAREA_H

#include <stdbool.h>

/* Empuja el centinela de Marea. Mismo contrato false-si-la-pila-está-
 * llena que metro_screen_list_push(). Refresca su snapshot de álbumes
 * (metro_screen_hub_albums()) en cada push -- no hace falta llamarlo
 * de nuevo entre visitas. */
bool moonlit_screen_marea_push(void);

/* True mientras la página en la cima de metro_screen_list es el
 * centinela de Marea -- metro_main.c lo usa para despachar dibujo/
 * entrada aquí en vez de a metro_screen_list, igual que
 * metro_screen_nowplaying_is_current()/metro_screen_photo_viewer_is_current(). */
bool moonlit_screen_marea_is_current(void);

void moonlit_screen_marea_show(void);
void moonlit_screen_marea_handle(int action, int steps);

/* Presupuesto de UN decode por llamada (mismo patrón que
 * metro_thumbs_tick(), DD-9): get_slot_for() nunca decodifica un JPEG
 * dentro de show() (regla dura de D-030/M8) -- un cache-miss cae al
 * monograma y encola el álbum; metro_main.c llama esto desde su rama
 * ociosa (MACT_NONE) mientras Marea es la pantalla actual, igual que
 * ya hace con el motor de miniaturas. Devuelve true si decodificó algo
 * (el llamador debe redibujar), false si no había nada pendiente. */
bool moonlit_screen_marea_tick(void);

#endif /* MOONLIT_SCREEN_MAREA_H */
