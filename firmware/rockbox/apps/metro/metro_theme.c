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
#include "metro_theme.h"

static enum metro_theme_kind current_theme = METRO_THEME_DEFAULT;
static enum metro_accent current_accent = METRO_ACCENT_DEFAULT;

void metro_theme_init(void)
{
    current_theme = METRO_THEME_DEFAULT;
    current_accent = METRO_ACCENT_DEFAULT;
}

enum metro_theme_kind metro_theme_get(void)
{
    return current_theme;
}

void metro_theme_set(enum metro_theme_kind theme)
{
    if (theme == METRO_THEME_DARK || theme == METRO_THEME_LIGHT)
        current_theme = theme;
}

enum metro_accent metro_accent_get(void)
{
    return current_accent;
}

void metro_accent_set(enum metro_accent accent)
{
    if ((unsigned)accent < METRO_ACCENT_COUNT)
        current_accent = accent;
}

/* moonlit (D-028, M4): las 4 tinturas WP7 (bg/fg/secondary/tertiary)
 * ceden su lugar a los roles MD3 -- surface/on_surface/on_surface_variant/
 * outline (D-034, DECISIONS.md). Los nombres de funcion se conservan
 * porque metro_screen_nowplaying.c, metro_screen_lock.c y
 * metro_screen_usb.c (M5) siguen llamandolos sin cambios. */
unsigned metro_color_bg(void)
{
    return moonlit_color(MROLE_SURFACE);
}

unsigned metro_color_fg(void)
{
    return moonlit_color(MROLE_ON_SURFACE);
}

unsigned metro_color_secondary(void)
{
    return moonlit_color(MROLE_ON_SURFACE_VARIANT);
}

unsigned metro_color_tertiary(void)
{
    return moonlit_color(MROLE_OUTLINE);
}
