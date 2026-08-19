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

#include "metro_screen_settings.h"
#include "metro_screen_about.h"
#include "metro_lang.h"
#include "metro_theme.h"
#include "metro_widgets.h"
#include "metro_settings.h"
#include "metro_sync.h"
#include "metro_main.h"

/* --- general: language, library update, reset settings ---------------- */

static int general_count(void *ctx)
{
    (void)ctx;
    return 3;
}

static void general_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;

    switch (index)
    {
        case 0:
            out->title = metro_lang_str(LANG_SETTING_LANGUAGE);
            out->subtitle = metro_lang_str(metro_lang_get() == METRO_LANG_ES
                                                ? LANG_VALUE_SPANISH
                                                : LANG_VALUE_ENGLISH);
            out->kind = METRO_ROW_SETTING;
            break;
        case 1:
            out->title = metro_lang_str(LANG_SETTING_LIBRARY);
            out->subtitle = NULL;
            out->kind = METRO_ROW_ACTION;
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
            metro_lang_set(metro_lang_get() == METRO_LANG_ES
                                ? METRO_LANG_EN : METRO_LANG_ES);
            metro_settings.language = metro_lang_get();
            metro_settings_save();
            break;

        case 1:
            if (metro_widgets_confirm(metro_lang_str(LANG_HUB_SETTINGS),
                                       metro_lang_str(LANG_DIALOG_LIBRARY_TITLE)))
            {
                metro_sync_request_manual();
                metro_run_sync_screen_if_needed();
            }
            break;

        default:
            if (metro_widgets_confirm(metro_lang_str(LANG_HUB_SETTINGS),
                                       metro_lang_str(LANG_DIALOG_RESET_TITLE)))
            {
                metro_settings.theme = METRO_THEME_DEFAULT;
                metro_settings.accent = METRO_ACCENT_DEFAULT;
                metro_settings.language = METRO_LANG_ES;
                metro_settings.animations = 1;
                metro_settings.graphics = 1;
                metro_settings_save();

                metro_theme_set(metro_settings.theme);
                metro_accent_set(metro_settings.accent);
                metro_lang_set(metro_settings.language);
            }
            break;
    }
}

/* --- display: theme, accent, brightness, backlight --------------------- */

static const enum metro_lang_id accent_names[METRO_ACCENT_COUNT] = {
    LANG_ACCENT_BLUE, LANG_ACCENT_BROWN, LANG_ACCENT_GREEN, LANG_ACCENT_LIME,
    LANG_ACCENT_MAGENTA, LANG_ACCENT_MANGO, LANG_ACCENT_PINK,
    LANG_ACCENT_PURPLE, LANG_ACCENT_RED, LANG_ACCENT_TEAL,
};

/* A handful of presets rather than the raw 1..MAX_BRIGHTNESS_SETTING
 * range or every possible timeout value -- same "cycle through a
 * short list via SELECT" pattern as theme/accent/repeat, simpler than
 * a slider Metro's input model doesn't have anyway (no drag gesture on
 * a clickwheel). */
static const int brightness_steps[] = { 16, 32, 48, MAX_BRIGHTNESS_SETTING };
#define BRIGHTNESS_STEPS_N (int)(sizeof(brightness_steps) / sizeof(brightness_steps[0]))

static const int backlight_steps[] = { 5, 10, 15, 30, 60, -1 }; /* -1 = never */
#define BACKLIGHT_STEPS_N (int)(sizeof(backlight_steps) / sizeof(backlight_steps[0]))

static int display_count(void *ctx)
{
    (void)ctx;
    return 4;
}

static void display_get_row(void *ctx, int index, struct metro_row *out)
{
    static char buf[16];

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
            snprintf(buf, sizeof(buf), "%d%%",
                     global_settings.brightness * 100 / MAX_BRIGHTNESS_SETTING);
            out->subtitle = buf;
            break;
        default:
            out->title = metro_lang_str(LANG_SETTING_BACKLIGHT);
            if (global_settings.backlight_timeout < 0)
                out->subtitle = metro_lang_str(LANG_VALUE_NEVER);
            else
            {
                snprintf(buf, sizeof(buf), "%ds", global_settings.backlight_timeout);
                out->subtitle = buf;
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
            int i, next = brightness_steps[0];
            for (i = 0; i < BRIGHTNESS_STEPS_N; i++)
                if (brightness_steps[i] > global_settings.brightness)
                {
                    next = brightness_steps[i];
                    break;
                }
            global_settings.brightness = next;
            backlight_set_brightness(next);
            settings_save();
            break;
        }

        default:
        {
            int i, next = backlight_steps[0];
            for (i = 0; i < BACKLIGHT_STEPS_N; i++)
                if (backlight_steps[i] > global_settings.backlight_timeout)
                {
                    next = backlight_steps[i];
                    break;
                }
            global_settings.backlight_timeout = next;
            backlight_set_timeout(next);
            settings_save();
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
