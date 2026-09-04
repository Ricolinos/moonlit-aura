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
/* moonlit (D-079, maestro SS A): `/.aura/settings.cfg`, contrato v19 --
 * texto plano "clave: valor", una por linea, compartido por las tres
 * familias (Aura, Metro, moonlit). Aura-Firmware escribe el texto
 * canonico (maestro SS A.4); este modulo es la version propia de
 * moonlit -- mismo formato, mismo vector de prueba SS A.3, pero sin
 * copiar codigo de un hermano (CLAUDE.md).
 *
 * Puro C99, sin una sola dependencia de Rockbox (mismo criterio que
 * metro_sync_marker.c/metro_nav.c) -- compila y se prueba igual en el
 * host (apps/metro/test/test_shared_settings.c) y en el firmware. El
 * I/O de archivo (leer/escribir `/.aura/settings.cfg`, escritura
 * atomica .tmp+rename) vive en metro_settings.c, que tambien es quien
 * mapea estos campos a `global_settings`/`metro_settings_t` y aplica
 * los limites reales del target (p.ej. MAX_BRIGHTNESS_SETTING, que
 * este modulo no puede conocer). */
#ifndef MOONLIT_SHARED_SETTINGS_H
#define MOONLIT_SHARED_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

#define MOONLIT_SHARED_SETTINGS_HEADER "# aura-shared-settings v1"

#define MOONLIT_SHARED_SETTINGS_UPDATED_BY_LEN 8  /* "aura"/"metro"/"moonlit" */
#define MOONLIT_SHARED_SETTINGS_PIN_LEN        5  /* 4 digitos ASCII + NUL */
#define MOONLIT_SHARED_SETTINGS_WORD_LEN       8  /* "hold"/"1min"/"5min"/"boot", "off"/"track"/"album", "dark"/"light" */
#define MOONLIT_SHARED_SETTINGS_LANG_LEN       4  /* "es".."it" + NUL */
/* Bytes de lineas VERBATIM de claves no reconocidas que se preservan
 * para el proximo serialize() -- generoso a proposito (varias claves
 * futuras de golpe), recorta sin desbordar si no alcanza. */
#define MOONLIT_SHARED_SETTINGS_UNKNOWN_CAP  512

typedef struct {
    bool have_rev;                 long rev;
    bool have_updated_by;          char updated_by[MOONLIT_SHARED_SETTINGS_UPDATED_BY_LEN];
    bool have_screen_lock_enabled; bool screen_lock_enabled;
    bool have_screen_lock_pin;     char screen_lock_pin[MOONLIT_SHARED_SETTINGS_PIN_LEN];
    bool have_screen_lock_require; char screen_lock_require[MOONLIT_SHARED_SETTINGS_WORD_LEN];
    bool have_brightness;          long brightness;
    bool have_backlight_timeout;   long backlight_timeout; /* segundos; -1 = nunca */
    bool have_idle_poweroff;       long idle_poweroff;     /* minutos; 0 = nunca */
    bool have_keyclick;            bool keyclick;
    bool have_volume_limit;        long volume_limit;      /* dB, puede ser negativo */
    bool have_replaygain;          char replaygain[MOONLIT_SHARED_SETTINGS_WORD_LEN];
    bool have_language;            char language[MOONLIT_SHARED_SETTINGS_LANG_LEN];
    bool have_appearance;          char appearance[MOONLIT_SHARED_SETTINGS_WORD_LEN];

    /* Lineas completas ("clave: valor\n") de claves NO reconocidas,
     * concatenadas tal cual aparecieron -- se reescriben verbatim en
     * el proximo serialize(). Vacio si no hubo ninguna. */
    char unknown_lines[MOONLIT_SHARED_SETTINGS_UNKNOWN_CAP];
} moonlit_shared_settings_t;

/* Deja `out` en "nada leido": todo have_*=false, unknown_lines vacio. */
void moonlit_shared_settings_init(moonlit_shared_settings_t *out);

/* Parsea el archivo entero. false SOLO si la primera linea no es
 * exactamente MOONLIT_SHARED_SETTINGS_HEADER -- el archivo entero se
 * rechaza (maestro SS A.2.5: "si el archivo falta o no tiene la
 * cabecera, se comporta como si no existiera") y `out` queda como
 * init() lo dejo. Con cabecera valida SIEMPRE devuelve true: una
 * clave conocida cuyo valor no tiene la FORMA esperada (un numero
 * donde no lo hay, una palabra que no es ninguna de las aceptadas)
 * se trata como si la clave no hubiera aparecido (su have_* en
 * false) -- nunca aborta el resto del archivo (SS A.2.2, "un valor
 * fuera de rango se ignora clave por clave"). Los LIMITES numericos
 * reales (brightness contra MAX_BRIGHTNESS_SETTING, etc.) son cosa
 * del llamador -- este modulo no los conoce; ver
 * moonlit_shared_settings_int_in_range(). */
bool moonlit_shared_settings_parse(const char *text, moonlit_shared_settings_t *out);

/* true si v esta en [lo, hi] -- helper puro y trivial para que el
 * llamador (o su test host) exprese "brightness: 999 fuera de rango"
 * sin repetir la comparacion en cada sitio. */
bool moonlit_shared_settings_int_in_range(long v, long lo, long hi);

/* Escribe la cabecera + cada una de las 13 claves conocidas cuyo
 * have_* este en true + las lineas desconocidas preservadas, en ese
 * orden (mismo orden que la tabla SS A.1). El llamador que va a
 * REESCRIBIR el archivo completo (maestro SS A.2.3: "se reescribe el
 * archivo completo... con todas las claves conocidas") arma un `in`
 * con las 13 en true y sus valores VIGENTES antes de llamar esto.
 * Devuelve los bytes escritos (sin el NUL) o -1 si no cupo en
 * `bufsize`. */
int moonlit_shared_settings_serialize(const moonlit_shared_settings_t *in,
                                       char *buf, size_t bufsize);

/* Mapas puros palabra<->entero. El entero coincide, a proposito, con
 * el orden de los enums de moonlit (metro_lock_require en
 * metro_settings.h, metro_theme_kind en metro_theme.h) para que el
 * llamador pueda castear el resultado directo sin una tabla propia.
 * -1 si la palabra no se reconoce. */
int moonlit_shared_settings_lock_require_from_str(const char *s);
const char *moonlit_shared_settings_lock_require_to_str(int v);

/* 0/1/2 = off/track/album (mismo orden que el replaygain_type de
 * Rockbox, REPLAYGAIN_OFF/TRACK/ALBUM). */
int moonlit_shared_settings_replaygain_from_str(const char *s);
const char *moonlit_shared_settings_replaygain_to_str(int v);

/* 0/1 = dark/light, mismo orden que METRO_THEME_DARK/LIGHT. */
int moonlit_shared_settings_appearance_from_str(const char *s);
const char *moonlit_shared_settings_appearance_to_str(int v);

#endif /* MOONLIT_SHARED_SETTINGS_H */
