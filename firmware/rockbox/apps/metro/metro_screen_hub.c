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
#include "fs_defines.h"

#include "metro_screen_hub.h"
#include "metro_screen_list.h"
#include "metro_screen_settings.h"
#include "metro_screen_nowplaying.h"
#include "metro_music.h"
#include "metro_video.h"
#include "metro_photos.h"
#include "metro_thumbs.h"
#include "metro_screen_photo_viewer.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_keymap.h"

#define METRO_HUB_FIRST_Y 32
#define METRO_HUB_PITCH   52
#define METRO_HUB_VISIBLE 4

static void open_album_songs(int32_t album_seek, const char *album_label);
static void open_genre_songs(int32_t genre_seek, const char *genre_label);

/* --- videos: real files under /Videos (F7) -- category pivots
 * (movies/series/clips) only appear when video_categories.cfg tagged
 * at least one currently-listed file; otherwise just "all"
 * (PLAN_MAESTRO.md S2.2, contract D-316: index absent/empty is a
 * supported case, not an error). video_pivots[]/videos_page.npivots
 * are rebuilt by build_videos_page() each time the videos row is
 * entered -- same reuse-one-static-instance rule as the music
 * subpages (metro_page.h: only one such page can be on the nav stack
 * at a time). */

static metro_video_item_t s_video_all[METRO_VIDEO_MAX];
static int s_video_all_n;
static metro_video_item_t s_video_movies[METRO_VIDEO_MAX];
static int s_video_movies_n;
static metro_video_item_t s_video_series[METRO_VIDEO_MAX];
static int s_video_series_n;
static metro_video_item_t s_video_clips[METRO_VIDEO_MAX];
static int s_video_clips_n;

struct video_pivot_ctx {
    metro_video_item_t *items;
    int *count;
};

static struct video_pivot_ctx video_all_ctx    = { s_video_all,    &s_video_all_n };
static struct video_pivot_ctx video_movies_ctx = { s_video_movies, &s_video_movies_n };
static struct video_pivot_ctx video_series_ctx = { s_video_series, &s_video_series_n };
static struct video_pivot_ctx video_clips_ctx  = { s_video_clips,  &s_video_clips_n };

static int video_pivot_count(void *ctx)
{
    return *((struct video_pivot_ctx *)ctx)->count;
}

static void video_pivot_get_row(void *ctx, int index, struct metro_row *out)
{
    struct video_pivot_ctx *c = ctx;
    out->title = c->items[index].filename;
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void video_pivot_on_select(void *ctx, int index)
{
    struct video_pivot_ctx *c = ctx;
    metro_video_play(c->items[index].filename);
}

static struct metro_pivot video_pivots[4];
static struct metro_page videos_page = { LANG_HUB_VIDEOS, video_pivots, 1, NULL };

static void build_videos_page(void)
{
    int i, n = 0;

    s_video_all_n = metro_video_list(s_video_all, METRO_VIDEO_MAX);
    s_video_movies_n = s_video_series_n = s_video_clips_n = 0;
    for (i = 0; i < s_video_all_n; i++)
    {
        switch (s_video_all[i].category)
        {
            case METRO_VIDEO_CAT_MOVIE:  s_video_movies[s_video_movies_n++] = s_video_all[i]; break;
            case METRO_VIDEO_CAT_SERIES: s_video_series[s_video_series_n++] = s_video_all[i]; break;
            case METRO_VIDEO_CAT_CLIP:   s_video_clips[s_video_clips_n++]  = s_video_all[i];  break;
            default: break;
        }
    }

    video_pivots[n++] = (struct metro_pivot){
        LANG_PIVOT_ALL, video_pivot_count, video_pivot_get_row, video_pivot_on_select, &video_all_ctx };
    if (s_video_movies_n || s_video_series_n || s_video_clips_n)
    {
        video_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_MOVIES, video_pivot_count, video_pivot_get_row, video_pivot_on_select, &video_movies_ctx };
        video_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_SERIES, video_pivot_count, video_pivot_get_row, video_pivot_on_select, &video_series_ctx };
        video_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_CLIPS, video_pivot_count, video_pivot_get_row, video_pivot_on_select, &video_clips_ctx };
    }
    videos_page.npivots = n;
}

/* --- photos: real files under /Photos (F7) -- same conditional-pivot
 * rule as videos, against photo_categories.cfg (photo/image/ai). */

static metro_photo_item_t s_photo_all[METRO_PHOTOS_MAX];
static int s_photo_all_n;
static metro_photo_item_t s_photo_photos[METRO_PHOTOS_MAX];
static int s_photo_photos_n;
static metro_photo_item_t s_photo_images[METRO_PHOTOS_MAX];
static int s_photo_images_n;
static metro_photo_item_t s_photo_ai[METRO_PHOTOS_MAX];
static int s_photo_ai_n;

struct photo_pivot_ctx {
    metro_photo_item_t *items;
    int *count;
};

static struct photo_pivot_ctx photo_all_ctx    = { s_photo_all,    &s_photo_all_n };
static struct photo_pivot_ctx photo_photos_ctx = { s_photo_photos, &s_photo_photos_n };
static struct photo_pivot_ctx photo_images_ctx = { s_photo_images, &s_photo_images_n };
static struct photo_pivot_ctx photo_ai_ctx     = { s_photo_ai,     &s_photo_ai_n };

static int photo_pivot_count(void *ctx)
{
    return *((struct photo_pivot_ctx *)ctx)->count;
}

static void photo_pivot_get_row(void *ctx, int index, struct metro_row *out)
{
    struct photo_pivot_ctx *c = ctx;
    out->title = c->items[index].filename;
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void photo_pivot_on_select(void *ctx, int index)
{
    /* R2-F3/DD-10 (M-058): Metro's own viewer, not imageviewer.rock
     * anymore -- browses the SAME array this pivot draws from, so
     * SCROLL_FWD/BACK inside the viewer moves through this exact
     * category, not the unfiltered "todos" list. */
    struct photo_pivot_ctx *c = ctx;
    metro_screen_photo_viewer_push(c->items, *c->count, index);
}

/* R3-F1/DD-1: Photos' own metro_thumb_source -- key is "<filename>.
 * <mtime>" (the same pair the old metro_photo_thumbs.c used to
 * invalidate a stale cache entry), decode is a plain JPEG-cover read
 * from /Photos/<filename> via the engine's shared helper. */
#define PHOTOS_DIR "/Photos"

static bool photo_thumb_cache_key(void *ctx, int index, char *out, size_t out_len)
{
    struct photo_pivot_ctx *c = ctx;
    if (index < 0 || index >= *c->count)
        return false;
    snprintf(out, out_len, "%s.%ld", c->items[index].filename, c->items[index].mtime);
    return true;
}

static bool photo_thumb_decode(void *ctx, int index, fb_data *dst)
{
    struct photo_pivot_ctx *c = ctx;
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", PHOTOS_DIR, c->items[index].filename);
    return metro_thumbs_decode_jpeg_cover(path, dst);
}

static const struct metro_thumb_source photo_thumb_source = {
    "photos", photo_thumb_cache_key, photo_thumb_decode
};

/* R2-F2/DD-7/DD-9: bitmap for the grid's get_tile() -- delegates
 * straight to the (now generic) thumbnail engine. */
static const fb_data *photo_pivot_get_tile(void *ctx, int index)
{
    return metro_thumbs_get(&photo_thumb_source, ctx, index);
}

static struct metro_pivot photo_pivots[4];
static struct metro_page photos_page = { LANG_HUB_PHOTOS, photo_pivots, 1, NULL };

static void build_photos_page(void)
{
    int i, n = 0;

    /* R2-F2/DD-9: a fresh visit to Photos always starts the thumbnail
     * engine clean -- otherwise the RAM window/pending queue could
     * still be serving or decoding for a pivot the user isn't even
     * looking at anymore (e.g. left "ia" scrolled deep, came back into
     * "todos"). */
    metro_thumbs_reset();

    s_photo_all_n = metro_photos_list(s_photo_all, METRO_PHOTOS_MAX);
    s_photo_photos_n = s_photo_images_n = s_photo_ai_n = 0;
    for (i = 0; i < s_photo_all_n; i++)
    {
        switch (s_photo_all[i].category)
        {
            case METRO_PHOTO_CAT_PHOTO: s_photo_photos[s_photo_photos_n++] = s_photo_all[i]; break;
            case METRO_PHOTO_CAT_IMAGE: s_photo_images[s_photo_images_n++] = s_photo_all[i]; break;
            case METRO_PHOTO_CAT_AI:    s_photo_ai[s_photo_ai_n++]         = s_photo_all[i]; break;
            default: break;
        }
    }

    photo_pivots[n++] = (struct metro_pivot){
        LANG_PIVOT_ALL, photo_pivot_count, photo_pivot_get_row, photo_pivot_on_select, &photo_all_ctx,
        METRO_TILE_COLS, photo_pivot_get_tile };
    if (s_photo_photos_n || s_photo_images_n || s_photo_ai_n)
    {
        photo_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_PHOTOS, photo_pivot_count, photo_pivot_get_row, photo_pivot_on_select, &photo_photos_ctx,
            METRO_TILE_COLS, photo_pivot_get_tile };
        photo_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_IMAGES, photo_pivot_count, photo_pivot_get_row, photo_pivot_on_select, &photo_images_ctx,
            METRO_TILE_COLS, photo_pivot_get_tile };
        photo_pivots[n++] = (struct metro_pivot){
            LANG_PIVOT_AI, photo_pivot_count, photo_pivot_get_row, photo_pivot_on_select, &photo_ai_ctx,
            METRO_TILE_COLS, photo_pivot_get_tile };
    }
    photos_page.npivots = n;
}

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
        case 1: build_videos_page(); metro_screen_list_push(&videos_page); break;
        case 2: build_photos_page(); metro_screen_list_push(&photos_page); break;
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
