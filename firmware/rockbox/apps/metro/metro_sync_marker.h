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
/* Pending sync marker -- Aura-Firmware's docs/contracts/library-layout-v1.md
 * S4 defines the format (read from that sibling repo, not copied here --
 * CLAUDE.md). Aura Studio writes /.aura/sync-pending.json when it finishes
 * a sync; the firmware reads it at boot and after returning from the USB
 * screen, rebuilds the indices for whichever sections are marked, and
 * deletes the file only once that finished cleanly.
 *
 * Pure C99, no Rockbox dependency (same criterion as metro_nav.c) --
 * compiles and tests identically on the host (apps/metro/test/) and in
 * the firmware. Direct port of Aura-Firmware's aura_sync_marker.c: this
 * is data-format code, not something Metro needed to redesign. Not a
 * general JSON parser -- it looks up each known key by name and ignores
 * everything else, so a key a future Studio adds never breaks an older
 * firmware (same rule as aura.cfg/settings_parseline()). */
#ifndef METRO_SYNC_MARKER_H
#define METRO_SYNC_MARKER_H

#include <stdbool.h>
#include <stddef.h>

/* Schema version this firmware understands. A marker with a HIGHER
 * "version" is left alone (nothing gets rebuilt) and reported on
 * screen -- see metro_sync.c. */
#define METRO_SYNC_MARKER_VERSION_SUPPORTED 1

/* Consecutive failed attempts tolerated before giving up on retrying
 * automatically and offering the manual trigger (the counter lives
 * inside the marker itself, the firmware writes it). */
#define METRO_SYNC_MARKER_MAX_ATTEMPTS 3

/* "2026-08-17T20:15:00Z" is 20 bytes; headroom for milliseconds/offset. */
#define METRO_SYNC_MARKER_TIMESTAMP_LEN 40

typedef struct {
    int  version;       /* mandatory; -1 if missing or not a number */
    char timestamp[METRO_SYNC_MARKER_TIMESTAMP_LEN]; /* verbatim, "" if absent */
    bool music;          /* changes.music  */
    bool video;          /* changes.video  */
    bool images;         /* changes.images */
    int  attempts;       /* firmware's own counter; 0 if absent */
} metro_sync_marker_t;

typedef enum {
    METRO_SYNC_MARKER_OK = 0,
    METRO_SYNC_MARKER_MALFORMED,       /* not a recognizable JSON object */
    METRO_SYNC_MARKER_MISSING_VERSION, /* no numeric "version" */
    METRO_SYNC_MARKER_UNSUPPORTED,     /* version > what we understand */
} metro_sync_marker_status_t;

/* Resets `out` to "no work": version -1, no sections, attempts 0,
 * empty timestamp. */
void metro_sync_marker_init(metro_sync_marker_t *out);

/* Parses the whole file's text. Never fails on extra keys or
 * whitespace/newlines. With METRO_SYNC_MARKER_UNSUPPORTED, `out` is
 * still filled in (so the version can be shown on screen). With
 * MALFORMED/MISSING_VERSION, `out` is left as init() left it. */
metro_sync_marker_status_t metro_sync_marker_parse(const char *text,
                                                    metro_sync_marker_t *out);

/* Writes the contract's canonical JSON into `buf`. Returns the bytes
 * written (without the NUL), or -1 if it doesn't fit. One key per
 * line, so it stays readable in a text editor if anyone ever needs to
 * inspect the iPod by hand. */
int metro_sync_marker_serialize(const metro_sync_marker_t *m,
                                 char *buf, size_t bufsize);

/* True if any section is marked -- if none is, there is nothing to
 * rebuild and the marker can just be deleted. */
bool metro_sync_marker_has_work(const metro_sync_marker_t *m);

#endif /* METRO_SYNC_MARKER_H */
