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
/* Reads /.rockbox/aura/device.cfg (device_name only -- Metro never
 * writes this file, only Aura Studio does). Reload on boot and after
 * every USB disk handoff (metro_main.c), same two moments as the sync
 * marker. */
#ifndef METRO_DEVICE_H
#define METRO_DEVICE_H

void metro_device_reload(void);

/* NULL if device.cfg is absent or has no usable device_name -- the
 * caller falls back to a generic label ("about" screen, F8). */
const char *metro_device_name(void);

#endif /* METRO_DEVICE_H */
