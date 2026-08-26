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
#include "lcd.h"

#include "metro_screen_specimen.h"
#include "metro_draw.h"
#include "moonlit_fonts.h"
#include "metro_theme.h"

/* Una fila por rol MD3, misma cadena de prueba en las 7 (mayuscula +
 * minusculas con y sin trazo ascendente/descendente, acentos y enie,
 * puntuacion invertida, raya) -- ver DECISIONS.md D-005/D-006. */
#define SPECIMEN_STRING "Hll rn \xC3\x81\xC3\x89\xC3\x91 \xC2\xBF? \xE2\x80\x94 pantalla"

/* Y de cada fila = offset acumulado (altura RB12 real del rol
 * anterior, medida con firmware/tools/check_fonts.py, + 6px de
 * separacion) para que firmware/tools/check_fonts.py --capheight vea
 * 7 bandas de tinta separadas por fondo, no una sola mancha. Alturas
 * reales (design-system/generate.py --fonts, 2026-08-25): display 42,
 * title 30, headline 23, list 22, list_sel 22, body 19, label 19. */
#define SPECIMEN_Y_DISPLAY  24
#define SPECIMEN_Y_TITLE    72
#define SPECIMEN_Y_HEADLINE 108
#define SPECIMEN_Y_LIST     137
#define SPECIMEN_Y_LISTSEL  165
#define SPECIMEN_Y_BODY     193
#define SPECIMEN_Y_LABEL    218

void metro_screen_specimen_show(void)
{
    unsigned fg = metro_color_fg();

    metro_draw_clear();
    metro_draw_header("specimen");

    metro_draw_text(MFONT_DISPLAY,  12, SPECIMEN_Y_DISPLAY,  SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_TITLE,    12, SPECIMEN_Y_TITLE,    SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_HEADLINE, 12, SPECIMEN_Y_HEADLINE, SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_LIST,     12, SPECIMEN_Y_LIST,     SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_LIST_SEL, 12, SPECIMEN_Y_LISTSEL,  SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_BODY,     12, SPECIMEN_Y_BODY,     SPECIMEN_STRING, fg);
    metro_draw_text(MFONT_LABEL,    12, SPECIMEN_Y_LABEL,    SPECIMEN_STRING, fg);

    lcd_update();
}
