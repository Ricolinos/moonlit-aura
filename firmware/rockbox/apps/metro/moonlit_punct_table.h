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
/* moonlit (D-074): tabla de codepoints con fuente de puntuación propia.
 *
 * `moonlit_punct_table.c` es GENERADO por `design-system/generate.py
 * --fonts` -- este header, la API estable sobre esa tabla, se escribe
 * a mano una sola vez. La tabla es la INTERSECCIÓN de los seis roles
 * con fuente de puntuación (todos salvo `MFONT_DISPLAY`): un codepoint
 * solo entra si los seis lo dibujan con un glifo real, para que un
 * mismo texto nunca mezcle la fuente de puntuación en un rol y la
 * transliteración ASCII en otro (ver DECISIONS.md D-074, el caso
 * concreto de "Ahora suena" que descartó la variante de dos roles en
 * el addendum de D-066).
 *
 * Módulo puro: sin dependencias de Rockbox, host-testable junto con
 * `moonlit_textseg.c`, que es quien la consulta. */
#ifndef MOONLIT_PUNCT_TABLE_H
#define MOONLIT_PUNCT_TABLE_H

#include <stdbool.h>
#include <stdint.h>

extern const uint32_t moonlit_punct_codepoints[];
extern const int moonlit_punct_codepoint_count;

/* true si `cp` tiene fuente de puntuación propia en los seis roles.
 * Recorrido lineal a propósito: la tabla mide un puñado de entradas
 * (≤ 23, el tamaño de moonlit_translit_table dentro del rango
 * 8208-8482), buscar no vale la pena frente a leer el código. */
static inline bool moonlit_punct_table_has(uint32_t cp)
{
    int i;

    for (i = 0; i < moonlit_punct_codepoint_count; i++)
        if (moonlit_punct_codepoints[i] == cp)
            return true;
    return false;
}

#endif /* MOONLIT_PUNCT_TABLE_H */
