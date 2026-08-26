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
/* moonlit (D-016, D-044): logotipo Waning Crescent -- creciente por
 * sustraccion de dos circulos y wordmark "moonlit" (Libre Baskerville a
 * contornos, OFL 1.1, D-004), como mascaras de cobertura de 8 bits
 * generadas offline por design-system/generate.py --logo y COMMITEADAS
 * en moonlit_logo_table.c -- mismo patron que moonlit_icons.h (D-033):
 * cero lecturas de disco en tiempo de ejecucion, verificacion mecanica
 * de tonos/cobertura/cuspides en generacion, nunca "a ojo".
 *
 * El wordmark solo se dibuja junto al creciente de 64px (arranque,
 * "Acerca de"); 16/24/40px son unicamente la geometria del creciente
 * (D-016).
 */
#ifndef MOONLIT_LOGO_H
#define MOONLIT_LOGO_H

#include <stdint.h>

#define MOONLIT_LOGO_CRESCENT_SIZE_16 16
#define MOONLIT_LOGO_CRESCENT_SIZE_24 24
#define MOONLIT_LOGO_CRESCENT_SIZE_40 40
#define MOONLIT_LOGO_CRESCENT_SIZE_64 64
#define MOONLIT_LOGO_CRESCENT_SIZE_COUNT 4

#define MOONLIT_LOGO_WORDMARK_WIDTH  140
#define MOONLIT_LOGO_WORDMARK_HEIGHT 28

/* width*height bytes, row-major, 0..255 (cobertura, no color). */
struct moonlit_logo_mask {
    int width;
    int height;
    const uint8_t *cov;
};

/* Indexado en el mismo orden que logo.crescent_sizes en tokens.json --
 * ver moonlit_logo_draw_crescent() para el mapeo tamano -> indice. */
extern const struct moonlit_logo_mask moonlit_logo_crescent[MOONLIT_LOGO_CRESCENT_SIZE_COUNT];
extern const struct moonlit_logo_mask moonlit_logo_wordmark;

/* Dibuja el creciente a `size_px` (16, 24, 40 o 64) con la esquina
 * superior izquierda en (x, y), mezclando `color` sobre lo que ya haya
 * en el framebuffer (metro_fb_plot_alpha). `size_px` fuera de los
 * cuatro tamanos generados no dibuja nada. */
void moonlit_logo_draw_crescent(int size_px, int x, int y, unsigned color);

/* Dibuja el wordmark "moonlit" (140x28) con la esquina superior
 * izquierda en (x, y). Solo se usa junto al creciente de 64px (D-016). */
void moonlit_logo_draw_wordmark(int x, int y, unsigned color);

#endif /* MOONLIT_LOGO_H */
