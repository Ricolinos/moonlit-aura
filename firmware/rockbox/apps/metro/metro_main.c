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
 * Metro UI -- replacement UI layer for this Rockbox fork (see
 * MODIFICATIONS.md, DECISIONS.md M-006 in the repository root).
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
#include <stdbool.h>
#include "kernel.h"
#include "button.h"
#include "misc.h"
#include "settings.h"
#include "statusbar.h"

#include "metro_main.h"
#include "metro_screen_splash.h"
#include "metro_fonts.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_draw.h"
#include "metro_screen_list.h"
#include "metro_screen_hub.h"
#include "metro_screen_nowplaying.h"
#include "metro_input.h"
#include "metro_keymap.h"
#include "metro_settings.h"
#include "metro_sync.h"
#include "metro_device.h"

/* See metro_main.h for why this must be called from apps/main.c's
 * init(), not from here. None of these settings are exposed anywhere
 * in the Metro UI (yet), so forcing them is the only way to guarantee
 * the behaviour regardless of what a previous install left in the
 * on-disk config file. */
void metro_apply_hygiene(void)
{
    global_settings.statusbar = STATUSBAR_OFF;
    global_settings.backdrop_file[0] = '-';
    global_settings.backdrop_file[1] = '\0';
    global_settings.show_shutdown_message = false;
    global_settings.talk_menu = false;
    global_settings.clear_settings_on_hold = false;
    global_settings.tagcache_ram = true;
    global_settings.keyclick = 0;              /* M-008: piezo off by default */
#ifdef USB_ENABLE_HID
    global_settings.usb_hid = false;
#endif
}

static void redraw_current(void)
{
    if (metro_nav_is_root(metro_screen_nav()))
        metro_screen_hub_show();
    else if (metro_screen_nowplaying_is_current())
        metro_screen_nowplaying_show();
    else
        metro_screen_list_show();
}

/* F6: the one full-screen wait state in Metro (PLAN_MAESTRO.md S4.3) --
 * drawn straight from metro_main.c, not a metro_screen_* module, same
 * as the plan's own file list for this phase (no new screen file).
 * MENU postpones a running job (metro_sync_postpone(), the job keeps
 * going in the background) or dismisses an error (metro_sync_dismiss()) --
 * either way the loop below exits as soon as metro_sync_needs_screen()
 * goes false. */
static void draw_sync_screen(void)
{
    enum metro_lang_id msg = LANG_MUSIC_DB_UPDATING;
    bool is_error = false;

    switch (metro_sync_state())
    {
        case METRO_SYNC_ERROR_VERSION:
            msg = LANG_SYNC_ERROR_VERSION;
            is_error = true;
            break;
        case METRO_SYNC_ERROR_ATTEMPTS:
            msg = LANG_SYNC_ERROR_ATTEMPTS;
            is_error = true;
            break;
        default:
            break;
    }

    metro_draw_clear();
    metro_draw_header("");
    metro_draw_text(MFONT_TITLE, 12, 100, metro_lang_str(msg), metro_color_fg());
    if (is_error)
        metro_draw_text(MFONT_CAPTION, 12, 140, metro_lang_str(LANG_SYNC_DISMISS_HINT),
                         metro_color_secondary());
    lcd_update();
}

static void run_sync_screen_if_needed(void)
{
    if (!metro_sync_needs_screen())
        return;

    draw_sync_screen();

    while (metro_sync_needs_screen())
    {
        int action = metro_input_next(MCTX_DIALOG, HZ / 10, NULL);

        if (action & SYS_EVENT)
        {
            default_event_handler(action);
            continue;
        }

        if (action == MACT_BACK)
        {
            if (metro_sync_state() == METRO_SYNC_ERROR_VERSION ||
                metro_sync_state() == METRO_SYNC_ERROR_ATTEMPTS)
                metro_sync_dismiss();
            else
                metro_sync_postpone();
            continue;
        }

        if (metro_sync_tick())
            draw_sync_screen();
    }
}

/* Boot, and every return from the USB screen -- the only two moments
 * the firmware ever recovers the disk (PLAN_MAESTRO.md S1.2). */
static void metro_disk_handoff(void)
{
    metro_settings_apply_pending_clock();
    metro_device_reload();
    metro_sync_check_pending();
    run_sync_screen_if_needed();
}

void metro_main(void)
{
    long last_player_tick = 0;

    /* metro_apply_hygiene() already ran inside init() (apps/main.c) --
     * see metro_main.h for why it can't run here, after init() returns. */
    metro_settings_load();
    metro_screen_splash_show();

    metro_fonts_init();
    metro_theme_init();
    /* metro_theme_init()/metro_lang.c's own module-level statics set
     * the compiled defaults; applying the loaded settings right after
     * is what makes them the actual starting values -- one place,
     * F8 does the same thing again whenever a Settings row changes one
     * live. */
    metro_theme_set(metro_settings.theme);
    metro_accent_set(metro_settings.accent);
    metro_lang_set(metro_settings.language);
    metro_screen_list_init();

    metro_disk_handoff();

    /* F3: the twist navigation core supersedes the F2 type/palette
     * specimen as the running UI (metro_screen_specimen.c stays in
     * the tree as a visual regression reference, just unused here --
     * see docs/DESVIACIONES.md F2-1). */
    redraw_current();

    while (1)
    {
        bool at_root = metro_nav_is_root(metro_screen_nav());
        bool at_player = !at_root && metro_screen_nowplaying_is_current();
        enum metro_context ctx = at_root ? MCTX_HUB
                                          : (at_player ? MCTX_PLAYER : MCTX_LIST);
        int steps = 1;
        int action = metro_input_next(ctx, HZ / 10, &steps);

        if (action & SYS_EVENT)
        {
            /* default_event_handler() handles SYS_POWEROFF (clean
             * shutdown) and SYS_USB_CONNECTED (mounts as storage,
             * blocks until the cable is unplugged) -- see
             * PLAN_MAESTRO.md M-006/A.1. metro_input_next() never
             * calls it itself, see metro_input.h. */
            if (default_event_handler(action) == SYS_USB_CONNECTED)
            {
                metro_disk_handoff();
                redraw_current();
            }
            continue;
        }

        if (action == MACT_NONE)
        {
            /* A postponed sync job keeps running in the background
             * with no screen showing -- still needs polling so it
             * actually finishes (marker cleared) instead of sitting
             * forever in METRO_SYNC_POSTPONED. */
            if (metro_sync_job_active())
                metro_sync_tick();

            /* Now Playing has no input of its own most of the time
             * (elapsed time, the progress bar, and the volume overlay's
             * 1.5s countdown all need to update on their own) -- redraw
             * it about once a second even without a button, instead of
             * only reacting to input like every other screen. */
            if (at_player && current_tick - last_player_tick >= HZ)
            {
                last_player_tick = current_tick;
                redraw_current();
            }
            continue;
        }

        if (at_root)
            metro_screen_hub_handle(action, steps);
        else if (at_player)
            metro_screen_nowplaying_handle(action, steps);
        else
            metro_screen_list_handle(action, steps);

        redraw_current();
    }
}
