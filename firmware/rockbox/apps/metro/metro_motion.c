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
#include "metro_motion.h"

/* f(t) = 1 - (e^(6*(1-t)) - 1)/(e^6 - 1), t = i/15 for i = 0..15 --
 * WP7's ExponentialEase(Exponent=6, EaseOut), sampled offline in
 * Python and rounded to fixed point *256 (INVESTIGACION.md F.3). */
static const unsigned short ease_out_expo_table[16] = {
    0, 85, 141, 179, 205, 222, 233, 241, 246, 250, 252, 253, 255, 255, 256, 256
};

static int ease_out_expo(int i, int frames)
{
    int pos, idx, frac, a, b;

    pos = i * 15 * 256 / frames; /* 0..15*256, table index * 256 + fraction */
    idx = pos / 256;
    if (idx >= 15)
        return 256;
    frac = pos % 256;

    a = ease_out_expo_table[idx];
    b = ease_out_expo_table[idx + 1];
    return a + ((b - a) * frac) / 256;
}

static int ease_out_quad(int i, int frames)
{
    int t = i * 256 / frames;        /* 0..256 */
    int remain = 256 - t;

    /* 1 - (1-t)^2, scaled to 0..256 */
    return 256 - (remain * remain) / 256;
}

static int ease_linear(int i, int frames)
{
    return i * 256 / frames;
}

int metro_ease(enum metro_ease_kind kind, int i, int frames)
{
    if (frames <= 0 || i >= frames)
        return 256;
    if (i <= 0)
        return 0;

    switch (kind)
    {
        case METRO_EASE_OUT_QUAD:
            return ease_out_quad(i, frames);
        case METRO_EASE_OUT_EXPO:
            return ease_out_expo(i, frames);
        default:
            return ease_linear(i, frames);
    }
}

long metro_seek_step_ms(int run)
{
    long step;

    if (run < 0)
        run = 0;

    step = (long)METRO_SEEK_STEP_MIN_MS * (1 + run / METRO_SEEK_RAMP_EVERY);
    return step > METRO_SEEK_STEP_MAX_MS ? METRO_SEEK_STEP_MAX_MS : step;
}
