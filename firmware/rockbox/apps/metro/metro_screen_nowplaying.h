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
#ifndef METRO_SCREEN_NOWPLAYING_H
#define METRO_SCREEN_NOWPLAYING_H

#include <stdbool.h>

/* Now Playing (PLAN_MAESTRO.md S1.4, S2.2): a custom screen, not a
 * generic metro_page list -- album art, transport, a progress bar,
 * nothing here is "pivots + rows". It still rides the shared
 * metro_nav_t stack for BACK/HOME to work exactly like everywhere
 * else (metro_screen_nowplaying_push() pushes a real, otherwise-empty
 * page as a sentinel so depth/pop bookkeeping stays in
 * metro_screen_list.c, which never has to know Now Playing exists).
 *
 * F4's plain-text placeholder page is gone -- every place that used to
 * push it (metro_screen_hub.c, after starting playback) now calls
 * metro_screen_nowplaying_push() instead. */

/* Pushes the Now Playing sentinel page. Same false-on-full-stack
 * contract as metro_screen_list_push(). */
bool metro_screen_nowplaying_push(void);

/* True when the page on top of metro_screen_list's stack is Now
 * Playing's sentinel -- metro_main.c uses this to route drawing/input
 * here instead of to metro_screen_list, and to pick MCTX_PLAYER
 * instead of MCTX_LIST. */
bool metro_screen_nowplaying_is_current(void);

void metro_screen_nowplaying_show(void);
void metro_screen_nowplaying_handle(int action, int steps);

#endif /* METRO_SCREEN_NOWPLAYING_H */
