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
/* moonlit (D-074): ver moonlit_textseg.h. */
#include <stdint.h>
#include <string.h>

#include "moonlit_textseg.h"
#include "moonlit_translit.h"
#include "moonlit_punct_table.h"

/* Decodifica UNA secuencia UTF-8 en `s`. Devuelve cuantos bytes ocupa
 * (1..4) y deja el codepoint en *out_cp. Una secuencia mal formada
 * devuelve 1 byte y el codepoint 0 -- el llamador la copia tal cual, no
 * la sanea. Copia deliberada de la misma funcion en moonlit_translit.c
 * (ambos son modulos puros pequenos, sin dependerse entre si a
 * proposito) en vez de compartirla. */
static int decode_utf8(const char *s, uint32_t *out_cp)
{
    unsigned char b0 = (unsigned char)s[0];
    int n, i;
    uint32_t cp;

    if (b0 < 0x80)
    {
        *out_cp = b0;
        return 1;
    }
    if ((b0 & 0xE0) == 0xC0) { n = 2; cp = b0 & 0x1F; }
    else if ((b0 & 0xF0) == 0xE0) { n = 3; cp = b0 & 0x0F; }
    else if ((b0 & 0xF8) == 0xF0) { n = 4; cp = b0 & 0x07; }
    else { *out_cp = 0; return 1; }

    for (i = 1; i < n; i++)
    {
        unsigned char bi = (unsigned char)s[i];

        if ((bi & 0xC0) != 0x80)
        {
            *out_cp = 0;
            return 1; /* truncada o invalida: un byte, tal cual */
        }
        cp = (cp << 6) | (bi & 0x3F);
    }
    *out_cp = cp;
    return n;
}

int moonlit_textseg_build(const char *in, bool has_punct_font,
                          bool has_cyrillic_font,
                          char *out_buf, size_t out_buf_sz,
                          struct moonlit_textseg *segs, int max_segs)
{
    size_t o = 0, run_start = 0;
    int n = 0;
    enum moonlit_textseg_kind cur_kind = MOONLIT_TEXTSEG_PRIMARY;
    bool have_run = false;

    if (!out_buf || out_buf_sz == 0 || max_segs <= 0)
        return 0;
    if (!in || !in[0])
        return 0;

    /* moonlit (D-081): MFONT_DISPLAY no tiene fuente de puntuacion pero
     * SI tiene cirilica (dibuja nombres de pivote del hub, que en ruso
     * son cirilicos de punta a punta -- "musica"/"настройки") -- el
     * atajo de un solo tramo transliterado solo aplica cuando NINGUNA
     * de las dos fuentes aparte existe. */
    if (!has_punct_font && !has_cyrillic_font)
    {
        moonlit_translit(in, out_buf, out_buf_sz);
        segs[0].kind = MOONLIT_TEXTSEG_PRIMARY;
        segs[0].text = out_buf;
        return 1;
    }

    while (*in)
    {
        uint32_t cp;
        int len = decode_utf8(in, &cp);
        enum moonlit_textseg_kind kind;
        const char *bytes = in;
        size_t blen = (size_t)len;

        if (cp >= 32 && cp <= 383)
        {
            kind = MOONLIT_TEXTSEG_PRIMARY;
        }
        else if (has_cyrillic_font && cp >= MOONLIT_TEXTSEG_CYRILLIC_START &&
                 cp <= MOONLIT_TEXTSEG_CYRILLIC_LIMIT)
        {
            /* moonlit (D-081): sin transliterar -- no hay ASCII
             * razonable para una letra rusa, a diferencia de la
             * puntuacion tipografica (D-066). */
            kind = MOONLIT_TEXTSEG_CYRILLIC;
        }
        else if (has_punct_font && moonlit_punct_table_has(cp))
        {
            /* Sin transliterar -- es justo lo que la fuente de
             * puntuacion del rol dibuja de verdad (D-074). */
            kind = MOONLIT_TEXTSEG_PUNCT;
        }
        else
        {
            const char *rep = cp ? moonlit_translit_lookup(cp) : NULL;

            kind = MOONLIT_TEXTSEG_PRIMARY;
            if (rep)
            {
                bytes = rep;
                blen = strlen(rep);
            }
            /* sin reemplazo: bytes/blen se quedan en los originales --
             * el defaultchar del rol primario ('·', D-066) resuelve lo
             * que ni transliteracion ni fuente de puntuacion cubren. */
        }

        if (kind != cur_kind && have_run)
        {
            if (o >= out_buf_sz || n >= max_segs)
                break; /* sin espacio para cerrar el tramo -- truncar aqui */
            out_buf[o++] = '\0';
            segs[n].kind = cur_kind;
            segs[n].text = out_buf + run_start;
            n++;
            have_run = false;
        }
        if (!have_run)
        {
            if (n >= max_segs)
                break; /* no habria donde guardar este tramo al cerrarlo */
            run_start = o;
            cur_kind = kind;
            have_run = true;
        }

        if (o + blen >= out_buf_sz)
            break; /* no cabe: lo ya escrito se cierra abajo */
        memcpy(out_buf + o, bytes, blen);
        o += blen;

        in += len;
    }

    if (have_run && o < out_buf_sz && n < max_segs)
    {
        out_buf[o++] = '\0';
        segs[n].kind = cur_kind;
        segs[n].text = out_buf + run_start;
        n++;
    }

    return n;
}
