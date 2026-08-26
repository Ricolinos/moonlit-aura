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
/* moonlit: derived from aura_wheel.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-019, D-041). */
#include "moonlit_wheel.h"

int moonlit_wheel_step(int velocity_deg_s)
{
    long v, thr, scaled;

    if (velocity_deg_s <= 0)
        return 1;

    thr = MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S;
    v = velocity_deg_s;
    if (v > thr)
        v = thr; /* mas alla del umbral es el modo de hojeo, no mas paso */

    /* step = 1 + round(2 * (v/thr)^2), en enteros: redondeo por +mitad
     * del divisor antes de la division. */
    scaled = 1 + (2 * v * v + (thr * thr) / 2) / (thr * thr);

    if (scaled > 3)
        scaled = 3;
    if (scaled < 1)
        scaled = 1;
    return (int)scaled;
}

int moonlit_wheel_should_hop_letters(int velocity_deg_s)
{
    return velocity_deg_s > MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S;
}
