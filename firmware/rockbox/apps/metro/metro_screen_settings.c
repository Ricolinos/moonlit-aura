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
#include <stddef.h>

#include "metro_screen_settings.h"
#include "metro_lang.h"
#include "metro_theme.h"
#include "metro_widgets.h"

/* --- general: language, reset settings ------------------------------- */

static int general_count(void *ctx)
{
    (void)ctx;
    return 2;
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

    if (index == 0)
    {
        metro_lang_set(metro_lang_get() == METRO_LANG_ES
                            ? METRO_LANG_EN : METRO_LANG_ES);
    }
    else
    {
        /* F3: proves the dialog widget end to end. The result isn't
         * acted on -- there's nothing real to reset until
         * metro_settings.c exists (F6/F8). */
        metro_widgets_confirm(metro_lang_str(LANG_HUB_SETTINGS),
                               metro_lang_str(LANG_DIALOG_RESET_TITLE));
    }
}

/* --- display: theme, accent -------------------------------------------- */

static const enum metro_lang_id accent_names[METRO_ACCENT_COUNT] = {
    LANG_ACCENT_BLUE, LANG_ACCENT_BROWN, LANG_ACCENT_GREEN, LANG_ACCENT_LIME,
    LANG_ACCENT_MAGENTA, LANG_ACCENT_MANGO, LANG_ACCENT_PINK,
    LANG_ACCENT_PURPLE, LANG_ACCENT_RED, LANG_ACCENT_TEAL,
};

static int display_count(void *ctx)
{
    (void)ctx;
    return 2;
}

static void display_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;

    out->kind = METRO_ROW_SETTING;
    if (index == 0)
    {
        out->title = metro_lang_str(LANG_SETTING_THEME);
        out->subtitle = metro_lang_str(metro_theme_get() == METRO_THEME_DARK
                                            ? LANG_VALUE_DARK : LANG_VALUE_LIGHT);
    }
    else
    {
        out->title = metro_lang_str(LANG_SETTING_ACCENT);
        out->subtitle = metro_lang_str(accent_names[metro_accent_get()]);
    }
}

static void display_on_select(void *ctx, int index)
{
    (void)ctx;

    if (index == 0)
    {
        metro_theme_set(metro_theme_get() == METRO_THEME_DARK
                             ? METRO_THEME_LIGHT : METRO_THEME_DARK);
    }
    else
    {
        int next = (metro_accent_get() + 1) % METRO_ACCENT_COUNT;
        metro_accent_set((enum metro_accent)next);
    }
}

/* --- about: static placeholder, real one lands in F8 -------------------- */

static int about_count(void *ctx)
{
    (void)ctx;
    return 1;
}

static void about_get_row(void *ctx, int index, struct metro_row *out)
{
    (void)ctx;
    (void)index;
    out->title = metro_lang_str(LANG_ABOUT_BASED_ON_ROCKBOX);
    out->subtitle = NULL;
    out->kind = METRO_ROW_ACTION;
}

static void about_on_select(void *ctx, int index)
{
    (void)ctx;
    (void)index;
}

static const struct metro_pivot pivots[] = {
    { LANG_PIVOT_GENERAL, general_count, general_get_row, general_on_select, NULL },
    { LANG_PIVOT_DISPLAY, display_count, display_get_row, display_on_select, NULL },
    { LANG_PIVOT_ABOUT,   about_count,   about_get_row,   about_on_select,   NULL },
};

static const struct metro_page settings_page = {
    LANG_HUB_SETTINGS, pivots, 3, NULL
};

const struct metro_page *metro_screen_settings_page(void)
{
    return &settings_page;
}
