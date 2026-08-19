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
#include <stddef.h>
#include "lcd.h"
#include "kernel.h"

#include "metro_screen_list.h"
#include "metro_draw.h"
#include "metro_lang.h"
#include "metro_widgets.h"

static metro_nav_t s_nav;

/* F10: floating index letter (S1.4) -- shown for 600ms after a fast
 * scroll (steps >= 3) lands on a new row, using that row's own first
 * character. Deliberately without the plan's "count >= 40" gate
 * (DESVIACIONES.md F10-1): none of Metro's real lists have that many
 * items with the test fixtures this session can generate, and a big
 * jump on a shorter list still benefits from the same feedback. */
#define METRO_INDEX_LETTER_TICKS (HZ * 3 / 5)
#define METRO_INDEX_LETTER_MIN_STEPS 3

static long s_index_letter_until = 0;
static char s_index_letter = '\0';

/* page_stack[d-1] holds the page pushed at nav depth d, for d>=2 --
 * index 0 (depth 1, the hub) is never used, metro_screen_hub.c owns
 * its own pivot and only borrows s_nav's depth-1 frame for its
 * selection/windowing state. */
static const struct metro_page *page_stack[METRO_NAV_MAX_DEPTH];

void metro_screen_list_init(void)
{
    metro_nav_init(&s_nav, 1);
}

metro_nav_t *metro_screen_nav(void)
{
    return &s_nav;
}

bool metro_screen_list_push(const struct metro_page *page)
{
    if (!metro_nav_push(&s_nav, page->npivots))
        return false;

    page_stack[metro_nav_depth(&s_nav) - 1] = page;
    return true;
}

bool metro_screen_list_pop(void)
{
    return metro_nav_pop(&s_nav);
}

void metro_screen_list_pop_to_root(void)
{
    metro_nav_pop_to_root(&s_nav);
}

static const struct metro_page *current_page(void)
{
    int depth = metro_nav_depth(&s_nav);

    if (depth < 2)
        return NULL;

    return page_stack[depth - 1];
}

const struct metro_page *metro_screen_list_current_page(void)
{
    return current_page();
}

bool metro_screen_list_has_pending_redraw(void)
{
    return current_tick < s_index_letter_until;
}

void metro_screen_list_show(void)
{
    const struct metro_page *page = current_page();
    const struct metro_pivot *pivot;
    int active;

    if (!page)
        return;

    active = metro_nav_pivot(&s_nav);
    pivot = &page->pivots[active];

    metro_draw_clear();
    metro_draw_header(page->title_dynamic ? page->title_dynamic
                                           : metro_lang_str(page->title));
    metro_draw_pivots(page, active, 0);

    if (pivot->count(pivot->ctx) == 0)
        metro_widgets_draw_empty_state(metro_lang_str(LANG_EMPTY_LIST));
    else
        metro_draw_rows(pivot, metro_nav_first_visible(&s_nav), metro_nav_sel(&s_nav), 0);

    if (current_tick < s_index_letter_until)
        metro_widgets_draw_index_letter(s_index_letter);

    lcd_update();
}

void metro_screen_list_handle(int action, int steps)
{
    const struct metro_page *page = current_page();
    const struct metro_pivot *pivot;
    int count;

    if (!page)
        return;

    pivot = &page->pivots[metro_nav_pivot(&s_nav)];
    count = pivot->count(pivot->ctx);

    switch (action)
    {
        case MACT_PREV:
        case MACT_NEXT:
            metro_nav_move_sel(&s_nav, action == MACT_PREV ? -steps : steps,
                                count, METRO_DRAW_ROWS_VISIBLE);
            if (steps >= METRO_INDEX_LETTER_MIN_STEPS && count > 0)
            {
                struct metro_row row;
                pivot->get_row(pivot->ctx, metro_nav_sel(&s_nav), &row);
                s_index_letter = row.title && row.title[0]
                                      ? (char)(row.title[0] >= 'a' && row.title[0] <= 'z'
                                                    ? row.title[0] - 32 : row.title[0])
                                      : '?';
                s_index_letter_until = current_tick + METRO_INDEX_LETTER_TICKS;
            }
            break;
        case MACT_PIVOT_PREV:
            metro_nav_pivot_prev(&s_nav);
            break;
        case MACT_PIVOT_NEXT:
            metro_nav_pivot_next(&s_nav);
            break;
        case MACT_SELECT:
            if (pivot->on_select)
                pivot->on_select(pivot->ctx, metro_nav_sel(&s_nav));
            break;
        case MACT_BACK:
            metro_screen_list_pop();
            break;
        case MACT_HOME:
            metro_screen_list_pop_to_root();
            break;
        default:
            break;
    }
}
