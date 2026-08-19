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
#include "metro_nav.h"

static void reset_frame(struct metro_nav_frame *f, int npivots)
{
    int i;

    if (npivots < 1)
        npivots = 1;
    if (npivots > METRO_NAV_MAX_PIVOTS)
        npivots = METRO_NAV_MAX_PIVOTS;

    f->npivots = npivots;
    f->pivot = 0;
    for (i = 0; i < METRO_NAV_MAX_PIVOTS; i++)
    {
        f->sel[i] = 0;
        f->first_visible[i] = 0;
    }
}

void metro_nav_init(metro_nav_t *nav, int root_npivots)
{
    nav->depth = 1;
    reset_frame(&nav->frames[0], root_npivots);
}

int metro_nav_depth(const metro_nav_t *nav)
{
    return nav->depth;
}

bool metro_nav_is_root(const metro_nav_t *nav)
{
    return nav->depth == 1;
}

bool metro_nav_push(metro_nav_t *nav, int npivots)
{
    if (nav->depth >= METRO_NAV_MAX_DEPTH)
        return false;

    reset_frame(&nav->frames[nav->depth], npivots);
    nav->depth++;
    return true;
}

bool metro_nav_pop(metro_nav_t *nav)
{
    if (nav->depth <= 1)
        return false;

    nav->depth--;
    return true;
}

void metro_nav_pop_to_root(metro_nav_t *nav)
{
    nav->depth = 1;
}

static struct metro_nav_frame *current_frame(metro_nav_t *nav)
{
    return &nav->frames[nav->depth - 1];
}

static const struct metro_nav_frame *current_frame_c(const metro_nav_t *nav)
{
    return &nav->frames[nav->depth - 1];
}

int metro_nav_pivot_count(const metro_nav_t *nav)
{
    return current_frame_c(nav)->npivots;
}

int metro_nav_pivot(const metro_nav_t *nav)
{
    return current_frame_c(nav)->pivot;
}

bool metro_nav_pivot_next(metro_nav_t *nav)
{
    struct metro_nav_frame *f = current_frame(nav);

    if (f->pivot + 1 >= f->npivots)
        return false;

    f->pivot++;
    return true;
}

bool metro_nav_pivot_prev(metro_nav_t *nav)
{
    struct metro_nav_frame *f = current_frame(nav);

    if (f->pivot <= 0)
        return false;

    f->pivot--;
    return true;
}

int metro_nav_sel(const metro_nav_t *nav)
{
    const struct metro_nav_frame *f = current_frame_c(nav);
    return f->sel[f->pivot];
}

int metro_nav_first_visible(const metro_nav_t *nav)
{
    const struct metro_nav_frame *f = current_frame_c(nav);
    return f->first_visible[f->pivot];
}

static void clamp_window(struct metro_nav_frame *f, int row_count, int visible_rows)
{
    int p = f->pivot;
    int max_first;

    if (f->sel[p] < f->first_visible[p])
        f->first_visible[p] = f->sel[p];
    else if (visible_rows > 0 && f->sel[p] >= f->first_visible[p] + visible_rows)
        f->first_visible[p] = f->sel[p] - visible_rows + 1;

    max_first = row_count - visible_rows;
    if (max_first < 0)
        max_first = 0;
    if (f->first_visible[p] > max_first)
        f->first_visible[p] = max_first;
    if (f->first_visible[p] < 0)
        f->first_visible[p] = 0;
}

void metro_nav_set_sel(metro_nav_t *nav, int index, int row_count, int visible_rows)
{
    struct metro_nav_frame *f = current_frame(nav);
    int p = f->pivot;

    if (row_count <= 0)
    {
        f->sel[p] = 0;
        f->first_visible[p] = 0;
        return;
    }

    if (index < 0)
        index = 0;
    if (index > row_count - 1)
        index = row_count - 1;

    f->sel[p] = index;
    clamp_window(f, row_count, visible_rows);
}

void metro_nav_move_sel(metro_nav_t *nav, int delta, int row_count, int visible_rows)
{
    int current = metro_nav_sel(nav);
    metro_nav_set_sel(nav, current + delta, row_count, visible_rows);
}
