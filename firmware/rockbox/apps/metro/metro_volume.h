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
#ifndef METRO_VOLUME_H
#define METRO_VOLUME_H

/* R5-F3 (M-083): Metro shows and sets volume as 16 levels, "00" (the
 * quietest the codec allows) to "15" (the loudest), never in dB -- the
 * owner's spec for the player redesign. These two functions are the
 * whole mapping between that scale and Rockbox's native dB range
 * (sound_min/sound_max of SOUND_VOLUME: -60..+12 on the iPod 6G).
 * Pure integer math, no Rockbox dependencies -- host-tested in
 * test/test_volume.c.
 *
 * Round-trip guarantee: level_from_db(db_from_level(L)) == L for every
 * L in 0..METRO_VOLUME_LEVELS-1, whatever the min/max range is. That
 * is what lets the player keep NO level state of its own -- the level
 * is always derived from global_status.volume, which Rockbox already
 * persists -- and still step exactly one level per wheel notch. */

#define METRO_VOLUME_LEVELS 16
#define METRO_VOLUME_MAX_LEVEL (METRO_VOLUME_LEVELS - 1)

/* dB value for a level (clamped to 0..15). Level 0 is exactly min_db,
 * level 15 exactly max_db; the rest spread linearly with rounding to
 * the nearest dB. */
int metro_volume_db_from_level(int level, int min_db, int max_db);

/* Nearest level for a dB value (clamped to min_db..max_db). */
int metro_volume_level_from_db(int db, int min_db, int max_db);

#endif /* METRO_VOLUME_H */
