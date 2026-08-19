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
#include <stdio.h>
#include <stdbool.h>
#include "lcd.h"

#include "metro_screen_hub.h"
#include "metro_screen_list.h"
#include "metro_screen_settings.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_keymap.h"

#define METRO_HUB_FIRST_Y 32
#define METRO_HUB_PITCH   52
#define METRO_HUB_VISIBLE 4

/* --- dummy row provider, shared by every music/videos/photos pivot ----
 * F3 only needs to prove pivots + windowing work (PLAN_MAESTRO.md F3
 * "listas de datos ficticios de 30 filas"); F4 replaces count/get_row
 * with real tagcache-backed providers and deletes this, nothing else
 * in metro_screen_list.c/metro_draw.c changes. */

struct dummy_ctx {
    enum metro_lang_id name_id;
    int count;
    char buf[32];
};

static int dummy_count(void *ctx)
{
    return ((struct dummy_ctx *)ctx)->count;
}

static void dummy_get_row(void *ctx, int index, struct metro_row *out)
{
    struct dummy_ctx *d = ctx;

    snprintf(d->buf, sizeof(d->buf), "%s %d", metro_lang_str(d->name_id), index + 1);
    out->title = d->buf;
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void dummy_on_select(void *ctx, int index)
{
    (void)ctx;
    (void)index;
}

#define DUMMY_ROWS 30

static struct dummy_ctx music_artists_ctx   = { LANG_PIVOT_ARTISTS,   DUMMY_ROWS, "" };
static struct dummy_ctx music_albums_ctx    = { LANG_PIVOT_ALBUMS,    DUMMY_ROWS, "" };
static struct dummy_ctx music_songs_ctx     = { LANG_PIVOT_SONGS,     DUMMY_ROWS, "" };
static struct dummy_ctx music_genres_ctx    = { LANG_PIVOT_GENRES,    DUMMY_ROWS, "" };
static struct dummy_ctx music_playlists_ctx = { LANG_PIVOT_PLAYLISTS, DUMMY_ROWS, "" };

static const struct metro_pivot music_pivots[] = {
    { LANG_PIVOT_ARTISTS,   dummy_count, dummy_get_row, dummy_on_select, &music_artists_ctx },
    { LANG_PIVOT_ALBUMS,    dummy_count, dummy_get_row, dummy_on_select, &music_albums_ctx },
    { LANG_PIVOT_SONGS,     dummy_count, dummy_get_row, dummy_on_select, &music_songs_ctx },
    { LANG_PIVOT_GENRES,    dummy_count, dummy_get_row, dummy_on_select, &music_genres_ctx },
    { LANG_PIVOT_PLAYLISTS, dummy_count, dummy_get_row, dummy_on_select, &music_playlists_ctx },
};
static const struct metro_page music_page = { LANG_HUB_MUSIC, music_pivots, 5 };

static struct dummy_ctx videos_all_ctx    = { LANG_PIVOT_ALL,    DUMMY_ROWS, "" };
static struct dummy_ctx videos_movies_ctx = { LANG_PIVOT_MOVIES, DUMMY_ROWS, "" };
static struct dummy_ctx videos_series_ctx = { LANG_PIVOT_SERIES, DUMMY_ROWS, "" };
static struct dummy_ctx videos_clips_ctx  = { LANG_PIVOT_CLIPS,  DUMMY_ROWS, "" };

static const struct metro_pivot videos_pivots[] = {
    { LANG_PIVOT_ALL,    dummy_count, dummy_get_row, dummy_on_select, &videos_all_ctx },
    { LANG_PIVOT_MOVIES, dummy_count, dummy_get_row, dummy_on_select, &videos_movies_ctx },
    { LANG_PIVOT_SERIES, dummy_count, dummy_get_row, dummy_on_select, &videos_series_ctx },
    { LANG_PIVOT_CLIPS,  dummy_count, dummy_get_row, dummy_on_select, &videos_clips_ctx },
};
static const struct metro_page videos_page = { LANG_HUB_VIDEOS, videos_pivots, 4 };

static struct dummy_ctx photos_all_ctx    = { LANG_PIVOT_ALL,    DUMMY_ROWS, "" };
static struct dummy_ctx photos_photos_ctx = { LANG_PIVOT_PHOTOS, DUMMY_ROWS, "" };
static struct dummy_ctx photos_images_ctx = { LANG_PIVOT_IMAGES, DUMMY_ROWS, "" };
static struct dummy_ctx photos_ai_ctx     = { LANG_PIVOT_AI,     DUMMY_ROWS, "" };

static const struct metro_pivot photos_pivots[] = {
    { LANG_PIVOT_ALL,    dummy_count, dummy_get_row, dummy_on_select, &photos_all_ctx },
    { LANG_PIVOT_PHOTOS, dummy_count, dummy_get_row, dummy_on_select, &photos_photos_ctx },
    { LANG_PIVOT_IMAGES, dummy_count, dummy_get_row, dummy_on_select, &photos_images_ctx },
    { LANG_PIVOT_AI,     dummy_count, dummy_get_row, dummy_on_select, &photos_ai_ctx },
};
static const struct metro_page photos_page = { LANG_HUB_PHOTOS, photos_pivots, 4 };

/* --- the hub's own 4 rows: music | videos | photos | settings --------- */

static int hub_count(void *ctx)
{
    (void)ctx;
    return 4;
}

static void hub_get_row(void *ctx, int index, struct metro_row *out)
{
    static const enum metro_lang_id titles[4] = {
        LANG_HUB_MUSIC, LANG_HUB_VIDEOS, LANG_HUB_PHOTOS, LANG_HUB_SETTINGS
    };

    (void)ctx;
    out->title = metro_lang_str(titles[index]);
    out->subtitle = NULL;
    out->kind = METRO_ROW_NAV;
}

static void hub_on_select(void *ctx, int index)
{
    (void)ctx;

    switch (index)
    {
        case 0: metro_screen_list_push(&music_page); break;
        case 1: metro_screen_list_push(&videos_page); break;
        case 2: metro_screen_list_push(&photos_page); break;
        case 3: metro_screen_list_push(metro_screen_settings_page()); break;
        default: break;
    }
}

void metro_screen_hub_show(void)
{
    metro_nav_t *nav = metro_screen_nav();
    int first = metro_nav_first_visible(nav);
    int sel = metro_nav_sel(nav);
    int count = hub_count(NULL);
    int i, y = METRO_HUB_FIRST_Y;

    metro_draw_clear();
    metro_draw_header("");

    for (i = first; i < count && i < first + METRO_HUB_VISIBLE + 1; i++)
    {
        struct metro_row row;
        bool selected = (i == sel);

        hub_get_row(NULL, i, &row);
        metro_draw_text(MFONT_DISPLAY, 12, y, row.title,
                         selected ? metro_color_fg() : metro_color_secondary());
        y += METRO_HUB_PITCH;
    }

    lcd_update();
}

void metro_screen_hub_handle(int action, int steps)
{
    metro_nav_t *nav = metro_screen_nav();
    int count = hub_count(NULL);

    switch (action)
    {
        case MACT_PREV:
            metro_nav_move_sel(nav, -steps, count, METRO_HUB_VISIBLE);
            break;
        case MACT_NEXT:
            metro_nav_move_sel(nav, steps, count, METRO_HUB_VISIBLE);
            break;
        case MACT_SELECT:
            hub_on_select(NULL, metro_nav_sel(nav));
            break;
        default:
            break;
    }
}
