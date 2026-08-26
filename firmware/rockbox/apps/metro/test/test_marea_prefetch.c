/* D-057: test host de moonlit_marea_prefetch_order() -- logica pura de
 * orden/prioridad, sin disco ni tagcache. Propiedades estructurales
 * (limites, sin repetidos, orden de prioridad), no una tabla exhaustiva.
 * Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../moonlit_marea_prefetch.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static int has_dup(const int *out, int n)
{
    int i, j;
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
            if (out[i] == out[j])
                return 1;
    return 0;
}

static int index_of(const int *out, int n, int val)
{
    int i;
    for (i = 0; i < n; i++)
        if (out[i] == val)
            return i;
    return -1;
}

/* D-057: target primero, luego alternando hacia `dir`/opuesto, cada
 * lado respetando su propio radio -- caso central de la funcion
 * (target=50, biblioteca grande, sin recortes de borde). */
static void test_order_and_priority(void)
{
    int out[32];
    int n = moonlit_marea_prefetch_order(50, 1000, 1, 10, 4, out, 32);

    CHECK(n == 15); /* 1 (centro) + 10 (adelante) + 4 (atras) */
    CHECK(!has_dup(out, n));
    CHECK(out[0] == 50); /* el propio destino siempre primero */

    /* la ventana +-2 (visible en moonlit_screen_marea.c) queda cubierta
     * antes que cualquier indice mas alla de +-4 (el lado corto). */
    CHECK(index_of(out, n, 51) < index_of(out, n, 55));
    CHECK(index_of(out, n, 49) < index_of(out, n, 46));

    /* el lado "hacia adelante" (dir=+1) llega mas lejos que el opuesto. */
    CHECK(index_of(out, n, 60) >= 0);  /* target+10 */
    CHECK(index_of(out, n, 61) < 0);   /* target+11 -- fuera del radio */
    CHECK(index_of(out, n, 46) >= 0);  /* target-4 */
    CHECK(index_of(out, n, 45) < 0);   /* target-5 -- fuera del radio */
}

static void test_dir_negative(void)
{
    int out[32];
    int n = moonlit_marea_prefetch_order(50, 1000, -1, 10, 4, out, 32);

    CHECK(n == 15);
    CHECK(index_of(out, n, 40) >= 0);  /* target-10, "adelante" cuando dir=-1 */
    CHECK(index_of(out, n, 39) < 0);
    CHECK(index_of(out, n, 54) >= 0);  /* target+4, lado corto */
    CHECK(index_of(out, n, 55) < 0);
}

static void test_dir_zero_defaults_forward(void)
{
    int out_zero[32], out_pos[32];
    int n0 = moonlit_marea_prefetch_order(20, 1000, 0, 6, 3, out_zero, 32);
    int n1 = moonlit_marea_prefetch_order(20, 1000, 1, 6, 3, out_pos, 32);
    int i;

    CHECK(n0 == n1);
    for (i = 0; i < n0 && i < n1; i++)
        CHECK(out_zero[i] == out_pos[i]);
}

/* Biblioteca chica: los indices fuera de [0, album_count) se omiten sin
 * dejar huecos ni repetir otro indice en su lugar. */
static void test_clips_to_library_bounds(void)
{
    int out[32];
    int n = moonlit_marea_prefetch_order(1, 5, 1, 10, 4, out, 32);
    int i;

    CHECK(!has_dup(out, n));
    for (i = 0; i < n; i++)
        CHECK(out[i] >= 0 && out[i] < 5);
    /* target=1 en una biblioteca de 5 (indices 0..4): solo caben 0..4,
     * el radio de 10/4 se recorta contra los bordes de verdad. */
    CHECK(n == 5);
}

static void test_out_cap_never_overflows(void)
{
    int out[3];
    int n = moonlit_marea_prefetch_order(50, 1000, 1, 10, 4, out, 3);

    CHECK(n == 3);
    CHECK(out[0] == 50);
}

static void test_empty_library(void)
{
    int out[8];
    CHECK(moonlit_marea_prefetch_order(0, 0, 1, 10, 4, out, 8) == 0);
}

int main(void)
{
    test_order_and_priority();
    test_dir_negative();
    test_dir_zero_defaults_forward();
    test_clips_to_library_bounds();
    test_out_cap_never_overflows();
    test_empty_library();

    printf("test_marea_prefetch: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
