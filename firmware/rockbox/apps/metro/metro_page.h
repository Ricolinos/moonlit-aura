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
#ifndef METRO_PAGE_H
#define METRO_PAGE_H

#include "metro_lang.h"

/* Declarative model for any twist page (PLAN_MAESTRO.md S1.1 point 3):
 * a page is a title + a small array of pivots; each pivot is a
 * provider (count/get_row/on_select), not a data copy. The generic
 * list screen (metro_screen_list.c) can render and drive ANY page
 * built from this without knowing what it actually contains --
 * artists, videos, settings rows, whatever.
 *
 * Page titles and pivot names are enum metro_lang_id, NOT a resolved
 * const char* -- these tables are static const arrays (compile-time
 * initializers), and metro_lang_str() is a function call, not a
 * constant expression, so it can't appear in one. Whoever draws a
 * page/pivot resolves the id at that moment
 * (metro_draw_header()/metro_draw_pivots() do this internally), which
 * is also what makes metro_lang_set() take effect immediately on the
 * next redraw. Row title/subtitle have no such restriction -- get_row()
 * runs at draw time, so it can call metro_lang_str() directly. */

enum metro_row_kind {
    METRO_ROW_NAV,     /* selecting it pushes a new page */
    METRO_ROW_ACTION,  /* selecting it performs an action, no push */
    METRO_ROW_SETTING, /* title = setting name, subtitle = current value */
};

struct metro_row {
    const char *title;
    const char *subtitle; /* NULL if none */
    enum metro_row_kind kind;
};

struct metro_pivot {
    enum metro_lang_id name;
    int  (*count)(void *ctx);
    void (*get_row)(void *ctx, int index, struct metro_row *out);
    /* Called on SELECT over row `index`. Free to do nothing, mutate
     * ctx (METRO_ROW_SETTING: cycle a value), or push a child page
     * via metro_screen_list_push() (METRO_ROW_NAV). */
    void (*on_select)(void *ctx, int index);
    void *ctx;
};

struct metro_page {
    enum metro_lang_id title;
    const struct metro_pivot *pivots;
    int npivots;
};

#endif /* METRO_PAGE_H */
