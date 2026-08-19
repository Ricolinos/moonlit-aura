/* Tests host-side de metro_motion.c: sin dependencias de Rockbox,
 * compila y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../metro_motion.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_endpoints(void)
{
    enum metro_ease_kind kinds[] = { METRO_EASE_LINEAR, METRO_EASE_OUT_QUAD, METRO_EASE_OUT_EXPO };
    int frame_counts[] = { 1, 4, 8, 15 };
    unsigned k, f;

    for (k = 0; k < sizeof(kinds) / sizeof(kinds[0]); k++)
    {
        for (f = 0; f < sizeof(frame_counts) / sizeof(frame_counts[0]); f++)
        {
            int frames = frame_counts[f];

            CHECK(metro_ease(kinds[k], 0, frames) == 0);
            CHECK(metro_ease(kinds[k], frames, frames) == 256);
            /* Out-of-range i clamps the same as the endpoints instead
             * of reading past the lookup table or dividing oddly --
             * callers pass i in [0, frames] by construction, but the
             * canonical loop's "i <= frames" boundary makes i==frames
             * the last in-range call, so one-past should still behave. */
            CHECK(metro_ease(kinds[k], -1, frames) == 0);
            CHECK(metro_ease(kinds[k], frames + 1, frames) == 256);
        }
    }
}

static void test_monotonic_and_bounded(void)
{
    enum metro_ease_kind kinds[] = { METRO_EASE_LINEAR, METRO_EASE_OUT_QUAD, METRO_EASE_OUT_EXPO };
    unsigned k;
    int i, frames = 8;

    for (k = 0; k < sizeof(kinds) / sizeof(kinds[0]); k++)
    {
        int prev = -1;

        for (i = 0; i <= frames; i++)
        {
            int p = metro_ease(kinds[k], i, frames);

            CHECK(p >= 0 && p <= 256);
            CHECK(p >= prev);
            prev = p;
        }
    }
}

static void test_linear_is_exact(void)
{
    CHECK(metro_ease(METRO_EASE_LINEAR, 2, 8) == 64);
    CHECK(metro_ease(METRO_EASE_LINEAR, 4, 8) == 128);
    CHECK(metro_ease(METRO_EASE_LINEAR, 6, 8) == 192);
}

static void test_ease_out_expo_shape(void)
{
    /* Ease-OUT: fast start, settling toward the end -- the first
     * frame alone should already cover more than half the distance
     * (WP7's Exponent=6 curve, INVESTIGACION.md F.3), and each later
     * frame should add less than the one before it. */
    int frames = 8, i;
    int prev_delta = 257; /* > any possible single-frame delta */

    CHECK(metro_ease(METRO_EASE_OUT_EXPO, 1, frames) > 128);

    for (i = 1; i <= frames; i++)
    {
        int p = metro_ease(METRO_EASE_OUT_EXPO, i, frames);
        int prev_p = metro_ease(METRO_EASE_OUT_EXPO, i - 1, frames);
        int delta = p - prev_p;

        CHECK(delta <= prev_delta);
        prev_delta = delta;
    }
}

static void test_ease_out_quad_shape(void)
{
    /* No overshoot (D-245-style curves, never past 256) and strictly
     * gentler than ease_out_expo at the start (less front-loaded). */
    int frames = 8;

    CHECK(metro_ease(METRO_EASE_OUT_QUAD, 1, frames) <
          metro_ease(METRO_EASE_OUT_EXPO, 1, frames));
    CHECK(metro_ease(METRO_EASE_OUT_QUAD, frames, frames) == 256);
}

int main(void)
{
    test_endpoints();
    test_monotonic_and_bounded();
    test_linear_is_exact();
    test_ease_out_expo_shape();
    test_ease_out_quad_shape();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
