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
/* F11: raw framebuffer capture/render/composite primitives
 * (PLAN_MAESTRO.md S3.1) -- the only module in apps/metro/ that reads
 * lcd_framebuffer_default or writes pixels directly instead of going
 * through metro_draw.c's shell. metro_transitions.c is the only
 * caller; nothing here knows what a "screen" or a "page" is. */
#ifndef METRO_FB_H
#define METRO_FB_H

#include "lcd.h"

typedef void (*metro_fb_draw_fn)(void);

/* Copies the real LCD's current pixels into dst (LCD_WIDTH*LCD_HEIGHT
 * fb_data, caller-owned). */
void metro_fb_capture(fb_data *dst);

/* Redirects all lcd_* drawing into dst for the duration of draw_fn(),
 * then restores drawing to the real screen -- draw_fn() is one of
 * Metro's normal *_show() functions, called exactly once, fully
 * off-screen (no lcd_update(), the real LCD is untouched). */
void metro_fb_render(fb_data *dst, metro_fb_draw_fn draw_fn);

/* Composites a horizontal slide of `from`/`to` at a signed pixel
 * offset dx directly into the real LCD and calls lcd_update().
 * dx > 0: `to` enters from the right, `from` exits to the left (PUSH,
 * pivot-next). dx < 0: `to` enters from the left, `from` exits to the
 * right (POP, pivot-prev). Clamped to [-LCD_WIDTH, LCD_WIDTH]. */
void metro_fb_present_slide(const fb_data *from, const fb_data *to, int dx);

/* Cross-fades from `from` to `to` at alpha256 (0 = from, 256 = to,
 * clamped) directly into the real LCD and calls lcd_update() -- per-
 * pixel RGB blend, ~77k pixels/frame (PLAN_MAESTRO.md B.4). Reserved
 * for `graphics=full` (metro_transitions.c's own gate; this function
 * doesn't check settings itself). */
void metro_fb_present_fade(const fb_data *from, const fb_data *to, int alpha256);

/* F12: draws ONE rotated surface (`src`, a full LCD_WIDTH*LCD_HEIGHT
 * frame like `from`/`to` above) at metro_turnstile_table.h's angle
 * index `angle_index`, directly into the real LCD -- does NOT clear
 * first and does NOT call lcd_update(): the caller composes two of
 * these per frame (the outgoing screen, then the incoming one on top)
 * before updating once. A destination column with no valid
 * projection at this angle (metro_turnstile_xs[i][x] == -1) is left
 * untouched -- whatever the caller already painted there (the other
 * layer, or the background) shows through. */
void metro_fb_draw_turnstile_layer(const fb_data *src, int angle_index);

/* Blends `img` (a full LCD_WIDTH*LCD_HEIGHT frame) over a SOLID
 * `bg_color` at alpha256 directly into the real LCD -- same per-pixel
 * blend as metro_fb_present_fade(), without needing a second full-size
 * buffer just to hold a flat color (F12's dimmed album-art background
 * in Now Playing). Unlike every other metro_fb_present_*()/blend
 * function, does NOT call lcd_update(): this one is meant as a static
 * BACKGROUND with more drawing on top of it before the caller updates
 * once, not a transition's only compositing step per frame. */
void metro_fb_blend_over_color(const fb_data *img, unsigned bg_color, int alpha256);

#endif /* METRO_FB_H */
