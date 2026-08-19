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
#ifndef METRO_INPUT_H
#define METRO_INPUT_H

#include "metro_keymap.h"

/* Thin wrapper over get_custom_action() -- see DECISIONS.md M-007.
 * Blocks up to timeout ticks. Returns:
 *   - a raw SYS_* event (test with `result & SYS_EVENT`, from
 *     firmware/kernel/include/queue.h) if the core reported one --
 *     the caller must pass it to default_event_handler() itself,
 *     metro_input doesn't (metro_main.c owns that, PLAN_MAESTRO.md
 *     M-006/A.1);
 *   - MACT_NONE if nothing happened within the timeout;
 *   - one of enum metro_action otherwise.
 *
 * For MACT_PREV/MACT_NEXT/MACT_VOL_UP/MACT_VOL_DOWN, *out_steps
 * receives how many rows/volume steps to move (always >= 1, already
 * through button_apply_acceleration() -- INVESTIGACION.md C.2 --
 * direction is implied by which action it was, not by the sign of
 * *out_steps). Untouched for any other return value. */
int metro_input_next(enum metro_context ctx, int timeout_ticks, int *out_steps);

#endif /* METRO_INPUT_H */
