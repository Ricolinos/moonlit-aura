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
/* moonlit (D-074): dibujo por tramos -- la fuente de puntuación se
 * dibuja con OTRO archivo .fnt que el texto normal, así que un mismo
 * `lcd_putsxy()` no puede pintar los dos: hay que partir la cadena en
 * tramos, uno por tipo de fuente, y dibujarlos uno tras otro avanzando
 * la x por el ancho medido de cada uno.
 *
 * Módulo puro (sin una sola dependencia de Rockbox) para que el arnés
 * de host lo enlace solo -- mismo patrón que moonlit_marea_prefetch.c
 * y moonlit_marquee_cycle.c. Todo lo que se puede equivocar sin que se
 * note vive aquí: qué codepoint cae en qué tramo, que los tramos
 * contiguos del mismo tipo se fundan en uno solo (menos llamadas a
 * dibujar), y que nunca se corte a mitad de una secuencia UTF-8 ni se
 * desborde el buffer del llamador. */
#ifndef MOONLIT_TEXTSEG_H
#define MOONLIT_TEXTSEG_H

#include <stdbool.h>
#include <stddef.h>

enum moonlit_textseg_kind {
    MOONLIT_TEXTSEG_PRIMARY = 0, /* rol normal -- ASCII/Latin-1 tal cual,
                                  * o ya transliterado (D-066) */
    MOONLIT_TEXTSEG_PUNCT,       /* fuente de puntuacion del rol (D-074) */
    MOONLIT_TEXTSEG_CYRILLIC,    /* fuente cirilica del rol (D-081, ruso) */
};

/* Un tramo, ya escrito y NUL-terminado dentro del buffer de salida del
 * llamador -- `text` apunta ahi, listo para pasar tal cual a
 * lcd_putsxy(). Valido solo mientras ese buffer no se reutilice. */
struct moonlit_textseg {
    enum moonlit_textseg_kind kind;
    const char *text;
};

/* moonlit (D-081): rango denso de la fuente cirilica (alfabeto ruso
 * completo, ver design-system/generate.py) -- Ё(0x401), А-я(0x410-0x44F),
 * ё(0x451). Publico para que el test host pueda recorrerlo sin
 * duplicar los limites. */
#define MOONLIT_TEXTSEG_CYRILLIC_START 1025
#define MOONLIT_TEXTSEG_CYRILLIC_LIMIT 1105

/* Parte `in` en como mucho `max_segs` tramos dentro de `out_buf`
 * (`out_buf_sz` bytes). Devuelve cuantos escribio -- 0 si `in` es NULL
 * o vacio, o si no cupo ni un byte.
 *
 * `has_punct_font` en false (el rol no tiene fuente de puntuacion --
 * hoy solo MFONT_DISPLAY) hace que el resultado sea SIEMPRE un unico
 * tramo PRIMARY con `in` transliterado entero (moonlit_translit.h,
 * D-066) -- el comportamiento de antes de D-074, sin excepciones.
 * `has_cyrillic_font` sigue el mismo criterio para el rango cirilico
 * (D-081) -- hoy los siete roles lo tienen, a diferencia de punct.
 *
 * Con `has_punct_font` en true, cada codepoint se clasifica:
 *   - 32-383 (rango primario, D-007): tramo PRIMARY, bytes tal cual.
 *   - 1025-1105 (D-081) SI `has_cyrillic_font`: tramo CYRILLIC, bytes
 *     tal cual (sin transliterar -- no hay ASCII razonable para una
 *     letra rusa).
 *   - en moonlit_punct_table_has() (D-074, interseccion de los seis
 *     roles): tramo PUNCT, bytes tal cual (SIN transliterar -- son
 *     justo los que la fuente de puntuacion dibuja de verdad).
 *   - cualquier otro: tramo PRIMARY, con el reemplazo de
 *     moonlit_translit_lookup() si existe, o los bytes originales tal
 *     cual si no (para que el defaultchar del rol primario, '·' desde
 *     D-066, resuelva lo que ni transliteracion ni fuente de
 *     puntuacion cubren).
 *
 * Tramos contiguos del mismo tipo se funden en uno: nunca hay dos
 * tramos PRIMARY seguidos en el resultado. Si `in` tiene mas
 * alternancias de las que caben en `max_segs`, o mas bytes de los que
 * caben en `out_buf_sz`, el resto se descarta -- mismo criterio de
 * truncar sin desbordar que ya sigue moonlit_translit(). */
int moonlit_textseg_build(const char *in, bool has_punct_font,
                          bool has_cyrillic_font,
                          char *out_buf, size_t out_buf_sz,
                          struct moonlit_textseg *segs, int max_segs);

#endif /* MOONLIT_TEXTSEG_H */
