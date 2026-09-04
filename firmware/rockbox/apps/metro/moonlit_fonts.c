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
#include <stdio.h>
#include "font.h"
#include "rbpaths.h"
#include "fs_defines.h" /* MAX_PATH */
#include "debug.h"

#include "moonlit_fonts.h"

/* Glyph budget passed to font_load_ex(): el rango decimal 32-383 de
 * convttf (D-007, design-system/generate.py --fonts) es 352 codepoints
 * para los roles Montserrat y 351 para los roles Libre Baskerville
 * (D-032: sin U+017F). 400 da margen sin forzar el glyph-cache (ver
 * font_load_ex() en firmware/font.c -- glyphs>0 dimensiona el buffer
 * para calzar exactamente esa cantidad de glifos, asi que mientras sea
 * >= el conteo real del font, el archivo entero carga de una, sin
 * cache). */
#define METRO_FONT_GLYPH_BUDGET 400

/* moonlit (D-074): las fuentes de puntuacion miden ~272-275 glifos
 * reales (rango denso 8208-8482, ver design-system/generate.py); 320
 * da el mismo tipo de margen que los 400 de arriba dan sobre 351-352. */
#define MOONLIT_PUNCT_GLYPH_BUDGET 320

/* moonlit (D-081): las fuentes cirilicas miden 81 glifos reales exactos
 * (rango denso 1025-1105, alfabeto ruso completo, ver
 * design-system/generate.py) -- 96 da el mismo tipo de margen. */
#define MOONLIT_CYRILLIC_GLYPH_BUDGET 96

struct metro_font_spec {
    const char *filename; /* under FONT_DIR */
    const char *role_name; /* for DEBUGF only */
    const char *punct_filename; /* moonlit (D-074): NULL si el rol no tiene */
    const char *cyrillic_filename; /* moonlit (D-081): los siete roles la tienen */
};

static const struct metro_font_spec font_specs[MFONT_COUNT] = {
    [MFONT_DISPLAY]  = { "moonlit-display-40.fnt",  "display",  NULL,
                          "moonlit-display-40-cyr.fnt" },
    [MFONT_TITLE]    = { "moonlit-title-28.fnt",    "title",    "moonlit-title-28-punct.fnt",
                          "moonlit-title-28-cyr.fnt" },
    [MFONT_HEADLINE] = { "moonlit-headline-22.fnt", "headline", "moonlit-headline-22-punct.fnt",
                          "moonlit-headline-22-cyr.fnt" },
    [MFONT_LIST]     = { "moonlit-list-20.fnt",     "list",     "moonlit-list-20-punct.fnt",
                          "moonlit-list-20-cyr.fnt" },
    [MFONT_LIST_SEL] = { "moonlit-listsel-20.fnt",  "list_sel", "moonlit-listsel-20-punct.fnt",
                          "moonlit-listsel-20-cyr.fnt" },
    [MFONT_BODY]     = { "moonlit-body-18.fnt",     "body",     "moonlit-body-18-punct.fnt",
                          "moonlit-body-18-cyr.fnt" },
    [MFONT_LABEL]    = { "moonlit-label-18.fnt",    "label",    "moonlit-label-18-punct.fnt",
                          "moonlit-label-18-cyr.fnt" },
};

static int font_ids[MFONT_COUNT];
static int punct_font_ids[MFONT_COUNT]; /* moonlit (D-074) */
static int cyrillic_font_ids[MFONT_COUNT]; /* moonlit (D-081) */

static int load_one(const char *filename, const char *role_name,
                    const char *what, int glyph_budget)
{
    char path[MAX_PATH];
    int id;

    /* role_name/what solo se usan dentro de DEBUGF() -- que en un build
     * sin logf se compila a nada, dejando los dos parametros sin usar
     * (-Wunused-parameter, -Wextra). */
    (void)role_name;
    (void)what;

    snprintf(path, sizeof(path), "%s/%s", FONT_DIR, filename);
    id = font_load_ex(path, 0, glyph_budget);
    if (id < 0)
    {
        DEBUGF("moonlit_fonts: %s %s (%s) failed to load\n",
               role_name, what, path);
        return -1;
    }
    DEBUGF("moonlit_fonts: %s %s (%s) loaded as font id %d\n",
           role_name, what, path, id);
    return id;
}

void metro_fonts_init(void)
{
    int role;

    for (role = 0; role < MFONT_COUNT; role++)
    {
        int id = load_one(font_specs[role].filename, font_specs[role].role_name,
                          "primary", METRO_FONT_GLYPH_BUDGET);

        font_ids[role] = (id >= 0) ? id : FONT_SYSFIXED;

        /* moonlit (D-074): sin fuente propia de puntuacion (MFONT_DISPLAY)
         * o si la carga falla, el tramo PUNCT cae al mismo id que el
         * primario -- metro_draw.c lo trata como "sin diferencia" en vez
         * de arriesgar un id invalido. */
        punct_font_ids[role] = font_ids[role];
        if (font_specs[role].punct_filename)
        {
            int punct_id = load_one(font_specs[role].punct_filename,
                                    font_specs[role].role_name, "punct",
                                    MOONLIT_PUNCT_GLYPH_BUDGET);
            if (punct_id >= 0)
                punct_font_ids[role] = punct_id;
        }

        /* moonlit (D-081): sin fuente cirilica propia, o si la carga
         * falla, el tramo CYRILLIC cae al id primario -- mismo criterio
         * "sin diferencia" que D-074 usa para PUNCT. Los siete roles SI
         * tienen archivo (a diferencia de punct, que excluye display). */
        cyrillic_font_ids[role] = font_ids[role];
        if (font_specs[role].cyrillic_filename)
        {
            int cyr_id = load_one(font_specs[role].cyrillic_filename,
                                  font_specs[role].role_name, "cyrillic",
                                  MOONLIT_CYRILLIC_GLYPH_BUDGET);
            if (cyr_id >= 0)
                cyrillic_font_ids[role] = cyr_id;
        }
    }
}

int metro_font_id(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return FONT_SYSFIXED;
    return font_ids[role];
}

bool metro_font_has_punct(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return false;
    return font_specs[role].punct_filename != NULL;
}

int metro_font_punct_id(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return FONT_SYSFIXED;
    return punct_font_ids[role];
}

bool metro_font_has_cyrillic(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return false;
    return font_specs[role].cyrillic_filename != NULL;
}

int metro_font_cyrillic_id(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return FONT_SYSFIXED;
    return cyrillic_font_ids[role];
}
