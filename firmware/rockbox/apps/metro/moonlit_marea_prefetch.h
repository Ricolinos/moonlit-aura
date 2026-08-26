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
/* D-057: orden de precarga direccional de moonlit_screen_marea.c,
 * separado a proposito en un modulo puro (mismo criterio que
 * moonlit_wheel.c/moonlit_flow.c: sin dependencias de Rockbox, compila
 * y se prueba igual en el host que en el firmware) -- ver
 * apps/metro/test/test_marea_prefetch.c.
 *
 * Diagnostico D-057 (ver DECISIONS.md): aura_musicflow.c precarga el
 * vecindario del destino de forma pareja (MF_VISIBLE_RADIUS+15 a cada
 * lado). moonlit prioriza en cambio la direccion en la que el usuario
 * ya venia girando la rueda -- un barrido largo tipicamente sigue en la
 * misma direccion, asi que vale la pena adelantarse mas ahi (10 tapas)
 * que en la contraria (4), sin gastar mas slots de cache de los que
 * moonlit_screen_marea.c ya reserva (MAREA_CACHE_SLOTS = 37, cubre de
 * sobra las <= 15 que este modulo puede pedir por llamada). */
#ifndef MOONLIT_MAREA_PREFETCH_H
#define MOONLIT_MAREA_PREFETCH_H

/* Llena `out` (capacidad `out_cap`) con indices de album a precargar,
 * en orden de prioridad: `target` primero, luego alternando
 * target+dir*d / target-dir*d para d=1,2,... -- el lado `dir` (+1/-1,
 * 0 se trata como +1) llega hasta `fwd_radius`, el lado opuesto hasta
 * `back_radius`. Indices fuera de [0, album_count) se omiten sin
 * ocupar un hueco de `out`. Nunca repite un indice. Devuelve cuantos
 * escribio (<= out_cap y <= 1 + fwd_radius + back_radius). Funcion
 * pura, sin E/S -- el llamador (moonlit_screen_marea.c) filtra el
 * resultado contra su propia cache de slots antes de tocar disco. */
int moonlit_marea_prefetch_order(int target, int album_count, int dir,
                                  int fwd_radius, int back_radius,
                                  int *out, int out_cap);

#endif /* MOONLIT_MAREA_PREFETCH_H */
