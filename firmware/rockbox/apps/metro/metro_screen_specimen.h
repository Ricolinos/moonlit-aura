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

/* Visual regression reference for the 7 MD3 type roles (M2,
 * design-system/tokens.json:type_scale) and the header (hour +
 * battery). One line per role, same pangram-ish string, so
 * firmware/tools/check_fonts.py --capheight can measure cap-height
 * per row mechanically (D-006). */
void metro_screen_specimen_show(void);

#endif /* METRO_SCREEN_SPECIMEN_H */
