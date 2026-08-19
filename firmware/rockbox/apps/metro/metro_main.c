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
 * Metro UI -- replacement UI layer for this Rockbox fork (see
 * MODIFICATIONS.md, DECISIONS.md M-006 in the repository root).
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
#include "kernel.h"
#include "button.h"
#include "misc.h"
#include "settings.h"
#include "statusbar.h"

#include "metro_main.h"
#include "metro_screen_splash.h"

/* See metro_main.h for why this must be called from apps/main.c's
 * init(), not from here. None of these settings are exposed anywhere
 * in the Metro UI (yet), so forcing them is the only way to guarantee
 * the behaviour regardless of what a previous install left in the
 * on-disk config file. */
void metro_apply_hygiene(void)
{
    global_settings.statusbar = STATUSBAR_OFF;
    global_settings.backdrop_file[0] = '-';
    global_settings.backdrop_file[1] = '\0';
    global_settings.show_shutdown_message = false;
    global_settings.talk_menu = false;
    global_settings.clear_settings_on_hold = false;
    global_settings.tagcache_ram = true;
    global_settings.keyclick = 0;              /* M-008: piezo off by default */
#ifdef USB_ENABLE_HID
    global_settings.usb_hid = false;
#endif
}

void metro_main(void)
{
    /* metro_apply_hygiene() already ran inside init() (apps/main.c) --
     * see metro_main.h for why it can't run here, after init() returns. */
    metro_screen_splash_show();

    while (1)
    {
        int button = button_get_w_tmo(HZ);

        /* default_event_handler() handles SYS_POWEROFF (clean shutdown)
         * and SYS_USB_CONNECTED (mounts as storage, blocks until the
         * cable is unplugged) -- see PLAN_MAESTRO.md M-006/A.1. Redraw
         * after returning from USB in case anything on screen changed. */
        if (default_event_handler(button) == SYS_USB_CONNECTED)
            metro_screen_splash_show();
    }
}
