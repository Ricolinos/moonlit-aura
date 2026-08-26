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
/* moonlit (D-029, D-030, M8): pantalla Marea -- ver moonlit_screen_marea.h.
 *
 * Geometria de dibujo derivada de aura_musicflow.c (leido como
 * referencia via `git show aura-upstream/main:...`, aura-upstream
 * 7ec39edbf7cbe8547afa55880336ecdf2f890104 -- ver DECISIONS.md D-041/
 * D-0xx): draw_slide_perspective() (~605-713), get_slot_for()
 * (~448-494), scroll_step()/aura_musicflow_handle_button() (~1238-1330).
 * Deliberadamente MAS CHICO que el original: sin reflejo (D-042 ya
 * decidio que moonlit_art no lo lleva), sin zoom-al-scrollear (D-245/
 * D-246 de Aura eran un encargo especifico del dueno de ESE producto,
 * fuera del alcance de D-030), sin flip/tracklist (SELECT empuja
 * directo la subpagina compartida de canciones, D-029/M8) -- el eje de
 * barrido tambien esta girado (fila en vez de columna), ver
 * moonlit_flow.h (D-041).
 *
 * D-053: el scroll ya NO bloquea. Igual que aura_musicflow.c
 * (anim_pos_x256() ~316-321, scroll_step() ~1238-1259), la posicion
 * visual es una funcion del reloj (current_tick) desde que se pidio el
 * ultimo destino; metro_main.c sondea a HZ/20 mientras
 * moonlit_screen_marea_animating() y pide un cuadro por vuelta. Un
 * nuevo paso de rueda a mitad de camino redirige desde la posicion
 * animada actual (retarget), nunca salta. Cada cuadro repinta SOLO la
 * banda izquierda (152x220); el panel derecho y la cabecera se
 * redibujan una unica vez, en el cuadro de asentamiento.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>   /* snprintf() */
#include <stdlib.h>  /* abs() */
#include <string.h>  /* memcpy() */

#include "file.h"    /* MAX_PATH */
#include "kernel.h"  /* current_tick, HZ */
#include "system.h"  /* cpu_boost() */

#include "moonlit_screen_marea.h"
#include "moonlit_flow.h"
#include "moonlit_wheel.h"
#include "moonlit_art.h"
#include "moonlit_art_cache.h"
#include "moonlit_palette.h"
#include "moonlit_elevation.h"
#include "moonlit_fonts.h"
#include "metro_fb.h"     /* metro_fb_blend_color() -- laterales sin arte */
#include "metro_draw.h"
#include "metro_widgets.h"
#include "metro_motion.h" /* metro_ease() */
#include "metro_screen_list.h"
#include "metro_screen_hub.h"
#include "metro_music.h"
#include "metro_theme.h"
#include "metro_settings.h" /* metro_settings.animations */
#include "metro_lang.h"
#include "metro_keymap.h"
#include "metro_input.h" /* metro_input_last_wheel_velocity() */

/* D-030: tapa central 120px, mismo radio que el horneado de M7. */
#define MAREA_COVER_SIZE    MOONLIT_ART_CACHE_SIZE
#define MAREA_CORNER_RADIUS MOONLIT_ART_CACHE_RADIUS

/* Plan D.3: 2 tapas visibles por lado + margen de 15 para que el LRU
 * por distancia no este descargando/recargando en cada paso mientras
 * el usuario gira la rueda dentro de ese radio. */
#define MAREA_VISIBLE_RADIUS 2
#define MAREA_CACHE_SLOTS    (2 * (MAREA_VISIBLE_RADIUS + 15) + 3) /* 37 */

/* D-030: barra de estado 20px (metro_draw.c:METRO_HEADER_HEIGHT, no
 * exportado -- mismo valor literal que MOONLIT_FLOW_AXIS_LEN=220 ya
 * asume: 240-20). Columna izquierda en x en [0,152); centro de esa
 * columna, x=76. */
#define MAREA_Y_OFFSET     20
#define MAREA_LEFT_BAND_W  152
#define MAREA_AXIS_X       (MAREA_LEFT_BAND_W / 2) /* 76 */

/* Tapa central en reposo: ancla directo (sin pasar por el motor de
 * flujo) para el caso "sin portada" -- D-030 D.5, y=[70,190). */
#define MAREA_CENTER_X ((MAREA_LEFT_BAND_W - MAREA_COVER_SIZE) / 2)             /* 16 */
#define MAREA_CENTER_Y (MAREA_Y_OFFSET + \
                         (MOONLIT_FLOW_AXIS_LEN - MOONLIT_FLOW_DISPLAY_LEN) / 2) /* 70 */

/* Panel derecho (D-030, M8 "Crea"): x=160, ancho 152 -- deja el mismo
 * margen de 8px que separa la columna izquierda del panel tambien del
 * lado derecho de la pantalla (160+152=312, 320-312=8). */
#define MAREA_PANEL_X      160
#define MAREA_PANEL_Y      MAREA_Y_OFFSET
#define MAREA_PANEL_W      152
#define MAREA_PANEL_H      MOONLIT_FLOW_AXIS_LEN
#define MAREA_PANEL_PAD    12
/* moonlit_tokens.h:MOONLIT_CORNER_M (design-system/tokens.json
 * shape.corner_m = 12) -- literal, moonlit_palette.c es el unico
 * includer de moonlit_tokens.h (D-028), mismo criterio que
 * metro_screen_nowplaying.c:corner_s. */
#define MAREA_PANEL_RADIUS 12

/* Geometria lateral (angulo/separacion), re-derivada de
 * aura_musicflow.c MF_ITILT/MF_OFFSETX_R/MF_SLIDE_SPACING_R para
 * MAREA_COVER_SIZE=120 en vez de MF_COVER_SIZE=130 (proporcion
 * *120/130) -- HIPOTESIS a retunear en M8/M12 igual que
 * MOONLIT_FLOW_CAM_DIST (D-041). */
#define MAREA_ITILT           199   /* ~70 grados, mismo valor fijo que pictureflow.c (no depende del tamano) */
#define MAREA_OFFSETX_R       84900
#define MAREA_SLIDE_SPACING_R 26800
#define MAREA_SIDE_FADE       165   /* de 255 -- laterales visibles, no apagadas (mismo valor que aura_musicflow.c) */

/* D-053: duracion del scroll = design-system/tokens.json
 * motion.transition_ms (220 ms) -- literal documentado, patron D-037
 * (este archivo no puede incluir moonlit_tokens.h, D-035). Ya no hay
 * "cuadros": la posicion se evalua por reloj y metro_main.c pide un
 * cuadro cada HZ/20 mientras anima -- si un cuadro tarda mas, el
 * siguiente simplemente cae mas adelante en la curva (salto de cuadros
 * implicito, nunca un bucle que se alarga). */
#define MAREA_SCROLL_ANIM_MS 220

/* D-053/D-055: estado del arte por slot. Un miss en get_slot_for()
 * reclama el slot SIN abrir archivos (monograma/relleno liso) y lo deja
 * en PENDING para moonlit_screen_marea_tick(), que es el unico sitio
 * de este archivo que toca disco (moonlit_art_load_for_album(): lee
 * el .pfraw o, si no existe, decodifica la caratula -- salvo que haya
 * un "<clave>.none" (D-056): entonces devuelve false sin abrir la
 * pista y el slot pasa a MISSING de inmediato). */
enum marea_art_state {
    MAREA_ART_PENDING = 0, /* nadie ha intentado cargarla todavia */
    MAREA_ART_LOADED,      /* cover[] valido */
    MAREA_ART_MISSING,     /* el album no tiene caratula resoluble: monograma definitivo */
};

typedef struct {
    int album_index; /* -1 = slot libre/no cargado */
    enum marea_art_state art;
    char initial[5]; /* UTF-8 + '\0', mismo tamano que metro_screen_list.c:s_index_letter */
    fb_data cover[MAREA_COVER_SIZE * MAREA_COVER_SIZE];
} marea_slot_t;

/* Radio de precarga ociosa alrededor del destino (tick): la ventana
 * visible (+-(MAREA_VISIBLE_RADIUS+1)) mas el paso maximo de rueda
 * (moonlit_wheel_step() <= 3), para que un paso normal no encuentre
 * misses en el centro. Cabe de sobra en los 37 slots. */
#define MAREA_PREFETCH_RADIUS (MAREA_VISIBLE_RADIUS + 1 + 3)

static marea_slot_t s_slots[MAREA_CACHE_SLOTS];
static int s_slots_theme = -1;

/* D-226: scratch estatico para el decode presupuestado de
 * moonlit_screen_marea_tick() -- nunca en el stack del hilo de UI,
 * mismo criterio que s_precache_cover de moonlit_art_cache.c. */
static fb_data s_tick_scratch[MAREA_COVER_SIZE * MAREA_COVER_SIZE];

static const metro_music_item_t *s_albums;
static int s_album_n;
static int s_target_index;

/* D-053: la posicion visual se deriva del reloj -- interpola desde
 * s_anim_from_x256 (donde estaba al pedir el ultimo destino) hacia
 * s_target_index*256 en MAREA_SCROLL_ANIM_MS a partir de s_anim_since.
 * s_settled: ya se dibujo el cuadro de asentamiento (posicion ==
 * destino) -- la unica vez que se repinta panel + cabecera y se cuenta
 * canciones. */
static int s_anim_from_x256;
static long s_anim_since;
static bool s_settled = true;

/* D-053: "N canciones" del panel cacheado por indice (patron
 * target_artist() de aura_musicflow.c ~845-856): el barrido de tagcache
 * de metro_music_song_count_of_album() ocurre una vez por album
 * enfocado, nunca por cuadro. */
static int s_songs_for_index = -1;
static int s_songs_count;

static int anim_pos_x256(void)
{
    int target = s_target_index * 256;
    long elapsed_ms;
    int t;

    if (s_anim_from_x256 == target)
        return target;
    elapsed_ms = (current_tick - s_anim_since) * 1000L / HZ;
    if (elapsed_ms >= MAREA_SCROLL_ANIM_MS)
        return target;
    t = metro_ease(METRO_EASE_OUT_EXPO, (int)elapsed_ms, MAREA_SCROLL_ANIM_MS);
    return s_anim_from_x256 + (target - s_anim_from_x256) * t / 256;
}

bool moonlit_screen_marea_animating(void)
{
    return s_album_n > 0 && !s_settled;
}

/* --- sentinel page: nunca se dibuja/consulta por el camino generico,
 * solo contabilidad de pila (mismo patron que metro_screen_nowplaying.c/
 * metro_screen_photo_viewer.c). */
static int sentinel_count(void *ctx) { (void)ctx; return 0; }
static void sentinel_get_row(void *ctx, int index, struct metro_row *out)
{ (void)ctx; (void)index; (void)out; }
static void sentinel_on_select(void *ctx, int index) { (void)ctx; (void)index; }

static const struct metro_pivot sentinel_pivots[] = {
    { LANG_MAREA_TITLE, sentinel_count, sentinel_get_row, sentinel_on_select, NULL },
};
static const struct metro_page sentinel_page = { LANG_MAREA_TITLE, sentinel_pivots, 1, NULL };

bool moonlit_screen_marea_push(void)
{
    if (!metro_screen_list_push(&sentinel_page))
        return false;

    s_albums = metro_screen_hub_albums(&s_album_n);
    s_target_index = 0;
    s_anim_from_x256 = 0;
    s_anim_since = current_tick;
    s_settled = true;
    s_songs_for_index = -1;
    return true;
}

bool moonlit_screen_marea_is_current(void)
{
    return metro_screen_list_current_page() == &sentinel_page;
}

/* --- cache de tapas: LRU por distancia al indice comprometido, mismo
 * algoritmo que aura_musicflow.c:get_slot_for(). Nunca decodifica un
 * JPEG (regla dura D-030/M8) y, desde D-053, tampoco LEE nada: un
 * cache-miss reclama el slot con el monograma y lo deja PENDING para
 * moonlit_screen_marea_tick() (una carga por vuelta ociosa, fuera de
 * todo cuadro). Con eso desaparece el "slot -1 reutilizado dentro del
 * mismo cuadro" de D-045: el slot queda ocupado por su album desde el
 * primer cuadro y el tick lo rellena en cuanto puede. */
static void invalidate_theme_if_needed(void)
{
    int theme_now = (int)metro_theme_get();
    int i;

    if (s_slots_theme == theme_now)
        return;
    s_slots_theme = theme_now;
    for (i = 0; i < MAREA_CACHE_SLOTS; i++)
        s_slots[i].album_index = -1;
}

static marea_slot_t *find_slot(int album_index)
{
    int i;

    for (i = 0; i < MAREA_CACHE_SLOTS; i++)
        if (s_slots[i].album_index == album_index)
            return &s_slots[i];
    return NULL;
}

/* Reclama un slot para `album_index` (libre, o el mas lejano al
 * destino) -- sin disco. */
static marea_slot_t *claim_slot(int album_index)
{
    int i, free_slot = -1, farthest = -1, farthest_dist = -1;

    for (i = 0; i < MAREA_CACHE_SLOTS; i++)
    {
        int dist;

        if (s_slots[i].album_index == -1)
        {
            free_slot = i;
            break;
        }
        dist = abs(s_slots[i].album_index - s_target_index);
        if (dist > farthest_dist)
        {
            farthest_dist = dist;
            farthest = i;
        }
    }
    i = (free_slot >= 0) ? free_slot : farthest;

    s_slots[i].album_index = album_index;
    s_slots[i].art = MAREA_ART_PENDING;
    metro_lang_initial(s_albums[album_index].label, s_slots[i].initial,
                        sizeof(s_slots[i].initial));
    return &s_slots[i];
}

static marea_slot_t *get_slot_for(int album_index)
{
    marea_slot_t *slot;

    invalidate_theme_if_needed();
    slot = find_slot(album_index);
    return slot ? slot : claim_slot(album_index);
}

/* D-053: el slot PENDING mas cercano al destino, o -- si no hay
 * ninguno -- el album mas cercano dentro de MAREA_PREFETCH_RADIUS que
 * todavia no tiene slot (precarga ociosa, sustituye al preload_range()
 * sincrono de D-045). Devuelve NULL si no hay nada que cargar. */
static marea_slot_t *next_slot_to_load(void)
{
    marea_slot_t *best = NULL;
    int best_dist = 0x7fffffff, i, d;

    invalidate_theme_if_needed();

    for (i = 0; i < MAREA_CACHE_SLOTS; i++)
    {
        if (s_slots[i].album_index < 0 || s_slots[i].art != MAREA_ART_PENDING)
            continue;
        d = abs(s_slots[i].album_index - s_target_index);
        if (d < best_dist)
        {
            best_dist = d;
            best = &s_slots[i];
        }
    }
    if (best)
        return best;

    for (d = 0; d <= MAREA_PREFETCH_RADIUS; d++)
    {
        int sign;

        for (sign = 1; sign >= -1; sign -= 2)
        {
            int idx = s_target_index + sign * d;

            if (idx < 0 || idx >= s_album_n)
                continue;
            if (find_slot(idx) == NULL)
                return claim_slot(idx);
            if (d == 0)
                break;
        }
    }
    return NULL;
}

bool moonlit_screen_marea_tick(void)
{
    marea_slot_t *slot;
    int idx;

    if (s_album_n <= 0)
        return false;

    slot = next_slot_to_load();
    if (slot == NULL)
        return false;

    idx = slot->album_index;
    if (moonlit_art_load_for_album(s_albums[idx].seek, s_tick_scratch))
    {
        /* el decode puede haber cedido la CPU: re-verificar que el
         * slot sigue siendo de este album antes de escribirlo */
        if (slot->album_index == idx)
        {
            memcpy(slot->cover, s_tick_scratch, sizeof(s_tick_scratch));
            slot->art = MAREA_ART_LOADED;
        }
        return true;
    }
    if (slot->album_index == idx)
        slot->art = MAREA_ART_MISSING;
    return true; /* presupuesto gastado: el llamador repinta igual (nada cambio a la vista, pero es barato) */
}

/* --- dibujo: proyeccion por filas (D-041/moonlit_flow.h), eje x=76 --- */

static int lerp256(int a, int b, int t256)
{
    return a + (b - a) * t256 / 256;
}

static void build_fade_lut(unsigned char *lut, int fade, int bg_channel)
{
    int v;

    for (v = 0; v < 256; v++)
        lut[v] = (unsigned char)(bg_channel + ((v - bg_channel) * fade) / 255);
}

static fb_data lut_pixel(fb_data px, const unsigned char *lut_r,
                          const unsigned char *lut_g, const unsigned char *lut_b)
{
    return (fb_data)LCD_RGBPACK(lut_r[RGB_UNPACK_RED(px)],
                                 lut_g[RGB_UNPACK_GREEN(px)],
                                 lut_b[RGB_UNPACK_BLUE(px)]);
}

static void slide_geometry(int offset256, moonlit_flow_slide_t *slide, int *out_fade)
{
    int sign = (offset256 < 0) ? -1 : (offset256 > 0 ? 1 : 0);
    int abs256 = (offset256 < 0) ? -offset256 : offset256;
    int t_center = abs256 < 256 ? abs256 : 256;
    int extra256 = abs256 - 256;

    if (extra256 < 0)
        extra256 = 0;

    slide->angle = -sign * lerp256(0, MAREA_ITILT, t_center);
    slide->distance = 0;
    slide->cx = sign * (lerp256(0, MAREA_OFFSETX_R, t_center)
                         + (int)((long)MAREA_SLIDE_SPACING_R * extra256 / 256));

    *out_fade = lerp256(255, MAREA_SIDE_FADE, t_center);
}

/* Tapa con carátula real (o el placeholder de una sin arte que no
 * quedó exactamente al centro -- ver draw_slide()). */
static void draw_slide_perspective(const fb_data *cover, int offset256)
{
    moonlit_flow_slide_t slide;
    moonlit_flow_projection_t proj;
    int fade;
    unsigned bg = moonlit_color(MROLE_SURFACE);
    int bg_r = RGB_UNPACK_RED(bg), bg_g = RGB_UNPACK_GREEN(bg), bg_b = RGB_UNPACK_BLUE(bg);
    bool use_fade;
    static fb_data row_buf[MAREA_COVER_SIZE];
    static unsigned char lut_r[256], lut_g[256], lut_b[256];

    slide_geometry(offset256, &slide, &fade);
    use_fade = fade < 255;
    if (use_fade)
    {
        build_fade_lut(lut_r, fade, bg_r);
        build_fade_lut(lut_g, fade, bg_g);
        build_fade_lut(lut_b, fade, bg_b);
    }

    moonlit_flow_begin_projection(&proj, &slide, MAREA_COVER_SIZE);

    while (proj.screen_y < MOONLIT_FLOW_AXIS_LEN)
    {
        int row = moonlit_flow_source_row(&proj);
        int dx = moonlit_flow_cross_scale(&proj);
        const fb_data *src_row = cover + (size_t)row * MAREA_COVER_SIZE;
        int cover_disp = (MAREA_COVER_SIZE << MOONLIT_FLOW_SHIFT) / dx;
        int x0 = MAREA_AXIS_X - cover_disp / 2;
        int p = 0, dest_col, n_cols = 0;

        for (dest_col = 0; dest_col < MAREA_COVER_SIZE; dest_col++)
        {
            int source_col = p >> MOONLIT_FLOW_SHIFT;
            fb_data px;

            if (source_col >= MAREA_COVER_SIZE)
                break;
            px = src_row[source_col];
            row_buf[dest_col] = use_fade ? lut_pixel(px, lut_r, lut_g, lut_b) : px;
            p += dx;
            n_cols++;
        }
        if (n_cols > 0)
            lcd_bitmap(row_buf, x0, proj.screen_y + MAREA_Y_OFFSET, n_cols, 1);

        if (!moonlit_flow_advance_column(&proj))
            break;
    }
}

/* Tapa sin carátula que NO cayó exacto al centro: en vez de reservar
 * un segundo buffer de 120x120 solo para un color plano (D.3's límite
 * de .bss ya está ajustado con los MAREA_CACHE_SLOTS), se proyecta un
 * relleno liso -- mismo eje/angulo/fundido que la real, sin muestreo
 * de textura. */
static void draw_slide_flat(int offset256)
{
    moonlit_flow_slide_t slide;
    moonlit_flow_projection_t proj;
    int fade;
    unsigned color;

    slide_geometry(offset256, &slide, &fade);
    color = metro_fb_blend_color(moonlit_color(MROLE_PRIMARY_CONTAINER),
                                  moonlit_color(MROLE_SURFACE), 255 - fade);

    moonlit_flow_begin_projection(&proj, &slide, MAREA_COVER_SIZE);
    lcd_set_foreground(color);

    while (proj.screen_y < MOONLIT_FLOW_AXIS_LEN)
    {
        int dx = moonlit_flow_cross_scale(&proj);
        int cover_disp = (MAREA_COVER_SIZE << MOONLIT_FLOW_SHIFT) / dx;
        int x0 = MAREA_AXIS_X - cover_disp / 2;

        if (cover_disp > 0)
            lcd_hline(x0, x0 + cover_disp - 1, proj.screen_y + MAREA_Y_OFFSET);

        if (!moonlit_flow_advance_column(&proj))
            break;
    }
}

/* D-030 D.5: tapa central sin carátula -- tarjeta plana (nivel 2) con
 * la inicial en primary, dibujada directo (sin pasar por el motor de
 * flujo: en offset==0 no hay perspectiva que proyectar). */
static void draw_monogram(const marea_slot_t *slot)
{
    const char *initial = slot->initial[0] ? slot->initial : "?";
    int w, h;

    moonlit_draw_surface(MAREA_CENTER_X, MAREA_CENTER_Y, MAREA_COVER_SIZE, MAREA_COVER_SIZE,
                          MSURFACE_BASE, MAREA_CORNER_RADIUS);
    lcd_setfont(metro_font_id(MFONT_HEADLINE));
    lcd_getstringsize((const unsigned char *)initial, &w, &h);
    metro_draw_text(MFONT_HEADLINE,
                     MAREA_CENTER_X + (MAREA_COVER_SIZE - w) / 2,
                     MAREA_CENTER_Y + (MAREA_COVER_SIZE - h) / 2,
                     initial, moonlit_color(MROLE_PRIMARY));
}

static void draw_slide(int idx, int offset256)
{
    marea_slot_t *slot = get_slot_for(idx);

    if (offset256 == 0 && slot->art != MAREA_ART_LOADED)
    {
        draw_monogram(slot);
        return;
    }

    if (slot->art == MAREA_ART_LOADED)
        draw_slide_perspective(slot->cover, offset256);
    else
        draw_slide_flat(offset256);
}

static void draw_panel(void)
{
    const metro_music_item_t *album = &s_albums[s_target_index];
    int text_x = MAREA_PANEL_X + MAREA_PANEL_PAD;
    int text_w = MAREA_PANEL_W - 2 * MAREA_PANEL_PAD;
    char songs_buf[48];

    moonlit_draw_surface(MAREA_PANEL_X, MAREA_PANEL_Y, MAREA_PANEL_W, MAREA_PANEL_H,
                          MSURFACE_LOW, MAREA_PANEL_RADIUS);

    metro_draw_text_cut_right(MFONT_HEADLINE, text_x, MAREA_PANEL_Y + 24,
                               album->label, moonlit_color(MROLE_ON_SURFACE), text_w);

    if (album->subtitle[0])
        metro_draw_text_cut_right(MFONT_BODY, text_x, MAREA_PANEL_Y + 56,
                                   album->subtitle, moonlit_color(MROLE_ON_SURFACE_VARIANT),
                                   text_w);

    /* D-053: barrido de tagcache solo al cambiar de album enfocado. */
    if (s_songs_for_index != s_target_index)
    {
        s_songs_count = metro_music_song_count_of_album(album->seek);
        s_songs_for_index = s_target_index;
    }
    snprintf(songs_buf, sizeof(songs_buf), metro_lang_str(LANG_MAREA_SONGS_FMT),
             s_songs_count);
    metro_draw_text_cut_right(MFONT_LABEL, text_x, MAREA_PANEL_Y + 84,
                               songs_buf, moonlit_color(MROLE_ON_SURFACE_VARIANT), text_w);
}

/* Banda izquierda (x en [0,152), y en [20,240)): 7 tapas como maximo,
 * de afuera hacia adentro. NO limpia ni actualiza el LCD -- lo hacen
 * los dos llamadores. */
static void draw_carousel(int pos256)
{
    struct { int idx; int offset256; } entries[2 * MAREA_VISIBLE_RADIUS + 3];
    int n = 0, i, center_idx;

    center_idx = (pos256 + (pos256 >= 0 ? 128 : -128)) / 256;

    for (i = -(MAREA_VISIBLE_RADIUS + 1); i <= MAREA_VISIBLE_RADIUS + 1; i++)
    {
        int idx = center_idx + i;

        if (idx < 0 || idx >= s_album_n)
            continue;
        entries[n].idx = idx;
        entries[n].offset256 = idx * 256 - pos256;
        n++;
    }

    /* de afuera hacia adentro (|offset256| descendente) -- la mas
     * cercana al centro se dibuja al final y queda encima de
     * cualquier lateral con la que se solape cerca del borde,
     * insercion simple (n es siempre un puñado de elementos). */
    for (i = 1; i < n; i++)
    {
        int a = i, key_abs;

        key_abs = entries[a].offset256 < 0 ? -entries[a].offset256 : entries[a].offset256;
        while (a > 0)
        {
            int b_abs = entries[a - 1].offset256 < 0 ? -entries[a - 1].offset256
                                                      : entries[a - 1].offset256;
            if (b_abs >= key_abs)
                break;
            { int t = entries[a].idx; entries[a].idx = entries[a - 1].idx; entries[a - 1].idx = t; }
            { int t = entries[a].offset256; entries[a].offset256 = entries[a - 1].offset256;
              entries[a - 1].offset256 = t; }
            a--;
        }
    }

    for (i = 0; i < n; i++)
        draw_slide(entries[i].idx, entries[i].offset256);
}

/* Pantalla completa: limpiar + cabecera + banda + panel + lcd_update().
 * Solo al entrar, al cambiar tema/idioma (redraw_current()) y en el
 * cuadro de asentamiento -- nunca por cuadro de scroll (D-053). */
void moonlit_screen_marea_show(void)
{
    metro_draw_clear();
    metro_draw_header(metro_lang_str(LANG_MAREA_TITLE));

    if (s_album_n <= 0)
    {
        metro_widgets_draw_empty_state(metro_lang_str(LANG_MAREA_EMPTY));
        lcd_update();
        return;
    }

    draw_carousel(anim_pos_x256());
    draw_panel();
    lcd_update();
}

/* D-053: un cuadro de scroll. Repinta SOLO la banda izquierda
 * (152x220 = 33 440 px de fondo + las tapas) con lcd_update_rect();
 * el panel derecho (otros 33 440 px + 576 plot_alpha + 144 isqrt de
 * moonlit_draw_surface) y la cabecera no se tocan. Bajo cpu_boost():
 * el S5L8702 corre a 54 MHz sin boost y la proyeccion de 7 tapas es
 * el trabajo mas caro por cuadro de todo moonlit. En el cuadro de
 * asentamiento (posicion == destino, primera vez) cae a
 * moonlit_screen_marea_show() completa -- ahi y solo ahi se actualiza
 * el panel. */
void moonlit_screen_marea_show_carousel(void)
{
    int pos256;

    if (s_album_n <= 0 || !lcd_active())
    {
        s_settled = true;
        return;
    }

    pos256 = anim_pos_x256();
    if (pos256 == s_target_index * 256)
    {
        if (!s_settled)
        {
            s_settled = true;
            moonlit_screen_marea_show();
            return;
        }
        /* ya asentada: solo la banda (p.ej. una tapa recien cargada) */
    }

    cpu_boost(true);
    lcd_set_foreground(moonlit_color(MROLE_SURFACE));
    lcd_fillrect(0, MAREA_Y_OFFSET, MAREA_LEFT_BAND_W, MOONLIT_FLOW_AXIS_LEN);
    draw_carousel(pos256);
    lcd_update_rect(0, MAREA_Y_OFFSET, MAREA_LEFT_BAND_W, MOONLIT_FLOW_AXIS_LEN);
    cpu_boost(false);
}

/* --- entrada: scroll (D-030, D-053), select/playpause/back/home ----- */

/* D-053 (patron aura_musicflow.c scroll_step() ~1238-1259): redirige
 * desde la posicion animada ACTUAL, nunca desde el destino viejo -- si
 * el usuario sigue girando la rueda antes de que el paso anterior
 * termine de asentarse, el carrusel sigue deslizandose suave desde
 * donde ya estaba. Regresa de inmediato: metro_main.c dibuja los
 * cuadros a HZ/20 mientras moonlit_screen_marea_animating(). */
static void scroll_step(int dir)
{
    int step, new_target, from_x256;

    if (s_album_n <= 0)
        return;

    step = moonlit_wheel_step(metro_input_last_wheel_velocity());
    new_target = s_target_index + dir * step;
    if (new_target < 0)
        new_target = 0;
    if (new_target >= s_album_n)
        new_target = s_album_n - 1;
    if (new_target == s_target_index)
        return;

    from_x256 = anim_pos_x256();
    s_target_index = new_target;
    s_anim_since = current_tick;
    s_settled = false;

    /* 05-plan-correctivo.md M8: animado solo bajo lcd_active() y
     * animations != off (patron metro_screen_hub.c hub_row_animates());
     * si no, la posicion ya ES el destino y el primer cuadro (que
     * metro_main.c pide en seguida) es directamente el de asentamiento. */
    if (!lcd_active() || metro_settings.animations == METRO_ANIM_OFF)
        s_anim_from_x256 = new_target * 256;
    else
        s_anim_from_x256 = from_x256;
}

void moonlit_screen_marea_handle(int action, int steps)
{
    (void)steps;

    switch (action)
    {
        case MACT_NEXT:
            scroll_step(1);
            break;
        case MACT_PREV:
            scroll_step(-1);
            break;
        case MACT_SELECT:
            if (s_album_n > 0)
                metro_screen_hub_open_album_songs(s_albums[s_target_index].seek,
                                                   s_albums[s_target_index].label);
            break;
        case MACT_PLAYPAUSE:
            if (s_album_n > 0)
                metro_music_play_songs_of_album(s_albums[s_target_index].seek, 0);
            break;
        case MACT_BACK:
            metro_screen_list_pop();
            break;
        case MACT_HOME:
            metro_screen_list_pop_to_root();
            break;
        default:
            break;
    }
}
