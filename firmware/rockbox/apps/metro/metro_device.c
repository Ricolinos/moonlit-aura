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

#include "file.h"
#include "rbpaths.h"
#include "misc.h"

#include "metro_device.h"
#include "metro_device_name.h"

#define METRO_DIR        ROCKBOX_DIR "/aura"
#define DEVICE_CFG_PATH  METRO_DIR "/device.cfg"

static char s_name[METRO_DEVICE_NAME_BUF];
static bool s_has_name = false;

void metro_device_reload(void)
{
    int fd;
    char line[64]; /* contract: lines <= 63 bytes, same buffer as aura.cfg */

    s_has_name = false;
    s_name[0] = '\0';

    fd = open(DEVICE_CFG_PATH, O_RDONLY);
    if (fd < 0)
        return;

    while (read_line(fd, line, sizeof(line)) > 0)
    {
        char *name, *value;
        if (!settings_parseline(line, &name, &value))
            continue;
        if (!strcmp(name, "device_name"))
        {
            s_has_name = metro_device_name_sanitize(value, s_name, sizeof(s_name)) > 0;
            break;
        }
    }
    close(fd);
}

const char *metro_device_name(void)
{
    return s_has_name ? s_name : NULL;
}
