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
#ifndef METRO_SCREEN_SPLASH_H
#define METRO_SCREEN_SPLASH_H

/* F9: the real splash (PLAN_MAESTRO.md S1.4) -- centered "metro"
 * wordmark in MFONT_DISPLAY (needs metro_fonts_init()/metro_theme_init()
 * already run, unlike F1's placeholder which drew before either
 * existed) plus a thin 120x2 accent bar underneath. No bitmap logo --
 * consistent with every other Metro screen so far (F5-1: text/font
 * rendering only, compiled icons land in F10). */
void metro_screen_splash_show(void);

/* Redraws the same wordmark with the bar filled to `pct` (0..100) --
 * metro_main() calls this in a short loop while waiting for
 * tagcache_is_fully_initialized() at boot (S1.4: "mientras
 * !tagcache_is_usable()/fuentes cargan" -- fonts are already loaded by
 * the time this runs, tagcache's initial "is there already a
 * database" determination is the part actually worth showing progress
 * for, not a full rebuild/scan -- that has its own screen, F6). */
void metro_screen_splash_progress(int pct);

#endif /* METRO_SCREEN_SPLASH_H */
