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
#ifndef METRO_SCREEN_SETTINGS_H
#define METRO_SCREEN_SETTINGS_H

#include "metro_page.h"

/* Settings page skeleton (PLAN_MAESTRO.md F3): general | display |
 * about pivots, values held in RAM only (metro_theme.c/metro_lang.c),
 * nothing persisted yet -- that's metro_settings.c's job (F6/F8),
 * which will read aura.cfg once at boot and call
 * metro_theme_set()/metro_lang_set() with the saved values; nothing
 * here changes when that lands. "reset settings" already wires
 * metro_widgets_confirm() end to end, even though there's nothing
 * real to reset yet. */
const struct metro_page *metro_screen_settings_page(void);

#endif /* METRO_SCREEN_SETTINGS_H */
