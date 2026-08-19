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
/* Rewrites apps/gui/splash.c's messages that come from the Rockbox
 * tree Metro doesn't control (tagcache, playlist/plugin loader,
 * low-battery shutdown) to Metro's own wording -- direct port of
 * Aura-Firmware's aura_splash_lang.c (same messages, same mechanism:
 * splash() is the one generic hook every one of these paths already
 * goes through, so translating there covers all of them without
 * touching each call site). See DECISIONS.md and MODIFICATIONS.md for
 * the one-line hook in apps/gui/splash.c itself. */
#ifndef METRO_SPLASH_LANG_H
#define METRO_SPLASH_LANG_H

#include <stddef.h>

/* Rewrites `buf` in place if its content matches (whole string or
 * prefix) one of the known Rockbox messages, in Metro's active
 * language. Leaves `buf` untouched if there's no match -- showing the
 * real text (even off-tone) beats replacing it with a generic message
 * that hides diagnostic information Metro didn't anticipate. */
void metro_splash_translate(char *buf, size_t bufsz);

#endif /* METRO_SPLASH_LANG_H */
