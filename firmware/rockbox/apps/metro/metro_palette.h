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
#ifndef METRO_PALETTE_H
#define METRO_PALETTE_H

#include "lcd.h"

/* Every color Metro draws with lives here -- see DECISIONS.md M-012,
 * INVESTIGACION.md F.5. Nothing outside this file may hardcode an RGB
 * triplet (CLAUDE.md). Quantization error into RGB565 is <=1% for all
 * of these (F.5), so the WP7 hex values are used verbatim. Plain
 * #define, no storage in this header (no metro_palette.c) -- matches
 * the token-header idiom already used elsewhere in this codebase. */

/* Base tones, dark theme (default) */
#define METRO_DARK_BG          LCD_RGBPACK(0x00, 0x00, 0x00)
#define METRO_DARK_FG          LCD_RGBPACK(0xFF, 0xFF, 0xFF)
#define METRO_DARK_SECONDARY   LCD_RGBPACK(0x99, 0x99, 0x99)
#define METRO_DARK_TERTIARY    LCD_RGBPACK(0x66, 0x66, 0x66)

/* Base tones, light theme */
#define METRO_LIGHT_BG         LCD_RGBPACK(0xFF, 0xFF, 0xFF)
#define METRO_LIGHT_FG         LCD_RGBPACK(0x00, 0x00, 0x00)
#define METRO_LIGHT_SECONDARY  LCD_RGBPACK(0x66, 0x66, 0x66)
#define METRO_LIGHT_TERTIARY   LCD_RGBPACK(0x99, 0x99, 0x99)

/* The 10 Windows Phone 7 accent colors (INVESTIGACION.md F.5). Same
 * hex in both themes -- accents are never tinted by theme. Mapped to
 * enum metro_accent by metro_theme.c, index order matches. */
#define METRO_ACCENT_COLOR_BLUE    LCD_RGBPACK(0x1B, 0xA1, 0xE2)
#define METRO_ACCENT_COLOR_BROWN   LCD_RGBPACK(0xA0, 0x50, 0x00)
#define METRO_ACCENT_COLOR_GREEN   LCD_RGBPACK(0x33, 0x99, 0x33)
#define METRO_ACCENT_COLOR_LIME    LCD_RGBPACK(0x8C, 0xBF, 0x26)
#define METRO_ACCENT_COLOR_MAGENTA LCD_RGBPACK(0xFF, 0x00, 0x97)
#define METRO_ACCENT_COLOR_MANGO   LCD_RGBPACK(0xF0, 0x96, 0x09)
#define METRO_ACCENT_COLOR_PINK    LCD_RGBPACK(0xE6, 0x71, 0xB8)
#define METRO_ACCENT_COLOR_PURPLE  LCD_RGBPACK(0xA2, 0x00, 0xFF)
#define METRO_ACCENT_COLOR_RED     LCD_RGBPACK(0xE5, 0x14, 0x00)
#define METRO_ACCENT_COLOR_TEAL    LCD_RGBPACK(0x00, 0xAB, 0xA9)

#endif /* METRO_PALETTE_H */
