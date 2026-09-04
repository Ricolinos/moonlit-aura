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

/* Pantalla completa (clear + cabecera + banda + panel + lcd_update()).
 * La llama redraw_current() al entrar y al cambiar tema/idioma, y
 * moonlit_screen_marea_show_carousel() en el cuadro de asentamiento. */
void moonlit_screen_marea_show(void);
void moonlit_screen_marea_handle(int action, int steps);

/* D-053 (modelo Music Flow de Aura-Firmware, aura_musicflow.c): el
 * scroll no bloquea. moonlit_screen_marea_handle(MACT_NEXT/PREV) solo
 * fija un destino y regresa; la posición visual es una función del
 * reloj (220 ms, out_expo, retarget desde la posición actual).
 *
 * animating(): true desde que se pidió un destino hasta que se dibujó
 * el cuadro de asentamiento -- metro_main.c sondea a HZ/20 mientras
 * tanto (patrón metro_screen_hub_wants_ticks()) y llama
 * show_carousel() en cada vuelta ociosa.
 *
 * show_carousel(): un cuadro -- repinta SOLO la banda izquierda
 * (0,20,152,220) con lcd_update_rect() bajo cpu_boost(); cuando la
 * posición alcanza el destino por primera vez cae a
 * moonlit_screen_marea_show() completa (panel + cabecera + conteo de
 * canciones). Ya asentada, repinta la banda sola (tapa recién cargada
 * por tick()). Nunca lee disco. */
bool moonlit_screen_marea_animating(void);
void moonlit_screen_marea_show_carousel(void);

/* D-057 (item 1): show_carousel() ahora se permite, mientras anima, a
 * lo sumo UNA lectura de disco PLANA por cuadro (un .pfraw cuya clave
 * ya se conocia de antes -- jamás decode JPEG ni tagcache, ver
 * try_frame_bounded_read() en el .c) -- el resto de la regla dura de
 * D-053 sigue igual: get_slot_for() nunca decodifica ni consulta
 * tagcache dentro de un cuadro.
 *
 * D-057 (item 2): moonlit_screen_marea_tick() ahora tiene presupuesto
 * (~15 ms medidos con current_tick, o hasta 4 lecturas) en vez de una
 * sola carga por vuelta ociosa -- sigue siendo la única función de
 * este módulo que decodifica JPEG (mismo patrón que
 * metro_thumbs_tick(), DD-9): un cache-miss reclama el slot con el
 * monograma y lo deja pendiente; esto lee el .pfraw (o decodifica la
 * carátula si falta) de los slots pendientes más cercanos al destino,
 * o precarga los álbumes más cercanos sin slot dentro del radio de
 * precarga direccional (moonlit_marea_prefetch_order()). metro_main.c
 * lo llama desde su rama ociosa (MACT_NONE) mientras Marea es la
 * pantalla actual y NO está animando. Devuelve true si cargó al menos
 * una tapa (el llamador repinta la banda), false si no había nada
 * pendiente. */
bool moonlit_screen_marea_tick(void);

/* D-057 (item 2): true mientras quede algo por cargar en la ventana
 * VISIBLE del destino actual (patrón metro_screen_hub_wants_ticks()) --
 * metro_main.c usa esto para pedir cuadros a HZ/20 en vez de HZ/10
 * también cuando Marea está asentada pero todavía tiene tapas
 * pendientes, no solo mientras moonlit_screen_marea_animating(). */
bool moonlit_screen_marea_wants_ticks(void);

/* D-078: un cuadro de marquesina del panel (titulo/subtitulo), asentado
 * -- repinta SOLO el panel derecho (lcd_update_rect(), nunca la banda ni
 * la cabecera). No lee disco ni decodifica nada. metro_main.c la llama
 * desde su rama ociosa cuando moonlit_marquee_wants_ticks() es cierto y
 * ni moonlit_screen_marea_animating() ni moonlit_screen_marea_tick() ya
 * cubrieron ese cuadro. */
void moonlit_screen_marea_show_panel(void);

#endif /* MOONLIT_SCREEN_MAREA_H */
