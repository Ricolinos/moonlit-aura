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
#include <stdio.h>

#include "metro_splash_lang.h"
#include "metro_lang.h"

typedef struct {
    const char *match; /* original Rockbox English text (or prefix) */
    int exact;          /* 1 = the whole text must match, 0 = prefix only */
    const char *es;
    const char *en;
} splash_rule_t;

/* Source strings copied verbatim from apps/lang/english.lang (the only
 * language Rockbox itself ever actually loads -- Metro doesn't use its
 * language system, M-009/D-013). Prefix rules preserve whatever comes
 * after (a dynamic suffix like "[3/9]"). Order matters: longer
 * prefixes go before their shorter variants. */
static const splash_rule_t s_rules[] = {
    { "Loading... (",                              0,
      "Cargando... (",                              "Loading... (" },
    { "Loading...",                                1,
      "Cargando...",                                "Loading..." },
    { "Scanning disk...",                          1,
      "Preparando el disco...",                     "Preparing storage..." },
    { "Shutting down...",                          1,
      "Apagando...",                                "Shutting down..." },
    { "Database is not ready",                     1,
      "Terminando de preparar la biblioteca...",    "Finishing up your library..." },
    { "WARNING! Low Battery! Shutting down...",    1,
      "Bateria baja. Apagando...",                  "Low battery. Shutting down..." },
    { "Battery empty! RECHARGE! Shutting down...", 1,
      "Bateria agotada. Conecta el cargador.",      "Battery empty. Plug in your charger." },
    { "Committing database [",                     0,
      "Preparando la biblioteca [",                 "Preparing your library [" },
};

void metro_splash_translate(char *buf, size_t bufsz)
{
    size_t i;

    for (i = 0; i < sizeof(s_rules) / sizeof(s_rules[0]); i++)
    {
        const splash_rule_t *r = &s_rules[i];
        size_t mlen = strlen(r->match);
        int matches = r->exact ? !strcmp(buf, r->match)
                                : !strncmp(buf, r->match, mlen);
        const char *translated;

        if (!matches)
            continue;

        translated = (metro_lang_get() == METRO_LANG_EN) ? r->en : r->es;

        if (r->exact)
        {
            snprintf(buf, bufsz, "%s", translated);
        }
        else
        {
            char rest[64];
            snprintf(rest, sizeof(rest), "%s", buf + mlen);
            snprintf(buf, bufsz, "%s%s", translated, rest);
        }
        return;
    }
}
