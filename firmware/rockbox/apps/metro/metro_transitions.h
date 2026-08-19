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
/* F11: the transition engine (PLAN_MAESTRO.md S3.2/S3.3) -- metro_main.c
 * is the only caller. It diffs metro_nav_t state before/after an
 * action to decide WHICH of these to call (depth up = push, depth
 * down = pop, pivot changed = twist, entering/leaving Now Playing =
 * fade); this module doesn't know about pages, pivots or Now Playing
 * at all, only "slide toward a signed direction" and "fade to a new
 * frame", each given a draw_to() callback that renders the destination
 * screen (one of Metro's normal *_show() functions). */
#ifndef METRO_TRANSITIONS_H
#define METRO_TRANSITIONS_H

#include "metro_fb.h"

typedef metro_fb_draw_fn metro_transitions_draw_fn;

/* SLIDE (pivot twist) and PUSH/POP (both v1: present_slide,
 * PLAN_MAESTRO.md S3.3) share this one entry point -- direction > 0:
 * draw_to enters from the right (pivot-next, push). direction < 0:
 * draw_to enters from the left (pivot-prev, pop). Falls straight
 * through to draw_to() with no animation at all when
 * animations=off, !lcd_active(), or after auto-degradation
 * (M-015) has dropped the effective level to off. */
void metro_transitions_slide(metro_transitions_draw_fn draw_to, int direction);

/* FADE: entering/leaving Now Playing, returning from a plugin
 * (PLAN_MAESTRO.md S3.3). Only actually cross-fades (metro_fb.c's
 * present_fade, per-pixel blend) when animations=all AND
 * graphics=full; animations=minimal or graphics=lite both fall back
 * to metro_transitions_slide(draw_to, 1) -- present_fade is reserved
 * to graphics=full (metro_fb.h), and `minimal` "usa SLIDE" for every
 * transition per the plan's own table, this one included. */
void metro_transitions_fade(metro_transitions_draw_fn draw_to);

#endif /* METRO_TRANSITIONS_H */
