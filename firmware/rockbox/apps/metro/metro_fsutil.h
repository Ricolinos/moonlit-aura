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
/* Directory scanning shared by metro_video.c/metro_photos.c -- both
 * browse a flat directory (D-192 of Aura-Firmware's contract: /Videos
 * and /Photos never get subfolders) filtered by extension, natural
 * order. Narrower than Aura-Firmware's aura_fsutil.h on purpose: that
 * one is about deleting caches Metro doesn't have (no disk-cached
 * thumbnails, no theme installs in v1) -- this one is just the listing
 * primitive neither metro_video.c nor metro_photos.c should duplicate. */
#ifndef METRO_FSUTIL_H
#define METRO_FSUTIL_H

/* PLAN_MAESTRO.md S1.2: VIDEO_NAME_LEN/PHOTO_NAME_LEN, both 96 bytes
 * (contract: filenames <= 95 bytes UTF-8 including extension, + NUL). */
#define METRO_FSUTIL_NAME_LEN 96

/* Lists `dir`'s entries whose name ends (case-insensitively) in one of
 * `exts` (an array of `n_exts` strings, each including the leading
 * dot, e.g. ".jpg") into `out` (`max` buffers of METRO_FSUTIL_NAME_LEN
 * bytes each), naturally sorted (strnatcasecmp -- "2.jpg" before
 * "10.jpg", case-insensitive) over the WHOLE matching set before
 * truncating to `max` -- the result is always the true first `max` in
 * natural order, never whatever readdir() happened to return first.
 * Does not recurse. Returns the count placed into `out` (0 if `dir`
 * doesn't exist). Matching entries beyond METRO_FSUTIL_SCAN_CEILING
 * are not considered at all (see its own comment) -- irrelevant for
 * the contract's own caps (100 videos, 500 photos). */
int metro_fsutil_list_by_ext(const char *dir, const char *const *exts, int n_exts,
                              char out[][METRO_FSUTIL_NAME_LEN], int max);

#endif /* METRO_FSUTIL_H */
