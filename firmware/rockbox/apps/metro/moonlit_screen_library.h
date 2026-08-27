/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* moonlit (D-049, D-059): "preparando biblioteca" -- the ONE blocking
 * screen between the hub's Música row and the Música page, now down
 * to a single phase: "construyendo la base de música" while the
 * tagcache is not usable yet, progress = tagcache commit steps.
 *
 * D-059: the second phase this screen used to own ("preparando
 * carátulas"/"revisando carátulas", D-056/D-058 -- covers decoded /
 * covers missing) is gone. Measured on the owner's iPod (4 556
 * tracks, ~1 083 albums) that synchronous cover pre-pass used to block
 * for 4 min 18 s with no screen and no buttons at all; the fix since
 * D-059 is not a bigger progress bar but making the wait disappear:
 * moonlit_master_art_builder.c walks the library on its own
 * background thread once the database is ready, and Marea/the grids
 * show the monogram/placeholder for whatever it hasn't reached yet,
 * repainting on their own tick() when it does. Nothing here waits on
 * it anymore.
 *
 * Blocking on purpose (its own input loop, same shape as
 * metro_run_sync_screen_if_needed(), metro_main.c) but interruptible:
 * MENU/back returns false and leaves the tagcache build for the next
 * visit -- idempotent, nothing is lost. A USB connection is handed to
 * default_event_handler() and also returns false. Returns true when
 * there was nothing to do or the build finished (in which case it drew
 * nothing at all if the database was already usable). */
#ifndef MOONLIT_SCREEN_LIBRARY_H
#define MOONLIT_SCREEN_LIBRARY_H

#include <stdbool.h>

bool moonlit_screen_library_prepare(void);

#endif /* MOONLIT_SCREEN_LIBRARY_H */
