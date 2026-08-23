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
#ifndef METRO_SCREEN_HUB_H
#define METRO_SCREEN_HUB_H

/* Root of the twist (PLAN_MAESTRO.md S1.4 "Hub"): a plain vertical
 * list in MFONT_DISPLAY, no pivots. Always nav depth 1 -- shares
 * metro_screen_nav()'s depth-1 frame for its own selection/windowing
 * instead of owning a separate metro_nav_t. Selecting music pushes
 * either metro_music's real tagcache-backed page or a plain "updating
 * library..." placeholder while metro_music_db_ready() is still
 * false (F4); videos/photos still push dummy 30-row data (F7 replaces
 * those). Selecting settings pushes metro_screen_settings_page(). A
 * 5th row ("now playing") appears above the other four whenever
 * metro_music_is_playing() -- selecting it pushes a plain-text now
 * playing placeholder (F5 replaces it with the real screen). */
void metro_screen_hub_show(void);
void metro_screen_hub_handle(int action, int steps);

/* R5-F5 (M-085): advances the "now playing" row's own animation
 * (marquee while playing, breathing while paused) and repaints ONLY
 * that row. metro_main.c calls it at ~20 Hz while the hub is the
 * current screen and there is audio; it returns false -- and does
 * nothing -- when there is nothing to animate (no audio, row scrolled
 * out of view, animations=off, LCD asleep), so the caller can drop back
 * to its idle cadence. Never called by show() itself. */
bool metro_screen_hub_tick(void);
/* Same predicate tick() uses, without drawing -- for the main loop to
 * shorten its input wait while there is something to animate. */
bool metro_screen_hub_wants_ticks(void);

#endif /* METRO_SCREEN_HUB_H */
