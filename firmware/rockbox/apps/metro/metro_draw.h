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
#ifndef METRO_DRAW_H
#define METRO_DRAW_H

#include "metro_fonts.h"

/* High-level drawing shell over lcd_* -- see PLAN_MAESTRO.md S1.2. F2
 * only implements what the type/palette specimen screen needs: text
 * (plain and right-edge-clipped), the header, and a battery readout.
 * metro_draw_rows()/metro_draw_pivots()/metro_draw_tile()/
 * metro_draw_progress() are deferred to F3, which is what actually
 * defines the data types (struct metro_page/metro_pivot/metro_row)
 * they would operate on -- see docs/DESVIACIONES.md F2-1. */

/* Fills the whole screen with metro_color_bg(). */
void metro_draw_clear(void);

/* Draws str at (x,y) in the given role's font and color, on the full
 * screen viewport -- naturally clipped at the screen edges by the LCD
 * driver (INVESTIGACION.md A.6), no explicit width needed. */
void metro_draw_text(enum metro_font_role role, int x, int y,
                      const char *str, unsigned color);

/* Same, but clipped to a viewport of exactly clip_w pixels starting
 * at x -- for content that must never run into a fixed-width
 * neighbour (e.g. a row that must stop before a right-aligned icon).
 * Metro-language "recorte en el borde derecho": pass a clip_w
 * narrower than the string's rendered width and the tail is cut, not
 * wrapped or refused. */
void metro_draw_text_cut_right(enum metro_font_role role, int x, int y,
                                const char *str, unsigned color, int clip_w);

/* Top line: page_title in caption/secondary at the left, current time
 * (if the RTC has one) and battery percentage at the right. */
void metro_draw_header(const char *page_title);

/* Battery percentage as caption-font text ending at (x_right, y) --
 * no bitmap icon yet (M-018, lands in F5 with the rest of the
 * compiled-in icon set). */
void metro_draw_battery(int x_right, int y);

#endif /* METRO_DRAW_H */
