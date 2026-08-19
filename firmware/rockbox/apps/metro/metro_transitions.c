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

#include "metro_transitions.h"
#include "metro_fb.h"
#include "metro_motion.h"
#include "metro_settings.h"

#define METRO_FB_PIXELS (LCD_WIDTH * LCD_HEIGHT)

static fb_data s_fb_from[METRO_FB_PIXELS];
static fb_data s_fb_to[METRO_FB_PIXELS];

/* DEBUGF is a no-op outside DEBUG builds -- always present in the
 * code at zero release cost, PLAN_MAESTRO.md S3.4. */
#define METRO_TRACE(fmt, ...) \
    DEBUGF("metro_transitions: " fmt "\n", ##__VA_ARGS__)

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

void metro_transitions_slide(metro_transitions_draw_fn draw_to, int direction)
{
    struct level_spec spec = anim_level_spec(effective_level());
    long start_tick = current_tick;
    int i;

    if (!lcd_active() || spec.frames == 0)
    {
        draw_to();
        return;
    }

    metro_fb_capture(s_fb_from);
    metro_fb_render(s_fb_to, draw_to);

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

    note_transition_cost("slide", spec, start_tick);
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
