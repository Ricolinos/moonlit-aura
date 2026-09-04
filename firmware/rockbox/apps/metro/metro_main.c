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
#include <stdlib.h> /* getenv(), M2: gancho de captura del especimen */
/* config.h before tagcache.h -- see DECISIONS.md M-030. */
#include "config.h"
#include "tagcache.h"
#include "kernel.h"
#include "button.h"
#include "lcd.h"
#include "misc.h"
#include "settings.h"
#include "statusbar.h"

#include "metro_main.h"
#include "metro_screen_splash.h"
#include "metro_screen_usb.h"
#include "moonlit_fonts.h"
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_draw.h"
#include "metro_screen_list.h"
#include "metro_screen_hub.h"
#include "metro_screen_nowplaying.h"
#include "metro_input.h"
#include "metro_keymap.h"
#include "metro_settings.h"
#include <stdio.h> /* D-061: snprintf */
#include "metro_sync.h"
#include "metro_device.h"
#include "metro_manifest.h"
#include "metro_transitions.h"
#include "metro_thumbs.h"
#include "metro_screen_photo_viewer.h"
#include "metro_screen_lock.h"
#include "metro_screen_specimen.h"
#include "moonlit_screen_marea.h" /* moonlit (D-029, M8) */
#include "moonlit_art_cache.h"    /* moonlit (D-055): moonlit_art_request_gc() */
#include "moonlit_marquee.h" /* moonlit (D-067): puerta de cuadros */
#include "moonlit_master_art_builder.h" /* moonlit (D-059): background master-art builder */

/* See metro_main.h for why this must be called from apps/main.c's
 * init(), not from here. None of these settings are exposed anywhere
 * in the Metro UI (yet), so forcing them is the only way to guarantee
 * the behaviour regardless of what a previous install left in the
 * on-disk config file. */
void metro_apply_hygiene(void)
{
    global_settings.statusbar = STATUSBAR_OFF;
    /* R5 (M-088): Rockbox's USB "keypad mode" (HID) has no place in
     * Metro -- and with it on, gui_usb_screen_run() takes the HID
     * branch of its loop, which never reaches Metro's animation tick. */
#ifdef USB_ENABLE_HID /* not defined in the simulator build */
    global_settings.usb_hid = false;
#endif
    global_settings.backdrop_file[0] = '-';
    global_settings.backdrop_file[1] = '\0';
    global_settings.show_shutdown_message = false;
    global_settings.talk_menu = false;
    global_settings.clear_settings_on_hold = false;
    global_settings.tagcache_ram = true;
    /* R3-F4/DD-4 (M-065): Quickplay needs tag_lastplayed, and Rockbox
     * never writes it unless this is on -- default is false, and
     * Metro has no menu of its own to expose it (INVESTIGACION-metro-r3.md
     * D.1: the writers, tagtree_buffer_event()/tagtree_track_finish_event(),
     * are already registered unconditionally from apps/main.c's own
     * tagtree_init() -- only the flag gating them was ever missing).
     * Purely local (the device's own playback history, on its own
     * disk, never sent anywhere) -- same "decide it for the user"
     * class as every other setting forced here. */
    global_settings.runtimedb = true;
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
    else if (metro_screen_photo_viewer_is_current())
        metro_screen_photo_viewer_show();
    else if (moonlit_screen_marea_is_current())
        moonlit_screen_marea_show();
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
        metro_draw_text(MFONT_LABEL, 12, 140, metro_lang_str(LANG_SYNC_DISMISS_HINT),
                         metro_color_secondary());
    else
    {
        /* D-061: la fase de imagenes comparte esta pantalla -- es parte
         * de "preparar la biblioteca", no una espera nueva. */
        moonlit_master_art_phase_t phase = MOONLIT_MASTER_ART_PHASE_IDLE;
        int done = 0, total = 0;

        if (metro_sync_art_progress(&phase, &done, &total))
        {
            char line[48];
            enum metro_lang_id fmt;

            if (phase == MOONLIT_MASTER_ART_PHASE_PHOTOS)
                fmt = LANG_SYNC_ART_PHOTOS;
            else if (phase == MOONLIT_MASTER_ART_PHASE_ARTISTS)
                fmt = LANG_SYNC_ART_ARTISTS;
            else
                fmt = LANG_SYNC_ART_ALBUMS;

            /* ART_ALBUMS lleva dos %d (hay total); ARTISTS y PHOTOS
             * uno solo -- sus recorridos son en streaming. */
            if (total > 0)
                snprintf(line, sizeof(line), metro_lang_str(fmt), done, total);
            else
                snprintf(line, sizeof(line), metro_lang_str(fmt), done);
            metro_draw_text(MFONT_LABEL, 12, 140, line, metro_color_secondary());
        }
    }
    lcd_update();
}

/* F9: with show_shutdown_message=false (M-019), Rockbox's own
 * "Shutting down..." splash never runs at all -- without this, the
 * screen would just go dark with zero feedback on power-off. Drawn
 * before default_event_handler(), which proceeds to actually power
 * down right after -- unlike the USB case (metro_screen_usb_show(),
 * DESVIACIONES.md F9-1), nothing else ever draws over this one, so it
 * really is the last thing shown. */
static void draw_shutdown_screen(void)
{
    const char *text = metro_lang_str(LANG_SHUTTING_DOWN);
    int w, h;

    metro_draw_clear();
    lcd_setfont(metro_font_id(MFONT_TITLE));
    lcd_getstringsize((const unsigned char *)text, &w, &h);
    metro_draw_text(MFONT_TITLE, (LCD_WIDTH - w) / 2, (LCD_HEIGHT - h) / 2,
                     text, metro_color_fg());
    lcd_update();
}

void metro_run_sync_screen_if_needed(void)
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
    /* moonlit (D-079, contrato v19): mismo punto y mismo criterio que
     * la hora -- ver metro_settings_apply_pending_shared(). */
    metro_settings_apply_pending_shared();
    /* R2-F1/DD-4 (M-054): a fresh disk (first boot, or a USB session
     * that just mounted a different volume) may not have any of the
     * four media folders yet -- ensure they exist before anything
     * else in this handoff (sync, device reload) tries to read from
     * them. */
    metro_ensure_media_dirs();
    metro_device_reload();
    metro_manifest_reload(); /* R5-F1 (M-081): About reads the RAM copy */
    metro_sync_check_pending();
    metro_run_sync_screen_if_needed();
    /* moonlit (D-049): playlists/artist_images.cfg only ever change
     * over USB -- this is the one place that knows the disk was handed
     * back. Tagcache changes invalidate on their own (hub's stamp). */
    metro_screen_hub_music_lists_invalidate();
}

/* F9: the splash's progress bar (S1.4) covers tagcache's initial "is
 * there already a usable database on disk" determination -- fast
 * (~1s, D-206-class async check) regardless of library size, NOT a
 * full rebuild/scan (that can take much longer for a real library and
 * has its own screen, F6's metro_run_sync_screen_if_needed() -- the
 * splash must never block boot on that). Capped so a wedged tagcache
 * thread can never hang the splash forever. */
#define METRO_SPLASH_MAX_WAIT_TICKS (HZ * 5)

static void wait_for_tagcache_with_splash(void)
{
    long start = current_tick;

    metro_screen_splash_progress(0);
    while (!tagcache_is_fully_initialized())
    {
        long elapsed = current_tick - start;
        if (elapsed >= METRO_SPLASH_MAX_WAIT_TICKS)
            break;
        metro_screen_splash_progress((int)(elapsed * 100 / METRO_SPLASH_MAX_WAIT_TICKS));
        sleep(HZ / 10);
    }
    metro_screen_splash_progress(100);
}

void metro_main(void)
{
    long last_player_tick = 0;
    bool index_letter_was_pending = false;
    long last_hub_tick = 0; /* R5-F5 */
    /* moonlit (D-059): last moonlit_master_art_builder_generation()
     * seen while a thumb decode came back WAITING -- see the
     * metro_thumbs_tick() branch below. */
    unsigned last_thumbs_wait_gen = 0;
    /* moonlit (D-069): estado del sondeo del interruptor Hold. */
    bool hold_was = false;
    long hold_since = 0;
    long hold_idle_drawn = 0;

    /* moonlit (D-059): mutex init only -- no thread yet
     * (moonlit_master_art_builder_poll() creates it once the database
     * is usable, polled from the idle branch below). Must run before
     * ANY album-art decode can happen (metro_albumart.c's
     * decode_file_into()/decode_embedded_into() always take this lock,
     * even the very first Now Playing draw), so first thing here. */
    moonlit_master_art_builder_init();

    /* metro_apply_hygiene() already ran inside init() (apps/main.c) --
     * see metro_main.h for why it can't run here, after init() returns. */
    metro_settings_load();
    /* moonlit (D-055): moonlitcache/{albums,artists,photos} ->
     * /.aura/thumbs/ by rename, once. Disk is up (settings just loaded). */
    if (metro_settings_migrate_shared_thumbs())
        moonlit_art_request_gc(); /* pre-D-055 key scheme: sweep the old names once */
    /* moonlit (D-063, contrato v18): antes que nada lea o escriba una
     * maestra, y antes de que el constructor exista (lo crea
     * moonlit_master_art_builder_poll(), mas abajo en la vuelta ociosa):
     * si /.aura/art/format.txt falta o es de una version anterior, las
     * caches derivadas se borran y el constructor las rehace. Sin
     * pantalla: lo que se borra son archivos por sufijo conocido en
     * tres directorios planos, no un recorrido de biblioteca. */
    metro_settings_purge_stale_art_caches();
    metro_fonts_init();
    /* R2-F1/DD-1 (M-051): DRMODE_FG is the drawmode every apps/metro/
     * text draw expects -- metro_draw_text()/metro_draw_text_cut_right()
     * also set it per call, but the LCD starts in Rockbox's default
     * DRMODE_SOLID and nothing has drawn text yet at this point (the
     * splash screen is next), so set the baseline here too.
     * metro_photos.c and metro_video.c re-set it after their
     * plugin_load() calls return, since imageviewer/mpegplayer are
     * free to leave the LCD in DRMODE_SOLID behind them. */
    lcd_set_drawmode(DRMODE_FG);
    metro_theme_init();
    /* metro_theme_init()/metro_lang.c's own module-level statics set
     * the compiled defaults; applying the loaded settings right after
     * is what makes them the actual starting values -- one place,
     * F8 does the same thing again whenever a Settings row changes one
     * live. Fonts/theme both have to be ready BEFORE the first splash
     * draw (F9: the real splash uses MFONT_DISPLAY/metro_color_fg(),
     * unlike F1's placeholder, which drew before either existed with
     * raw FONT_SYSFIXED). */
    metro_theme_set(metro_settings.theme);
    metro_accent_set(metro_settings.accent);
    metro_lang_set(metro_settings.language);

    metro_screen_splash_show();
    wait_for_tagcache_with_splash();

    metro_screen_list_init();

    /* R3-F7/DD-8 (M-068): el candado se arma aquí y se cobra AQUÍ, antes
     * de metro_disk_handoff() -- que puede levantar la pantalla de
     * "actualizando biblioteca" (metro_run_sync_screen_if_needed()) y
     * dejarla visible por encima del candado. Todo lo que esta pantalla
     * necesita ya está listo a esta altura (fuentes, tema, idioma); lo
     * único que corrió antes es el splash. Al desbloquear, el arranque
     * sigue normal y el handoff recoge cualquier marcador que Studio
     * hubiera dejado. */
    /* moonlit (D-079): las claves compartidas se aplican una vez AQUI,
     * antes de armar/cobrar el candado -- un screen_lock_enabled: 0
     * que Studio dejo escrito mientras el aparato estaba APAGADO
     * (conectado a la computadora sin encender) tiene que ganarle al
     * "bloqueado" local guardado la ultima vez que este mismo aparato
     * SI arranco, o la salida de emergencia por USB del maestro SS
     * A.2.6 quedaria inalcanzable: el candado viejo se cobraria antes
     * de que metro_disk_handoff() (mas abajo) tuviera oportunidad de
     * aplicar el archivo nuevo. metro_disk_handoff() la vuelve a
     * llamar de todos modos -- idempotente (rev ya al dia, no hace
     * nada) para el camino normal donde nada cambio, y es la unica
     * llamada real cuando quien acaba de sincronizar es una vuelta
     * DESDE USB, no un arranque en frio. */
    metro_settings_apply_pending_shared();

    metro_screen_lock_init();
    metro_screen_lock_run_if_active();

    metro_disk_handoff();

    /* F3: the twist navigation core supersedes the F2 type/palette
     * specimen as the running UI (metro_screen_specimen.c stays in
     * the tree as a visual regression reference, just unused here --
     * see docs/DESVIACIONES.md F2-1). */
#ifdef SIMULATOR
    /* M2: unico gancho para que firmware/tools/sim_shot.sh pueda
     * capturar metro_screen_specimen.c -- sin esto es inalcanzable
     * desde la navegacion real (comentario de arriba). SIMULATOR solo
     * lo define tools/configure para el build del simulador
     * (tools/configure:1165,4345), asi que este bloque no existe en
     * el target real. */
    if (getenv("METRO_SIM_SPECIMEN") != NULL)
    {
        int specimen_steps;
        metro_screen_specimen_show();
        while (1)
            metro_input_next(MCTX_HUB, HZ, &specimen_steps);
    }
#endif
    redraw_current();

    while (1)
    {
        bool at_root;
        bool at_player;
        bool at_viewer;
        bool at_marea;

        /* R3-F7/DD-8 (M-068): la interceptación, antes de CUALQUIER
         * despacho de pantalla -- eso es lo que hace que el candado
         * alcance a todo el aparato y no solo a una pantalla. No-op
         * mientras el estado no sea ACTIVE (el caso normal: ya se cobró
         * arriba, en el arranque), así que no cuesta nada por vuelta.
         * Sigue aquí igual, y no solo en el arranque, porque es esta
         * línea -- no el orden de las llamadas de más arriba -- la que
         * vuelve estructuralmente cierto que ninguna otra pantalla es
         * alcanzable con el candado puesto. */
        metro_screen_lock_run_if_active();

        /* moonlit (D-069, maestro SS D): la maquina del Hold, aqui y no
         * dentro de una pantalla -- igual que el candado mismo, tiene
         * que alcanzar a todo el aparato.
         *
         * El interruptor Hold del 6G NO genera eventos de boton
         * (pmu_holdswitch_locked() es sondeo), asi que se compara su
         * valor con el de la vuelta anterior. El bucle ya espera como
         * mucho HZ/10, por debajo del HZ/2 que pide el maestro, asi que
         * NO hizo falta tocar ningun timeout: el flanco se ve dentro de
         * los 100 ms siguientes.
         *
         * Flanco OFF->ON: se anota cuando. Con clave configurada, la
         * pantalla en reposo se queda puesta mientras el Hold siga --
         * redibujada una vez por segundo y solo con lcd_active(), que
         * es lo unico que cuesta energia aqui.
         *
         * Flanco ON->OFF: si el ajuste lo pide y el Hold duro lo
         * suficiente, se rearma el candado; la vuelta siguiente lo cobra
         * en metro_screen_lock_run_if_active(), que sigue siendo el
         * unico punto de interceptacion. Si no, se repinta y ya. */
        {
            bool hold_now = button_hold();

            if (hold_now != hold_was)
            {
                if (hold_now)
                    hold_since = current_tick;
                else
                {
                    long need = metro_screen_lock_require_ticks();

                    if (need >= 0 && metro_screen_lock_state() == METRO_LOCK_ARMED &&
                        current_tick - hold_since >= need)
                        metro_screen_lock_arm_now();
                }
                hold_was = hold_now;
                hold_idle_drawn = 0;
                /* El icono de candado de la barra (D-068) cambia con el
                 * flanco, y sin esto no se veria hasta el siguiente
                 * repintado por otra causa. */
                if (!hold_now)
                    redraw_current();
            }

            if (hold_now && metro_screen_lock_state() == METRO_LOCK_ARMED &&
                lcd_active() &&
                (hold_idle_drawn == 0 || current_tick - hold_idle_drawn >= HZ))
            {
                metro_screen_lock_draw_idle();
                hold_idle_drawn = current_tick;
            }
        }

        at_root = metro_nav_is_root(metro_screen_nav());
        at_player = !at_root && metro_screen_nowplaying_is_current();
        /* R2-F3: mutually exclusive with at_player -- only one sentinel
         * page can be current at a time (metro_screen_photo_viewer.h). */
        at_viewer = !at_root && metro_screen_photo_viewer_is_current();
        /* moonlit (D-029, M8): tercer centinela, mutuamente excluyente
         * con los otros dos por la misma razón. Marea reusa MCTX_LIST
         * -- MACT_PREV/NEXT/SELECT/BACK/HOME/PLAYPAUSE ya cubren todo
         * lo que moonlit_screen_marea_handle() necesita, sin tocar
         * metro_keymap.c. */
        at_marea = !at_root && moonlit_screen_marea_is_current();
        enum metro_context ctx = at_root ? MCTX_HUB
                                          : (at_player ? MCTX_PLAYER
                                                        : (at_viewer ? MCTX_VIEWER : MCTX_LIST));
        int steps = 1;
        /* R5-F5 (M-085): espera más corta mientras la fila
         * "reproduciendo" del hub se anima, para que el tick llegue a
         * ~20 Hz; el resto del tiempo, la de siempre. */
        /* moonlit (D-053): misma cadencia de ~20 Hz mientras Marea
         * anima su scroll por reloj (moonlit_screen_marea_animating()).
         * moonlit (D-057): tambien mientras siga habiendo tapas
         * pendientes en la ventana visible tras el asentamiento
         * (moonlit_screen_marea_wants_ticks(), patron
         * metro_screen_hub_wants_ticks()) -- antes esa espera caia a
         * HZ/10 en cuanto la animacion terminaba, aunque
         * moonlit_screen_marea_tick() todavia tuviera trabajo. */
        /* moonlit (D-067): misma cadencia mientras una marquesina este
         * desplazando. Se apaga sola en cuanto el texto cabe, el LCD se
         * duerme o las animaciones estan apagadas -- la puerta la
         * decide moonlit_marquee_draw() en cada dibujo, no una pantalla
         * declarando "yo animo". */
        int action = metro_input_next(ctx,
                                      ((at_root && metro_screen_hub_wants_ticks()) ||
                                       (at_marea && (moonlit_screen_marea_animating() ||
                                                     moonlit_screen_marea_wants_ticks())) ||
                                       moonlit_marquee_wants_ticks())
                                          ? HZ / 20 : HZ / 10,
                                      &steps);

        if (action & SYS_EVENT)
        {
            /* default_event_handler() handles SYS_POWEROFF (clean
             * shutdown) and SYS_USB_CONNECTED (mounts as storage,
             * blocks until the cable is unplugged) -- see
             * PLAN_MAESTRO.md M-006/A.1. metro_input_next() never
             * calls it itself, see metro_input.h. F9: Metro's own
             * "connected" screen draws first -- default_event_handler()
             * hands the screen to the stock gui_usb_screen_run() for
             * the actual mounted duration right after, see
             * metro_screen_usb.h and DESVIACIONES.md F9-1. */
            if (action == SYS_USB_CONNECTED)
                metro_screen_usb_show();
            else if (action == SYS_POWEROFF || action == SYS_REBOOT)
            {
                draw_shutdown_screen();
                /* R3-F4/DD-5 (M-065): stock Rockbox flushes tagcache's
                 * async command queue (queued playcount/lastplayed/
                 * rating writes, tagcache.c's CMD_UPDATE_NUMERIC) via
                 * tree_flush() -> tagcache_shutdown(), called from deep
                 * inside root_menu()'s own shutdown path. Metro replaces
                 * root_menu() entirely (main.c's own comment, apps/tree.c
                 * is off-limits per this file's header) so that call
                 * never happened -- found while verifying Quickplay's
                 * "order survives a restart" criterion: a normal
                 * shutdown was silently dropping whatever hadn't been
                 * force-flushed yet (the queue only self-flushes at 32
                 * pending entries, tagcache.c's
                 * TAGCACHE_COMMAND_QUEUE_LENGTH). Same risk for R3-F5's
                 * ratings import, next phase -- fixing here, once, in
                 * Metro's own shutdown handling rather than per-feature. */
                tagcache_shutdown();
            }

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

            /* moonlit (D-059): ~1 Hz-ish no-op until the database is
             * usable, then creates the background master-art builder
             * thread once and returns immediately every call after
             * that (a single s_thread_running check) -- cheap enough
             * to poll on every idle iteration, same as the sync job
             * above. */
            moonlit_master_art_builder_poll();

            /* Now Playing has no input of its own most of the time
             * (elapsed time, the progress bar, and the volume overlay's
             * 1.5s countdown all need to update on their own) -- redraw
             * it about once a second even without a button, instead of
             * only reacting to input like every other screen. */
            /* R5-F3 (M-083): mientras el nivel de volumen está en
             * pantalla (3 s quieto + 1 s de fundido) se redibuja a
             * ~8 Hz para que el fundido se vea como tal; el resto del
             * tiempo, la cadencia de siempre. */
            if (at_player && current_tick - last_player_tick >=
                    (metro_screen_nowplaying_volume_visible() ? HZ / 8 : HZ))
            {
                last_player_tick = current_tick;
                redraw_current();
            }

            /* F10: the floating index letter (metro_screen_list.c)
             * needs one more redraw right after it expires to clear
             * itself -- nothing else about the list changed to
             * trigger that on its own otherwise. R2-F3: excluded while
             * in the photo viewer too -- neither this nor the thumb
             * engine below has anything to do on a full-screen photo. */
            /* R5-F5 (M-085): la fila "reproduciendo" del hub se anima
             * por su cuenta (marquesina / respiración) -- un repintado
             * parcial a ~20 Hz, solo mientras hay audio y la fila está a
             * la vista; metro_screen_hub_tick() decide y devuelve false
             * cuando no hay nada que hacer. */
            if (at_root && current_tick - last_hub_tick >= HZ / 20)
            {
                last_hub_tick = current_tick;
                metro_screen_hub_tick();
            }

            /* moonlit (D-067): un cuadro mas de marquesina. Va antes
             * del reparto por pantalla porque la marquesina existe en
             * listas, cuadriculas, "Ahora suena" y Marea por igual --
             * y redraw_current() ya sabe cual dibujar. Marea se excluye:
             * ahi el repintado de la banda lo maneja su propio camino
             * de animacion, mas fino que una pantalla entera. */
            if (!at_marea && moonlit_marquee_wants_ticks())
                redraw_current();

            if (!at_root && !at_player && !at_viewer && !at_marea)
            {
                bool pending = metro_screen_list_has_pending_redraw();
                if (pending || index_letter_was_pending)
                    redraw_current();
                index_letter_was_pending = pending;

                /* R2-F2/DD-9, generalized R3-F1/DD-1: budget one
                 * thumbnail decode per idle tick, same poll as the
                 * index-letter redraw above -- a no-op (returns false
                 * immediately) on any screen that hasn't queued
                 * anything (the engine is shared across every tile
                 * grid, not just Photos anymore). Redraw only when it
                 * actually decoded something, so a freshly-ready tile
                 * replaces its placeholder without redrawing every
                 * single idle tick for nothing. moonlit (D-059): a
                 * WAITING decode (metro_thumbs_take_waiting()) also
                 * makes tick() return true, but redrawing on every
                 * such tick would just repaint the same placeholder in
                 * a tight loop while the builder is mid-pass -- only
                 * worth it once its generation actually moves (some
                 * master may have landed since). */
                if (metro_thumbs_tick())
                {
                    if (!metro_thumbs_take_waiting())
                        redraw_current();
                    else
                    {
                        unsigned gen = moonlit_master_art_builder_generation();

                        if (gen != last_thumbs_wait_gen)
                        {
                            last_thumbs_wait_gen = gen;
                            redraw_current();
                        }
                    }
                }
            }

            /* moonlit (D-029, D-030, M8): presupuesto de decode por
             * vuelta ociosa, mismo patrón que metro_thumbs_tick()
             * arriba -- moonlit_screen_marea.c nunca decodifica un
             * JPEG dentro de show()/draw_slide() (regla dura de
             * D-030), solo aquí, fuera de cualquier bucle de
             * animación. moonlit (D-057): el presupuesto ahora es
             * varias tapas por vuelta (~15 ms o 4 lecturas, ver
             * moonlit_screen_marea_tick()), no una sola. */
            /* moonlit (D-053): mientras el scroll anima, cada vuelta
             * ociosa (HZ/20) es un cuadro -- solo la banda izquierda;
             * el asentamiento repinta la pantalla completa una vez. El
             * decode de JPEG NO corre durante la animación (regla del
             * repo); moonlit (D-057) sí permite una lectura PLANA
             * acotada por cuadro dentro de la propia animación
             * (moonlit_screen_marea_show_carousel() -> try_frame_bounded_read(),
             * nunca decode ni tagcache) -- al asentar,
             * moonlit_screen_marea_tick() sigue siendo quien decodifica,
             * ahora con su presupuesto más grande, y repinta la banda. */
            /* moonlit (D-059): pause the background master-art builder
             * for the whole duration of a Marea scroll animation --
             * "the animation owns the disk and the CPU"
             * (moonlit_master_art_builder.h). Recomputed every idle
             * iteration from the current state (never left dangling
             * true after backing out of Marea mid-scroll): outside
             * Marea, or once settled, the builder resumes. */
            moonlit_master_art_builder_pause(at_marea && moonlit_screen_marea_animating());
            if (at_marea)
            {
                if (moonlit_screen_marea_animating())
                    moonlit_screen_marea_show_carousel();
                else if (moonlit_screen_marea_tick())
                    moonlit_screen_marea_show_carousel();
                /* moonlit (D-078): ni la animacion del carrusel ni una
                 * tapa recien cargada ya repintaron este cuadro -- si
                 * el titulo o el subtitulo del panel siguen barriendo,
                 * les toca a ellos (moonlit_screen_marea_show_panel()
                 * es la contraparte, fina, de show_carousel() de
                 * arriba: solo el panel, nunca la banda). */
                else if (moonlit_marquee_wants_ticks())
                    moonlit_screen_marea_show_panel();
            }

            continue;
        }

        {
            /* F11: transitions are picked by diffing metro_nav_t
             * before/after the action instead of each screen module
             * announcing "I just pushed" -- one place knows the nav
             * stack shape, metro_screen_hub/list/nowplaying.c stay
             * exactly as they were before this phase. Order matters:
             * entering/leaving a sentinel page (Now Playing, F5; the
             * photo viewer, R2-F3) also changes depth (pushed like any
             * other page), so the sentinel-specific checks must win
             * over the generic push/pop ones, or "push(sentinel)"
             * would slide instead of fade. A push that lands on a LIST
             * page while ALREADY on a sentinel (e.g. MACT_OPTIONS from
             * Now Playing) falls through to the generic push case on
             * purpose -- see metro_transitions.h. Now Playing and the
             * viewer are mutually exclusive (metro_screen_photo_viewer.h),
             * so "either sentinel" is just player_x || viewer_x below,
             * never both true at once. */
            metro_nav_t *nav = metro_screen_nav();
            int depth_before = metro_nav_depth(nav);
            int pivot_before = metro_nav_pivot(nav);
            bool player_before = at_player;
            bool viewer_before = at_viewer;
            bool marea_before = at_marea;
            int depth_after, pivot_after;
            bool root_after, player_after, viewer_after, marea_after;

            if (at_root)
                metro_screen_hub_handle(action, steps);
            else if (at_player)
                metro_screen_nowplaying_handle(action, steps);
            else if (at_viewer)
                metro_screen_photo_viewer_handle(action, steps);
            else if (at_marea)
                moonlit_screen_marea_handle(action, steps);
            else
                metro_screen_list_handle(action, steps);

            depth_after = metro_nav_depth(nav);
            pivot_after = metro_nav_pivot(nav);
            root_after = metro_nav_is_root(nav);
            player_after = !root_after && metro_screen_nowplaying_is_current();
            viewer_after = !root_after && metro_screen_photo_viewer_is_current();
            /* moonlit (D-029, M8): tercer centinela -- misma regla de
             * FADE al entrar/salir que Now Playing/el visor de fotos. */
            marea_after = !root_after && moonlit_screen_marea_is_current();

            /* moonlit (D-059): every branch below either runs a
             * metro_transitions_* animation or draws a single Marea
             * scroll frame -- "the animation owns the disk and the
             * CPU" for the whole block, same reasoning as the idle
             * branch's pause around Marea's own scroll above. Reset
             * right after: none of these block for long, and the next
             * idle iteration would just recompute the same state
             * anyway. */
            moonlit_master_art_builder_pause(true);
            if (depth_after > depth_before && (player_after || viewer_after || marea_after))
                metro_transitions_fade(redraw_current);
            else if (depth_after > depth_before)
            {
                metro_transitions_push(redraw_current, 1);
                /* F12: the cascade only makes sense on the list this
                 * push landed on, never the hub (no rows, its own
                 * *_show()) or a sentinel (handled by the fade branch
                 * above, never reaches here). */
                if (!root_after && !player_after && !viewer_after && !marea_after)
                    metro_screen_list_run_feather_if_pending();
            }
            else if (depth_after < depth_before && (player_before || viewer_before || marea_before) &&
                     !player_after && !viewer_after && !marea_after)
                metro_transitions_fade(redraw_current);
            else if (depth_after < depth_before)
                metro_transitions_push(redraw_current, -1);
            else if (!root_after && pivot_after != pivot_before)
                metro_transitions_slide(redraw_current, pivot_after > pivot_before ? 1 : -1);
            /* moonlit (D-053): un paso de rueda en Marea no repinta la
             * pantalla completa -- dibuja el primer cuadro de la banda
             * ahora mismo (latencia cero) y deja el resto a la rama
             * ociosa de arriba. */
            else if (marea_after && moonlit_screen_marea_animating())
                moonlit_screen_marea_show_carousel();
            else
                redraw_current();
            moonlit_master_art_builder_pause(false);
        }
    }
}
