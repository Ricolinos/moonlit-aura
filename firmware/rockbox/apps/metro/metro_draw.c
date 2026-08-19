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
#include <stdbool.h>
#include "lcd.h"
#include "viewport.h"
#include "powermgmt.h"
#include "timefuncs.h"

#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"

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

    /* viewport_set_defaults() -- NOT viewport_set_fullscreen() directly.
     * Both end up in lcd_init_viewport(), which READS vp->buffer before
     * anything sets it: if it's non-NULL it is dereferenced as a
     * struct frame_buffer_t* and its stride/data/get_address_fn fields
     * are read and possibly written through. With a stack viewport that
     * is whatever garbage was on the stack -- undefined behaviour that
     * in practice corrupted the LCD state and made later
     * screen_dump() calls (FBADDR() -> buffer->get_address_fn) jump
     * into random code. viewport_set_defaults() zeroes vp->buffer first,
     * which is why every core caller uses it. See DECISIONS.md M-027. */
    viewport_set_defaults(&vp, SCREEN_MAIN);
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

#define METRO_PIVOT_Y      28
#define METRO_PIVOT_GAP    24
#define METRO_ROWS_FIRST_Y 84
#define METRO_ROW_PITCH    28
#define METRO_ROWS_VISIBLE METRO_DRAW_ROWS_VISIBLE
#define METRO_ROWS_LEFT_X  12

void metro_draw_pivots(const struct metro_page *page, int active_pivot,
                        int x_offset)
{
    int i, x = METRO_ROWS_LEFT_X + x_offset;

    lcd_setfont(metro_font_id(MFONT_DISPLAY));

    for (i = active_pivot; i < page->npivots && x < LCD_WIDTH; i++)
    {
        int w, h;
        const char *name = metro_lang_str(page->pivots[i].name);

        lcd_set_foreground(i == active_pivot ? metro_color_fg()
                                              : metro_color_tertiary());
        lcd_getstringsize((const unsigned char *)name, &w, &h);
        lcd_putsxy(x, METRO_PIVOT_Y, (const unsigned char *)name);
        x += w + METRO_PIVOT_GAP;
    }
}

void metro_draw_rows(const struct metro_pivot *pivot, int first, int sel,
                      int x_offset)
{
    int count = pivot->count(pivot->ctx);
    int i, y = METRO_ROWS_FIRST_Y;
    int x = METRO_ROWS_LEFT_X + x_offset;

    /* Draw one row past METRO_ROWS_VISIBLE on purpose -- it peeks,
     * naturally cut by the bottom of the 240px screen (A.6), same
     * "next thing asoma cortado" effect as the pivot header. */
    for (i = first; i < count && i < first + METRO_ROWS_VISIBLE + 1; i++)
    {
        struct metro_row row;
        bool selected = (i == sel);

        pivot->get_row(pivot->ctx, i, &row);
        metro_draw_text(selected ? MFONT_LIST_SEL : MFONT_LIST, x, y,
                         row.title,
                         selected ? metro_color_fg() : metro_color_secondary());

        if (row.subtitle)
        {
            int w, h;
            lcd_setfont(metro_font_id(MFONT_CAPTION));
            lcd_getstringsize((const unsigned char *)row.subtitle, &w, &h);
            metro_draw_text(MFONT_CAPTION, LCD_WIDTH - 12 - w, y + 4,
                             row.subtitle, metro_color_tertiary());
        }

        y += METRO_ROW_PITCH;
    }
}
