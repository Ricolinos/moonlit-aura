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
#include "metro_page.h"

/* How many rows metro_draw_rows() shows fully before the next one
 * starts peeking off the bottom -- callers need this to compute
 * metro_nav_move_sel()'s visible_rows argument consistently with what
 * actually gets drawn. */
#define METRO_DRAW_ROWS_VISIBLE 5

/* High-level drawing shell over lcd_* -- see PLAN_MAESTRO.md S1.2.
 * metro_draw_tile()/metro_draw_progress() are still deferred -- F3
 * doesn't have tiles or a progress bar yet, those land with Now
 * Playing/Music Flow. metro_draw_rows()/metro_draw_pivots() were
 * deferred from F2 to F3 (docs/DESVIACIONES.md F2-1, because
 * metro_page.h didn't exist yet) and are implemented here now. */

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

/* Pivot header at y=28..76 (PLAN_MAESTRO.md S1.4): draws pivots from
 * active_pivot onwards only -- pivots to the left of the active one
 * are never drawn (WP7 Pivot behaviour, no "peek back"). The active
 * pivot is in metro_color_fg(), the rest in metro_color_tertiary();
 * natural screen-edge clipping (A.6) cuts the last one that doesn't
 * fully fit. x_offset shifts everything horizontally, for the slide
 * transition that lands in F11 -- always 0 for now. */
void metro_draw_pivots(const struct metro_page *page, int active_pivot,
                        int x_offset);

/* Row list at y=84.. (PLAN_MAESTRO.md S1.4): `first` is the first
 * visible row (metro_nav_first_visible()), `sel` the selected one
 * (metro_nav_sel()) drawn in MFONT_LIST_SEL/metro_color_fg(), the
 * rest in MFONT_LIST/metro_color_secondary(). Draws one row past the
 * visible window on purpose -- it peeks, cut by the bottom of the
 * screen, exactly like the pivot header. x_offset: see
 * metro_draw_pivots(). */
void metro_draw_rows(const struct metro_pivot *pivot, int first, int sel,
                      int x_offset);

/* Flat bar: metro_color_tertiary() background, metro_color_accent()
 * fill for the first `pct` percent (0..100, clamped). Now Playing's
 * progress bar (PLAN_MAESTRO.md S1.4: 320x4 at y=214), reusable
 * anywhere else a determinate progress needs showing. */
void metro_draw_progress(int x, int y, int width, int height, int pct);

/* metro_color_accent() square with a single big uppercase initial
 * (first byte of `label`, ASCII only) centered in MFONT_DISPLAY/bg --
 * the no-album-art fallback (PLAN_MAESTRO.md S1.4 "tile acento con
 * inicial del album"). */
void metro_draw_tile(int x, int y, int size, const char *label);

#endif /* METRO_DRAW_H */
