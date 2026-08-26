/* moonlit: derived from aura_art.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-020, D-042). Ejecutar con
 * `make -C apps/metro/test` -- ../moonlit_art.c se compila y enlaza
 * standalone gracias a los sustitutos de host file.h/lcd.h en este
 * mismo directorio (D-042). */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> /* mkdir() -- D-056 sweep test */
#include <unistd.h>   /* rmdir() */
#include "../moonlit_art.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define TEST_SIZE   24
#define TEST_RADIUS 8
#define TEST_PATH   "build/test_art_tmp.pfraw"

static void fill_gradient(fb_data *buf, int size, unsigned base)
{
    int i;
    for (i = 0; i < size * size; i++)
        buf[i] = (fb_data)(base + i);
}

static void test_round_trip(void)
{
    fb_data src[TEST_SIZE * TEST_SIZE];
    fb_data dst[TEST_SIZE * TEST_SIZE];

    fill_gradient(src, TEST_SIZE, 0x1000);

    moonlit_art_write_pfraw(TEST_PATH, TEST_SIZE, TEST_RADIUS, 0, src);

    CHECK(moonlit_art_pfraw_is_cached(TEST_PATH, TEST_SIZE, TEST_RADIUS, 0));
    CHECK(moonlit_art_read_pfraw(TEST_PATH, TEST_SIZE, TEST_RADIUS, 0, dst));
    CHECK(memcmp(src, dst, sizeof(src)) == 0);
}

static void test_header_rejects_mismatch(void)
{
    fb_data src[TEST_SIZE * TEST_SIZE];
    fb_data dst[TEST_SIZE * TEST_SIZE];

    fill_gradient(src, TEST_SIZE, 0x2000);
    moonlit_art_write_pfraw(TEST_PATH, TEST_SIZE, TEST_RADIUS, 0, src);

    /* size distinto -- cabecera no coincide */
    CHECK(!moonlit_art_pfraw_is_cached(TEST_PATH, TEST_SIZE + 1, TEST_RADIUS, 0));
    CHECK(!moonlit_art_read_pfraw(TEST_PATH, TEST_SIZE + 1, TEST_RADIUS, 0, dst));

    /* radius distinto -- cabecera no coincide */
    CHECK(!moonlit_art_pfraw_is_cached(TEST_PATH, TEST_SIZE, TEST_RADIUS + 1, 0));

    /* theme distinto (D-027: cambio de esquema invalida la cache) */
    CHECK(!moonlit_art_pfraw_is_cached(TEST_PATH, TEST_SIZE, TEST_RADIUS, 1));

    /* archivo inexistente */
    CHECK(!moonlit_art_pfraw_is_cached("build/no-such-file.pfraw", TEST_SIZE, TEST_RADIUS, 0));
}

static void test_mask_corners_leaves_bg_at_corners(void)
{
    fb_data buf[TEST_SIZE * TEST_SIZE];
    unsigned bg = 0x0000;
    int i;

    for (i = 0; i < TEST_SIZE * TEST_SIZE; i++)
        buf[i] = 0xFFFF;

    moonlit_art_mask_corners(buf, TEST_SIZE, TEST_RADIUS, bg);

    /* La esquina extrema de cada una de las 4 -- fuera del arco por
     * construccion (distancia al centro del cuarto de circulo siempre
     * >= radius) -- debe quedar exactamente en bg, sin importar el
     * valor original. */
    CHECK(buf[0] == (fb_data)bg);                                   /* (0,0) */
    CHECK(buf[TEST_SIZE - 1] == (fb_data)bg);                       /* (0,size-1) */
    CHECK(buf[(TEST_SIZE - 1) * TEST_SIZE] == (fb_data)bg);         /* (size-1,0) */
    CHECK(buf[(TEST_SIZE - 1) * TEST_SIZE + TEST_SIZE - 1] == (fb_data)bg); /* (size-1,size-1) */

    /* El centro del bitmap, lejos de toda esquina, no se toca. */
    CHECK(buf[(TEST_SIZE / 2) * TEST_SIZE + TEST_SIZE / 2] == 0xFFFF);
}

/* D-049: moonlit_art_count_uncached() -- the number the "preparando
 * biblioteca" screen shows as its total. 4 paths: two valid, one with a
 * stale header (other theme), one missing -> 2 pending. */
static const char *const s_count_paths[4] = {
    "build/test_art_count_0-24.pfraw", /* valid */
    "build/test_art_count_1-24.pfraw", /* valid */
    "build/test_art_count_2-24.pfraw", /* header for theme 1, asked for 0 */
    "build/test_art_count_3-24.pfraw", /* never written */
};

static void count_path_at(int index, char *out, size_t outsz, void *ctx)
{
    const char *const *paths = ctx;
    snprintf(out, outsz, "%s", paths[index]);
}

static void test_count_uncached(void)
{
    fb_data src[TEST_SIZE * TEST_SIZE];

    fill_gradient(src, TEST_SIZE, 0x3000);
    moonlit_art_write_pfraw(s_count_paths[0], TEST_SIZE, TEST_RADIUS, 0, src);
    moonlit_art_write_pfraw(s_count_paths[1], TEST_SIZE, TEST_RADIUS, 0, src);
    moonlit_art_write_pfraw(s_count_paths[2], TEST_SIZE, TEST_RADIUS, 1, src);
    remove(s_count_paths[3]);

    CHECK(moonlit_art_count_uncached(4, count_path_at, (void *)s_count_paths,
                                     TEST_SIZE, TEST_RADIUS, 0) == 2);
    /* Other theme: only the third one matches now. */
    CHECK(moonlit_art_count_uncached(4, count_path_at, (void *)s_count_paths,
                                     TEST_SIZE, TEST_RADIUS, 1) == 3);
    /* Empty library: nothing pending, path_fn never called. */
    CHECK(moonlit_art_count_uncached(0, count_path_at, (void *)s_count_paths,
                                     TEST_SIZE, TEST_RADIUS, 0) == 0);
}

/* D-056: negative cache. The .none marker shares the D-055 key with the
 * .pfraw ("<dir>/<key>-<size>.pfraw" -> "<dir>/<key>.none"), counts as
 * resolved for the pre-pass, and is swept as an orphan like a .pfraw. */
static void test_none_path(void)
{
    char out[MOONLIT_ART_PATH_MAX];

    CHECK(moonlit_art_none_path("build/a-031b464b.1787718316-120.pfraw", out, sizeof(out)));
    CHECK(strcmp(out, "build/a-031b464b.1787718316.none") == 0);
    /* no directory component */
    CHECK(moonlit_art_none_path("a-deadbeef.1-24.pfraw", out, sizeof(out)));
    CHECK(strcmp(out, "a-deadbeef.1.none") == 0);
    /* not a .pfraw path / no "-<size>" -> refused, out emptied */
    CHECK(!moonlit_art_none_path("build/a-deadbeef.1.none", out, sizeof(out)));
    CHECK(out[0] == '\0');
    CHECK(!moonlit_art_none_path("build/nodash.pfraw", out, sizeof(out)));
    CHECK(!moonlit_art_none_path("", out, sizeof(out)));
    /* too small a buffer */
    CHECK(!moonlit_art_none_path("build/a-deadbeef.1-24.pfraw", out, 8));
}

static void test_none_marker_round_trip(void)
{
    const char *pfraw = "build/test_art_none-24.pfraw";
    char none[MOONLIT_ART_PATH_MAX];
    fb_data src[TEST_SIZE * TEST_SIZE];

    CHECK(moonlit_art_none_path(pfraw, none, sizeof(none)));
    remove(pfraw);
    remove(none);

    CHECK(!moonlit_art_none_exists(none));
    CHECK(!moonlit_art_is_resolved(pfraw, TEST_SIZE, TEST_RADIUS, 0));

    moonlit_art_write_none(none);
    CHECK(moonlit_art_none_exists(none));
    /* resolved for every theme/size: "no art" is not theme-specific */
    CHECK(moonlit_art_is_resolved(pfraw, TEST_SIZE, TEST_RADIUS, 0));
    CHECK(moonlit_art_is_resolved(pfraw, TEST_SIZE, TEST_RADIUS, 1));
    /* still not a positive hit */
    CHECK(!moonlit_art_pfraw_is_cached(pfraw, TEST_SIZE, TEST_RADIUS, 0));

    /* a real .pfraw resolves on its own too */
    remove(none);
    fill_gradient(src, TEST_SIZE, 0x4000);
    moonlit_art_write_pfraw(pfraw, TEST_SIZE, TEST_RADIUS, 0, src);
    CHECK(moonlit_art_is_resolved(pfraw, TEST_SIZE, TEST_RADIUS, 0));
    CHECK(!moonlit_art_is_resolved(pfraw, TEST_SIZE, TEST_RADIUS, 1));
    remove(pfraw);
}

static void test_count_uncached_treats_none_as_cached(void)
{
    char none[MOONLIT_ART_PATH_MAX];

    /* from test_count_uncached(): [2] stale header, [3] missing -> 2.
     * Marking [3] as "no art" leaves only the stale one pending. */
    CHECK(moonlit_art_none_path(s_count_paths[3], none, sizeof(none)));
    moonlit_art_write_none(none);
    CHECK(moonlit_art_count_uncached(4, count_path_at, (void *)s_count_paths,
                                     TEST_SIZE, TEST_RADIUS, 0) == 1);
    remove(none);
    CHECK(moonlit_art_count_uncached(4, count_path_at, (void *)s_count_paths,
                                     TEST_SIZE, TEST_RADIUS, 0) == 2);
}

static bool keep_live_stem(const char *stem, void *ctx)
{
    (void)ctx;
    return strcmp(stem, "a-11111111.1") == 0;
}

static void test_sweep_removes_orphan_none_and_pfraw(void)
{
    const char *dir = "build/test_art_gc";
    const char *live_pfraw = "build/test_art_gc/a-11111111.1-24.pfraw";
    const char *live_none = "build/test_art_gc/a-11111111.1.none";
    const char *orphan_pfraw = "build/test_art_gc/a-22222222.1-24.pfraw";
    const char *orphan_none = "build/test_art_gc/a-33333333.1.none";
    const char *flag = "build/test_art_gc/.gc-pending";
    fb_data src[TEST_SIZE * TEST_SIZE];

    mkdir(dir, 0777);
    fill_gradient(src, TEST_SIZE, 0x5000);
    moonlit_art_write_pfraw(live_pfraw, TEST_SIZE, TEST_RADIUS, 0, src);
    moonlit_art_write_pfraw(orphan_pfraw, TEST_SIZE, TEST_RADIUS, 0, src);
    moonlit_art_write_none(live_none);
    moonlit_art_write_none(orphan_none);
    moonlit_art_write_none(flag);

    CHECK(moonlit_art_sweep(dir, "-24.pfraw", keep_live_stem, NULL) == 1);
    CHECK(moonlit_art_sweep(dir, ".none", keep_live_stem, NULL) == 1);

    CHECK(moonlit_art_pfraw_is_cached(live_pfraw, TEST_SIZE, TEST_RADIUS, 0));
    CHECK(moonlit_art_none_exists(live_none));
    CHECK(!moonlit_art_none_exists(orphan_pfraw));
    CHECK(!moonlit_art_none_exists(orphan_none));
    /* dot-files (the D-055 gc flag) are never touched */
    CHECK(moonlit_art_none_exists(flag));
    /* nothing left to sweep; missing directory is a no-op */
    CHECK(moonlit_art_sweep(dir, ".none", keep_live_stem, NULL) == 0);
    CHECK(moonlit_art_sweep("build/no-such-dir", ".none", keep_live_stem, NULL) == 0);

    remove(live_pfraw);
    remove(live_none);
    remove(flag);
    rmdir(dir);
}

int main(void)
{
    test_round_trip();
    test_header_rejects_mismatch();
    test_mask_corners_leaves_bg_at_corners();
    test_count_uncached();
    test_none_path();
    test_none_marker_round_trip();
    test_count_uncached_treats_none_as_cached();
    test_sweep_removes_orphan_none_and_pfraw();

    printf("test_art: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
