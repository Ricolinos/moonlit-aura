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
/* Row provider for Settings' "about" pivot (PLAN_MAESTRO.md S2.2) --
 * not a page/screen of its own, "about" lives inside metro_page.h's
 * regular settings_page like "general"/"display" always have. Reads
 * device.cfg (metro_device.c, F6) and sync_summary.cfg
 * (metro_manifest.c, F8) fresh on every row draw -- both are tiny
 * files read rarely (Settings isn't a hot path), no caching needed. */
#ifndef METRO_SCREEN_ABOUT_H
#define METRO_SCREEN_ABOUT_H

#include <stdbool.h>

#include "metro_page.h"

extern const struct metro_pivot metro_screen_about_pivot;

/* moonlit (D-062 §E.4, D-064): la fila de version es fija (indice 1,
 * justo bajo el nombre del aparato) y es el ancla de la fila oculta de
 * diagnostico -- SELECT sostenido sobre ella revela/oculta la marca de
 * agua de la pila del hilo principal. En el simulador la fila esta
 * siempre visible (ahi no hay marca de agua que leer, pero si hay
 * geometria que verificar). */
bool metro_screen_about_row_is_version(int index);
void metro_screen_about_toggle_diag(void);

#endif /* METRO_SCREEN_ABOUT_H */
