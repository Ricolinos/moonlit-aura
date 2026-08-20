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
#ifndef METRO_WIDGETS_H
#define METRO_WIDGETS_H

#include <stdbool.h>

/* The system's single modal dialog (PLAN_MAESTRO.md S2.3, MCTX_DIALOG):
 * blocks with its own small input loop (MCTX_DIALOG context) until the
 * user confirms or cancels. Defaults the selection to "no" -- the safe
 * answer -- never to "yes". Redraws the whole screen; the caller must
 * redraw whatever was behind it afterwards. */
bool metro_widgets_confirm(const char *title, const char *question);

/* Volume overlay (PLAN_MAESTRO.md S1.4): a 320x6 bar at y=232 plus a
 * "volume NN%" caption -- pure draw, no input loop of its own.
 * metro_screen_nowplaying.c calls this for ~1.5s after a wheel move on
 * the Now Playing screen and decides on its own when to stop calling
 * it (this widget has no timer state, it just draws one frame). */
void metro_widgets_draw_volume_overlay(int pct);

/* F10: floating index letter, shown ~600ms after a fast scroll (steps
 * >= 3) stops moving -- PLAN_MAESTRO.md S1.4. `c` is the first
 * character of the row the selection landed on, uppercased by the
 * caller. Draws over whatever the list already shows (a solid
 * MFONT_DISPLAY-sized accent square, centered) -- the caller decides
 * when to stop calling it and let the next real redraw clear it. */
void metro_widgets_draw_index_letter(char c);

/* Empty-list state (PLAN_MAESTRO.md S1.4): a centered 96x96 accent
 * tile (blank -- metro_draw_tile() with no letter, this isn't naming
 * anything) plus a caption message below it. Callers: any pivot whose
 * count() is 0. */
void metro_widgets_draw_empty_state(const char *message);

/* F10: Now Playing's shuffle/repeat status icons (M-018's icon set),
 * replacing F5-1's deferred "shuffle"/"repeat todo" text badges --
 * simple line-art, top-left corner at (x, y), METRO_WIDGETS_ICON_SIZE
 * square. Repeat draws a small "1" centered over the loop when `one`
 * is true (REPEAT_ONE vs REPEAT_ALL). */
#define METRO_WIDGETS_ICON_SIZE 16

void metro_widgets_draw_shuffle_icon(int x, int y);

/* R4/FA-6 (M-073): glifos de transporte, METRO_WIDGETS_ICON_SIZE de
 * lado. **Misma geometría exacta** que el OSD del reproductor de video
 * (`draw_status_icon()`/`draw_tri_stepped()`,
 * apps/plugins/mpegplayer/mpegplayer.c:796-845) -- se porta en vez de
 * inventar otra precisamente para que el mismo estado se dibuje igual
 * en música y en video. El triángulo se traza columna por columna (no
 * con una primitiva de polígono, que Rockbox no tiene) y por eso
 * queda "escalonado" a propósito: a 16 px se lee como triángulo y
 * encaja con el lenguaje anguloso de Metro. */
void metro_widgets_draw_play_icon(int x, int y, unsigned color);
void metro_widgets_draw_pause_icon(int x, int y, unsigned color);
void metro_widgets_draw_repeat_icon(int x, int y, bool one);

#endif /* METRO_WIDGETS_H */
