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
#include "moonlit_fonts.h"
#include "metro_theme.h"
#include "metro_draw.h"
#include "metro_lang.h"

#define METRO_SPLASH_BAR_WIDTH  120
#define METRO_SPLASH_BAR_HEIGHT 2
#define METRO_SPLASH_BAR_GAP    16 /* below the wordmark's baseline */

static void draw_wordmark(void)
{
    /* moonlit (D-026): one-line provisional wordmark "moonlit.aura" from
     * the string table (the "metro"/"aura" pair of M-092 is retired);
     * H5 replaces it with the Waning Crescent logo (D-016). */
    const char *text = metro_lang_str(LANG_WORDMARK);
    int w, h, top;

    metro_draw_clear();
    lcd_setfont(metro_font_id(MFONT_DISPLAY));
    lcd_getstringsize((const unsigned char *)text, &w, &h);
    top = (LCD_HEIGHT - h) / 2;
    metro_draw_text(MFONT_DISPLAY, (LCD_WIDTH - w) / 2, top, text, metro_color_fg());
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

    /* Below the one-line wordmark (D-026). */
    lcd_setfont(metro_font_id(MFONT_DISPLAY));
    lcd_getstringsize((const unsigned char *)metro_lang_str(LANG_WORDMARK), &w, &h);
    bar_y = (LCD_HEIGHT - h) / 2 + h + METRO_SPLASH_BAR_GAP;

    metro_draw_progress(bar_x, bar_y, METRO_SPLASH_BAR_WIDTH, METRO_SPLASH_BAR_HEIGHT, pct);
    lcd_update();
}
