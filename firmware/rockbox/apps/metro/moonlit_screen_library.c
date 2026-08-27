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

    lcd_setfont(metro_font_id(role));
    lcd_getstringsize((const unsigned char *)text, &w, &h);
    metro_draw_text(role, (LCD_WIDTH - w) / 2, y, text, color);
}

/* `done`/`total` <= 0: no bar, no counter (before the commit starts
 * reporting steps). D-059: this screen only ever has the ONE phase
 * left (tagcache) -- the subtitle is always LANG_LIBRARY_PHASE_DB, no
 * longer a parameter. */
static void draw_screen(int done, int total)
{
    int y = LIB_TOP;
    char count[32];

    metro_draw_clear();
    moonlit_logo_draw_crescent(LIB_CRESCENT_SIZE, (LCD_WIDTH - LIB_CRESCENT_SIZE) / 2, y,
                                moonlit_color(MROLE_ON_SURFACE));
    y += LIB_CRESCENT_SIZE + LIB_TITLE_GAP;
    draw_centered(MFONT_HEADLINE, y, metro_lang_str(LANG_LIBRARY_PREPARING),
                  moonlit_color(MROLE_ON_SURFACE));
    y += LIB_TITLE_H + LIB_SUB_GAP;
    draw_centered(MFONT_BODY, y, metro_lang_str(LANG_LIBRARY_PHASE_DB),
                  moonlit_color(MROLE_ON_SURFACE_VARIANT));
    y += LIB_SUB_H + LIB_BAR_GAP;

    if (total > 0)
    {
        if (done < 0)
            done = 0;
        if (done > total)
            done = total;
        metro_draw_progress((LCD_WIDTH - LIB_BAR_W) / 2, y, LIB_BAR_W, LIB_BAR_H,
                             done * 100 / total);
        y += LIB_BAR_H + LIB_COUNT_GAP;
        snprintf(count, sizeof(count), metro_lang_str(LANG_LIBRARY_COUNT_FMT), done, total);
        draw_centered(MFONT_LABEL, y, count, moonlit_color(MROLE_ON_SURFACE_VARIANT));
    }
    lcd_update();
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

    draw_screen(tagcache_get_commit_step(), tagcache_get_max_commit_step());
    while (!db_ready())
    {
        if (poll_interrupt(HZ / 10))
            return false;
        /* apps/main.c:380-388 pattern: commit_step is 0 until the scan
         * finishes and the commit starts, then 1..max_commit_step. */
        draw_screen(tagcache_get_commit_step(), tagcache_get_max_commit_step());
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
