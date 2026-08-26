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
#include "lcd.h"
#include "kernel.h"
#include "system.h"
#include "button.h"
#include "debug.h"
/* F13: LOGF_ENABLE before logf.h opts THIS file's logf() calls into
 * Rockbox's own circular buffer (firmware/debug.c) -- without it,
 * logf() compiles to a no-op even where ROCKBOX_HAS_LOGF is defined
 * (logf.h's own per-file gate, to keep the 16KB buffer from filling
 * with every file's logging at once). */
#define LOGF_ENABLE
#include "logf.h"

#include "metro_transitions.h"
#include "metro_fb.h"
#include "metro_motion.h"
#include "metro_settings.h"
#include "metro_theme.h"
#include "metro_draw.h"  /* R3-F8: metro_draw_text() para el volador */
#include "moonlit_fonts.h"
#include "moonlit_palette.h" /* moonlit (D-052 C3): color del filo de luna, una vez por transicion */
#include "string-extra.h"

#define METRO_FB_PIXELS (LCD_WIDTH * LCD_HEIGHT)

static fb_data s_fb_from[METRO_FB_PIXELS];
static fb_data s_fb_to[METRO_FB_PIXELS];

/* DEBUGF is a no-op outside DEBUG builds (only useful in the sim,
 * where it always goes to stderr regardless -- debug.h) -- on real
 * hardware it stays silent even with DEBUG defined, since generic
 * firmware/debug.c's own debug() hook is a no-op there (no JTAG/GDB
 * connected). logf() is what's actually observable on real hardware
 * without extra equipment: ipod6g has ROCKBOX_HAS_LOGF, so this ships
 * straight to Rockbox's own on-device log viewer (Debug menu -> "Show
 * Log File") -- see docs/ESTADO_FINAL.md for the full measurement
 * procedure this exists for (PLAN_MAESTRO.md S3.4). */
#define METRO_TRACE(fmt, ...) \
    do { \
        DEBUGF("metro_transitions: " fmt "\n", ##__VA_ARGS__); \
        logf("metro_transitions: " fmt, ##__VA_ARGS__); \
    } while (0)

/* Real bug class documented by Aura-Firmware's own aura_transitions.c
 * (D-074): these loops sleep() between frames without ever reading a
 * button, so several transitions chained by a user holding/repeating
 * a button can overflow the button queue before anything drains it.
 * Not draining every frame the button (that would mean reacting to
 * input mid-animation, real added complexity) -- just keeping the
 * queue from growing without bound. */
static void drain_button_queue_if_full(void)
{
    if (button_queue_full())
        button_clear_queue();
}

/* moonlit (D-011, M4): duracion de PUSH/POP bajo animations=all, desde
 * design-system/tokens.json:motion.transition_ms (220ms) -- literal, no
 * macro importada: este archivo no puede incluir moonlit_tokens.h
 * (moonlit_palette.c/.h es su unico includer, M4). HZ=100 fijo en todo
 * Rockbox (firmware/kernel/include/tick.h), asi que 3 ticks de
 * frame_delay = 30ms/cuadro; 220/30 = 7.33 -> 7 cuadros (210ms real,
 * antes 8 cuadros = 240ms sin relacion con ningun token). */
#define METRO_TRANSITION_MS 220

struct level_spec {
    int frames;
    int frame_delay;
};

static struct level_spec anim_level_spec(enum metro_anim_level level)
{
    struct level_spec s;

    switch (level)
    {
        case METRO_ANIM_ALL:
            s.frames = 7; /* == METRO_TRANSITION_MS / (3 ticks * 10ms), redondeado */
            s.frame_delay = 3;
            break;
        case METRO_ANIM_MINIMAL:
            s.frames = 4;
            s.frame_delay = 3;
            break;
        default:
            s.frames = 0;
            s.frame_delay = 0;
            break;
    }
    return s;
}

/* M-015 auto-degradation: > 2x budget in 3 consecutive transitions
 * drops the EFFECTIVE animations level one notch for the rest of this
 * session -- metro_settings.animations (the user's own persisted
 * choice) is never touched. Session-only by construction: these are
 * plain statics, reset to 0 on every boot. */
static int s_degrade_notches = 0;
static int s_over_budget_streak = 0;

static enum metro_anim_level effective_level(void)
{
    int lvl = (int)metro_settings.animations - s_degrade_notches;

    return (lvl < METRO_ANIM_OFF) ? METRO_ANIM_OFF : (enum metro_anim_level)lvl;
}

static void note_transition_cost(const char *name, struct level_spec spec, long start_tick)
{
    long elapsed = current_tick - start_tick;

    (void)name; /* only actually read inside METRO_TRACE -- a no-op on
                   non-DEBUG target builds (debug.h), unused there otherwise */
    long budget = (long)spec.frames * spec.frame_delay;

    METRO_TRACE("%s %d frames in %ld ticks (budget %ld)", name, spec.frames, elapsed, budget);

    if (budget > 0 && elapsed > budget * 2)
    {
        s_over_budget_streak++;
        if (s_over_budget_streak >= 3 && effective_level() > METRO_ANIM_OFF)
        {
            s_degrade_notches++;
            s_over_budget_streak = 0;
            METRO_TRACE("auto-degrade: effective animations level now %d",
                        (int)effective_level());
        }
    }
    else
        s_over_budget_streak = 0;
}

/* --- R3-F8/DD-9 (M-069): CONTINUUM --------------------------------------
 *
 * El título de la fila elegida vuela hasta la ceja de la página nueva
 * mientras el resto del PUSH se desliza detrás (D-052 C1: antes giraba, F12). Es el continuum
 * real de WP7: lo que el usuario tocó es lo único que NO gira -- se
 * queda plano, legible, y viaja a su lugar nuevo.
 *
 * Posición interpolada de forma continua; tamaño por ESCALONES de las
 * fuentes que Metro ya tiene, nunca escalado por muestreo (son fuentes
 * bitmap de tamaño fijo y un escalado falso se ve sucio). Aquí el
 * escalón es uno solo -- MFONT_LIST_SEL (20 px, la fila seleccionada)
 * a MFONT_LABEL (18 px, la ceja, moonlit M4) -- y no los tres que DD-9 suponía,
 * porque el destino real resultó ser la ceja y no el título grande:
 * ver docs/DESVIACIONES.md R3-7. El color acompaña al mismo salto
 * (fg -> secondary), en el mismo cuadro, para que el cambio se lea
 * como un solo paso y no como dos efectos desacoplados. */

/* Destino: (x, y) exactos de la ceja en metro_draw_header(). El x de
 * la fila de origen es el mismo (METRO_ROWS_LEFT_X), así que el vuelo
 * es puramente vertical -- por eso no se interpola x. */
#define METRO_CONTINUUM_TO_Y 4

static char s_cont_text[METRO_CONTINUUM_TITLE_MAX];
static int  s_cont_from_y;
static bool s_cont_armed = false;

void metro_transitions_arm_continuum(const char *text, int from_y)
{
    if (!text || !text[0])
        return;

    strlcpy(s_cont_text, text, sizeof(s_cont_text));
    s_cont_from_y = from_y;
    s_cont_armed = true;
}

/* Borra la ceja de la página de destino DENTRO del buffer off-screen,
 * antes de que empiece la animación: si no, se vería dos veces al
 * mismo tiempo (la de s_fb_to deslizándose con el PUSH, y el volador
 * encima viajando hacia ella). Se recorta al ancho real del texto para
 * no tocar el reloj ni la batería, que viven en el otro extremo del
 * mismo encabezado. */
static void erase_dest_eyebrow(void)
{
    int w, h;

    lcd_setfont(metro_font_id(MFONT_LABEL));
    lcd_getstringsize((const unsigned char *)s_cont_text, &w, &h);

    metro_fb_fill_rect(s_fb_to, METRO_DRAW_LEFT_X, METRO_CONTINUUM_TO_Y,
                        w, h, metro_color_bg());
}

/* Un cuadro del volador, encima de las dos capas del slide y antes
 * del único lcd_update() del cuadro.
 *
 * Curva OUT_QUAD -- la misma que desde D-052 usa el propio PUSH/POP
 * (antes el giro de F12 iba en OUT_EXPO y el volador en OUT_QUAD, y el
 * argumento de entonces sigue valiendo para ambos): out-expo gasta el
 * 82 % del recorrido en los dos primeros cuadros de ocho -- perfecto
 * para una rotación que debe sentirse instantánea, pésimo para un
 * texto que tiene que LEERSE viajando. Con out-expo el título
 * aterrizaba casi de inmediato y se quedaba quieto los seis cuadros
 * restantes, que es justo lo contrario de lo que CONTINUUM cuenta.
 * Out-quad reparte el viaje a lo largo de toda la animación y sigue
 * frenando al llegar. Encontrado al intentar capturar el vuelo a mitad
 * y descubrir que a mitad ya no había vuelo. */
static void draw_continuum_frame(int i, int frames)
{
    int p = metro_ease(METRO_EASE_OUT_QUAD, i, frames);
    int y = s_cont_from_y + ((METRO_CONTINUUM_TO_Y - s_cont_from_y) * p) / 256;
    bool landed = (p >= 128);

    metro_draw_text(landed ? MFONT_LABEL : MFONT_LIST_SEL,
                     METRO_DRAW_LEFT_X, y, s_cont_text,
                     landed ? metro_color_secondary() : metro_color_fg());
}

/* moonlit (D-052 C1/C3): the ONE slide loop every push/pop/twist runs
 * through -- both capture/render were already done by the caller.
 * direction < 0: `to` enters from the left ("Luz de canto": PUSH under
 * every level, pivot-prev twist); direction > 0: `to` enters from the
 * right (POP, pivot-next twist). Each frame: two row-copy blits
 * (LCD_WIDTH*LCD_HEIGHT px total), the 1px "Filo de luna" seam
 * (LCD_HEIGHT px, skipped by compose_slide on the last frame where the
 * seam would be a screen edge), CONTINUUM's flying title if armed, one
 * lcd_update(). ~77k px/frame -- a third of the F12 rotation's 230k
 * column-walk this replaced. */
static void run_slide(int direction, struct level_spec spec, enum metro_ease_kind ease,
                      bool continuum, const char *name)
{
    long seam = (long)moonlit_surface(MSURFACE_HIGH, MEDGE_LIGHT);
    long start_tick = current_tick;
    int i;

    if (continuum)
        erase_dest_eyebrow();

    cpu_boost(true);
    for (i = 1; i <= spec.frames; i++)
    {
        int p = metro_ease(ease, i, spec.frames);
        int dx = (direction < 0 ? -1 : 1) * (p * LCD_WIDTH / 256);

        metro_fb_compose_slide(s_fb_from, s_fb_to, dx, seam);
        /* Encima de las dos capas y antes del único update del cuadro
         * -- el contrato "compón varias capas, actualiza una vez" que
         * el giro de F12 establecía y compose_slide() hereda. En el
         * último cuadro el volador ES la ceja de la página nueva (misma
         * fuente, color y posición), que erase_dest_eyebrow() había
         * borrado de s_fb_to para que no se viera doble. */
        if (continuum)
            draw_continuum_frame(i, spec.frames);
        lcd_update();
        METRO_TRACE("%s frame %d/%d at +%ld ticks", name, i, spec.frames,
                    current_tick - start_tick);

        drain_button_queue_if_full();
        if (i < spec.frames)
            sleep(spec.frame_delay);
    }
    cpu_boost(false);
}

void metro_transitions_slide(metro_transitions_draw_fn draw_to, int direction)
{
    struct level_spec spec = anim_level_spec(effective_level());
    long start_tick = current_tick;

    if (!lcd_active() || spec.frames == 0)
    {
        draw_to();
        return;
    }

    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);
    /* El twist conserva su dirección natural (izquierda/derecha según
     * el pivote) y su curva OUT_EXPO de siempre -- D-052 solo cambia
     * PUSH/POP. */
    run_slide(direction, spec, METRO_EASE_OUT_EXPO, false, "slide");

    note_transition_cost("slide", spec, start_tick);
}

bool metro_transitions_effective_all(void)
{
    return effective_level() == METRO_ANIM_ALL;
}

/* moonlit (D-052 C1, "Luz de canto"): PUSH/POP is a slide from the
 * LEFT at every level -- direction > 0 (deepening) means the new page
 * enters from x < 0 (compose_slide dx < 0), direction < 0 (going
 * back) is the exact mirror: the outgoing page retreats to the left
 * and the destination is uncovered from the right. That is the
 * INVERSE of the twist's convention, on purpose: light comes from the
 * left (D-012), so what is new arrives from there. Replaces the
 * F12 rotation that ran here under ALL+FULL. Easing OUT_QUAD, not
 * OUT_EXPO: see draw_continuum_frame() -- at 7 frames the expo curve
 * lands almost at once and idles, the quad one spreads the travel over
 * all 210 ms and still brakes at the end. Under MINIMAL (4 frames) and
 * under graphics=lite the same loop runs; only the frame count and
 * CONTINUUM (ALL+FULL, PUSH only) differ. */
void metro_transitions_push(metro_transitions_draw_fn draw_to, int direction)
{
    enum metro_anim_level level = effective_level();
    struct level_spec spec = anim_level_spec(level);
    long start_tick = current_tick;
    /* R3-F8/DD-9 (M-069): el armado es de un solo uso -- se consume
     * aquí SIEMPRE, se llegue o no a animarlo. Si no se limpiara, un
     * PUSH sin continuidad real (o un POP, o un push con la animación
     * apagada) heredaría el texto volador del anterior. */
    bool continuum = s_cont_armed;

    s_cont_armed = false;

    if (!lcd_active() || spec.frames == 0)
    {
        draw_to();
        return;
    }

    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);

    /* Tercera puerta de CONTINUUM (las otras dos: nivel ALL con
     * graphics=full, y que solo PUSH llegue hasta aquí): direction > 0
     * = ir hacia adentro. Al volver (POP) no hay fila de origen de la
     * cual volar. */
    continuum = continuum && direction > 0 &&
                level == METRO_ANIM_ALL && metro_settings.graphics == METRO_GFX_FULL;

    run_slide(-direction, spec, METRO_EASE_OUT_QUAD, continuum,
              direction > 0 ? "push" : "pop");
    note_transition_cost(direction > 0 ? "push" : "pop", spec, start_tick);
}

/* moonlit (D-052 C2, "Menguante"): 7 frames x 3 ticks = 210 ms, the
 * same budget as PUSH (METRO_TRANSITION_MS 220 rounded to whole
 * 30 ms frames, D-037) instead of Metro's 6x3; OUT_QUAD instead of
 * LINEAR, so the blend starts decisively and settles like the moon
 * waning. Only reached on the graphics=full real-blend path below. */
static const struct level_spec fade_spec = { 7, 3 };

void metro_transitions_fade(metro_transitions_draw_fn draw_to)
{
    enum metro_anim_level level = effective_level();
    long start_tick;
    int i;

    if (!lcd_active() || level == METRO_ANIM_OFF)
    {
        draw_to();
        return;
    }

    /* present_fade() is reserved to graphics=full (metro_fb.h); every
     * other combination -- including animations=all with
     * graphics=lite -- rides the same slide as PUSH (C1, from the
     * left) instead of a second bespoke fallback animation. */
    if (level == METRO_ANIM_MINIMAL || metro_settings.graphics != METRO_GFX_FULL)
    {
        metro_transitions_push(draw_to, 1);
        return;
    }

    start_tick = current_tick;
    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);

    cpu_boost(true);
    for (i = 1; i <= fade_spec.frames; i++)
    {
        int p = metro_ease(METRO_EASE_OUT_QUAD, i, fade_spec.frames);

        metro_fb_present_fade(s_fb_from, s_fb_to, p);
        METRO_TRACE("fade frame %d/%d at +%ld ticks", i, fade_spec.frames,
                    current_tick - start_tick);
        drain_button_queue_if_full();
        if (i < fade_spec.frames)
            sleep(fade_spec.frame_delay);
    }
    cpu_boost(false);

    note_transition_cost("fade", fade_spec, start_tick);
}

void metro_transitions_trace(const char *name, int frame, int frames, long start_tick)
{
    (void)name; (void)frame; (void)frames; (void)start_tick;
    METRO_TRACE("%s frame %d/%d at +%ld ticks", name, frame, frames,
                current_tick - start_tick);
}
