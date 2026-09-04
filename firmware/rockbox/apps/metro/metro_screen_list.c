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
#include <stddef.h>
#include <string.h>
#include "string-extra.h"
#include "lcd.h"
#include "kernel.h"
#include "button.h"

#include "metro_screen_list.h"
#include "metro_draw.h"
#include "metro_lang.h"
#include "metro_widgets.h"
#include "metro_motion.h"
#include "metro_settings.h" /* moonlit (D-052 C4): puerta metro_settings.animations */
#include "metro_transitions.h"
#include "metro_music.h" /* R4/FA-8: metro_music_playpause() */
#include "moonlit_marquee.h" /* moonlit (D-067) */
#include "moonlit_palette.h" /* moonlit (D-011, M4): divisores outline_variant */
#include "moonlit_elevation.h" /* moonlit (D-044, M9): tarjeta de fila de "Acerca de" */
#include "moonlit_logo.h" /* moonlit (D-016, D-044, M9, D-064): creciente + wordmark de "Acerca de" */
#include "metro_screen_about.h" /* moonlit (D-062): fila de diagnostico de la pila */

static metro_nav_t s_nav;

/* moonlit (D-016, D-044, M9): "Acerca de" es la unica pantalla ademas
 * del arranque que muestra el creciente + wordmark de 64px (D-016).
 * No pasa por metro_draw_rows()/metro_draw_rows_ex() -- esas dibujan
 * SIEMPRE desde METRO_DRAW_ROWS_FIRST_Y, un eje compartido con
 * CONTINUUM (metro_transitions.c) y con cada otro pivote de settings;
 * moverlo para uno solo rompería esa alineacion global. En cambio,
 * "Acerca de" tiene su propio bucle de filas -- mismo patron que ya
 * usa el hub (metro_screen_hub.c: "su propio bucle de dibujo") --
 * que arranca mas abajo, sin cascada FEATHER ni divisores de fila:
 * perdida aceptada, "Acerca de" ya asume scroll para su contenido
 * largo (créditos, conteos de biblioteca). */
/* moonlit (D-064): el hero deja de ser un bloque fijo de 64px que se
 * come 84 px de alto y deja sitio para DOS filas -- el defecto que el
 * plan de la ronda localizo: la navegacion movia la seleccion con
 * METRO_DRAW_ROWS_VISIBLE (5) mientras el dibujo solo tenia sitio para
 * 2, asi que en los desplazamientos relativos 3 y 4 la seleccion se
 * salia de la pantalla y habia filas de "Acerca de" (creditos,
 * licencias) que NUNCA se veian.
 *
 * Ahora el hero es compacto (creciente de 40 px + wordmark en una sola
 * linea de 40 px) y es la "fila -1" de la lista: solo se dibuja
 * mientras la ventana esta arriba del todo (first == 0), y en cuanto
 * el usuario baja desaparece y las filas usan toda la pantalla. El
 * numero de filas visibles se DERIVA de esa geometria en
 * about_visible_rows() -- nunca de METRO_DRAW_ROWS_VISIBLE -- y el
 * mismo numero lo usan el dibujo y la navegacion.
 *
 * D-016 decia "el wordmark solo se dibuja junto al creciente de 64px";
 * este hero es la segunda pareja admitida (40 px), ver D-064. */
#define METRO_ABOUT_HERO_Y         80
#define METRO_ABOUT_HERO_SIZE      MOONLIT_LOGO_CRESCENT_SIZE_40
#define METRO_ABOUT_HERO_GAP       8   /* creciente <-> wordmark */
#define METRO_ABOUT_HERO_BOTTOM    4   /* hero <-> primera fila */
#define METRO_ABOUT_ROWS_FIRST_Y   80
#define METRO_ABOUT_ROW_PITCH      METRO_DRAW_ROW_PITCH

/* Una fila "asomando" cuenta como asomando de verdad solo si se le ve
 * al menos esto de texto; si no, no se dibuja (un sliver de 4 px es
 * ruido, no una pista de que hay mas lista). */
#define METRO_ABOUT_PEEK_MIN       12

static void draw_about_hero(void)
{
    int wordmark_y = METRO_ABOUT_HERO_Y +
                      (METRO_ABOUT_HERO_SIZE - MOONLIT_LOGO_WORDMARK_HEIGHT) / 2;

    moonlit_logo_draw_crescent(METRO_ABOUT_HERO_SIZE, METRO_DRAW_LEFT_X,
                               METRO_ABOUT_HERO_Y, metro_color_fg());
    moonlit_logo_draw_wordmark(METRO_DRAW_LEFT_X + METRO_ABOUT_HERO_SIZE +
                               METRO_ABOUT_HERO_GAP, wordmark_y, metro_color_fg());
}

/* El hero solo esta cuando la ventana no se ha movido. */
static bool about_hero_visible(int first)
{
    return first == 0;
}

static int about_rows_first_y(int first)
{
    if (!about_hero_visible(first))
        return METRO_ABOUT_ROWS_FIRST_Y;

    return METRO_ABOUT_HERO_Y + METRO_ABOUT_HERO_SIZE + METRO_ABOUT_HERO_BOTTOM;
}

/* Filas COMPLETAS que caben, derivadas de la geometria real (nunca de
 * METRO_DRAW_ROWS_VISIBLE): 4 con el hero puesto (124/152/180/208), 5
 * sin el (80/108/136/164/192). Es el numero que usan por igual el
 * bucle de dibujo y metro_nav_move_sel(). */
static int about_visible_rows(int first)
{
    return (LCD_HEIGHT - about_rows_first_y(first)) / METRO_ABOUT_ROW_PITCH;
}

static void draw_about_rows(const struct metro_pivot *pivot, int first, int sel)
{
    int count = pivot->count(pivot->ctx);
    int visible = about_visible_rows(first);
    int i, y = about_rows_first_y(first);

    for (i = first; i < count && i < first + visible + 1; i++)
    {
        struct metro_row row;
        bool selected = (i == sel);

        if (y + METRO_ABOUT_PEEK_MIN > LCD_HEIGHT)
            break;

        pivot->get_row(pivot->ctx, i, &row);

        if (selected)
            moonlit_draw_selection_card(y - 4, METRO_ABOUT_ROW_PITCH, 256,
                                        METRO_ABOUT_ROW_PITCH, true);

        /* moonlit (D-067): las filas de "Acerca de" son las mas largas
         * de moonlit (URL del repositorio, licencias) y no caben en
         * 308 px -- la seleccionada desplaza. */
        if (selected)
            moonlit_marquee_draw(MOONLIT_MARQUEE_ABOUT, MFONT_LIST_SEL,
                                  METRO_DRAW_LEFT_X,
                                  LCD_WIDTH - METRO_DRAW_LEFT_X, y, row.title,
                                  metro_color_fg());
        else
            metro_draw_text_cut_right(MFONT_LIST, METRO_DRAW_LEFT_X, y,
                                       row.title, metro_color_secondary(),
                                       LCD_WIDTH - METRO_DRAW_LEFT_X);
        y += METRO_ABOUT_ROW_PITCH;
    }
}

/* moonlit (D-011, M4): un lcd_hline por borde entre filas visibles,
 * en outline_variant (D-028) -- se dibuja ANTES que metro_draw_rows(),
 * asi que la tarjeta de la fila seleccionada (metro_draw_rows_ex(),
 * metro_draw.c) lo tapa dentro de su propio slot y no compite con sus
 * bordes de luz/sombra. Solo aplica a listas de texto -- las
 * cuadriculas (tile_cols > 0) no llaman a esto. */
static void draw_row_dividers(int count)
{
    int i, y = METRO_DRAW_ROWS_FIRST_Y - 4;
    int visible = METRO_DRAW_ROWS_VISIBLE + 1;

    if (visible > count)
        visible = count;

    lcd_set_foreground(moonlit_color(MROLE_OUTLINE_VARIANT));
    for (i = 1; i < visible; i++)
    {
        y += METRO_DRAW_ROW_PITCH;
        lcd_hline(METRO_DRAW_LEFT_X, LCD_WIDTH - METRO_DRAW_LEFT_X, y);
    }
}

/* F10: floating index letter (S1.4) -- shown for 600ms after a fast
 * scroll (steps >= 3) lands on a new row, using that row's own first
 * character. Deliberately without the plan's "count >= 40" gate
 * (DESVIACIONES.md F10-1): none of Metro's real lists have that many
 * items with the test fixtures this session can generate, and a big
 * jump on a shorter list still benefits from the same feedback. */
#define METRO_INDEX_LETTER_TICKS (HZ * 3 / 5)
#define METRO_INDEX_LETTER_MIN_STEPS 3

static long s_index_letter_until = 0;
static char s_index_letter[5] = "";

/* F12: FEATHER (S3.3) -- staggered row entrance after a PUSH, animations=all
 * only. Set on every metro_screen_list_push() and consumed (or just
 * discarded, both are fine -- see the module comment on
 * metro_screen_list_run_feather_if_pending()) by whatever
 * metro_main.c does next; the push into Now Playing's sentinel page
 * sets it too but that path always ends in FADE, never calling the
 * run function below, so it just sits there until the NEXT real push
 * resets it -- never a stale cascade on an unrelated screen. */
#define METRO_FEATHER_FRAMES 6
#define METRO_FEATHER_ROW_OFFSET_PX 8

static bool s_feather_pending = false;

/* page_stack[d-1] holds the page pushed at nav depth d, for d>=2 --
 * index 0 (depth 1, the hub) is never used, metro_screen_hub.c owns
 * its own pivot and only borrows s_nav's depth-1 frame for its
 * selection/windowing state. */
static const struct metro_page *page_stack[METRO_NAV_MAX_DEPTH];

void metro_screen_list_init(void)
{
    metro_nav_init(&s_nav, 1);
}

metro_nav_t *metro_screen_nav(void)
{
    return &s_nav;
}

/* moonlit (D-077): pivot 0 de la pagina que se acaba de mostrar (recien
 * empujada, o revelada por un metro_nav_pivot_prev() que aterrizo ahi)
 * es un "launcher" -- no se dibuja nunca, se dispara solo. Ver el
 * comentario de is_launcher en metro_page.h. */
static void maybe_auto_launch_pivot_zero(const struct metro_page *page)
{
    if (page->npivots > 0 && page->pivots[0].is_launcher)
        page->pivots[0].on_select(page->pivots[0].ctx, 0);
}

bool metro_screen_list_push(const struct metro_page *page)
{
    if (!metro_nav_push(&s_nav, page->npivots))
        return false;

    /* moonlit (D-067 addendum): la marquesina guarda su reloj POR
     * RANURA, no por texto, y la ranura de "fila seleccionada" es una
     * sola para todas las listas. Sin este reinicio, dos pantallas cuyo
     * primer texto coincida en los primeros 48 bytes de la clave
     * heredan el ciclo a medias y la fila nueva arranca desplazandose,
     * sin el tramo quieto que existe para poder leerla. Se detecto al
     * portarla a Metro (M-106): la funcion estaba escrita y documentada
     * pero no la llamaba nadie. */
    moonlit_marquee_reset();

    page_stack[metro_nav_depth(&s_nav) - 1] = page;
    s_feather_pending = true;

    /* moonlit (D-077): metro_nav_push() siempre arranca en pivot 0
     * (metro_nav.h) -- "Música entra directo a Marea" es exactamente
     * esto para music_page, sin que metro_screen_hub.c necesite saber
     * que Marea existe. */
    maybe_auto_launch_pivot_zero(page);
    return true;
}

bool metro_screen_list_pop(void)
{
    moonlit_marquee_reset(); /* moonlit (D-067 addendum): ver push() */
    return metro_nav_pop(&s_nav);
}

void metro_screen_list_pop_to_root(void)
{
    moonlit_marquee_reset();
    metro_nav_pop_to_root(&s_nav);
}

static const struct metro_page *current_page(void)
{
    int depth = metro_nav_depth(&s_nav);

    if (depth < 2)
        return NULL;

    return page_stack[depth - 1];
}

const struct metro_page *metro_screen_list_current_page(void)
{
    return current_page();
}

bool metro_screen_list_has_pending_redraw(void)
{
    return current_tick < s_index_letter_until;
}

void metro_screen_list_show(void)
{
    const struct metro_page *page = current_page();
    const struct metro_pivot *pivot;
    int active;

    if (!page)
        return;

    active = metro_nav_pivot(&s_nav);
    pivot = &page->pivots[active];

    metro_draw_clear();
    metro_draw_header(page->title_dynamic ? page->title_dynamic
                                           : metro_lang_str(page->title));
    metro_draw_pivots(page, active, 0);

    /* metro_screen_settings_page() COPIES metro_screen_about_pivot by
     * value into its own all_pivots[2] (metro_screen_settings.c), so
     * comparing pointers here would never match -- name survives the
     * copy and LANG_PIVOT_ABOUT is unique to this one pivot. */
    if (pivot->name == LANG_PIVOT_ABOUT)
    {
        /* moonlit (D-016, D-044, M9): geometria propia -- ver el
         * comentario junto a draw_about_hero() mas arriba. */
        if (about_hero_visible(metro_nav_first_visible(&s_nav)))
            draw_about_hero();
        draw_about_rows(pivot, metro_nav_first_visible(&s_nav), metro_nav_sel(&s_nav));
    }
    else if (pivot->count(pivot->ctx) == 0)
        metro_widgets_draw_empty_state(metro_lang_str(
            pivot->empty_message ? pivot->empty_message : LANG_EMPTY_LIST));
    else if (pivot->tile_cols > 0)
        metro_draw_tiles(pivot, metro_nav_first_visible(&s_nav), metro_nav_sel(&s_nav), 0);
    else
    {
        draw_row_dividers(pivot->count(pivot->ctx));
        metro_draw_rows(pivot, metro_nav_first_visible(&s_nav), metro_nav_sel(&s_nav), 0);
    }

    /* R2-F2/DD-7: the floating index letter doesn't apply to grids
     * (also never gets armed for one -- see metro_screen_list_handle()) */
    if (pivot->tile_cols == 0 && current_tick < s_index_letter_until)
        metro_widgets_draw_index_letter(s_index_letter);

    lcd_update();
}

/* moonlit (D-052 C4, "Marea que sube"): the row the selection just
 * moved to lights up instead of snapping -- METRO_SELECTION_FRAMES
 * frames, METRO_SELECTION_FRAME_TICKS apart (metro_transitions.h: 80
 * ms, tokens.json motion.selection_ms), METRO_EASE_OUT_QUAD: the card
 * tone rises from surface to surface_container_high, the primary
 * marker grows from the top of the row, and the D-012 edges land on
 * the last frame only. Only the two rows involved are repainted
 * (metro_draw_row_slot()) and pushed with lcd_update_rect(), ~18k px
 * per frame, no framebuffer capture -- so it also runs under
 * `minimal`, unlike FEATHER. Never on a wheel jump (steps > 1: the
 * eye is on the index letter, not on one row), never when the window
 * scrolled (every row moved, a partial update would tear), never in
 * grids, "Acerca de" (its own geometry) or empty lists. The caller's
 * redraw_current() (metro_main.c) paints the settled screen right
 * after, as it always did. */
static void run_selection_rise(const struct metro_pivot *pivot, int prev_first, int prev_sel)
{
    int first = metro_nav_first_visible(&s_nav);
    int sel = metro_nav_sel(&s_nav);
    int top, frame;
    long start_tick = current_tick;

    if (!lcd_active() || metro_settings.animations == METRO_ANIM_OFF)
        return;
    if (pivot->tile_cols > 0 || pivot->name == LANG_PIVOT_ABOUT)
        return;
    if (pivot->count(pivot->ctx) == 0 || sel == prev_sel || first != prev_first)
        return;
    if (current_tick < s_index_letter_until)
        return; /* the F10 letter still floats over the rows: a band update would cut it */

    top = metro_draw_row_slot(pivot, prev_sel, prev_sel - first, false, -1, 0, false);
    lcd_update_rect(0, top, LCD_WIDTH, METRO_DRAW_ROW_PITCH);

    for (frame = 1; frame <= METRO_SELECTION_FRAMES; frame++)
    {
        int p = metro_ease(METRO_EASE_OUT_QUAD, frame, METRO_SELECTION_FRAMES);
        bool last = (frame == METRO_SELECTION_FRAMES);

        top = metro_draw_row_slot(pivot, sel, sel - first, true, p,
                                  (METRO_DRAW_ROW_PITCH * p) / 256, last);
        lcd_update_rect(0, top, LCD_WIDTH, METRO_DRAW_ROW_PITCH);
        metro_transitions_trace("select", frame, METRO_SELECTION_FRAMES, start_tick);

        if (!last)
            sleep(METRO_SELECTION_FRAME_TICKS);
    }
}

void metro_screen_list_handle(int action, int steps)
{
    const struct metro_page *page = current_page();
    const struct metro_pivot *pivot;
    int count;

    if (!page)
        return;

    pivot = &page->pivots[metro_nav_pivot(&s_nav)];
    count = pivot->count(pivot->ctx);

    switch (action)
    {
        case MACT_PREV:
        case MACT_NEXT:
            /* R2-F2/DD-7: grid pivots move the same linear index one
             * tile at a time (reading order, not a column jump) but
             * window by whole rows of tile_cols -- the floating index
             * letter (F10) doesn't apply here, it's a row-list-only
             * affordance. */
            if (pivot->tile_cols > 0)
            {
                metro_nav_move_sel_grid(&s_nav, action == MACT_PREV ? -steps : steps,
                                         count, pivot->tile_cols, METRO_TILE_ROWS_VISIBLE);
                break;
            }
            {
            int prev_first = metro_nav_first_visible(&s_nav);
            int prev_sel = metro_nav_sel(&s_nav);
            /* moonlit (D-064): "Acerca de" tiene su propia geometria, y
             * por lo tanto su propio numero de filas visibles -- que
             * ademas cambia cuando el hero se va (4 -> 5). Pasar aqui
             * la constante global era el bug: la seleccion se movia a
             * filas que el dibujo nunca ponia en pantalla. */
            int visible = (pivot->name == LANG_PIVOT_ABOUT)
                              ? about_visible_rows(prev_first)
                              : METRO_DRAW_ROWS_VISIBLE;

            metro_nav_move_sel(&s_nav, action == MACT_PREV ? -steps : steps,
                                count, visible);
            if (steps == 1)
                run_selection_rise(pivot, prev_first, prev_sel);
            }
            if (steps >= METRO_INDEX_LETTER_MIN_STEPS && count > 0)
            {
                struct metro_row row;
                pivot->get_row(pivot->ctx, metro_nav_sel(&s_nav), &row);
                /* R4/FA-5a (M-076): carácter completo, no primer byte. */
                metro_lang_initial(row.title, s_index_letter,
                                    sizeof(s_index_letter));
                if (!s_index_letter[0])
                {
                    s_index_letter[0] = '?';
                    s_index_letter[1] = '\0';
                }
                s_index_letter_until = current_tick + METRO_INDEX_LETTER_TICKS;
            }
            break;
        case MACT_PIVOT_PREV:
            moonlit_marquee_reset(); /* D-067 addendum */
            if (metro_nav_pivot_prev(&s_nav))
                /* moonlit (D-077): "desde el primer pivote de lista LEFT
                 * vuelve a Marea" -- pivot 0 de music_page es un
                 * launcher (nunca se dibuja), asi que aterrizar ahi lo
                 * dispara en vez de mostrarlo. En cualquier otra pagina
                 * multi-pivote pivots[0].is_launcher es false (el valor
                 * por defecto) y esto no hace nada, F.1 sigue intacto. */
                maybe_auto_launch_pivot_zero(page);
            break;
        case MACT_PIVOT_NEXT:
            moonlit_marquee_reset();
            metro_nav_pivot_next(&s_nav);
            break;
        case MACT_SELECT:
            if (pivot->on_select)
            {
                /* R3-F8/DD-9 (M-069): CONTINUUM. El título de la fila
                 * elegida, y dónde está dibujada AHORA, hay que
                 * capturarlos antes de on_select() -- que puede empujar
                 * una página nueva y dejar `pivot`/la selección
                 * apuntando a otra cosa. */
                char from_title[METRO_CONTINUUM_TITLE_MAX];
                int from_y = 0;
                bool have_from = false;

                if (pivot->tile_cols == 0 && count > 0)
                {
                    struct metro_row row;

                    pivot->get_row(pivot->ctx, metro_nav_sel(&s_nav), &row);
                    if (row.title && row.title[0])
                    {
                        strlcpy(from_title, row.title, sizeof(from_title));
                        from_y = METRO_DRAW_ROWS_FIRST_Y +
                                 (metro_nav_sel(&s_nav) - metro_nav_first_visible(&s_nav)) *
                                     METRO_DRAW_ROW_PITCH;
                        have_from = true;
                    }
                }

                pivot->on_select(pivot->ctx, metro_nav_sel(&s_nav));

                /* La tercera puerta de DD-9: solo si hay continuidad
                 * REAL que mostrar, es decir si la página nueva se
                 * llama igual que la fila que la abrió (su ceja es
                 * title_dynamic). Un "canciones" genérico abierto desde
                 * la fila "Analog Dreams" no coincide con nada y hace
                 * PUSH normal, sin inventar una animación. */
                if (have_from)
                {
                    const struct metro_page *dest = current_page();

                    if (dest && dest != page && dest->title_dynamic &&
                        !strcmp(dest->title_dynamic, from_title))
                        metro_transitions_arm_continuum(from_title, from_y);
                }
            }
            break;
        /* moonlit (D-062 §E.4): SELECT sostenido sobre la fila de version
         * de "Acerca de" revela/oculta la marca de agua de la pila del
         * hilo principal. En cualquier otra lista o fila no hace nada:
         * hasta ahora SELECT sostenido tampoco hacia nada en LIST (el
         * REL posterior no dispara MACT_SELECT porque su prerrequisito
         * es BUTTON_SELECT y el ultimo boton pasa a ser
         * BUTTON_SELECT|BUTTON_REPEAT), asi que no le quita el gesto a
         * nadie -- mismo patron que MACT_TOGGLE_SHUFFLE en PLAYER. */
        case MACT_SELECT_HOLD:
            if (pivot->name == LANG_PIVOT_ABOUT &&
                metro_screen_about_row_is_version(metro_nav_sel(&s_nav)))
                metro_screen_about_toggle_diag();
            break;
        case MACT_BACK:
            metro_screen_list_pop();
            break;
        case MACT_HOME:
            metro_screen_list_pop_to_root();
            break;
        case MACT_PLAYPAUSE:
            metro_music_playpause();
            break;
        default:
            break;
    }
}

/* F12: no-op (just clears the flag) unless a push actually happened
 * and metro_main.c's dispatch decided this is the moment to run it --
 * see the module comment on s_feather_pending. Redraws only the row
 * area (metro_draw_clear_rows_area()) each frame, never the header/
 * pivots above it -- those already settled during the PUSH slide
 * that ran just before this. */
void metro_screen_list_run_feather_if_pending(void)
{
    const struct metro_page *page;
    const struct metro_pivot *pivot;
    int active, count, first, sel, frame;

    if (!s_feather_pending)
        return;
    s_feather_pending = false;

    /* PLAN_MAESTRO.md S3.3: FEATHER only runs under animations=all,
     * "off" under both minimal and off -- more restrictive than
     * PUSH/POP/twist, which still animate (as a plain slide) under
     * minimal. */
    if (!lcd_active() || !metro_transitions_effective_all())
        return;

    page = current_page();
    if (!page)
        return;

    active = metro_nav_pivot(&s_nav);
    pivot = &page->pivots[active];
    count = pivot->count(pivot->ctx);
    if (count == 0)
        return; /* empty-state tile drew instead -- nothing to cascade */
    if (pivot->tile_cols > 0)
        return; /* R2-F2/DD-7: FEATHER is a row-list affordance, not for grids */

    first = metro_nav_first_visible(&s_nav);
    sel = metro_nav_sel(&s_nav);

    for (frame = 0; frame < METRO_FEATHER_FRAMES; frame++)
    {
        int y_offsets[METRO_DRAW_ROWS_VISIBLE + 1];
        int row;

        for (row = 0; row <= METRO_DRAW_ROWS_VISIBLE; row++)
        {
            /* 1-frame/row stagger (S3.3): row N doesn't start falling
             * until frame N, then eases the remaining frames it has
             * left in the budget down to 0 -- always lands exactly on
             * 0 in its own last frame regardless of how few frames
             * that leaves it (metro_ease()'s i==frames endpoint). */
            int start_frame = row;
            int local_frame = frame - start_frame;
            int local_total = METRO_FEATHER_FRAMES - start_frame;

            if (local_frame < 0)
                y_offsets[row] = METRO_FEATHER_ROW_OFFSET_PX;
            else
            {
                int p = metro_ease(METRO_EASE_OUT_QUAD, local_frame + 1, local_total);
                y_offsets[row] = METRO_FEATHER_ROW_OFFSET_PX -
                                  (METRO_FEATHER_ROW_OFFSET_PX * p) / 256;
            }
        }

        metro_draw_clear_rows_area();
        metro_draw_rows_ex(pivot, first, sel, 0, y_offsets);
        lcd_update();

        if (button_queue_full())
            button_clear_queue();
        if (frame < METRO_FEATHER_FRAMES - 1)
            sleep(3);
    }
}
