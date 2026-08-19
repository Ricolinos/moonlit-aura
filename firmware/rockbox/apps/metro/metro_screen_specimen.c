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

#include "metro_screen_specimen.h"
#include "metro_draw.h"
#include "metro_fonts.h"
#include "metro_theme.h"

void metro_screen_specimen_show(void)
{
    unsigned fg = metro_color_fg();
    int i;
    int swatch_w = 28, swatch_h = 16, gap = 2;
    int x;

    metro_draw_clear();
    metro_draw_header("specimen");

    /* display 48px -- deliberately wide enough to overflow past the
     * right edge of the screen; the LCD driver clips it automatically
     * (INVESTIGACION.md A.6), no explicit viewport needed here. */
    metro_draw_text(MFONT_DISPLAY, 140, 26, "recorte al borde", fg);

    metro_draw_text(MFONT_TITLE, 12, 84, "title 28px", fg);
    metro_draw_text(MFONT_LIST, 12, 118, "list 20px regular", fg);
    metro_draw_text(MFONT_LIST_SEL, 12, 142, "list_sel 20px semibold", fg);

    /* caption 14px, explicitly clipped to a 90px-wide box (narrower
     * than the screen) to exercise metro_draw_text_cut_right()
     * separately from the natural screen-edge clip used above. */
    metro_draw_text_cut_right(MFONT_CAPTION, 12, 168,
                               "caption 14px clipped at ninety pixels wide",
                               fg, 90);

    /* 10 WP7 accent swatches (INVESTIGACION.md F.5) */
    x = 10;
    for (i = 0; i < METRO_ACCENT_COUNT; i++)
    {
        lcd_set_foreground(metro_accent_color((enum metro_accent)i));
        lcd_fillrect(x, 194, swatch_w, swatch_h);
        x += swatch_w + gap;
    }

    lcd_update();
}
