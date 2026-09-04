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
/* moonlit (D-066): ver moonlit_translit.h para el porque. */
#include "moonlit_translit.h"

/* La tabla. Una linea por entrada, formato `{ 0xNNNN, "..." },` --
 * firmware/tools/check_fonts.py la lee con esa forma exacta.
 *
 * Criterio de que entra: puntuacion que aparece de verdad en metadatos
 * de musica Y tiene un equivalente ASCII honesto. Un simbolo que no lo
 * tiene (corchea, estrella, corazon) NO se inventa: cae en el
 * defaultchar '·'. Traducir ♪ como "b" seria peor que no traducirlo. */
const struct moonlit_translit_entry moonlit_translit_table[] = {
    /* espacio duro -- se ve como un espacio, es un espacio */
    { 0x00A0, " " },

    /* guiones (U+2010..U+2015): todos al guion ASCII */
    { 0x2010, "-" },
    { 0x2011, "-" },
    { 0x2012, "-" },
    { 0x2013, "-" },
    { 0x2014, "-" },
    { 0x2015, "-" },

    /* comillas simples tipograficas y sus variantes */
    { 0x2018, "'" },
    { 0x2019, "'" },
    { 0x201A, "'" },
    { 0x201B, "'" },

    /* comillas dobles tipograficas */
    { 0x201C, "\"" },
    { 0x201D, "\"" },
    { 0x201E, "\"" },
    { 0x201F, "\"" },

    /* vinetas y puntos suspensivos */
    { 0x2022, "-" },
    { 0x2024, "." },
    { 0x2025, ".." },
    { 0x2026, "..." },

    /* primas: se leen como comillas */
    { 0x2032, "'" },
    { 0x2033, "\"" },

    /* angulares simples */
    { 0x2039, "<" },
    { 0x203A, ">" },

    /* marca registrada */
    { 0x2122, "(TM)" },
};

const int moonlit_translit_count =
    (int)(sizeof(moonlit_translit_table) / sizeof(moonlit_translit_table[0]));

bool moonlit_translit_needed(const char *s)
{
    if (!s)
        return false;

    for (; *s; s++)
    {
        unsigned char b = (unsigned char)*s;

        /* 0xC2: U+0080..U+00BF (ahi vive el espacio duro).
         * 0xE2: U+2000..U+2FFF (puntuacion general y simbolos). */
        if (b == 0xC2 || b == 0xE2)
            return true;
    }
    return false;
}

/* Decodifica UNA secuencia UTF-8 en `s`. Devuelve cuantos bytes ocupa
 * (1..4) y deja el codepoint en *out_cp. Una secuencia mal formada
 * devuelve 1 byte y el codepoint 0 -- el llamador la copia tal cual, no
 * la sanea: esta funcion no es un validador de UTF-8. */
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

static const char *lookup(uint32_t cp)
{
    int i;

    for (i = 0; i < moonlit_translit_count; i++)
        if (moonlit_translit_table[i].cp == cp)
            return moonlit_translit_table[i].ascii;
    return NULL;
}

const char *moonlit_translit(const char *in, char *out, size_t outsz)
{
    size_t o = 0;

    if (!out || outsz == 0)
        return out;
    if (!in)
    {
        out[0] = '\0';
        return out;
    }

    while (*in)
    {
        uint32_t cp;
        int n = decode_utf8(in, &cp);
        const char *rep = cp ? lookup(cp) : NULL;
        size_t need;
        size_t k;

        if (rep)
        {
            for (need = 0; rep[need]; need++)
                ;
            if (o + need + 1 > outsz)
                break;
            for (k = 0; k < need; k++)
                out[o++] = rep[k];
        }
        else
        {
            /* La secuencia entera o nada: cortar a mitad de un UTF-8
             * dejaria bytes sueltos que el motor de fuentes dibujaria
             * como basura. */
            if (o + (size_t)n + 1 > outsz)
                break;
            for (k = 0; k < (size_t)n; k++)
                out[o++] = in[k];
        }
        in += n;
    }

    out[o] = '\0';
    return out;
}
