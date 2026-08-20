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
    [LANG_HUB_MUSIC]    = "música",
    [LANG_HUB_VIDEOS]   = "videos",
    [LANG_HUB_PHOTOS]   = "fotos",
    [LANG_HUB_SETTINGS] = "ajustes",

    [LANG_PIVOT_ARTISTS]   = "artistas",
    [LANG_PIVOT_ALBUMS]    = "álbumes",
    [LANG_PIVOT_SONGS]     = "canciones",
    [LANG_PIVOT_GENRES]    = "géneros",
    [LANG_PIVOT_PLAYLISTS] = "listas",

    [LANG_PIVOT_ALL]    = "todos",
    [LANG_PIVOT_MOVIES] = "películas",
    [LANG_PIVOT_SERIES] = "series",
    [LANG_PIVOT_CLIPS]  = "clips",

    [LANG_PIVOT_PHOTOS] = "fotos",
    [LANG_PIVOT_IMAGES] = "imágenes",
    [LANG_PIVOT_AI]     = "ia",

    [LANG_PIVOT_GENERAL] = "general",
    [LANG_PIVOT_DISPLAY] = "pantalla",
    [LANG_PIVOT_ABOUT]   = "acerca de",

    [LANG_SETTING_LANGUAGE] = "idioma",
    [LANG_SETTING_THEME]    = "tema",
    [LANG_SETTING_ACCENT]   = "acento",
    [LANG_SETTING_RESET]    = "restablecer ajustes",
    [LANG_VALUE_SPANISH]    = "español",
    [LANG_VALUE_ENGLISH]    = "inglés",
    [LANG_VALUE_DARK]       = "oscuro",
    [LANG_VALUE_LIGHT]      = "claro",

    [LANG_ACCENT_BLUE]    = "azul",
    [LANG_ACCENT_BROWN]   = "café",
    [LANG_ACCENT_GREEN]   = "verde",
    [LANG_ACCENT_LIME]    = "lima",
    [LANG_ACCENT_MAGENTA] = "magenta",
    [LANG_ACCENT_MANGO]   = "mango",
    [LANG_ACCENT_PINK]    = "rosa",
    [LANG_ACCENT_PURPLE]  = "púrpura",
    [LANG_ACCENT_RED]     = "rojo",
    [LANG_ACCENT_TEAL]    = "verde azulado",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "basado en rockbox",

    [LANG_DIALOG_RESET_TITLE] = "¿restablecer ajustes?",
    [LANG_DIALOG_YES]         = "sí",
    [LANG_DIALOG_NO]          = "no",

    [LANG_MUSIC_DB_UPDATING] = "actualizando biblioteca...",
    [LANG_HUB_NOWPLAYING]    = "reproduciendo",
    [LANG_UNKNOWN_ARTIST]    = "artista desconocido",
    [LANG_UNKNOWN_ALBUM]     = "álbum desconocido",
    [LANG_UNKNOWN_GENRE]     = "género desconocido",
    [LANG_UNKNOWN_TITLE]     = "título desconocido",

    [LANG_NP_VOLUME]        = "volumen",
    [LANG_NP_OPTIONS_TITLE] = "opciones",
    [LANG_NP_SHUFFLE]       = "aleatorio",
    [LANG_NP_REPEAT]        = "repetir",
    [LANG_VALUE_ON]         = "activado",
    [LANG_VALUE_OFF]        = "desactivado",
    [LANG_REPEAT_ALL]       = "todo",
    [LANG_REPEAT_ONE]       = "uno",

    [LANG_SYNC_ERROR_VERSION]  = "esta versión de metro es más vieja que aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "no se pudo actualizar la biblioteca",
    [LANG_SYNC_DISMISS_HINT]   = "menú para continuar",

    [LANG_SETTING_LIBRARY]      = "biblioteca",
    [LANG_SETTING_BRIGHTNESS]   = "brillo",
    [LANG_SETTING_BACKLIGHT]    = "retroiluminación",
    [LANG_VALUE_NEVER]          = "nunca",
    [LANG_DIALOG_LIBRARY_TITLE] = "¿actualizar biblioteca ahora?",
    [LANG_LIBRARY_UPDATING]     = "actualizando...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "mi ipod",
    [LANG_ABOUT_SONGS]          = "canciones",
    [LANG_ABOUT_NOT_SYNCED]     = "sin sincronizar todavía",
    [LANG_ABOUT_PLAYLISTS]      = "listas",
    [LANG_ABOUT_MOVIES]         = "películas",
    [LANG_ABOUT_SERIES]         = "series",
    [LANG_ABOUT_CLIPS]          = "videoclips",
    [LANG_ABOUT_IMAGES]         = "imágenes",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "fotografías",
    [LANG_ABOUT_AI]             = "ia",

    [LANG_USB_CONNECTED] = "conectado",
    [LANG_SHUTTING_DOWN] = "apagando...",

    [LANG_EMPTY_LIST] = "aún no hay nada aquí -- sincroniza con aura studio",

    [LANG_SETTING_ANIMATIONS] = "animación",
    [LANG_SETTING_GRAPHICS]   = "gráficos",
    [LANG_ANIM_ALL]     = "completa",
    [LANG_ANIM_MINIMAL] = "mínima",
    [LANG_ANIM_OFF]     = "apagada",
    [LANG_GFX_FULL] = "completos",
    [LANG_GFX_LITE] = "ligeros",

    [LANG_PHOTO_LOADING]     = "cargando...",
    [LANG_PHOTO_UNSUPPORTED] = "formato no soportado",

    [LANG_NP_LYRICS]           = "letra",
    [LANG_VALUE_UNAVAILABLE]   = "no disponible",

    [LANG_PIVOT_QUICKPLAY]     = "reproducir ya",
    [LANG_QUICKPLAY_EMPTY]     = "sin historial todavía -- reproduce algo primero",

    [LANG_NP_RATING]           = "calificación",

    [LANG_SETTING_LOCK]         = "candado",
    [LANG_LOCK_TITLE_LOCKED]    = "bloqueado",
    [LANG_LOCK_TITLE_SET]       = "clave nueva",
    [LANG_LOCK_TITLE_CONFIRM]   = "confirma",
    [LANG_LOCK_HINT_UNLOCK]     = "marca tu clave con la rueda",
    [LANG_LOCK_HINT_WRONG]      = "clave incorrecta -- intenta de nuevo",
    [LANG_LOCK_HINT_SET]        = "elige 4 dígitos con la rueda",
    [LANG_LOCK_HINT_CONFIRM]    = "márcala otra vez para confirmar",
    [LANG_LOCK_HINT_MISMATCH]   = "no coincidieron -- empieza de nuevo",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "¿quitar el candado?",

    [LANG_SETTING_SLEEP]      = "temporizador de sueño",
    [LANG_SETTING_EQ]         = "ecualizador",
    [LANG_EQ_FLAT]            = "plano",
    [LANG_EQ_BASS]            = "graves",
    [LANG_EQ_VOCAL]           = "voz",
    [LANG_EQ_BRIGHT]          = "brillante",
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

    [LANG_SETTING_LOCK]         = "screen lock",
    [LANG_LOCK_TITLE_LOCKED]    = "locked",
    [LANG_LOCK_TITLE_SET]       = "new code",
    [LANG_LOCK_TITLE_CONFIRM]   = "confirm",
    [LANG_LOCK_HINT_UNLOCK]     = "dial your code with the wheel",
    [LANG_LOCK_HINT_WRONG]      = "wrong code -- try again",
    [LANG_LOCK_HINT_SET]        = "pick 4 digits with the wheel",
    [LANG_LOCK_HINT_CONFIRM]    = "dial it again to confirm",
    [LANG_LOCK_HINT_MISMATCH]   = "they didn't match -- start over",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "remove the lock?",

    [LANG_SETTING_SLEEP]      = "sleep timer",
    [LANG_SETTING_EQ]         = "equalizer",
    [LANG_EQ_FLAT]            = "flat",
    [LANG_EQ_BASS]            = "bass",
    [LANG_EQ_VOCAL]           = "vocal",
    [LANG_EQ_BRIGHT]          = "bright",
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

void metro_lang_initial(const char *s, char *out, size_t outsz)
{
    unsigned char lead;
    size_t len, i;

    if (outsz == 0)
        return;
    out[0] = '\0';
    if (!s || !s[0] || outsz < 2)
        return;

    lead = (unsigned char)s[0];

    /* Largo de la secuencia UTF-8 a partir del byte guía. Un byte de
     * continuación suelto (10xxxxxx) o un guía inválido se tratan como
     * 1 byte: entrada corrupta no debe leer de más. */
    if (lead < 0x80)            len = 1;
    else if ((lead & 0xE0) == 0xC0) len = 2;
    else if ((lead & 0xF0) == 0xE0) len = 3;
    else if ((lead & 0xF8) == 0xF0) len = 4;
    else                        len = 1;

    /* Nunca leer más allá del fin de la cadena si viene truncada. */
    for (i = 1; i < len; i++)
    {
        if (!s[i])
        {
            len = i;
            break;
        }
    }

    if (len + 1 > outsz)
        len = outsz - 1;

    for (i = 0; i < len; i++)
        out[i] = s[i];
    out[len] = '\0';

    if (len == 1 && out[0] >= 'a' && out[0] <= 'z')
    {
        out[0] -= 32;
    }
    else if (len == 2 && (unsigned char)out[0] == 0xC3)
    {
        /* Latin-1 en UTF-8: las minúsculas acentuadas viven en
         * 0xC3 0xA0..0xBE y su mayúscula está 0x20 abajo (á 0xC3 0xA1
         * -> Á 0xC3 0x81). Se excluyen 0xB7 (÷, no es letra) y 0xBF
         * (ÿ, cuya mayúscula Ÿ no está en Latin-1). */
        unsigned char b = (unsigned char)out[1];

        if (b >= 0xA0 && b <= 0xBE && b != 0xB7)
            out[1] = (char)(b - 0x20);
    }
}
