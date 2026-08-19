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
/* Metro's own "connected" frame, shown for the one redraw right after
 * SYS_USB_CONNECTED and before metro_main.c hands off to
 * default_event_handler() (which owns the screen via the stock
 * gui_usb_screen_run() for the whole mounted duration -- see
 * DESVIACIONES.md F9-1 for why that part isn't Metro-styled too). */
#ifndef METRO_SCREEN_USB_H
#define METRO_SCREEN_USB_H

void metro_screen_usb_show(void);

#endif /* METRO_SCREEN_USB_H */
