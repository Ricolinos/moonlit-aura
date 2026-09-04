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
/* Directory scanning shared by metro_video.c/metro_photos.c -- both
 * browse a flat directory (D-192 of Aura-Firmware's contract: /Videos
 * and /Photos never get subfolders) filtered by extension, natural
 * order. Narrower than Aura-Firmware's aura_fsutil.h on purpose: that
 * one is about deleting caches Metro doesn't have (no disk-cached
 * thumbnails, no theme installs in v1) -- this one is just the listing
 * primitive neither metro_video.c nor metro_photos.c should duplicate. */
#ifndef METRO_FSUTIL_H
#define METRO_FSUTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* R4/FA-2 (M-075): true para lo que NINGUNA lista de Metro debe
 * mostrar jamás.
 *
 * El caso que motivó esto: macOS, al escribir en el FAT del iPod, deja
 * sidecars AppleDouble llamados `._<nombre original>` -- y **conservan
 * la extensión**. `._IMG_1234.jpg` pasaba el filtro de extensión (que
 * compara solo el sufijo) y no es un directorio, así que se listaba
 * como una foto más. Lo mismo con `._Mi Lista.m3u8` entre las
 * playlists. Se observó en vivo en el iPod del dueño: `._rockbox.ipod`,
 * `._version.txt`, `._sync-pending.json`, más `.Spotlight-V100`,
 * `.Trashes` y `.fseventsd`.
 *
 * La regla es **el punto inicial**, no el `._` específico: en FAT un
 * nombre que empieza con punto es oculto por convención, ninguna de las
 * fuentes de contenido legítimo lo genera (Aura Studio sanea nombres;
 * una copia manual tampoco), y así cubre de paso `.DS_Store`, los
 * directorios de servicio de macOS, y `.`/`..`. Un punto en cualquier
 * OTRA posición (`mi.foto.jpg`) no se ve afectado.
 *
 * Defensivo a propósito, y solo del lado del firmware (decisión del
 * dueño): el usuario puede copiar archivos a mano sin que Studio
 * intervenga, así que filtrar al leer es lo único que cubre todos los
 * caminos. `static inline` en el header para que quede cubierto por el
 * arnés de host sin arrastrar dependencias de Rockbox. */
static inline bool metro_fsutil_is_hidden_name(const char *name)
{
    return name == NULL || name[0] == '.';
}

/* PLAN_MAESTRO.md S1.2: VIDEO_NAME_LEN/PHOTO_NAME_LEN, both 96 bytes
 * (contract: filenames <= 95 bytes UTF-8 including extension, + NUL). */
#define METRO_FSUTIL_NAME_LEN 96

/* Lists `dir`'s entries whose name ends (case-insensitively) in one of
 * `exts` (an array of `n_exts` strings, each including the leading
 * dot, e.g. ".jpg") into `out` (`max` buffers of METRO_FSUTIL_NAME_LEN
 * bytes each), naturally sorted (strnatcasecmp -- "2.jpg" before
 * "10.jpg", case-insensitive) over the WHOLE matching set before
 * truncating to `max` -- the result is always the true first `max` in
 * natural order, never whatever readdir() happened to return first.
 * Does not recurse. Returns the count placed into `out` (0 if `dir`
 * doesn't exist). Matching entries beyond METRO_FSUTIL_SCAN_CEILING
 * are not considered at all (see its own comment) -- irrelevant for
 * the contract's own caps (100 videos, 500 photos). */
int metro_fsutil_list_by_ext(const char *dir, const char *const *exts, int n_exts,
                              char out[][METRO_FSUTIL_NAME_LEN], int max);

/* R2-F2/DD-9: same scan as metro_fsutil_list_by_ext(), plus each
 * entry's mtime in out_mtimes (parallel array, same indices as out) --
 * metro_thumbs.c's cache invalidation key. out_mtimes may be
 * NULL (metro_fsutil_list_by_ext() itself is exactly this call with
 * NULL). */
int metro_fsutil_list_by_ext_mtime(const char *dir, const char *const *exts, int n_exts,
                                    char out[][METRO_FSUTIL_NAME_LEN], long out_mtimes[],
                                    int max);

/* moonlit (D-063, contrato v18): copia en `out` el directorio que
 * contiene a `file_path` -- SIN la barra final, salvo la raiz, que se
 * queda en "/". Pura (solo cadenas), `static inline` por el mismo
 * motivo que metro_fsutil_is_hidden_name(): asi la cubre el arnes de
 * host sin arrastrar dependencias de Rockbox.
 *
 * Existe para la clave de album v18: el `<mtime>` de `a-<crc32>.<mtime>`
 * pasa a ser el mayor entre el de la pista representativa y el de la
 * `cover.jpg` HERMANA, para que una caratula reescrita sin tocar la
 * pista invalide la maestra (hipotesis (a) de D-055/D-056). Este helper
 * es el "de que carpeta hermana hablamos".
 *
 * false (y `out` sin tocar) si no hay carpeta que extraer: ruta vacia,
 * ruta relativa sin ninguna barra, o `out` demasiado corto. */
static inline bool metro_fsutil_parent_dir(const char *file_path, char *out,
                                            size_t outsz)
{
    const char *slash;
    size_t len;

    if (!file_path || !out || outsz == 0)
        return false;

    slash = strrchr(file_path, '/');
    if (!slash)
        return false;

    len = (size_t)(slash - file_path);
    if (len == 0)
        len = 1; /* "/pista.mp3" -> "/" */
    if (len + 1 > outsz)
        return false;

    memcpy(out, file_path, len);
    out[len] = '\0';
    return true;
}

/* moonlit (D-063): mtime del archivo `name` (comparado sin distinguir
 * mayusculas, como el FAT) dentro de `dir`, o -1 si no esta. Rockbox no
 * expone un stat() de un solo archivo -- dir_get_info() sobre una
 * entrada de readdir() es la unica via, la misma que ya usa
 * metro_fsutil_list_by_ext_mtime(). Recorre hasta encontrarlo: en una
 * carpeta de album tipica son una decena de entradas. */
long metro_fsutil_mtime_in_dir(const char *dir, const char *name);

#endif /* METRO_FSUTIL_H */
