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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "file.h"
#include "dir.h"
#include "misc.h"
#include "rbpaths.h"
#include "rtc.h"
#include "string-extra.h"
#include "timefuncs.h"
#include "system.h"          /* system_reboot(), TIME_BEFORE() -- M-090/D-060 */
#include "settings.h"        /* settings_save() -- M-090 */
#include "ata_idle_notify.h" /* call_storage_idle_notifys() -- M-090 */
#include "tagcache.h"        /* tagcache_shutdown(), tagcache_get_commit_step() -- M-090/D-060 */
#include "kernel.h"          /* current_tick, sleep() -- D-060 */
#include "backlight.h"       /* backlight_set_brightness()/_timeout(), MAX_BRIGHTNESS_SETTING -- moonlit (D-079) */
#include "dsp_misc.h"        /* dsp_replaygain_set_settings(), REPLAYGAIN_* -- moonlit (D-079) */
#include "powermgmt.h"       /* set_poweroff_timeout() -- moonlit (D-079) */
#include "sound.h"           /* sound_min()/sound_max(SOUND_VOLUME) -- moonlit (D-079) */

#include "metro_settings.h"
#include "metro_sync.h" /* marcador de sync al cambiar de firmware -- M-090 */
#include "metro_firmware_families.h" /* moonlit (D-047): tabla de hermanas */
#include "moonlit_art.h"         /* moonlit_art_sweep() -- moonlit (D-063) */
#include "moonlit_master_art.h"  /* format.txt -- moonlit (D-063) */
#include "moonlit_shared_settings.h" /* moonlit (D-079, contrato v19) */
#include "metro_theme.h"         /* metro_theme_set() -- moonlit (D-079) */
#include "metro_lang.h"          /* metro_lang_set()/metro_lang_code_* -- moonlit (D-079) */

#define METRO_DIR      ROCKBOX_DIR "/aura"
#define METRO_CFG_PATH METRO_DIR "/aura.cfg"

metro_settings_t metro_settings;

static const metro_settings_t defaults = {
    .theme = METRO_THEME_DEFAULT,
    .accent = METRO_ACCENT_DEFAULT,
    .language = METRO_LANG_ES,
    .animations = METRO_ANIM_DEFAULT,
    .graphics = METRO_GFX_DEFAULT,
    .tz_local_quarters = 0,
    .first_boot_done = false,
    .screen_lock = false,
    .screen_lock_pin = "",
    .screen_lock_require = METRO_LOCK_REQUIRE_HOLD, /* moonlit (D-069) */
};

static int clamp_enum(int v, int count)
{
    return (v >= 0 && v < count) ? v : 0;
}

void metro_settings_load(void)
{
    int fd;
    char line[64];
    bool existed;

    metro_settings = defaults;

    fd = open(METRO_CFG_PATH, O_RDONLY);
    existed = fd >= 0;
    if (existed)
    {
        while (read_line(fd, line, sizeof(line)) > 0)
        {
            char *name, *value;
            int v;

            if (!settings_parseline(line, &name, &value))
                continue;
            v = atoi(value);

            if (!strcmp(name, "theme"))
                metro_settings.theme = (enum metro_theme_kind)clamp_enum(v, 2);
            else if (!strcmp(name, "accent"))
                metro_settings.accent = (enum metro_accent)clamp_enum(v, METRO_ACCENT_COUNT);
            else if (!strcmp(name, "language"))
                metro_settings.language = (enum metro_language)clamp_enum(v, METRO_LANG_COUNT);
            else if (!strcmp(name, "animations"))
                metro_settings.animations = (enum metro_anim_level)clamp_enum(v, METRO_ANIM_COUNT);
            else if (!strcmp(name, "graphics"))
                metro_settings.graphics = (enum metro_gfx_level)clamp_enum(v, METRO_GFX_COUNT);
            else if (!strcmp(name, "tz_local_quarters"))
                metro_settings.tz_local_quarters = v;
            else if (!strcmp(name, "first_boot_done"))
                metro_settings.first_boot_done = (v != 0);
            else if (!strcmp(name, "screen_lock"))
                metro_settings.screen_lock = (v != 0);
            else if (!strcmp(name, "screen_lock_require"))
            {
                /* moonlit (D-069): un valor fuera de rango cae al
                 * predeterminado, nunca a un estado invalido -- mismo
                 * criterio de "fallar abierto" que el resto del
                 * candado (metro_screen_lock.h). */
                metro_settings.screen_lock_require =
                    (v >= 0 && v < METRO_LOCK_REQUIRE_COUNT)
                        ? (enum metro_lock_require)v
                        : METRO_LOCK_REQUIRE_HOLD;
            }
            else if (!strcmp(name, "screen_lock_pin"))
            {
                /* Cadena, no atoi() -- los ceros a la izquierda son
                 * parte de la clave. metro_screen_lock.c valida que
                 * sean 4 dígitos y falla ABIERTO si no lo son. */
                strlcpy(metro_settings.screen_lock_pin, value,
                         sizeof(metro_settings.screen_lock_pin));
            }
            else if (!strcmp(name, "shared_rev_applied"))
                /* atol() no esta disponible en el build de target
                 * (Rockbox no trae libc completa) -- atoi() alcanza,
                 * el resto del archivo tampoco usa nada mas grande. */
                metro_settings.shared_rev_applied = (long)atoi(value);
            /* firmware_family/sync_marker_supported: write-only, Aura
             * Studio reads these off the mounted disk -- never read back
             * here. rtc_sync_*: transient, only
             * metro_settings_apply_pending_clock() reads them, straight
             * from disk, not through this struct. */
        }
        close(fd);
    }

    metro_settings.first_boot_done = true;
    if (!existed)
        metro_settings_save();
}

void metro_settings_save(void)
{
    int fd;

    if (!dir_exists(METRO_DIR))
        mkdir(METRO_DIR);

    fd = creat(METRO_CFG_PATH, 0666);
    if (fd < 0)
        return;

    /* moonlit (D-001): family string, read by Aura Studio off the disk. */
    fdprintf(fd, "firmware_family: moonlit\n");
    fdprintf(fd, "sync_marker_supported: 1\n");
    fdprintf(fd, "theme: %d\n", (int)metro_settings.theme);
    fdprintf(fd, "accent: %d\n", (int)metro_settings.accent);
    fdprintf(fd, "language: %d\n", (int)metro_settings.language);
    fdprintf(fd, "animations: %d\n", metro_settings.animations);
    fdprintf(fd, "graphics: %d\n", metro_settings.graphics);
    fdprintf(fd, "tz_local_quarters: %d\n", metro_settings.tz_local_quarters);
    fdprintf(fd, "first_boot_done: %d\n", metro_settings.first_boot_done ? 1 : 0);
    /* moonlit (D-079): 0 (nunca se aplico nada compartido) no hace
     * falta escribirlo -- es el valor por defecto de un aura.cfg
     * fresco y metro_settings_apply_pending_shared() lo trata igual
     * que si la linea faltara. */
    if (metro_settings.shared_rev_applied > 0)
        fdprintf(fd, "shared_rev_applied: %ld\n", metro_settings.shared_rev_applied);

    /* R3-F7/DD-8 (M-068): las dos claves del candado se escriben SOLO
     * cuando hay candado. Así, un aparato sin candado no las tiene en
     * absoluto (en vez de un `screen_lock: 0` permanente), y sobre todo:
     * la salida de emergencia documentada -- "conecta por USB y borra
     * estas dos líneas" -- deja un archivo que no las vuelve a hacer
     * crecer solo en el siguiente guardado. */
    if (metro_settings.screen_lock)
    {
        fdprintf(fd, "screen_lock: 1\n");
        fdprintf(fd, "screen_lock_pin: %s\n", metro_settings.screen_lock_pin);
        fdprintf(fd, "screen_lock_require: %d\n",
                 (int)metro_settings.screen_lock_require);
    }

    close(fd);
}

void metro_settings_apply_pending_clock(void)
{
    int fd;
    char line[64];
    struct tm tm;
    bool have_year = false, have_month = false, have_day = false;
    bool have_hour = false, have_min = false, have_sec = false;

    memset(&tm, 0, sizeof(tm));

    fd = open(METRO_CFG_PATH, O_RDONLY);
    if (fd < 0)
        return;

    while (read_line(fd, line, sizeof(line)) > 0)
    {
        char *name, *value;
        int v;

        if (!settings_parseline(line, &name, &value))
            continue;
        v = atoi(value);

        if (!strcmp(name, "rtc_sync_year"))       { tm.tm_year = v - 1900; have_year  = true; }
        else if (!strcmp(name, "rtc_sync_month")) { tm.tm_mon  = v - 1;    have_month = true; }
        else if (!strcmp(name, "rtc_sync_day"))   { tm.tm_mday = v;        have_day   = true; }
        else if (!strcmp(name, "rtc_sync_hour"))  { tm.tm_hour = v;        have_hour  = true; }
        else if (!strcmp(name, "rtc_sync_min"))   { tm.tm_min  = v;        have_min   = true; }
        else if (!strcmp(name, "rtc_sync_sec"))   { tm.tm_sec  = v;        have_sec   = true; }
        else if (!strcmp(name, "tz_local_quarters"))
            metro_settings.tz_local_quarters = v;
    }
    close(fd);

    if (have_year && have_month && have_day && have_hour && have_min && have_sec)
    {
#if CONFIG_RTC
        rtc_write_datetime(&tm);
#endif
        /* Rewrites aura.cfg entirely from metro_settings -- the
         * rtc_sync_* keys just read aren't part of that struct, so
         * they disappear on their own, nothing to delete by hand. */
        metro_settings_save();
    }
}

/* moonlit (D-079, contrato v19): /.aura/settings.cfg -- compartido por
 * las tres familias, Studio nunca lo toca ni lo borra (igual que
 * /.aura/art, ver metro_settings_shared_art_dir()). */
#define AURA_SHARED_SETTINGS_PATH "/.aura/settings.cfg"
#define AURA_SHARED_SETTINGS_BUF  1024

static int read_shared_settings_text(char *buf, size_t bufsize)
{
    int fd = open(AURA_SHARED_SETTINGS_PATH, O_RDONLY);
    ssize_t n;

    if (fd < 0)
        return -1;
    n = read(fd, buf, bufsize - 1);
    close(fd);
    if (n < 0)
        return -1;
    buf[n] = '\0';
    return (int)n;
}

/* replaygain: el 0/1/2 de moonlit_shared_settings.h (off/track/album,
 * el orden de la tabla SS A.1) no coincide con el enum de Rockbox
 * (REPLAYGAIN_TRACK=0/ALBUM=1/SHUFFLE=2/OFF=3, dsp_misc.h) -- SHUFFLE
 * nunca se expone (mismo criterio que metro_screen_settings.c's
 * replaygain_label()/cycle_replaygain()). */
static int replaygain_shared_to_rockbox(int shared)
{
    switch (shared)
    {
    case 0: return REPLAYGAIN_OFF;
    case 1: return REPLAYGAIN_TRACK;
    case 2: return REPLAYGAIN_ALBUM;
    default: return -1;
    }
}

static int replaygain_rockbox_to_shared(int type)
{
    switch (type)
    {
    case REPLAYGAIN_OFF:   return 0;
    case REPLAYGAIN_TRACK: return 1;
    case REPLAYGAIN_ALBUM: return 2;
    default:                return 1; /* SHUFFLE -> "track", ver arriba */
    }
}

void metro_settings_apply_pending_shared(void)
{
    char buf[AURA_SHARED_SETTINGS_BUF];
    moonlit_shared_settings_t s;
    bool touched_global = false;
    int v;

    if (read_shared_settings_text(buf, sizeof(buf)) < 0)
        return;
    if (!moonlit_shared_settings_parse(buf, &s))
        return; /* sin cabecera valida: como si no existiera (SS A.2.5) */
    if (!s.have_rev || s.rev <= metro_settings.shared_rev_applied)
        return;

    if (s.have_screen_lock_enabled)
        metro_settings.screen_lock = s.screen_lock_enabled;
    if (s.have_screen_lock_pin)
        strlcpy(metro_settings.screen_lock_pin, s.screen_lock_pin,
                sizeof(metro_settings.screen_lock_pin));
    if (s.have_screen_lock_require &&
        (v = moonlit_shared_settings_lock_require_from_str(s.screen_lock_require)) >= 0)
        metro_settings.screen_lock_require = (enum metro_lock_require)v;

    if (s.have_brightness &&
        moonlit_shared_settings_int_in_range(s.brightness, 1, MAX_BRIGHTNESS_SETTING))
    {
        global_settings.brightness = (int)s.brightness;
        backlight_set_brightness(global_settings.brightness);
        touched_global = true;
    }
    if (s.have_backlight_timeout && s.backlight_timeout >= -1)
    {
        global_settings.backlight_timeout = (int)s.backlight_timeout;
        backlight_set_timeout(global_settings.backlight_timeout);
        touched_global = true;
    }
    if (s.have_idle_poweroff && s.idle_poweroff >= 0)
    {
        global_settings.poweroff = (int)s.idle_poweroff;
        set_poweroff_timeout(global_settings.poweroff);
        touched_global = true;
    }
    if (s.have_keyclick)
    {
        global_settings.keyclick = s.keyclick ? 2 : 0;
        touched_global = true;
    }
    if (s.have_volume_limit &&
        moonlit_shared_settings_int_in_range(s.volume_limit, sound_min(SOUND_VOLUME),
                                              sound_max(SOUND_VOLUME)))
    {
        global_settings.volume_limit = (int)s.volume_limit;
        touched_global = true;
    }
    if (s.have_replaygain &&
        (v = moonlit_shared_settings_replaygain_from_str(s.replaygain)) >= 0 &&
        (v = replaygain_shared_to_rockbox(v)) >= 0)
    {
        global_settings.replaygain_settings.type = v;
        dsp_replaygain_set_settings(&global_settings.replaygain_settings);
        touched_global = true;
    }
    if (s.have_language && (v = metro_lang_code_to_enum(s.language)) >= 0)
    {
        metro_settings.language = (enum metro_language)v;
        metro_lang_set(metro_settings.language);
    }
    if (s.have_appearance &&
        (v = moonlit_shared_settings_appearance_from_str(s.appearance)) >= 0)
    {
        metro_settings.theme = (enum metro_theme_kind)v;
        metro_theme_set(metro_settings.theme);
    }

    metro_settings.shared_rev_applied = s.rev;
    metro_settings_save();
    if (touched_global)
    {
        settings_save();
        call_storage_idle_notifys(true);
    }
}

/* moonlit (D-079, contrato v19): cuánto se reserva para preservar
 * claves desconocidas al reescribir -- generoso a proposito, la
 * misma capacidad de moonlit_shared_settings_t.unknown_lines. */
void metro_settings_write_shared(void)
{
    char buf[AURA_SHARED_SETTINGS_BUF];
    char tmp_path[sizeof(AURA_SHARED_SETTINGS_PATH) + 4];
    moonlit_shared_settings_t s;
    long old_rev = 0;
    int fd, n;
    const char *word;

    /* Lee el archivo previo SOLO para heredar su "rev" y las lineas
     * desconocidas -- las 13 claves conocidas se pisan todas con el
     * valor VIGENTE ahora mismo, nunca con lo que decia el archivo
     * viejo (SS A.2.3: "se reescribe el archivo completo... con todas
     * las claves conocidas"). */
    moonlit_shared_settings_init(&s);
    if (read_shared_settings_text(buf, sizeof(buf)) >= 0)
    {
        moonlit_shared_settings_t old;
        if (moonlit_shared_settings_parse(buf, &old))
        {
            old_rev = old.rev;
            strlcpy(s.unknown_lines, old.unknown_lines, sizeof(s.unknown_lines));
        }
    }

    s.have_rev = true;             s.rev = old_rev + 1;
    s.have_updated_by = true;      strlcpy(s.updated_by, "moonlit", sizeof(s.updated_by));
    s.have_screen_lock_enabled = true; s.screen_lock_enabled = metro_settings.screen_lock;
    s.have_screen_lock_pin = true; strlcpy(s.screen_lock_pin, metro_settings.screen_lock_pin,
                                            sizeof(s.screen_lock_pin));
    s.have_screen_lock_require = true;
    word = moonlit_shared_settings_lock_require_to_str((int)metro_settings.screen_lock_require);
    strlcpy(s.screen_lock_require, word ? word : "hold", sizeof(s.screen_lock_require));
    s.have_brightness = true;        s.brightness = global_settings.brightness;
    s.have_backlight_timeout = true; s.backlight_timeout = global_settings.backlight_timeout;
    s.have_idle_poweroff = true;     s.idle_poweroff = global_settings.poweroff;
    s.have_keyclick = true;          s.keyclick = global_settings.keyclick != 0;
    s.have_volume_limit = true;      s.volume_limit = global_settings.volume_limit;
    s.have_replaygain = true;
    strlcpy(s.replaygain,
            moonlit_shared_settings_replaygain_to_str(
                replaygain_rockbox_to_shared(global_settings.replaygain_settings.type)),
            sizeof(s.replaygain));
    s.have_language = true;
    word = metro_lang_code_from_enum(metro_settings.language);
    strlcpy(s.language, word ? word : "es", sizeof(s.language));
    s.have_appearance = true;
    word = moonlit_shared_settings_appearance_to_str((int)metro_settings.theme);
    strlcpy(s.appearance, word ? word : "dark", sizeof(s.appearance));

    n = moonlit_shared_settings_serialize(&s, buf, sizeof(buf));
    if (n < 0)
        return;

    if (!dir_exists("/.aura"))
        mkdir("/.aura");

    /* Escritura atomica (SS A.1): .tmp + rename -- un corte de luz a
     * mitad de escritura deja el archivo VIEJO intacto, nunca uno a
     * medias. rename() sobre un destino existente no es portable
     * (mismo motivo que moonlit_master_art_write()), asi que se borra
     * el viejo primero. */
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", AURA_SHARED_SETTINGS_PATH);
    fd = creat(tmp_path, 0666);
    if (fd < 0)
        return;
    if (write(fd, buf, (size_t)n) != n)
    {
        close(fd);
        remove(tmp_path);
        return;
    }
    close(fd);
    remove(AURA_SHARED_SETTINGS_PATH);
    rename(tmp_path, AURA_SHARED_SETTINGS_PATH);

    metro_settings.shared_rev_applied = s.rev;
    metro_settings_save();
}

/* --- moonlit (D-054): base tagcache compartida en /.aura/tagcache ---- */

/* Mueve (rename) todo "database_*" del arbol activo al directorio
 * compartido: database_idx.tcd, database_<n>.tcd, database_tmp.tcd,
 * database_state.tcd, database_changelog.txt (nombres de
 * apps/tagcache.c TAGCACHE_FILE_*). "database.ignore"/"database_commit.ignore"
 * (marcadores del arbol, no de la base) se quedan. Si `move` es false
 * los borra: el compartido ya existe y esos son restos muertos que
 * solo ocupan disco -- ningun hermano los va a leer (contrato v15). */
static void migrate_tree_db_files(bool move)
{
    DIR *d = opendir(ROCKBOX_DIR);
    struct DIRENT *entry;
    char from[MAX_PATH], to[MAX_PATH];

    if (!d)
        return;
    while ((entry = readdir(d)) != NULL)
    {
        const char *name = entry->d_name;
        size_t len = strlen(name);

        if (strncmp(name, "database_", 9) != 0)
            continue;
        if (len < 5 || (strcmp(name + len - 4, ".tcd") != 0 &&
                        strcmp(name + len - 4, ".txt") != 0))
            continue;
        strlcpy(from, ROCKBOX_DIR "/", sizeof(from));
        strlcat(from, name, sizeof(from));
        if (move)
        {
            strlcpy(to, AURA_SHARED_DB_DIR "/", sizeof(to));
            strlcat(to, name, sizeof(to));
            rename(from, to);
        }
        else
            remove(from);
    }
    closedir(d);
}

void metro_force_shared_db_path(void)
{
    bool shared_exists = file_exists(AURA_SHARED_DB_DIR "/database_idx.tcd");
    bool tree_exists = file_exists(ROCKBOX_DIR "/database_idx.tcd");

    strmemccpy(global_settings.tagcache_db_path, AURA_SHARED_DB_DIR,
               sizeof(global_settings.tagcache_db_path));

    if (!dir_exists("/.aura"))
        mkdir("/.aura");
    if (!dir_exists(AURA_SHARED_DB_DIR))
        mkdir(AURA_SHARED_DB_DIR);

    /* Migracion sin rebuild: la base que este mismo arbol construyo
     * antes de D-054 pasa a ser la compartida, tal cual. */
    if (!shared_exists && tree_exists)
        migrate_tree_db_files(true);
    else if (shared_exists && tree_exists)
        migrate_tree_db_files(false);

    /* Sello v12 (.rockbox/aura/db_stamp.txt) -> compartido, solo si el
     * compartido no existe: describe la base que acabamos de mover. */
    if (!file_exists(AURA_SHARED_DB_STAMP_PATH) &&
        file_exists(METRO_DIR "/db_stamp.txt"))
        rename(METRO_DIR "/db_stamp.txt", AURA_SHARED_DB_STAMP_PATH);
    else if (file_exists(METRO_DIR "/db_stamp.txt"))
        remove(METRO_DIR "/db_stamp.txt");

    /* Una base que este arbol construyo antes de v12 nunca tuvo sello.
     * Misma regla que el arranque en frio de metro_sync_switch_needs_rebuild():
     * la base que el firmware activo venia usando SI esta al dia -- se
     * sella al migrarla, para que el primer cambio de firmware tras la
     * actualizacion no reconstruya. */
    if (!shared_exists && tree_exists && !file_exists(AURA_SHARED_DB_STAMP_PATH))
        metro_sync_record_db_stamp();
}

void metro_ensure_media_dirs(void)
{
    static const char *const dirs[] = { "/Music", "/Videos", "/Photos", "/Playlists" };
    unsigned i;

    for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++)
    {
        if (!dir_exists(dirs[i]))
            mkdir(dirs[i]);
    }
}

void metro_settings_metro_cache_dir(const char *subdir, char *out, size_t outsz)
{
    /* moonlit (D-001/D-023): own cache tree, never the Metro one --
     * both families may coexist on one device (COMPAT C23). */
    snprintf(out, outsz, "%s/moonlitcache/%s", METRO_DIR, subdir);
}

/* moonlit (D-055): shared 80 px thumbs, see metro_settings.h. Only
 * this file spells the path (CLAUDE.md path rule). */
#define AURA_SHARED_THUMBS_DIR "/.aura/thumbs"

void metro_settings_shared_thumbs_dir(const char *subdir, char *out, size_t outsz)
{
    snprintf(out, outsz, "%s/%s", AURA_SHARED_THUMBS_DIR, subdir);
}

/* moonlit (D-059, contrato v16): shared MASTER art under /.aura/art/
 * -- only this file spells the path. */
#define AURA_SHARED_ART_DIR "/.aura/art"

void metro_settings_shared_art_dir(const char *subdir, char *out, size_t outsz)
{
    snprintf(out, outsz, "%s/%s", AURA_SHARED_ART_DIR, subdir);
}

/* moonlit (D-063, contrato v18): la version de formato del arbol de
 * caratulas derivadas. Solo este archivo escribe la ruta (regla de
 * rutas de CLAUDE.md); el formato del archivo vive en
 * moonlit_master_art.c. */
#define AURA_SHARED_ART_FORMAT_FILE AURA_SHARED_ART_DIR "/format.txt"

static bool purge_none(const char *stem, void *ctx)
{
    (void)stem;
    (void)ctx;
    return false; /* nada se conserva */
}

/* Borra de `dir` todo archivo con uno de los sufijos de cache derivada.
 * Deliberadamente por sufijo y no un "borra todo el arbol": estos
 * directorios son COMPARTIDOS (/.aura/art, /.aura/thumbs) y un barrido
 * ciego podria llevarse algo que escribio otra familia o Studio. */
static int purge_derived_dir(const char *dir)
{
    static const char *const suffixes[] = { ".art", ".none", ".mth", ".pfraw" };
    unsigned i;
    int n = 0;

    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++)
        n += moonlit_art_sweep(dir, suffixes[i], purge_none, NULL);
    return n;
}

bool metro_settings_purge_stale_art_caches(void)
{
    static const char *const subdirs[] = { "albums", "artists", "photos" };
    char path[MAX_PATH];
    unsigned i;
    int removed = 0;
    int version;

    if (!dir_exists(AURA_SHARED_ART_DIR))
    {
        /* Nada que purgar todavia; deja el sello puesto igual para que
         * el primer arranque no cuente como "version 0" mas adelante. */
        moonlit_master_art_ensure_dir(AURA_SHARED_ART_DIR);
        moonlit_master_art_format_write(AURA_SHARED_ART_FORMAT_FILE,
                                        MOONLIT_MASTER_ART_FORMAT_VERSION);
        return false;
    }

    version = moonlit_master_art_format_read(AURA_SHARED_ART_FORMAT_FILE);
    if (version >= MOONLIT_MASTER_ART_FORMAT_VERSION)
        return false;

    for (i = 0; i < sizeof(subdirs) / sizeof(subdirs[0]); i++)
    {
        metro_settings_shared_art_dir(subdirs[i], path, sizeof(path));
        removed += purge_derived_dir(path);
        metro_settings_shared_thumbs_dir(subdirs[i], path, sizeof(path));
        removed += purge_derived_dir(path);
        metro_settings_metro_cache_dir(subdirs[i], path, sizeof(path));
        removed += purge_derived_dir(path);
    }
    /* moonlitcache/art: el .pfraw privado de antes de D-059. Ya no se
     * escribe, pero un disco actualizado desde v0.1.4 todavia lo tiene. */
    metro_settings_metro_cache_dir("art", path, sizeof(path));
    removed += purge_derived_dir(path);

    moonlit_master_art_format_write(AURA_SHARED_ART_FORMAT_FILE,
                                    MOONLIT_MASTER_ART_FORMAT_VERSION);
    return removed > 0;
}

bool metro_settings_migrate_shared_thumbs(void)
{
    static const char *const subdirs[] = { "albums", "artists", "photos" };
    char from[MAX_PATH], to[MAX_PATH];
    unsigned i;
    bool moved = false;

    for (i = 0; i < sizeof(subdirs) / sizeof(subdirs[0]); i++)
    {
        metro_settings_metro_cache_dir(subdirs[i], from, sizeof(from));
        if (!dir_exists(from))
            continue;
        metro_settings_shared_thumbs_dir(subdirs[i], to, sizeof(to));
        if (dir_exists(to))
            continue;
        if (!dir_exists("/.aura"))
            mkdir("/.aura");
        if (!dir_exists(AURA_SHARED_THUMBS_DIR))
            mkdir(AURA_SHARED_THUMBS_DIR);
        if (rename(from, to) == 0) /* same FAT partition: atomic, no copy */
            moved = true;
    }
    return moved;
}

/* R3-F3/DD-6 (M-064): Studio's own index + photo cache -- distinct
 * from metro_settings_metro_cache_dir("artists", ...) above, which is
 * Metro's OWN derived 80x80 tile cache
 * (.../aura/moonlitcache/artists/). These two point at
 * .../aura/artist_images.cfg and .../aura/artists/ respectively --
 * Studio writes both, Metro only ever reads them. */
void metro_settings_artist_images_cfg_path(char *out, size_t outsz)
{
    strlcpy(out, METRO_DIR "/artist_images.cfg", outsz);
}

void metro_settings_artists_dir(char *out, size_t outsz)
{
    strlcpy(out, METRO_DIR "/artists", outsz);
}

/* R3-F5/DD-7 (M-066): Studio writes this, Metro only ever reads it --
 * same one-way relationship as artist_images.cfg above. */
void metro_settings_ratings_cfg_path(char *out, size_t outsz)
{
    strlcpy(out, METRO_DIR "/ratings.cfg", outsz);
}

/* --- R5 (M-090, contrato v10): cambio de firmware por renombre -------
 * moonlit (D-047): generalizado a cualquier hermana de
 * metro_firmware_families.h; el nombre del propio arbol dormido
 * (METRO_FW_OWN_DORMANT, D-001) vive alli. */

#define METRO_FW_ACTIVE_DIR    ROCKBOX_DIR        /* "/.rockbox" */
#define METRO_FW_ROOT_BINARY   "/rockbox.ipod"
#define METRO_FW_TREE_BINARY   ROCKBOX_DIR "/rockbox.ipod"

bool metro_firmware_sibling_installed(int i)
{
    const struct metro_fw_family *sibling = metro_fw_sibling(i);

    return sibling != NULL && dir_exists(sibling->dormant_dir);
}

/* /rockbox.ipod := /.rockbox/rockbox.ipod, a trozos, con buffer estatico
 * (nunca en la pila de 8 KB). No es fatal si falla: el bootloader
 * prefiere el del arbol y este es solo el respaldo. */
static void refresh_root_binary(void)
{
    static char buf[16 * 1024];
    int in, out;
    ssize_t n;

    in = open(METRO_FW_TREE_BINARY, O_RDONLY);
    if (in < 0)
        return;
    out = creat(METRO_FW_ROOT_BINARY, 0666);
    if (out < 0)
    {
        close(in);
        return;
    }
    while ((n = read(in, buf, sizeof(buf))) > 0)
        if (write(out, buf, (size_t)n) != n)
            break;
    close(out);
    close(in);
}

bool metro_firmware_switch_to(int i)
{
    const struct metro_fw_family *sibling = metro_fw_sibling(i);

    if (sibling == NULL || !metro_firmware_sibling_installed(i))
        return false;
    if (dir_exists(METRO_FW_OWN_DORMANT))
        return false; /* no adivinar: Studio garantiza que no pase */

    /* 1. todo lo de moonlit al disco, AHORA */
    metro_settings_save();
    settings_save();

    /* moonlit (D-060): don't cut a tagcache commit mid-write -- see
     * DECISIONS.md. tagcache_shutdown() below only flushes the queue of
     * numeric-tag writes (playcount/lastplayed/rating); it never waits
     * for a commit() already in progress (apps/tagcache.c). A reboot
     * while commit_step is nonzero can leave the SHARED master header's
     * dirty flag stuck, forcing every family to rebuild from scratch on
     * its next boot even though the data was fine. Bounded so a wedged
     * commit thread can never block a firmware switch forever; if the
     * cap is hit we proceed anyway -- see D-060 for why that's the
     * right call here. */
#define METRO_SWITCH_COMMIT_WAIT_TICKS (HZ * 8)
    {
        long wait_start = current_tick;
        while (tagcache_get_commit_step() != 0 &&
               TIME_BEFORE(current_tick, wait_start + METRO_SWITCH_COMMIT_WAIT_TICKS))
            sleep(HZ / 10);
    }

    tagcache_shutdown();
    call_storage_idle_notifys(true);

    /* 2. saliente primero */
    if (rename(METRO_FW_ACTIVE_DIR, METRO_FW_OWN_DORMANT) < 0)
        return false;

    /* 3. entrante */
    if (rename(sibling->dormant_dir, METRO_FW_ACTIVE_DIR) < 0)
    {
        /* deshacer el paso 2: seguimos siendo moonlit */
        rename(METRO_FW_OWN_DORMANT, METRO_FW_ACTIVE_DIR);
        return false;
    }

    /* 4 y 5 -- el marcador SOLO si la biblioteca cambio desde que la
     * hermana construyo su base (M-091, contrato v12): sin sync de por
     * medio el cambio es instantaneo, sin "optimizando" de 5 minutos. */
    refresh_root_binary();
    /* moonlit (D-054): la base es compartida -- se compara el sello
     * compartido contra /.aura/library-stamp, el arbol saliente ya no
     * cuenta. */
    if (metro_sync_switch_needs_rebuild())
        metro_sync_write_music_pending_marker();

    /* 6: en seco. Nada de lo de arriba queda pendiente de escribir. */
    system_reboot();
    return true; /* no se alcanza */
}
