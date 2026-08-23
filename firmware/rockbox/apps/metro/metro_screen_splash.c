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
#include "lcd.h"

#include "metro_screen_splash.h"
#include "metro_fonts.h"
#include "metro_theme.h"
#include "metro_draw.h"

#define METRO_SPLASH_BAR_WIDTH  120
#define METRO_SPLASH_BAR_HEIGHT 2
#define METRO_SPLASH_BAR_GAP    16 /* below the wordmark's baseline */

static void draw_wordmark(void)
{
    /* R5 (M-092, encargo del dueño): "metro" y, debajo, "aura" -- la
     * familia, en el cuerpo de título y color secundario. El bloque
     * completo queda centrado; el mismo par vive en el bitmap embebido
     * (gen_logo.py) que usan el arranque y la pantalla USB. */
    const char *text = "metro";
    const char *sub = "aura";
    int w, h, sw, sh, top;

    metro_draw_clear();
    lcd_setfont(metro_font_id(MFONT_DISPLAY));
    lcd_getstringsize((const unsigned char *)text, &w, &h);
    lcd_setfont(metro_font_id(MFONT_TITLE));
    lcd_getstringsize((const unsigned char *)sub, &sw, &sh);
    top = (LCD_HEIGHT - (h + 4 + sh)) / 2;
    metro_draw_text(MFONT_DISPLAY, (LCD_WIDTH - w) / 2, top, text, metro_color_fg());
    metro_draw_text(MFONT_TITLE, (LCD_WIDTH - sw) / 2, top + h + 4, sub,
                     metro_color_secondary());
}

void metro_screen_splash_show(void)
{
    draw_wordmark();
    lcd_update();
}

void metro_screen_splash_progress(int pct)
{
    int w, h;
    int bar_x = (LCD_WIDTH - METRO_SPLASH_BAR_WIDTH) / 2;
    int bar_y;

    draw_wordmark();

    /* Debajo del bloque metro+aura (M-092). */
    {
        int sw, sh;
        lcd_setfont(metro_font_id(MFONT_DISPLAY));
        lcd_getstringsize((const unsigned char *)"metro", &w, &h);
        lcd_setfont(metro_font_id(MFONT_TITLE));
        lcd_getstringsize((const unsigned char *)"aura", &sw, &sh);
        bar_y = (LCD_HEIGHT - (h + 4 + sh)) / 2 + h + 4 + sh + METRO_SPLASH_BAR_GAP;
    }

    metro_draw_progress(bar_x, bar_y, METRO_SPLASH_BAR_WIDTH, METRO_SPLASH_BAR_HEIGHT, pct);
    lcd_update();
}
