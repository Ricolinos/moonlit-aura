/* Tests host-side del predicado de archivos ocultos de metro_fsutil.h
 * (R4/FA-2, M-075). Solo incluye el header -- no hay .c que compilar,
 * el predicado es `static inline` y sin dependencias de Rockbox.
 * Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../metro_fsutil.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

/* El caso que motivó el filtro: AppleDouble CONSERVA la extensión, así
 * que el filtro por sufijo (matches_any_ext) lo deja pasar. */
static void test_appledouble(void)
{
    CHECK(metro_fsutil_is_hidden_name("._IMG_1234.jpg"));
    CHECK(metro_fsutil_is_hidden_name("._video.mpg"));
    CHECK(metro_fsutil_is_hidden_name("._Mi Lista.m3u8"));
    CHECK(metro_fsutil_is_hidden_name("._rockbox.ipod"));
    CHECK(metro_fsutil_is_hidden_name("._sync-pending.json"));
}

/* Residuales de macOS observados en el iPod real. */
static void test_macos_leftovers(void)
{
    CHECK(metro_fsutil_is_hidden_name(".DS_Store"));
    CHECK(metro_fsutil_is_hidden_name(".Spotlight-V100"));
    CHECK(metro_fsutil_is_hidden_name(".Trashes"));
    CHECK(metro_fsutil_is_hidden_name(".fseventsd"));
    CHECK(metro_fsutil_is_hidden_name(".TemporaryItems"));
}

/* `.` y `..` caen bajo la misma regla, sin necesitar un caso aparte. */
static void test_dotdirs(void)
{
    CHECK(metro_fsutil_is_hidden_name("."));
    CHECK(metro_fsutil_is_hidden_name(".."));
}

/* Lo que NO debe filtrarse -- la guarda de regresión que importa: un
 * punto en cualquier posición que no sea la primera es contenido
 * legítimo y frecuente. */
static void test_keeps_real_content(void)
{
    CHECK(!metro_fsutil_is_hidden_name("IMG_1234.jpg"));
    CHECK(!metro_fsutil_is_hidden_name("mi.foto.jpg"));
    CHECK(!metro_fsutil_is_hidden_name("S01E03.mpg"));
    CHECK(!metro_fsutil_is_hidden_name("track 01 - intro.mp3"));
    CHECK(!metro_fsutil_is_hidden_name("Mi Lista.m3u8"));
    /* Un nombre que EMPIEZA con guion bajo, no con punto, se conserva. */
    CHECK(!metro_fsutil_is_hidden_name("_borrador.jpg"));
}

/* Defensivo: nunca debe desreferenciar NULL ni leer fuera de una
 * cadena vacía. */
static void test_degenerate(void)
{
    CHECK(metro_fsutil_is_hidden_name(NULL));
    CHECK(!metro_fsutil_is_hidden_name(""));
}

int main(void)
{
    test_appledouble();
    test_macos_leftovers();
    test_dotdirs();
    test_keeps_real_content();
    test_degenerate();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
