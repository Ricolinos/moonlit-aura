/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2026 Ricardo Gómez
 *
 * Aura UI -- capa de interfaz sobre este fork de Rockbox (ver
 * MODIFICATIONS.md, DECISIONS.md D-001/D-002 en la raíz del repositorio).
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
/* D-042/D-059: UI-thread side of Marea's cover cache -- album ->
 * master path (/.aura/art/albums/<key>.art, moonlit_master_art.h) ->
 * the 120 px cover Marea keeps in RAM (130 -> 120 box downscale +
 * corners baked against moonlit_color(MROLE_SURFACE)). Since D-059
 * there is NO private on-disk L2 anymore (moonlitcache/art/ .pfraw is
 * gone; the master is derived on every load, ~16 900 px of integer
 * box filter -- cheaper than a second 28.8 KB file per album, see
 * DECISIONS.md D-059) and the precache pass moved to the background
 * builder (moonlit_master_art_builder.c). Not host-testable (needs
 * metro_music.h/metro_albumart.h/metro_settings.h), same criterion as
 * metro_thumbs.c; the pure parts live in moonlit_master_art.c. */
#ifndef MOONLIT_ART_CACHE_H
#define MOONLIT_ART_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lcd.h"

/* D-030: tapa central de Marea, 120 px; D-020/plan M7: mismo radio de
 * esquina que design-system/tokens.json shape.corner_s (8). */
#define MOONLIT_ART_CACHE_SIZE   120
#define MOONLIT_ART_CACHE_RADIUS 8

/* /.aura/art/albums/<key>.art for album `seek`, key from
 * metro_music_album_art_key() (D-055: "a-<crc32 ruta>.<mtime>",
 * memoized). false if the album has no resolvable track. Touches
 * tagcache on a memo miss: never from an animation frame -- Marea uses
 * the _peek variant there (D-053/D-057). */
bool moonlit_art_master_path(int32_t seek, char *out, size_t outsz);
bool moonlit_art_master_path_peek(int32_t seek, char *out, size_t outsz);

/* /.aura/art/<subdir>/<prefix>-<crc32 file_path>.<mtime>.art -- the
 * master of an artist photo ('r', "artists") or a /Photos file ('p',
 * "photos"). Pure string work (crc_32 + metro_settings path). */
void moonlit_art_master_file_path(char prefix, const char *subdir, const char *file_path,
                                  long mtime, char *out, size_t outsz);

/* Reads the 130 px master at `master_path` and derives Marea's 120 px
 * cover into `out` (box downscale + baked corners, current theme's
 * surface). One plain read + integer resample, no tagcache, no JPEG:
 * the only disk access allowed inside a Marea frame (D-057). UI
 * thread only (static 130 px scratch). false if no valid master. */
bool moonlit_art_derive_from_master(const char *master_path, fb_data *out);

enum moonlit_art_result {
    MOONLIT_ART_LOADED,  /* `out` holds the cover */
    MOONLIT_ART_NONE,    /* no resolvable cover (shared .none) -- monogram for good */
    MOONLIT_ART_WAITING, /* builder pass in flight, hinted; retry when its generation moves */
};

/* Resolves the cover of `album_seek` into `out` (MOONLIT_ART_CACHE_SIZE
 * square, caller's). Order (D-059): master -> derive; ".none" ->
 * NONE; otherwise, if the builder is mid-pass, hint it and WAITING
 * (never a JPEG decode on the UI thread while the builder works);
 * builder idle -> decode the JPEG here, WRITE THE MASTER (always --
 * contract v16), derive. Never from an animation frame. */
enum moonlit_art_result moonlit_art_load_for_album(int32_t album_seek, fb_data *out);

/* The library changed (sync finish, bootstrap seal): forget nothing
 * here (keys are stable), just (re)start the builder pass. */
void moonlit_art_library_changed(void);

/* D-055: orphan sweep. Requested by metro_sync.c after a music sync
 * (disk flag under moonlitcache/art/.gc-pending, survives reboots);
 * EXECUTED by the builder thread at the end of its next pass
 * (moonlit_master_art_builder.c run_gc()), never on screen anymore. */
void moonlit_art_request_gc(void);
bool moonlit_art_gc_pending(void);
void moonlit_art_gc_clear(void);

#endif /* MOONLIT_ART_CACHE_H */
