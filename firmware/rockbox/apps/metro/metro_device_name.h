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
/* Device name (Aura-Firmware's CONTRATO-dispositivo.md, read from that
 * sibling repo, not copied here). Aura Studio writes
 * /.rockbox/aura/device.cfg (device_id, device_name, device_owner, ...);
 * the firmware only reads `device_name` and shows it in About instead
 * of a generic literal. This module is the pure part (C99, no Rockbox,
 * host-testable): sanitizing whatever value the file may contain. Disk
 * reading lives in metro_device.c. Direct port of Aura-Firmware's
 * aura_device_name.c -- data-sanitizing code, not something Metro
 * needed to redesign. */
#ifndef METRO_DEVICE_NAME_H
#define METRO_DEVICE_NAME_H

#include <stddef.h>

/* Contract: <= 32 characters and <= 48 UTF-8 bytes. 48 + NUL. */
#define METRO_DEVICE_NAME_MAX_BYTES 48
#define METRO_DEVICE_NAME_BUF       (METRO_DEVICE_NAME_MAX_BYTES + 1)

/* Copies `in` into `out` (at least METRO_DEVICE_NAME_BUF bytes)
 * applying the firmware side's rules: trims leading/trailing
 * whitespace, collapses internal whitespace, drops control characters
 * (< 0x20 and 0x7F), and truncates to 48 bytes without splitting a
 * UTF-8 sequence. Returns the result's length in bytes; 0 if it ends
 * up empty (the caller falls back to a generic literal). */
size_t metro_device_name_sanitize(const char *in, char *out, size_t outsz);

#endif /* METRO_DEVICE_NAME_H */
