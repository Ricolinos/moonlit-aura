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
#include <string.h>
#include "kernel.h"
#include "misc.h"
#include "lcd.h"

#include "metro_widgets.h"
#include "moonlit_icons.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_input.h"
#include "metro_lang.h"
#include "metro_fb.h"
#include "moonlit_fonts.h"

#define CONFIRM_QUESTION_X 12
#define CONFIRM_QUESTION_Y 90
#define CONFIRM_YES_Y      150

/* moonlit (D-047, ported from Metro M-093): the question used to be one line at MFONT_TITLE, which held
 * "¿cambiar a Aura y reiniciar?" but not "¿cambiar a moonlit.aura y
 * reiniciar?". When it does not fit, break it at the last space that
 * does and draw two lines, centred on the single-line baseline so the
 * block still ends above "sí"/"no". Never more than two lines: every
 * question in the catalogue fits in two at 320 px, and a third would
 * run into the answers. */
static void draw_question_at(const char *question, int qy)
{
    static char head[96];
    int w, h, max_w = LCD_WIDTH - 2 * CONFIRM_QUESTION_X;
    const char *tail;
    size_t cut;

    lcd_setfont(metro_font_id(MFONT_TITLE));
    lcd_getstringsize((const unsigned char *)question, &w, &h);
    if (w <= max_w)
    {
        metro_draw_text(MFONT_TITLE, CONFIRM_QUESTION_X, qy,
                         question, metro_color_fg());
        return;
    }

    /* Longest head ending at a space that fits. */
    cut = 0;
    for (tail = question; (tail = strchr(tail, ' ')) != NULL; tail++)
    {
        size_t n = (size_t)(tail - question);
        if (n >= sizeof(head))
            break;
        memcpy(head, question, n);
        head[n] = '\0';
        lcd_getstringsize((const unsigned char *)head, &w, NULL);
        if (w > max_w)
            break;
        cut = n;
    }
    if (cut == 0)
    {
        /* No usable space: let the LCD clip it, as before. */
        metro_draw_text(MFONT_TITLE, CONFIRM_QUESTION_X, qy,
                         question, metro_color_fg());
        return;
    }
    memcpy(head, question, cut);
    head[cut] = '\0';
    metro_draw_text(MFONT_TITLE, CONFIRM_QUESTION_X, qy - h / 2,
                     head, metro_color_fg());
    metro_draw_text(MFONT_TITLE, CONFIRM_QUESTION_X, qy + h / 2,
                     question + cut + 1, metro_color_fg());
}

/* D-061: linea de detalle bajo la pregunta, en MFONT_BODY.
 * draw_question() esta topada en DOS lineas a proposito (una tercera
 * choca con "si"/"no"), asi que una advertencia larga -- "puede tardar
 * varios minutos, segun cuantos archivos tengas y como este el disco" --
 * no cabe ahi sin mutilarla. Va debajo, en tipografia menor: la
 * jerarquia habitual (titulo + cuerpo). */
/* Tres lineas: con la pregunta en 62 (dos lineas de Baskerville 28,
 * hasta ~104) y "sí" en 178, entran tres de MFONT_BODY (18 px) desde
 * 112 -- la tercera termina en ~166. Con dos se perdia la ultima
 * palabra de la advertencia. */
#define CONFIRM_DETAIL_LINES  3

static void draw_detail_at(const char *detail, int dy)
{
    static char line[128];
    int max_w = LCD_WIDTH - 2 * CONFIRM_QUESTION_X;
    const char *p = detail;
    int drawn = 0;
    int y = dy;
    int lh;

    lcd_setfont(metro_font_id(MFONT_BODY));
    lcd_getstringsize((const unsigned char *)"Ag", NULL, &lh);

    while (*p && drawn < CONFIRM_DETAIL_LINES)
    {
        const char *tail;
        size_t cut = 0, len;
        int w;

        /* Si lo que queda entra entero, va entero -- sin esto la ULTIMA
         * linea se cortaria igual en su ultimo espacio y la palabra final
         * se perderia. */
        len = strlen(p);
        if (len < sizeof(line))
        {
            memcpy(line, p, len);
            line[len] = '\0';
            lcd_getstringsize((const unsigned char *)line, &w, NULL);
            if (w <= max_w)
            {
                metro_draw_text(MFONT_BODY, CONFIRM_QUESTION_X, y, line,
                                 metro_color_secondary());
                return;
            }
        }

        for (tail = p; (tail = strchr(tail, ' ')) != NULL; tail++)
        {
            size_t n = (size_t)(tail - p);

            if (n >= sizeof(line))
                break;
            memcpy(line, p, n);
            line[n] = '\0';
            lcd_getstringsize((const unsigned char *)line, &w, NULL);
            if (w > max_w)
                break;
            cut = n;
        }
        if (cut == 0)
        {
            len = strlen(p);
            if (len >= sizeof(line))
                len = sizeof(line) - 1;
            memcpy(line, p, len);
            line[len] = '\0';
            metro_draw_text(MFONT_BODY, CONFIRM_QUESTION_X, y, line,
                             metro_color_secondary());
            return;
        }
        memcpy(line, p, cut);
        line[cut] = '\0';
        metro_draw_text(MFONT_BODY, CONFIRM_QUESTION_X, y, line,
                         metro_color_secondary());
        p += cut + 1;
        y += lh;
        drawn++;
    }
}

bool metro_widgets_confirm(const char *title, const char *question)
{
    return metro_widgets_confirm_detail(title, question, NULL);
}

bool metro_widgets_confirm_detail(const char *title, const char *question,
                                   const char *detail)
{
    bool sel_yes = false; /* default to "no" -- the safe answer */
    /* D-061: con detalle hace falta reacomodar TODO el bloque, no solo
     * meter una linea. La tipografia de moonlit es grande (Baskerville
     * 28 px en la pregunta): "¿actualizar biblioteca ahora?" ya ocupa
     * dos lineas por si sola y termina donde empezaria el detalle. Con
     * los valores de siempre el detalle se encimaba sobre la segunda
     * linea de la pregunta y sobre "sí". Sin detalle, las posiciones no
     * cambian ni un pixel -- ningun otro dialogo se mueve. */
    const int qy  = detail ? 62  : CONFIRM_QUESTION_Y;
    const int dy  = 112;
    const int yy  = detail ? 178 : CONFIRM_YES_Y;
    const int ny  = detail ? 206 : 178;

    while (1)
    {
        int action;

        metro_draw_clear();
        metro_draw_header(title);
        draw_question_at(question, qy);
        if (detail)
            draw_detail_at(detail, dy);
        metro_draw_text(sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, yy,
                         metro_lang_str(LANG_DIALOG_YES),
                         sel_yes ? metro_color_fg() : metro_color_secondary());
        metro_draw_text(!sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, ny,
                         metro_lang_str(LANG_DIALOG_NO),
                         !sel_yes ? metro_color_fg() : metro_color_secondary());
        lcd_update();

        action = metro_input_next(MCTX_DIALOG, HZ / 10, NULL);

        if (action & SYS_EVENT)
        {
            default_event_handler(action);
            continue;
        }

        switch (action)
        {
            case MACT_PREV:
            case MACT_NEXT:
                sel_yes = !sel_yes;
                break;
            case MACT_SELECT:
                return sel_yes;
            case MACT_BACK:
                return false;
            default:
                break;
        }
    }
}

#define METRO_INDEX_LETTER_SIZE 80

void metro_widgets_draw_index_letter(const char *letter)
{
    int x = (LCD_WIDTH - METRO_INDEX_LETTER_SIZE) / 2;
    int y = (LCD_HEIGHT - METRO_INDEX_LETTER_SIZE) / 2;

    /* R4/FA-5a (M-076): recibe una CADENA, no un char -- la inicial
     * puede ser un carácter UTF-8 de varios bytes. metro_draw_tile()
     * ya toma el primer carácter completo de lo que se le pase. */
    metro_draw_tile(x, y, METRO_INDEX_LETTER_SIZE, letter);
}

#define METRO_EMPTY_TILE_SIZE 96

void metro_widgets_draw_empty_state(const char *message)
{
    int x = (LCD_WIDTH - METRO_EMPTY_TILE_SIZE) / 2;
    int y = 60;
    int w, h;

    metro_draw_tile(x, y, METRO_EMPTY_TILE_SIZE, " ");

    lcd_setfont(metro_font_id(MFONT_BODY));
    lcd_getstringsize((const unsigned char *)message, &w, &h);
    metro_draw_text(MFONT_BODY, (LCD_WIDTH - w) / 2, y + METRO_EMPTY_TILE_SIZE + 16,
                     message, metro_color_secondary());
}

void metro_widgets_draw_icon(enum moonlit_icon_id id, int x, int y, unsigned color)
{
    moonlit_icon_draw(id, MOONLIT_ICON_SIZE_16, x, y, color);
}

/* Integer sqrt, rounded down. Inputs here are < 2^24 (distances in
 * 8.8 fixed point squared), so the 32-bit loop is enough. */
static unsigned isqrt32(unsigned v)
{
    unsigned res = 0, bit = 1u << 30;

    while (bit > v)
        bit >>= 2;
    while (bit)
    {
        if (v >= res + bit)
        {
            v -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

void metro_widgets_draw_circle(int cx, int cy, int r, unsigned color)
{
    int dx, dy;
    int r256 = r * 256;

    if (r <= 0)
        return;

    for (dy = -r - 1; dy <= r + 1; dy++)
    {
        for (dx = -r - 1; dx <= r + 1; dx++)
        {
            /* distance in 8.8: sqrt((dx^2+dy^2) * 65536) = d * 256 */
            unsigned d256 = isqrt32((unsigned)(dx * dx + dy * dy) << 16);
            int diff = (int)d256 - r256;
            int alpha;

            if (diff < 0)
                diff = -diff;
            /* ~1.5px ring: full within 0.25px of r, gone at 1.25px. A
             * strict 1px ring spreads over two pixel rows at half
             * intensity and reads grey on the panel; this keeps it thin
             * but solid. */
            alpha = 320 - diff;
            if (alpha > 256)
                alpha = 256;
            if (alpha > 0)
                metro_fb_plot_alpha(cx + dx, cy + dy, color, alpha);
        }
    }
}

void metro_widgets_draw_icon_in_circle(enum moonlit_icon_id id, int x, int y,
                                        int r, unsigned ring_color,
                                        unsigned glyph_color)
{
    int cx = x + r, cy = y + r;

    metro_widgets_draw_circle(cx, cy, r, ring_color);
    /* The 16px glyph cell centred on the ring's centre; with r=13 that
     * leaves 5px of air between cell and ring. */
    metro_widgets_draw_icon(id, cx - MOONLIT_ICON_SIZE_16 / 2, cy - MOONLIT_ICON_SIZE_16 / 2,
                            glyph_color);
}

