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
#include "metro_icons.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_input.h"
#include "metro_lang.h"

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

    lcd_setfont(metro_font_id(MFONT_CAPTION));
    lcd_getstringsize((const unsigned char *)message, &w, &h);
    metro_draw_text(MFONT_CAPTION, (LCD_WIDTH - w) / 2, y + METRO_EMPTY_TILE_SIZE + 16,
                     message, metro_color_secondary());
}

void metro_widgets_draw_icon(enum metro_icon_id id, int x, int y, unsigned color)
{
    const struct metro_icon *icon;
    int row;

    if ((unsigned)id >= METRO_ICON_COUNT)
        return;

    icon = &metro_icons[id];
    lcd_set_foreground(color);

    for (row = 0; row < METRO_ICON_SIZE; row++)
    {
        unsigned mask = icon->rows[row];
        int col = 0;

        /* Por CORRIDAS horizontales, no pixel por pixel: un icono de
         * 16x16 son hasta 256 lcd_drawpixel() sueltos, y estos glifos
         * son siluetas rellenas donde una fila suele ser una o dos
         * corridas. Mismo criterio que el resto del dibujo de Metro,
         * que usa lcd_fillrect() para todo lo que sea un bloque. */
        while (col < METRO_ICON_SIZE)
        {
            int run;

            if (!(mask & (1u << (METRO_ICON_SIZE - 1 - col))))
            {
                col++;
                continue;
            }
            run = 0;
            while (col + run < METRO_ICON_SIZE &&
                   (mask & (1u << (METRO_ICON_SIZE - 1 - (col + run)))))
                run++;

            lcd_fillrect(x + col, y + row, run, 1);
            col += run;
        }
    }
}

void metro_widgets_draw_circle(int cx, int cy, int r, unsigned color)
{
    int x = r, y = 0;
    int err = 1 - r;

    if (r <= 0)
        return;

    lcd_set_foreground(color);
    while (x >= y)
    {
        lcd_drawpixel(cx + x, cy + y);
        lcd_drawpixel(cx + y, cy + x);
        lcd_drawpixel(cx - y, cy + x);
        lcd_drawpixel(cx - x, cy + y);
        lcd_drawpixel(cx - x, cy - y);
        lcd_drawpixel(cx - y, cy - x);
        lcd_drawpixel(cx + y, cy - x);
        lcd_drawpixel(cx + x, cy - y);
        y++;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void metro_widgets_draw_icon_in_circle(enum metro_icon_id id, int x, int y,
                                        int r, unsigned ring_color,
                                        unsigned glyph_color)
{
    int cx = x + r, cy = y + r;

    metro_widgets_draw_circle(cx, cy, r, ring_color);
    /* The 16px glyph cell centred on the ring's centre; with r=13 that
     * leaves 5px of air between cell and ring, and Fluent's own ~2px
     * internal padding makes the visible ink ~12px. */
    metro_widgets_draw_icon(id, cx - METRO_ICON_SIZE / 2, cy - METRO_ICON_SIZE / 2,
                            glyph_color);
}
