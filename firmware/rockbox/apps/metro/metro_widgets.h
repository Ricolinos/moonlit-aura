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
#ifndef METRO_WIDGETS_H
#define METRO_WIDGETS_H

#include <stdbool.h>

/* The system's single modal dialog (PLAN_MAESTRO.md S2.3, MCTX_DIALOG):
 * blocks with its own small input loop (MCTX_DIALOG context) until the
 * user confirms or cancels. Defaults the selection to "no" -- the safe
 * answer -- never to "yes". Redraws the whole screen; the caller must
 * redraw whatever was behind it afterwards. */
bool metro_widgets_confirm(const char *title, const char *question);

#endif /* METRO_WIDGETS_H */
