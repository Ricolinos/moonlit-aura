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
/* moonlit (D-066): transliteracion de la puntuacion tipografica que las
 * fuentes de moonlit no traen.
 *
 * Por que no basta con ampliar el rango de convttf: el formato RB12 es
 * un rango DENSO -- `size` entradas de offset + ancho desde `firstchar`,
 * exista o no el glifo. Llevar el rango de 32-383 a 32-8482 (para que
 * entren las comillas tipograficas y companiia) pagaria 8 071 entradas
 * vacias por fuente. Medido, no estimado: +1 010 578 B en disco y
 * +286 998 B de tablas en RAM con los 7 roles cargados, contra un
 * presupuesto de 40 KB. Ver DECISIONS.md D-066.
 *
 * Lo que SI resuelve el problema real: casi toda la puntuacion que
 * aparece de verdad en metadatos de musica tiene un equivalente ASCII
 * que ya esta en el rango. `Don't Stop Believin'` con apostrofo
 * tipografico se dibuja con apostrofo recto en vez de con el caracter
 * de reemplazo, que es lo que el dueno ve como defecto.
 *
 * Lo que NO tiene equivalente (corcheas, estrellas, corazones, CJK,
 * emoji) cae en el `defaultchar` de la fuente, que D-066 cambia de '?'
 * (63) a '·' (183, U+00B7): un punto medio no parece un error de
 * lectura, un signo de interrogacion si.
 *
 * ESTA TABLA ES LA FUENTE UNICA. firmware/tools/check_fonts.py la lee
 * de este header (--coverage) para no reportar como faltante algo que
 * en realidad se translitera. Si cambias el formato de las lineas de
 * la tabla, actualiza ese lector.
 *
 * Modulo puro: sin dependencias de Rockbox, host-testable
 * (apps/metro/test/test_translit.c). */
#ifndef MOONLIT_TRANSLIT_H
#define MOONLIT_TRANSLIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Una entrada: codepoint fuera del rango 32-383 -> su equivalente ASCII
 * (1 a 3 bytes). El lector de check_fonts.py espera exactamente este
 * formato: `{ 0xNNNN, "..." },` una por linea. */
struct moonlit_translit_entry {
    uint32_t cp;
    const char *ascii;
};

extern const struct moonlit_translit_entry moonlit_translit_table[];
extern const int moonlit_translit_count;

/* moonlit (D-074): el reemplazo de UN codepoint, o NULL si la tabla no
 * lo tiene. Es el `lookup()` interno de moonlit_translit(), expuesto
 * para que moonlit_textseg.c decida caracter por caracter -- ahi es
 * donde hace falta separar "esto tiene fuente de puntuacion propia" de
 * "esto solo tiene equivalente ASCII", y moonlit_translit() por si sola
 * solo sabe transliterar la cadena entera. */
const char *moonlit_translit_lookup(uint32_t cp);

/* true si `s` contiene algun byte que PODRIA iniciar una secuencia
 * transliterable (0xC2 para U+00A0, 0xE2 para el bloque de puntuacion
 * general). Barato: un recorrido de bytes sin decodificar. El camino
 * comun -- texto que ya es ASCII o Latin-1 acentuado -- sale por aqui
 * sin copiar nada. */
bool moonlit_translit_needed(const char *s);

/* Copia `in` a `out` reemplazando cada codepoint de la tabla por su
 * equivalente. Cualquier otra secuencia se copia tal cual, byte a byte,
 * incluidas las secuencias UTF-8 mal formadas (no es trabajo de esta
 * funcion sanearlas). Nunca escribe mas de `outsz` bytes contando el
 * '\0', y nunca corta a mitad de una secuencia UTF-8. Devuelve `out`. */
const char *moonlit_translit(const char *in, char *out, size_t outsz);

#endif /* MOONLIT_TRANSLIT_H */
