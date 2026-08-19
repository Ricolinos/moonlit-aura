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
 * instead of owning a separate metro_nav_t. Selecting music/videos/
 * photos pushes a page of dummy 30-row data (F3 -- F4 replaces the
 * providers with real tagcache-backed ones, nothing else changes);
 * selecting settings pushes metro_screen_settings_page(). */
void metro_screen_hub_show(void);
void metro_screen_hub_handle(int action, int steps);

#endif /* METRO_SCREEN_HUB_H */
