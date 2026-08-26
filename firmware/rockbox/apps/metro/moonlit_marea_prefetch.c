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
/* D-057: ver moonlit_marea_prefetch.h. */
#include "moonlit_marea_prefetch.h"

int moonlit_marea_prefetch_order(int target, int album_count, int dir,
                                  int fwd_radius, int back_radius,
                                  int *out, int out_cap)
{
    int n = 0, d, max_radius;

    if (out_cap <= 0 || album_count <= 0)
        return 0;
    if (dir == 0)
        dir = 1;
    if (fwd_radius < 0)
        fwd_radius = 0;
    if (back_radius < 0)
        back_radius = 0;

    if (target >= 0 && target < album_count && n < out_cap)
        out[n++] = target;

    max_radius = (fwd_radius > back_radius) ? fwd_radius : back_radius;
    for (d = 1; d <= max_radius; d++)
    {
        if (d <= fwd_radius && n < out_cap)
        {
            int idx = target + dir * d;
            if (idx >= 0 && idx < album_count)
                out[n++] = idx;
        }
        if (d <= back_radius && n < out_cap)
        {
            int idx = target - dir * d;
            if (idx >= 0 && idx < album_count)
                out[n++] = idx;
        }
    }
    return n;
}
