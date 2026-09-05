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

#include "audio.h"
#include "file.h"
#include "jpeg_load.h"
#include "bmp.h"
#include "kernel.h" /* moonlit (D-082): current_tick, HZ -- el debounce de scrubbing */

#include "metro_screen_photo_viewer.h"
#include "metro_screen_list.h"
#include "metro_nav.h" /* moonlit (D-072): conservar la seleccion al volver */
#include "metro_music.h" /* R4/FA-8: metro_music_playpause() */
#include "metro_draw.h"
#include "metro_transitions.h" /* moonlit (D-072) */
#include "metro_theme.h"
#include "metro_lang.h"
#include "metro_keymap.h"
#include "moonlit_master_art.h" /* moonlit (D-082): moonlit_master_art_read() -- la vista previa de scrubbing */
#include "moonlit_art_cache.h"  /* moonlit (D-082): moonlit_art_master_file_path() */

#define PHOTOS_DIR "/Photos" /* same local-constant precedent metro_photos.c/metro_video.c already use */

/* R2-F3/DD-10 (M-058): oversized on purpose (x2 margin, DECISIONS.md
 * M-033) -- FORMAT_RESIZE needs real headroom over the final bitmap
 * size before it downscales, not just LCD_WIDTH*LCD_HEIGHT*sizeof(fb_data).
 * Own buffer, not shared with Now Playing's metro_albumart.c -- both
 * can be the largest thing on screen for their own page, but never at
 * the same time (this page's sentinel and Now Playing's are mutually
 * exclusive, see metro_screen_photo_viewer_is_current()). */
#define METRO_PHOTO_VIEW_SCRATCH_SIZE (LCD_WIDTH * LCD_HEIGHT * 2 * 2)
static unsigned char s_scratch[METRO_PHOTO_VIEW_SCRATCH_SIZE];

/* Only show a "loading" message before a decode that will actually be
 * noticeable -- same threshold Aura-Firmware's aura_photos.c uses
 * (read-only reference), which also happens to match this contract's
 * own photo size cap (COMPAT_STUDIO.md C13, <=640px): a contract-
 * compliant photo never shows this, it's a courtesy for anything that
 * slipped past that cap. */
#define METRO_PHOTO_LOADING_INDICATOR_SIDE 640

/* moonlit (D-082, maestro SS C.2, portado de Metro M-109): ventana de
 * "quietud" antes de decodificar de verdad. >= 150 ms sin eventos de
 * rueda nuevos. HZ = 100 en este target, asi que son exactamente 15
 * ticks, sin redondeo. */
#define METRO_PHOTO_SETTLE_TICKS (HZ * 150 / 1000)

static const metro_photo_item_t *s_items;
static int s_count;
static int s_index;

/* moonlit (D-082): current_tick del ultimo cambio de s_index (por
 * rueda o por LEFT/RIGHT). Mientras `current_tick - s_nav_tick` sea
 * menor que METRO_PHOTO_SETTLE_TICKS, el visor esta "en scrubbing": se
 * muestra la vista previa barata (draw_scrub_preview()) y NO se
 * decodifica nada, ni se consume el deslizamiento (s_slide_dir se
 * consume recien en el redibujo asentado, dentro de
 * metro_screen_photo_viewer_show()). metro_screen_photo_viewer_push()
 * lo deja deliberadamente "viejo" -- abrir el visor debe mostrar la
 * foto de una vez, no la vista previa (que es para scrubbing DENTRO
 * del visor, no para su apertura). */
static long s_nav_tick;

static int s_loaded_index = -1;
static bool s_loaded_ok = false;
/* Not reset on push() on purpose -- persists across photos and across
 * viewer sessions during the same boot (matches Aura-Firmware's own
 * viewer, whose equivalent flag is likewise never reset per-photo).
 * Starts at false (ajustar) the first time ever, same default DD-10
 * implies by listing "ajustar" first. */
static bool s_cover_mode = false;
static int s_display_w, s_display_h;
static struct bitmap s_bm;

static int s_probed_index = -1;
static bool s_probe_ok = false;
static int s_probed_w, s_probed_h;

/* --- JPEG dimension probe (no pixel decode) -------------------------
 *
 * Ported from Aura-Firmware's aura_photos.c (probe_jpeg_dimensions(),
 * read as reference per PLAN-metro-r2-maestro.md DD-10) -- reads only
 * the JPEG marker stream up to SOFn, never decodes a single pixel.
 * Needed for "cubrir": computing the cover scale factor ahead of the
 * real decode requires knowing the source dimensions first (unlike
 * the photo grid's thumbnails, R2-F2/DD-9, which are small enough to
 * get away with a single KEEP_ASPECT decode + a cheap nearest-neighbour
 * re-crop instead -- see metro_thumbs.c's own comment on why
 * that shortcut isn't precise enough here at full screen size). */
typedef enum {
    METRO_JPEG_PROBE_UNKNOWN = 0, /* unrecognized header -- let the decoder try anyway */
    METRO_JPEG_PROBE_BASELINE,
    METRO_JPEG_PROBE_UNSUPPORTED, /* progressive/arithmetic SOF -- Rockbox can't decode it */
} metro_jpeg_probe_t;

static metro_jpeg_probe_t probe_jpeg_dimensions(const char *path, int *out_w, int *out_h)
{
    int fd;
    unsigned char marker[2];
    metro_jpeg_probe_t result = METRO_JPEG_PROBE_UNKNOWN;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return METRO_JPEG_PROBE_UNKNOWN;

    if (read(fd, marker, 2) != 2 || marker[0] != 0xFF || marker[1] != 0xD8)
    {
        close(fd);
        return METRO_JPEG_PROBE_UNKNOWN;
    }

    for (;;)
    {
        unsigned char lenbuf[2];
        unsigned char sofbuf[5];
        int len;

        if (read(fd, marker, 2) != 2 || marker[0] != 0xFF)
            break;
        while (marker[1] == 0xFF) /* padding bytes before the real marker code */
        {
            if (read(fd, &marker[1], 1) != 1)
                break;
        }

        if (marker[1] == 0x01 || (marker[1] >= 0xD0 && marker[1] <= 0xD7))
            continue; /* TEM/RSTn -- no length field */
        if (marker[1] == 0xD9 || marker[1] == 0xDA)
            break; /* EOI/SOS with no SOFn seen -- give up */

        if (read(fd, lenbuf, 2) != 2)
            break;
        len = (lenbuf[0] << 8) | lenbuf[1];

        if (marker[1] == 0xC0 || marker[1] == 0xC1) /* SOF0/SOF1 baseline */
        {
            if (read(fd, sofbuf, 5) == 5)
            {
                *out_h = (sofbuf[1] << 8) | sofbuf[2];
                *out_w = (sofbuf[3] << 8) | sofbuf[4];
                result = METRO_JPEG_PROBE_BASELINE;
            }
            break;
        }
        if (marker[1] >= 0xC2 && marker[1] <= 0xCF && marker[1] != 0xC4 && marker[1] != 0xC8)
        {
            result = METRO_JPEG_PROBE_UNSUPPORTED;
            break;
        }

        if (len < 2 || lseek(fd, len - 2, SEEK_CUR) < 0)
            break;
    }

    close(fd);
    return result;
}

/* --- cover-mode scale math (Q16.16, no FPU) --------------------------
 *
 * Ported from Aura-Firmware's compute_decode_and_display_size()
 * (aura_photos.c:943-1002, read as reference per DD-10) -- computes
 * the cover scale factor (max(LCD_WIDTH/w, LCD_HEIGHT/h), the opposite
 * of FORMAT_KEEP_ASPECT's fit-scale), then clamps the DECODE size down
 * (display size stays the true cover size) if that would overflow
 * METRO_PHOTO_VIEW_SCRATCH_SIZE -- an integer sqrt by bisection, the same
 * shift-instead-of-float fixed-point idiom metro_motion.c's easing
 * tables use, just a different shift width. */
static void compute_decode_and_display_size(int src_w, int src_h,
                                             int *decode_w, int *decode_h,
                                             int *display_w, int *display_h)
{
    long scale_x = ((long)LCD_WIDTH << 16) / src_w;
    long scale_y = ((long)LCD_HEIGHT << 16) / src_h;
    long ideal_scale = (scale_x > scale_y) ? scale_x : scale_y; /* max: cover, not fit */
    long dw, dh;

    dw = (src_w * ideal_scale) >> 16;
    dh = (src_h * ideal_scale) >> 16;
    if (dw < 1) dw = 1;
    if (dh < 1) dh = 1;
    *display_w = (int)dw;
    *display_h = (int)dh;

    if (ideal_scale <= (1L << 16))
    {
        /* Source already covers without enlarging -- decode straight
         * to the final size, cheapest path (1:1 blit, no resampling). */
        *decode_w = *display_w;
        *decode_h = *display_h;
    }
    else
    {
        *decode_w = src_w;
        *decode_h = src_h;
    }

    /* R2-F3 bug found verifying this against a real fixture (640x300,
     * cover mode): Aura-Firmware's own formula (ported above) only
     * checks decode_w*decode_h against the scratch size -- but
     * FORMAT_RESIZE without FORMAT_KEEP_ASPECT can't always reach an
     * arbitrary target through the JPEG decoder's own DCT power-of-two
     * downscale steps (1/1, 1/2, 1/4, 1/8) alone; when the requested
     * decode size falls strictly BETWEEN two of those steps (as it did
     * here: 512x240 sits between 640x300 at 1/1 and 320x150 at 1/2),
     * the decoder has to decode the FULL SOURCE resolution as an
     * intermediate buffer before its own resize pass can shrink that
     * down to decode_w x decode_h -- read_jpeg_file() failed outright
     * (ret <= 0) rather than silently corrupting anything, so this was
     * caught, not a repeat of M-033's crash. Fix: budget against
     * whichever of decode/source pixel counts is larger, and actually
     * reserve JPEG_DECODE_OVERHEAD (apps/recorder/jpeg_load.h) instead
     * of assuming it fits in whatever slack happened to be left over. */
    {
        long decode_px = (long)(*decode_w) * (*decode_h);
        long src_px = (long)src_w * src_h;
        long check_px = (decode_px > src_px) ? decode_px : src_px;
        long budget_bytes = (long)METRO_PHOTO_VIEW_SCRATCH_SIZE - (long)JPEG_DECODE_OVERHEAD;

        if (budget_bytes < 0)
            budget_bytes = 0;

        if (check_px * (long)sizeof(fb_data) > budget_bytes)
        {
            long budget_px = budget_bytes / (long)sizeof(fb_data);
            long num = budget_px << 16, den = check_px;
            long mem_scale2 = num / den; /* factor^2, Q16.16 */
            long lo = 0, hi = 1L << 16, mem_scale;
            while (lo < hi)
            {
                long mid = (lo + hi + 1) / 2;
                if ((mid * mid) >> 16 <= mem_scale2)
                    lo = mid;
                else
                    hi = mid - 1;
            }
            mem_scale = lo;
            *decode_w = (int)(((long)(*decode_w) * mem_scale) >> 16);
            *decode_h = (int)(((long)(*decode_h) * mem_scale) >> 16);
            if (*decode_w < 1) *decode_w = 1;
            if (*decode_h < 1) *decode_h = 1;
        }
    }
}

/* Nearest-neighbour blit, ported from Aura-Firmware's
 * draw_scaled_centered() (aura_photos.c, read as reference per DD-10):
 * `display_w`x`display_h` is the TRUE final on-screen size (cover mode
 * can exceed the decode buffer's own size, DD-9-class memory clamp
 * above) -- centers and crops against the screen when larger, centers
 * and leaves the surrounding metro_color_bg() showing when smaller
 * (fit mode's letterbox bars, already painted by metro_draw_clear()
 * before this runs). 1:1 fast path with lcd_bitmap_part() when no
 * resampling is actually needed. */
static void draw_scaled_centered(const fb_data *src, int src_w, int src_h,
                                  int display_w, int display_h)
{
    int display_x0 = (display_w > LCD_WIDTH) ? (display_w - LCD_WIDTH) / 2 : 0;
    int display_y0 = (display_h > LCD_HEIGHT) ? (display_h - LCD_HEIGHT) / 2 : 0;
    int screen_x0 = (display_w < LCD_WIDTH) ? (LCD_WIDTH - display_w) / 2 : 0;
    int screen_y0 = (display_h < LCD_HEIGHT) ? (LCD_HEIGHT - display_h) / 2 : 0;
    int draw_w = (display_w < LCD_WIDTH) ? display_w : LCD_WIDTH;
    int draw_h = (display_h < LCD_HEIGHT) ? display_h : LCD_HEIGHT;
    int x, y;

    if (src_w == display_w && src_h == display_h)
    {
        lcd_bitmap_part(src, display_x0, display_y0, src_w, screen_x0, screen_y0, draw_w, draw_h);
        return;
    }

    for (y = 0; y < draw_h; y++)
    {
        int dy = display_y0 + y;
        int sy = dy * src_h / display_h;
        const fb_data *srow;
        fb_data *drow = FBADDR(screen_x0, screen_y0 + y);

        if (sy >= src_h) sy = src_h - 1;
        srow = src + (long)sy * src_w;
        for (x = 0; x < draw_w; x++)
        {
            int dx = display_x0 + x;
            int sx = dx * src_w / display_w;
            if (sx >= src_w) sx = src_w - 1;
            drow[x] = srow[sx];
        }
    }
}

/* moonlit (D-082, maestro SS C.2, portado de Metro M-109): vista
 * previa INSTANTANEA mientras la rueda todavia se esta moviendo -- la
 * maestra compartida de 80 px (contrato v16/D-059, la MISMA que ya
 * llena la cuadricula, D-072) ampliada a 240x240 (llena el alto de
 * pantalla, centrada) con el nombre y la posicion debajo.
 * draw_scaled_centered() ya sabe centrar y ampliar por muestreo
 * vecino-mas-cercano (es la misma funcion que dibuja el modo "cubrir"
 * de la foto completa) -- se reusa tal cual, sin una segunda
 * primitiva de escalado. Nunca decodifica un JPEG: si la maestra
 * todavia no existe (biblioteca grande, el constructor en segundo
 * plano no ha llegado a esta foto todavia) se ve solo el texto sobre
 * el fondo limpio -- preferible a forzar un decode que reintroduciria
 * el bloqueo que este mecanismo existe para evitar. */
#define METRO_PHOTO_PREVIEW_SIDE (LCD_HEIGHT) /* 240: llena el alto, dejando aire a los lados */

/* moonlit (D-082): franja de fondo SOLIDO al pie, pintada
 * explicitamente con lcd_fillrect() antes del texto -- el CLAUDE.md
 * prohibe conseguir un fondo opaco detras de texto via DRMODE_SOLID
 * (M-051), asi que cualquier sitio que de verdad necesite texto
 * legible sobre una imagen lo pinta a mano (mismo patron que la
 * caption de un tile, metro_draw.c). La vista previa ya llena los
 * 240 px de alto (METRO_PHOTO_PREVIEW_SIDE == LCD_HEIGHT), asi que no
 * queda aire debajo donde dibujar el nombre/indice -- tiene que ir
 * SOBRE la imagen, no despues de ella. */
#define METRO_PHOTO_PREVIEW_CAPTION_H 40

static fb_data s_preview_master[MOONLIT_MASTER_ART_PHOTO_SIZE * MOONLIT_MASTER_ART_PHOTO_SIZE];

static void draw_preview_caption(void)
{
    char buf[METRO_FSUTIL_NAME_LEN + 16];
    int w, h, y;

    y = LCD_HEIGHT - METRO_PHOTO_PREVIEW_CAPTION_H;
    lcd_set_foreground(metro_color_bg());
    lcd_fillrect(0, y, LCD_WIDTH, METRO_PHOTO_PREVIEW_CAPTION_H);

    snprintf(buf, sizeof(buf), "%s", s_items[s_index].filename);
    /* moonlit (D-081, addendum): por tramos -- un nombre de archivo
     * cirilico se mide con su propia fuente, no con la primaria. */
    metro_draw_text_size(MFONT_LABEL, buf, &w, &h);
    metro_draw_text_cut_right(MFONT_LABEL, (LCD_WIDTH - w) / 2 > 0 ? (LCD_WIDTH - w) / 2 : 4,
                              y + 4, buf, metro_color_fg(), LCD_WIDTH - 8);

    snprintf(buf, sizeof(buf), "%d / %d", s_index + 1, s_count);
    metro_draw_text_size(MFONT_LABEL, buf, &w, &h);
    metro_draw_text(MFONT_LABEL, (LCD_WIDTH - w) / 2, y + 4 + h + 2,
                    buf, metro_color_secondary());
}

static void draw_scrub_preview(void)
{
    char master_path[MAX_PATH];
    char photo_path[MAX_PATH];

    metro_draw_clear();

    snprintf(photo_path, sizeof(photo_path), "%s/%s", PHOTOS_DIR, s_items[s_index].filename);
    moonlit_art_master_file_path('p', "photos", photo_path, s_items[s_index].mtime,
                                 master_path, sizeof(master_path));

    if (moonlit_master_art_read(master_path, MOONLIT_MASTER_ART_PHOTO_SIZE, s_preview_master))
        draw_scaled_centered(s_preview_master, MOONLIT_MASTER_ART_PHOTO_SIZE,
                             MOONLIT_MASTER_ART_PHOTO_SIZE,
                             METRO_PHOTO_PREVIEW_SIDE, METRO_PHOTO_PREVIEW_SIDE);

    draw_preview_caption();
    lcd_update();
}

static void probe_current(void)
{
    char path[MAX_PATH];

    if (s_probed_index == s_index)
        return;
    s_probed_index = s_index;

    snprintf(path, sizeof(path), "%s/%s", PHOTOS_DIR, s_items[s_index].filename);
    s_probe_ok = (probe_jpeg_dimensions(path, &s_probed_w, &s_probed_h) == METRO_JPEG_PROBE_BASELINE);
}

static void load_current(void)
{
    char path[MAX_PATH];
    int format;
    int ret;

    if (s_loaded_index == s_index)
        return;
    s_loaded_index = s_index;
    s_loaded_ok = false;

    snprintf(path, sizeof(path), "%s/%s", PHOTOS_DIR, s_items[s_index].filename);

    s_bm.data = (char *)s_scratch;
#if (LCD_DEPTH > 1)
    s_bm.maskdata = NULL;
#endif

    /* Cover mode needs the probed source size to compute the exact
     * decode/display split above; without it (probe failed) fall back
     * to fit regardless of s_cover_mode -- no source dimensions to
     * cover-crop against. */
    if (s_cover_mode && s_probe_ok)
    {
        int decode_w, decode_h;
        compute_decode_and_display_size(s_probed_w, s_probed_h, &decode_w, &decode_h,
                                         &s_display_w, &s_display_h);
        s_bm.width = decode_w;
        s_bm.height = decode_h;
        format = FORMAT_NATIVE | FORMAT_RESIZE;
    }
    else
    {
        s_bm.width = LCD_WIDTH;
        s_bm.height = LCD_HEIGHT;
        format = FORMAT_NATIVE | FORMAT_RESIZE | FORMAT_KEEP_ASPECT;
    }

    ret = read_jpeg_file(path, &s_bm, sizeof(s_scratch), format, NULL);
    s_loaded_ok = (ret > 0);
    if (s_loaded_ok && !(s_cover_mode && s_probe_ok))
    {
        /* Ajustar: the decoder already landed on the exact final size
         * (KEEP_ASPECT) -- decode and display are the same thing. */
        s_display_w = s_bm.width;
        s_display_h = s_bm.height;
    }
}

static void draw_centered_message(enum metro_lang_id id)
{
    const char *text = metro_lang_str(id);
    int w, h;

    /* moonlit (D-081, addendum): por tramos -- cadena traducida. */
    metro_draw_text_size(MFONT_LABEL, text, &w, &h);
    metro_draw_text(MFONT_LABEL, (LCD_WIDTH - w) / 2, (LCD_HEIGHT - h) / 2,
                     text, metro_color_fg());
}

/* --- sentinel page: same pattern as metro_screen_nowplaying.c's --
 * never drawn/queried through the generic list path, just a
 * stack-bookkeeping placeholder metro_screen_photo_viewer_is_current()
 * recognizes by pointer. */

static int sentinel_count(void *ctx) { (void)ctx; return 0; }
static void sentinel_get_row(void *ctx, int index, struct metro_row *out)
{ (void)ctx; (void)index; (void)out; }
static void sentinel_on_select(void *ctx, int index) { (void)ctx; (void)index; }

static const struct metro_pivot sentinel_pivots[] = {
    { LANG_HUB_PHOTOS, sentinel_count, sentinel_get_row, sentinel_on_select, NULL },
};
static const struct metro_page sentinel_page = { LANG_HUB_PHOTOS, sentinel_pivots, 1, NULL };

bool metro_screen_photo_viewer_push(const metro_photo_item_t *items, int count, int start_index)
{
    if (count <= 0)
        return false;
    if (!metro_screen_list_push(&sentinel_page))
        return false;

    s_items = items;
    s_count = count;
    s_index = start_index;
    if (s_index < 0) s_index = 0;
    if (s_index > count - 1) s_index = count - 1;

    s_loaded_index = -1;
    s_probed_index = -1;
    /* moonlit (D-082): "ya paso el tiempo de quietud" desde el primer
     * dibujo -- abrir el visor debe mostrar la foto de una vez, no la
     * vista previa (que es para scrubbing DENTRO del visor). */
    s_nav_tick = current_tick - METRO_PHOTO_SETTLE_TICKS;
    return true;
}

/* moonlit (D-082, portado de Metro M-109): true mientras haya que
 * seguir sondeando -- durante el debounce de 150 ms (metro_main.c baja
 * su espera a HZ/20), y una vuelta MAS despues de que el debounce
 * termina, para que ese ultimo redibujo dispare el decode real. Se
 * apaga sola en cuanto la foto asentada queda decodificada: de ahi en
 * mas no hace falta seguir despertando al bucle principal hasta el
 * proximo evento de rueda. */
bool metro_screen_photo_viewer_wants_ticks(void)
{
    if (s_count == 0)
        return false;
    return (current_tick - s_nav_tick < METRO_PHOTO_SETTLE_TICKS) ||
           (s_loaded_index != s_index);
}

bool metro_screen_photo_viewer_is_current(void)
{
    return metro_screen_list_current_page() == &sentinel_page;
}

/* moonlit (D-072): direccion armada por el ultimo cambio de foto, o 0
 * si el repintado no viene de un cambio (entrada al visor, cambio de
 * ajustar/cubrir, vuelta de una sesion USB). */
static int s_slide_dir;

static void draw_current_photo(void);

void metro_screen_photo_viewer_show(void)
{
    if (s_count == 0)
        return;

    /* moonlit (D-082, maestro SS C.1/C.2, portado de Metro M-109):
     * mientras la rueda sigue moviendose (o LEFT/RIGHT se siguen
     * apretando), NINGUN indice intermedio se decodifica -- solo se
     * dibuja la vista previa barata del destino ACTUAL, y el
     * deslizamiento armado (s_slide_dir) NO se consume todavia: el
     * primer redibujo asentado (fuera de esta ventana) es el que
     * decide la direccion, como si todo el gesto hubiera sido un solo
     * paso. El decode real de una foto ocurre como mucho una vez por
     * gesto, cuando el usuario se detiene -- antes de esto, cada paso
     * de la rueda disparaba su propio decode sincrono en
     * draw_current_photo(), asi que un giro continuo (incluso una vez
     * corregido D-082 en metro_keymap.c para que TODOS sus eventos
     * llegaran) habria seguido sintiendose atascado. */
    if (current_tick - s_nav_tick < METRO_PHOTO_SETTLE_TICKS)
    {
        draw_scrub_preview();
        return;
    }

    /* moonlit (D-072): si el repintado viene de pasar de foto, entra
     * deslizando. metro_transitions_photo_slide() ya cae solo a un
     * dibujo directo con animations=off o el LCD dormido. */
    if (s_slide_dir != 0)
    {
        int dir = s_slide_dir;

        s_slide_dir = 0;
        metro_transitions_photo_slide(draw_current_photo, dir);
        return;
    }

    draw_current_photo();
}

static void draw_current_photo(void)
{
    metro_draw_clear();

    probe_current();

    if (s_loaded_index != s_index)
    {
        bool big = s_probe_ok && (s_probed_w > METRO_PHOTO_LOADING_INDICATOR_SIDE ||
                                   s_probed_h > METRO_PHOTO_LOADING_INDICATOR_SIDE);
        if (big)
        {
            /* Only forced when it will actually be noticeable (see
             * METRO_PHOTO_LOADING_INDICATOR_SIDE's comment) -- an
             * explicit lcd_update() here because load_current() below
             * blocks; the normal draw cycle only flips to the real LCD
             * after this whole function returns. */
            draw_centered_message(LANG_PHOTO_LOADING);
            lcd_update();
        }
        load_current();
    }

    if (!s_loaded_ok)
    {
        draw_centered_message(LANG_PHOTO_UNSUPPORTED);
        lcd_update();
        return;
    }

    draw_scaled_centered((const fb_data *)s_scratch, s_bm.width, s_bm.height,
                          s_display_w, s_display_h);
    lcd_update();
}

void metro_screen_photo_viewer_handle(int action, int steps)
{
    switch (action)
    {
        /* moonlit (D-072): al cambiar de foto se arma el deslizamiento
         * horizontal; el dibujo lo hace metro_main.c en su repintado, y
         * metro_screen_photo_viewer_show() lo consume. Se arma aqui y no
         * se dibuja aqui para no tener dos caminos de dibujo del visor
         * (mismo criterio que CONTINUUM, D-052 C1).
         *
         * moonlit (D-082, portado de Metro M-109): tambien se reinicia
         * el reloj de quietud -- CADA evento de este tipo mueve
         * s_index (barato, sin decodificar nada) y reinicia
         * s_nav_tick; un giro rapido reinicia el reloj en cada paso, asi
         * que el decode real (dentro de metro_screen_photo_viewer_show())
         * no ocurre hasta que la rueda se detiene 150 ms. El reloj SOLO
         * se reinicia si el indice de verdad cambio -- igual que el
         * guard que ya existia para `s_slide_dir`: sin este guard,
         * seguir apretando en la primera o la ultima foto (clamp sin
         * cambio real) mantendria wants_ticks() en true para siempre. */
        case MACT_PREV:
        {
            int new_index = s_index - steps;
            if (new_index < 0) new_index = 0;
            if (new_index != s_index)
            {
                s_slide_dir = -1;
                s_nav_tick = current_tick;
            }
            s_index = new_index;
            break;
        }
        case MACT_NEXT:
        {
            int new_index = s_index + steps;
            if (new_index > s_count - 1) new_index = s_count - 1;
            if (new_index != s_index)
            {
                s_slide_dir = 1;
                s_nav_tick = current_tick;
            }
            s_index = new_index;
            break;
        }
        case MACT_TOGGLE_VIEW_MODE:
            s_cover_mode = !s_cover_mode;
            s_loaded_index = -1; /* DD-10: re-decode, never resample a stale buffer */
            break;
        case MACT_BACK:
            /* moonlit (D-072, plan de la ronda): volver conserva la
             * seleccion. El visor puede haber avanzado varias fotos con
             * la rueda o con LEFT/RIGHT, y la rejilla se quedaba en la
             * que se abrio -- perder el sitio despues de recorrer
             * cincuenta fotos es exactamente lo que uno no quiere. Se
             * escribe ANTES del pop: despues, el marco de arriba de la
             * pila ya es otro. */
            metro_nav_pop(metro_screen_nav());
            /* metro_nav_move_sel_grid(), no _set_sel(): en una
             * cuadricula `first_visible` TIENE que ser multiplo de
             * METRO_TILE_COLS (metro_draw_tiles() mapea slot->fila/
             * columna suponiendolo), y _set_sel() ventana por filas
             * sueltas -- dejaria la rejilla corrida. */
            metro_nav_move_sel_grid(metro_screen_nav(),
                                     s_index - metro_nav_sel(metro_screen_nav()),
                                     s_count, METRO_TILE_COLS,
                                     METRO_TILE_ROWS_VISIBLE);
            break;
        case MACT_HOME:
            metro_screen_list_pop_to_root();
            break;
        case MACT_PLAYPAUSE:
            /* La música sigue sonando con el visor abierto (DD-10); el
             * toggle es el compartido de metro_music.c (R4/FA-8). */
            metro_music_playpause();
            break;
        default:
            break;
    }
}
