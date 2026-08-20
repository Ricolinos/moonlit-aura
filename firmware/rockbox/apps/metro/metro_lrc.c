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
#include "metro_lrc.h"

#include <string.h>

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool metro_lrc_sibling_path(const char *audio_path, char *out, size_t out_len)
{
    const char *dot = strrchr(audio_path, '.');
    const char *slash = strrchr(audio_path, '/');
    size_t base_len;

    if (!dot || (slash && dot < slash))
        return false; /* no extension to replace */

    base_len = (size_t)(dot - audio_path);
    if (base_len + 4 >= out_len) /* ".lrc" + NUL */
        return false;

    memcpy(out, audio_path, base_len);
    strcpy(out + base_len, ".lrc");
    return true;
}

/* Parses one "mm:ss" or "mm:ss.fff" tag starting right after the '['
 * at `p`. On success, returns a pointer just past the matching ']' and
 * writes the timestamp in ms to *ms_out; on failure (not a timestamp --
 * metadata like "[ar:...]", or malformed) returns NULL and *ms_out is
 * untouched. */
static const char *parse_timestamp(const char *p, uint32_t *ms_out)
{
    unsigned min = 0, sec = 0, frac = 0;
    int frac_digits = 0;
    const char *s = p;

    if (!is_digit(*s))
        return NULL;
    while (is_digit(*s))
    {
        min = min * 10 + (unsigned)(*s - '0');
        s++;
    }

    if (*s != ':')
        return NULL;
    s++;

    if (!is_digit(*s))
        return NULL;
    while (is_digit(*s))
    {
        sec = sec * 10 + (unsigned)(*s - '0');
        s++;
    }

    if (*s == '.')
    {
        s++;
        while (is_digit(*s))
        {
            if (frac_digits < 3)
                frac = frac * 10 + (unsigned)(*s - '0');
            frac_digits++;
            s++;
        }
        if (frac_digits == 0)
            return NULL; /* bare '.' with no digits -- malformed */
    }

    if (*s != ']')
        return NULL;
    s++;

    /* Normalize to milliseconds: 1 digit is tenths-of-second (x100), 2
     * is hundredths (x10), 3+ is already millisecond-scale (extra
     * digits beyond 3 already dropped above -- "se trunca a
     * milisegundos", INVESTIGACION-metro-r3.md A.1). */
    if (frac_digits == 1)
        frac *= 100;
    else if (frac_digits == 2)
        frac *= 10;

    *ms_out = (min * 60u + sec) * 1000u + frac;
    return s;
}

#define MAX_TS_PER_LINE 8

bool metro_lrc_parse(struct metro_lrc *lrc, size_t len)
{
    char *p, *end;

    lrc->count = 0;

    if (len >= METRO_LRC_BUF_SIZE)
        len = METRO_LRC_BUF_SIZE - 1;
    lrc->buf[len] = '\0';

    p = lrc->buf;
    end = lrc->buf + len;

    while (p < end && lrc->count < METRO_LRC_MAX_LINES)
    {
        char *line = p;
        char *nl = memchr(p, '\n', (size_t)(end - p));
        char *line_end = nl ? nl : end;
        char *cur;
        uint32_t timestamps[MAX_TS_PER_LINE];
        int nts = 0;

        *line_end = '\0';
        if (line_end > line && line_end[-1] == '\r')
            line_end[-1] = '\0';

        /* Leading run of "[...]" tags: a valid "[mm:ss(.fff)]" adds a
         * timestamp; anything else between '[' and ']' (metadata like
         * "[ar:...]", or a malformed tag) is skipped. The scan starts
         * strictly at the line's first byte -- a line that doesn't
         * open with '[' (a stray BOM, plain text) yields zero
         * timestamps and is dropped whole, same as Aura's own parser. */
        cur = line;
        while (cur < line_end && *cur == '[')
        {
            uint32_t ms;
            const char *after = parse_timestamp(cur + 1, &ms);

            if (after)
            {
                if (nts < MAX_TS_PER_LINE)
                    timestamps[nts++] = ms;
                cur = (char *)after;
            }
            else
            {
                char *close = memchr(cur, ']', (size_t)(line_end - cur));
                if (!close)
                    break; /* unterminated tag -- stop scanning this line */
                cur = close + 1;
            }
        }

        if (nts > 0)
        {
            uint16_t text_offset = (uint16_t)(cur - lrc->buf);
            int i;
            for (i = 0; i < nts && lrc->count < METRO_LRC_MAX_LINES; i++)
            {
                lrc->entries[lrc->count].ms = timestamps[i];
                lrc->entries[lrc->count].offset = text_offset;
                lrc->count++;
            }
        }
        /* else: no valid timestamp on this line -- dropped. */

        p = nl ? nl + 1 : end;
    }

    return lrc->count > 0;
}

int metro_lrc_find_active(const struct metro_lrc *lrc, uint32_t elapsed_ms)
{
    int lo = 0, hi = lrc->count - 1, best = -1;

    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;
        if (lrc->entries[mid].ms <= elapsed_ms)
        {
            best = mid;
            lo = mid + 1;
        }
        else
        {
            hi = mid - 1;
        }
    }
    return best;
}

const char *metro_lrc_text(const struct metro_lrc *lrc, int index)
{
    if (index < 0 || index >= lrc->count)
        return NULL;
    return lrc->buf + lrc->entries[index].offset;
}
