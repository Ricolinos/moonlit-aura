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
/* Row provider for Settings' "about" pivot (PLAN_MAESTRO.md S2.2) --
 * not a page/screen of its own, "about" lives inside metro_page.h's
 * regular settings_page like "general"/"display" always have. Reads
 * device.cfg (metro_device.c, F6) and sync_summary.cfg
 * (metro_manifest.c, F8) fresh on every row draw -- both are tiny
 * files read rarely (Settings isn't a hot path), no caching needed. */
#ifndef METRO_SCREEN_ABOUT_H
#define METRO_SCREEN_ABOUT_H

#include "metro_page.h"

extern const struct metro_pivot metro_screen_about_pivot;

#endif /* METRO_SCREEN_ABOUT_H */
