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
#include "metro_sync_marker.h"

#include <string.h>
#include <stdio.h>

void metro_sync_marker_init(metro_sync_marker_t *out)
{
    out->version = -1;
    out->timestamp[0] = '\0';
    out->music = false;
    out->video = false;
    out->images = false;
    out->attempts = 0;
}

static const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    return p;
}

/* Looks for `"key"` followed (after whitespace) by ':' and returns a
 * pointer to the value's first character, or NULL. Only counts
 * occurrences that are really a key (closing quote + ':'), not a
 * string value that happens to contain the word. */
static const char *find_value(const char *text, const char *key)
{
    size_t klen = strlen(key);
    const char *p = text;

    while ((p = strchr(p, '"')) != NULL)
    {
        const char *q;
        p++;
        if (strncmp(p, key, klen) != 0 || p[klen] != '"')
        {
            /* skip to this string's closing quote */
            q = strchr(p, '"');
            if (!q)
                return NULL;
            p = q + 1;
            continue;
        }
        q = skip_ws(p + klen + 1);
        if (*q != ':')
        {
            p = p + klen + 1;
            continue;
        }
        return skip_ws(q + 1);
    }
    return NULL;
}

/* -1 if there's no non-negative integer there. */
static int parse_int(const char *v)
{
    int n = 0;
    if (*v < '0' || *v > '9')
        return -1;
    while (*v >= '0' && *v <= '9')
    {
        if (n > 100000)
            return -1;
        n = n * 10 + (*v - '0');
        v++;
    }
    return n;
}

/* 1/0 for true/false, -1 if it's not a boolean. */
static int parse_bool(const char *v)
{
    if (!strncmp(v, "true", 4))
        return 1;
    if (!strncmp(v, "false", 5))
        return 0;
    return -1;
}

static void parse_string(const char *v, char *out, size_t outsz)
{
    size_t n = 0;
    out[0] = '\0';
    if (*v != '"')
        return;
    v++;
    while (*v && *v != '"' && n + 1 < outsz)
    {
        if (*v == '\\' && v[1])
            v++; /* simple escape: the escaped character is copied as-is */
        out[n++] = *v++;
    }
    out[n] = '\0';
}

metro_sync_marker_status_t metro_sync_marker_parse(const char *text,
                                                    metro_sync_marker_t *out)
{
    const char *v;
    int n;

    metro_sync_marker_init(out);
    if (!text)
        return METRO_SYNC_MARKER_MALFORMED;

    v = skip_ws(text);
    if (*v != '{')
        return METRO_SYNC_MARKER_MALFORMED;

    v = find_value(text, "version");
    if (!v || (n = parse_int(v)) < 0)
    {
        metro_sync_marker_init(out);
        return METRO_SYNC_MARKER_MISSING_VERSION;
    }
    out->version = n;

    v = find_value(text, "timestamp");
    if (v)
        parse_string(v, out->timestamp, sizeof(out->timestamp));

    /* The three section keys are unique across the whole document
     * (they live inside "changes"), so a flat search is enough and
     * also tolerates a future Studio moving them up a level. */
    v = find_value(text, "music");
    out->music = v && parse_bool(v) == 1;
    v = find_value(text, "video");
    out->video = v && parse_bool(v) == 1;
    v = find_value(text, "images");
    out->images = v && parse_bool(v) == 1;

    v = find_value(text, "attempts");
    if (v && (n = parse_int(v)) >= 0)
        out->attempts = n;

    if (out->version > METRO_SYNC_MARKER_VERSION_SUPPORTED)
        return METRO_SYNC_MARKER_UNSUPPORTED;

    return METRO_SYNC_MARKER_OK;
}

int metro_sync_marker_serialize(const metro_sync_marker_t *m,
                                 char *buf, size_t bufsize)
{
    int n = snprintf(buf, bufsize,
                     "{\n"
                     "  \"version\": %d,\n"
                     "  \"timestamp\": \"%s\",\n"
                     "  \"changes\": {\n"
                     "    \"music\": %s,\n"
                     "    \"video\": %s,\n"
                     "    \"images\": %s\n"
                     "  },\n"
                     "  \"attempts\": %d\n"
                     "}\n",
                     m->version < 0 ? METRO_SYNC_MARKER_VERSION_SUPPORTED : m->version,
                     m->timestamp,
                     m->music ? "true" : "false",
                     m->video ? "true" : "false",
                     m->images ? "true" : "false",
                     m->attempts < 0 ? 0 : m->attempts);
    if (n < 0 || (size_t)n >= bufsize)
        return -1;
    return n;
}

bool metro_sync_marker_has_work(const metro_sync_marker_t *m)
{
    return m->music || m->video || m->images;
}
