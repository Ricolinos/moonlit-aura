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
/* moonlit (D-008, D-033): iconografia de Material Symbols Rounded
 * (Google, Apache 2.0) como mascaras de cobertura de 8 bits (una por
 * icono x tamano), generadas offline por design-system/generate.py
 * --icons y COMMITEADAS en moonlit_icons_table.c -- mismo patron que
 * la tabla de glifos grandes de M-089 (retirada en M5, D-040): cero
 * lecturas de disco en tiempo de ejecucion, verificacion mecanica de
 * tonos >= 4 en generacion, nunca "a ojo".
 *
 * Sustituye a la tabla anterior de mascaras monocromas de 1 bit: el
 * antialiasing real de 8 bits evita los bordes en escalera que un
 * icono binario deja en un LCD de 320x240.
 *
 * El orden del enum es el mismo que 'icon.names' en tokens.json. Si se
 * agrega un icono, va AL FINAL de ambos.
 */
#ifndef MOONLIT_ICONS_H
#define MOONLIT_ICONS_H

#include <stdint.h>

/* Los tres tamanos generados (tokens.json:icon.sizes) -- coinciden con
 * MOONLIT_ICON_SIZE_COUNT columnas de la tabla moonlit_icons[][]. */
#define MOONLIT_ICON_SIZE_16 16
#define MOONLIT_ICON_SIZE_24 24
#define MOONLIT_ICON_SIZE_40 40
#define MOONLIT_ICON_SIZE_COUNT 3

enum moonlit_icon_id {
    MOONLIT_ICON_PLAY_ARROW = 0,
    MOONLIT_ICON_PAUSE,
    MOONLIT_ICON_SKIP_NEXT,
    MOONLIT_ICON_SKIP_PREVIOUS,
    MOONLIT_ICON_REPEAT,
    MOONLIT_ICON_SHUFFLE,
    MOONLIT_ICON_VOLUME_UP,
    MOONLIT_ICON_FAVORITE,
    MOONLIT_ICON_SYNC,
    /* Reservados para M4 (ajustes/hub), M5 (candado/USB) y M8 (Marea):
     * vendoreados y compilados desde ya (D-033), sin consumidor todavia. */
    MOONLIT_ICON_ALBUM,
    MOONLIT_ICON_PERSON,
    MOONLIT_ICON_QUEUE_MUSIC,
    MOONLIT_ICON_PHOTO,
    MOONLIT_ICON_MOVIE,
    MOONLIT_ICON_SETTINGS,
    MOONLIT_ICON_INFO,
    MOONLIT_ICON_LOCK,
    MOONLIT_ICON_BATTERY_FULL,
    MOONLIT_ICON_BATTERY_CHARGING_FULL,
    MOONLIT_ICON_USB,
    MOONLIT_ICON_COUNT
};

/* width*height bytes, row-major, 0..255 (cobertura, no color). */
struct moonlit_icon_mask {
    int width;
    int height;
    /* moonlit (D-068, maestro SS H): caja de TINTA dentro de la celda --
     * primera fila con cobertura y cuantas filas ocupa. Un Material
     * Symbol de 16 px dibuja ~12 px de tinta centrados en su celda, asi
     * que alinear la CELDA con el texto y la bateria deja el simbolo
     * desplazado. La miden en generacion (design-system/generate.py
     * --icons), nunca a ojo. */
    int ink_top;
    int ink_h;
    const uint8_t *cov;
};

/* [icono][indice de tamano] -- indice = MOONLIT_ICON_SIZE_16/24/40 no
 * se usan como indice directo, ver moonlit_icon_draw(). */
extern const struct moonlit_icon_mask moonlit_icons[MOONLIT_ICON_COUNT][MOONLIT_ICON_SIZE_COUNT];

/* Dibuja el icono `id` a `size_px` (16, 24 o 40 -- ver arriba) con la
 * esquina superior izquierda en (x, y), mezclando `color` sobre lo que
 * ya haya en el framebuffer (metro_fb_plot_alpha). `size_px` fuera de
 * las tres tallas generadas no dibuja nada. */
void moonlit_icon_draw(enum moonlit_icon_id id, int size_px, int x, int y, unsigned color);

#endif /* MOONLIT_ICONS_H */
