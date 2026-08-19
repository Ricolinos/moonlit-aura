/* Tests host-side de metro_nav.c: sin dependencias de Rockbox, compila
 * y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../metro_nav.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_init(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);

    CHECK(metro_nav_depth(&nav) == 1);
    CHECK(metro_nav_is_root(&nav));
    CHECK(metro_nav_pivot_count(&nav) == 1);
    CHECK(metro_nav_pivot(&nav) == 0);
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);
}

static void test_push_pop(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);

    CHECK(metro_nav_push(&nav, 4)); /* e.g. music: artists|albums|songs|genres */
    CHECK(metro_nav_depth(&nav) == 2);
    CHECK(!metro_nav_is_root(&nav));
    CHECK(metro_nav_pivot_count(&nav) == 4);

    CHECK(metro_nav_push(&nav, 2)); /* e.g. one artist: albums|songs */
    CHECK(metro_nav_depth(&nav) == 3);
    CHECK(metro_nav_pivot_count(&nav) == 2);

    CHECK(metro_nav_pop(&nav));
    CHECK(metro_nav_depth(&nav) == 2);
    CHECK(metro_nav_pivot_count(&nav) == 4);

    CHECK(metro_nav_pop(&nav));
    CHECK(metro_nav_depth(&nav) == 1);
    CHECK(metro_nav_is_root(&nav));
}

static void test_pop_at_root_is_noop(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);

    CHECK(!metro_nav_pop(&nav));
    CHECK(metro_nav_depth(&nav) == 1);
}

static void test_max_depth_is_respected(void)
{
    metro_nav_t nav;
    int i;
    metro_nav_init(&nav, 1);

    for (i = 1; i < METRO_NAV_MAX_DEPTH; i++)
        CHECK(metro_nav_push(&nav, 3));

    CHECK(metro_nav_depth(&nav) == METRO_NAV_MAX_DEPTH);
    CHECK(!metro_nav_push(&nav, 3));
    CHECK(metro_nav_depth(&nav) == METRO_NAV_MAX_DEPTH);
}

static void test_pop_to_root(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 3);
    metro_nav_push(&nav, 3);
    metro_nav_push(&nav, 3);

    CHECK(metro_nav_depth(&nav) == 4);
    metro_nav_pop_to_root(&nav);
    CHECK(metro_nav_depth(&nav) == 1);
    CHECK(metro_nav_is_root(&nav));
}

static void test_pivot_no_wrap(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 3);

    CHECK(metro_nav_pivot(&nav) == 0);
    CHECK(!metro_nav_pivot_prev(&nav)); /* already at first, no wrap */
    CHECK(metro_nav_pivot(&nav) == 0);

    CHECK(metro_nav_pivot_next(&nav));
    CHECK(metro_nav_pivot(&nav) == 1);
    CHECK(metro_nav_pivot_next(&nav));
    CHECK(metro_nav_pivot(&nav) == 2);
    CHECK(!metro_nav_pivot_next(&nav)); /* already at last, no wrap */
    CHECK(metro_nav_pivot(&nav) == 2);

    CHECK(metro_nav_pivot_prev(&nav));
    CHECK(metro_nav_pivot(&nav) == 1);
}

static void test_selection_and_window_move(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    /* 30 rows, 5 visible -- matches the hub/list windowing spec */
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);

    metro_nav_move_sel(&nav, 1, 30, 5);
    CHECK(metro_nav_sel(&nav) == 1);
    CHECK(metro_nav_first_visible(&nav) == 0); /* still within first window */

    metro_nav_move_sel(&nav, 10, 30, 5); /* sel -> 11, past row 4 */
    CHECK(metro_nav_sel(&nav) == 11);
    CHECK(metro_nav_first_visible(&nav) == 7); /* 11 - 5 + 1 */

    metro_nav_move_sel(&nav, -20, 30, 5); /* sel -> 0 (clamped) */
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);

    metro_nav_move_sel(&nav, 100, 30, 5); /* sel -> 29 (clamped to last row) */
    CHECK(metro_nav_sel(&nav) == 29);
    CHECK(metro_nav_first_visible(&nav) == 25); /* 29 - 5 + 1 */
}

static void test_selection_empty_list(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    metro_nav_move_sel(&nav, 5, 0, 5); /* row_count == 0 */
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);
}

static void test_selection_remembered_per_pivot(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 2); /* two pivots, e.g. artists|albums */

    metro_nav_move_sel(&nav, 5, 30, 5); /* move sel in pivot 0 */
    CHECK(metro_nav_sel(&nav) == 5);

    metro_nav_pivot_next(&nav); /* switch to pivot 1 */
    CHECK(metro_nav_sel(&nav) == 0); /* independent selection */
    metro_nav_move_sel(&nav, 2, 30, 5);
    CHECK(metro_nav_sel(&nav) == 2);

    metro_nav_pivot_prev(&nav); /* back to pivot 0 */
    CHECK(metro_nav_sel(&nav) == 5); /* remembered, per S2.1 */
}

static void test_selection_remembered_across_push_pop(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);
    metro_nav_move_sel(&nav, 7, 30, 5);
    CHECK(metro_nav_sel(&nav) == 7);

    metro_nav_push(&nav, 1); /* enter a child page */
    metro_nav_move_sel(&nav, 3, 10, 5);
    CHECK(metro_nav_sel(&nav) == 3);

    metro_nav_pop(&nav); /* back to parent -- exact position restored */
    CHECK(metro_nav_sel(&nav) == 7);
    CHECK(metro_nav_first_visible(&nav) == 3); /* 7 - 5 + 1 */
}

static void test_set_sel_direct(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    metro_nav_set_sel(&nav, 15, 30, 5);
    CHECK(metro_nav_sel(&nav) == 15);
    CHECK(metro_nav_first_visible(&nav) == 11);

    metro_nav_set_sel(&nav, -5, 30, 5); /* clamped to 0 */
    CHECK(metro_nav_sel(&nav) == 0);

    metro_nav_set_sel(&nav, 999, 30, 5); /* clamped to last row */
    CHECK(metro_nav_sel(&nav) == 29);
}

int main(void)
{
    test_init();
    test_push_pop();
    test_pop_at_root_is_noop();
    test_max_depth_is_respected();
    test_pop_to_root();
    test_pivot_no_wrap();
    test_selection_and_window_move();
    test_selection_empty_list();
    test_selection_remembered_per_pivot();
    test_selection_remembered_across_push_pop();
    test_set_sel_direct();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
