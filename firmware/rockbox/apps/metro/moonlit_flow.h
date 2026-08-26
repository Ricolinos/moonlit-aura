/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* moonlit: derived from aura_flow.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-041). Eje de barrido girado de
 * columnas de pantalla (x) a filas de pantalla (y) para Marea vertical
 * (D-030) -- ver moonlit_flow.c para el detalle de cada cambio. */
/* Nucleo matematico de Music Flow real (Fase 31.1, PLAN-APPLE2026.md).
 *
 * Cosechado de `apps/plugins/pictureflow/pictureflow.c` (D-vease
 * PLAN-APPLE2026.md SS0: "el renderizador de perspectiva por columnas
 * ... el corazon del efecto, ~400 lineas portables casi sin rb->") --
 * NO es el plugin, es su aritmetica de punto fijo y su formula de
 * proyeccion por columnas, portada tal cual y adaptada a la geometria
 * fija de Aura (320x240, un solo target, a diferencia del plugin que
 * generaliza a cualquier LCD_WIDTH/HEIGHT de todo el arbol de Rockbox).
 *
 * La tecnica de reflejo por tabla que el mismo SS0 menciona ("atenuacion
 * precalculada por fila, sin gradiente en tiempo real") NO se porta de
 * nuevo aca -- Aura ya la tiene implementada con su propia convencion
 * documentada (35% de alto, doc SS5.4) en aura_art.c desde la Fase 29
 * (D-077); este modulo se enfoca en lo que todavia no existe: la
 * perspectiva.
 *
 * Modulo puro en C99, sin dependencias de Rockbox -- mismo criterio que
 * aura_nav.c/aura_motion.c/aura_wheel.c, compila igual en el host que
 * en el firmware. NO dibuja nada ni toca el framebuffer: eso es
 * deliberado (Fase 31.1 es "sin tocar hardware, sin conectar a la
 * pantalla real" -- ver DECISIONS D-079). Cuando llegue el momento de
 * blitear pixeles de verdad, ese codigo vive en un modulo de dibujo
 * separado que consume este (Marea, `05-plan-correctivo.md` SS M8).
 */
#ifndef MOONLIT_FLOW_H
#define MOONLIT_FLOW_H

/* -- Aritmetica de punto fijo (PFreal de pictureflow.c, portada tal
 * cual: PFREAL_SHIFT=10) ---------------------------------------------- */
#define MOONLIT_FLOW_SHIFT 10
#define MOONLIT_FLOW_ONE   (1 << MOONLIT_FLOW_SHIFT)
#define MOONLIT_FLOW_HALF  (MOONLIT_FLOW_ONE >> 1)

/* angulo en unidades propias: IANGLE_MAX=1024 equivale a una vuelta
 * completa (2*pi) -- mismo mapeo que pictureflow.c, para que un angulo
 * ya calculado en terminos de "eje de rotacion del slide" se pueda usar
 * tal cual. */
#define MOONLIT_FLOW_IANGLE_MAX  1024
#define MOONLIT_FLOW_IANGLE_MASK 1023

int moonlit_flow_fmul(int a, int b);
int moonlit_flow_fdiv(int num, int den);
int moonlit_flow_fsin(int iangle);
int moonlit_flow_fcos(int iangle);

/* -- Geometria de camara fija para 320x240 EN VERTICAL (D-030, D-041):
 * el eje de barrido de aura_flow.c era horizontal (columnas de
 * pantalla, ancho 320); Marea barre filas de pantalla sobre el alto
 * util de la pantalla (240 menos la barra de estado de 20 px). Las
 * macros de camara de pictureflow.c se re-derivan sobre ese eje. ---- */
#define MOONLIT_FLOW_AXIS_LEN      220  /* alto util = 240 - barra de estado 20 (D-030) */
#define MOONLIT_FLOW_SCREEN_H      240  /* alto real de pantalla; no lo usa el motor (informativo) */
#define MOONLIT_FLOW_DISPLAY_LEN   120  /* alto de la "franja" de flujo -- tapa central 120 px (D-030) */
#define MOONLIT_FLOW_CAM_DIST      240  /* HIPOTESIS (D-041): retunear en M8 contra hardware real */
#define MOONLIT_FLOW_CAM_DIST_R    (MOONLIT_FLOW_CAM_DIST << MOONLIT_FLOW_SHIFT)
#define MOONLIT_FLOW_DISPLAY_TOP_R  (MOONLIT_FLOW_HALF - MOONLIT_FLOW_AXIS_LEN * MOONLIT_FLOW_HALF)
#define MOONLIT_FLOW_MAXSLIDE_TOP_R (MOONLIT_FLOW_HALF - MOONLIT_FLOW_DISPLAY_LEN * MOONLIT_FLOW_HALF)

/* -- Proyeccion por filas -------------------------------------------- */

/* Un slide a proyectar: angulo (unidades IANGLE, 0 = de frente a la
 * camara), distancia adicional de camara (0 = posicion de reposo -- SIN
 * escalar por MOONLIT_FLOW_ONE, es un entero simple igual que
 * slide_data.distance en pictureflow.c; moonlit_flow_begin_projection() lo
 * escala internamente) y posicion vertical de su centro en punto fijo
 * (coordenadas de pantalla, PFreal). */
typedef struct {
    int angle;
    int distance;
    int cx;
} moonlit_flow_slide_t;

/* Estado de la recurrencia de proyeccion (formula de pictureflow.c
 * render_slide(), extraida del blit de pixeles): en vez de recalcular
 * la formula de perspectiva completa por fila, xsnum/xsden avanzan
 * con un incremento constante (xsnumi/xsdeni) y una sola division por
 * fila recupera xs -- es una transformacion de Mobius de la fila
 * de pantalla, la misma optimizacion que ya paga su costo en hardware
 * real sin FPU. */
typedef struct {
    int slide_width_px;
    int slide_left;    /* PFreal: borde superior de la fuente */
    int cosr, sinr;
    int zo;             /* PFreal: distancia efectiva de camara para este slide */
    int xsnum, xsden;
    int xsnumi, xsdeni;
    int xs;              /* PFreal: posicion fuente de la fila actual */
    int screen_y;         /* fila de pantalla actual */
    int has_rotation;      /* angle!=0 || zo!=0 -- decide recurrencia vs paso fijo */
} moonlit_flow_projection_t;

/* Inicializa `proj` en la primera fila de pantalla donde `slide` es
 * visible (proj->screen_y). Si el slide no es visible en absoluto
 * (fuera de rango), proj->screen_y >= MOONLIT_FLOW_AXIS_LEN. */
void moonlit_flow_begin_projection(moonlit_flow_projection_t *proj,
                                 const moonlit_flow_slide_t *slide,
                                 int slide_width_px);
/* proj->screen_y siempre queda en [0, MOONLIT_FLOW_AXIS_LEN). */

/* Avanza a la siguiente fila de pantalla. Devuelve 0 (y no modifica
 * `proj`) si ya no queda fuente visible (la fila fuente actual llego
 * al borde del slide) o si se salio de pantalla -- el llamador corta el
 * bucle de dibujo ahi. */
int moonlit_flow_advance_column(moonlit_flow_projection_t *proj);

/* Fila de la imagen fuente (0..slide_width-1) que corresponde a la
 * posicion actual de la proyeccion. */
int moonlit_flow_source_row(const moonlit_flow_projection_t *proj);

/* Escala horizontal (eje cruzado) en punto fijo para la fila actual:
 * cuantos "pixeles fuente" avanzar por cada pixel horizontal de
 * pantalla al muestrear esa fila (mas alla de MOONLIT_FLOW_ONE = la
 * fila se ve mas chica que su tamano fuente, perspectiva alejandose). */
int moonlit_flow_cross_scale(const moonlit_flow_projection_t *proj);

#endif /* MOONLIT_FLOW_H */
