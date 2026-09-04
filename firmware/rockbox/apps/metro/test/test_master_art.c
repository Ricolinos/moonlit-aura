/* D-059: host tests for the shared master art cache (moonlit_master_art.c)
 * -- names, header, negative marker, and the two integer resamplers.
 * Run with `make -C apps/metro/test test`; ../moonlit_master_art.c
 * links standalone against the host stand-ins file.h/dir.h/lcd.h here. */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "../moonlit_master_art.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define TEST_ART  "build/test_master_a-deadbeef.42.art"
#define TEST_NONE "build/test_master_a-deadbeef.42.none"

static unsigned r_of(fb_data px) { return ((px >> 8) & 0xf8) | ((px >> 13) & 0x07); }
static unsigned g_of(fb_data px) { return ((px >> 3) & 0xfc) | ((px >>  9) & 0x03); }
static unsigned b_of(fb_data px) { return ((px << 3) & 0xf8) | ((px >>  2) & 0x07); }
static fb_data rgb(unsigned r, unsigned g, unsigned b)
{
    return (fb_data)(((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3));
}

static void test_file_key(void)
{
    char key[MOONLIT_MASTER_ART_KEY_LEN];

    moonlit_master_art_file_key('r', 0x031b464bu, 1787718316L, key, sizeof(key));
    CHECK(strcmp(key, "r-031b464b.1787718316") == 0);
    moonlit_master_art_file_key('p', 0xffffffffu, 0L, key, sizeof(key));
    CHECK(strcmp(key, "p-ffffffff.0") == 0);
    /* same shape as metro_music_album_art_key() ("a-%08lx.%ld") */
    moonlit_master_art_file_key('a', 0x1u, 7L, key, sizeof(key));
    CHECK(strcmp(key, "a-00000001.7") == 0);
}

static void test_none_path(void)
{
    char out[MOONLIT_MASTER_ART_PATH_MAX];

    CHECK(moonlit_master_art_none_path("/.aura/art/albums/a-031b464b.1787718316.art",
                                       out, sizeof(out)));
    CHECK(strcmp(out, "/.aura/art/albums/a-031b464b.1787718316.none") == 0);
    CHECK(moonlit_master_art_none_path("p-1.2.art", out, sizeof(out)));
    CHECK(strcmp(out, "p-1.2.none") == 0);
    CHECK(!moonlit_master_art_none_path("p-1.2.none", out, sizeof(out)));
    CHECK(out[0] == '\0');
    CHECK(!moonlit_master_art_none_path(".art", out, sizeof(out)));
    CHECK(!moonlit_master_art_none_path("", out, sizeof(out)));
    CHECK(!moonlit_master_art_none_path("build/a-deadbeef.1.art", out, 8));
}

static void test_round_trip_and_header(void)
{
    enum { N = 6 };
    fb_data src[N * N], dst[N * N];
    unsigned char raw[MOONLIT_MASTER_ART_HEADER_SIZE + 4];
    FILE *f;
    int i;

    for (i = 0; i < N * N; i++)
        src[i] = (fb_data)(0x1000 + i);

    remove(TEST_ART);
    remove(TEST_NONE);
    CHECK(!moonlit_master_art_exists(TEST_ART, N));
    CHECK(!moonlit_master_art_is_resolved(TEST_ART, N));

    moonlit_master_art_write(TEST_ART, N, src);
    CHECK(access(TEST_ART ".tmp", F_OK) != 0); /* renamed into place */
    CHECK(moonlit_master_art_exists(TEST_ART, N));
    CHECK(!moonlit_master_art_exists(TEST_ART, N + 1));
    CHECK(moonlit_master_art_is_resolved(TEST_ART, N));
    CHECK(moonlit_master_art_read(TEST_ART, N, dst));
    CHECK(memcmp(src, dst, sizeof(src)) == 0);
    CHECK(!moonlit_master_art_read(TEST_ART, N + 1, dst));

    /* Contract v16 header, byte for byte: 'MAST' LE, w, h, flags, reserved. */
    f = fopen(TEST_ART, "rb");
    CHECK(f != NULL);
    if (f)
    {
        CHECK(fread(raw, 1, sizeof(raw), f) == sizeof(raw));
        fclose(f);
        CHECK(raw[0] == 'M' && raw[1] == 'A' && raw[2] == 'S' && raw[3] == 'T');
        CHECK(raw[4] == N && raw[5] == 0 && raw[6] == N && raw[7] == 0);
        CHECK(memcmp(raw + 8, "\0\0\0\0\0\0\0\0", 8) == 0);
        /* first pixel, RGB565 LE */
        CHECK(raw[16] == (0x1000 & 0xff) && raw[17] == (0x1000 >> 8));
    }

    /* A truncated file is not a hit. */
    f = fopen(TEST_ART, "wb");
    if (f)
    {
        unsigned char h[MOONLIT_MASTER_ART_HEADER_SIZE];
        memcpy(h, "MAST", 4);
        h[4] = N; h[5] = 0; h[6] = N; h[7] = 0;
        memset(h + 8, 0, 8);
        fwrite(h, 1, sizeof(h), f);
        fwrite(src, 1, 4, f);
        fclose(f);
    }
    CHECK(moonlit_master_art_exists(TEST_ART, N));
    CHECK(!moonlit_master_art_read(TEST_ART, N, dst));
    remove(TEST_ART);
}

static void test_none_marker(void)
{
    remove(TEST_ART);
    remove(TEST_NONE);
    CHECK(!moonlit_master_art_none_exists(TEST_NONE));
    moonlit_master_art_write_none(TEST_NONE);
    CHECK(moonlit_master_art_none_exists(TEST_NONE));
    /* resolved for every size: "no art" does not depend on it */
    CHECK(moonlit_master_art_is_resolved(TEST_ART, 130));
    CHECK(moonlit_master_art_is_resolved(TEST_ART, 80));
    CHECK(!moonlit_master_art_exists(TEST_ART, 130));
    remove(TEST_NONE);
}

static void test_ensure_dir(void)
{
    const char *deep = "build/test_master_dir/a/b";
    struct stat st;

    moonlit_master_art_ensure_dir(deep);
    CHECK(stat(deep, &st) == 0 && S_ISDIR(st.st_mode));
    /* idempotent */
    moonlit_master_art_ensure_dir(deep);
    CHECK(stat(deep, &st) == 0);
    rmdir("build/test_master_dir/a/b");
    rmdir("build/test_master_dir/a");
    rmdir("build/test_master_dir");
}

static void test_box_downscale_integral(void)
{
    /* 4x4 -> 2x2: each output = average of a 2x2 block. */
    fb_data src[16], dst[4];
    int i;

    for (i = 0; i < 16; i++)
        src[i] = rgb(0, 0, 0);
    /* top-left block all white, rest black */
    src[0] = src[1] = src[4] = src[5] = rgb(255, 255, 255);
    /* bottom-right block: two white, two black -> mid grey */
    src[10] = src[15] = rgb(255, 255, 255);

    moonlit_master_art_box_downscale(src, 4, dst, 2);
    CHECK(dst[0] == rgb(255, 255, 255));
    CHECK(dst[1] == rgb(0, 0, 0));
    CHECK(dst[2] == rgb(0, 0, 0));
    CHECK(r_of(dst[3]) >= 124 && r_of(dst[3]) <= 132);
    CHECK(g_of(dst[3]) >= 124 && g_of(dst[3]) <= 132);
    CHECK(b_of(dst[3]) >= 124 && b_of(dst[3]) <= 132);

    /* same size == plain copy */
    {
        fb_data same[16];
        moonlit_master_art_box_downscale(src, 4, same, 4);
        CHECK(memcmp(same, src, sizeof(src)) == 0);
    }
}

static void test_cover_crop_landscape(void)
{
    /* 8x4 source: left half red, right half blue, with a 2-wide green
     * stripe in the centre columns 3..4. Cover-crop to 4x4 keeps the
     * centre 4 columns of the (unscaled: short side already 4) source:
     * columns 2..5 -> red, green, green, blue. */
    fb_data src[8 * 4], dst[4 * 4];
    int x, y;

    for (y = 0; y < 4; y++)
        for (x = 0; x < 8; x++)
            src[y * 8 + x] = (x < 4) ? rgb(255, 0, 0) : rgb(0, 0, 255);
    for (y = 0; y < 4; y++)
        src[y * 8 + 3] = src[y * 8 + 4] = rgb(0, 255, 0);

    moonlit_master_art_resample_cover(src, 8, 4, dst, 4);
    for (y = 0; y < 4; y++)
    {
        CHECK(dst[y * 4 + 0] == rgb(255, 0, 0));
        CHECK(dst[y * 4 + 1] == rgb(0, 255, 0));
        CHECK(dst[y * 4 + 2] == rgb(0, 255, 0));
        CHECK(dst[y * 4 + 3] == rgb(0, 0, 255));
    }
}

static void test_cover_crop_portrait_downscale(void)
{
    /* 4x8 source, top half white, bottom half black; cover to 2x2:
     * short side 4 -> 2 (scale 1/2), virtual height 4, crop rows 1..2
     * of the virtual image == source rows 2..5 -> white, black. */
    fb_data src[4 * 8], dst[2 * 2];
    int i;

    for (i = 0; i < 32; i++)
        src[i] = (i < 16) ? rgb(255, 255, 255) : rgb(0, 0, 0);

    moonlit_master_art_resample_cover(src, 4, 8, dst, 2);
    CHECK(dst[0] == rgb(255, 255, 255));
    CHECK(dst[1] == rgb(255, 255, 255));
    CHECK(dst[2] == rgb(0, 0, 0));
    CHECK(dst[3] == rgb(0, 0, 0));
}

static void test_real_sizes_preserve_flat_and_gradient(void)
{
    /* The production derivations: 136x136 -> 130 (master), 130 -> 120
     * (Marea), 130 -> 80 (grids), and 136x90 -> 80 (photo, landscape).
     * A flat colour must survive untouched; a horizontal gradient must
     * stay monotonic. Buffers are static: 136*136*2 is too big for a
     * comfortable stack on some hosts' default limits in -O0. */
    static fb_data a[136 * 136], m[130 * 130], d[120 * 120], g[80 * 80];
    int i, x, ok;

    for (i = 0; i < 136 * 136; i++)
        a[i] = rgb(0x40, 0x80, 0xc0);
    moonlit_master_art_resample_cover(a, 136, 136, m, 130);
    ok = 1;
    for (i = 0; i < 130 * 130; i++)
        if (m[i] != rgb(0x40, 0x80, 0xc0)) ok = 0;
    CHECK(ok);
    moonlit_master_art_box_downscale(m, 130, d, 120);
    ok = 1;
    for (i = 0; i < 120 * 120; i++)
        if (d[i] != rgb(0x40, 0x80, 0xc0)) ok = 0;
    CHECK(ok);
    moonlit_master_art_box_downscale(m, 130, g, 80);
    ok = 1;
    for (i = 0; i < 80 * 80; i++)
        if (g[i] != rgb(0x40, 0x80, 0xc0)) ok = 0;
    CHECK(ok);

    /* gradient on 130 -> 120 and -> 80: red rises left to right */
    for (i = 0; i < 130 * 130; i++)
        m[i] = rgb((i % 130) * 255 / 129, 0, 0);
    moonlit_master_art_box_downscale(m, 130, d, 120);
    ok = 1;
    for (x = 1; x < 120; x++)
        if (r_of(d[x]) < r_of(d[x - 1])) ok = 0;
    CHECK(ok);
    CHECK(r_of(d[0]) < 16 && r_of(d[119]) > 239);
    moonlit_master_art_box_downscale(m, 130, g, 80);
    ok = 1;
    for (x = 1; x < 80; x++)
        if (r_of(g[x]) < r_of(g[x - 1])) ok = 0;
    CHECK(ok);
    /* every output row identical (vertical uniformity preserved) */
    CHECK(memcmp(g, g + 80 * 79, 80 * sizeof(fb_data)) == 0);

    /* landscape 136x90 photo -> 80x80: flat stays flat */
    for (i = 0; i < 136 * 90; i++)
        a[i] = rgb(0xf8, 0xfc, 0xf8);
    moonlit_master_art_resample_cover(a, 136, 90, g, 80);
    ok = 1;
    for (i = 0; i < 80 * 80; i++)
        if (g[i] != rgb(0xf8, 0xfc, 0xf8)) ok = 0;
    CHECK(ok);
}

/* moonlit (D-063, contrato v18): el sello de version del arbol de
 * caratulas derivadas. Lo que importa es que un archivo AUSENTE, VACIO
 * o con basura cuente como "version 0" -- ahi es donde se decide si hay
 * purga, y un falso "ya estas al dia" dejaria tiles rotos para
 * siempre. */
static void test_format_version(void)
{
    const char *path = "build/format.txt";
    int fd;

    remove(path);
    CHECK(moonlit_master_art_format_read(path) == 0); /* ausente */

    CHECK(moonlit_master_art_format_write(path, MOONLIT_MASTER_ART_FORMAT_VERSION));
    CHECK(moonlit_master_art_format_read(path) == MOONLIT_MASTER_ART_FORMAT_VERSION);

    CHECK(moonlit_master_art_format_write(path, 1));
    CHECK(moonlit_master_art_format_read(path) == 1);
    CHECK(moonlit_master_art_format_read(path) < MOONLIT_MASTER_ART_FORMAT_VERSION);

    /* Vacio y basura: version 0, es decir, purga. */
    fd = creat(path, 0666);
    CHECK(fd >= 0);
    close(fd);
    CHECK(moonlit_master_art_format_read(path) == 0);

    fd = creat(path, 0666);
    CHECK(fd >= 0);
    CHECK(write(fd, "no-soy-un-numero\n", 17) == 17);
    close(fd);
    CHECK(moonlit_master_art_format_read(path) == 0);

    /* Un numero con cola (salto de linea, espacios) si cuenta. */
    fd = creat(path, 0666);
    CHECK(fd >= 0);
    CHECK(write(fd, "2\n", 2) == 2);
    close(fd);
    CHECK(moonlit_master_art_format_read(path) == 2);

    remove(path);
}

int main(void)
{
    mkdir("build", 0777);
    test_file_key();
    test_none_path();
    test_round_trip_and_header();
    test_none_marker();
    test_ensure_dir();
    test_box_downscale_integral();
    test_cover_crop_landscape();
    test_cover_crop_portrait_downscale();
    test_real_sizes_preserve_flat_and_gradient();
    test_format_version();

    printf("test_master_art: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
