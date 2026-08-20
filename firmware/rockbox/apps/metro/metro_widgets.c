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

#define METRO_VOLUME_OVERLAY_Y 232

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

void metro_widgets_draw_volume_overlay(int pct)
{
    char label[24];
    /* R4/FA-1 (M-077): el overlay era texto puro ("volumen 42%"). Con
     * el glifo de altavoz, el porcentaje solo ya dice qué es y la
     * palabra sobra -- se gana claridad y se acorta la línea. El icono
     * se alinea con el texto: METRO_ICON_SIZE (16) contra la caja de
     * MFONT_CAPTION (14), así que un píxel abajo lo centra. */
    int icon_y = METRO_VOLUME_OVERLAY_Y - 15;
    int text_x = 12 + METRO_ICON_SIZE + 6;

    metro_draw_progress(0, METRO_VOLUME_OVERLAY_Y, LCD_WIDTH, 6, pct);
    metro_widgets_draw_icon(METRO_ICON_SPEAKER, 12, icon_y, metro_color_secondary());
    snprintf(label, sizeof(label), "%d%%", pct);
    metro_draw_text(MFONT_CAPTION, text_x, METRO_VOLUME_OVERLAY_Y - 14, label,
                     metro_color_secondary());
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
