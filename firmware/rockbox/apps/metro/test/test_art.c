/* moonlit: derived from aura_art.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-020, D-042; D-059 retiro el
 * formato .pfraw -- ver test_master_art.c). Ejecutar con
 * `make -C apps/metro/test` -- ../moonlit_art.c se compila y enlaza
 * standalone gracias a los sustitutos de host file.h/lcd.h en este
 * mismo directorio (D-042). */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> /* mkdir() -- D-056 sweep test */
#include <unistd.h>   /* rmdir() */
#include "../moonlit_art.h"
#include <fcntl.h>

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

static bool keep_live_stem(const char *stem, void *ctx)
{
    (void)ctx;
    return strcmp(stem, "a-11111111.1") == 0;
}

static void touch(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0)
        close(fd);
}

static bool exists(const char *path)
{
    return access(path, F_OK) == 0;
}

/* D-056/D-059: the same sweep the builder runs over /.aura/art/<src>
 * (.art/.none) and /.aura/thumbs/<src> (.mth). */
static void test_sweep_removes_orphans(void)
{
    const char *dir = "build/test_art_gc";
    const char *live_art = "build/test_art_gc/a-11111111.1.art";
    const char *live_none = "build/test_art_gc/a-11111111.1.none";
    const char *orphan_art = "build/test_art_gc/a-22222222.1.art";
    const char *orphan_none = "build/test_art_gc/a-33333333.1.none";
    const char *other_suffix = "build/test_art_gc/a-44444444.1.mth";
    const char *flag = "build/test_art_gc/.gc-pending";

    mkdir(dir, 0777);
    touch(live_art);
    touch(orphan_art);
    touch(live_none);
    touch(orphan_none);
    touch(other_suffix);
    touch(flag);

    CHECK(moonlit_art_sweep(dir, ".art", keep_live_stem, NULL) == 1);
    CHECK(moonlit_art_sweep(dir, ".none", keep_live_stem, NULL) == 1);

    CHECK(exists(live_art));
    CHECK(exists(live_none));
    CHECK(!exists(orphan_art));
    CHECK(!exists(orphan_none));
    /* other suffixes are left alone until asked for */
    CHECK(exists(other_suffix));
    CHECK(moonlit_art_sweep(dir, ".mth", keep_live_stem, NULL) == 1);
    CHECK(!exists(other_suffix));
    /* dot-files (the D-055 gc flag) are never touched */
    CHECK(exists(flag));
    /* nothing left to sweep; missing directory is a no-op */
    CHECK(moonlit_art_sweep(dir, ".none", keep_live_stem, NULL) == 0);
    CHECK(moonlit_art_sweep("build/no-such-dir", ".none", keep_live_stem, NULL) == 0);

    remove(live_art);
    remove(live_none);
    remove(flag);
    rmdir(dir);
}

int main(void)
{
    mkdir("build", 0777);
    test_mask_corners_leaves_bg_at_corners();
    test_sweep_removes_orphans();

    printf("test_art: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
