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
#include "file.h"

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
#include "metro_lrc.h"
#include "metro_motion.h" /* R4/FA-9: metro_seek_step_ms() */

/* R4/FA-9 (M-072): sin un evento de búsqueda en este lapso, la racha
 * se considera terminada (el usuario soltó el botón) y el paso vuelve
 * a ser fino. Holgado respecto al intervalo de repetición del driver,
 * para no reiniciarse a mitad de un hold. La rampa en sí vive en
 * metro_motion.c (host-testeable). */
#define METRO_SEEK_IDLE_TICKS   (HZ / 2)
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

/* R3-F2/DD-2/DD-3 (M-063): synced lyrics, full-screen mode. "cache of
 * 1" keyed by id3->path, same shape as metro_albumart.c's own
 * load_art() -- reload only when the track actually changed, never
 * per redraw. metro_lrc_parse() works in place on lrc.buf, so the file
 * is read directly into it (no second buffer). A track with no
 * sibling .lrc (or an unreadable/empty one) leaves s_lrc.count at 0 --
 * that's the single source of truth both the Options row and the
 * full-screen mode itself check before showing anything, so there's
 * never a blank lyrics screen (same fallback criterion Aura's own
 * parser follows, INVESTIGACION-metro-r3.md A.3). */
static struct metro_lrc s_lrc;
static char s_lrc_loaded_path[MAX_PATH];
static bool s_lrc_loaded;
static bool s_lyrics_mode;

static bool ensure_lyrics_loaded(struct mp3entry *id3)
{
    char lrc_path[MAX_PATH];
    int fd;
    ssize_t n;

    if (s_lrc_loaded && !strcmp(s_lrc_loaded_path, id3->path))
        return s_lrc.count > 0;

    s_lrc.count = 0;
    s_lrc_loaded = true;
    strlcpy(s_lrc_loaded_path, id3->path, sizeof(s_lrc_loaded_path));

    if (!metro_lrc_sibling_path(id3->path, lrc_path, sizeof(lrc_path)))
        return false;

    fd = open(lrc_path, O_RDONLY);
    if (fd < 0)
        return false;

    n = read(fd, s_lrc.buf, sizeof(s_lrc.buf) - 1);
    close(fd);
    if (n <= 0)
        return false;

    return metro_lrc_parse(&s_lrc, (size_t)n);
}

/* R4/FA-9 (M-072): racha de eventos consecutivos. Estado de sesión
 * puro; la política de cuánto saltar es metro_seek_step_ms(). */
static int  s_seek_run;
static long s_seek_last_tick;

static long seek_step_ms(void)
{
    if (current_tick - s_seek_last_tick > METRO_SEEK_IDLE_TICKS)
        s_seek_run = 0; /* se soltó el botón: vuelve al paso fino */

    s_seek_last_tick = current_tick;
    return metro_seek_step_ms(s_seek_run++);
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

/* R3-F5/DD-7 (M-066): tag_rating is native Rockbox, stored 0-10; the
 * UI cycles/shows 5 stars, mapped x2 -- same even-number convention
 * Aura-Firmware's own commit_rating() uses (consultado read-only), so
 * a rating either side writes/reads means the same thing on disk. */
static char s_rating_subtitle[8];

static void cycle_rating(void)
{
    struct mp3entry *id3 = audio_current_track();
    int stars;

    if (!id3)
        return;

    stars = (id3->rating / 2 + 1) % 6; /* 0..5, wraps */
    id3->rating = stars * 2;

    /* R3-F5/DD-7: verified live (calificar -> cambiar de pista ->
     * volver) que el `tagtree_buffer_event()` de stock (apps/tagtree.c,
     * registrado sin condiciones desde el propio `tagtree_init()` de
     * apps/main.c, jamás tocado por Metro) ya restaura `tag_rating` en
     * `id3->rating` en cada buffer -- a diferencia de Aura, que nunca
     * llama `tagtree_init()` (árbol propio, `INVESTIGACION-metro-r3.md`
     * C.2) y por eso sí necesita su propio listener de
     * `PLAYBACK_EVENT_TRACK_BUFFER` para esto mismo. Metro lo hereda
     * gratis -- ningún callback nuevo registrado aquí. Verificación en
     * `docs/DESVIACIONES.md` R3-5. */
    if (id3->tagcache_idx && global_settings.runtimedb)
    {
        tagcache_update_numeric(id3->tagcache_idx - 1, tag_rating, id3->rating);

        /* R3-F5/DD-7 (M-066): mismo motivo que `import_ratings()`
         * (`metro_sync.c`) -- `tagcache_update_numeric()` solo ENCOLA
         * la escritura; sin forzar el volcado, calificar una pista
         * quedaría invisible para cualquier lectura hasta que la cola
         * se llenara sola (32 pendientes) o el dispositivo se apagara
         * de verdad. Una calificación puesta a mano es exactamente la
         * clase de escritura poco frecuente e intencional que vale la
         * pena hacer persistir de inmediato -- `tagcache_shutdown()`
         * es solo `run_command_queue(true)` en este target (ver
         * `import_ratings()`), seguro de llamar aquí mismo. */
        tagcache_shutdown();
    }
}

static const char *rating_subtitle(void)
{
    struct mp3entry *id3 = audio_current_track();
    int stars = id3 ? id3->rating / 2 : 0;

    if (stars < 0)
        stars = 0;
    else if (stars > 5)
        stars = 5; /* defensive: tag_rating is meant to stay 0-10 */

    snprintf(s_rating_subtitle, sizeof(s_rating_subtitle), "%d/5", stars);
    return s_rating_subtitle;
}

/* R3-F2/DD-2: whether the CURRENT track has usable lyrics -- the
 * single check both this row and metro_screen_nowplaying_show() rely
 * on so the mode is never reachable (nor stays shown) without them.
 * ensure_lyrics_loaded() is a cache-of-1 keyed by path (same shape as
 * metro_albumart.c's load_art()), so calling it every redraw of this
 * row is as cheap as calling it every redraw of Now Playing itself
 * already is -- a no-op strcmp() unless the track changed. */
static bool lyrics_available(void)
{
    struct mp3entry *id3 = audio_current_track();
    return id3 && ensure_lyrics_loaded(id3);
}

static int options_count(void *ctx) { (void)ctx; return 4 + s_queue_n; }

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
    else if (index == 2)
    {
        out->title = metro_lang_str(LANG_NP_LYRICS);
        out->subtitle = !lyrics_available() ? metro_lang_str(LANG_VALUE_UNAVAILABLE)
                         : s_lyrics_mode     ? metro_lang_str(LANG_VALUE_ON)
                                             : metro_lang_str(LANG_VALUE_OFF);
        out->kind = METRO_ROW_SETTING;
    }
    else if (index == 3)
    {
        out->title = metro_lang_str(LANG_NP_RATING);
        out->subtitle = rating_subtitle();
        out->kind = METRO_ROW_SETTING;
    }
    else
    {
        out->title = s_queue_labels[index - 4];
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
    else if (index == 2 && lyrics_available())
        s_lyrics_mode = !s_lyrics_mode;
    else if (index == 3)
        cycle_rating();
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
/* R4/FA-6 (M-073): el hueco de iconografía más grande que tenía esta
 * pantalla -- al pausar quedaba visualmente idéntica salvo que el
 * tiempo dejaba de avanzar, sin ninguna forma de saberlo a simple
 * vista.
 *
 * Va al extremo IZQUIERDO de la fila de glifos de estado (la de
 * aleatorio/repetir, que se llena desde la derecha), así que no
 * desplaza nada de lo que ya había.
 *
 * Asimetría deliberada de color: **pausa en acento, reproducción en
 * secundario**. Reproducir es el estado normal y no necesita gritar;
 * pausa es el estado que explica por qué no se oye nada, y es el que
 * uno viene a buscar con la mirada. */
static void draw_transport_indicator(void)
{
    int x = 12;
    int y = 176;
    int status = audio_status();

    if (status & AUDIO_STATUS_PAUSE)
        metro_widgets_draw_pause_icon(x, y, metro_color_accent());
    else if (status & AUDIO_STATUS_PLAY)
        metro_widgets_draw_play_icon(x, y, metro_color_secondary());
}

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
 * like the rest of this screen, never animated on its own.
 *
 * R2-F1/DD-2 (M-052): re-verified against a near-white cover fixture
 * (0xF2F2EC, "Wheel & Click/Analog Dreams" in gen_test_media.sh) --
 * kept at 77 rather than dropping to 51 (20%). Blending at only 30%
 * keeps metro_color_bg()'s dark base dominant in the composite
 * regardless of how bright the source art is (0.3*brightCover +
 * 0.7*darkBg still lands in a medium-dark gray, not near-white), so
 * the tertiary (album) text line stays legible in practice even
 * against the palest cover this fixture set can produce -- see
 * docs/screenshots/R2-F1-np-worstcase.png. */
#define METRO_NP_BG_ALPHA256 77 /* ~30% of 256 */

/* R3-F2/DD-2: full-screen lyrics -- active line in the title face at
 * screen center, up to 2 lines of context each side in the list face,
 * dimmed further with distance. Left-aligned at x=12 like every other
 * NP text (Metro doesn't center text), never the "vidrio" translucent
 * panel Aura-Firmware uses for the same problem (INVESTIGACION-metro-r3.md
 * A.4) -- Metro's flat language has no glass, and the 30% cover dim
 * already applied above (metro_fb_blend_over_color) is what keeps this
 * legible over a bright cover instead. Y positions are fixed pixels,
 * same convention as the rest of this screen (and the whole app) --
 * not computed from font metrics. */
#define METRO_LYRICS_TOP_Y   40
#define METRO_LYRICS_PITCH   32
#define METRO_LYRICS_CONTEXT 2

static void draw_lyrics_mode(struct mp3entry *id3)
{
    int active = metro_lrc_find_active(&s_lrc, (uint32_t)id3->elapsed);
    int center_y = METRO_LYRICS_TOP_Y + (LCD_HEIGHT - METRO_LYRICS_TOP_Y) / 2;
    int i;

    for (i = -METRO_LYRICS_CONTEXT; i <= METRO_LYRICS_CONTEXT; i++)
    {
        int idx = active + i;
        int dist = (i < 0) ? -i : i;
        int y = center_y + i * METRO_LYRICS_PITCH;
        const char *text;

        if (idx < 0 || idx >= s_lrc.count)
            continue;

        text = metro_lrc_text(&s_lrc, idx);
        if (!text || !*text)
            continue;

        if (dist == 0)
            metro_draw_text_cut_right(MFONT_TITLE, 12, y, text,
                                       metro_color_fg(), LCD_WIDTH - 24);
        else
            metro_draw_text_cut_right(MFONT_LIST, 12, y, text,
                                       dist == 1 ? metro_color_secondary()
                                                  : metro_color_tertiary(),
                                       LCD_WIDTH - 24);
    }
}

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

    if (s_lyrics_mode && ensure_lyrics_loaded(id3) && s_lrc.count > 0)
    {
        /* Replaces the whole normal layout (art, title/artist/album,
         * times, progress bar, mode icons) -- "pantalla completa"
         * means the whole content area, not an overlay on top of it.
         * The volume overlay below still applies on top regardless:
         * it's transient feedback for an action just taken, not part
         * of the layout this mode replaces. */
        draw_lyrics_mode(id3);
    }
    else
    {
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
        draw_transport_indicator();
    }

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
                pos = (long)id3->elapsed - seek_step_ms();
                audio_ff_rewind(pos > 0 ? pos : 0);
            }
            break;
        case MACT_SEEK_FWD:
            id3 = audio_current_track();
            if (id3)
            {
                pos = (long)id3->elapsed + seek_step_ms();
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
            metro_music_playpause();
            break;
        case MACT_BACK:
            metro_screen_list_pop();
            break;
        default:
            break;
    }
}
