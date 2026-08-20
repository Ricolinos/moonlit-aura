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
/* F11: fixed-point easing, frame-index based (PLAN_MAESTRO.md S3.2:
 * "p = ease(i, frames)" in the canonical transition loop) -- pure C99,
 * no Rockbox dependency, no floats or transcendental math at runtime.
 * Compiles and runs host-side (apps/metro/test/test_motion.c), same
 * pattern as metro_nav.c/.h. ease_out_expo's curve (WP7 ExponentialEase,
 * Exponent=6, EaseOut -- INVESTIGACION.md F.3) was computed OFFLINE
 * with a floating-point script and hardcoded as a 16-entry table;
 * metro_ease() only does integer lookup + linear interpolation. */
#ifndef METRO_MOTION_H
#define METRO_MOTION_H

enum metro_ease_kind {
    METRO_EASE_LINEAR = 0,
    METRO_EASE_OUT_QUAD,
    METRO_EASE_OUT_EXPO,
};

/* Returns eased progress in [0, 256] for frame `i` of `frames` total
 * (i in [0, frames]: i<=0 -> 0, i>=frames -> 256). `frames` is
 * whatever the caller's current FX level uses (8 for `all`, 4 for
 * `minimal`) -- the curve shape doesn't depend on it, only where each
 * frame lands along it. */
int metro_ease(enum metro_ease_kind kind, int i, int frames);

/* R4/FA-9 (M-072): rampa del salto de búsqueda (seek) al sostener
 * LEFT/RIGHT en Now Playing. Antes era un paso FIJO de 5000 ms por
 * evento de repetición, y eso -- no el refresco del LCD ni la lógica de
 * "hold" -- era la causa de que el recorrido se sintiera a brincos.
 *
 * Vive aquí, y no en la pantalla, por dos razones: es la misma familia
 * que metro_ease() (una curva de "cuánto avanzo por paso"), y este
 * módulo es C99 puro sin dependencias de Rockbox, así que la rampa
 * queda cubierta por el arnés de host (test_motion.c) en vez de
 * depender de sostener un botón en el simulador -- que el inyector de
 * botones ni siquiera puede simular (hace press-release corto, nunca
 * llega a BUTTON_REPEAT).
 *
 * Deliberadamente NO se reusa button_apply_acceleration(): esa función
 * es específica de la rueda, lee la velocidad del wheel del bit 31 de
 * get_action_data() (firmware/drivers/button.c:632-659), y un
 * BUTTON_LEFT|BUTTON_REPEAT no trae ese dato -- devolvería 0. */
#define METRO_SEEK_STEP_MIN_MS   1000
#define METRO_SEEK_STEP_MAX_MS  10000
#define METRO_SEEK_RAMP_EVERY       4

/* `run` = cuántos eventos de búsqueda consecutivos van ya en esta
 * racha (0 = el primero). Empieza en METRO_SEEK_STEP_MIN_MS y duplica
 * cada METRO_SEEK_RAMP_EVERY eventos, con tope en
 * METRO_SEEK_STEP_MAX_MS. Un `run` negativo se trata como 0. */
long metro_seek_step_ms(int run);

#endif /* METRO_MOTION_H */
