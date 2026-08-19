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
#include "lcd.h"

#include "metro_screen_usb.h"
#include "metro_draw.h"
#include "metro_theme.h"
#include "metro_lang.h"

void metro_screen_usb_show(void)
{
    const char *text = "metro";
    const char *sub = metro_lang_str(LANG_USB_CONNECTED);
    int w, h;

    metro_draw_clear();

    lcd_setfont(metro_font_id(MFONT_DISPLAY));
    lcd_getstringsize((const unsigned char *)text, &w, &h);
    metro_draw_text(MFONT_DISPLAY, (LCD_WIDTH - w) / 2, LCD_HEIGHT / 2 - h,
                     text, metro_color_fg());

    lcd_setfont(metro_font_id(MFONT_LIST));
    lcd_getstringsize((const unsigned char *)sub, &w, &h);
    metro_draw_text(MFONT_LIST, (LCD_WIDTH - w) / 2, LCD_HEIGHT / 2 + 8,
                     sub, metro_color_secondary());

    lcd_update();
}
