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

/* moonlit (D-065): igual, pero contra un buffer cuadrado de `size` x
 * `size` (stride = size) en vez de uno del tamano de la pantalla, con
 * el viewport acotado a esa caja. Para rasterizar una tapa de Marea
 * fuera de pantalla: el consumidor de la tapa no tiene por que
 * distinguir si lleva una caratula real o un monograma. */
void metro_fb_render_tile(fb_data *dst, int size, metro_fb_draw_fn draw_fn);

/* moonlit (D-052 C1/C3): composes a horizontal slide of `from`/`to` at
 * a signed pixel offset dx directly into the real LCD, WITHOUT calling
 * lcd_update() -- the transition loop paints CONTINUUM's flying title
 * on top and updates once per frame. dx > 0: `to` enters from the
 * right, `from` exits to the left (pivot-next twist, POP). dx < 0: `to`
 * enters from the left, `from` exits to the right (pivot-prev twist,
 * PUSH -- "light from the left", D-012). Clamped to [-LCD_WIDTH,
 * LCD_WIDTH]. seam_color: "Filo de luna" (C3) -- a 1px lcd_vline() the
 * full height of the screen at the seam column (where the right-hand
 * layer starts), or METRO_FB_NO_SEAM for none; never drawn when the
 * seam would fall on a screen edge (|dx| == 0 or LCD_WIDTH). Cost per
 * frame: LCD_WIDTH*LCD_HEIGHT pixels of straight row copies + 240 px. */
#define METRO_FB_NO_SEAM (-1L)
void metro_fb_compose_slide(const fb_data *from, const fb_data *to, int dx,
                            long seam_color);

/* metro_fb_compose_slide() followed by lcd_update() -- for a caller
 * with nothing to paint on top. */
void metro_fb_present_slide(const fb_data *from, const fb_data *to, int dx,
                            long seam_color);

/* Cross-fades from `from` to `to` at alpha256 (0 = from, 256 = to,
 * clamped) directly into the real LCD and calls lcd_update() -- per-
 * pixel RGB blend, ~77k pixels/frame (PLAN_MAESTRO.md B.4). Reserved
 * for `graphics=full` (metro_transitions.c's own gate; this function
 * doesn't check settings itself). */
void metro_fb_present_fade(const fb_data *from, const fb_data *to, int alpha256);

/* Blends `img` (a full LCD_WIDTH*LCD_HEIGHT frame) over a SOLID
 * `bg_color` at alpha256 directly into the real LCD -- same per-pixel
 * blend as metro_fb_present_fade(), without needing a second full-size
 * buffer just to hold a flat color (F12's dimmed album-art background
 * in Now Playing). Unlike every other metro_fb_present_*()/blend
 * function, does NOT call lcd_update(): this one is meant as a static
 * BACKGROUND with more drawing on top of it before the caller updates
 * once, not a transition's only compositing step per frame. */
void metro_fb_blend_over_color(const fb_data *img, unsigned bg_color, int alpha256);

/* R5-F3 (M-083): the same per-channel blend the fades use, for ONE
 * color -- `from` at alpha256 = 0, `to` at 256. For fading a piece of
 * text toward the background (the volume level overlay in Now Playing,
 * the paused-title breathing in the hub) without touching any frame
 * buffer: the caller just draws the text in the returned color. */
unsigned metro_fb_blend_color(unsigned from, unsigned to, int alpha256);

/* R5 (M-086): plots ONE pixel of `color` at alpha256 over whatever is
 * already in the CURRENT viewport's buffer (the real LCD, or an
 * offscreen metro_fb_render() frame) -- read, blend, write. Out-of-
 * screen coordinates are ignored. This is the primitive the
 * anti-aliased ring uses; it is NOT for area fills (per-pixel
 * read-modify-write is the slow path on purpose). */
void metro_fb_plot_alpha(int x, int y, unsigned color, int alpha256);

/* R3-F8/DD-9 (M-069): rellena un rectángulo de color DENTRO de un
 * buffer off-screen (no en el LCD real) -- el único caso hasta ahora es
 * CONTINUUM borrando la ceja de la página de destino en `s_fb_to`
 * antes de animarla, para que no se vea a la vez en su sitio y viajando
 * encima. Recorta a los límites del buffer, así que un rectángulo que
 * se pase de ancho (una ceja larga cerca del borde) no escribe fuera.
 * Deliberadamente NO es `lcd_fillrect()` con `metro_fb_render()`
 * alrededor: eso redirige TODO el dibujo y está pensado para pintar una
 * pantalla entera, no para tocar unos pixeles de una que ya está
 * compuesta. */
void metro_fb_fill_rect(fb_data *dst, int x, int y, int w, int h, unsigned color);

#endif /* METRO_FB_H */
