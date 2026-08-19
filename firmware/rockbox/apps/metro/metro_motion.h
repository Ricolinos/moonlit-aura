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

#endif /* METRO_MOTION_H */
