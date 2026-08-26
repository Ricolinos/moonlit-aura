/* moonlit: derived from aura_art.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-020, D-042). Ejecutar con
 * `make -C apps/metro/test` -- ../moonlit_art.c se compila y enlaza
 * standalone gracias a los sustitutos de host file.h/lcd.h en este
 * mismo directorio (D-042). */
#include <stdio.h>
#include <string.h>
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

int main(void)
{
    test_round_trip();
    test_header_rejects_mismatch();
    test_mask_corners_leaves_bg_at_corners();

    printf("test_art: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
