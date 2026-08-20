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
#ifndef METRO_NAV_H
#define METRO_NAV_H

#include <stdbool.h>

/* Pure navigation-stack math for the twist skeleton: depth (push/pop),
 * horizontal pivot position within a level, and vertical selection +
 * windowing within the active pivot. Knows nothing about screens,
 * fonts or Rockbox -- compiles and runs host-side
 * (apps/metro/test/test_nav.c), same pattern as Aura-Firmware's
 * aura_nav.c. The real, single instance used by the app lives in
 * metro_screen_list.c; every function here takes an explicit pointer
 * so tests can create as many independent instances as they want.
 *
 * See PLAN_MAESTRO.md S2.1: depth 1 is always the hub (root); push()
 * enters a sub-page, pop() leaves it and restores exactly where the
 * user was (pivot + selection), per level -- matching the Zune 30. */

#define METRO_NAV_MAX_DEPTH  8
/* R3-F4/DD-5 (M-065): 6->8 -- Música gana un 6to pivot (Quickplay), and
 * a little headroom past exactly-6 so a future pivot doesn't need
 * another one-line bump right away. */
#define METRO_NAV_MAX_PIVOTS 8

struct metro_nav_frame {
    int npivots;
    int pivot;
    int sel[METRO_NAV_MAX_PIVOTS];
    int first_visible[METRO_NAV_MAX_PIVOTS];
};

typedef struct {
    struct metro_nav_frame frames[METRO_NAV_MAX_DEPTH];
    int depth; /* always >= 1 after metro_nav_init() */
} metro_nav_t;

/* root_npivots: how many pivots the hub itself has (1 -- it's a plain
 * list, not a twist page, but reuses the same sel/windowing slot). */
void metro_nav_init(metro_nav_t *nav, int root_npivots);

int  metro_nav_depth(const metro_nav_t *nav);
bool metro_nav_is_root(const metro_nav_t *nav);

/* Pushes a new frame with npivots pivots (pivot 0, all selections at
 * 0). Returns false without changing anything if already at
 * METRO_NAV_MAX_DEPTH or npivots is out of range. */
bool metro_nav_push(metro_nav_t *nav, int npivots);

/* Pops the current frame, restoring the parent's pivot/selection
 * exactly as left. Returns false (no-op) if already at the root. */
bool metro_nav_pop(metro_nav_t *nav);
void metro_nav_pop_to_root(metro_nav_t *nav);

int  metro_nav_pivot_count(const metro_nav_t *nav);
int  metro_nav_pivot(const metro_nav_t *nav);
/* No wrap at either end (Zune Pivot behaviour, F.1) -- return false if
 * already at the first/last pivot. */
bool metro_nav_pivot_next(metro_nav_t *nav);
bool metro_nav_pivot_prev(metro_nav_t *nav);

int  metro_nav_sel(const metro_nav_t *nav);
int  metro_nav_first_visible(const metro_nav_t *nav);

/* Moves the selection of the CURRENT pivot by delta, clamped to
 * [0, row_count-1] (row_count == 0 leaves sel at 0), and adjusts
 * first_visible so the selection stays within a window of
 * visible_rows rows (PLAN_MAESTRO.md S1.4 "ventaneo"). */
void metro_nav_move_sel(metro_nav_t *nav, int delta, int row_count, int visible_rows);
void metro_nav_set_sel(metro_nav_t *nav, int index, int row_count, int visible_rows);

/* R2-F2/DD-7: grid counterpart of metro_nav_move_sel() for pivots with
 * tile_cols > 0 -- delta still moves the LINEAR index by one tile at a
 * time (wheel = next/previous tile in reading order, same as a plain
 * list, not a column jump), clamped to [0, count-1]. The difference is
 * windowing: first_visible tracks whole GRID ROWS (count/cols each),
 * kept within [0, ceil(count/cols)-visible_rows] and always a multiple
 * of cols -- metro_draw_tiles() depends on that alignment to map
 * first_visible straight to a (row, col) start with no remainder
 * bookkeeping. metro_nav_move_sel() is untouched; a pivot never mixes
 * both in the same frame slot (tile_cols is fixed per pivot). */
void metro_nav_move_sel_grid(metro_nav_t *nav, int delta, int count, int cols,
                              int visible_rows);

#endif /* METRO_NAV_H */
