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
/* Music library infra: wraps tagcache (apps/tagcache.h) and playlist
 * (apps/playlist.h) -- the screens layer (metro_screen_hub.c) only
 * decides what to query and when, same split as Aura-Firmware's
 * aura_music.c/.h (PLAN_MAESTRO.md S1.1 point 1, S5 F4). Deliberately
 * narrower than Aura's at F4: no composers, no ratings import, no
 * album art precache. Ratings/artist photos were explicitly out of
 * F4's plan, not out of Metro's forever -- both are backlog items
 * ronda 3 picks up: artist photos here (R3-F3/DD-6), ratings import
 * in metro_sync.c (R3-F5/DD-7). Composers remain out of scope.
 */
#ifndef METRO_MUSIC_H
#define METRO_MUSIC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define METRO_MUSIC_ITEM_LEN      64
#define METRO_MUSIC_SUBTITLE_LEN  40
#define METRO_MUSIC_MAX_ITEMS     300

typedef struct {
    char label[METRO_MUSIC_ITEM_LEN];
    /* Empty if the row has none. Songs: track duration ("3:24").
     * Albums: artist/albumartist label. Never both -- which one a row
     * carries depends only on which list produced it. */
    char subtitle[METRO_MUSIC_SUBTITLE_LEN];
    /* tagcache seek: result_seek for group tags (artist/album/genre),
     * idx_id for tag_title (a specific track). */
    int32_t seek;
} metro_music_item_t;

/* True once the database can be searched. Triggers the one-time
 * initial build (tagcache_rebuild()) the first time it is called after
 * tagcache confirms there is no usable database on disk -- Rockbox
 * never does this on its own without the folder-browser "Database >
 * Initialize now" screen, which Metro doesn't have (same gap as
 * Aura-Firmware, D-021). Cheap to call on every attempt to enter
 * Music; the caller (metro_screen_hub.c) pushes a plain "updating
 * library..." placeholder page while this returns false. */
bool metro_music_db_ready(void);

bool metro_music_is_playing(void);

/* Current track's visible title/subtitle ("<artist> - <album>"),
 * filled only if metro_music_is_playing(). Returns false (buffers left
 * untouched) otherwise. */
bool metro_music_now_playing(char *title_out, size_t title_sz,
                              char *sub_out, size_t sub_sz);

int metro_music_artists(metro_music_item_t *out, int max);
int metro_music_albums(metro_music_item_t *out, int max);
int metro_music_songs(metro_music_item_t *out, int max);
int metro_music_genres(metro_music_item_t *out, int max);

int metro_music_albums_of_artist(int32_t artist_seek,
                                  metro_music_item_t *out, int max);
int metro_music_songs_of_album(int32_t album_seek,
                                metro_music_item_t *out, int max);
int metro_music_songs_of_genre(int32_t genre_seek,
                                metro_music_item_t *out, int max);

/* Same order as the matching list above (alphabetical, or by track
 * number for an album) -- the row index the user selected on screen is
 * always the same track that starts playing. */
bool metro_music_play_all_songs(int start_index);
bool metro_music_play_songs_of_album(int32_t album_seek, int start_index);
bool metro_music_play_songs_of_genre(int32_t genre_seek, int start_index);
bool metro_music_shuffle_all(void);

int metro_music_list_playlists(char labels[][METRO_MUSIC_ITEM_LEN], int max);
bool metro_music_play_playlist(int index);
void metro_music_playlist_display_name(const char *filename, char *out,
                                        size_t outsz);

/* R3-F3/DD-6 (M-064): artist_images.cfg, Studio's index mapping an
 * artist tag to a cached photo filename under artists/
 * (CONTRATO-firmware-studio.md §D.3), plus a scan of artists/ itself
 * for each listed file's current mtime (metro_thumbs.c's cache
 * invalidation key, DD-1 -- same "<name>.<mtime>" scheme as photos and
 * quickplay). Re-parses/re-scans fresh from disk -- same "refresh on
 * enter" rule metro_photos_list() and the rest of this file's lists
 * already follow (DESVIACIONES.md F6-1) -- call once when the Music
 * page is entered, before any artist row/tile is drawn. */
void metro_music_reload_artist_images(void);

/* Resolves `artist_tag` (an exact match against the raw tag string,
 * same one metro_music_artists() already exposes as
 * metro_music_item_t.label) to its image: writes the filename
 * (relative to artists/, NOT a full path) into filename_out and its
 * current mtime into *mtime_out. Returns false if artist_images.cfg
 * has no entry for this tag, OR the file it names doesn't exist on
 * disk right now ("archivo referenciado ausente -> placeholder, no
 * error", B.1) -- either way the caller falls back to the accent-tile
 * placeholder, same as metro_thumbs_get() returning NULL for any other
 * reason. */
bool metro_music_artist_image(const char *artist_tag, char *filename_out,
                               size_t filename_sz, long *mtime_out);

#endif /* METRO_MUSIC_H */
