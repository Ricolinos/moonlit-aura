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
/* moonlit (D-049): "preparando biblioteca" -- the ONE blocking screen
 * between the hub's Música row and the Música page. Measured on the
 * owner's iPod (4 556 tracks, ~1 083 albums): the cover pre-pass used
 * to run synchronously inside metro_music_db_ready() with no screen
 * and no buttons for 4 min 18 s. This screen owns that time instead:
 *
 *   phase 1 (only while the tagcache is not usable yet): "construyendo
 *           la base de música", progress = tagcache commit steps;
 *   phase 2 (only if moonlit_art_pending_count() > 0): "preparando
 *           carátulas", progress = covers decoded / covers missing,
 *           repainted every 4 albums (lcd_update() costs more than a
 *           small decode, AF/aura_music.c:376-380).
 *
 * Blocking on purpose (its own input loop, same shape as
 * metro_run_sync_screen_if_needed(), metro_main.c) but interruptible:
 * MENU/back returns false and leaves whatever is pending for the next
 * visit -- both phases are idempotent, nothing is lost. A USB
 * connection is handed to default_event_handler() and also returns
 * false. Returns true when there was nothing to do or everything
 * finished (in which case it drew nothing at all if both phases were
 * already satisfied). */
#ifndef MOONLIT_SCREEN_LIBRARY_H
#define MOONLIT_SCREEN_LIBRARY_H

#include <stdbool.h>

bool moonlit_screen_library_prepare(void);

#endif /* MOONLIT_SCREEN_LIBRARY_H */
