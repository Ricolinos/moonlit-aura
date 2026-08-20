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
#include "metro_lang.h"

static enum metro_language current_lang = METRO_LANG_ES;

static const char *const strings_es[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "musica",
    [LANG_HUB_VIDEOS]   = "videos",
    [LANG_HUB_PHOTOS]   = "fotos",
    [LANG_HUB_SETTINGS] = "ajustes",

    [LANG_PIVOT_ARTISTS]   = "artistas",
    [LANG_PIVOT_ALBUMS]    = "albumes",
    [LANG_PIVOT_SONGS]     = "canciones",
    [LANG_PIVOT_GENRES]    = "generos",
    [LANG_PIVOT_PLAYLISTS] = "listas",

    [LANG_PIVOT_ALL]    = "todos",
    [LANG_PIVOT_MOVIES] = "peliculas",
    [LANG_PIVOT_SERIES] = "series",
    [LANG_PIVOT_CLIPS]  = "clips",

    [LANG_PIVOT_PHOTOS] = "fotos",
    [LANG_PIVOT_IMAGES] = "imagenes",
    [LANG_PIVOT_AI]     = "ia",

    [LANG_PIVOT_GENERAL] = "general",
    [LANG_PIVOT_DISPLAY] = "pantalla",
    [LANG_PIVOT_ABOUT]   = "acerca de",

    [LANG_SETTING_LANGUAGE] = "idioma",
    [LANG_SETTING_THEME]    = "tema",
    [LANG_SETTING_ACCENT]   = "acento",
    [LANG_SETTING_RESET]    = "restablecer ajustes",
    [LANG_VALUE_SPANISH]    = "espanol",
    [LANG_VALUE_ENGLISH]    = "ingles",
    [LANG_VALUE_DARK]       = "oscuro",
    [LANG_VALUE_LIGHT]      = "claro",

    [LANG_ACCENT_BLUE]    = "azul",
    [LANG_ACCENT_BROWN]   = "cafe",
    [LANG_ACCENT_GREEN]   = "verde",
    [LANG_ACCENT_LIME]    = "lima",
    [LANG_ACCENT_MAGENTA] = "magenta",
    [LANG_ACCENT_MANGO]   = "mango",
    [LANG_ACCENT_PINK]    = "rosa",
    [LANG_ACCENT_PURPLE]  = "purpura",
    [LANG_ACCENT_RED]     = "rojo",
    [LANG_ACCENT_TEAL]    = "verde azulado",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "basado en rockbox",

    [LANG_DIALOG_RESET_TITLE] = "restablecer ajustes?",
    [LANG_DIALOG_YES]         = "si",
    [LANG_DIALOG_NO]          = "no",

    [LANG_MUSIC_DB_UPDATING] = "actualizando biblioteca...",
    [LANG_HUB_NOWPLAYING]    = "reproduciendo",
    [LANG_UNKNOWN_ARTIST]    = "artista desconocido",
    [LANG_UNKNOWN_ALBUM]     = "album desconocido",
    [LANG_UNKNOWN_GENRE]     = "genero desconocido",
    [LANG_UNKNOWN_TITLE]     = "titulo desconocido",

    [LANG_NP_VOLUME]        = "volumen",
    [LANG_NP_OPTIONS_TITLE] = "opciones",
    [LANG_NP_SHUFFLE]       = "aleatorio",
    [LANG_NP_REPEAT]        = "repetir",
    [LANG_VALUE_ON]         = "activado",
    [LANG_VALUE_OFF]        = "desactivado",
    [LANG_REPEAT_ALL]       = "todo",
    [LANG_REPEAT_ONE]       = "uno",

    [LANG_SYNC_ERROR_VERSION]  = "esta version de metro es mas vieja que aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "no se pudo actualizar la biblioteca",
    [LANG_SYNC_DISMISS_HINT]   = "menu para continuar",

    [LANG_SETTING_LIBRARY]      = "biblioteca",
    [LANG_SETTING_BRIGHTNESS]   = "brillo",
    [LANG_SETTING_BACKLIGHT]    = "retroiluminacion",
    [LANG_VALUE_NEVER]          = "nunca",
    [LANG_DIALOG_LIBRARY_TITLE] = "actualizar biblioteca ahora?",
    [LANG_LIBRARY_UPDATING]     = "actualizando...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "mi ipod",
    [LANG_ABOUT_SONGS]          = "canciones",
    [LANG_ABOUT_NOT_SYNCED]     = "sin sincronizar todavia",
    [LANG_ABOUT_PLAYLISTS]      = "listas",
    [LANG_ABOUT_MOVIES]         = "peliculas",
    [LANG_ABOUT_SERIES]         = "series",
    [LANG_ABOUT_CLIPS]          = "videoclips",
    [LANG_ABOUT_IMAGES]         = "imagenes",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "fotografias",
    [LANG_ABOUT_AI]             = "ia",

    [LANG_USB_CONNECTED] = "conectado",
    [LANG_SHUTTING_DOWN] = "apagando...",

    [LANG_EMPTY_LIST] = "aun no hay nada aqui -- sincroniza con aura studio",

    [LANG_SETTING_ANIMATIONS] = "animacion",
    [LANG_SETTING_GRAPHICS]   = "graficos",
    [LANG_ANIM_ALL]     = "completa",
    [LANG_ANIM_MINIMAL] = "minima",
    [LANG_ANIM_OFF]     = "apagada",
    [LANG_GFX_FULL] = "completos",
    [LANG_GFX_LITE] = "ligeros",

    [LANG_PHOTO_LOADING]     = "cargando...",
    [LANG_PHOTO_UNSUPPORTED] = "formato no soportado",

    [LANG_NP_LYRICS]           = "letra",
    [LANG_VALUE_UNAVAILABLE]   = "no disponible",

    [LANG_PIVOT_QUICKPLAY]     = "reproducir ya",
    [LANG_QUICKPLAY_EMPTY]     = "sin historial todavia -- reproduce algo primero",

    [LANG_NP_RATING]           = "calificacion",
};

static const char *const strings_en[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "music",
    [LANG_HUB_VIDEOS]   = "videos",
    [LANG_HUB_PHOTOS]   = "photos",
    [LANG_HUB_SETTINGS] = "settings",

    [LANG_PIVOT_ARTISTS]   = "artists",
    [LANG_PIVOT_ALBUMS]    = "albums",
    [LANG_PIVOT_SONGS]     = "songs",
    [LANG_PIVOT_GENRES]    = "genres",
    [LANG_PIVOT_PLAYLISTS] = "playlists",

    [LANG_PIVOT_ALL]    = "all",
    [LANG_PIVOT_MOVIES] = "movies",
    [LANG_PIVOT_SERIES] = "series",
    [LANG_PIVOT_CLIPS]  = "clips",

    [LANG_PIVOT_PHOTOS] = "photos",
    [LANG_PIVOT_IMAGES] = "images",
    [LANG_PIVOT_AI]     = "ai",

    [LANG_PIVOT_GENERAL] = "general",
    [LANG_PIVOT_DISPLAY] = "display",
    [LANG_PIVOT_ABOUT]   = "about",

    [LANG_SETTING_LANGUAGE] = "language",
    [LANG_SETTING_THEME]    = "theme",
    [LANG_SETTING_ACCENT]   = "accent",
    [LANG_SETTING_RESET]    = "reset settings",
    [LANG_VALUE_SPANISH]    = "spanish",
    [LANG_VALUE_ENGLISH]    = "english",
    [LANG_VALUE_DARK]       = "dark",
    [LANG_VALUE_LIGHT]      = "light",

    [LANG_ACCENT_BLUE]    = "blue",
    [LANG_ACCENT_BROWN]   = "brown",
    [LANG_ACCENT_GREEN]   = "green",
    [LANG_ACCENT_LIME]    = "lime",
    [LANG_ACCENT_MAGENTA] = "magenta",
    [LANG_ACCENT_MANGO]   = "mango",
    [LANG_ACCENT_PINK]    = "pink",
    [LANG_ACCENT_PURPLE]  = "purple",
    [LANG_ACCENT_RED]     = "red",
    [LANG_ACCENT_TEAL]    = "teal",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "based on rockbox",

    [LANG_DIALOG_RESET_TITLE] = "reset settings?",
    [LANG_DIALOG_YES]         = "yes",
    [LANG_DIALOG_NO]          = "no",

    [LANG_MUSIC_DB_UPDATING] = "updating library...",
    [LANG_HUB_NOWPLAYING]    = "now playing",
    [LANG_UNKNOWN_ARTIST]    = "unknown artist",
    [LANG_UNKNOWN_ALBUM]     = "unknown album",
    [LANG_UNKNOWN_GENRE]     = "unknown genre",
    [LANG_UNKNOWN_TITLE]     = "unknown title",

    [LANG_NP_VOLUME]        = "volume",
    [LANG_NP_OPTIONS_TITLE] = "options",
    [LANG_NP_SHUFFLE]       = "shuffle",
    [LANG_NP_REPEAT]        = "repeat",
    [LANG_VALUE_ON]         = "on",
    [LANG_VALUE_OFF]        = "off",
    [LANG_REPEAT_ALL]       = "all",
    [LANG_REPEAT_ONE]       = "one",

    [LANG_SYNC_ERROR_VERSION]  = "this metro version is older than aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "could not update the library",
    [LANG_SYNC_DISMISS_HINT]   = "menu to continue",

    [LANG_SETTING_LIBRARY]      = "library",
    [LANG_SETTING_BRIGHTNESS]   = "brightness",
    [LANG_SETTING_BACKLIGHT]    = "backlight",
    [LANG_VALUE_NEVER]          = "never",
    [LANG_DIALOG_LIBRARY_TITLE] = "update library now?",
    [LANG_LIBRARY_UPDATING]     = "updating...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "my ipod",
    [LANG_ABOUT_SONGS]          = "songs",
    [LANG_ABOUT_NOT_SYNCED]     = "not synced yet",
    [LANG_ABOUT_PLAYLISTS]      = "playlists",
    [LANG_ABOUT_MOVIES]         = "movies",
    [LANG_ABOUT_SERIES]         = "series",
    [LANG_ABOUT_CLIPS]          = "clips",
    [LANG_ABOUT_IMAGES]         = "images",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "photos",
    [LANG_ABOUT_AI]             = "ai",

    [LANG_USB_CONNECTED] = "connected",
    [LANG_SHUTTING_DOWN] = "shutting down...",

    [LANG_EMPTY_LIST] = "nothing here yet -- sync with aura studio",

    [LANG_SETTING_ANIMATIONS] = "animation",
    [LANG_SETTING_GRAPHICS]   = "graphics",
    [LANG_ANIM_ALL]     = "full",
    [LANG_ANIM_MINIMAL] = "minimal",
    [LANG_ANIM_OFF]     = "off",
    [LANG_GFX_FULL] = "full",
    [LANG_GFX_LITE] = "lite",

    [LANG_PHOTO_LOADING]     = "loading...",
    [LANG_PHOTO_UNSUPPORTED] = "unsupported format",

    [LANG_NP_LYRICS]           = "lyrics",
    [LANG_VALUE_UNAVAILABLE]   = "unavailable",

    [LANG_PIVOT_QUICKPLAY]     = "quickplay",
    [LANG_QUICKPLAY_EMPTY]     = "nothing played yet -- play something first",

    [LANG_NP_RATING]           = "rating",
};

void metro_lang_set(enum metro_language lang)
{
    if (lang == METRO_LANG_ES || lang == METRO_LANG_EN)
        current_lang = lang;
}

enum metro_language metro_lang_get(void)
{
    return current_lang;
}

const char *metro_lang_str(enum metro_lang_id id)
{
    const char *s;

    if ((unsigned)id >= LANG_COUNT)
        return "";

    s = (current_lang == METRO_LANG_EN) ? strings_en[id] : strings_es[id];
    return s ? s : "";
}
