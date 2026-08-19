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
#include <string.h>

#include "dir.h"
#include "string-extra.h"
#include "strnatcmp.h"

#include "metro_fsutil.h"

/* Generous upper bound on how many matching files a single directory
 * scan considers before sorting/truncating to the caller's `max` --
 * see metro_fsutil.h. Static, not stack (same D-226-class concern as
 * metro_music.c's own scratch buffers): 4096 * 96 bytes = 384KB,
 * trivial against 64MB RAM. */
#define METRO_FSUTIL_SCAN_CEILING 4096

static char s_scan[METRO_FSUTIL_SCAN_CEILING][METRO_FSUTIL_NAME_LEN];

static bool matches_any_ext(const char *name, const char *const *exts, int n_exts)
{
    size_t len = strlen(name);
    int i;

    for (i = 0; i < n_exts; i++)
    {
        size_t elen = strlen(exts[i]);
        if (len > elen && !strcasecmp(name + len - elen, exts[i]))
            return true;
    }
    return false;
}

static void sort_natural(char names[][METRO_FSUTIL_NAME_LEN], int n)
{
    int a, b;

    for (a = 1; a < n; a++)
    {
        char key[METRO_FSUTIL_NAME_LEN];
        strlcpy(key, names[a], sizeof(key));
        b = a - 1;
        while (b >= 0 && strnatcasecmp(names[b], key) > 0)
        {
            strlcpy(names[b + 1], names[b], METRO_FSUTIL_NAME_LEN);
            b--;
        }
        strlcpy(names[b + 1], key, METRO_FSUTIL_NAME_LEN);
    }
}

int metro_fsutil_list_by_ext(const char *dir, const char *const *exts, int n_exts,
                              char out[][METRO_FSUTIL_NAME_LEN], int max)
{
    DIR *d;
    struct DIRENT *entry;
    int n = 0, i;

    d = opendir(dir);
    if (!d)
        return 0;

    while (n < METRO_FSUTIL_SCAN_CEILING && (entry = readdir(d)) != NULL)
    {
        if (!matches_any_ext(entry->d_name, exts, n_exts))
            continue;
        strlcpy(s_scan[n], entry->d_name, METRO_FSUTIL_NAME_LEN);
        n++;
    }
    closedir(d);

    sort_natural(s_scan, n);

    if (n > max)
        n = max;
    for (i = 0; i < n; i++)
        strlcpy(out[i], s_scan[i], METRO_FSUTIL_NAME_LEN);

    return n;
}
