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
#include "metro_turnstile_table.h"

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
            s.frames = 8;
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

/* Shared by metro_transitions_slide() (twist, always) and
 * metro_transitions_push()'s fallback (animations=minimal, or
 * graphics=lite even under animations=all) -- both capture/render
 * were already done by the caller. */
static void run_slide(int direction, struct level_spec spec)
{
    int i;

    cpu_boost(true);
    for (i = 1; i <= spec.frames; i++)
    {
        int p = metro_ease(METRO_EASE_OUT_EXPO, i, spec.frames);
        int dx = (direction < 0 ? -1 : 1) * (p * LCD_WIDTH / 256);

        metro_fb_present_slide(s_fb_from, s_fb_to, dx);
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
    run_slide(direction, spec);

    note_transition_cost("slide", spec, start_tick);
}

/* F12: angle in degrees -> nearest metro_turnstile_table.h index.
 * Rounds to the closest of the 32 precomputed angles instead of
 * interpolating between two rows -- the animation only spans ~250ms
 * either way, and PUSH/POP already only run under animations=all
 * (8 frames), so the extra precision an interpolated lookup would buy
 * is not visible. */
static int turnstile_angle_index(int angle_deg)
{
    int span = METRO_TURNSTILE_ANGLE_MAX - METRO_TURNSTILE_ANGLE_MIN;
    int idx = ((angle_deg - METRO_TURNSTILE_ANGLE_MIN) * (METRO_TURNSTILE_ANGLES - 1)
               + span / 2) / span;

    if (idx < 0)
        idx = 0;
    if (idx >= METRO_TURNSTILE_ANGLES)
        idx = METRO_TURNSTILE_ANGLES - 1;
    return idx;
}

/* F12: both surfaces rotate the SAME way each frame -- direction > 0
 * (push): outgoing sweeps 0 -> 50 deg, incoming sweeps -80 -> 0 deg
 * (WP7's own "Turnstile Out"/"Turnstile In (forward)",
 * INVESTIGACION.md F.3). direction < 0 (pop): the exact mirror,
 * outgoing 0 -> -80, incoming 50 -> 0 ("Turnstile In (back)" plus its
 * unstated-but-symmetric out). No opacity fade on top of the
 * projection itself (DESVIACIONES.md F12-1) -- the perspective
 * shrink alone already reads as "leaving". */
static void run_turnstile(int direction, struct level_spec spec)
{
    int i;

    cpu_boost(true);
    for (i = 1; i <= spec.frames; i++)
    {
        int p = metro_ease(METRO_EASE_OUT_EXPO, i, spec.frames);
        int angle_out, angle_in;

        if (direction > 0)
        {
            angle_out = (p * 50) / 256;
            angle_in = -80 + (p * 80) / 256;
        }
        else
        {
            angle_out = -(p * 80) / 256;
            angle_in = 50 - (p * 50) / 256;
        }

        lcd_set_foreground(metro_color_bg());
        lcd_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT);
        metro_fb_draw_turnstile_layer(s_fb_from, turnstile_angle_index(angle_out));
        metro_fb_draw_turnstile_layer(s_fb_to, turnstile_angle_index(angle_in));
        lcd_update();

        drain_button_queue_if_full();
        if (i < spec.frames)
            sleep(spec.frame_delay);
    }
    cpu_boost(false);

    /* The table only has METRO_TURNSTILE_ANGLES discrete samples
     * (metro_turnstile_table.h) -- the last frame's angle_in rounds
     * to whichever one is closest to 0 deg, rarely exactly 0, so the
     * projected result is a near-miss of the real destination rather
     * than a pixel-exact match. Settle on the real thing once,
     * unprojected -- otherwise the header/pivots (which nothing
     * redraws again after this, unlike the rows FEATHER touches next)
     * would stay very slightly warped for good. */
    lcd_bitmap_part(s_fb_to, 0, 0, LCD_WIDTH, 0, 0, LCD_WIDTH, LCD_HEIGHT);
    lcd_update();
}

bool metro_transitions_effective_all(void)
{
    return effective_level() == METRO_ANIM_ALL;
}

void metro_transitions_push(metro_transitions_draw_fn draw_to, int direction)
{
    enum metro_anim_level level = effective_level();
    struct level_spec spec = anim_level_spec(level);
    long start_tick = current_tick;

    if (!lcd_active() || spec.frames == 0)
    {
        draw_to();
        return;
    }

    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);

    if (level == METRO_ANIM_ALL && metro_settings.graphics == METRO_GFX_FULL)
    {
        run_turnstile(direction, spec);
        note_transition_cost("push-turnstile", spec, start_tick);
    }
    else
    {
        run_slide(direction, spec);
        note_transition_cost("push-slide", spec, start_tick);
    }
}

/* FADE's own timing (6x3 under `all`, PLAN_MAESTRO.md S3.3) rather
 * than anim_level_spec()'s SLIDE numbers -- only reached when the
 * graphics=full real-blend path below is taken. */
static const struct level_spec fade_spec = { 6, 3 };

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
     * graphics=lite -- rides the same slide as PUSH/POP/twist instead
     * of a second bespoke fallback animation. */
    if (level == METRO_ANIM_MINIMAL || metro_settings.graphics != METRO_GFX_FULL)
    {
        metro_transitions_slide(draw_to, 1);
        return;
    }

    start_tick = current_tick;
    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);

    cpu_boost(true);
    for (i = 1; i <= fade_spec.frames; i++)
    {
        int p = metro_ease(METRO_EASE_LINEAR, i, fade_spec.frames);

        metro_fb_present_fade(s_fb_from, s_fb_to, p);
        drain_button_queue_if_full();
        if (i < fade_spec.frames)
            sleep(fade_spec.frame_delay);
    }
    cpu_boost(false);

    note_transition_cost("fade", fade_spec, start_tick);
}
