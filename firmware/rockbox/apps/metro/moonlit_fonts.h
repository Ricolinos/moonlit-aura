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
#ifndef MOONLIT_FONTS_H
#define MOONLIT_FONTS_H

/* Seven MD3 type roles (D-005, D-028): Libre Baskerville for
 * display/title/headline, Montserrat for list/list_sel/body/label
 * (design-system/tokens.json:type_scale). Loaded once, fully
 * (font_load_ex(path, 0, N), same rationale as metro_fonts.c M-010:
 * no glyph-cache path needed on this target). Falls back to
 * FONT_SYSFIXED for any role whose .fnt is missing or fails to load. */
enum metro_font_role {
    MFONT_DISPLAY = 0, /* Libre Baskerville 40px -- hub rows, giant titles */
    MFONT_TITLE,        /* Libre Baskerville 28px -- Now Playing title, pivot headers */
    MFONT_HEADLINE,      /* Libre Baskerville 22px -- Marea panel title (M8) */
    MFONT_LIST,         /* Montserrat Regular 20px -- unselected list rows */
    MFONT_LIST_SEL,     /* Montserrat SemiBold 20px -- selected list row */
    MFONT_BODY,         /* Montserrat Regular 18px -- body text */
    MFONT_LABEL,        /* Montserrat Medium 18px -- header, subtitles, values */
    MFONT_COUNT
};

/* Compat temporal (M2): los 19 sitios que hoy dicen MFONT_CAPTION
 * (header/subtitles/values, antes Regular 14px de la familia previa)
 * compilan sin tocarlos -- el rol MD3 mas cercano es MFONT_BODY (18px,
 * ningun rol < 18px per D-005). Se retira en M4 cuando esos sitios se
 * re-anotan con su rol MD3 real (label en la mayoria de los casos). */
#define MFONT_CAPTION MFONT_BODY

/* Loads all 7 roles from FONT_DIR ("/.rockbox/fonts/moonlit-*.fnt").
 * Safe to call more than once. Call once from metro_main() before the
 * first screen draws. */
void metro_fonts_init(void);

/* Rockbox font id for a role, ready to pass to lcd_setfont()/
 * font_getstringsize(). Always valid -- returns FONT_SYSFIXED if the
 * role's .fnt never loaded. */
int metro_font_id(enum metro_font_role role);

#endif /* MOONLIT_FONTS_H */
