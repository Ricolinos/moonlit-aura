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

/* R2-F2/DD-7: metro_nav_move_sel_grid() -- same windowing contract as
 * metro_nav_move_sel() (clamp, remember-per-pivot, push/pop) already
 * covered above via the untouched metro_nav_move_sel() itself; these
 * cover what's actually NEW: row-aligned windowing (first_visible is
 * always a multiple of `cols`, never a mid-row remainder) and a
 * count that isn't a multiple of cols (the grid's real-world case --
 * a photo library rarely lands on an exact multiple of 4). */

static void test_grid_init(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);
}

static void test_grid_move_row_alignment(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    /* 21 tiles, 4 cols, 2 visible rows (real geometry, DD-8) */
    metro_nav_move_sel_grid(&nav, 1, 21, 4, 2);
    CHECK(metro_nav_sel(&nav) == 1);
    CHECK(metro_nav_first_visible(&nav) == 0); /* row 0, still in window */

    metro_nav_move_sel_grid(&nav, 6, 21, 4, 2); /* sel -> 7, row 1 -- still in [0,2) */
    CHECK(metro_nav_sel(&nav) == 7);
    CHECK(metro_nav_first_visible(&nav) == 0);

    metro_nav_move_sel_grid(&nav, 1, 21, 4, 2); /* sel -> 8, row 2 -- pushes the window */
    CHECK(metro_nav_sel(&nav) == 8);
    CHECK(metro_nav_first_visible(&nav) == 4); /* row 1 * cols, first row scrolled off */
    CHECK(metro_nav_first_visible(&nav) % 4 == 0); /* always row-aligned */
}

static void test_grid_move_clamps_at_edges(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    metro_nav_move_sel_grid(&nav, -5, 21, 4, 2); /* clamped to 0 */
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);

    metro_nav_move_sel_grid(&nav, 999, 21, 4, 2); /* clamped to count-1 */
    CHECK(metro_nav_sel(&nav) == 20);
    /* count=21, cols=4 -> 6 rows (0..5); max first row = 6-2 = 4 -> *cols = 16 */
    CHECK(metro_nav_first_visible(&nav) == 16);
    CHECK(metro_nav_first_visible(&nav) % 4 == 0);
}

static void test_grid_count_not_multiple_of_cols(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    /* 21 isn't a multiple of 4 -- row 5 (the last) only has 1 tile
     * (index 20), not a full row of 4. Landing sel exactly on it must
     * not push first_visible past the real last row. */
    metro_nav_move_sel_grid(&nav, 20, 21, 4, 2);
    CHECK(metro_nav_sel(&nav) == 20);
    CHECK(metro_nav_first_visible(&nav) == 16);

    /* One step back (sel -> 19, still row 4) must not move the window
     * either -- row 4 is already the first visible row. */
    metro_nav_move_sel_grid(&nav, -1, 21, 4, 2);
    CHECK(metro_nav_sel(&nav) == 19);
    CHECK(metro_nav_first_visible(&nav) == 16);
}

static void test_grid_selection_empty(void)
{
    metro_nav_t nav;
    metro_nav_init(&nav, 1);
    metro_nav_push(&nav, 1);

    metro_nav_move_sel_grid(&nav, 5, 0, 4, 2); /* count == 0 */
    CHECK(metro_nav_sel(&nav) == 0);
    CHECK(metro_nav_first_visible(&nav) == 0);
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
    test_grid_init();
    test_grid_move_row_alignment();
    test_grid_move_clamps_at_edges();
    test_grid_count_not_multiple_of_cols();
    test_grid_selection_empty();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
