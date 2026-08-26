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
#include "lcd.h" /* LCD_RGBPACK real del target -- ANTES de moonlit_tokens.h,
                   * que si no la ve ya definida cae a un fallback de 24 bits
                   * (moonlit_tokens.h) incorrecto para el panel RGB565. */
#include "moonlit_palette.h"
#include "moonlit_tokens.h"

/* [noche][preset] -> {primary, on_primary, primary_container,
 * on_primary_container}, mismo orden que enum metro_accent
 * (moonstone/tide/ember/moss, D-028) y que color.primary_presets en
 * tokens.json. */
static const unsigned primary_table[2][METRO_ACCENT_COUNT][4] = {
    {
        { MOONLIT_NIGHT_MOONSTONE_PRIMARY, MOONLIT_NIGHT_MOONSTONE_ON_PRIMARY,
          MOONLIT_NIGHT_MOONSTONE_PRIMARY_CONTAINER, MOONLIT_NIGHT_MOONSTONE_ON_PRIMARY_CONTAINER },
        { MOONLIT_NIGHT_TIDE_PRIMARY, MOONLIT_NIGHT_TIDE_ON_PRIMARY,
          MOONLIT_NIGHT_TIDE_PRIMARY_CONTAINER, MOONLIT_NIGHT_TIDE_ON_PRIMARY_CONTAINER },
        { MOONLIT_NIGHT_EMBER_PRIMARY, MOONLIT_NIGHT_EMBER_ON_PRIMARY,
          MOONLIT_NIGHT_EMBER_PRIMARY_CONTAINER, MOONLIT_NIGHT_EMBER_ON_PRIMARY_CONTAINER },
        { MOONLIT_NIGHT_MOSS_PRIMARY, MOONLIT_NIGHT_MOSS_ON_PRIMARY,
          MOONLIT_NIGHT_MOSS_PRIMARY_CONTAINER, MOONLIT_NIGHT_MOSS_ON_PRIMARY_CONTAINER },
    },
    {
        { MOONLIT_DAWN_MOONSTONE_PRIMARY, MOONLIT_DAWN_MOONSTONE_ON_PRIMARY,
          MOONLIT_DAWN_MOONSTONE_PRIMARY_CONTAINER, MOONLIT_DAWN_MOONSTONE_ON_PRIMARY_CONTAINER },
        { MOONLIT_DAWN_TIDE_PRIMARY, MOONLIT_DAWN_TIDE_ON_PRIMARY,
          MOONLIT_DAWN_TIDE_PRIMARY_CONTAINER, MOONLIT_DAWN_TIDE_ON_PRIMARY_CONTAINER },
        { MOONLIT_DAWN_EMBER_PRIMARY, MOONLIT_DAWN_EMBER_ON_PRIMARY,
          MOONLIT_DAWN_EMBER_PRIMARY_CONTAINER, MOONLIT_DAWN_EMBER_ON_PRIMARY_CONTAINER },
        { MOONLIT_DAWN_MOSS_PRIMARY, MOONLIT_DAWN_MOSS_ON_PRIMARY,
          MOONLIT_DAWN_MOSS_PRIMARY_CONTAINER, MOONLIT_DAWN_MOSS_ON_PRIMARY_CONTAINER },
    },
};

/* [noche][rol - MROLE_SURFACE] -- los 12 roles fijos por esquema,
 * mismo orden que COLOR_ROLES[4..] en generate.py. */
static const unsigned surface_table[2][12] = {
    {
        MOONLIT_NIGHT_SURFACE, MOONLIT_NIGHT_SURFACE_DIM, MOONLIT_NIGHT_SURFACE_BRIGHT,
        MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST, MOONLIT_NIGHT_SURFACE_CONTAINER_LOW,
        MOONLIT_NIGHT_SURFACE_CONTAINER, MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH,
        MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST,
        MOONLIT_NIGHT_ON_SURFACE, MOONLIT_NIGHT_ON_SURFACE_VARIANT,
        MOONLIT_NIGHT_OUTLINE, MOONLIT_NIGHT_OUTLINE_VARIANT,
    },
    {
        MOONLIT_DAWN_SURFACE, MOONLIT_DAWN_SURFACE_DIM, MOONLIT_DAWN_SURFACE_BRIGHT,
        MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST, MOONLIT_DAWN_SURFACE_CONTAINER_LOW,
        MOONLIT_DAWN_SURFACE_CONTAINER, MOONLIT_DAWN_SURFACE_CONTAINER_HIGH,
        MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST,
        MOONLIT_DAWN_ON_SURFACE, MOONLIT_DAWN_ON_SURFACE_VARIANT,
        MOONLIT_DAWN_OUTLINE, MOONLIT_DAWN_OUTLINE_VARIANT,
    },
};

/* [noche][nivel] -- base/luz/sombra de cada surface_container (D-012). */
static const unsigned surface_base[2][MSURFACE_COUNT] = {
    { MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST, MOONLIT_NIGHT_SURFACE_CONTAINER_LOW,
      MOONLIT_NIGHT_SURFACE_CONTAINER, MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH,
      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST },
    { MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST, MOONLIT_DAWN_SURFACE_CONTAINER_LOW,
      MOONLIT_DAWN_SURFACE_CONTAINER, MOONLIT_DAWN_SURFACE_CONTAINER_HIGH,
      MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST },
};
static const unsigned surface_light[2][MSURFACE_COUNT] = {
    { MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_EDGE_LIGHT, MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_EDGE_LIGHT,
      MOONLIT_NIGHT_SURFACE_CONTAINER_EDGE_LIGHT, MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_EDGE_LIGHT,
      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_EDGE_LIGHT },
    { MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_EDGE_LIGHT, MOONLIT_DAWN_SURFACE_CONTAINER_LOW_EDGE_LIGHT,
      MOONLIT_DAWN_SURFACE_CONTAINER_EDGE_LIGHT, MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_EDGE_LIGHT,
      MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_EDGE_LIGHT },
};
static const unsigned surface_shadow[2][MSURFACE_COUNT] = {
    { MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_EDGE_SHADOW, MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_EDGE_SHADOW,
      MOONLIT_NIGHT_SURFACE_CONTAINER_EDGE_SHADOW, MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_EDGE_SHADOW,
      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_EDGE_SHADOW },
    { MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_EDGE_SHADOW, MOONLIT_DAWN_SURFACE_CONTAINER_LOW_EDGE_SHADOW,
      MOONLIT_DAWN_SURFACE_CONTAINER_EDGE_SHADOW, MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_EDGE_SHADOW,
      MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_EDGE_SHADOW },
};

static int dawn_index(void)
{
    return metro_theme_get() == METRO_THEME_LIGHT ? 1 : 0;
}

static int preset_index(enum metro_accent preset)
{
    return (unsigned)preset < METRO_ACCENT_COUNT ? (int)preset : (int)METRO_ACCENT_DEFAULT;
}

unsigned moonlit_color(enum moonlit_role role)
{
    int dawn = dawn_index();

    if (role <= MROLE_ON_PRIMARY_CONTAINER)
        return primary_table[dawn][preset_index(metro_accent_get())][role];

    return surface_table[dawn][role - MROLE_SURFACE];
}

unsigned moonlit_color_accent(void)
{
    return moonlit_color(MROLE_PRIMARY);
}

unsigned moonlit_preset_primary(enum metro_accent preset)
{
    return primary_table[dawn_index()][preset_index(preset)][MROLE_PRIMARY];
}

unsigned moonlit_surface(enum moonlit_surface_level level, enum moonlit_edge edge)
{
    int dawn = dawn_index();

    if ((unsigned)level >= MSURFACE_COUNT)
        level = MSURFACE_BASE;

    switch (edge)
    {
        case MEDGE_LIGHT:  return surface_light[dawn][level];
        case MEDGE_SHADOW: return surface_shadow[dawn][level];
        default:           return surface_base[dawn][level];
    }
}
