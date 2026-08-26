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
#include "kernel.h"
#include "misc.h"
#include "lcd.h"

#include "metro_widgets.h"
#include "moonlit_icons.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_input.h"
#include "metro_lang.h"
#include "metro_fb.h"

bool metro_widgets_confirm(const char *title, const char *question)
{
    bool sel_yes = false; /* default to "no" -- the safe answer */

    while (1)
    {
        int action;

        metro_draw_clear();
        metro_draw_header(title);
        metro_draw_text(MFONT_TITLE, 12, 90, question, metro_color_fg());
        metro_draw_text(sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, 150,
                         metro_lang_str(LANG_DIALOG_YES),
                         sel_yes ? metro_color_fg() : metro_color_secondary());
        metro_draw_text(!sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, 178,
                         metro_lang_str(LANG_DIALOG_NO),
                         !sel_yes ? metro_color_fg() : metro_color_secondary());
        lcd_update();

        action = metro_input_next(MCTX_DIALOG, HZ / 10, NULL);

        if (action & SYS_EVENT)
        {
            default_event_handler(action);
            continue;
        }

        switch (action)
        {
            case MACT_PREV:
            case MACT_NEXT:
                sel_yes = !sel_yes;
                break;
            case MACT_SELECT:
                return sel_yes;
            case MACT_BACK:
                return false;
            default:
                break;
        }
    }
}

#define METRO_INDEX_LETTER_SIZE 80

void metro_widgets_draw_index_letter(const char *letter)
{
    int x = (LCD_WIDTH - METRO_INDEX_LETTER_SIZE) / 2;
    int y = (LCD_HEIGHT - METRO_INDEX_LETTER_SIZE) / 2;

    /* R4/FA-5a (M-076): recibe una CADENA, no un char -- la inicial
     * puede ser un carácter UTF-8 de varios bytes. metro_draw_tile()
     * ya toma el primer carácter completo de lo que se le pase. */
    metro_draw_tile(x, y, METRO_INDEX_LETTER_SIZE, letter);
}

#define METRO_EMPTY_TILE_SIZE 96

void metro_widgets_draw_empty_state(const char *message)
{
    int x = (LCD_WIDTH - METRO_EMPTY_TILE_SIZE) / 2;
    int y = 60;
    int w, h;

    metro_draw_tile(x, y, METRO_EMPTY_TILE_SIZE, " ");

    lcd_setfont(metro_font_id(MFONT_BODY));
    lcd_getstringsize((const unsigned char *)message, &w, &h);
    metro_draw_text(MFONT_BODY, (LCD_WIDTH - w) / 2, y + METRO_EMPTY_TILE_SIZE + 16,
                     message, metro_color_secondary());
}

void metro_widgets_draw_icon(enum moonlit_icon_id id, int x, int y, unsigned color)
{
    moonlit_icon_draw(id, MOONLIT_ICON_SIZE_16, x, y, color);
}

/* Integer sqrt, rounded down. Inputs here are < 2^24 (distances in
 * 8.8 fixed point squared), so the 32-bit loop is enough. */
static unsigned isqrt32(unsigned v)
{
    unsigned res = 0, bit = 1u << 30;

    while (bit > v)
        bit >>= 2;
    while (bit)
    {
        if (v >= res + bit)
        {
            v -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

void metro_widgets_draw_circle(int cx, int cy, int r, unsigned color)
{
    int dx, dy;
    int r256 = r * 256;

    if (r <= 0)
        return;

    for (dy = -r - 1; dy <= r + 1; dy++)
    {
        for (dx = -r - 1; dx <= r + 1; dx++)
        {
            /* distance in 8.8: sqrt((dx^2+dy^2) * 65536) = d * 256 */
            unsigned d256 = isqrt32((unsigned)(dx * dx + dy * dy) << 16);
            int diff = (int)d256 - r256;
            int alpha;

            if (diff < 0)
                diff = -diff;
            /* ~1.5px ring: full within 0.25px of r, gone at 1.25px. A
             * strict 1px ring spreads over two pixel rows at half
             * intensity and reads grey on the panel; this keeps it thin
             * but solid. */
            alpha = 320 - diff;
            if (alpha > 256)
                alpha = 256;
            if (alpha > 0)
                metro_fb_plot_alpha(cx + dx, cy + dy, color, alpha);
        }
    }
}

void metro_widgets_draw_icon_in_circle(enum moonlit_icon_id id, int x, int y,
                                        int r, unsigned ring_color,
                                        unsigned glyph_color)
{
    int cx = x + r, cy = y + r;

    metro_widgets_draw_circle(cx, cy, r, ring_color);
    /* The 16px glyph cell centred on the ring's centre; with r=13 that
     * leaves 5px of air between cell and ring. */
    metro_widgets_draw_icon(id, cx - MOONLIT_ICON_SIZE_16 / 2, cy - MOONLIT_ICON_SIZE_16 / 2,
                            glyph_color);
}

