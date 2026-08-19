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
#include <string.h>

/* config.h before tagcache.h -- see DECISIONS.md M-030. */
#include "config.h"
#include "tagcache.h"
#include "playlist.h"
#include "audio.h"
#include "kernel.h"
#include "lcd.h"
#include "misc.h"
#include "sound.h"
#include "settings.h"
#include "string-extra.h"

#include "metro_screen_nowplaying.h"
#include "metro_screen_list.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_widgets.h"
#include "metro_albumart.h"
#include "metro_music.h"
#include "metro_settings.h"
#include "metro_fb.h"
#include "metro_keymap.h"

#define METRO_SEEK_STEP_MS      5000
#define METRO_VOL_OVERLAY_TICKS (HZ * 3 / 2)
#define METRO_NP_QUEUE_MAX      60

/* --- sentinel page: never drawn/queried through the generic list path,
 * just a stack-bookkeeping placeholder metro_screen_nowplaying_is_current()
 * recognizes by pointer (see metro_screen_nowplaying.h). */

static int sentinel_count(void *ctx) { (void)ctx; return 0; }
static void sentinel_get_row(void *ctx, int index, struct metro_row *out)
{ (void)ctx; (void)index; (void)out; }
static void sentinel_on_select(void *ctx, int index) { (void)ctx; (void)index; }

static const struct metro_pivot sentinel_pivots[] = {
    { LANG_HUB_NOWPLAYING, sentinel_count, sentinel_get_row, sentinel_on_select, NULL },
};
static const struct metro_page sentinel_page = { LANG_HUB_NOWPLAYING, sentinel_pivots, 1, NULL };

bool metro_screen_nowplaying_push(void)
{
    return metro_screen_list_push(&sentinel_page);
}

bool metro_screen_nowplaying_is_current(void)
{
    return metro_screen_list_current_page() == &sentinel_page;
}

/* --- options page: up next (real queue via playlist_get_track_info(),
 * titles resolved once per push, not per row-draw) + shuffle/repeat
 * toggles. A normal metro_page pushed on top of the sentinel -- MENU
 * pops back to Now Playing through the ordinary metro_screen_list
 * path, nothing special needed here for that. F5 keeps this read-only
 * beyond the two toggle rows: selecting a queued track doesn't jump to
 * it (deferred, not part of this phase's "hecho" list). */

static char s_queue_labels[METRO_NP_QUEUE_MAX][METRO_MUSIC_ITEM_LEN];
static int s_queue_n;

static void queue_refresh(void)
{
    int cur = playlist_get_display_index() - 1;
    int total = playlist_amount();
    int i, n = 0;

    if (cur < 0)
        cur = 0;

    for (i = cur; i < total && n < METRO_NP_QUEUE_MAX; i++)
    {
        struct playlist_track_info info;
        bool got_title = false;

        /* playlist_get_track_info() returns 0 on success, -1 on
         * failure -- an errno-style status, not a bool. `!result`
         * would skip every successfully-resolved track instead of
         * the failed ones. */
        if (playlist_get_track_info(NULL, i, &info) != 0)
            continue;

        if (tagcache_is_usable())
        {
            struct tagcache_search tcs;
            if (tagcache_find_index(&tcs, info.filename))
            {
                if (tagcache_retrieve(&tcs, tcs.idx_id, tag_title,
                                       s_queue_labels[n], METRO_MUSIC_ITEM_LEN)
                    && strcmp(s_queue_labels[n], UNTAGGED) != 0)
                    got_title = true;
                tagcache_search_finish(&tcs);
            }
        }

        if (!got_title)
        {
            const char *base = strrchr(info.filename, '/');
            strlcpy(s_queue_labels[n], base ? base + 1 : info.filename,
                    METRO_MUSIC_ITEM_LEN);
        }
        n++;
    }

    s_queue_n = n;
}

static void toggle_shuffle(void)
{
    /* Rockbox has no clean "unshuffle" (real limitation, not a Metro
     * gap) -- turning shuffle off only stops treating the flag as on
     * from here on, it doesn't restore the original order. Turning it
     * on reorders the remaining queue right away, same as any other
     * Rockbox shuffle trigger. */
    global_settings.playlist_shuffle = !global_settings.playlist_shuffle;
    if (global_settings.playlist_shuffle)
        playlist_shuffle(current_tick, playlist_get_display_index());
}

static void cycle_repeat(void)
{
    switch (global_settings.repeat_mode)
    {
        case REPEAT_OFF: global_settings.repeat_mode = REPEAT_ALL; break;
        case REPEAT_ALL: global_settings.repeat_mode = REPEAT_ONE; break;
        default:         global_settings.repeat_mode = REPEAT_OFF; break;
    }
}

static enum metro_lang_id repeat_value_lang(void)
{
    switch (global_settings.repeat_mode)
    {
        case REPEAT_ALL: return LANG_REPEAT_ALL;
        case REPEAT_ONE: return LANG_REPEAT_ONE;
        default:         return LANG_VALUE_OFF;
    }
}

static int options_count(void *ctx) { (void)ctx; return 2 + s_queue_n; }

static void options_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;

    if (index == 0)
    {
        out->title = metro_lang_str(LANG_NP_SHUFFLE);
        out->subtitle = metro_lang_str(global_settings.playlist_shuffle
                                            ? LANG_VALUE_ON : LANG_VALUE_OFF);
        out->kind = METRO_ROW_SETTING;
    }
    else if (index == 1)
    {
        out->title = metro_lang_str(LANG_NP_REPEAT);
        out->subtitle = metro_lang_str(repeat_value_lang());
        out->kind = METRO_ROW_SETTING;
    }
    else
    {
        out->title = s_queue_labels[index - 2];
        out->subtitle = NULL;
        out->kind = METRO_ROW_ACTION;
    }
}

static void options_on_select(void *ctx, int index)
{
    (void)ctx;

    if (index == 0)
        toggle_shuffle();
    else if (index == 1)
        cycle_repeat();
}

static const struct metro_pivot options_pivots[] = {
    { LANG_NP_OPTIONS_TITLE, options_count, options_get_row, options_on_select, NULL },
};
static const struct metro_page options_page = { LANG_NP_OPTIONS_TITLE, options_pivots, 1, NULL };

static void push_options(void)
{
    queue_refresh();
    metro_screen_list_push(&options_page);
}

/* --- Now Playing itself: custom layout, PLAN_MAESTRO.md S1.4. ------- */

static long s_vol_overlay_until = 0;

static int current_volume_pct(void)
{
    int min = sound_min(SOUND_VOLUME);
    int max = sound_max(SOUND_VOLUME);
    int pct;

    if (max <= min)
        return 0;

    pct = (global_status.volume - min) * 100 / (max - min);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

/* F10: real geometric icons (M-018) replacing F5-1's text badges --
 * right-aligned, repeat first (closer to the edge) then shuffle,
 * METRO_WIDGETS_ICON_SIZE square each with a small gap between. */
static void draw_mode_indicators(void)
{
    int x = LCD_WIDTH - 12 - METRO_WIDGETS_ICON_SIZE;
    int y = 176;

    if (global_settings.repeat_mode == REPEAT_ALL ||
        global_settings.repeat_mode == REPEAT_ONE)
    {
        metro_widgets_draw_repeat_icon(x, y, global_settings.repeat_mode == REPEAT_ONE);
        x -= METRO_WIDGETS_ICON_SIZE + 12;
    }

    if (global_settings.playlist_shuffle)
        metro_widgets_draw_shuffle_icon(x, y);
}

/* F12: 30% of the track's own art, scaled to fill the screen, behind
 * everything else -- graphics=full only (PLAN_MAESTRO.md S3.3);
 * static, redrawn plainly every metro_screen_nowplaying_show() call
 * like the rest of this screen, never animated on its own. */
#define METRO_NP_BG_ALPHA256 77 /* ~30% of 256 */

void metro_screen_nowplaying_show(void)
{
    struct mp3entry *id3;
    char elapsed_buf[16], remaining_buf[16];
    int w, h, pct;
    bool has_bg = metro_settings.graphics == METRO_GFX_FULL &&
                  metro_albumart_load_background();

    if (has_bg)
        metro_fb_blend_over_color(metro_albumart_background_bitmap(),
                                   metro_color_bg(), METRO_NP_BG_ALPHA256);
    else
        metro_draw_clear();

    metro_draw_header(metro_lang_str(LANG_HUB_NOWPLAYING));

    if (!metro_music_is_playing() || !(id3 = audio_current_track()))
    {
        metro_draw_tile(12, 40, METRO_ALBUMART_SIZE, "?");
        lcd_update();
        return;
    }

    if (metro_albumart_load_current())
        lcd_bitmap(metro_albumart_bitmap(), 12, 40, METRO_ALBUMART_SIZE, METRO_ALBUMART_SIZE);
    else
        metro_draw_tile(12, 40, METRO_ALBUMART_SIZE,
                         id3->album ? id3->album : (id3->title ? id3->title : "?"));

    metro_draw_text_cut_right(MFONT_TITLE, 160, 44, id3->title ? id3->title : "?",
                               metro_color_fg(), LCD_WIDTH - 160);
    if (id3->artist)
        metro_draw_text_cut_right(MFONT_LIST, 160, 80, id3->artist,
                                   metro_color_secondary(), LCD_WIDTH - 160);
    if (id3->album)
        metro_draw_text_cut_right(MFONT_CAPTION, 160, 104, id3->album,
                                   metro_color_tertiary(), LCD_WIDTH - 160);

    format_time(elapsed_buf, sizeof(elapsed_buf), (long)id3->elapsed);
    metro_draw_text(MFONT_CAPTION, 12, 200, elapsed_buf, metro_color_secondary());

    format_time(remaining_buf, sizeof(remaining_buf),
                (long)(id3->length > id3->elapsed ? id3->length - id3->elapsed : 0));
    lcd_setfont(metro_font_id(MFONT_CAPTION));
    lcd_getstringsize((const unsigned char *)remaining_buf, &w, &h);
    metro_draw_text(MFONT_CAPTION, LCD_WIDTH - 12 - w, 200, remaining_buf,
                     metro_color_secondary());

    pct = id3->length > 0 ? (int)((unsigned long long)id3->elapsed * 100 / id3->length) : 0;
    metro_draw_progress(0, 214, LCD_WIDTH, 4, pct);

    draw_mode_indicators();

    if (current_tick < s_vol_overlay_until)
        metro_widgets_draw_volume_overlay(current_volume_pct());

    lcd_update();
}

void metro_screen_nowplaying_handle(int action, int steps)
{
    struct mp3entry *id3;
    long pos;

    switch (action)
    {
        case MACT_VOL_UP:
            adjust_volume(steps);
            s_vol_overlay_until = current_tick + METRO_VOL_OVERLAY_TICKS;
            break;
        case MACT_VOL_DOWN:
            adjust_volume(-steps);
            s_vol_overlay_until = current_tick + METRO_VOL_OVERLAY_TICKS;
            break;
        case MACT_TRACK_PREV:
            audio_prev();
            break;
        case MACT_TRACK_NEXT:
            audio_next();
            break;
        case MACT_SEEK_BACK:
            id3 = audio_current_track();
            if (id3)
            {
                pos = (long)id3->elapsed - METRO_SEEK_STEP_MS;
                audio_ff_rewind(pos > 0 ? pos : 0);
            }
            break;
        case MACT_SEEK_FWD:
            id3 = audio_current_track();
            if (id3)
            {
                pos = (long)id3->elapsed + METRO_SEEK_STEP_MS;
                audio_ff_rewind(pos < (long)id3->length ? pos : (long)id3->length);
            }
            break;
        case MACT_OPTIONS:
            push_options();
            break;
        case MACT_TOGGLE_SHUFFLE:
            toggle_shuffle();
            break;
        case MACT_PLAYPAUSE:
            if (audio_status() & AUDIO_STATUS_PAUSE)
                audio_resume();
            else if (audio_status() & AUDIO_STATUS_PLAY)
                audio_pause();
            break;
        case MACT_BACK:
            metro_screen_list_pop();
            break;
        default:
            break;
    }
}
