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

    metro_draw_progress(0, METRO_VOLUME_OVERLAY_Y, LCD_WIDTH, 6, pct);
    snprintf(label, sizeof(label), "%s %d%%", metro_lang_str(LANG_NP_VOLUME), pct);
    metro_draw_text(MFONT_CAPTION, 12, METRO_VOLUME_OVERLAY_Y - 14, label,
                     metro_color_secondary());
}

#define METRO_INDEX_LETTER_SIZE 80

void metro_widgets_draw_index_letter(char c)
{
    char s[2] = { c, '\0' };
    int x = (LCD_WIDTH - METRO_INDEX_LETTER_SIZE) / 2;
    int y = (LCD_HEIGHT - METRO_INDEX_LETTER_SIZE) / 2;

    metro_draw_tile(x, y, METRO_INDEX_LETTER_SIZE, s);
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

/* F10: crossed paths with arrowheads at both right-hand ends -- the
 * "shuffle" metaphor, contained within a METRO_WIDGETS_ICON_SIZE
 * square starting at (x, y). */
void metro_widgets_draw_shuffle_icon(int x, int y)
{
    int s = METRO_WIDGETS_ICON_SIZE;

    lcd_set_foreground(metro_color_accent());

    lcd_drawline(x, y, x + s, y + s);
    lcd_drawline(x, y + s, x + s, y);

    lcd_drawline(x + s - 4, y - 3, x + s, y);
    lcd_drawline(x + s - 4, y + 3, x + s, y);
    lcd_drawline(x + s - 4, y + s - 3, x + s, y + s);
    lcd_drawline(x + s - 4, y + s + 3, x + s, y + s);
}

/* F10: square loop outline with a small arrowhead breaking its
 * top-right corner -- the "repeat" metaphor. `one` overlays a small
 * "1" (REPEAT_ONE vs REPEAT_ALL), same square as
 * metro_widgets_draw_shuffle_icon() for side-by-side alignment on the
 * Now Playing screen. */
void metro_widgets_draw_repeat_icon(int x, int y, bool one)
{
    int s = METRO_WIDGETS_ICON_SIZE;

    lcd_set_foreground(metro_color_accent());
    lcd_drawrect(x, y, s, s);

    lcd_drawline(x + s - 5, y - 3, x + s + 1, y - 3);
    lcd_drawline(x + s + 1, y - 3, x + s - 2, y + 1);

    if (one)
    {
        char digit[2] = { '1', '\0' };
        int w, h;

        lcd_setfont(metro_font_id(MFONT_CAPTION));
        lcd_getstringsize((const unsigned char *)digit, &w, &h);
        lcd_set_foreground(metro_color_accent());
        lcd_putsxy(x + (s - w) / 2, y + (s - h) / 2, (const unsigned char *)digit);
    }
}
