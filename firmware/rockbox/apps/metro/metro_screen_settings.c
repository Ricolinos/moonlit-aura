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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "settings.h"
#include "backlight.h"
#include "ata_idle_notify.h" /* call_storage_idle_notifys() -- moonlit (D-071) */
#include "dsp_misc.h"  /* dsp_replaygain_set_settings() -- moonlit (D-071) */
#include "powermgmt.h" /* R3-F6/DD-10: set_sleeptimer_duration()/get_sleep_timer() */
#include "eq.h" /* R3-F6/DD-10: dsp_eq_enable()/dsp_set_eq_coefs() */

#include "metro_screen_settings.h"
#include "metro_screen_list.h" /* moonlit (D-047): push del submenu "cambiar sistema" */
#include "metro_screen_about.h"
#include "metro_screen_lock.h"
#include "metro_screen_adjust.h" /* moonlit (D-071) */
#include "metro_screen_text.h"   /* moonlit (D-071) */
#include "metro_lang.h"
#include "metro_theme.h"
#include "metro_widgets.h"
#include "metro_settings.h"
#include "metro_firmware_families.h" /* moonlit (D-047): "cambiar sistema" */
#include "metro_sync.h"
#include "metro_main.h"
#include "metro_music.h"  /* R5-F3: límite de volumen */
#include "metro_volume.h"

/* --- general: language, library update, reset settings ---------------- */

static const enum metro_lang_id anim_names[METRO_ANIM_COUNT] = {
    LANG_ANIM_OFF, LANG_ANIM_MINIMAL, LANG_ANIM_ALL,
};
static const enum metro_lang_id gfx_names[METRO_GFX_COUNT] = {
    LANG_GFX_LITE, LANG_GFX_FULL,
};

/* R3-F6/DD-10: cicla desactivado -> 15 -> 30 -> 60 -> 90 min llamando
 * set_sleeptimer_duration() directo (firmware/powermgmt.c) -- sin
 * pantalla propia, sin tocar apps/settings.h (Aura no implementa esta
 * feature en absoluto, INVESTIGACION-metro-r3.md F.1). El índice de
 * qué paso sigue se guarda aparte de get_sleep_timer() (que cuenta
 * hacia abajo en tiempo real, no sirve como "posición en la lista" a
 * ciclar) -- puramente de sesión, un temporizador de sueño reinicia en
 * cada boot en el propio Rockbox stock también, no hay nada que
 * persistir. */
static const int sleep_steps_min[] = { 0, 15, 30, 60, 90 };
#define SLEEP_STEPS_N (int)(sizeof(sleep_steps_min) / sizeof(sleep_steps_min[0]))
static int s_sleep_step = 0;

static void cycle_sleep(void)
{
    s_sleep_step = (s_sleep_step + 1) % SLEEP_STEPS_N;
    set_sleeptimer_duration(sleep_steps_min[s_sleep_step]);
}

static const char *sleep_subtitle(void)
{
    static char buf[16];
    int remaining_min = get_sleep_timer() / 60;

    if (remaining_min <= 0)
        return metro_lang_str(LANG_VALUE_OFF);

    snprintf(buf, sizeof(buf), "%d min", remaining_min);
    return buf;
}

/* R3-F6/DD-10: tabla propia de Metro, no la de settings.h (Aura sí
 * necesita ese extern de 4 líneas porque reusa los defaults de stock;
 * Metro no los reusa -- INVESTIGACION-metro-r3.md F.2, cero cambios a
 * `apps/settings.h`). Forma de banda (tipo/corte/Q) igual para los 4
 * presets, el mismo layout de 10 bandas que el menú EQ de stock usa
 * por default (`apps/settings_list.c`) -- valores genéricos de
 * ingeniería de audio, no código de Aura -- solo la GANANCIA por banda
 * cambia de preset a preset. Ganancia en décimas de dB (`eq_menu.h`:
 * EQ_GAIN_MIN/MAX = -240/240, o sea ±24.0 dB). */
enum metro_eq_preset {
    METRO_EQ_FLAT = 0,
    METRO_EQ_BASS,
    METRO_EQ_VOCAL,
    METRO_EQ_BRIGHT,
    METRO_EQ_PRESET_COUNT
};

static const enum metro_lang_id eq_preset_names[METRO_EQ_PRESET_COUNT] = {
    LANG_EQ_FLAT, LANG_EQ_BASS, LANG_EQ_VOCAL, LANG_EQ_BRIGHT,
};

struct eq_band_shape { enum eq_filter_type type; int cutoff; int q; };
static const struct eq_band_shape eq_shapes[EQ_NUM_BANDS] = {
    { EQ_FILTER_LOW_SHELF,     32,  7 },
    { EQ_FILTER_PEAK,          64, 10 },
    { EQ_FILTER_PEAK,         125, 10 },
    { EQ_FILTER_PEAK,         250, 10 },
    { EQ_FILTER_PEAK,         500, 10 },
    { EQ_FILTER_PEAK,        1000, 10 },
    { EQ_FILTER_PEAK,        2000, 10 },
    { EQ_FILTER_PEAK,        4000, 10 },
    { EQ_FILTER_PEAK,        8000, 10 },
    { EQ_FILTER_HIGH_SHELF, 16000,  7 },
};

static const int eq_preset_gains[METRO_EQ_PRESET_COUNT][EQ_NUM_BANDS] = {
    /* plano */     {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
    /* graves */    {  60,  40,  20,   0,   0,   0,   0,   0,   0,   0 },
    /* voz */       { -20, -10,   0,   0,  20,  40,  30,   0,   0,   0 },
    /* brillante */ {   0,   0,   0,   0,   0,   0,   0,  30,  40,  50 },
};

static int s_eq_preset = METRO_EQ_FLAT;

static void apply_eq_preset(enum metro_eq_preset preset)
{
    int i;

    /* Plano == sin procesamiento -- apagar el DSP entero en vez de
     * aplicar 10 bandas en 0 es más barato y más correcto (bypass
     * real, no "todas las bandas midiendo cero"). */
    dsp_eq_enable(preset != METRO_EQ_FLAT);

    for (i = 0; i < EQ_NUM_BANDS; i++)
    {
        struct eq_band_setting band;

        band.type   = eq_shapes[i].type;
        band.cutoff = eq_shapes[i].cutoff;
        band.q      = eq_shapes[i].q;
        band.gain   = eq_preset_gains[preset][i];
        dsp_set_eq_coefs(i, &band);
    }
}

static void cycle_eq(void)
{
    s_eq_preset = (s_eq_preset + 1) % METRO_EQ_PRESET_COUNT;
    apply_eq_preset((enum metro_eq_preset)s_eq_preset);
}

/* R5-F3 (M-083): límite de volumen en la escala 00..15 de Metro
 * (metro_volume.h), nunca en dB. Cinco presets que se ciclan con SELECT,
 * igual que brillo/retroiluminación -- 16 valores uno a uno serían
 * quince pulsaciones en el peor caso, y un límite por debajo de 06 no
 * tiene uso real. Persiste en global_settings.volume_limit (Rockbox). */
static const int volume_limit_steps[] = { 15, 12, 10, 8, 6 };
#define VOLUME_LIMIT_STEPS_N (int)(sizeof(volume_limit_steps) / sizeof(volume_limit_steps[0]))

static const char *volume_limit_subtitle(void)
{
    static char buf[4];
    snprintf(buf, sizeof(buf), "%02d", metro_music_volume_limit_level());
    return buf;
}

static void cycle_volume_limit(void)
{
    int cur = metro_music_volume_limit_level();
    int i, next = volume_limit_steps[0];

    /* Siguiente preset ESTRICTAMENTE menor que el actual; si no hay
     * (estamos en el más bajo, o en un valor raro por debajo), vuelve
     * al máximo. */
    for (i = 0; i < VOLUME_LIMIT_STEPS_N; i++)
        if (volume_limit_steps[i] < cur)
        {
            next = volume_limit_steps[i];
            break;
        }
    metro_music_set_volume_limit_level(next);
}

/* --- "cambiar sistema" (moonlit D-047, contrato v10 con tres familias)
 * Una fila por familia hermana de metro_firmware_families.h, con las
 * mismas primitivas que el resto de Ajustes (metro_row de la lista
 * generica: titulo en fuente de cuerpo, subtitulo en fuente de valor,
 * colores por rol via moonlit_color() dentro de metro_draw). Sin arbol
 * dormido la fila lleva "no instalado" y no hace nada; con el, pide
 * confirmacion y hace el cambio (metro_firmware_switch_to solo vuelve
 * si no pudo: el aparato sigue siendo moonlit y la lista se redibuja). */
static int switch_count(void *ctx)
{
    (void)ctx;
    return metro_fw_sibling_count();
}

static void switch_get_row(void *ctx, int index, struct metro_row *out)
{
    const struct metro_fw_family *sibling = metro_fw_sibling(index);
    (void)ctx;

    out->title = metro_lang_str(sibling->name);
    out->subtitle = metro_firmware_sibling_installed(index)
                        ? NULL : metro_lang_str(LANG_VALUE_NOT_INSTALLED);
    out->kind = METRO_ROW_ACTION;
}

static void switch_on_select(void *ctx, int index)
{
    const struct metro_fw_family *sibling = metro_fw_sibling(index);
    char question[96];
    (void)ctx;

    if (sibling == NULL || !metro_firmware_sibling_installed(index))
        return;

    snprintf(question, sizeof(question),
             metro_lang_str(LANG_DIALOG_SWITCH_FMT),
             metro_lang_str(sibling->name));
    if (metro_widgets_confirm(metro_lang_str(LANG_HUB_SETTINGS), question))
        metro_firmware_switch_to(index);
}

static const struct metro_pivot switch_pivots[] = {
    { .name = LANG_SETTING_SWITCH_SYSTEM, .count = switch_count,
      .get_row = switch_get_row, .on_select = switch_on_select },
};
static const struct metro_page switch_page = {
    LANG_SETTING_SWITCH_SYSTEM, switch_pivots, 1, NULL
};

/* --- moonlit (D-069, maestro SS D): sub-pagina de Bloqueo ---------- */

/* Sin clave configurada solo tiene sentido "activar": cambiar, quitar o
 * elegir cuando pedir una clave que no existe son filas muertas. Con
 * clave, las cuatro. Mismo criterio que Metro (M-104). */
enum {
    LOCK_ROW_ENABLE = 0,   /* sin clave: la unica fila */
    LOCK_ROW_CHANGE = 0,   /* con clave, primera */
    LOCK_ROW_REQUIRE,
    LOCK_ROW_REMOVE,
    LOCK_ROW_COUNT_ON,
};

static bool lock_is_set(void)
{
    return metro_screen_lock_state() != METRO_LOCK_NONE;
}

static int lock_count(void *ctx)
{
    (void)ctx;
    return lock_is_set() ? LOCK_ROW_COUNT_ON : 1;
}

static enum metro_lang_id lock_require_label(void)
{
    switch (metro_settings.screen_lock_require)
    {
        case METRO_LOCK_REQUIRE_1MIN: return LANG_LOCK_REQUIRE_1MIN;
        case METRO_LOCK_REQUIRE_5MIN: return LANG_LOCK_REQUIRE_5MIN;
        case METRO_LOCK_REQUIRE_BOOT: return LANG_LOCK_REQUIRE_BOOT;
        case METRO_LOCK_REQUIRE_HOLD:
        default:                      return LANG_LOCK_REQUIRE_HOLD;
    }
}

static void lock_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;

    if (!lock_is_set())
    {
        out->title = metro_lang_str(LANG_LOCK_ENABLE);
        return;
    }

    switch (index)
    {
        case LOCK_ROW_CHANGE:
            out->title = metro_lang_str(LANG_LOCK_CHANGE);
            break;
        case LOCK_ROW_REQUIRE:
            out->title = metro_lang_str(LANG_LOCK_REQUIRE);
            out->subtitle = metro_lang_str(lock_require_label());
            out->kind = METRO_ROW_SETTING;
            break;
        case LOCK_ROW_REMOVE:
        default:
            out->title = metro_lang_str(LANG_LOCK_REMOVE);
            break;
    }
}

static void lock_on_select(void *ctx, int index)
{
    (void)ctx;

    if (!lock_is_set())
    {
        metro_screen_lock_setup();
        return;
    }

    switch (index)
    {
        case LOCK_ROW_CHANGE:
            metro_screen_lock_setup();
            break;
        case LOCK_ROW_REQUIRE:
            metro_settings.screen_lock_require =
                (enum metro_lock_require)((metro_settings.screen_lock_require + 1)
                                           % METRO_LOCK_REQUIRE_COUNT);
            metro_settings_save();
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;
        case LOCK_ROW_REMOVE:
        default:
            /* Quitar pide confirmacion (destruye la clave guardada);
             * poner no la necesita -- la doble captura ya lo es. */
            if (metro_widgets_confirm(metro_lang_str(LANG_SETTING_LOCK),
                                       metro_lang_str(LANG_DIALOG_LOCK_OFF_TITLE)))
                metro_screen_lock_clear();
            break;
    }
}

static const struct metro_pivot lock_pivots[] = {
    { .name = LANG_SETTING_LOCK, .count = lock_count,
      .get_row = lock_get_row, .on_select = lock_on_select },
};
static const struct metro_page lock_page = {
    LANG_SETTING_LOCK, lock_pivots, 1, NULL
};

/* moonlit (D-071): guardar los ajustes de Rockbox NO los escribe en el
 * acto -- settings_save() registra un callback en DISK_EVENT_SPINUP y
 * call_storage_idle_notifys() se auto-bloquea 30 s entre corridas, asi
 * que el flush real llegaba con el apagado limpio. Un aparato al que se
 * le acaba la bateria perdia el cambio. Toda fila que toque
 * global_settings termina aqui. (Hallazgo de Metro R7-5, estandar de
 * los tres repos.) */
static void save_global_settings_now(void)
{
    settings_save();
    call_storage_idle_notifys(true);
}

/* moonlit (D-071 addendum, maestro SS C): "ajuste de volumen"
 * (replaygain) con TRES valores -- desactivado / por pista / por album.
 * Rockbox tiene un cuarto, REPLAYGAIN_SHUFFLE ("track shuffle"), que
 * NO se expone: significa "por pista solo cuando el aleatorio esta
 * puesto", una condicion que hay que explicar para poder elegirla y que
 * en una lista de tres palabras no cabe explicar. El que no lo tenga
 * puesto desde otro firmware ve la fila en "por pista", que es lo que
 * REPLAYGAIN_SHUFFLE hace la mitad del tiempo. Mismo criterio que
 * Metro (M-103). */
static enum metro_lang_id replaygain_label(void)
{
    switch (global_settings.replaygain_settings.type)
    {
        case REPLAYGAIN_ALBUM: return LANG_REPLAYGAIN_ALBUM;
        case REPLAYGAIN_OFF:   return LANG_VALUE_OFF;
        default:               return LANG_REPLAYGAIN_TRACK;
    }
}

static void cycle_replaygain(void)
{
    int next;

    switch (global_settings.replaygain_settings.type)
    {
        case REPLAYGAIN_OFF:   next = REPLAYGAIN_TRACK; break;
        case REPLAYGAIN_ALBUM: next = REPLAYGAIN_OFF;   break;
        default:               next = REPLAYGAIN_ALBUM; break;
    }
    global_settings.replaygain_settings.type = next;
    /* En vivo: el DSP lo aplica a la pista en curso, no al siguiente
     * arranque. */
    dsp_replaygain_set_settings(&global_settings.replaygain_settings);
    save_global_settings_now();
    metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
}

static const char *poweroff_subtitle(void)
{
    static char buf[16];

    /* moonlit (D-071): 0 = nunca (apps/settings_list.c, UNIT_MIN). */
    if (global_settings.poweroff <= 0)
        return metro_lang_str(LANG_VALUE_NEVER);
    snprintf(buf, sizeof(buf), "%d min", global_settings.poweroff);
    return buf;
}

static int general_count(void *ctx)
{
    (void)ctx;
    return 14; /* moonlit (D-071): +4 filas (apagado, clicker, replaygain, legales) */
}

static void general_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;

    switch (index)
    {
        case 0:
            out->title = metro_lang_str(LANG_SETTING_LANGUAGE);
            /* moonlit (D-080, maestro SS D.2): nombre NATIVO, no
             * traducido -- "Русский" se ve igual sin importar si la
             * interfaz esta en español o en alemán. */
            out->subtitle = metro_lang_native_name(metro_lang_get());
            out->kind = METRO_ROW_SETTING;
            break;
        case 1:
            out->title = metro_lang_str(LANG_SETTING_ANIMATIONS);
            out->subtitle = metro_lang_str(anim_names[metro_settings.animations]);
            out->kind = METRO_ROW_SETTING;
            break;
        case 2:
            out->title = metro_lang_str(LANG_SETTING_GRAPHICS);
            out->subtitle = metro_lang_str(gfx_names[metro_settings.graphics]);
            out->kind = METRO_ROW_SETTING;
            break;
        case 3:
            out->title = metro_lang_str(LANG_SETTING_SLEEP);
            out->subtitle = sleep_subtitle();
            out->kind = METRO_ROW_SETTING;
            break;
        case 4:
            out->title = metro_lang_str(LANG_SETTING_EQ);
            out->subtitle = metro_lang_str(eq_preset_names[s_eq_preset]);
            out->kind = METRO_ROW_SETTING;
            break;
        case 5:
            out->title = metro_lang_str(LANG_SETTING_VOLUME_LIMIT);
            out->subtitle = volume_limit_subtitle();
            out->kind = METRO_ROW_SETTING;
            break;
        case 6:
            /* R3-F7/DD-8 (M-068): estado real del candado, no la
             * preferencia guardada -- ARMED y ACTIVE se ven igual desde
             * aquí (para llegar a esta fila el aparato ya está
             * desbloqueado), así que basta con "activado"/"desactivado". */
            out->title = metro_lang_str(LANG_SETTING_LOCK);
            out->subtitle = metro_lang_str(
                metro_screen_lock_state() == METRO_LOCK_NONE ? LANG_VALUE_OFF
                                                              : LANG_VALUE_ON);
            out->kind = METRO_ROW_SETTING;
            break;
        case 7:
            out->title = metro_lang_str(LANG_SETTING_LIBRARY);
            out->subtitle = NULL;
            out->kind = METRO_ROW_ACTION;
            break;
        /* moonlit (D-071, maestro SS C): las tres filas que moonlit no
         * tenia y Aura si. El apagado automatico es la que de verdad
         * faltaba -- sin ella, un aparato olvidado en un bolsillo se
         * queda encendido hasta agotar la bateria. */
        case 8:
            out->title = metro_lang_str(LANG_SETTING_POWEROFF);
            out->subtitle = poweroff_subtitle();
            out->kind = METRO_ROW_SETTING;
            break;
        case 9:
            out->title = metro_lang_str(LANG_SETTING_CLICKER);
            out->subtitle = metro_lang_str(global_settings.keyclick
                                                ? LANG_VALUE_ON : LANG_VALUE_OFF);
            out->kind = METRO_ROW_SETTING;
            break;
        case 10:
            out->title = metro_lang_str(LANG_SETTING_REPLAYGAIN);
            out->subtitle = metro_lang_str(replaygain_label());
            out->kind = METRO_ROW_SETTING;
            break;
        case 11:
            /* GPL v2 SS3: el aviso de licencia tiene que estar a la vista
             * del usuario, no solo en el repositorio. */
            out->title = metro_lang_str(LANG_SETTING_LEGAL);
            out->subtitle = NULL;
            out->kind = METRO_ROW_NAV;
            break;
        case 12:
            /* R5 (M-090, contrato v10) / moonlit D-047: submenu con una
             * fila por familia hermana (Aura, Metro). La fila siempre
             * se ve, para que se sepa que existe la opcion; las hermanas
             * sin arbol dormido quedan inertes dentro del submenu. */
            out->title = metro_lang_str(LANG_SETTING_SWITCH_SYSTEM);
            out->subtitle = NULL;
            out->kind = METRO_ROW_NAV;
            break;
        default:
            out->title = metro_lang_str(LANG_SETTING_RESET);
            out->subtitle = NULL;
            out->kind = METRO_ROW_ACTION;
            break;
    }
}

static void general_on_select(void *ctx, int index)
{
    (void)ctx;

    switch (index)
    {
        case 0:
            /* moonlit (D-080): recorre los seis en el orden de
             * enum metro_language (SELECT avanza uno, envuelve). */
            metro_lang_set((enum metro_language)((metro_lang_get() + 1) % METRO_LANG_COUNT));
            metro_settings.language = metro_lang_get();
            metro_settings_save();
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;

        case 1:
            metro_settings.animations =
                (enum metro_anim_level)((metro_settings.animations + 1) % METRO_ANIM_COUNT);
            metro_settings_save();
            break;

        case 2:
            metro_settings.graphics =
                (enum metro_gfx_level)((metro_settings.graphics + 1) % METRO_GFX_COUNT);
            metro_settings_save();
            break;

        case 3:
            cycle_sleep();
            break;

        case 4:
            cycle_eq();
            break;

        case 5:
            cycle_volume_limit();
            /* moonlit (D-071): metro_music_set_volume_limit_level() ya
             * llama settings_save(), pero eso solo REGISTRA el guardado
             * -- el flush hay que forzarlo igual que en las demas filas
             * que tocan global_settings. */
            call_storage_idle_notifys(true);
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;

        case 6:
            /* moonlit (D-069): la fila deja de hacer una sola cosa y
             * empuja la sub-pagina de Bloqueo -- ahora hay cuatro
             * (activar, cambiar codigo, pedir codigo, quitar), y la de
             * "pedir codigo" es la que hace que el bloqueo sirva para
             * algo mas que el arranque. Mismo patron que "cambiar
             * sistema" (D-047). */
            metro_screen_list_push(&lock_page);
            break;

        case 7:
            /* D-061: la advertencia de duración va como detalle, no
             * dentro de la pregunta -- draw_question() está topada en
             * dos líneas y la cortaría a la mitad. */
            if (metro_widgets_confirm_detail(metro_lang_str(LANG_HUB_SETTINGS),
                                              metro_lang_str(LANG_DIALOG_LIBRARY_TITLE),
                                              metro_lang_str(LANG_DIALOG_LIBRARY_DETAIL)))
            {
                metro_sync_request_manual();
                metro_run_sync_screen_if_needed();
            }
            break;

        case 8:
        {
            /* moonlit (D-071): {nunca, 10, 20, 60 min}, los mismos
             * valores que Aura. global_settings.poweroff esta en
             * minutos y 0 = nunca (apps/settings_list.c). */
            static const int steps[] = { 0, 10, 20, 60 };
            int i, next = steps[0];

            for (i = 0; i < (int)(sizeof(steps) / sizeof(steps[0])); i++)
                if (steps[i] > global_settings.poweroff)
                {
                    next = steps[i];
                    break;
                }
            global_settings.poweroff = next;
            set_poweroff_timeout(next);
            save_global_settings_now();
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;
        }

        case 9:
            /* keyclick de Rockbox: 0 = apagado, 1..3 intensidades. Aqui
             * es un interruptor -- moonlit no expone tres fuerzas de un
             * clic que dura 10 ms. */
            global_settings.keyclick = global_settings.keyclick ? 0 : 2;
            save_global_settings_now();
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;

        case 10:
            cycle_replaygain();
            break;

        case 11:
            metro_screen_text_show(metro_lang_str(LANG_SETTING_LEGAL),
                                    metro_lang_str(LANG_LEGAL_BODY));
            break;

        case 12:
            metro_screen_list_push(&switch_page);
            break;

        default:
            if (metro_widgets_confirm(metro_lang_str(LANG_HUB_SETTINGS),
                                       metro_lang_str(LANG_DIALOG_RESET_TITLE)))
            {
                metro_settings.theme = METRO_THEME_DEFAULT;
                metro_settings.accent = METRO_ACCENT_DEFAULT;
                metro_settings.language = METRO_LANG_ES;
                metro_settings.animations = METRO_ANIM_DEFAULT;
                metro_settings.graphics = METRO_GFX_DEFAULT;
                /* moonlit (D-079, maestro SS A.2.4): "restablecer
                 * ajustes" tambien vuelve a los valores por defecto de
                 * las CLAVES COMPARTIDAS -- antes de esta ronda el
                 * candado y los cuatro ajustes homologados (D-071)
                 * quedaban fuera de este boton por completo. */
                metro_settings.screen_lock = false;
                metro_settings.screen_lock_pin[0] = '\0';
                metro_settings.screen_lock_require = METRO_LOCK_REQUIRE_HOLD;
                metro_settings_save();

                metro_theme_set(metro_settings.theme);
                metro_accent_set(metro_settings.accent);
                metro_lang_set(metro_settings.language);

                /* R3-F6/DD-10: sueño/EQ son de sesión, no parte de
                 * metro_settings -- pero "restablecer ajustes" debería
                 * verse consistente con el resto de esta fila. */
                s_sleep_step = 0;
                set_sleeptimer_duration(sleep_steps_min[s_sleep_step]);
                s_eq_preset = METRO_EQ_FLAT;
                apply_eq_preset((enum metro_eq_preset)s_eq_preset);
                metro_music_set_volume_limit_level(METRO_VOLUME_MAX_LEVEL);

                global_settings.backlight_timeout = 15;
                backlight_set_timeout(global_settings.backlight_timeout);
                global_settings.poweroff = 0;
                set_poweroff_timeout(global_settings.poweroff);
                global_settings.keyclick = 0;
                global_settings.replaygain_settings.type = REPLAYGAIN_OFF;
                dsp_replaygain_set_settings(&global_settings.replaygain_settings);
                save_global_settings_now();

                metro_settings_write_shared(); /* moonlit (D-079): rev+1 con los defaults */
            }
            break;
    }
}

/* --- display: theme, accent, brightness, backlight --------------------- */

/* moonlit (D-028, M4): 4 presets MD3 en vez de los 10 acentos WP7. */
static const enum metro_lang_id accent_names[METRO_ACCENT_COUNT] = {
    LANG_ACCENT_MOONSTONE, LANG_ACCENT_TIDE, LANG_ACCENT_EMBER, LANG_ACCENT_MOSS,
};

/* moonlit (D-071, maestro SS C): brillo y retroiluminacion dejan de
 * ciclar valores con SELECT y pasan a su propia pantalla de barra
 * (metro_screen_adjust.h). Con cuatro pasos de brillo no se podia
 * afinar, y con seis de retroiluminacion llegar al que uno quiere podia
 * costar cinco pulsaciones sin ver nunca el rango completo. El brillo
 * pasa de 4 pasos fijos a los DIEZ que pide el maestro, repartidos
 * sobre el rango real del aparato; la retroiluminacion conserva sus
 * seis valores no lineales (incluido "nunca"), que no son una rejilla
 * y no tendria sentido interpolar. */
#define BRIGHTNESS_STEPS_N 10

static int brightness_of_step(int step)
{
    /* Reparto lineal sobre 1..MAX_BRIGHTNESS_SETTING, con el paso 0 en
     * el minimo REAL del aparato (nunca 0: apagar la pantalla no es un
     * nivel de brillo). */
    int lo = 1, hi = MAX_BRIGHTNESS_SETTING;

    return lo + (hi - lo) * step / (BRIGHTNESS_STEPS_N - 1);
}

static int brightness_to_step(int value)
{
    int lo = 1, hi = MAX_BRIGHTNESS_SETTING;
    int step;

    if (value <= lo)
        return 0;
    if (value >= hi)
        return BRIGHTNESS_STEPS_N - 1;
    step = (value - lo) * (BRIGHTNESS_STEPS_N - 1) / (hi - lo);
    return step;
}

static const int backlight_steps[] = { 5, 10, 15, 30, 60, -1 }; /* -1 = never */
#define BACKLIGHT_STEPS_N (int)(sizeof(backlight_steps) / sizeof(backlight_steps[0]))

/* --- especificaciones de las dos pantallas de barra (D-071) -------- */

static const char *brightness_label(void *ctx, int step)
{
    static char buf[16];

    (void)ctx;
    snprintf(buf, sizeof(buf), "%d%%",
             brightness_of_step(step) * 100 / MAX_BRIGHTNESS_SETTING);
    return buf;
}

static void brightness_apply(void *ctx, int step)
{
    (void)ctx;
    global_settings.brightness = brightness_of_step(step);
    backlight_set_brightness(global_settings.brightness);
}

static const char *backlight_label(void *ctx, int step)
{
    static char buf[16];

    (void)ctx;
    if (backlight_steps[step] < 0)
        return metro_lang_str(LANG_VALUE_NEVER);
    snprintf(buf, sizeof(buf), "%ds", backlight_steps[step]);
    return buf;
}

static void backlight_apply(void *ctx, int step)
{
    (void)ctx;
    global_settings.backlight_timeout = backlight_steps[step];
    backlight_set_timeout(global_settings.backlight_timeout);
}

static int display_count(void *ctx)
{
    (void)ctx;
    return 4;
}

static void display_get_row(void *ctx, int index, struct metro_row *out)
{
    /* One buffer per row that formats a value -- rows are drawn
     * straight after get_row today, but a shared buffer would silently
     * show the last-formatted value on both rows the moment a caller
     * keeps two struct metro_row around (R5-F1 audit). */
    static char brightness_buf[16];
    static char backlight_buf[16];

    (void)ctx;
    out->kind = METRO_ROW_SETTING;

    switch (index)
    {
        case 0:
            out->title = metro_lang_str(LANG_SETTING_THEME);
            out->subtitle = metro_lang_str(metro_theme_get() == METRO_THEME_DARK
                                                ? LANG_VALUE_DARK : LANG_VALUE_LIGHT);
            break;
        case 1:
            out->title = metro_lang_str(LANG_SETTING_ACCENT);
            out->subtitle = metro_lang_str(accent_names[metro_accent_get()]);
            break;
        case 2:
            out->title = metro_lang_str(LANG_SETTING_BRIGHTNESS);
            /* D-071: el rotulo es la POSICION DEL CONTROL, no el crudo
             * de global_settings -- si no, la fila y la barra pueden
             * mostrar dos porcentajes distintos para el mismo estado. */
            snprintf(brightness_buf, sizeof(brightness_buf), "%s",
                     brightness_label(NULL,
                         brightness_to_step(global_settings.brightness)));
            out->subtitle = brightness_buf;
            break;
        default:
            out->title = metro_lang_str(LANG_SETTING_BACKLIGHT);
            if (global_settings.backlight_timeout < 0)
                out->subtitle = metro_lang_str(LANG_VALUE_NEVER);
            else
            {
                snprintf(backlight_buf, sizeof(backlight_buf), "%ds",
                         global_settings.backlight_timeout);
                out->subtitle = backlight_buf;
            }
            break;
    }
}

static void display_on_select(void *ctx, int index)
{
    (void)ctx;

    switch (index)
    {
        case 0:
            metro_theme_set(metro_theme_get() == METRO_THEME_DARK
                                 ? METRO_THEME_LIGHT : METRO_THEME_DARK);
            metro_settings.theme = metro_theme_get();
            metro_settings_save();
            metro_settings_write_shared(); /* moonlit (D-079): "appearance" */
            break;

        case 1:
        {
            int next = (metro_accent_get() + 1) % METRO_ACCENT_COUNT;
            metro_accent_set((enum metro_accent)next);
            metro_settings.accent = metro_accent_get();
            metro_settings_save();
            break;
        }

        case 2:
        {
            static const struct metro_adjust_spec spec = {
                LANG_SETTING_BRIGHTNESS, BRIGHTNESS_STEPS_N,
                brightness_label, brightness_apply, NULL
            };

            metro_screen_adjust_run(&spec, brightness_to_step(global_settings.brightness));
            save_global_settings_now(); /* D-071: un solo guardado, al salir */
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;
        }

        default:
        {
            static const struct metro_adjust_spec spec = {
                LANG_SETTING_BACKLIGHT, BACKLIGHT_STEPS_N,
                backlight_label, backlight_apply, NULL
            };
            int i, start = 0;

            for (i = 0; i < BACKLIGHT_STEPS_N; i++)
                if (backlight_steps[i] == global_settings.backlight_timeout)
                {
                    start = i;
                    break;
                }
            metro_screen_adjust_run(&spec, start);
            save_global_settings_now();
            metro_settings_write_shared(); /* moonlit (D-079): clave compartida */
            break;
        }
    }
}

/* metro_screen_about_pivot (metro_screen_about.c) owns the 3rd pivot's
 * provider -- About outgrew a single static row (F8: device name,
 * sync counts) -- it's an extern const from another translation unit,
 * so it can't sit in a static initializer here; all_pivots[] is filled
 * in once, at first use. */
static struct metro_pivot all_pivots[3];
static const struct metro_page settings_page = {
    LANG_HUB_SETTINGS, all_pivots, 3, NULL
};

const struct metro_page *metro_screen_settings_page(void)
{
    static bool built = false;

    if (!built)
    {
        all_pivots[0] = (struct metro_pivot){
            LANG_PIVOT_GENERAL, general_count, general_get_row, general_on_select, NULL };
        all_pivots[1] = (struct metro_pivot){
            LANG_PIVOT_DISPLAY, display_count, display_get_row, display_on_select, NULL };
        all_pivots[2] = metro_screen_about_pivot;
        built = true;
    }
    return &settings_page;
}
