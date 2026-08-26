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

/* moonlit (D-041): grados/seg crudos del ultimo MACT_PREV/MACT_NEXT,
 * para moonlit_wheel_step() (Marea, M8). Mismo campo de bits que lee
 * button_apply_acceleration() dos lineas mas abajo (button.c:632:
 * "[23:0] Velocity - degree/sec") -- confirmado identico al
 * button_get_data() & 0xFFFFFF de aura_main_wheel_velocity()
 * (AF/aura_musicflow.c:1240). 0 si la ultima accion no fue de rueda o
 * el target no tiene HAVE_WHEEL_ACCELERATION. */
static int s_last_wheel_velocity_deg_s = 0;

int metro_input_last_wheel_velocity(void)
{
    return s_last_wheel_velocity_deg_s;
}

int metro_input_next(enum metro_context ctx, int timeout_ticks, int *out_steps)
{
    int action = get_custom_action((int)ctx | CONTEXT_PLUGIN, timeout_ticks,
                                    metro_keymap_get_context_map);

    if (action & SYS_EVENT)
        return action;

    if ((action == MACT_PREV || action == MACT_NEXT ||
         action == MACT_VOL_UP || action == MACT_VOL_DOWN) && out_steps)
    {
#ifdef HAVE_WHEEL_ACCELERATION
        int steps = button_apply_acceleration(get_action_data());
        *out_steps = steps > 0 ? steps : 1;
        if (action == MACT_PREV || action == MACT_NEXT)
            s_last_wheel_velocity_deg_s = get_action_data() & 0xffffff;
#else
        *out_steps = 1;
#endif
    }

    return action;
}
