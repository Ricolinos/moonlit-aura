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
#include "metro_volume.h"

static int clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

int metro_volume_db_from_level(int level, int min_db, int max_db)
{
    int span = max_db - min_db;

    level = clamp(level, 0, METRO_VOLUME_MAX_LEVEL);
    if (span <= 0)
        return min_db;

    /* Nearest-dB rounding of min + level*span/15. Integer-only and
     * symmetric with level_from_db below, so the round trip holds. */
    return min_db + (level * span + METRO_VOLUME_MAX_LEVEL / 2) / METRO_VOLUME_MAX_LEVEL;
}

int metro_volume_level_from_db(int db, int min_db, int max_db)
{
    int span = max_db - min_db;
    int best = 0, best_err = -1, level;

    if (span <= 0)
        return 0;
    db = clamp(db, min_db, max_db);

    /* 16 candidates: pick the level whose own dB is closest. Brute
     * force on purpose -- it is the definition of "nearest level" and
     * makes the round-trip guarantee true by construction, instead of
     * depending on two rounding formulas agreeing. */
    for (level = 0; level < METRO_VOLUME_LEVELS; level++)
    {
        int err = metro_volume_db_from_level(level, min_db, max_db) - db;
        if (err < 0)
            err = -err;
        if (best_err < 0 || err < best_err)
        {
            best = level;
            best_err = err;
        }
    }
    return best;
}
