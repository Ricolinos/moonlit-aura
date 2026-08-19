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
#ifndef METRO_SCREEN_SPECIMEN_H
#define METRO_SCREEN_SPECIMEN_H

/* F2 only: temporary boot screen exercising the 5 font roles, the
 * right-edge clip primitives, the 10 accent colors, and the header
 * (hour + battery) -- see PLAN_MAESTRO.md F2. Superseded by the real
 * hub screen (metro_screen_hub) once F3 lands; the file stays until
 * then as a visual regression reference. */
void metro_screen_specimen_show(void);

#endif /* METRO_SCREEN_SPECIMEN_H */
