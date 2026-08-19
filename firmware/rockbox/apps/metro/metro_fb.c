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

void metro_fb_present_slide(const fb_data *from, const fb_data *to, int dx)
{
    int from_w, to_w;

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
    }
    else
    {
        to_w = -dx;
        from_w = LCD_WIDTH - to_w;
        lcd_bitmap_part(to, LCD_WIDTH - to_w, 0, LCD_WIDTH, 0, 0, to_w, LCD_HEIGHT);
        if (from_w > 0)
            lcd_bitmap_part(from, 0, 0, LCD_WIDTH, to_w, 0, from_w, LCD_HEIGHT);
    }

    lcd_update();
}

static inline fb_data blend_pixel(fb_data from, fb_data to, int a)
{
    int r = RGB_UNPACK_RED(from)   + (((RGB_UNPACK_RED(to)   - RGB_UNPACK_RED(from))   * a) >> 8);
    int g = RGB_UNPACK_GREEN(from) + (((RGB_UNPACK_GREEN(to) - RGB_UNPACK_GREEN(from)) * a) >> 8);
    int b = RGB_UNPACK_BLUE(from)  + (((RGB_UNPACK_BLUE(to)  - RGB_UNPACK_BLUE(from))  * a) >> 8);

    return LCD_RGBPACK(r, g, b);
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
