/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* moonlit (D-049): ver moonlit_screen_library.h. */
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "lcd.h"
#include "kernel.h"
#include "button.h"   /* SYS_EVENT, SYS_USB_CONNECTED */
#include "misc.h"     /* default_event_handler() */
#include "tagcache.h"

#include "moonlit_screen_library.h"
#include "moonlit_logo.h"
#include "moonlit_palette.h"
#include "moonlit_fonts.h"
#include "metro_draw.h"
#include "metro_lang.h"
#include "metro_music.h"
#include "metro_input.h"
#include "metro_keymap.h"
#include "metro_screen_usb.h"

/* Same crescent and bar as metro_screen_splash.c (64px, 120x2) so the
 * two "wait" screens of the firmware read as one family. Vertical
 * rhythm: crescent, title, subtitle, bar, counter -- centred as a
 * block on the 240px screen. */
#define LIB_CRESCENT_SIZE 64
#define LIB_TITLE_GAP     12 /* crescent -> title */
#define LIB_TITLE_H       22 /* MFONT_HEADLINE */
#define LIB_SUB_GAP       6
#define LIB_SUB_H         18 /* MFONT_BODY */
#define LIB_BAR_GAP       14
#define LIB_BAR_W         120
#define LIB_BAR_H         2
#define LIB_COUNT_GAP     10
#define LIB_COUNT_H       18 /* MFONT_LABEL */
#define LIB_BLOCK_H \
    (LIB_CRESCENT_SIZE + LIB_TITLE_GAP + LIB_TITLE_H + LIB_SUB_GAP + LIB_SUB_H + \
     LIB_BAR_GAP + LIB_BAR_H + LIB_COUNT_GAP + LIB_COUNT_H)
#define LIB_TOP ((LCD_HEIGHT - LIB_BLOCK_H) / 2)

static void draw_centered(enum metro_font_role role, int y, const char *text, unsigned color)
{
    int w, h;

    /* moonlit (D-081, addendum): por tramos -- ver metro_draw_text_size(). */
    metro_draw_text_size(role, text, &w, &h);
    metro_draw_text(role, (LCD_WIDTH - w) / 2, y, text, color);
}

/* Nombre de la fase SIN sus marcadores de formato. Las cadenas de
 * D-061 (LANG_SYNC_ART_*) nacieron para snprintf y llevan el conteo
 * dentro ("preparando carátulas %d/%d"); aquí el conteo lo dicen la
 * barra y el contador de abajo, así que la línea de fase se queda con
 * la parte descriptiva. Se corta en el primer '%' y se recortan los
 * espacios que queden.
 *
 * Verificado que los marcadores van al FINAL en las dieciocho cadenas
 * (seis lenguas x tres fases) antes de escribir esto -- si alguna
 * lengua futura los pusiera en medio, esta función la dejaría a
 * medias, y por eso el corte vive aquí y no en cada llamador. La fase
 * de base de datos (LANG_LIBRARY_PHASE_DB) no tiene marcadores y pasa
 * intacta. */
static const char *phase_name(enum metro_lang_id phase)
{
    static char buf[64];
    const char *src = metro_lang_str(phase);
    size_t n = 0;

    while (src[n] && src[n] != '%' && n + 1 < sizeof(buf))
        n++;
    while (n > 0 && src[n - 1] == ' ')
        n--;
    memcpy(buf, src, n);
    buf[n] = '\0';
    return buf;
}

/* moonlit (D-084): el dibujo de las DOS esperas de biblioteca -- esta
 * y la de metro_run_sync_screen_if_needed(). `title`/`phase` salen de
 * metro_lang.c (sin cadenas nuevas: D-049 y D-061 ya las dejaron en
 * las seis lenguas).
 *
 * `total > 0`  -> barra determinada + "N de M" debajo.
 * `total <= 0` -> pista vacia y, si `done > 0`, el contador real solo.
 *                 Es el caso de los recorridos en streaming (artistas,
 *                 fotos) y del escaneo de tagcache antes de que empiece
 *                 el commit: el porcentaje no se puede estimar, pero el
 *                 conteo SI avanza, y eso es lo que hay que mostrar.
 *                 Sin animacion indeterminada -- ver D-084: Aura
 *                 resolvio lo mismo sin animacion propia a proposito
 *                 (su cadencia vive fuera de lcd_active() porque el
 *                 trabajo sigue con la pantalla dormida), y la regla de
 *                 CLAUDE.md solo permite animar bajo lcd_active() y con
 *                 animations != OFF. */
void moonlit_screen_library_draw_progress(enum metro_lang_id title,
                                           enum metro_lang_id phase,
                                           int pct, int done, int total)
{
    int y = LIB_TOP;
    char count[32];

    metro_draw_clear();
    moonlit_logo_draw_crescent(LIB_CRESCENT_SIZE, (LCD_WIDTH - LIB_CRESCENT_SIZE) / 2, y,
                                moonlit_color(MROLE_ON_SURFACE));
    y += LIB_CRESCENT_SIZE + LIB_TITLE_GAP;
    draw_centered(MFONT_HEADLINE, y, metro_lang_str(title),
                  moonlit_color(MROLE_ON_SURFACE));
    y += LIB_TITLE_H + LIB_SUB_GAP;
    draw_centered(MFONT_BODY, y, phase_name(phase),
                  moonlit_color(MROLE_ON_SURFACE_VARIANT));
    y += LIB_SUB_H + LIB_BAR_GAP;

    if (done < 0)
        done = 0;
    if (pct < 0)
        pct = 0;   /* pista vacia: la misma primitiva al 0 %, para que el
                    * bloque no cambie de altura entre fases. */
    if (pct > 100)
        pct = 100;

    metro_draw_progress((LCD_WIDTH - LIB_BAR_W) / 2, y, LIB_BAR_W, LIB_BAR_H, pct);
    y += LIB_BAR_H + LIB_COUNT_GAP;

    if (total > 0)
    {
        if (done > total)
            done = total;
        snprintf(count, sizeof(count), metro_lang_str(LANG_LIBRARY_COUNT_FMT), done, total);
        draw_centered(MFONT_LABEL, y, count, moonlit_color(MROLE_ON_SURFACE_VARIANT));
    }
    else if (done > 0)
    {
        snprintf(count, sizeof(count), "%d", done);
        draw_centered(MFONT_LABEL, y, count, moonlit_color(MROLE_ON_SURFACE_VARIANT));
    }
    lcd_update();
}

/* moonlit (D-084 addendum): los dos tramos de la fase de base de datos,
 * compartidos por esta pantalla y la de metro_main.c -- son la MISMA
 * espera y tienen que verse igual. Mismo reparto que Aura (D-344):
 * el escaneo se queda con el primer 78 % de la barra y el commit con el
 * ultimo 22 %, en vez de dos barridos independientes de 0 a 100 que
 * pareceria que la barra se reinicia a la mitad.
 *
 * Durante el escaneo, `progress` de tagcache es una ESTIMACION (0 sin
 * dircache) y `processed_entries` es un conteo real que siempre avanza:
 * por eso la barra usa uno y el contador el otro. */
void moonlit_screen_library_db_progress(int *pct, int *done, int *total)
{
    const struct tagcache_stat *st = tagcache_get_stat();
    int max_step = tagcache_get_max_commit_step();

    if (st->commit_step > 0 && max_step > 0)
    {
        *pct = 78 + (22 * st->commit_step) / max_step;
        *done = st->commit_step;
        *total = max_step;
        return;
    }
    /* get_progress() (apps/tagcache.c) devuelve -1 cuando NO tiene con
     * que estimar: sin dircache y sin una base previa en RAM, que es
     * justo el primer build. No es 0, es "no se sabe" -- se pasa como
     * pista vacia en vez de dibujar un 0 % que parecería estancado.
     * Con base previa (una actualizacion, no un build desde cero) si
     * hay estimacion y la barra avanza. En los dos casos el contador
     * de archivos procesados avanza, que es lo que de verdad informa
     * durante estos minutos. */
    *pct = st->progress < 0 ? -1 : (78 * st->progress) / 100;
    *done = st->processed_entries;
    *total = 0;
}

static void draw_screen(void)
{
    int pct, done, total;

    moonlit_screen_library_db_progress(&pct, &done, &total);
    moonlit_screen_library_draw_progress(LANG_LIBRARY_PREPARING,
                                          LANG_LIBRARY_PHASE_DB, pct, done, total);
}

/* Shared by both phases: true = the user (MENU) or the system (USB)
 * wants out. Consumes one pending action per call, non-blocking when
 * `timeout_ticks` is 0. Any other button is ignored on purpose -- the
 * only decision this screen offers is "keep going" or "later". */
static bool poll_interrupt(int timeout_ticks)
{
    int action = metro_input_next(MCTX_DIALOG, timeout_ticks, NULL);

    if (action & SYS_EVENT)
    {
        /* Same shape as metro_main.c's main loop: Metro's own
         * "connected" screen first, then the stock handler owns the
         * cable for its whole duration. The disk may have changed
         * underneath -- returning false makes the hub re-evaluate
         * (metro_disk_handoff() ran from the main loop's own USB path
         * is NOT reachable from here; see D-049 open point). */
        if (action == SYS_USB_CONNECTED)
            metro_screen_usb_show();
        if (default_event_handler(action) == SYS_USB_CONNECTED)
            return true;
        return false;
    }
    return action == MACT_BACK;
}

/* metro_music_db_ready() is what starts the first build / per-boot
 * scan (data layer, metro_music.c); polling it here is what keeps the
 * tagcache thread fed with that trigger while we wait. */
static bool db_ready(void)
{
    return metro_music_db_ready() && tagcache_is_fully_initialized();
}

static bool run_phase_db(void)
{
    if (db_ready())
        return true;

    draw_screen();
    while (!db_ready())
    {
        if (poll_interrupt(HZ / 10))
            return false;
        /* apps/main.c:380-388 pattern: commit_step is 0 until the scan
         * finishes and the commit starts, then 1..max_commit_step. */
        draw_screen();
    }
    return true;
}

/* D-059: this is now the WHOLE screen -- the "preparando caratulas"/
 * "revisando caratulas" second phase (D-056/D-058) is gone. The
 * background builder (moonlit_master_art_builder.c) replaces it: it
 * walks the library on its own low-priority thread once the database
 * is ready, and Marea/the grids show the monogram/placeholder for
 * whatever it hasn't reached yet, repainting via their own tick()
 * mechanism when it does (moonlit_master_art_builder_generation()).
 * The orphan sweep (D-055) moved with it -- moonlit_art_gc_pending()/
 * moonlit_art_gc_clear() are now serviced by the builder thread at the
 * end of its next pass, never on screen. */
bool moonlit_screen_library_prepare(void)
{
    return run_phase_db();
}
