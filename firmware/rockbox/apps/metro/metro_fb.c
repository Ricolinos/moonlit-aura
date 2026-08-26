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
#include <string.h>

#include "lcd.h"
#include "viewport.h"

#include "metro_fb.h"

/* The real LCD's own frame_buffer_t (firmware/drivers/lcd-color-common.c)
 * -- not declared in any header we can include, same local-extern
 * idiom Rockbox's own apps/gui/skin_engine/skin_backdrops.c uses to
 * read the screen's raw pixels for a backdrop. */
extern struct frame_buffer_t lcd_framebuffer_default;

#define METRO_FB_PIXELS (LCD_WIDTH * LCD_HEIGHT)

void metro_fb_capture(fb_data *dst)
{
    memcpy(dst, lcd_framebuffer_default.data, METRO_FB_PIXELS * sizeof(fb_data));
}

/* metro_fb_render()'s offscreen target -- module-static because
 * viewport_set_buffer()'s get_address_fn takes only (x, y), no buffer
 * context of its own (same constraint Aura-Firmware's aura_transitions.c
 * works around the same way for its own push_fb_address()). Safe: only
 * one render is ever in flight at a time (metro_transitions.c's loops
 * are synchronous, never reentrant). */
static fb_data *s_render_target;

static void *metro_fb_address(int x, int y)
{
    return &s_render_target[y * LCD_WIDTH + x];
}

void metro_fb_render(fb_data *dst, metro_fb_draw_fn draw_fn)
{
    struct frame_buffer_t fb;

    fb.fb_ptr = dst;
    fb.get_address_fn = &metro_fb_address;
    fb.stride = LCD_WIDTH;
    fb.elems = METRO_FB_PIXELS;

    s_render_target = dst;
    viewport_set_buffer(NULL, &fb, SCREEN_MAIN);
    draw_fn();
    viewport_set_buffer(NULL, NULL, SCREEN_MAIN);
}

/* moonlit (D-052 C1/C3): composes the slide WITHOUT updating -- the
 * caller (metro_transitions.c's slide loop) may still paint CONTINUUM's
 * flying title on top before its single lcd_update() per frame. The
 * seam is one lcd_vline() at the column where the right-hand layer
 * starts (x = LCD_WIDTH - |dx|), skipped when the seam would sit on a
 * screen edge (first/last frame: nothing to separate). */
void metro_fb_compose_slide(const fb_data *from, const fb_data *to, int dx,
                            long seam_color)
{
    int from_w, to_w, seam_x;

    if (dx > LCD_WIDTH)
        dx = LCD_WIDTH;
    if (dx < -LCD_WIDTH)
        dx = -LCD_WIDTH;

    if (dx >= 0)
    {
        from_w = LCD_WIDTH - dx;
        if (from_w > 0)
            lcd_bitmap_part(from, dx, 0, LCD_WIDTH, 0, 0, from_w, LCD_HEIGHT);
        if (dx > 0)
            lcd_bitmap_part(to, 0, 0, LCD_WIDTH, from_w, 0, dx, LCD_HEIGHT);
        seam_x = from_w;
    }
    else
    {
        to_w = -dx;
        from_w = LCD_WIDTH - to_w;
        lcd_bitmap_part(to, LCD_WIDTH - to_w, 0, LCD_WIDTH, 0, 0, to_w, LCD_HEIGHT);
        if (from_w > 0)
            lcd_bitmap_part(from, 0, 0, LCD_WIDTH, to_w, 0, from_w, LCD_HEIGHT);
        seam_x = to_w;
    }

    if (seam_color != METRO_FB_NO_SEAM && seam_x > 0 && seam_x < LCD_WIDTH)
    {
        lcd_set_foreground((unsigned)seam_color);
        lcd_vline(seam_x, 0, LCD_HEIGHT - 1);
    }
}

void metro_fb_present_slide(const fb_data *from, const fb_data *to, int dx,
                            long seam_color)
{
    metro_fb_compose_slide(from, to, dx, seam_color);
    lcd_update();
}

static inline fb_data blend_pixel(fb_data from, fb_data to, int a)
{
    int r = RGB_UNPACK_RED(from)   + (((RGB_UNPACK_RED(to)   - RGB_UNPACK_RED(from))   * a) >> 8);
    int g = RGB_UNPACK_GREEN(from) + (((RGB_UNPACK_GREEN(to) - RGB_UNPACK_GREEN(from)) * a) >> 8);
    int b = RGB_UNPACK_BLUE(from)  + (((RGB_UNPACK_BLUE(to)  - RGB_UNPACK_BLUE(from))  * a) >> 8);

    return LCD_RGBPACK(r, g, b);
}

unsigned metro_fb_blend_color(unsigned from, unsigned to, int alpha256)
{
    if (alpha256 < 0)
        alpha256 = 0;
    if (alpha256 > 256)
        alpha256 = 256;
    return blend_pixel((fb_data)from, (fb_data)to, alpha256);
}

void metro_fb_plot_alpha(int x, int y, unsigned color, int alpha256)
{
    fb_data *px;

    if (x < 0 || y < 0 || x >= LCD_WIDTH || y >= LCD_HEIGHT || alpha256 <= 0)
        return;
    px = FBADDR(x, y);
    if (alpha256 >= 256)
        *px = (fb_data)color;
    else
        *px = blend_pixel(*px, (fb_data)color, alpha256);
}

void metro_fb_present_fade(const fb_data *from, const fb_data *to, int alpha256)
{
    int x, y;

    if (alpha256 < 0)
        alpha256 = 0;
    if (alpha256 > 256)
        alpha256 = 256;

    for (y = 0; y < LCD_HEIGHT; y++)
    {
        fb_data *row = FBADDR(0, y);
        const fb_data *from_row = from + y * LCD_WIDTH;
        const fb_data *to_row = to + y * LCD_WIDTH;

        for (x = 0; x < LCD_WIDTH; x++)
            row[x] = blend_pixel(from_row[x], to_row[x], alpha256);
    }

    lcd_update();
}

void metro_fb_blend_over_color(const fb_data *img, unsigned bg_color, int alpha256)
{
    int x, y;

    if (alpha256 < 0)
        alpha256 = 0;
    if (alpha256 > 256)
        alpha256 = 256;

    for (y = 0; y < LCD_HEIGHT; y++)
    {
        fb_data *row = FBADDR(0, y);
        const fb_data *img_row = img + y * LCD_WIDTH;

        for (x = 0; x < LCD_WIDTH; x++)
            row[x] = blend_pixel((fb_data)bg_color, img_row[x], alpha256);
    }
}

void metro_fb_fill_rect(fb_data *dst, int x, int y, int w, int h, unsigned color)
{
    int row, col;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (w <= 0 || h <= 0)
        return;

    for (row = y; row < y + h; row++)
    {
        fb_data *p = dst + (size_t)row * LCD_WIDTH + x;
        for (col = 0; col < w; col++)
            p[col] = (fb_data)color;
    }
}
