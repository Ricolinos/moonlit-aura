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
#ifndef MOONLIT_ELEVATION_H
#define MOONLIT_ELEVATION_H

#include "moonlit_palette.h"

/* Elevacion tonal MD3 (D-012, M4): dibuja una tarjeta de (w x h) en
 * (x, y), rellena con el tono de `level` (surface_container_<nivel>,
 * D-028), esquinas redondeadas a `radius` px por cobertura (antialias,
 * misma tecnica que metro_widgets_draw_circle) y un borde de 1px --
 * mas claro arriba/izquierda, mas oscuro abajo/derecha -- luz desde
 * arriba-izquierda, nunca blur ni sombra difusa (identidad Waning
 * Crescent). Cero aritmetica de color por cuadro: los tonos de borde
 * ya vienen precalculados en moonlit_tokens.h.
 *
 * `radius` se recorta a la mitad del lado mas chico; 0 = esquinas
 * rectas (solo el borde). El color de "afuera" de la esquina redondeada
 * es siempre moonlit_color(MROLE_SURFACE) -- el fondo plano de pantalla
 * que metro_draw_clear() ya pinto detras de cualquier tarjeta. */
void moonlit_draw_surface(int x, int y, int w, int h,
                           enum moonlit_surface_level level, int radius);

#endif /* MOONLIT_ELEVATION_H */
