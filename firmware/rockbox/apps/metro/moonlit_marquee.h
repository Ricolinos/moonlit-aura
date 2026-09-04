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
/* moonlit (D-067): marquesina de texto largo. Referencia del maestro
 * SS G (aura_patterns.c:32-49 + aura_marquee.c:29-70, leidos de
 * ../Aura-Firmware, solo lectura): si el texto cabe, estatico; si no,
 * MOONLIT_MOTION_MARQUEE_STATIC_MS quieto, luego
 * MOONLIT_MOTION_MARQUEE_SCROLL_MS desplazando de derecha a izquierda
 * de forma LINEAL, con MOONLIT_MOTION_MARQUEE_LOOP_GAP_PX de hueco
 * entre copias, en bucle. Se dibujan DOS copias del texto para que el
 * bucle no tenga costura: cuando la primera termina de salir, la
 * segunda esta exactamente donde arranco la primera.
 *
 * Puerta de energia (regla del repo, D-030/D-052): solo pide cuadros
 * mientras HAY un texto visible que desborda, `lcd_active()` es cierto
 * y `metro_settings.animations != METRO_ANIM_OFF`. Con las animaciones
 * apagadas se corta a la derecha como siempre -- ni un tick de mas.
 *
 * El calculo del desplazamiento es PURO y esta probado en host
 * (apps/metro/test/test_marquee.c); lo unico que toca el LCD es
 * moonlit_marquee_draw(). */
#ifndef MOONLIT_MARQUEE_H
#define MOONLIT_MARQUEE_H

#include <stdbool.h>

#include "moonlit_fonts.h"

/* Un sitio de la UI con marquesina. El estado (cuando arranco el ciclo,
 * que texto era) vive en este modulo indexado por ranura, para que las
 * pantallas no tengan que arrastrarlo. Una ranura por SITIO, no por
 * fila: solo la fila seleccionada desplaza, y solo hay una. */
enum moonlit_marquee_slot {
    MOONLIT_MARQUEE_ROW = 0,   /* titulo de la fila seleccionada de una lista */
    MOONLIT_MARQUEE_TILE,      /* rotulo del tile seleccionado */
    MOONLIT_MARQUEE_NP_TITLE,
    MOONLIT_MARQUEE_NP_ARTIST,
    MOONLIT_MARQUEE_NP_ALBUM,
    MOONLIT_MARQUEE_MAREA,     /* titulo del panel de Marea */
    MOONLIT_MARQUEE_MAREA_SUBTITLE, /* moonlit (D-078): artista/album del panel de Marea */
    MOONLIT_MARQUEE_ABOUT,     /* fila seleccionada de "Acerca de" */
    MOONLIT_MARQUEE_HEADER,    /* moonlit (D-076): titulo de la ceja */
    MOONLIT_MARQUEE_COUNT
};

/* Desplazamiento en pixeles dentro del ciclo. PURA. `span_px` es el
 * ancho de un ciclo completo (ancho del texto + hueco): al final del
 * tramo de scroll el desplazamiento vale exactamente `span_px`, que es
 * la misma imagen que 0 gracias a la segunda copia. Un `span_px` <= 0
 * o duraciones no positivas devuelven 0 (nada que desplazar). */
int moonlit_marquee_offset_px(long elapsed_ms, int span_px,
                              int static_ms, int scroll_ms);

/* Dibuja `text` en la banda [clip_x, clip_x+clip_w) a la altura `y`.
 * Si cabe, o si las animaciones estan apagadas o el LCD dormido, cae en
 * metro_draw_text_cut_right() -- exactamente el comportamiento de
 * antes. Si no cabe y puede animar, desplaza.
 *
 * Devuelve true si esta desplazando (el llamador debe seguir pidiendo
 * cuadros). El reloj de cada ranura se reinicia solo cuando cambia el
 * texto: pasar de una fila a otra empieza de nuevo por el tramo quieto,
 * que es lo que se espera al mover la seleccion. */
bool moonlit_marquee_draw(enum moonlit_marquee_slot slot,
                          enum metro_font_role role, int clip_x, int clip_w,
                          int y, const char *text, unsigned color);

/* moonlit (D-078): igual que moonlit_marquee_draw(), pero el ciclo de
 * ESTA ranura arranca `phase_ms` adelantado -- el reloj de la ranura
 * (`since`) no se toca, solo se suma al tiempo transcurrido antes de
 * calcular el desplazamiento, asi que dos ranuras con el mismo `since`
 * (arrancaron juntas, ej. el panel de Marea) quedan en puntos distintos
 * del ciclo sin coordinarse entre si. Panel de Marea: el subtitulo pasa
 * MOONLIT_MOTION_MARQUEE_STATIC_MS + MOONLIT_MOTION_MARQUEE_SCROLL_MS
 * (el ciclo completo) entre 2 para que su tramo de barrido no compita
 * con el del titulo (maestro SS E.3). moonlit_marquee_draw() es
 * exactamente esta funcion con phase_ms=0. */
bool moonlit_marquee_draw_offset(enum moonlit_marquee_slot slot,
                                 enum metro_font_role role, int clip_x, int clip_w,
                                 int y, const char *text, unsigned color,
                                 long phase_ms);

/* true si ALGUNA ranura quedo desplazando en el ultimo dibujo -- la
 * puerta que metro_main.c consulta para bajar su espera a HZ/20 y
 * repintar. Se apaga sola: cada ranura marca su estado en cada
 * moonlit_marquee_draw(), y una pantalla que deja de dibujar una
 * ranura la deja marcada como quieta via moonlit_marquee_reset(). */
bool moonlit_marquee_wants_ticks(void);

/* Olvida el estado de todas las ranuras (cambio de pantalla, de tema o
 * de idioma). Sin esto, entrar a otra lista heredaria el reloj a mitad
 * de ciclo de la anterior. */
void moonlit_marquee_reset(void);

#endif /* MOONLIT_MARQUEE_H */
