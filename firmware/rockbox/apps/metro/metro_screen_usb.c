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
/* R5 (M-088): the "connected to the computer" screen, drawn ONLY from
 * what is embedded in the binary.
 *
 * Why that constraint: once the host owns the disk,
 * gui_usb_screen_run() (apps/gui/usb_screen.c) calls font_disable_all()
 * on purpose -- reading .fnt/.bmp from a volume the Mac is writing to
 * would risk real corruption. So no Metro font, no album art, no
 * icons from disk. What IS safe: metro_theme's colours (an enum in
 * RAM), the icon table (metro_icons_table.c, compiled in), and the
 * "moonlit.aura" wordmark (D-026) -- bm_rockboxlogo, the 320x98 bitmap gen_logo.py
 * generates into apps/bitmaps/native/ and bmp2rb compiles into the
 * binary. It is white-on-black with anti-aliasing baked in, so it is
 * drawn here as a MASK: each pixel's luminance becomes the blend
 * between background and foreground, which makes it right in both
 * themes (a flat lcd_bitmap() would paint a black slab on the light
 * theme). INITDATA_ATTR on that bitmap is a no-op on the S5L8702
 * (verified by Aura-Firmware, D-223), so it stays valid all session.
 *
 * Composition: the sync glyph (Fluent arrow_sync rasterised at 40px
 * with anti-aliasing, embedded as an 8-bit coverage mask -- M-089;
 * the first cut scaled the 16px mask 2x and looked blocky), the wordmark,
 * and WP7's indeterminate progress: five accent dots that cross the
 * screen, fast at the edges and slow through the middle, staggered,
 * with a pause between sweeps. No text at all -- the only font left
 * would be Rockbox's 8px sysfont, which has nothing to do with Metro.
 * metro_main.c draws this once before handing the LCD over;
 * usb_screen.c (see MODIFICATIONS.md) draws it again after
 * font_disable_all() and calls metro_screen_usb_tick() while the
 * cable is in. */
#include "lcd.h"
#include "kernel.h"
#include "backlight.h"
#include "bitmaps/rockboxlogo.h"

#include "metro_screen_usb.h"
#include "metro_theme.h"
#include "metro_fb.h"
#include "metro_widgets.h"
#include "metro_settings.h"

#define USB_ICON_Y       56   /* 40px anti-aliased glyph, M-089 */
#define USB_WORDMARK_Y   84   /* ink rows 28..70 of the bitmap -> 112..154 on screen */
#define USB_DOTS_Y       184
#define USB_DOT          4
#define USB_DOTS         5
#define USB_DOT_STAGGER  (HZ / 6)     /* delay between consecutive dots */
#define USB_SWEEP_TICKS  (HZ * 2)     /* one dot, edge to edge */
#define USB_PAUSE_TICKS  (HZ * 3 / 4) /* nothing on screen between sweeps */
#define USB_PERIOD_TICKS (USB_SWEEP_TICKS + USB_DOT_STAGGER * (USB_DOTS - 1) + USB_PAUSE_TICKS)

/* The wordmark as a mask: luminance -> alpha of fg over bg. Done once
 * per full draw (31k pixels), never per tick. */
static void draw_wordmark(int x, int y)
{
    const fb_data *px = (const fb_data *)bm_rockboxlogo.data;
    unsigned fg = metro_color_fg();
    int w = bm_rockboxlogo.width, h = bm_rockboxlogo.height;
    int cx, cy;

    for (cy = 0; cy < h; cy++)
        for (cx = 0; cx < w; cx++)
        {
            fb_data p = px[cy * w + cx];
            /* white-on-black source: any channel is the coverage */
            int lum = RGB_UNPACK_GREEN(p); /* 0..255 on RGB565 unpack */
            if (lum > 8)
                metro_fb_plot_alpha(x + cx, y + cy, fg, lum * 256 / 255);
        }
}

/* WP7 indeterminate curve: fast in, slow across the middle, fast out.
 * p in 0..1024 -> x fraction in 0..1024. Piecewise linear through
 * (0,0) (0.2,0.38) (0.8,0.62) (1,1). */
static int sweep_curve(int p)
{
    if (p < 205)  return p * 389 / 205;                       /* 0 .. 0.38 */
    if (p < 819)  return 389 + (p - 205) * (635 - 389) / 614; /* 0.38 .. 0.62 */
    return 635 + (p - 819) * (1024 - 635) / 205;              /* 0.62 .. 1 */
}

static void draw_dots(long t)
{
    long phase = t % USB_PERIOD_TICKS;
    int i;

    lcd_set_foreground(metro_color_bg());
    lcd_fillrect(0, USB_DOTS_Y - 1, LCD_WIDTH, USB_DOT + 2);
    lcd_set_foreground(metro_color_accent());

    for (i = 0; i < USB_DOTS; i++)
    {
        long local = phase - i * USB_DOT_STAGGER;
        int p, x;

        if (local < 0 || local >= USB_SWEEP_TICKS)
            continue; /* not on screen yet / already gone */
        p = (int)(local * 1024 / USB_SWEEP_TICKS);
        x = -USB_DOT + sweep_curve(p) * (LCD_WIDTH + 2 * USB_DOT) / 1024;
        lcd_fillrect(x, USB_DOTS_Y, USB_DOT, USB_DOT);
    }
}

void metro_screen_usb_show(void)
{
    lcd_set_viewport(NULL);
    lcd_set_foreground(metro_color_bg());
    lcd_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT);

    /* M-089: the 16px mask scaled 2x read as blocks on the real panel
     * ("se ve pixeleado") -- the same Fluent glyph, rasterised at 40px
     * with anti-aliasing and embedded as a coverage mask, drawn blended. */
    metro_widgets_draw_glyph(&metro_glyph_sync_large,
                             (LCD_WIDTH - metro_glyph_sync_large.width) / 2, USB_ICON_Y,
                             metro_color_accent());
    draw_wordmark((LCD_WIDTH - bm_rockboxlogo.width) / 2, USB_WORDMARK_Y);
    draw_dots(current_tick);
    lcd_update();
}

void metro_screen_usb_tick(void)
{
    /* Respect the backlight like every Metro animation: with the LCD
     * asleep there is nobody to animate for. With animations=off the
     * dots freeze on the first frame rather than sweep. */
    if (!lcd_active() || metro_settings.animations == METRO_ANIM_OFF)
        return;
    lcd_set_viewport(NULL);
    draw_dots(current_tick);
    lcd_update_rect(0, USB_DOTS_Y - 1, LCD_WIDTH, USB_DOT + 2);
}
