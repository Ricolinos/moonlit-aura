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
 * Metro UI -- replacement UI layer for this Rockbox fork (see
 * MODIFICATIONS.md, DECISIONS.md M-006 in the repository root).
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
#ifndef METRO_MAIN_H
#define METRO_MAIN_H

#include "config.h"
#include "gcc_extensions.h"

/* Single entry point of the Metro UI, called once from apps/main.c
 * instead of root_menu() -- never returns. root_menu.c stays compiled
 * (other core settings still reference it) but is never invoked. */
void metro_main(void) NORETURN_ATTR;

/* One-time override of global_settings inherited from stock Rockbox,
 * tuned for its own UI rather than a closed appliance (DECISIONS.md
 * M-019). Must run inside apps/main.c's init(), right after
 * settings_load() and before settings_apply(true)/settings_apply_skins()
 * -- settings_apply_skins() is what actually paints the Cabbie v2
 * backdrop onto the LCD by reading global_settings.backdrop_file at
 * that point; overriding it later (e.g. at the top of metro_main(),
 * which runs after init() returns) is too late and the stock backdrop
 * already shows through every lcd_clear_display() call. */
void metro_apply_hygiene(void);

#endif /* METRO_MAIN_H */
