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
#include <stdio.h>
#include "kernel.h"
#include "misc.h"
#include "lcd.h"

#include "metro_widgets.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_input.h"
#include "metro_lang.h"

#define METRO_VOLUME_OVERLAY_Y 232

bool metro_widgets_confirm(const char *title, const char *question)
{
    bool sel_yes = false; /* default to "no" -- the safe answer */

    while (1)
    {
        int action;

        metro_draw_clear();
        metro_draw_header(title);
        metro_draw_text(MFONT_TITLE, 12, 90, question, metro_color_fg());
        metro_draw_text(sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, 150,
                         metro_lang_str(LANG_DIALOG_YES),
                         sel_yes ? metro_color_fg() : metro_color_secondary());
        metro_draw_text(!sel_yes ? MFONT_LIST_SEL : MFONT_LIST, 12, 178,
                         metro_lang_str(LANG_DIALOG_NO),
                         !sel_yes ? metro_color_fg() : metro_color_secondary());
        lcd_update();

        action = metro_input_next(MCTX_DIALOG, HZ / 10, NULL);

        if (action & SYS_EVENT)
        {
            default_event_handler(action);
            continue;
        }

        switch (action)
        {
            case MACT_PREV:
            case MACT_NEXT:
                sel_yes = !sel_yes;
                break;
            case MACT_SELECT:
                return sel_yes;
            case MACT_BACK:
                return false;
            default:
                break;
        }
    }
}

void metro_widgets_draw_volume_overlay(int pct)
{
    char label[24];

    metro_draw_progress(0, METRO_VOLUME_OVERLAY_Y, LCD_WIDTH, 6, pct);
    snprintf(label, sizeof(label), "%s %d%%", metro_lang_str(LANG_NP_VOLUME), pct);
    metro_draw_text(MFONT_CAPTION, 12, METRO_VOLUME_OVERLAY_Y - 14, label,
                     metro_color_secondary());
}
