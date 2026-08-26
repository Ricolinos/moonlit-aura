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
#ifndef METRO_THEME_H
#define METRO_THEME_H

/* Active theme (dark/light) + accent state -- see DECISIONS.md M-012.
 * F2: RAM-only, always starts at the defaults. Persisting the user's
 * choice to aura.cfg is metro_settings.c's job (F6/F8); when that
 * lands it will call metro_theme_set()/metro_accent_set() once at
 * boot after reading the saved values, nothing here changes.
 *
 * moonlit (D-027, D-028, M4): METRO_THEME_DARK/LIGHT keep their names
 * (metro_settings.c:83 persists this exact enum) but now mean the
 * MD3 "night" (predeterminado) and "dawn" schemes; the role->color
 * resolver moved to moonlit_palette.h/.c, the only includer of
 * moonlit_tokens.h -- this header only owns the STATE (which scheme,
 * which accent preset is active), never a literal RGB. */

enum metro_theme_kind {
    METRO_THEME_DARK = 0,
    METRO_THEME_LIGHT
};

/* moonlit (D-028): reemplaza los 10 acentos WP7 -- 4 presets de
 * `primary` MD3 (design-system/tokens.json:color.primary_presets).
 * Mismo campo `accent` de aura.cfg, mismo nombre de tipo (metro_settings.h
 * no cambia); solo los VALORES son nuevos. */
enum metro_accent {
    METRO_ACCENT_MOONSTONE = 0,
    METRO_ACCENT_TIDE,
    METRO_ACCENT_EMBER,
    METRO_ACCENT_MOSS,
    METRO_ACCENT_COUNT
};

#define METRO_ACCENT_DEFAULT METRO_ACCENT_MOONSTONE
#define METRO_THEME_DEFAULT  METRO_THEME_DARK

void metro_theme_init(void);

enum metro_theme_kind metro_theme_get(void);
void metro_theme_set(enum metro_theme_kind theme);

enum metro_accent metro_accent_get(void);
void metro_accent_set(enum metro_accent accent);

/* moonlit_palette.h: moonlit_color()/moonlit_surface() (los 16 roles
 * MD3 y los niveles de elevacion, D-028/D-012) y moonlit_color_accent()
 * -- metro_color_accent() de abajo es su alias de compatibilidad. */
#include "moonlit_palette.h"

/* Resolved to the active scheme -- pass straight to
 * lcd_set_foreground()/lcd_set_background() or LCD_RGBPACK-style
 * drawing calls. moonlit (M4): ya no leen metro_palette.h, resuelven
 * via moonlit_color() (moonlit_palette.c). */
unsigned metro_color_bg(void);
unsigned metro_color_fg(void);
unsigned metro_color_secondary(void);
unsigned metro_color_tertiary(void);

#endif /* METRO_THEME_H */
