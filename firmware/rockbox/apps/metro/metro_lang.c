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
#include <string.h>

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
    /* moonlit (D-027, M4): "noche"/"amanecer" en vez de "oscuro"/"claro"
     * -- los dos esquemas MD3 llevan la identidad Waning Crescent, no
     * solo una polaridad de contraste. */
    [LANG_VALUE_DARK]       = "noche",
    [LANG_VALUE_LIGHT]      = "amanecer",

    /* moonlit (D-028, M4): 4 presets de acento MD3. */
    [LANG_ACCENT_MOONSTONE] = "piedra lunar",
    [LANG_ACCENT_TIDE]      = "marea",
    [LANG_ACCENT_EMBER]     = "ascua",
    [LANG_ACCENT_MOSS]      = "musgo",

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

    [LANG_NP_OPTIONS_TITLE] = "opciones",
    [LANG_NP_SHUFFLE]       = "aleatorio",
    [LANG_NP_REPEAT]        = "repetir",
    [LANG_VALUE_ON]         = "activado",
    [LANG_VALUE_OFF]        = "desactivado",
    [LANG_REPEAT_ALL]       = "todo",
    [LANG_REPEAT_ONE]       = "uno",

    [LANG_SYNC_ERROR_VERSION]  = "esta versión de moonlit es más vieja que aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "no se pudo actualizar la biblioteca",
    [LANG_SYNC_DISMISS_HINT]   = "menú para continuar",
    [LANG_SYNC_ART_ALBUMS]     = "preparando carátulas %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "preparando fotos de artistas %d",
    [LANG_SYNC_ART_PHOTOS]     = "preparando imágenes %d",

    [LANG_SETTING_LIBRARY]      = "actualizar biblioteca",
    [LANG_SETTING_BRIGHTNESS]   = "brillo",
    [LANG_SETTING_BACKLIGHT]    = "retroiluminación",
    [LANG_VALUE_NEVER]          = "nunca",
    [LANG_DIALOG_LIBRARY_TITLE] = "¿actualizar biblioteca ahora?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "puede tardar varios minutos, según cuántos archivos tengas y cómo esté el disco.",
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

    [LANG_SETTING_LOCK]         = "bloqueo", /* moonlit (D-069): era "candado" */
    [LANG_LOCK_TITLE_LOCKED]    = "bloqueado",
    [LANG_LOCK_TITLE_SET]       = "clave nueva",
    [LANG_LOCK_TITLE_CONFIRM]   = "confirma",
    [LANG_LOCK_HINT_UNLOCK]     = "marca tu clave con la rueda",
    [LANG_LOCK_HINT_WRONG]      = "clave incorrecta -- intenta de nuevo",
    [LANG_LOCK_HINT_SET]        = "elige 4 dígitos con la rueda",
    [LANG_LOCK_HINT_CONFIRM]    = "márcala otra vez para confirmar",
    [LANG_LOCK_HINT_MISMATCH]   = "no coincidieron -- empieza de nuevo",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "¿quitar el bloqueo?",

    [LANG_SETTING_SLEEP]      = "temporizador de sueño",
    [LANG_SETTING_EQ]         = "ecualizador",
    [LANG_EQ_FLAT]            = "plano",
    [LANG_EQ_BASS]            = "graves",
    [LANG_EQ_VOCAL]           = "voz",
    [LANG_EQ_BRIGHT]          = "brillante",

    [LANG_SETTING_VOLUME_LIMIT] = "límite de volumen",

    [LANG_LIST_TRUNCATED] = "…y más: la lista está llena",

    [LANG_VALUE_NOT_INSTALLED] = "no instalado",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "creado por ricardo gómez\n"
        "basado en rockbox, gpl v2\n"
        "moonlit hereda esa licencia\n"
        "código fuente:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "tipografía e íconos:\n"
        "libre baskerville (sil ofl)\n"
        "montserrat (sil ofl)\n"
        "material symbols (apache 2.0)\n"
        "ipod es marca de apple inc.\n"
        "moonlit no está afiliado a apple",
    [LANG_MAREA_TITLE]        = "marea",
    [LANG_MAREA_EMPTY]        = "no hay álbumes en la biblioteca",
    [LANG_MAREA_SONGS_FMT]    = "%d canciones",

    /* moonlit (D-047): cambiar sistema */
    [LANG_SETTING_SWITCH_SYSTEM] = "cambiar sistema",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "¿cambiar a %s y reiniciar?",

    /* moonlit (D-049): preparando biblioteca */
    [LANG_LIBRARY_PREPARING] = "preparando biblioteca",
    [LANG_LIBRARY_PHASE_DB]  = "construyendo la base de música",
    [LANG_LIBRARY_COUNT_FMT] = "%d de %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "versión %s",
    [LANG_ABOUT_STACK_FMT]   = "pila principal %d %%",
    [LANG_ABOUT_STACK_NA]    = "pila principal n/d",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "activar",
    [LANG_LOCK_CHANGE]       = "cambiar código",
    [LANG_LOCK_REQUIRE]      = "pedir código",
    [LANG_LOCK_REQUIRE_HOLD] = "al bloquear",
    [LANG_LOCK_REQUIRE_1MIN] = "tras 1 minuto",
    [LANG_LOCK_REQUIRE_5MIN] = "tras 5 minutos",
    [LANG_LOCK_REQUIRE_BOOT] = "solo al encender",
    [LANG_LOCK_REMOVE]       = "quitar bloqueo",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "apagado automático",
    [LANG_SETTING_CLICKER]   = "clicker",
    [LANG_SETTING_LEGAL]     = "avisos legales",
    [LANG_SETTING_REPLAYGAIN] = "ajuste de volumen",
    [LANG_REPLAYGAIN_TRACK]  = "por pista",
    [LANG_REPLAYGAIN_ALBUM]  = "por álbum",
    [LANG_LEGAL_BODY] =
        "moonlit.aura es software libre.\n"
        "\n"
        "Está basado en Rockbox y hereda su licencia: GNU General "
        "Public License, versión 2 o posterior. Se distribuye SIN "
        "NINGUNA GARANTÍA.\n"
        "\n"
        "El código fuente completo, incluidas las modificaciones a "
        "Rockbox, está en:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox: rockbox.org\n"
        "\n"
        "Tipografías:\n"
        "Libre Baskerville y Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Iconos:\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod es marca registrada de Apple Inc. moonlit.aura no está "
        "afiliado a Apple ni cuenta con su respaldo.",
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
    [LANG_VALUE_DARK]       = "night",
    [LANG_VALUE_LIGHT]      = "dawn",

    [LANG_ACCENT_MOONSTONE] = "moonstone",
    [LANG_ACCENT_TIDE]      = "tide",
    [LANG_ACCENT_EMBER]     = "ember",
    [LANG_ACCENT_MOSS]      = "moss",

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

    [LANG_NP_OPTIONS_TITLE] = "options",
    [LANG_NP_SHUFFLE]       = "shuffle",
    [LANG_NP_REPEAT]        = "repeat",
    [LANG_VALUE_ON]         = "on",
    [LANG_VALUE_OFF]        = "off",
    [LANG_REPEAT_ALL]       = "all",
    [LANG_REPEAT_ONE]       = "one",

    [LANG_SYNC_ERROR_VERSION]  = "this moonlit version is older than aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "could not update the library",
    [LANG_SYNC_DISMISS_HINT]   = "menu to continue",
    [LANG_SYNC_ART_ALBUMS]     = "preparing album art %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "preparing artist photos %d",
    [LANG_SYNC_ART_PHOTOS]     = "preparing images %d",

    [LANG_SETTING_LIBRARY]      = "update library",
    [LANG_SETTING_BRIGHTNESS]   = "brightness",
    [LANG_SETTING_BACKLIGHT]    = "backlight",
    [LANG_VALUE_NEVER]          = "never",
    [LANG_DIALOG_LIBRARY_TITLE] = "update library now?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "it can take several minutes, depending on your file count and the state of the disk.",
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

    [LANG_SETTING_VOLUME_LIMIT] = "volume limit",

    [LANG_LIST_TRUNCATED] = "…and more: the list is full",

    [LANG_VALUE_NOT_INSTALLED] = "not installed",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "created by ricardo gómez\n"
        "based on rockbox, gpl v2\n"
        "moonlit inherits that license\n"
        "source code:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "typography and icons:\n"
        "libre baskerville (sil ofl)\n"
        "montserrat (sil ofl)\n"
        "material symbols (apache 2.0)\n"
        "ipod is a trademark of apple inc.\n"
        "moonlit is not affiliated with apple",
    [LANG_MAREA_TITLE]        = "tide",
    [LANG_MAREA_EMPTY]        = "no albums in the library",
    [LANG_MAREA_SONGS_FMT]    = "%d songs",

    /* moonlit (D-047): switch system */
    [LANG_SETTING_SWITCH_SYSTEM] = "switch system",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "switch to %s and restart?",

    /* moonlit (D-049): preparing library */
    [LANG_LIBRARY_PREPARING] = "preparing library",
    [LANG_LIBRARY_PHASE_DB]  = "building the music database",
    [LANG_LIBRARY_COUNT_FMT] = "%d of %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "version %s",
    [LANG_ABOUT_STACK_FMT]   = "main stack %d %%",
    [LANG_ABOUT_STACK_NA]    = "main stack n/a",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "turn on",
    [LANG_LOCK_CHANGE]       = "change code",
    [LANG_LOCK_REQUIRE]      = "ask for code",
    [LANG_LOCK_REQUIRE_HOLD] = "when locking",
    [LANG_LOCK_REQUIRE_1MIN] = "after 1 minute",
    [LANG_LOCK_REQUIRE_5MIN] = "after 5 minutes",
    [LANG_LOCK_REQUIRE_BOOT] = "only at power-on",
    [LANG_LOCK_REMOVE]       = "remove lock",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "auto power-off",
    [LANG_SETTING_CLICKER]   = "clicker",
    [LANG_SETTING_LEGAL]     = "legal notices",
    [LANG_SETTING_REPLAYGAIN] = "volume levelling",
    [LANG_REPLAYGAIN_TRACK]  = "per track",
    [LANG_REPLAYGAIN_ALBUM]  = "per album",
    [LANG_LEGAL_BODY] =
        "moonlit.aura is free software.\n"
        "\n"
        "It is based on Rockbox and inherits its licence: GNU General "
        "Public License, version 2 or later. It comes with ABSOLUTELY "
        "NO WARRANTY.\n"
        "\n"
        "The complete source code, including the modifications to "
        "Rockbox, is at:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox: rockbox.org\n"
        "\n"
        "Typefaces:\n"
        "Libre Baskerville and Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Icons:\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod is a trademark of Apple Inc. moonlit.aura is not "
        "affiliated with or endorsed by Apple.",
};

/* moonlit (D-080, maestro SS D.1): regla de tipografia francesa --
 * el espacio fino antes de "?"/":" se sustituye por un espacio normal
 * (no hay glifo de espacio fino en las fuentes de moonlit, y el maestro
 * pide justo eso en vez de agregar uno). */
static const char *const strings_fr[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "musique",
    [LANG_HUB_VIDEOS]   = "vidéos",
    [LANG_HUB_PHOTOS]   = "photos",
    [LANG_HUB_SETTINGS] = "réglages",

    [LANG_PIVOT_ARTISTS]   = "artistes",
    [LANG_PIVOT_ALBUMS]    = "albums",
    [LANG_PIVOT_SONGS]     = "titres",
    [LANG_PIVOT_GENRES]    = "genres",
    [LANG_PIVOT_PLAYLISTS] = "listes de lecture",

    [LANG_PIVOT_ALL]    = "tout",
    [LANG_PIVOT_MOVIES] = "films",
    [LANG_PIVOT_SERIES] = "séries",
    [LANG_PIVOT_CLIPS]  = "clips",

    [LANG_PIVOT_PHOTOS] = "photos",
    [LANG_PIVOT_IMAGES] = "images",
    [LANG_PIVOT_AI]     = "ia",

    [LANG_PIVOT_GENERAL] = "général",
    [LANG_PIVOT_DISPLAY] = "écran",
    [LANG_PIVOT_ABOUT]   = "à propos",

    [LANG_SETTING_LANGUAGE] = "langue",
    [LANG_SETTING_THEME]    = "thème",
    [LANG_SETTING_ACCENT]   = "accent",
    [LANG_SETTING_RESET]    = "réinitialiser les réglages",
    [LANG_VALUE_SPANISH]    = "espagnol",
    [LANG_VALUE_ENGLISH]    = "anglais",
    [LANG_VALUE_DARK]       = "nuit",
    [LANG_VALUE_LIGHT]      = "aube",

    [LANG_ACCENT_MOONSTONE] = "pierre de lune",
    [LANG_ACCENT_TIDE]      = "marée",
    [LANG_ACCENT_EMBER]     = "braise",
    [LANG_ACCENT_MOSS]      = "mousse",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "basé sur rockbox",

    [LANG_DIALOG_RESET_TITLE] = "réinitialiser les réglages ?",
    [LANG_DIALOG_YES]         = "oui",
    [LANG_DIALOG_NO]          = "non",

    [LANG_MUSIC_DB_UPDATING] = "mise à jour de la bibliothèque...",
    [LANG_HUB_NOWPLAYING]    = "en cours de lecture",
    [LANG_UNKNOWN_ARTIST]    = "artiste inconnu",
    [LANG_UNKNOWN_ALBUM]     = "album inconnu",
    [LANG_UNKNOWN_GENRE]     = "genre inconnu",
    [LANG_UNKNOWN_TITLE]     = "titre inconnu",

    [LANG_NP_OPTIONS_TITLE] = "options",
    [LANG_NP_SHUFFLE]       = "aléatoire",
    [LANG_NP_REPEAT]        = "répéter",
    [LANG_VALUE_ON]         = "activé",
    [LANG_VALUE_OFF]        = "désactivé",
    [LANG_REPEAT_ALL]       = "tout",
    [LANG_REPEAT_ONE]       = "un",

    [LANG_SYNC_ERROR_VERSION]  = "cette version de moonlit est plus ancienne qu'aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "impossible de mettre à jour la bibliothèque",
    [LANG_SYNC_DISMISS_HINT]   = "menu pour continuer",
    [LANG_SYNC_ART_ALBUMS]     = "préparation des pochettes %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "préparation des photos d'artistes %d",
    [LANG_SYNC_ART_PHOTOS]     = "préparation des images %d",

    [LANG_SETTING_LIBRARY]      = "mettre à jour la bibliothèque",
    [LANG_SETTING_BRIGHTNESS]   = "luminosité",
    [LANG_SETTING_BACKLIGHT]    = "rétroéclairage",
    [LANG_VALUE_NEVER]          = "jamais",
    [LANG_DIALOG_LIBRARY_TITLE] = "mettre à jour la bibliothèque maintenant ?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "cela peut prendre plusieurs minutes, selon le nombre de fichiers et l'état du disque.",
    [LANG_LIBRARY_UPDATING]     = "mise à jour...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "mon ipod",
    [LANG_ABOUT_SONGS]          = "titres",
    [LANG_ABOUT_NOT_SYNCED]     = "pas encore synchronisé",
    [LANG_ABOUT_PLAYLISTS]      = "listes de lecture",
    [LANG_ABOUT_MOVIES]         = "films",
    [LANG_ABOUT_SERIES]         = "séries",
    [LANG_ABOUT_CLIPS]          = "clips vidéo",
    [LANG_ABOUT_IMAGES]         = "images",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "photos",
    [LANG_ABOUT_AI]             = "ia",

    [LANG_USB_CONNECTED] = "connecté",
    [LANG_SHUTTING_DOWN] = "extinction...",

    [LANG_EMPTY_LIST] = "rien ici pour l'instant -- synchronise avec aura studio",

    [LANG_SETTING_ANIMATIONS] = "animation",
    [LANG_SETTING_GRAPHICS]   = "graphismes",
    [LANG_ANIM_ALL]     = "complète",
    [LANG_ANIM_MINIMAL] = "minimale",
    [LANG_ANIM_OFF]     = "désactivée",
    [LANG_GFX_FULL] = "complets",
    [LANG_GFX_LITE] = "légers",

    [LANG_PHOTO_LOADING]     = "chargement...",
    [LANG_PHOTO_UNSUPPORTED] = "format non pris en charge",

    [LANG_NP_LYRICS]           = "paroles",
    [LANG_VALUE_UNAVAILABLE]   = "indisponible",

    [LANG_PIVOT_QUICKPLAY]     = "lecture rapide",
    [LANG_QUICKPLAY_EMPTY]     = "aucun historique pour l'instant -- lis quelque chose d'abord",

    [LANG_NP_RATING]           = "note",

    [LANG_SETTING_LOCK]         = "verrouillage",
    [LANG_LOCK_TITLE_LOCKED]    = "verrouillé",
    [LANG_LOCK_TITLE_SET]       = "nouveau code",
    [LANG_LOCK_TITLE_CONFIRM]   = "confirme",
    [LANG_LOCK_HINT_UNLOCK]     = "compose ton code avec la molette",
    [LANG_LOCK_HINT_WRONG]      = "code incorrect -- réessaie",
    [LANG_LOCK_HINT_SET]        = "choisis 4 chiffres avec la molette",
    [LANG_LOCK_HINT_CONFIRM]    = "compose-le à nouveau pour confirmer",
    [LANG_LOCK_HINT_MISMATCH]   = "ils ne correspondent pas -- recommence",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "retirer le verrouillage ?",

    [LANG_SETTING_SLEEP]      = "minuterie de veille",
    [LANG_SETTING_EQ]         = "égaliseur",
    [LANG_EQ_FLAT]            = "plat",
    [LANG_EQ_BASS]            = "graves",
    [LANG_EQ_VOCAL]           = "voix",
    [LANG_EQ_BRIGHT]          = "brillant",

    [LANG_SETTING_VOLUME_LIMIT] = "limite de volume",

    [LANG_LIST_TRUNCATED] = "…et plus : la liste est pleine",

    [LANG_VALUE_NOT_INSTALLED] = "non installé",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "créé par ricardo gómez\n"
        "basé sur rockbox, gpl v2\n"
        "moonlit hérite de cette licence\n"
        "code source :\n"
        "github.com/ricolinos/moonlit-aura\n"
        "typographie et icônes :\n"
        "libre baskerville (sil ofl)\n"
        "montserrat (sil ofl)\n"
        "material symbols (apache 2.0)\n"
        "ipod est une marque d'apple inc.\n"
        "moonlit n'est pas affilié à apple",
    [LANG_MAREA_TITLE]        = "marée",
    [LANG_MAREA_EMPTY]        = "aucun album dans la bibliothèque",
    [LANG_MAREA_SONGS_FMT]    = "%d titres",

    /* moonlit (D-047): changer de système */
    [LANG_SETTING_SWITCH_SYSTEM] = "changer de système",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "passer à %s et redémarrer ?",

    /* moonlit (D-049): préparation de la bibliothèque */
    [LANG_LIBRARY_PREPARING] = "préparation de la bibliothèque",
    [LANG_LIBRARY_PHASE_DB]  = "construction de la base musicale",
    [LANG_LIBRARY_COUNT_FMT] = "%d sur %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "version %s",
    [LANG_ABOUT_STACK_FMT]   = "pile principale %d %%",
    [LANG_ABOUT_STACK_NA]    = "pile principale n/d",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "activer",
    [LANG_LOCK_CHANGE]       = "changer le code",
    [LANG_LOCK_REQUIRE]      = "demander le code",
    [LANG_LOCK_REQUIRE_HOLD] = "au verrouillage",
    [LANG_LOCK_REQUIRE_1MIN] = "après 1 minute",
    [LANG_LOCK_REQUIRE_5MIN] = "après 5 minutes",
    [LANG_LOCK_REQUIRE_BOOT] = "seulement au démarrage",
    [LANG_LOCK_REMOVE]       = "retirer le verrouillage",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "extinction automatique",
    [LANG_SETTING_CLICKER]   = "clic des touches",
    [LANG_SETTING_LEGAL]     = "mentions légales",
    [LANG_SETTING_REPLAYGAIN] = "égalisation du volume",
    [LANG_REPLAYGAIN_TRACK]  = "par titre",
    [LANG_REPLAYGAIN_ALBUM]  = "par album",
    [LANG_LEGAL_BODY] =
        "moonlit.aura est un logiciel libre.\n"
        "\n"
        "Il est basé sur Rockbox et hérite de sa licence : GNU General "
        "Public License, version 2 ou ultérieure. Il est distribué SANS "
        "AUCUNE GARANTIE.\n"
        "\n"
        "Le code source complet, y compris les modifications apportées "
        "à Rockbox, se trouve sur :\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox : rockbox.org\n"
        "\n"
        "Typographies :\n"
        "Libre Baskerville et Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Icônes :\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod est une marque déposée d'Apple Inc. moonlit.aura n'est ni "
        "affilié à Apple ni approuvé par elle.",
};

/* moonlit (D-080, maestro SS D.1): reglas del aleman -- los sustantivos
 * (comunes y propios) llevan mayuscula inicial, como manda su
 * ortografia; lo demas sigue el mismo tono discreto que el resto de
 * las tablas. */
static const char *const strings_de[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "Musik",
    [LANG_HUB_VIDEOS]   = "Videos",
    [LANG_HUB_PHOTOS]   = "Fotos",
    [LANG_HUB_SETTINGS] = "Einstellungen",

    [LANG_PIVOT_ARTISTS]   = "Interpreten",
    [LANG_PIVOT_ALBUMS]    = "Alben",
    [LANG_PIVOT_SONGS]     = "Titel",
    [LANG_PIVOT_GENRES]    = "Genres",
    [LANG_PIVOT_PLAYLISTS] = "Playlists",

    [LANG_PIVOT_ALL]    = "Alle",
    [LANG_PIVOT_MOVIES] = "Filme",
    [LANG_PIVOT_SERIES] = "Serien",
    [LANG_PIVOT_CLIPS]  = "Clips",

    [LANG_PIVOT_PHOTOS] = "Fotos",
    [LANG_PIVOT_IMAGES] = "Bilder",
    [LANG_PIVOT_AI]     = "KI",

    [LANG_PIVOT_GENERAL] = "Allgemein",
    [LANG_PIVOT_DISPLAY] = "Anzeige",
    [LANG_PIVOT_ABOUT]   = "Info",

    [LANG_SETTING_LANGUAGE] = "Sprache",
    [LANG_SETTING_THEME]    = "Thema",
    [LANG_SETTING_ACCENT]   = "Akzent",
    [LANG_SETTING_RESET]    = "Einstellungen zurücksetzen",
    [LANG_VALUE_SPANISH]    = "Spanisch",
    [LANG_VALUE_ENGLISH]    = "Englisch",
    [LANG_VALUE_DARK]       = "Nacht",
    [LANG_VALUE_LIGHT]      = "Morgendämmerung",

    [LANG_ACCENT_MOONSTONE] = "Mondstein",
    [LANG_ACCENT_TIDE]      = "Gezeiten",
    [LANG_ACCENT_EMBER]     = "Glut",
    [LANG_ACCENT_MOSS]      = "Moos",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "basiert auf Rockbox",

    [LANG_DIALOG_RESET_TITLE] = "Einstellungen zurücksetzen?",
    [LANG_DIALOG_YES]         = "Ja",
    [LANG_DIALOG_NO]          = "Nein",

    [LANG_MUSIC_DB_UPDATING] = "Bibliothek wird aktualisiert...",
    [LANG_HUB_NOWPLAYING]    = "Wird wiedergegeben",
    [LANG_UNKNOWN_ARTIST]    = "Unbekannter Interpret",
    [LANG_UNKNOWN_ALBUM]     = "Unbekanntes Album",
    [LANG_UNKNOWN_GENRE]     = "Unbekanntes Genre",
    [LANG_UNKNOWN_TITLE]     = "Unbekannter Titel",

    [LANG_NP_OPTIONS_TITLE] = "Optionen",
    [LANG_NP_SHUFFLE]       = "Zufall",
    [LANG_NP_REPEAT]        = "Wiederholen",
    [LANG_VALUE_ON]         = "an",
    [LANG_VALUE_OFF]        = "aus",
    [LANG_REPEAT_ALL]       = "alle",
    [LANG_REPEAT_ONE]       = "eins",

    [LANG_SYNC_ERROR_VERSION]  = "diese moonlit-Version ist älter als Aura Studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "Bibliothek konnte nicht aktualisiert werden",
    [LANG_SYNC_DISMISS_HINT]   = "Menü zum Fortfahren",
    [LANG_SYNC_ART_ALBUMS]     = "Cover werden vorbereitet %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "Interpretenfotos werden vorbereitet %d",
    [LANG_SYNC_ART_PHOTOS]     = "Bilder werden vorbereitet %d",

    [LANG_SETTING_LIBRARY]      = "Bibliothek aktualisieren",
    [LANG_SETTING_BRIGHTNESS]   = "Helligkeit",
    [LANG_SETTING_BACKLIGHT]    = "Beleuchtung",
    [LANG_VALUE_NEVER]          = "nie",
    [LANG_DIALOG_LIBRARY_TITLE] = "Bibliothek jetzt aktualisieren?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "je nach Dateianzahl und Zustand der Festplatte kann dies mehrere Minuten dauern.",
    [LANG_LIBRARY_UPDATING]     = "Wird aktualisiert...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "Mein iPod",
    [LANG_ABOUT_SONGS]          = "Titel",
    [LANG_ABOUT_NOT_SYNCED]     = "noch nicht synchronisiert",
    [LANG_ABOUT_PLAYLISTS]      = "Playlists",
    [LANG_ABOUT_MOVIES]         = "Filme",
    [LANG_ABOUT_SERIES]         = "Serien",
    [LANG_ABOUT_CLIPS]          = "Videoclips",
    [LANG_ABOUT_IMAGES]         = "Bilder",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "Fotos",
    [LANG_ABOUT_AI]             = "KI",

    [LANG_USB_CONNECTED] = "Verbunden",
    [LANG_SHUTTING_DOWN] = "Wird heruntergefahren...",

    [LANG_EMPTY_LIST] = "hier ist noch nichts -- mit Aura Studio synchronisieren",

    [LANG_SETTING_ANIMATIONS] = "Animation",
    [LANG_SETTING_GRAPHICS]   = "Grafik",
    [LANG_ANIM_ALL]     = "voll",
    [LANG_ANIM_MINIMAL] = "minimal",
    [LANG_ANIM_OFF]     = "aus",
    [LANG_GFX_FULL] = "voll",
    [LANG_GFX_LITE] = "einfach",

    [LANG_PHOTO_LOADING]     = "Wird geladen...",
    [LANG_PHOTO_UNSUPPORTED] = "nicht unterstütztes Format",

    [LANG_NP_LYRICS]           = "Songtext",
    [LANG_VALUE_UNAVAILABLE]   = "nicht verfügbar",

    [LANG_PIVOT_QUICKPLAY]     = "Direktwiedergabe",
    [LANG_QUICKPLAY_EMPTY]     = "noch kein Verlauf -- zuerst etwas abspielen",

    [LANG_NP_RATING]           = "Bewertung",

    [LANG_SETTING_LOCK]         = "Sperre",
    [LANG_LOCK_TITLE_LOCKED]    = "Gesperrt",
    [LANG_LOCK_TITLE_SET]       = "Neuer Code",
    [LANG_LOCK_TITLE_CONFIRM]   = "Bestätigen",
    [LANG_LOCK_HINT_UNLOCK]     = "Code mit dem Rad eingeben",
    [LANG_LOCK_HINT_WRONG]      = "falscher Code -- erneut versuchen",
    [LANG_LOCK_HINT_SET]        = "4 Ziffern mit dem Rad wählen",
    [LANG_LOCK_HINT_CONFIRM]    = "zur Bestätigung erneut eingeben",
    [LANG_LOCK_HINT_MISMATCH]   = "stimmt nicht überein -- von vorn beginnen",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "Sperre entfernen?",

    [LANG_SETTING_SLEEP]      = "Sleep-Timer",
    [LANG_SETTING_EQ]         = "Equalizer",
    [LANG_EQ_FLAT]            = "linear",
    [LANG_EQ_BASS]            = "Bass",
    [LANG_EQ_VOCAL]           = "Stimme",
    [LANG_EQ_BRIGHT]          = "hell",

    [LANG_SETTING_VOLUME_LIMIT] = "Lautstärkegrenze",

    [LANG_LIST_TRUNCATED] = "…und mehr: die Liste ist voll",

    [LANG_VALUE_NOT_INSTALLED] = "nicht installiert",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "erstellt von Ricardo Gómez\n"
        "basiert auf Rockbox, GPL v2\n"
        "moonlit übernimmt diese Lizenz\n"
        "Quellcode:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "Typografie und Icons:\n"
        "Libre Baskerville (SIL OFL)\n"
        "Montserrat (SIL OFL)\n"
        "Material Symbols (Apache 2.0)\n"
        "iPod ist eine Marke von Apple Inc.\n"
        "moonlit steht in keiner Verbindung zu Apple",
    [LANG_MAREA_TITLE]        = "Gezeiten",
    [LANG_MAREA_EMPTY]        = "keine Alben in der Bibliothek",
    [LANG_MAREA_SONGS_FMT]    = "%d Titel",

    /* moonlit (D-047): System wechseln */
    [LANG_SETTING_SWITCH_SYSTEM] = "System wechseln",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "zu %s wechseln und neu starten?",

    /* moonlit (D-049): Bibliothek wird vorbereitet */
    [LANG_LIBRARY_PREPARING] = "Bibliothek wird vorbereitet",
    [LANG_LIBRARY_PHASE_DB]  = "Musikdatenbank wird erstellt",
    [LANG_LIBRARY_COUNT_FMT] = "%d von %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "Version %s",
    [LANG_ABOUT_STACK_FMT]   = "Hauptstapel %d %%",
    [LANG_ABOUT_STACK_NA]    = "Hauptstapel n/v",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "Aktivieren",
    [LANG_LOCK_CHANGE]       = "Code ändern",
    [LANG_LOCK_REQUIRE]      = "Code abfragen",
    [LANG_LOCK_REQUIRE_HOLD] = "beim Sperren",
    [LANG_LOCK_REQUIRE_1MIN] = "nach 1 Minute",
    [LANG_LOCK_REQUIRE_5MIN] = "nach 5 Minuten",
    [LANG_LOCK_REQUIRE_BOOT] = "nur beim Einschalten",
    [LANG_LOCK_REMOVE]       = "Sperre entfernen",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "automatische Abschaltung",
    [LANG_SETTING_CLICKER]   = "Tastenklick",
    [LANG_SETTING_LEGAL]     = "rechtliche Hinweise",
    [LANG_SETTING_REPLAYGAIN] = "Lautstärkeanpassung",
    [LANG_REPLAYGAIN_TRACK]  = "pro Titel",
    [LANG_REPLAYGAIN_ALBUM]  = "pro Album",
    [LANG_LEGAL_BODY] =
        "moonlit.aura ist freie Software.\n"
        "\n"
        "Es basiert auf Rockbox und übernimmt dessen Lizenz: GNU General "
        "Public License, Version 2 oder später. Es wird OHNE JEGLICHE "
        "GEWÄHRLEISTUNG bereitgestellt.\n"
        "\n"
        "Der vollständige Quellcode, einschließlich der Änderungen an "
        "Rockbox, befindet sich unter:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox: rockbox.org\n"
        "\n"
        "Schriftarten:\n"
        "Libre Baskerville und Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Symbole:\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod ist eine eingetragene Marke von Apple Inc. moonlit.aura "
        "steht in keiner Verbindung zu Apple und wird nicht von Apple "
        "unterstützt.",
};

/* moonlit (D-080, maestro SS D.1): cirilico completo -- verificado con
 * check_fonts.py --coverage contra U+0400-U+045F antes de cerrar D-081. */
static const char *const strings_ru[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "музыка",
    [LANG_HUB_VIDEOS]   = "видео",
    [LANG_HUB_PHOTOS]   = "фото",
    [LANG_HUB_SETTINGS] = "настройки",

    [LANG_PIVOT_ARTISTS]   = "исполнители",
    [LANG_PIVOT_ALBUMS]    = "альбомы",
    [LANG_PIVOT_SONGS]     = "песни",
    [LANG_PIVOT_GENRES]    = "жанры",
    [LANG_PIVOT_PLAYLISTS] = "плейлисты",

    [LANG_PIVOT_ALL]    = "все",
    [LANG_PIVOT_MOVIES] = "фильмы",
    [LANG_PIVOT_SERIES] = "сериалы",
    [LANG_PIVOT_CLIPS]  = "клипы",

    [LANG_PIVOT_PHOTOS] = "фото",
    [LANG_PIVOT_IMAGES] = "изображения",
    [LANG_PIVOT_AI]     = "ии",

    [LANG_PIVOT_GENERAL] = "общие",
    [LANG_PIVOT_DISPLAY] = "экран",
    [LANG_PIVOT_ABOUT]   = "о программе",

    [LANG_SETTING_LANGUAGE] = "язык",
    [LANG_SETTING_THEME]    = "тема",
    [LANG_SETTING_ACCENT]   = "акцент",
    [LANG_SETTING_RESET]    = "сбросить настройки",
    [LANG_VALUE_SPANISH]    = "испанский",
    [LANG_VALUE_ENGLISH]    = "английский",
    [LANG_VALUE_DARK]       = "ночь",
    [LANG_VALUE_LIGHT]      = "рассвет",

    [LANG_ACCENT_MOONSTONE] = "лунный камень",
    [LANG_ACCENT_TIDE]      = "прилив",
    [LANG_ACCENT_EMBER]     = "уголёк",
    [LANG_ACCENT_MOSS]      = "мох",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "на основе rockbox",

    [LANG_DIALOG_RESET_TITLE] = "сбросить настройки?",
    [LANG_DIALOG_YES]         = "да",
    [LANG_DIALOG_NO]          = "нет",

    [LANG_MUSIC_DB_UPDATING] = "обновление библиотеки...",
    [LANG_HUB_NOWPLAYING]    = "сейчас играет",
    [LANG_UNKNOWN_ARTIST]    = "неизвестный исполнитель",
    [LANG_UNKNOWN_ALBUM]     = "неизвестный альбом",
    [LANG_UNKNOWN_GENRE]     = "неизвестный жанр",
    [LANG_UNKNOWN_TITLE]     = "неизвестное название",

    [LANG_NP_OPTIONS_TITLE] = "параметры",
    [LANG_NP_SHUFFLE]       = "перемешать",
    [LANG_NP_REPEAT]        = "повтор",
    [LANG_VALUE_ON]         = "вкл",
    [LANG_VALUE_OFF]        = "выкл",
    [LANG_REPEAT_ALL]       = "все",
    [LANG_REPEAT_ONE]       = "один",

    [LANG_SYNC_ERROR_VERSION]  = "эта версия moonlit старше, чем aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "не удалось обновить библиотеку",
    [LANG_SYNC_DISMISS_HINT]   = "меню, чтобы продолжить",
    [LANG_SYNC_ART_ALBUMS]     = "подготовка обложек %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "подготовка фото исполнителей %d",
    [LANG_SYNC_ART_PHOTOS]     = "подготовка изображений %d",

    [LANG_SETTING_LIBRARY]      = "обновить библиотеку",
    [LANG_SETTING_BRIGHTNESS]   = "яркость",
    [LANG_SETTING_BACKLIGHT]    = "подсветка",
    [LANG_VALUE_NEVER]          = "никогда",
    [LANG_DIALOG_LIBRARY_TITLE] = "обновить библиотеку сейчас?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "это может занять несколько минут, в зависимости от количества файлов и состояния диска.",
    [LANG_LIBRARY_UPDATING]     = "обновление...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "мой ipod",
    [LANG_ABOUT_SONGS]          = "песни",
    [LANG_ABOUT_NOT_SYNCED]     = "ещё не синхронизировано",
    [LANG_ABOUT_PLAYLISTS]      = "плейлисты",
    [LANG_ABOUT_MOVIES]         = "фильмы",
    [LANG_ABOUT_SERIES]         = "сериалы",
    [LANG_ABOUT_CLIPS]          = "видеоклипы",
    [LANG_ABOUT_IMAGES]         = "изображения",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "фотографии",
    [LANG_ABOUT_AI]             = "ии",

    [LANG_USB_CONNECTED] = "подключено",
    [LANG_SHUTTING_DOWN] = "выключение...",

    [LANG_EMPTY_LIST] = "здесь пока пусто -- синхронизируйте с aura studio",

    [LANG_SETTING_ANIMATIONS] = "анимация",
    [LANG_SETTING_GRAPHICS]   = "графика",
    [LANG_ANIM_ALL]     = "полная",
    [LANG_ANIM_MINIMAL] = "минимальная",
    [LANG_ANIM_OFF]     = "выключена",
    [LANG_GFX_FULL] = "полная",
    [LANG_GFX_LITE] = "облегчённая",

    [LANG_PHOTO_LOADING]     = "загрузка...",
    [LANG_PHOTO_UNSUPPORTED] = "формат не поддерживается",

    [LANG_NP_LYRICS]           = "текст песни",
    [LANG_VALUE_UNAVAILABLE]   = "недоступно",

    [LANG_PIVOT_QUICKPLAY]     = "быстрое воспроизведение",
    [LANG_QUICKPLAY_EMPTY]     = "истории пока нет -- сначала что-нибудь включите",

    [LANG_NP_RATING]           = "оценка",

    [LANG_SETTING_LOCK]         = "блокировка",
    [LANG_LOCK_TITLE_LOCKED]    = "заблокировано",
    [LANG_LOCK_TITLE_SET]       = "новый код",
    [LANG_LOCK_TITLE_CONFIRM]   = "подтвердите",
    [LANG_LOCK_HINT_UNLOCK]     = "наберите код колесом",
    [LANG_LOCK_HINT_WRONG]      = "неверный код -- попробуйте снова",
    [LANG_LOCK_HINT_SET]        = "выберите 4 цифры колесом",
    [LANG_LOCK_HINT_CONFIRM]    = "наберите ещё раз для подтверждения",
    [LANG_LOCK_HINT_MISMATCH]   = "коды не совпали -- начните заново",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "снять блокировку?",

    [LANG_SETTING_SLEEP]      = "таймер сна",
    [LANG_SETTING_EQ]         = "эквалайзер",
    [LANG_EQ_FLAT]            = "плоский",
    [LANG_EQ_BASS]            = "бас",
    [LANG_EQ_VOCAL]           = "вокал",
    [LANG_EQ_BRIGHT]          = "яркий",

    [LANG_SETTING_VOLUME_LIMIT] = "ограничение громкости",

    [LANG_LIST_TRUNCATED] = "…и другие: список заполнен",

    [LANG_VALUE_NOT_INSTALLED] = "не установлено",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "создано ricardo gómez\n"
        "на основе rockbox, gpl v2\n"
        "moonlit наследует эту лицензию\n"
        "исходный код:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "шрифты и значки:\n"
        "libre baskerville (sil ofl)\n"
        "montserrat (sil ofl)\n"
        "material symbols (apache 2.0)\n"
        "ipod -- товарный знак apple inc.\n"
        "moonlit не связан с apple",
    [LANG_MAREA_TITLE]        = "прилив",
    [LANG_MAREA_EMPTY]        = "в библиотеке нет альбомов",
    [LANG_MAREA_SONGS_FMT]    = "%d песен",

    /* moonlit (D-047): сменить систему */
    [LANG_SETTING_SWITCH_SYSTEM] = "сменить систему",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "перейти на %s и перезагрузиться?",

    /* moonlit (D-049): подготовка библиотеки */
    [LANG_LIBRARY_PREPARING] = "подготовка библиотеки",
    [LANG_LIBRARY_PHASE_DB]  = "создание базы музыки",
    [LANG_LIBRARY_COUNT_FMT] = "%d из %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "версия %s",
    [LANG_ABOUT_STACK_FMT]   = "основной стек %d %%",
    [LANG_ABOUT_STACK_NA]    = "основной стек н/д",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "включить",
    [LANG_LOCK_CHANGE]       = "изменить код",
    [LANG_LOCK_REQUIRE]      = "запрашивать код",
    [LANG_LOCK_REQUIRE_HOLD] = "при блокировке",
    [LANG_LOCK_REQUIRE_1MIN] = "через 1 минуту",
    [LANG_LOCK_REQUIRE_5MIN] = "через 5 минут",
    [LANG_LOCK_REQUIRE_BOOT] = "только при включении",
    [LANG_LOCK_REMOVE]       = "снять блокировку",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "автовыключение",
    [LANG_SETTING_CLICKER]   = "щелчок клавиш",
    [LANG_SETTING_LEGAL]     = "правовая информация",
    [LANG_SETTING_REPLAYGAIN] = "выравнивание громкости",
    [LANG_REPLAYGAIN_TRACK]  = "по треку",
    [LANG_REPLAYGAIN_ALBUM]  = "по альбому",
    [LANG_LEGAL_BODY] =
        "moonlit.aura -- свободное программное обеспечение.\n"
        "\n"
        "Оно основано на Rockbox и наследует его лицензию: GNU General "
        "Public License версии 2 или более поздней. Оно распространяется "
        "БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ.\n"
        "\n"
        "Полный исходный код, включая изменения в Rockbox, находится по "
        "адресу:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox: rockbox.org\n"
        "\n"
        "Шрифты:\n"
        "Libre Baskerville и Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Значки:\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod -- зарегистрированный товарный знак Apple Inc. moonlit.aura "
        "не связан с Apple и не одобрен компанией Apple.",
};

static const char *const strings_it[LANG_COUNT] = {
    [LANG_HUB_MUSIC]    = "musica",
    [LANG_HUB_VIDEOS]   = "video",
    [LANG_HUB_PHOTOS]   = "foto",
    [LANG_HUB_SETTINGS] = "impostazioni",

    [LANG_PIVOT_ARTISTS]   = "artisti",
    [LANG_PIVOT_ALBUMS]    = "album",
    [LANG_PIVOT_SONGS]     = "brani",
    [LANG_PIVOT_GENRES]    = "generi",
    [LANG_PIVOT_PLAYLISTS] = "playlist",

    [LANG_PIVOT_ALL]    = "tutti",
    [LANG_PIVOT_MOVIES] = "film",
    [LANG_PIVOT_SERIES] = "serie",
    [LANG_PIVOT_CLIPS]  = "clip",

    [LANG_PIVOT_PHOTOS] = "foto",
    [LANG_PIVOT_IMAGES] = "immagini",
    [LANG_PIVOT_AI]     = "ia",

    [LANG_PIVOT_GENERAL] = "generali",
    [LANG_PIVOT_DISPLAY] = "schermo",
    [LANG_PIVOT_ABOUT]   = "informazioni",

    [LANG_SETTING_LANGUAGE] = "lingua",
    [LANG_SETTING_THEME]    = "tema",
    [LANG_SETTING_ACCENT]   = "accento",
    [LANG_SETTING_RESET]    = "ripristina impostazioni",
    [LANG_VALUE_SPANISH]    = "spagnolo",
    [LANG_VALUE_ENGLISH]    = "inglese",
    [LANG_VALUE_DARK]       = "notte",
    [LANG_VALUE_LIGHT]      = "alba",

    [LANG_ACCENT_MOONSTONE] = "pietra di luna",
    [LANG_ACCENT_TIDE]      = "marea",
    [LANG_ACCENT_EMBER]     = "brace",
    [LANG_ACCENT_MOSS]      = "muschio",

    [LANG_ABOUT_BASED_ON_ROCKBOX] = "basato su rockbox",

    [LANG_DIALOG_RESET_TITLE] = "ripristinare le impostazioni?",
    [LANG_DIALOG_YES]         = "sì",
    [LANG_DIALOG_NO]          = "no",

    [LANG_MUSIC_DB_UPDATING] = "aggiornamento della libreria...",
    [LANG_HUB_NOWPLAYING]    = "in riproduzione",
    [LANG_UNKNOWN_ARTIST]    = "artista sconosciuto",
    [LANG_UNKNOWN_ALBUM]     = "album sconosciuto",
    [LANG_UNKNOWN_GENRE]     = "genere sconosciuto",
    [LANG_UNKNOWN_TITLE]     = "titolo sconosciuto",

    [LANG_NP_OPTIONS_TITLE] = "opzioni",
    [LANG_NP_SHUFFLE]       = "casuale",
    [LANG_NP_REPEAT]        = "ripeti",
    [LANG_VALUE_ON]         = "attivo",
    [LANG_VALUE_OFF]        = "disattivo",
    [LANG_REPEAT_ALL]       = "tutti",
    [LANG_REPEAT_ONE]       = "uno",

    [LANG_SYNC_ERROR_VERSION]  = "questa versione di moonlit è più vecchia di aura studio",
    [LANG_SYNC_ERROR_ATTEMPTS] = "impossibile aggiornare la libreria",
    [LANG_SYNC_DISMISS_HINT]   = "menu per continuare",
    [LANG_SYNC_ART_ALBUMS]     = "preparazione copertine %d/%d",
    [LANG_SYNC_ART_ARTISTS]    = "preparazione foto degli artisti %d",
    [LANG_SYNC_ART_PHOTOS]     = "preparazione immagini %d",

    [LANG_SETTING_LIBRARY]      = "aggiorna libreria",
    [LANG_SETTING_BRIGHTNESS]   = "luminosità",
    [LANG_SETTING_BACKLIGHT]    = "retroilluminazione",
    [LANG_VALUE_NEVER]          = "mai",
    [LANG_DIALOG_LIBRARY_TITLE] = "aggiornare la libreria ora?",
    [LANG_DIALOG_LIBRARY_DETAIL] = "può richiedere diversi minuti, a seconda del numero di file e dello stato del disco.",
    [LANG_LIBRARY_UPDATING]     = "aggiornamento...",

    [LANG_ABOUT_DEVICE_DEFAULT] = "il mio ipod",
    [LANG_ABOUT_SONGS]          = "brani",
    [LANG_ABOUT_NOT_SYNCED]     = "non ancora sincronizzato",
    [LANG_ABOUT_PLAYLISTS]      = "playlist",
    [LANG_ABOUT_MOVIES]         = "film",
    [LANG_ABOUT_SERIES]         = "serie",
    [LANG_ABOUT_CLIPS]          = "videoclip",
    [LANG_ABOUT_IMAGES]         = "immagini",
    [LANG_ABOUT_PHOTOS_TAKEN]   = "foto",
    [LANG_ABOUT_AI]             = "ia",

    [LANG_USB_CONNECTED] = "connesso",
    [LANG_SHUTTING_DOWN] = "spegnimento...",

    [LANG_EMPTY_LIST] = "non c'è ancora nulla qui -- sincronizza con aura studio",

    [LANG_SETTING_ANIMATIONS] = "animazione",
    [LANG_SETTING_GRAPHICS]   = "grafica",
    [LANG_ANIM_ALL]     = "completa",
    [LANG_ANIM_MINIMAL] = "minima",
    [LANG_ANIM_OFF]     = "disattivata",
    [LANG_GFX_FULL] = "completa",
    [LANG_GFX_LITE] = "leggera",

    [LANG_PHOTO_LOADING]     = "caricamento...",
    [LANG_PHOTO_UNSUPPORTED] = "formato non supportato",

    [LANG_NP_LYRICS]           = "testo",
    [LANG_VALUE_UNAVAILABLE]   = "non disponibile",

    [LANG_PIVOT_QUICKPLAY]     = "riproduci ora",
    [LANG_QUICKPLAY_EMPTY]     = "ancora nessuna cronologia -- riproduci prima qualcosa",

    [LANG_NP_RATING]           = "valutazione",

    [LANG_SETTING_LOCK]         = "blocco",
    [LANG_LOCK_TITLE_LOCKED]    = "bloccato",
    [LANG_LOCK_TITLE_SET]       = "nuovo codice",
    [LANG_LOCK_TITLE_CONFIRM]   = "conferma",
    [LANG_LOCK_HINT_UNLOCK]     = "componi il codice con la rotella",
    [LANG_LOCK_HINT_WRONG]      = "codice errato -- riprova",
    [LANG_LOCK_HINT_SET]        = "scegli 4 cifre con la rotella",
    [LANG_LOCK_HINT_CONFIRM]    = "componilo di nuovo per confermare",
    [LANG_LOCK_HINT_MISMATCH]   = "non corrispondono -- ricomincia",
    [LANG_DIALOG_LOCK_OFF_TITLE] = "rimuovere il blocco?",

    [LANG_SETTING_SLEEP]      = "timer di spegnimento",
    [LANG_SETTING_EQ]         = "equalizzatore",
    [LANG_EQ_FLAT]            = "piatto",
    [LANG_EQ_BASS]            = "bassi",
    [LANG_EQ_VOCAL]           = "voce",
    [LANG_EQ_BRIGHT]          = "brillante",

    [LANG_SETTING_VOLUME_LIMIT] = "limite di volume",

    [LANG_LIST_TRUNCATED] = "…e altri: l'elenco è pieno",

    [LANG_VALUE_NOT_INSTALLED] = "non installato",

    /* moonlit H1 (D-001, D-002) */
    [LANG_ABOUT_CREDITS_BODY] =
        "moonlit.aura\n"
        "creato da ricardo gómez\n"
        "basato su rockbox, gpl v2\n"
        "moonlit eredita questa licenza\n"
        "codice sorgente:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "tipografia e icone:\n"
        "libre baskerville (sil ofl)\n"
        "montserrat (sil ofl)\n"
        "material symbols (apache 2.0)\n"
        "ipod è un marchio di apple inc.\n"
        "moonlit non è affiliato ad apple",
    [LANG_MAREA_TITLE]        = "marea",
    [LANG_MAREA_EMPTY]        = "nessun album nella libreria",
    [LANG_MAREA_SONGS_FMT]    = "%d brani",

    /* moonlit (D-047): cambia sistema */
    [LANG_SETTING_SWITCH_SYSTEM] = "cambia sistema",
    [LANG_FAMILY_AURA]           = "Aura",
    [LANG_FAMILY_METRO]          = "Metro",
    [LANG_FAMILY_MOONLIT]        = "moonlit.aura",
    [LANG_DIALOG_SWITCH_FMT]     = "passare a %s e riavviare?",

    /* moonlit (D-049): preparazione della libreria */
    [LANG_LIBRARY_PREPARING] = "preparazione della libreria",
    [LANG_LIBRARY_PHASE_DB]  = "creazione del database musicale",
    [LANG_LIBRARY_COUNT_FMT] = "%d di %d",

    /* moonlit (D-062, D-064) */
    [LANG_ABOUT_VERSION_FMT] = "versione %s",
    [LANG_ABOUT_STACK_FMT]   = "stack principale %d %%",
    [LANG_ABOUT_STACK_NA]    = "stack principale n/d",

    /* moonlit (D-069) */
    [LANG_LOCK_ENABLE]       = "attiva",
    [LANG_LOCK_CHANGE]       = "cambia codice",
    [LANG_LOCK_REQUIRE]      = "richiedi codice",
    [LANG_LOCK_REQUIRE_HOLD] = "al blocco",
    [LANG_LOCK_REQUIRE_1MIN] = "dopo 1 minuto",
    [LANG_LOCK_REQUIRE_5MIN] = "dopo 5 minuti",
    [LANG_LOCK_REQUIRE_BOOT] = "solo all'accensione",
    [LANG_LOCK_REMOVE]       = "rimuovi blocco",

    /* moonlit (D-071) */
    [LANG_SETTING_POWEROFF]  = "spegnimento automatico",
    [LANG_SETTING_CLICKER]   = "clic dei tasti",
    [LANG_SETTING_LEGAL]     = "note legali",
    [LANG_SETTING_REPLAYGAIN] = "livellamento del volume",
    [LANG_REPLAYGAIN_TRACK]  = "per brano",
    [LANG_REPLAYGAIN_ALBUM]  = "per album",
    [LANG_LEGAL_BODY] =
        "moonlit.aura è software libero.\n"
        "\n"
        "È basato su Rockbox e ne eredita la licenza: GNU General Public "
        "License, versione 2 o successiva. Viene distribuito SENZA ALCUNA "
        "GARANZIA.\n"
        "\n"
        "Il codice sorgente completo, incluse le modifiche a Rockbox, si "
        "trova su:\n"
        "github.com/ricolinos/moonlit-aura\n"
        "\n"
        "Rockbox: rockbox.org\n"
        "\n"
        "Caratteri tipografici:\n"
        "Libre Baskerville e Montserrat, SIL Open Font License 1.1.\n"
        "\n"
        "Icone:\n"
        "Material Symbols (Google), Apache License 2.0.\n"
        "\n"
        "iPod è un marchio registrato di Apple Inc. moonlit.aura non è "
        "affiliato né sponsorizzato da Apple.",
};

/* moonlit (D-080): indexado por enum metro_language -- agregar un
 * idioma es agregar una entrada aca Y una tabla arriba, nada mas. */
static const char *const *const lang_tables[METRO_LANG_COUNT] = {
    [METRO_LANG_ES] = strings_es,
    [METRO_LANG_EN] = strings_en,
    [METRO_LANG_FR] = strings_fr,
    [METRO_LANG_DE] = strings_de,
    [METRO_LANG_RU] = strings_ru,
    [METRO_LANG_IT] = strings_it,
};

/* moonlit (D-080): nombre nativo, SIEMPRE en su propio idioma --
 * metro_lang_native_name() lo sirve tal cual, sin pasar por
 * lang_tables[]/current_lang. */
static const char *const native_names[METRO_LANG_COUNT] = {
    [METRO_LANG_ES] = "Español",
    [METRO_LANG_EN] = "English",
    [METRO_LANG_FR] = "Français",
    [METRO_LANG_DE] = "Deutsch",
    [METRO_LANG_RU] = "Русский",
    [METRO_LANG_IT] = "Italiano",
};

void metro_lang_set(enum metro_language lang)
{
    if ((unsigned)lang < METRO_LANG_COUNT)
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

    s = lang_tables[current_lang][id];
    return s ? s : "";
}

const char *metro_lang_native_name(enum metro_language lang)
{
    return ((unsigned)lang < METRO_LANG_COUNT) ? native_names[lang] : "";
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

/* Clave de ordenamiento del primer carácter de *s; avanza *s más allá
 * de él. Escala x4 para dejar hueco entre letras contiguas (ahí entra
 * la ñ, entre N y O). */
static int collate_key(const char **s)
{
    const unsigned char *p = (const unsigned char *)*s;
    unsigned char c = p[0];

    if (c < 0x80)
    {
        *s += 1;
        if (c >= 'a' && c <= 'z')
            c -= 32;
        return (int)c * 4;
    }

    /* Latin-1 en UTF-8: 0xC3 seguido del segundo byte. Es el único
     * bloque que necesita plegado para español; cualquier otro
     * multibyte se ordena por su valor crudo, después del ASCII. */
    if (c == 0xC3 && p[1])
    {
        unsigned char b = p[1];
        unsigned char base;

        *s += 2;
        /* Normaliza a minúscula dentro del bloque (0x80-0x9E son las
         * mayúsculas, 0xA0-0xBE las minúsculas: 0x20 de diferencia). */
        if (b >= 0x80 && b <= 0x9E)
            b += 0x20;

        switch (b)
        {
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5:
            base = 'A'; break;                    /* à á â ã ä å */
        case 0xA8: case 0xA9: case 0xAA: case 0xAB:
            base = 'E'; break;                    /* è é ê ë */
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
            base = 'I'; break;                    /* ì í î ï */
        case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6:
            base = 'O'; break;                    /* ò ó ô õ ö */
        case 0xB9: case 0xBA: case 0xBB: case 0xBC:
            base = 'U'; break;                    /* ù ú û ü */
        case 0xA7:
            base = 'C'; break;                    /* ç */
        case 0xB1:
            /* ñ: letra propia, justo después de toda la N. */
            return (int)'N' * 4 + 1;
        default:
            return 0x100 * 4 + (int)b;            /* resto: tras el ASCII */
        }
        return (int)base * 4;
    }

    /* Cualquier otro multibyte (CJK, emoji): valor crudo, después del
     * ASCII. Se avanza un byte -- no hace falta decodificar bien para
     * que el orden sea estable y no se lea de más. */
    *s += 1;
    return 0x100 * 4 + (int)c;
}

void metro_lang_upper(const char *s, char *out, size_t outsz)
{
    size_t used = 0;

    if (outsz == 0)
        return;
    out[0] = '\0';
    if (!s)
        return;

    while (*s)
    {
        char ch[5];
        size_t len;

        /* metro_lang_initial() ya sabe medir y mayusculizar UN carácter
         * UTF-8; aquí solo se encadena sobre toda la cadena. */
        metro_lang_initial(s, ch, sizeof(ch));
        len = strlen(ch);
        if (len == 0)
            break;
        if (used + len + 1 > outsz)
            break; /* no cabe entero: truncar en frontera de carácter */
        memcpy(out + used, ch, len);
        used += len;
        s += len;
    }
    out[used] = '\0';
}

int metro_lang_collate(const char *a, const char *b)
{
    const char *pa = a, *pb = b;

    if (!a || !b)
        return (a ? 1 : 0) - (b ? 1 : 0);

    while (*pa && *pb)
    {
        int ka = collate_key(&pa);
        int kb = collate_key(&pb);

        if (ka != kb)
            return ka - kb;
    }

    if (*pa || *pb)
        return *pa ? 1 : -1;

    /* Empate en el nivel plegado ("Angela" vs "Ángela"): desempate por
     * bytes crudos, para que el orden sea determinista. */
    return strcmp(a, b);
}

int metro_lang_code_to_enum(const char *code)
{
    if (!strcmp(code, "es")) return METRO_LANG_ES;
    if (!strcmp(code, "en")) return METRO_LANG_EN;
    if (!strcmp(code, "fr")) return METRO_LANG_FR;
    if (!strcmp(code, "de")) return METRO_LANG_DE;
    if (!strcmp(code, "ru")) return METRO_LANG_RU;
    if (!strcmp(code, "it")) return METRO_LANG_IT;
    return -1;
}

const char *metro_lang_code_from_enum(enum metro_language lang)
{
    switch (lang)
    {
    case METRO_LANG_ES: return "es";
    case METRO_LANG_EN: return "en";
    case METRO_LANG_FR: return "fr";
    case METRO_LANG_DE: return "de";
    case METRO_LANG_RU: return "ru";
    case METRO_LANG_IT: return "it";
    default:            return NULL;
    }
}
