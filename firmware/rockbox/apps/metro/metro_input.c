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
#include "button.h"

#include "metro_input.h"

int metro_input_next(enum metro_context ctx, int timeout_ticks, int *out_steps)
{
    int action = get_custom_action((int)ctx | CONTEXT_PLUGIN, timeout_ticks,
                                    metro_keymap_get_context_map);

    if (action & SYS_EVENT)
        return action;

    if ((action == MACT_PREV || action == MACT_NEXT) && out_steps)
    {
#ifdef HAVE_WHEEL_ACCELERATION
        int steps = button_apply_acceleration(get_action_data());
        *out_steps = steps > 0 ? steps : 1;
#else
        *out_steps = 1;
#endif
    }

    return action;
}
