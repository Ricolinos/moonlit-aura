#!/usr/bin/env python3
"""Genera apps/metro/metro_turnstile_table.c -- la tabla de proyeccion
del turnstile (F12), precalculada offline porque el target no tiene
FPU (INVESTIGACION.md B.5/B.6, mismo motivo que metro_motion.c's tabla
de easing).

Modela una rotacion RIGIDA de toda la pantalla alrededor de su propio
CENTRO VERTICAL (x = LCD_WIDTH/2), vista por una camara de perspectiva
simple a distancia focal D. Tanto el eje de rotacion como D son
decision de diseno visual (PLAN_MAESTRO.md F.3 solo confirma el rango
de angulo/duracion del turnstile de WP7, no la geometria de
proyeccion exacta) -- [ESTIMADO], documentado en DECISIONS.md.

Para cada (angulo, columna de PANTALLA), resuelve hacia atras
(backward-mapping, sin huecos) que columna de la imagen FUENTE
corresponde y con que escala vertical -- el dispositivo solo hace un
lookup + acumulador de paso fijo por columna (PictureFlow's tecnica
"sin division por pixel", B.5), toda la trigonometria ya esta resuelta
aqui.

Uso:
    python3 firmware/tools/gen_turnstile_table.py
"""
import math
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent.parent
OUT_PATH = ROOT_DIR / "firmware/rockbox/apps/metro/metro_turnstile_table.c"

LCD_WIDTH = 320
FOCAL_DISTANCE = LCD_WIDTH * 1.2  # [ESTIMADO], eleccion de diseno

ANGLE_MIN = -80.0
ANGLE_MAX = 50.0
N_ANGLES = 32

STEP_FIXED_SHIFT = 8  # punto fijo 8.8 -- ver metro_turnstile_table.h


def compute_row(angle_deg):
    theta = math.radians(angle_deg)
    cos_t = math.cos(theta)
    sin_t = math.sin(theta)
    xs_row = []
    step_row = []
    for x_dst in range(LCD_WIDTH):
        u_screen = x_dst - LCD_WIDTH / 2.0
        denom = cos_t * FOCAL_DISTANCE - u_screen * sin_t
        valid = denom > 1.0
        if valid:
            u = u_screen * FOCAL_DISTANCE / denom
            xs = u + LCD_WIDTH / 2.0
            scale = FOCAL_DISTANCE / (FOCAL_DISTANCE + u * sin_t)
            if scale <= 0.02 or xs < -LCD_WIDTH or xs > 2 * LCD_WIDTH:
                valid = False
        if not valid:
            xs_row.append(-1)  # sin fuente valida -- el llamador pinta bg
            step_row.append(1 << STEP_FIXED_SHIFT)
            continue
        xs_i = min(LCD_WIDTH - 1, max(0, int(round(xs))))
        scale = min(scale, 8.0)
        step_fixed = int(round((1.0 / scale) * (1 << STEP_FIXED_SHIFT)))
        step_fixed = min(65535, max(1, step_fixed))
        xs_row.append(xs_i)
        step_row.append(step_fixed)
    return xs_row, step_row


def main():
    xs_table, step_table = [], []
    for i in range(N_ANGLES):
        angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * i / (N_ANGLES - 1)
        xs_row, step_row = compute_row(angle)
        xs_table.append(xs_row)
        step_table.append(step_row)

    lines = ["""/***************************************************************************
 *             __________               __   ___.
 *   Open      \\______   \\ ____   ____ |  | _\\_ |__   _______  ___
 *   Source     |       _//  _ \\_/ ___\\|  |/ /| __ \\ /  _ \\  \\/  /
 *   Jukebox    |    |   (  <_> )  \\___|    < | \\_\\ (  <_> > <  <
 *   Firmware   |____|_  /\\____/ \\___  >__|_ \\|___  /\\____/__/\\_ \\
 *                     \\/            \\/     \\/    \\/            \\/
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
/* F12: turnstile projection table -- GENERATED, do not hand-edit.
 * Regenerate with firmware/tools/gen_turnstile_table.py (DECISIONS.md
 * M-046). Never trig at runtime (no FPU on this target,
 * INVESTIGACION.md B.5/B.6). METRO_TURNSTILE_ANGLES angles spanning
 * [METRO_TURNSTILE_ANGLE_MIN, METRO_TURNSTILE_ANGLE_MAX] degrees,
 * LCD_WIDTH columns each -- a rigid rotation of the whole screen
 * around its own vertical center under a simple perspective camera,
 * both the rotation axis and the focal distance a visual design
 * choice ([ESTIMADO], see the generator script's own docstring).
 *
 * Per (angle index, destination column x): metro_turnstile_xs[i][x] is
 * the SOURCE column to read from (-1 = no valid source at this angle/
 * column -- the rotated surface doesn't cover this part of the screen
 * at all, caller fills with the background color instead).
 * metro_turnstile_step[i][x] is the fixed-point 8.8 vertical sampling
 * step (source rows per destination row) -- a plain fixed-point
 * accumulator (p += step; src_row = p >> 8) walks the source column
 * top to bottom, PictureFlow's own "no per-pixel division" technique
 * (INVESTIGACION.md B.5).
 */
#include "metro_turnstile_table.h"

const short metro_turnstile_xs[METRO_TURNSTILE_ANGLES][LCD_WIDTH] = {
"""]
    for row in xs_table:
        lines.append("    { " + ", ".join(str(v) for v in row) + " },\n")
    lines.append("};\n\n")
    lines.append("const unsigned short metro_turnstile_step[METRO_TURNSTILE_ANGLES][LCD_WIDTH] = {\n")
    for row in step_table:
        lines.append("    { " + ", ".join(str(v) for v in row) + " },\n")
    lines.append("};\n")

    OUT_PATH.write_text("".join(lines))
    print(f"==> Escrito {OUT_PATH} ({OUT_PATH.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
