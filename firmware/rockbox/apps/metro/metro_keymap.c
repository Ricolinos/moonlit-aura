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
#include "metro_keymap.h"

/* HUB (root): wheel moves selection, SELECT enters. No pivots, no
 * Back target -- MENU/LEFT/RIGHT simply aren't mapped, so they
 * resolve to ACTION_NONE (matches PLAN_MAESTRO.md S2.3 "sin efecto"). */
static const struct button_mapping hub_mapping[] = {
    { MACT_PREV,   BUTTON_SCROLL_BACK,                BUTTON_NONE },
    { MACT_PREV,   BUTTON_SCROLL_BACK | BUTTON_REPEAT, BUTTON_NONE },
    { MACT_NEXT,   BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { MACT_NEXT,   BUTTON_SCROLL_FWD | BUTTON_REPEAT,  BUTTON_NONE },
    { MACT_SELECT, BUTTON_SELECT | BUTTON_REL,         BUTTON_SELECT },
    LAST_ITEM_IN_LIST
};

/* LIST (any pivoted page, including Settings): wheel moves selection,
 * LEFT/RIGHT twist between pivots (no wrap, handled in metro_nav.c),
 * SELECT enters/activates, MENU goes back, MENU held jumps to the
 * hub. */
static const struct button_mapping list_mapping[] = {
    { MACT_PREV,       BUTTON_SCROLL_BACK,                BUTTON_NONE },
    { MACT_PREV,       BUTTON_SCROLL_BACK | BUTTON_REPEAT, BUTTON_NONE },
    { MACT_NEXT,       BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { MACT_NEXT,       BUTTON_SCROLL_FWD | BUTTON_REPEAT,  BUTTON_NONE },
    { MACT_PIVOT_PREV, BUTTON_LEFT  | BUTTON_REL,          BUTTON_LEFT  },
    { MACT_PIVOT_NEXT, BUTTON_RIGHT | BUTTON_REL,          BUTTON_RIGHT },
    { MACT_SELECT,     BUTTON_SELECT | BUTTON_REL,         BUTTON_SELECT },
    { MACT_BACK,       BUTTON_MENU | BUTTON_REL,           BUTTON_MENU },
    { MACT_HOME,       BUTTON_MENU | BUTTON_REPEAT,        BUTTON_NONE },
    LAST_ITEM_IN_LIST
};

/* DIALOG (yes/no confirm): wheel toggles the choice, SELECT confirms,
 * MENU cancels -- PLAN_MAESTRO.md S2.3. */
static const struct button_mapping dialog_mapping[] = {
    { MACT_PREV,   BUTTON_SCROLL_BACK,                BUTTON_NONE },
    { MACT_NEXT,   BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { MACT_SELECT, BUTTON_SELECT | BUTTON_REL,         BUTTON_SELECT },
    { MACT_BACK,   BUTTON_MENU | BUTTON_REL,           BUTTON_MENU },
    LAST_ITEM_IN_LIST
};

const struct button_mapping *metro_keymap_get_context_map(int context)
{
    switch (context & ~CONTEXT_PLUGIN)
    {
        case MCTX_HUB:    return hub_mapping;
        case MCTX_LIST:   return list_mapping;
        case MCTX_DIALOG: return dialog_mapping;
        default:          return NULL;
    }
}
