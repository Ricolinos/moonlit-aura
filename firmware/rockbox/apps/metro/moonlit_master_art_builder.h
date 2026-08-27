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
/* D-059: background builder of the shared master art cache
 * (/.aura/art/, moonlit_master_art.h) -- replaces the blocking
 * "preparando carátulas" phase of moonlit_screen_library.c (D-049/
 * D-058). Own low-priority Rockbox thread (pictureflow.c's cache
 * thread pattern; own static stack and decode buffers, never the 8 KB
 * UI stack): once the database is usable it walks albums -> artists
 * -> photos and writes the master (or the shared ".none") for every
 * element that lacks one, sleep(HZ/20) between elements, pausable
 * while Marea/transitions animate. Nothing ever waits on it: Marea and
 * the grids show the monogram/placeholder until the element lands and
 * repaint when moonlit_master_art_builder_generation() moves.
 *
 * Thread safety (verified by reading the code, DECISIONS.md D-059):
 *  - tagcache: tagcache_search() bumps a global write_lock so a commit
 *    waits for every open session; all search state is inside the
 *    caller's `struct tagcache_search`; the RAM copy (tagcache_ram) is
 *    read-only for searches. Rockbox schedules cooperatively (a switch
 *    only happens at yield/sleep/blocking calls), so the non-atomic
 *    write_lock++ is safe. The builder uses
 *    metro_music_album_art_source() (no memo) so the UI-only key memo
 *    that Marea's frames read without a lock never has a second writer.
 *  - JPEG: apps/recorder/jpeg_load.c decodes through ONE static
 *    `struct jpeg` and yields per MCU row -- two decodes on two threads
 *    WOULD interleave. Every read_jpeg_file()/clip_jpeg_file() in
 *    apps/metro/ therefore runs under moonlit_master_art_lock()
 *    (metro_albumart.c, metro_screen_photo_viewer.c). The buffering
 *    thread never decodes JPEG here (moonlit has no skin engine, so
 *    playback never requests album-art buffering).
 *  - metro_albumart.c's s_scratch/s_track_id3 stay UI-only; the
 *    builder owns its own METRO_ALBUMART_SCRATCH_SIZE scratch and
 *    mp3entry. */
#ifndef MOONLIT_MASTER_ART_BUILDER_H
#define MOONLIT_MASTER_ART_BUILDER_H

#include <stdbool.h>
#include <stdint.h>

/* Once, before the main loop (metro_main()): mutex init. No thread yet. */
void moonlit_master_art_builder_init(void);

/* Main loop, ~1 Hz until the thread exists: creates it the first time
 * metro_music_db_ready() (which also fires the bootstrap rebuild /
 * per-boot scan) and tagcache_is_fully_initialized() both hold. Cheap
 * no-op afterwards. */
void moonlit_master_art_builder_poll(void);

/* The library changed (sync finished, bootstrap seal, gc requested):
 * run another pass when the current one (if any) ends. */
void moonlit_master_art_builder_kick(void);

/* true while the thread pauses between elements (Marea scroll,
 * screen transitions -- the animation owns the disk and the CPU).
 * Checked between elements only; an element in flight finishes. */
void moonlit_master_art_builder_pause(bool paused);

/* true while a pass is walking the library. The UI decodes a JPEG
 * itself only when this is false (idle builder, e.g. a single album
 * added after the pass): otherwise it shows the placeholder, hints
 * the builder and waits for the generation to move. */
bool moonlit_master_art_builder_active(void);

/* Bumps after every element written (.art or .none) and at the end of
 * a pass -- the UI's "something may have landed, look again" signal. */
unsigned moonlit_master_art_builder_generation(void);

/* Marea/grids: "I am showing this album right now" -- serviced before
 * the next sequential element. Small ring; duplicates and overflow are
 * dropped (the sequential pass gets there anyway). */
void moonlit_master_art_builder_hint_album(int32_t album_seek);

/* Recursive mutex around the JPEG decoder's static state (and around a
 * whole decode+resample+write element on the builder). */
void moonlit_master_art_lock(void);
void moonlit_master_art_unlock(void);

#endif /* MOONLIT_MASTER_ART_BUILDER_H */
