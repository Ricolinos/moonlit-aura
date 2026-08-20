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
#ifndef METRO_LANG_H
#define METRO_LANG_H

/* Own string table, no Rockbox .lang system -- same mechanism as
 * Aura-Firmware's aura_lang.c (D-013, INVESTIGACION.md A.9). Spanish
 * by default (DECISIONS.md M-009); append-only as new screens land. */

enum metro_language {
    METRO_LANG_ES = 0,
    METRO_LANG_EN,
    METRO_LANG_COUNT
};

enum metro_lang_id {
    LANG_HUB_MUSIC = 0,
    LANG_HUB_VIDEOS,
    LANG_HUB_PHOTOS,
    LANG_HUB_SETTINGS,

    LANG_PIVOT_ARTISTS,
    LANG_PIVOT_ALBUMS,
    LANG_PIVOT_SONGS,
    LANG_PIVOT_GENRES,
    LANG_PIVOT_PLAYLISTS,

    LANG_PIVOT_ALL,
    LANG_PIVOT_MOVIES,
    LANG_PIVOT_SERIES,
    LANG_PIVOT_CLIPS,

    LANG_PIVOT_PHOTOS,
    LANG_PIVOT_IMAGES,
    LANG_PIVOT_AI,

    LANG_PIVOT_GENERAL,
    LANG_PIVOT_DISPLAY,
    LANG_PIVOT_ABOUT,

    LANG_SETTING_LANGUAGE,
    LANG_SETTING_THEME,
    LANG_SETTING_ACCENT,
    LANG_SETTING_RESET,
    LANG_VALUE_SPANISH,
    LANG_VALUE_ENGLISH,
    LANG_VALUE_DARK,
    LANG_VALUE_LIGHT,

    LANG_ACCENT_BLUE,
    LANG_ACCENT_BROWN,
    LANG_ACCENT_GREEN,
    LANG_ACCENT_LIME,
    LANG_ACCENT_MAGENTA,
    LANG_ACCENT_MANGO,
    LANG_ACCENT_PINK,
    LANG_ACCENT_PURPLE,
    LANG_ACCENT_RED,
    LANG_ACCENT_TEAL,

    LANG_ABOUT_BASED_ON_ROCKBOX,

    LANG_DIALOG_RESET_TITLE,
    LANG_DIALOG_YES,
    LANG_DIALOG_NO,

    LANG_MUSIC_DB_UPDATING,
    LANG_HUB_NOWPLAYING,
    LANG_UNKNOWN_ARTIST,
    LANG_UNKNOWN_ALBUM,
    LANG_UNKNOWN_GENRE,
    LANG_UNKNOWN_TITLE,

    LANG_NP_VOLUME,
    LANG_NP_OPTIONS_TITLE,
    LANG_NP_SHUFFLE,
    LANG_NP_REPEAT,
    LANG_VALUE_ON,
    LANG_VALUE_OFF,
    LANG_REPEAT_ALL,
    LANG_REPEAT_ONE,

    LANG_SYNC_ERROR_VERSION,
    LANG_SYNC_ERROR_ATTEMPTS,
    LANG_SYNC_DISMISS_HINT,

    LANG_SETTING_LIBRARY,
    LANG_SETTING_BRIGHTNESS,
    LANG_SETTING_BACKLIGHT,
    LANG_VALUE_NEVER,
    LANG_DIALOG_LIBRARY_TITLE,
    LANG_LIBRARY_UPDATING,

    LANG_ABOUT_DEVICE_DEFAULT,
    LANG_ABOUT_SONGS,
    LANG_ABOUT_NOT_SYNCED,
    LANG_ABOUT_PLAYLISTS,
    LANG_ABOUT_MOVIES,
    LANG_ABOUT_SERIES,
    LANG_ABOUT_CLIPS,
    LANG_ABOUT_IMAGES,
    LANG_ABOUT_PHOTOS_TAKEN,
    LANG_ABOUT_AI,

    LANG_USB_CONNECTED,
    LANG_SHUTTING_DOWN,

    LANG_EMPTY_LIST,

    LANG_SETTING_ANIMATIONS,
    LANG_SETTING_GRAPHICS,
    LANG_ANIM_ALL,
    LANG_ANIM_MINIMAL,
    LANG_ANIM_OFF,
    LANG_GFX_FULL,
    LANG_GFX_LITE,

    LANG_PHOTO_LOADING,
    LANG_PHOTO_UNSUPPORTED,

    LANG_NP_LYRICS,
    LANG_VALUE_UNAVAILABLE,

    LANG_PIVOT_QUICKPLAY,
    LANG_QUICKPLAY_EMPTY,

    LANG_NP_RATING,

    LANG_COUNT
};

void metro_lang_set(enum metro_language lang);
enum metro_language metro_lang_get(void);
const char *metro_lang_str(enum metro_lang_id id);

#endif /* METRO_LANG_H */
