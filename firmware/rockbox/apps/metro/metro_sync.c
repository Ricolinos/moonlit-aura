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

/* config.h before tagcache.h -- see DECISIONS.md M-030. */
#include "config.h"
#include "tagcache.h"
#include "file.h"
#include "dir.h"

#include "metro_sync.h"
#include "metro_sync_marker.h"

/* Contract S4: the marker lives at the disk ROOT, not under
 * /.rockbox/aura -- it's the one thing Studio leaves for the firmware
 * as "mail", kept separate from Metro's own settings/caches. */
#define METRO_SYNC_DIR         "/.aura"
#define METRO_SYNC_MARKER_PATH METRO_SYNC_DIR "/sync-pending.json"

/* A real marker is ~150 bytes; 1KB leaves room for future keys. */
#define MARKER_BUF_SIZE 1024

typedef enum { JOB_NONE = 0, JOB_UPDATE, JOB_REBUILD } job_kind_t;

static metro_sync_state_t s_state = METRO_SYNC_IDLE;
static metro_sync_marker_t s_marker;
static job_kind_t s_job = JOB_NONE;
static unsigned s_jobs_before = 0;
static int s_attempts_before = 0;
static bool s_recovery_done = false;

static int read_marker_text(char *buf, size_t bufsize)
{
    int fd, n;

    fd = open(METRO_SYNC_MARKER_PATH, O_RDONLY);
    if (fd < 0)
        return -1;
    n = read(fd, buf, bufsize - 1);
    close(fd);
    if (n < 0)
        return -1;
    if ((size_t)n >= bufsize - 1)
        return -2; /* bigger than any legitimate marker: not ours */
    buf[n] = '\0';
    return n;
}

static bool write_marker(const metro_sync_marker_t *m)
{
    char buf[MARKER_BUF_SIZE];
    int fd, n = metro_sync_marker_serialize(m, buf, sizeof(buf));

    if (n < 0)
        return false;
    if (!dir_exists(METRO_SYNC_DIR))
        mkdir(METRO_SYNC_DIR);

    fd = creat(METRO_SYNC_MARKER_PATH, 0666);
    if (fd < 0)
        return false;
    write(fd, buf, (size_t)n);
    close(fd);
    return true;
}

static void remove_marker(void)
{
    remove(METRO_SYNC_MARKER_PATH);
}

static void go_idle(void)
{
    s_state = METRO_SYNC_IDLE;
    s_job = JOB_NONE;
}

void metro_sync_check_pending(void)
{
    static char buf[MARKER_BUF_SIZE];
    metro_sync_marker_status_t st;
    int n;

    /* A job in flight rules: don't re-read the marker out from under it. */
    if (metro_sync_job_active())
        return;

    n = read_marker_text(buf, sizeof(buf));
    if (n == -1)
    {
        go_idle();
        return;
    }
    if (n == -2)
    {
        remove_marker();
        go_idle();
        return;
    }

    st = metro_sync_marker_parse(buf, &s_marker);

    switch (st)
    {
        case METRO_SYNC_MARKER_MALFORMED:
        case METRO_SYNC_MARKER_MISSING_VERSION:
            /* Corrupt (Studio writes atomically, so this shouldn't
             * happen): nothing reliable to rebuild from, and leaving
             * it would trip every boot over it. Gone. */
            remove_marker();
            go_idle();
            return;

        case METRO_SYNC_MARKER_UNSUPPORTED:
            /* Contract S4: ignored (nothing gets rebuilt with rules we
             * don't understand) and reported on screen. The marker
             * stays -- a newer firmware will resolve it. */
            s_state = METRO_SYNC_ERROR_VERSION;
            return;

        case METRO_SYNC_MARKER_OK:
        default:
            break;
    }

    if (!metro_sync_marker_has_work(&s_marker))
    {
        remove_marker();
        go_idle();
        return;
    }

    if (s_marker.attempts >= METRO_SYNC_MARKER_MAX_ATTEMPTS)
    {
        s_state = METRO_SYNC_ERROR_ATTEMPTS;
        return;
    }

    s_state = METRO_SYNC_WAIT_TAGCACHE;
}

bool metro_sync_needs_screen(void)
{
    return s_state == METRO_SYNC_WAIT_TAGCACHE
        || s_state == METRO_SYNC_RUNNING
        || s_state == METRO_SYNC_ERROR_VERSION
        || s_state == METRO_SYNC_ERROR_ATTEMPTS;
}

bool metro_sync_job_active(void)
{
    return s_state == METRO_SYNC_WAIT_TAGCACHE
        || s_state == METRO_SYNC_RUNNING
        || s_state == METRO_SYNC_POSTPONED;
}

metro_sync_state_t metro_sync_state(void)
{
    return s_state;
}

static void finish_ok(void)
{
    remove_marker();
    go_idle();
}

static void start_job(void)
{
    /* The attempt counts from the moment it starts: if power is lost
     * halfway through, the marker already carries the raised counter.
     * Postponing restores it. */
    s_attempts_before = s_marker.attempts;
    s_marker.attempts++;
    write_marker(&s_marker);

    /* Video/images: nothing cached to invalidate yet (F7's
     * metro_video.c/metro_photos.c re-scan their directory fresh every
     * time their page is entered, same as metro_music.c's own lists) --
     * see the module comment in metro_sync.h / DESVIACIONES.md F6-1. */
    if (!s_marker.music)
    {
        finish_ok();
        return;
    }

    /* With a usable database and no orphaned temp file, tagcache_update():
     * walks the whole tree, re-reads files whose mtime changed (Studio
     * never preserves dates when copying, so anything it touched has a
     * new mtime), adds new ones, drops deleted ones -- the OLD database
     * stays usable meanwhile. No database (first boot, or an old Studio
     * that deleted it), or an orphaned temp from an aborted pass:
     * tagcache_rebuild() from scratch. */
    s_jobs_before = tagcache_get_build_jobs_done();
    if (tagcache_is_usable() && !tagcache_has_pending_temp())
    {
        s_job = JOB_UPDATE;
        tagcache_update();
    }
    else
    {
        s_job = JOB_REBUILD;
        tagcache_rebuild();
    }
    s_recovery_done = false;
    s_state = METRO_SYNC_RUNNING;
}

static void job_ended(void)
{
    bool ok = tagcache_is_usable() && !tagcache_has_pending_temp();

    if (ok)
    {
        finish_ok();
        return;
    }

    if (!tagcache_is_usable() && !tagcache_has_pending_temp() && !s_recovery_done)
    {
        /* Finished, but the database ended up disabled: tagcache found
         * corrupt structures while reloading it. The only way out is
         * building it from scratch -- same job, same screen, once. */
        s_recovery_done = true;
        s_jobs_before = tagcache_get_build_jobs_done();
        s_job = JOB_REBUILD;
        tagcache_rebuild();
        if (s_state != METRO_SYNC_POSTPONED)
            s_state = METRO_SYNC_RUNNING;
        return;
    }

    if (tagcache_get_stat()->commit_delayed)
    {
        /* tagcache scanned everything but couldn't get a temp buffer to
         * sort the indices, and left the commit "for the next boot"
         * (its own mechanism -- the thread confirms it at boot, before
         * anything else). Not a failure of ours: the counter goes back
         * to where it was, the marker stays, and next boot the update
         * completes (everything already indexed, a fast pass) and
         * clears it. */
        s_marker.attempts = s_attempts_before;
        write_marker(&s_marker);
        s_state = METRO_SYNC_IDLE;
        s_job = JOB_NONE;
        return;
    }

    if (s_state == METRO_SYNC_POSTPONED)
    {
        /* Aborted on purpose (Menu): not a failure -- the counter goes
         * back to where it was and the marker stays for next boot. An
         * aborted update leaves a half-done temp file that would block
         * any later build this boot; the old database is intact, so
         * it's discarded. An aborted rebuild has no old database left:
         * the temp is kept so tagcache's own boot confirms (commits)
         * it instead of losing what was already scanned. */
        s_marker.attempts = s_attempts_before;
        write_marker(&s_marker);
        if (s_job == JOB_UPDATE && tagcache_has_pending_temp())
            tagcache_discard_pending_temp();
        go_idle();
        return;
    }

    /* Real failure (USB plugged in halfway, database that never came
     * back usable...): the counter already went up; the marker stays
     * and gets retried next boot, unless that's already too many. */
    if (s_marker.attempts >= METRO_SYNC_MARKER_MAX_ATTEMPTS)
    {
        s_state = METRO_SYNC_ERROR_ATTEMPTS;
        s_job = JOB_NONE;
    }
    else
        go_idle();
}

bool metro_sync_tick(void)
{
    switch (s_state)
    {
        case METRO_SYNC_WAIT_TAGCACHE:
            /* Same precaution as metro_music_db_ready() (M-030-adjacent,
             * D-206 in Aura-Firmware): don't decide update-vs-rebuild
             * before tagcache has determined whether a usable database
             * already exists on disk (~1s after boot). */
            if (!tagcache_is_fully_initialized())
                return false;
            start_job();
            return true;

        case METRO_SYNC_RUNNING:
        case METRO_SYNC_POSTPONED:
            if (tagcache_get_build_jobs_done() == s_jobs_before)
                return s_state == METRO_SYNC_RUNNING; /* still going: redraw */
            job_ended();
            return true;

        default:
            return false;
    }
}

void metro_sync_postpone(void)
{
    switch (s_state)
    {
        case METRO_SYNC_WAIT_TAGCACHE:
            /* Nothing started yet: the marker is left exactly as is. */
            go_idle();
            break;
        case METRO_SYNC_RUNNING:
            tagcache_stop_scan();
            s_state = METRO_SYNC_POSTPONED;
            break;
        default:
            break;
    }
}

void metro_sync_dismiss(void)
{
    if (s_state == METRO_SYNC_ERROR_VERSION || s_state == METRO_SYNC_ERROR_ATTEMPTS)
        go_idle();
}
