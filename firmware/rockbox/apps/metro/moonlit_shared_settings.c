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
#include "moonlit_shared_settings.h"

#include <string.h>
#include <stdio.h>

void moonlit_shared_settings_init(moonlit_shared_settings_t *out)
{
    memset(out, 0, sizeof(*out));
}

/* --- lectura linea por linea, sin tocar disco (el buffer ya esta en RAM) */

static size_t line_len(const char *p)
{
    const char *nl = strchr(p, '\n');
    size_t n = nl ? (size_t)(nl - p) : strlen(p);
    /* CRLF tolerante -- Studio corre en macOS/Windows, un editor
     * cualquiera podria dejar \r\n. */
    if (n > 0 && p[n - 1] == '\r')
        n--;
    return n;
}

static const char *next_line(const char *p)
{
    const char *nl = strchr(p, '\n');
    return nl ? nl + 1 : NULL;
}

/* Divide "clave: valor" (la clave nunca lleva espacios). *key_len y
 * *val queda apuntando dentro de la misma linea -- val_len es hasta
 * el final de la linea (line_len ya recorto el \r final). false si no
 * hay ':' en la linea. */
static bool split_kv(const char *line, size_t line_n, size_t *key_len,
                      const char **val, size_t *val_len)
{
    const char *colon = memchr(line, ':', line_n);
    const char *v;
    size_t vn;

    if (!colon)
        return false;
    *key_len = (size_t)(colon - line);

    v = colon + 1;
    vn = line_n - (size_t)(v - line);
    while (vn > 0 && *v == ' ') { v++; vn--; }
    *val = v;
    *val_len = vn;
    return true;
}

static bool key_is(const char *line, size_t key_len, const char *name)
{
    size_t n = strlen(name);
    return key_len == n && !memcmp(line, name, n);
}

static void copy_str(const char *v, size_t vn, char *out, size_t outsz)
{
    if (vn >= outsz)
        vn = outsz - 1;
    memcpy(out, v, vn);
    out[vn] = '\0';
}

/* -1 si no hay ni un digito (con signo opcional) en [v, v+vn). */
static bool parse_long(const char *v, size_t vn, long *out)
{
    long sign = 1, n = 0;
    size_t i = 0;

    if (vn > 0 && (v[0] == '-' || v[0] == '+'))
    {
        sign = (v[0] == '-') ? -1 : 1;
        i = 1;
    }
    if (i >= vn)
        return false;
    for (; i < vn; i++)
    {
        if (v[i] < '0' || v[i] > '9')
            return false;
        n = n * 10 + (v[i] - '0');
    }
    *out = sign * n;
    return true;
}

/* Estricto: SOLO "0" o "1" -- el contrato no admite otra forma para
 * las claves booleanas (screen_lock_enabled, keyclick). */
static bool parse_bool01(const char *v, size_t vn, bool *out)
{
    if (vn != 1)
        return false;
    if (v[0] == '0') { *out = false; return true; }
    if (v[0] == '1') { *out = true;  return true; }
    return false;
}

static void append_unknown(moonlit_shared_settings_t *out, const char *line, size_t line_n)
{
    size_t used = strlen(out->unknown_lines);
    /* +1 por el '\n' que se agrega, +1 por el NUL final. */
    size_t room = sizeof(out->unknown_lines) - used;

    if (room < line_n + 2)
        return; /* no cupo -- se recorta en silencio, D-079 lo documenta */
    memcpy(out->unknown_lines + used, line, line_n);
    out->unknown_lines[used + line_n] = '\n';
    out->unknown_lines[used + line_n + 1] = '\0';
}

bool moonlit_shared_settings_parse(const char *text, moonlit_shared_settings_t *out)
{
    const char *p = text;
    size_t first_len;

    moonlit_shared_settings_init(out);
    if (!text)
        return false;

    first_len = line_len(p);
    if (first_len != strlen(MOONLIT_SHARED_SETTINGS_HEADER) ||
        memcmp(p, MOONLIT_SHARED_SETTINGS_HEADER, first_len) != 0)
        return false;

    p = next_line(p);
    while (p && *p)
    {
        size_t line_n = line_len(p);
        size_t key_len;
        const char *val;
        size_t val_len;
        long n;
        bool b;

        if (line_n == 0 || !split_kv(p, line_n, &key_len, &val, &val_len))
        {
            p = next_line(p);
            continue;
        }

        if (key_is(p, key_len, "rev"))
        {
            if (parse_long(val, val_len, &n)) { out->have_rev = true; out->rev = n; }
        }
        else if (key_is(p, key_len, "updated_by"))
        {
            if (val_len > 0 && val_len < sizeof(out->updated_by))
            {
                out->have_updated_by = true;
                copy_str(val, val_len, out->updated_by, sizeof(out->updated_by));
            }
        }
        else if (key_is(p, key_len, "screen_lock_enabled"))
        {
            if (parse_bool01(val, val_len, &b)) { out->have_screen_lock_enabled = true; out->screen_lock_enabled = b; }
        }
        else if (key_is(p, key_len, "screen_lock_pin"))
        {
            /* Vacio es valido (D-079: "4 digitos o vacio"); 4 digitos
             * es lo unico mas ademas de vacio, pero eso lo valida
             * metro_screen_lock.c como siempre -- aqui solo hace
             * falta que quepa. */
            if (val_len < sizeof(out->screen_lock_pin))
            {
                out->have_screen_lock_pin = true;
                copy_str(val, val_len, out->screen_lock_pin, sizeof(out->screen_lock_pin));
            }
        }
        else if (key_is(p, key_len, "screen_lock_require"))
        {
            if (val_len > 0 && val_len < sizeof(out->screen_lock_require))
            {
                out->have_screen_lock_require = true;
                copy_str(val, val_len, out->screen_lock_require, sizeof(out->screen_lock_require));
            }
        }
        else if (key_is(p, key_len, "brightness"))
        {
            if (parse_long(val, val_len, &n)) { out->have_brightness = true; out->brightness = n; }
        }
        else if (key_is(p, key_len, "backlight_timeout"))
        {
            if (parse_long(val, val_len, &n)) { out->have_backlight_timeout = true; out->backlight_timeout = n; }
        }
        else if (key_is(p, key_len, "idle_poweroff"))
        {
            if (parse_long(val, val_len, &n)) { out->have_idle_poweroff = true; out->idle_poweroff = n; }
        }
        else if (key_is(p, key_len, "keyclick"))
        {
            if (parse_bool01(val, val_len, &b)) { out->have_keyclick = true; out->keyclick = b; }
        }
        else if (key_is(p, key_len, "volume_limit"))
        {
            if (parse_long(val, val_len, &n)) { out->have_volume_limit = true; out->volume_limit = n; }
        }
        else if (key_is(p, key_len, "replaygain"))
        {
            if (val_len > 0 && val_len < sizeof(out->replaygain))
            {
                out->have_replaygain = true;
                copy_str(val, val_len, out->replaygain, sizeof(out->replaygain));
            }
        }
        else if (key_is(p, key_len, "language"))
        {
            if (val_len > 0 && val_len < sizeof(out->language))
            {
                out->have_language = true;
                copy_str(val, val_len, out->language, sizeof(out->language));
            }
        }
        else if (key_is(p, key_len, "appearance"))
        {
            if (val_len > 0 && val_len < sizeof(out->appearance))
            {
                out->have_appearance = true;
                copy_str(val, val_len, out->appearance, sizeof(out->appearance));
            }
        }
        else
        {
            append_unknown(out, p, line_n);
        }

        p = next_line(p);
    }

    return true;
}

bool moonlit_shared_settings_int_in_range(long v, long lo, long hi)
{
    return v >= lo && v <= hi;
}

int moonlit_shared_settings_serialize(const moonlit_shared_settings_t *in,
                                       char *buf, size_t bufsize)
{
    size_t n = 0;
    int w;

#define APPEND(...) do { \
        if (n >= bufsize) return -1; \
        w = snprintf(buf + n, bufsize - n, __VA_ARGS__); \
        if (w < 0 || (size_t)w >= bufsize - n) return -1; \
        n += (size_t)w; \
    } while (0)

    APPEND("%s\n", MOONLIT_SHARED_SETTINGS_HEADER);
    if (in->have_rev)                 APPEND("rev: %ld\n", in->rev);
    if (in->have_updated_by)          APPEND("updated_by: %s\n", in->updated_by);
    if (in->have_screen_lock_enabled) APPEND("screen_lock_enabled: %d\n", in->screen_lock_enabled ? 1 : 0);
    if (in->have_screen_lock_pin)     APPEND("screen_lock_pin: %s\n", in->screen_lock_pin);
    if (in->have_screen_lock_require) APPEND("screen_lock_require: %s\n", in->screen_lock_require);
    if (in->have_brightness)          APPEND("brightness: %ld\n", in->brightness);
    if (in->have_backlight_timeout)   APPEND("backlight_timeout: %ld\n", in->backlight_timeout);
    if (in->have_idle_poweroff)       APPEND("idle_poweroff: %ld\n", in->idle_poweroff);
    if (in->have_keyclick)            APPEND("keyclick: %d\n", in->keyclick ? 1 : 0);
    if (in->have_volume_limit)        APPEND("volume_limit: %ld\n", in->volume_limit);
    if (in->have_replaygain)          APPEND("replaygain: %s\n", in->replaygain);
    if (in->have_language)            APPEND("language: %s\n", in->language);
    if (in->have_appearance)          APPEND("appearance: %s\n", in->appearance);

    if (in->unknown_lines[0])
    {
        size_t un = strlen(in->unknown_lines);
        if (n + un >= bufsize)
            return -1;
        memcpy(buf + n, in->unknown_lines, un);
        n += un;
    }

#undef APPEND

    if (n >= bufsize)
        return -1;
    buf[n] = '\0';
    return (int)n;
}

/* --- mapas palabra<->entero, puros -------------------------------------- */

int moonlit_shared_settings_lock_require_from_str(const char *s)
{
    if (!strcmp(s, "hold")) return 0; /* METRO_LOCK_REQUIRE_HOLD */
    if (!strcmp(s, "1min")) return 1; /* METRO_LOCK_REQUIRE_1MIN */
    if (!strcmp(s, "5min")) return 2; /* METRO_LOCK_REQUIRE_5MIN */
    if (!strcmp(s, "boot")) return 3; /* METRO_LOCK_REQUIRE_BOOT */
    return -1;
}

const char *moonlit_shared_settings_lock_require_to_str(int v)
{
    switch (v)
    {
    case 0: return "hold";
    case 1: return "1min";
    case 2: return "5min";
    case 3: return "boot";
    default: return NULL;
    }
}

int moonlit_shared_settings_replaygain_from_str(const char *s)
{
    if (!strcmp(s, "off"))   return 0;
    if (!strcmp(s, "track")) return 1;
    if (!strcmp(s, "album")) return 2;
    return -1;
}

const char *moonlit_shared_settings_replaygain_to_str(int v)
{
    switch (v)
    {
    case 0: return "off";
    case 1: return "track";
    case 2: return "album";
    default: return NULL;
    }
}

int moonlit_shared_settings_appearance_from_str(const char *s)
{
    if (!strcmp(s, "dark"))  return 0; /* METRO_THEME_DARK */
    if (!strcmp(s, "light")) return 1; /* METRO_THEME_LIGHT */
    return -1;
}

const char *moonlit_shared_settings_appearance_to_str(int v)
{
    switch (v)
    {
    case 0: return "dark";
    case 1: return "light";
    default: return NULL;
    }
}
