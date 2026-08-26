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
#include "moonlit_logo.h"
#include "metro_theme.h"
#include "metro_draw.h"

#define METRO_SPLASH_CRESCENT_SIZE 64
#define METRO_SPLASH_WORDMARK_GAP  8  /* between the crescent and the wordmark */
#define METRO_SPLASH_HERO_WIDTH \
    (METRO_SPLASH_CRESCENT_SIZE + METRO_SPLASH_WORDMARK_GAP + MOONLIT_LOGO_WORDMARK_WIDTH)

#define METRO_SPLASH_BAR_WIDTH  120
#define METRO_SPLASH_BAR_HEIGHT 2
#define METRO_SPLASH_BAR_GAP    16 /* below the hero */

/* Whole block (crescent + bar) centered as a unit on the 240px screen. */
#define METRO_SPLASH_HERO_Y \
    ((LCD_HEIGHT - METRO_SPLASH_CRESCENT_SIZE - METRO_SPLASH_BAR_GAP - METRO_SPLASH_BAR_HEIGHT) / 2)

/* moonlit (D-016, D-044): 64px creciente + wordmark "moonlit" --
 * sustituye el texto/bitmap provisional de D-026, retirado en este
 * mismo hito. */
static void draw_hero(void)
{
    int hero_x = (LCD_WIDTH - METRO_SPLASH_HERO_WIDTH) / 2;
    int wordmark_y = METRO_SPLASH_HERO_Y +
                      (METRO_SPLASH_CRESCENT_SIZE - MOONLIT_LOGO_WORDMARK_HEIGHT) / 2;

    metro_draw_clear();
    moonlit_logo_draw_crescent(METRO_SPLASH_CRESCENT_SIZE, hero_x, METRO_SPLASH_HERO_Y,
                                metro_color_fg());
    moonlit_logo_draw_wordmark(hero_x + METRO_SPLASH_CRESCENT_SIZE + METRO_SPLASH_WORDMARK_GAP,
                                wordmark_y, metro_color_fg());
}

void metro_screen_splash_show(void)
{
    draw_hero();
    lcd_update();
}

void metro_screen_splash_progress(int pct)
{
    int bar_x = (LCD_WIDTH - METRO_SPLASH_BAR_WIDTH) / 2;
    int bar_y = METRO_SPLASH_HERO_Y + METRO_SPLASH_CRESCENT_SIZE + METRO_SPLASH_BAR_GAP;

    draw_hero();
    metro_draw_progress(bar_x, bar_y, METRO_SPLASH_BAR_WIDTH, METRO_SPLASH_BAR_HEIGHT, pct);
    lcd_update();
}
