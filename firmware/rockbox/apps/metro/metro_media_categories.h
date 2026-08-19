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
/* OPTIONAL per-file category index for /Videos and /Photos --
 * Aura-Firmware's CONTRATO-firmware-studio.md S D.2 defines the format
 * (read from that sibling repo, not copied here). /Videos and /Photos
 * on the device stay flat by contract -- this module never reorganizes
 * anything, it only reads an index Aura Studio writes alongside the
 * media itself, `filename: code` per line, same settings_parseline()
 * format as aura.cfg. Total absence of either file is a SUPPORTED
 * case, not an error: every lookup returns *_CAT_NONE and the screens
 * that filter by category just look empty -- same message as "no
 * content of this type yet", honest degradation, no special state to
 * track.
 *
 * Unlike Aura-Firmware's aura_media_categories.c, there is no
 * persistent cross-visit cache here: metro_video.c/metro_photos.c
 * re-scan their directory fresh every time their page is entered
 * (DESVIACIONES.md F6-1), so this index is loaded fresh alongside
 * that scan too -- metro_media_categories_load_video()/_photo() at the
 * start of each rebuild, then metro_media_categories_video_lookup()/
 * _photo_lookup() per file found. */
#ifndef METRO_MEDIA_CATEGORIES_H
#define METRO_MEDIA_CATEGORIES_H

typedef enum {
    METRO_VIDEO_CAT_NONE = 0, /* no entry in the index (or index absent) */
    METRO_VIDEO_CAT_MOVIE,
    METRO_VIDEO_CAT_SERIES,
    METRO_VIDEO_CAT_CLIP,
} metro_video_cat_t;

typedef enum {
    METRO_PHOTO_CAT_NONE = 0,
    METRO_PHOTO_CAT_PHOTO,
    METRO_PHOTO_CAT_IMAGE,
    METRO_PHOTO_CAT_AI,
} metro_photo_cat_t;

/* Reads video_categories.cfg / photo_categories.cfg into memory (a
 * no-op, cheaply, if the file doesn't exist -- every subsequent lookup
 * then returns *_CAT_NONE). Call once before a run of lookups over a
 * freshly-scanned file list. */
void metro_media_categories_load_video(void);
void metro_media_categories_load_photo(void);

/* Category of `filename` (exact name as it appears in /Videos or
 * /Photos, with extension) against whichever index was last loaded. */
metro_video_cat_t metro_media_categories_video_lookup(const char *filename);
metro_photo_cat_t metro_media_categories_photo_lookup(const char *filename);

#endif /* METRO_MEDIA_CATEGORIES_H */
