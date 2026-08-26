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
#ifndef MOONLIT_PALETTE_H
#define MOONLIT_PALETTE_H

#include "metro_theme.h"

/* moonlit (D-028, M4): unico includer de moonlit_tokens.h -- todo lo
 * demas en apps/metro/ pide un color por ROL, nunca por hex (regla del
 * CLAUDE.md). Resuelve segun metro_theme_get() (DARK=night, LIGHT=dawn,
 * D-027) y metro_accent_get() (el preset activo, D-028) sin que el
 * llamador tenga que saber cual esquema/preset esta activo. */

/* Los 16 roles MD3 de D-028, mismo orden que COLOR_ROLES en
 * design-system/generate.py y color.night/dawn en tokens.json. Los
 * primeros 4 (la familia "primary") varian por preset de acento
 * (moonstone/tide/ember/moss, enum metro_accent); los otros 12 son
 * fijos por esquema, iguales en cualquier preset. */
enum moonlit_role {
    MROLE_PRIMARY = 0,
    MROLE_ON_PRIMARY,
    MROLE_PRIMARY_CONTAINER,
    MROLE_ON_PRIMARY_CONTAINER,
    MROLE_SURFACE,
    MROLE_SURFACE_DIM,
    MROLE_SURFACE_BRIGHT,
    MROLE_SURFACE_CONTAINER_LOWEST,
    MROLE_SURFACE_CONTAINER_LOW,
    MROLE_SURFACE_CONTAINER,
    MROLE_SURFACE_CONTAINER_HIGH,
    MROLE_SURFACE_CONTAINER_HIGHEST,
    MROLE_ON_SURFACE,
    MROLE_ON_SURFACE_VARIANT,
    MROLE_OUTLINE,
    MROLE_OUTLINE_VARIANT,
    MROLE_COUNT
};

/* Los 5 niveles de elevacion tonal (D-012) -- surface_container_<nivel>
 * de D-028, del mas hundido al mas alto. moonlit_draw_surface()
 * (moonlit_elevation.h) los usa para dibujar tarjetas; moonlit_surface()
 * de abajo para cuando solo hace falta el color, sin la tarjeta entera. */
enum moonlit_surface_level {
    MSURFACE_LOWEST = 0,
    MSURFACE_LOW,
    MSURFACE_BASE,
    MSURFACE_HIGH,
    MSURFACE_HIGHEST,
    MSURFACE_COUNT
};

/* Borde de una superficie elevada (D-012): NONE = el tono base del
 * nivel: LIGHT = borde superior/izquierdo (luz), SHADOW = borde
 * inferior/derecho (sombra) -- luz desde arriba-izquierda, calma
 * nocturna, nunca blur ni ripple. */
enum moonlit_edge {
    MEDGE_NONE = 0,
    MEDGE_LIGHT,
    MEDGE_SHADOW
};

/* Color resuelto para un rol MD3, segun el esquema activo (noche/dawn)
 * y -- solo para la familia primary -- el preset de acento activo. */
unsigned moonlit_color(enum moonlit_role role);

/* == moonlit_color(MROLE_PRIMARY). */
unsigned moonlit_color_accent(void);

/* moonlit (D-028): metro_color_accent() queda como alias de
 * compatibilidad hacia moonlit_color_accent() -- mismo patron de
 * "#define hacia el rol nuevo" que moonlit_fonts.h uso en M2 para
 * migrar sus 19 sitios -- hasta que M11 lo retire. */
#define metro_color_accent moonlit_color_accent

/* El swatch "primary" de un preset arbitrario, sin importar cual esta
 * activo -- p.ej. para una fila de Ajustes que lista los 4 presets.
 * Fuera de rango cae al preset por defecto (METRO_ACCENT_DEFAULT). */
unsigned moonlit_preset_primary(enum metro_accent preset);

/* Color de un nivel de superficie (D-012), sin dibujar nada -- para
 * cuando solo hace falta el tono (p.ej. lcd_set_background()) y no la
 * tarjeta completa de moonlit_draw_surface(). */
unsigned moonlit_surface(enum moonlit_surface_level level, enum moonlit_edge edge);

#endif /* MOONLIT_PALETTE_H */
