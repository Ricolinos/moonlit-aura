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
/* Reads/writes /.rockbox/aura/aura.cfg (settings_parseline()/read_line(),
 * regenerates the whole file on every save -- PLAN_MAESTRO.md S1.2, M-017).
 * Aura Studio's AuraDeviceProbe reads this file to decide whether the
 * device has ever booted Metro; the file existing with the right keys IS
 * the contract, regardless of what those values are (E.2: it must exist
 * from the very first boot). */
#ifndef METRO_SETTINGS_H
#define METRO_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>
#include "metro_theme.h"
#include "metro_lang.h"

/* F11: FX level matrix (PLAN_MAESTRO.md M-015/S3) -- animations is
 * "how it moves" (metro_transitions.c's frame count), graphics is
 * "what gets drawn" (currently just whether FADE does a real per-pixel
 * blend or falls back to a slide, metro_fb.c's own doc comment).
 * Canon = maximum fidelity (ALL/FULL); lower levels are subtractions
 * on the same code, never a different code path. */
enum metro_anim_level {
    METRO_ANIM_OFF = 0,
    METRO_ANIM_MINIMAL,
    METRO_ANIM_ALL,
    METRO_ANIM_COUNT
};
#define METRO_ANIM_DEFAULT METRO_ANIM_ALL

enum metro_gfx_level {
    METRO_GFX_LITE = 0,
    METRO_GFX_FULL,
    METRO_GFX_COUNT
};
#define METRO_GFX_DEFAULT METRO_GFX_FULL

/* moonlit (D-069, maestro SS D): los cuatro valores de "pedir codigo".
 * `HOLD` es el predeterminado -- es el gesto que el dueno ya hace. */
enum metro_lock_require {
    METRO_LOCK_REQUIRE_HOLD = 0, /* al poner Hold */
    METRO_LOCK_REQUIRE_1MIN,     /* si estuvo con Hold >= 1 minuto */
    METRO_LOCK_REQUIRE_5MIN,     /* ... >= 5 minutos */
    METRO_LOCK_REQUIRE_BOOT,     /* solo al encender (lo de siempre) */
    METRO_LOCK_REQUIRE_COUNT
};

typedef struct {
    enum metro_theme_kind theme;
    enum metro_accent accent;
    enum metro_language language;
    enum metro_anim_level animations;
    enum metro_gfx_level graphics;
    int tz_local_quarters; /* quarter-hours from UTC, D.4 of the contract */
    bool first_boot_done;
    /* R3-F7/DD-8 (M-068): candado de interfaz. La clave se guarda como
     * CADENA de 4 dígitos, no como entero -- con un entero, "0000" y
     * "clave ausente" son el mismo valor 0, que es exactamente la
     * trampa por la que un aura.cfg a medio escribir dejaría el aparato
     * bloqueado con una clave que nadie configuró (Aura-Firmware la
     * tiene). Texto plano a propósito: es un candado de interfaz, no de
     * datos -- ver metro_screen_lock.h. Cadena vacía = sin clave. */
    bool screen_lock;
    char screen_lock_pin[5];
    /* moonlit (D-069, maestro SS D): cuando se vuelve a pedir la clave.
     * Hasta ahora solo se pedia al arrancar, asi que el bloqueo no
     * servia para lo unico que la gente hace con el: guardarse el
     * aparato en el bolsillo. */
    enum metro_lock_require screen_lock_require;
    /* moonlit (D-079, contrato v19, maestro SS A.2.1): "rev" de
     * /.aura/settings.cfg ya aplicado -- desempata si hace falta
     * volver a aplicar el archivo compartido (rev > este valor) en el
     * proximo handoff de disco. 0 = nunca se aplico nada todavia. */
    long shared_rev_applied;
} metro_settings_t;

extern metro_settings_t metro_settings;

/* moonlit (D-054, contrato v15): base de datos tagcache COMPARTIDA
 * entre las tres familias (Aura, Metro, moonlit) -- apps/tagcache.c es
 * byte-identico en los tres repos (TAGCACHE_MAGIC 0x54434810) y la
 * ruta es runtime (global_settings.tagcache_db_path), asi que un solo
 * juego de database_*.tcd en la raiz del volumen sirve a todos y el
 * cambio de firmware deja de reconstruir 5 minutos. Unico dueno de
 * las rutas del contrato: este header + metro_settings.c/metro_sync.c
 * (regla de rutas del CLAUDE.md). Studio ignora el directorio salvo
 * para borrarlo al forzar una reconstruccion. */
#define AURA_SHARED_DB_DIR        "/.aura/tagcache"
#define AURA_SHARED_DB_STAMP_PATH AURA_SHARED_DB_DIR "/db_stamp.txt"

/* D-054: fija global_settings.tagcache_db_path = AURA_SHARED_DB_DIR y
 * migra por rename() (misma particion FAT, atomico, nunca copia) los
 * database_*.tcd y el db_stamp.txt v12 que un arranque anterior dejo
 * dentro del arbol (.rockbox/). Debe correr en apps/main.c init()
 * DESPUES de settings_load() (que sobreescribiria la ruta con la de
 * config.cfg) y ANTES de init_tagcache() (que abre la base en la ruta
 * que encuentre) -- ver MODIFICATIONS.md. */
void metro_force_shared_db_path(void);

/* Loads from disk into metro_settings, falling back to defaults for
 * any key that's missing or the file doesn't exist at all -- in which
 * case this also creates it (E.2, first-boot guarantee). Does NOT
 * apply theme/accent/language anywhere; the caller (metro_main.c)
 * does that right after, in one place. */
void metro_settings_load(void);

/* Regenerates aura.cfg entirely from metro_settings -- never edits
 * the file in place. Call after changing any field. */
void metro_settings_save(void);

/* D.4 of the contract: reads the six transient rtc_sync_* keys (and
 * the persistent tz_local_quarters) straight from disk -- NOT from
 * metro_settings, which may be stale relative to a marker Aura Studio
 * just wrote. If all six are present, sets the real RTC and calls
 * metro_settings_save() (which drops the transient keys on its own,
 * they were never part of metro_settings_t). No-op otherwise. Call at
 * the same two moments as the sync marker: boot and after returning
 * from the USB screen. */
void metro_settings_apply_pending_clock(void);

/* moonlit (D-079, contrato v19, maestro SS A.2.2): lee
 * /.aura/settings.cfg (moonlit_shared_settings.h); si existe, tiene la
 * cabecera valida y su `rev` es mayor que
 * metro_settings.shared_rev_applied, aplica las 13 claves conocidas
 * (a global_settings con el flush de save_global_settings_now() y a
 * metro_settings/metro_theme_set()/metro_lang_set() segun corresponda)
 * y actualiza shared_rev_applied. Una clave con un valor fuera del
 * rango real del target se ignora sola, nunca aborta las demas. Sin
 * archivo, sin cabecera, o rev <= shared_rev_applied: no-op. Llamar en
 * los mismos dos momentos que metro_settings_apply_pending_clock():
 * arranque y al volver de USB (metro_main.c's metro_disk_handoff()). */
void metro_settings_apply_pending_shared(void);

/* moonlit (D-079, contrato v19, maestro SS A.2.3/A.2.4): reescribe
 * /.aura/settings.cfg ENTERO (escritura atomica .tmp+rename) con
 * rev+1, updated_by="moonlit", las 13 claves conocidas tomadas de su
 * valor VIGENTE ahora mismo (global_settings/metro_settings) y
 * cualquier clave desconocida que el archivo ya tuviera, preservada
 * verbatim. Actualiza metro_settings.shared_rev_applied al rev nuevo
 * (esta escritura ES la fuente de verdad, no hace falta reaplicarla).
 * Llamar despues de cualquier cambio de UI a una de las 13 claves
 * compartidas (metro_screen_settings.c) y desde "restablecer
 * ajustes". Sin archivo previo: lo crea con rev 1. */
void metro_settings_write_shared(void);

/* R2-F1/DD-4 (M-054): creates /Music, /Videos, /Photos, /Playlists
 * (mkdir(), no-op if a path already exists) -- library-layout-v1.md's
 * four top-level media folders, so a device that never went through
 * an Aura Studio sync (music copied over USB by hand, or a completely
 * fresh install) still has somewhere for Metro's own screens/plugins
 * to read and write from on first boot. Call at the same two moments
 * as metro_settings_apply_pending_clock(): boot and after returning
 * from the USB screen (metro_main.c's metro_disk_handoff()) -- a USB
 * session is exactly when a folder could have been deleted or a fresh
 * disk mounted. */
void metro_ensure_media_dirs(void);

/* R2-F2/DD-9 (M-057), generalized R3-F1/DD-1: .../aura/moonlitcache/<subdir>/
 * (moonlit, D-001: renamed from the Metro cache name so both families coexist)
 * -- Metro's own on-disk thumbnail cache, one subdirectory per source
 * (NOT Aura's photocache/: format and thumbnail size differ, and
 * family-switch cleanup wipes that one anyway). The only function
 * allowed to build this path (CLAUDE.md's compat-path rule) --
 * metro_thumbs.c calls this instead of composing ROCKBOX_DIR itself.
 * Writes into `out` (does not create the directory -- caller's job,
 * mkdir() if missing, before writing inside it). */
void metro_settings_metro_cache_dir(const char *subdir, char *out, size_t outsz);

/* moonlit (D-055, contrato v15): /.aura/thumbs/<subdir>/ -- the 80x80
 * .mth tiles (raw fb_data, METRO_TILE_SIZE, byte-identical format in
 * Metro and moonlit) are SHARED between both families so a switch
 * never re-decodes them. moonlitcache/art (120 px .pfraw, moonlit's
 * own format) stays in the tree. Studio ignores the directory except
 * to delete it when forcing a rebuild. Migration (rename of
 * moonlitcache/{albums,artists,photos} on first boot):
 * metro_settings_migrate_shared_thumbs(), called from metro_main(). */
void metro_settings_shared_thumbs_dir(const char *subdir, char *out, size_t outsz);
bool metro_settings_migrate_shared_thumbs(void); /* true if anything was renamed */

/* moonlit (D-059, contrato v16): /.aura/art/<subdir>/ -- the shared
 * MASTER art cache ("<key>.art", 130 px albums/artists, 80 px photos,
 * plus the shared "<key>.none" negative marker), decoded once by
 * whichever family is running and only DERIVED by the others
 * (moonlit_master_art.h). Subdirs: "albums", "artists", "photos".
 * Studio ignores the directory except to delete it when forcing a
 * rebuild. Writes into `out` only; moonlit_master_art_ensure_dir()
 * creates it. */
void metro_settings_shared_art_dir(const char *subdir, char *out, size_t outsz);

/* moonlit (D-063, contrato v18): al arrancar, compara
 * /.aura/art/format.txt con MOONLIT_MASTER_ART_FORMAT_VERSION. Si falta
 * o es menor, borra las caches DERIVADAS de las tres fuentes
 * (/.aura/art, /.aura/thumbs y moonlitcache/, por sufijo conocido) y
 * escribe la version nueva. Devuelve true si borro algo -- el
 * constructor de fondo (D-059) las vuelve a generar. Cierra el caso que
 * ninguna clave puede cerrar: un tile mal derivado por codigo anterior
 * sobrevive aunque el codigo ya este corregido, porque su clave no
 * cambio. Se llama UNA vez, antes de arrancar el constructor. */
bool metro_settings_purge_stale_art_caches(void);

/* R3-F3/DD-6 (M-064): .../aura/artist_images.cfg (Studio's index) and
 * .../aura/artists/ (Studio's own source photo cache, the directory
 * artist_images.cfg's filenames resolve into) -- CONTRATO-firmware-studio.md
 * §D.3. Distinct from metro_settings_metro_cache_dir("artists", ...)
 * above, Metro's OWN derived 80x80 tile cache. */
void metro_settings_artist_images_cfg_path(char *out, size_t outsz);
void metro_settings_artists_dir(char *out, size_t outsz);

/* R3-F5/DD-7 (M-066): .../aura/ratings.cfg -- Studio's one-way ratings
 * export (`<path absoluta>: <rating 0-10>` por línea), reimportado por
 * metro_sync.c cada vez que el import de música termina bien. */
void metro_settings_ratings_cfg_path(char *out, size_t outsz);

/* R5 (M-090, contrato v10 -- varios firmwares instalados a la vez;
 * moonlit D-047: tres familias, tabla en metro_firmware_families.h).
 *
 * El arbol activo es siempre /.rockbox (lo unico que el bootloader
 * arranca); cada familia hermana duerme, completa y con sus ajustes,
 * como /.firmware-<familia> (Aura: /.firmware-aura, Metro:
 * /.firmware-metro). Cambiar de firmware son dos renombres mas
 * reiniciar. Las rutas viven aqui (y en la tabla de familias) porque
 * son del contrato (regla del CLAUDE.md). `i` es siempre un indice de
 * metro_fw_sibling(); fuera de rango ambas devuelven false.
 *
 * metro_firmware_sibling_installed(i): hay un arbol dormido de esa
 * familia que despertar -- su fila del submenu es inerte si no. */
bool metro_firmware_sibling_installed(int i);

/* Ejecuta el cambio al hermano i (contrato v10, en este orden y sin
 * nada en medio):
 *   1. guarda todo lo de moonlit (aura.cfg, config.cfg, cola de
 *      tagcache) y fuerza el vaciado a disco -- despues del renombre
 *      /.rockbox es el arbol del HERMANO y cualquier escritura tardia
 *      caeria alli;
 *   2. /.rockbox -> /.firmware-moonlit (saliente primero: el peor corte
 *      deja un dormido entero; Studio repara al conectar);
 *   3. /.firmware-<hermano> -> /.rockbox;
 *   4. copia /.rockbox/rockbox.ipod (ya el del hermano) sobre
 *      /rockbox.ipod, el respaldo del bootloader, que debe ser siempre
 *      el del activo;
 *   5. deja /.aura/sync-pending.json con music=true SOLO si la
 *      biblioteca cambio desde el sello del arbol saliente (M-091);
 *   6. reinicia EN SECO (system_reboot) -- nunca por el apagado normal,
 *      que volveria a guardar los ajustes de moonlit... en el otro arbol.
 * Solo vuelve si fallo antes de tocar nada, o si el paso 3 fallo y el
 * 2 se pudo deshacer (devuelve false; el firmware sigue siendo moonlit).
 * Si ya existe /.firmware-moonlit (no deberia: Studio garantiza "nunca
 * dos de la misma familia") aborta sin tocar nada en vez de borrarlo. */
bool metro_firmware_switch_to(int i);

#endif /* METRO_SETTINGS_H */
