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
     * which is why every core caller uses it. See DECISIONS.md M-027.
     *
     * F11: vp.buffer is overwritten right after with whatever buffer
     * lcd_current_viewport is ACTUALLY drawing into -- NULL (real
     * screen) normally, but an offscreen metro_fb.c buffer while
     * metro_transitions.c is pre-rendering a destination frame. Left
     * at viewport_set_defaults()'s NULL, this function always drew
     * into the real LCD regardless -- row titles never made it into
     * an offscreen "to" frame, so a completed SLIDE composited a
     * blank row area (found visually: rows present with
     * animations=off, gone with animations=all). Still well-defined
     * either way -- never the M-027 stack-garbage case, just a
     * pointer copy of an already-valid buffer. */
    viewport_set_defaults(&vp, SCREEN_MAIN);
    vp.buffer = lcd_current_viewport->buffer;
    vp.x = x;
    vp.width = clip_w;
    vp.font = metro_font_id(role);
    vp.fg_pattern = color;
    vp.bg_pattern = metro_color_bg();

    old_vp = lcd_set_viewport(&vp);
    lcd_putsxy(0, y - vp.y, (const unsigned char *)str);
    lcd_set_viewport(old_vp);
}

/* F10: real geometric icon (rect body + nub, proportional fill)
 * replacing the "N%" text (M-018's deferred placeholder). Body is
 * outlined regardless of charge level; a negative battery_level()
 * (charge unknown, e.g. running off USB power in the sim) just draws
 * the empty outline with no fill instead of Rockbox's own "--%". */
#define METRO_BATTERY_W     18
#define METRO_BATTERY_H     9
#define METRO_BATTERY_NUB_W 2
#define METRO_BATTERY_NUB_H 4

void metro_draw_battery(int x_right, int y)
{
    int level = battery_level();
    int body_x = x_right - METRO_BATTERY_NUB_W - METRO_BATTERY_W;
    int nub_x = x_right - METRO_BATTERY_NUB_W;
    int nub_y = y + (METRO_BATTERY_H - METRO_BATTERY_NUB_H) / 2;
    int fill_w;

    lcd_set_foreground(metro_color_secondary());
    lcd_drawrect(body_x, y, METRO_BATTERY_W, METRO_BATTERY_H);
    lcd_fillrect(nub_x, nub_y, METRO_BATTERY_NUB_W, METRO_BATTERY_NUB_H);

    if (level > 100)
        level = 100; /* battery_level() is documented as percent, but
                        its prototype can't tell the compiler that. */
    if (level > 0)
    {
        fill_w = (METRO_BATTERY_W - 4) * level / 100;
        if (fill_w > 0)
            lcd_fillrect(body_x + 2, y + 2, fill_w, METRO_BATTERY_H - 4);
    }
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

void metro_draw_clear_rows_area(void)
{
    lcd_set_foreground(metro_color_bg());
    lcd_fillrect(0, METRO_ROWS_FIRST_Y, LCD_WIDTH, LCD_HEIGHT - METRO_ROWS_FIRST_Y);
}

/* F12: y_offsets[] (one entry per VISIBLE row slot, same indexing as
 * the loop below -- NULL for the plain, un-offset draw
 * metro_draw_rows() still does) is metro_screen_list.c's FEATHER
 * cascade (S3.3): each row's own y nudged down by a shrinking amount
 * as its own entrance animates in, independent of every other row's
 * offset. Never clears first -- metro_screen_list.c owns clearing the
 * row area between frames (metro_draw_clear_rows_area()) since the
 * feather loop redraws far more often than a single metro_draw_rows()
 * call would otherwise need to. */
void metro_draw_rows_ex(const struct metro_pivot *pivot, int first, int sel,
                         int x_offset, const int *y_offsets)
{
    int count = pivot->count(pivot->ctx);
    int i, y = METRO_ROWS_FIRST_Y;
    int x = METRO_ROWS_LEFT_X + x_offset;
    int visible_index = 0;

    /* Draw one row past METRO_ROWS_VISIBLE on purpose -- it peeks,
     * naturally cut by the bottom of the 240px screen (A.6), same
     * "next thing asoma cortado" effect as the pivot header. */
    for (i = first; i < count && i < first + METRO_ROWS_VISIBLE + 1; i++, visible_index++)
    {
        struct metro_row row;
        bool selected = (i == sel);
        int row_y = y + (y_offsets ? y_offsets[visible_index] : 0);

        pivot->get_row(pivot->ctx, i, &row);

        /* F10: clip the title so it can never run into the
         * right-aligned subtitle (long filenames especially --
         * videos/photos, up to METRO_FSUTIL_NAME_LEN bytes). Subtitle
         * width has to be measured first to know where the title's
         * clip boundary is. */
        int title_clip_w = LCD_WIDTH - x;

        if (row.subtitle)
        {
            int sub_w, sub_h;
            lcd_setfont(metro_font_id(MFONT_CAPTION));
            lcd_getstringsize((const unsigned char *)row.subtitle, &sub_w, &sub_h);
            metro_draw_text(MFONT_CAPTION, LCD_WIDTH - 12 - sub_w, row_y + 4,
                             row.subtitle, metro_color_tertiary());
            title_clip_w = LCD_WIDTH - 12 - sub_w - x - 8;
        }

        metro_draw_text_cut_right(selected ? MFONT_LIST_SEL : MFONT_LIST, x, row_y,
                                   row.title,
                                   selected ? metro_color_fg() : metro_color_secondary(),
                                   title_clip_w);

        y += METRO_ROW_PITCH;
    }
}

void metro_draw_rows(const struct metro_pivot *pivot, int first, int sel,
                      int x_offset)
{
    metro_draw_rows_ex(pivot, first, sel, x_offset, NULL);
}

void metro_draw_progress(int x, int y, int width, int height, int pct)
{
    int fill_w;

    if (pct < 0)
        pct = 0;
    if (pct > 100)
        pct = 100;
    fill_w = width * pct / 100;

    lcd_set_foreground(metro_color_tertiary());
    lcd_fillrect(x, y, width, height);

    if (fill_w > 0)
    {
        lcd_set_foreground(metro_color_accent());
        lcd_fillrect(x, y, fill_w, height);
    }
}

void metro_draw_tile(int x, int y, int size, const char *label)
{
    char initial[2] = { ' ', '\0' };
    int w, h;

    if (label && label[0] >= 'a' && label[0] <= 'z')
        initial[0] = label[0] - 32;
    else if (label && label[0])
        initial[0] = label[0];

    lcd_set_foreground(metro_color_accent());
    lcd_fillrect(x, y, size, size);

    /* A blank label (metro_widgets_draw_empty_state()'s plain accent
     * square, no letter) must draw nothing here -- the custom
     * MFONT_DISPLAY bitmap font has no real space glyph and falls
     * back to garbage (observed rendering an unrelated glyph) instead
     * of blank pixels. */
    if (initial[0] != ' ')
    {
        lcd_setfont(metro_font_id(MFONT_DISPLAY));
        lcd_getstringsize((const unsigned char *)initial, &w, &h);
        lcd_set_foreground(metro_color_bg());
        lcd_putsxy(x + (size - w) / 2, y + (size - h) / 2, (const unsigned char *)initial);
    }
}
