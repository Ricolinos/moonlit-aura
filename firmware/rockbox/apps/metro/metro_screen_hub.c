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
#include <string.h>
#include "lcd.h"
#include "string-extra.h"

#include "metro_screen_hub.h"
#include "metro_screen_list.h"
#include "metro_screen_settings.h"
#include "metro_screen_nowplaying.h"
#include "metro_music.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_keymap.h"

#define METRO_HUB_FIRST_Y 32
#define METRO_HUB_PITCH   52
#define METRO_HUB_VISIBLE 4

static void open_album_songs(int32_t album_seek, const char *album_label);
static void open_genre_songs(int32_t genre_seek, const char *genre_label);

/* --- dummy row provider, shared by every videos/photos pivot -- F4
 * replaced music's own dummy_* usage with metro_music (below); F7
 * replaces this one for videos/photos, nothing else changes. */

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
static const struct metro_page videos_page = { LANG_HUB_VIDEOS, videos_pivots, 4, NULL };

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
static const struct metro_page photos_page = { LANG_HUB_PHOTOS, photos_pivots, 4, NULL };

/* --- music: real tagcache-backed lists (F4) -----------------------------
 * Each top-level pivot (artists/albums/songs/genres/playlists) caches
 * its query result in a static array, refreshed once when the user
 * enters Music (music_lists_refresh()) -- count()/get_row() just index
 * into it, no tagcache traffic on every redraw/keypress. Drilling into
 * an artist or an album/genre reuses ONE shared subpage per kind
 * (single path down the twist stack at a time, see metro_page.h), its
 * cache overwritten by open_album_songs()/open_genre_songs() right
 * before each push. */

static metro_music_item_t s_artists[METRO_MUSIC_MAX_ITEMS];
static int s_artists_n;
static metro_music_item_t s_albums[METRO_MUSIC_MAX_ITEMS];
static int s_albums_n;
static metro_music_item_t s_songs[METRO_MUSIC_MAX_ITEMS];
static int s_songs_n;
static metro_music_item_t s_genres[METRO_MUSIC_MAX_ITEMS];
static int s_genres_n;
static char s_playlist_files[METRO_MUSIC_MAX_ITEMS][METRO_MUSIC_ITEM_LEN];
static char s_playlist_names[METRO_MUSIC_MAX_ITEMS][METRO_MUSIC_ITEM_LEN];
static int s_playlists_n;

static void music_lists_refresh(void)
{
    int i;

    s_artists_n = metro_music_artists(s_artists, METRO_MUSIC_MAX_ITEMS);
    s_albums_n  = metro_music_albums(s_albums, METRO_MUSIC_MAX_ITEMS);
    s_songs_n   = metro_music_songs(s_songs, METRO_MUSIC_MAX_ITEMS);
    s_genres_n  = metro_music_genres(s_genres, METRO_MUSIC_MAX_ITEMS);

    s_playlists_n = metro_music_list_playlists(s_playlist_files, METRO_MUSIC_MAX_ITEMS);
    for (i = 0; i < s_playlists_n; i++)
        metro_music_playlist_display_name(s_playlist_files[i], s_playlist_names[i],
                                           METRO_MUSIC_ITEM_LEN);
}

/* pivot: artists -> on_select drills into that artist's albums */

static int artists_count(void *ctx) { (void)ctx; return s_artists_n; }

static void artists_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_artists[index].label;
    out->subtitle = NULL;
    out->kind = METRO_ROW_NAV;
}

static metro_music_item_t s_artist_albums[METRO_MUSIC_MAX_ITEMS];
static int s_artist_albums_n;
static char s_artist_albums_title[METRO_MUSIC_ITEM_LEN];

static int artist_albums_count(void *ctx) { (void)ctx; return s_artist_albums_n; }

static void artist_albums_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_artist_albums[index].label;
    out->subtitle = s_artist_albums[index].subtitle[0] ? s_artist_albums[index].subtitle : NULL;
    out->kind = METRO_ROW_NAV;
}

static void artist_albums_on_select(void *ctx, int index)
{
    (void)ctx;
    open_album_songs(s_artist_albums[index].seek, s_artist_albums[index].label);
}

static const struct metro_pivot artist_albums_pivots[] = {
    { LANG_PIVOT_ALBUMS, artist_albums_count, artist_albums_get_row, artist_albums_on_select, NULL },
};
static struct metro_page artist_albums_page = { LANG_PIVOT_ALBUMS, artist_albums_pivots, 1, NULL };

static void artists_on_select(void *ctx, int index)
{
    (void)ctx;
    s_artist_albums_n = metro_music_albums_of_artist(s_artists[index].seek,
                                                       s_artist_albums, METRO_MUSIC_MAX_ITEMS);
    strlcpy(s_artist_albums_title, s_artists[index].label, sizeof(s_artist_albums_title));
    artist_albums_page.title_dynamic = s_artist_albums_title;
    metro_screen_list_push(&artist_albums_page);
}

/* pivot: albums -> on_select drills into that album's songs */

static int albums_count(void *ctx) { (void)ctx; return s_albums_n; }

static void albums_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_albums[index].label;
    out->subtitle = s_albums[index].subtitle[0] ? s_albums[index].subtitle : NULL;
    out->kind = METRO_ROW_NAV;
}

static void albums_on_select(void *ctx, int index)
{
    (void)ctx;
    open_album_songs(s_albums[index].seek, s_albums[index].label);
}

/* pivot: songs (all, alphabetical) -> on_select plays from that row on */

static int songs_count(void *ctx) { (void)ctx; return s_songs_n; }

static void songs_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_songs[index].label;
    out->subtitle = s_songs[index].subtitle[0] ? s_songs[index].subtitle : NULL;
    out->kind = METRO_ROW_ACTION;
}

static void songs_on_select(void *ctx, int index)
{
    (void)ctx;
    if (metro_music_play_all_songs(index))
        metro_screen_nowplaying_push();
}

/* pivot: genres -> on_select drills into that genre's songs */

static int genres_count(void *ctx) { (void)ctx; return s_genres_n; }

static void genres_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_genres[index].label;
    out->subtitle = NULL;
    out->kind = METRO_ROW_NAV;
}

static void genres_on_select(void *ctx, int index)
{
    (void)ctx;
    open_genre_songs(s_genres[index].seek, s_genres[index].label);
}

/* pivot: playlists -> on_select plays the whole saved playlist */

static int playlists_count(void *ctx) { (void)ctx; return s_playlists_n; }

static void playlists_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_playlist_names[index];
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void playlists_on_select(void *ctx, int index)
{
    (void)ctx;
    if (metro_music_play_playlist(index))
        metro_screen_nowplaying_push();
}

static const struct metro_pivot music_pivots[] = {
    { LANG_PIVOT_ARTISTS,   artists_count,   artists_get_row,   artists_on_select,   NULL },
    { LANG_PIVOT_ALBUMS,    albums_count,    albums_get_row,    albums_on_select,    NULL },
    { LANG_PIVOT_SONGS,     songs_count,     songs_get_row,     songs_on_select,     NULL },
    { LANG_PIVOT_GENRES,    genres_count,    genres_get_row,    genres_on_select,    NULL },
    { LANG_PIVOT_PLAYLISTS, playlists_count, playlists_get_row, playlists_on_select, NULL },
};
static const struct metro_page music_page = { LANG_HUB_MUSIC, music_pivots, 5, NULL };

/* Shared "songs of one album" subpage -- reached from either the
 * top-level albums pivot or an artist's albums (only one such page can
 * ever be on the nav stack at a time, see metro_page.h). */

static metro_music_item_t s_album_songs[METRO_MUSIC_MAX_ITEMS];
static int s_album_songs_n;
static int32_t s_album_songs_seek;
static char s_album_songs_title[METRO_MUSIC_ITEM_LEN];

static int album_songs_count(void *ctx) { (void)ctx; return s_album_songs_n; }

static void album_songs_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_album_songs[index].label;
    out->subtitle = s_album_songs[index].subtitle[0] ? s_album_songs[index].subtitle : NULL;
    out->kind = METRO_ROW_ACTION;
}

static void album_songs_on_select(void *ctx, int index)
{
    (void)ctx;
    if (metro_music_play_songs_of_album(s_album_songs_seek, index))
        metro_screen_nowplaying_push();
}

static const struct metro_pivot album_songs_pivots[] = {
    { LANG_PIVOT_SONGS, album_songs_count, album_songs_get_row, album_songs_on_select, NULL },
};
static struct metro_page album_songs_page = { LANG_PIVOT_SONGS, album_songs_pivots, 1, NULL };

static void open_album_songs(int32_t album_seek, const char *album_label)
{
    s_album_songs_seek = album_seek;
    s_album_songs_n = metro_music_songs_of_album(album_seek, s_album_songs, METRO_MUSIC_MAX_ITEMS);
    strlcpy(s_album_songs_title, album_label, sizeof(s_album_songs_title));
    album_songs_page.title_dynamic = s_album_songs_title;
    metro_screen_list_push(&album_songs_page);
}

/* Shared "songs of one genre" subpage -- same reuse rule as above. */

static metro_music_item_t s_genre_songs[METRO_MUSIC_MAX_ITEMS];
static int s_genre_songs_n;
static int32_t s_genre_songs_seek;
static char s_genre_songs_title[METRO_MUSIC_ITEM_LEN];

static int genre_songs_count(void *ctx) { (void)ctx; return s_genre_songs_n; }

static void genre_songs_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->title = s_genre_songs[index].label;
    out->subtitle = s_genre_songs[index].subtitle[0] ? s_genre_songs[index].subtitle : NULL;
    out->kind = METRO_ROW_ACTION;
}

static void genre_songs_on_select(void *ctx, int index)
{
    (void)ctx;
    if (metro_music_play_songs_of_genre(s_genre_songs_seek, index))
        metro_screen_nowplaying_push();
}

static const struct metro_pivot genre_songs_pivots[] = {
    { LANG_PIVOT_SONGS, genre_songs_count, genre_songs_get_row, genre_songs_on_select, NULL },
};
static struct metro_page genre_songs_page = { LANG_PIVOT_SONGS, genre_songs_pivots, 1, NULL };

static void open_genre_songs(int32_t genre_seek, const char *genre_label)
{
    s_genre_songs_seek = genre_seek;
    s_genre_songs_n = metro_music_songs_of_genre(genre_seek, s_genre_songs, METRO_MUSIC_MAX_ITEMS);
    strlcpy(s_genre_songs_title, genre_label, sizeof(s_genre_songs_title));
    genre_songs_page.title_dynamic = s_genre_songs_title;
    metro_screen_list_push(&genre_songs_page);
}

/* "updating library..." placeholder -- pushed instead of music_page
 * while metro_music_db_ready() is still false. */

static int updating_count(void *ctx) { (void)ctx; return 1; }

static void updating_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    (void)index;
    out->title = metro_lang_str(LANG_MUSIC_DB_UPDATING);
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void updating_on_select(void *ctx, int index) { (void)ctx; (void)index; }

static const struct metro_pivot updating_pivots[] = {
    { LANG_HUB_MUSIC, updating_count, updating_get_row, updating_on_select, NULL },
};
static const struct metro_page updating_page = { LANG_HUB_MUSIC, updating_pivots, 1, NULL };


/* --- the hub's own rows: [now playing] | music | videos | photos | settings */

static int hub_count(void *ctx)
{
    (void)ctx;
    return metro_music_is_playing() ? 5 : 4;
}

static void hub_get_row(void *ctx, int index, struct metro_row *out)
{
    static const enum metro_lang_id titles[4] = {
        LANG_HUB_MUSIC, LANG_HUB_VIDEOS, LANG_HUB_PHOTOS, LANG_HUB_SETTINGS
    };
    bool playing = metro_music_is_playing();

    (void)ctx;

    if (playing && index == 0)
    {
        static char buf[METRO_MUSIC_ITEM_LEN];
        char sub[METRO_MUSIC_ITEM_LEN];

        if (!metro_music_now_playing(buf, sizeof(buf), sub, sizeof(sub)))
            strlcpy(buf, metro_lang_str(LANG_HUB_NOWPLAYING), sizeof(buf));
        out->title = buf;
        out->subtitle = NULL;
        out->kind = METRO_ROW_NAV;
        return;
    }

    out->title = metro_lang_str(titles[playing ? index - 1 : index]);
    out->subtitle = NULL;
    out->kind = METRO_ROW_NAV;
}

static void hub_on_select(void *ctx, int index)
{
    bool playing = metro_music_is_playing();
    int base;

    (void)ctx;

    if (playing && index == 0)
    {
        metro_screen_nowplaying_push();
        return;
    }

    base = playing ? index - 1 : index;
    switch (base)
    {
        case 0:
            if (!metro_music_db_ready())
                metro_screen_list_push(&updating_page);
            else
            {
                music_lists_refresh();
                metro_screen_list_push(&music_page);
            }
            break;
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
