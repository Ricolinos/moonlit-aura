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

#include "metro_fonts.h"

/* Glyph budget passed to font_load_ex(): 0x20-0x17F is 352 codepoints
 * (gen_fonts.sh); 400 gives headroom without forcing the glyph-cache
 * path (see font_load_ex() in firmware/font.c -- glyphs>0 sizes the
 * buffer to fit exactly that many glyphs, so as long as it's >= the
 * font's real glyph count the whole file loads at once, uncached). */
#define METRO_FONT_GLYPH_BUDGET 400

struct metro_font_spec {
    const char *filename; /* under FONT_DIR */
    const char *role_name; /* for DEBUGF only */
};

static const struct metro_font_spec font_specs[MFONT_COUNT] = {
    [MFONT_DISPLAY]  = { "metro-display-48.fnt",  "display"  },
    [MFONT_TITLE]    = { "metro-title-28.fnt",    "title"    },
    [MFONT_LIST]     = { "metro-list-20.fnt",     "list"     },
    [MFONT_LIST_SEL] = { "metro-listsel-20.fnt",  "list_sel" },
    [MFONT_CAPTION]  = { "metro-caption-14.fnt",  "caption"  },
};

static int font_ids[MFONT_COUNT];

void metro_fonts_init(void)
{
    int role;

    for (role = 0; role < MFONT_COUNT; role++)
    {
        char path[MAX_PATH];
        int id;

        snprintf(path, sizeof(path), "%s/%s",
                  FONT_DIR, font_specs[role].filename);

        id = font_load_ex(path, 0, METRO_FONT_GLYPH_BUDGET);
        if (id < 0)
        {
            DEBUGF("metro_fonts: %s (%s) failed to load, falling back"
                   " to FONT_SYSFIXED\n", font_specs[role].role_name, path);
            font_ids[role] = FONT_SYSFIXED;
        }
        else
        {
            DEBUGF("metro_fonts: %s (%s) loaded as font id %d\n",
                   font_specs[role].role_name, path, id);
            font_ids[role] = id;
        }
    }
}

int metro_font_id(enum metro_font_role role)
{
    if ((unsigned)role >= MFONT_COUNT)
        return FONT_SYSFIXED;
    return font_ids[role];
}
