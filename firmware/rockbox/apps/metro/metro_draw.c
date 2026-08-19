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
#include <stdio.h>
#include "lcd.h"
#include "viewport.h"
#include "powermgmt.h"
#include "timefuncs.h"

#include "metro_draw.h"
#include "metro_theme.h"

#define METRO_HEADER_HEIGHT 24

void metro_draw_clear(void)
{
    lcd_set_background(metro_color_bg());
    lcd_clear_display();
}

void metro_draw_text(enum metro_font_role role, int x, int y,
                      const char *str, unsigned color)
{
    lcd_setfont(metro_font_id(role));
    lcd_set_foreground(color);
    lcd_putsxy(x, y, (const unsigned char *)str);
}

void metro_draw_text_cut_right(enum metro_font_role role, int x, int y,
                                const char *str, unsigned color, int clip_w)
{
    struct viewport vp;
    struct viewport *old_vp;

    viewport_set_fullscreen(&vp, SCREEN_MAIN);
    vp.x = x;
    vp.width = clip_w;
    vp.font = metro_font_id(role);
    vp.fg_pattern = color;
    vp.bg_pattern = metro_color_bg();

    old_vp = lcd_set_viewport(&vp);
    lcd_putsxy(0, y - vp.y, (const unsigned char *)str);
    lcd_set_viewport(old_vp);
}

void metro_draw_battery(int x_right, int y)
{
    char buf[8];
    int w, h;
    int level = battery_level();

    if (level < 0)
        snprintf(buf, sizeof(buf), "--%%");
    else
    {
        if (level > 100)
            level = 100; /* battery_level() is documented as percent,
                            but its prototype can't tell the compiler
                            that -- clamp so %d can't overflow buf */
        snprintf(buf, sizeof(buf), "%d%%", level);
    }

    lcd_setfont(metro_font_id(MFONT_CAPTION));
    lcd_getstringsize((const unsigned char *)buf, &w, &h);
    lcd_set_foreground(metro_color_secondary());
    lcd_putsxy(x_right - w, y, (const unsigned char *)buf);
}

void metro_draw_header(const char *page_title)
{
    struct tm *now = get_time();
    char timebuf[8];
    int w, h;

    lcd_setfont(metro_font_id(MFONT_CAPTION));
    lcd_set_foreground(metro_color_secondary());
    lcd_putsxy(12, 4, (const unsigned char *)page_title);

    if (now != NULL)
    {
        snprintf(timebuf, sizeof(timebuf), "%02d:%02d", now->tm_hour, now->tm_min);
        lcd_getstringsize((const unsigned char *)timebuf, &w, &h);
        lcd_set_foreground(metro_color_secondary());
        lcd_putsxy(LCD_WIDTH - 40 - w, 4, (const unsigned char *)timebuf);
    }

    metro_draw_battery(LCD_WIDTH - 4, 4);
}
