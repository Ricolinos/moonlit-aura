/* moonlit: derived from aura_wheel.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-019, D-041). Sin test host
 * previo en Aura-Firmware para este modulo -- test nuevo, mismo criterio
 * de propiedades estructurales que test_flow.c (limites, monotonia),
 * no una tabla de valores exactos. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../moonlit_wheel.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_step_bounds(void)
{
    CHECK(moonlit_wheel_step(-100) == 1);
    CHECK(moonlit_wheel_step(0) == 1);
    CHECK(moonlit_wheel_step(1) == 1);
    CHECK(moonlit_wheel_step(MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S) == 3);
    CHECK(moonlit_wheel_step(MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S * 10) == 3);
}

static void test_step_is_monotonic_and_capped(void)
{
    int prev = moonlit_wheel_step(0);
    int v;

    for (v = 0; v <= MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S; v += 10)
    {
        int step = moonlit_wheel_step(v);
        CHECK(step >= 1 && step <= 3);
        CHECK(step >= prev); /* nunca disminuye al acelerar */
        prev = step;
    }
}

static void test_hop_letters_threshold(void)
{
    CHECK(!moonlit_wheel_should_hop_letters(0));
    CHECK(!moonlit_wheel_should_hop_letters(MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S));
    CHECK(moonlit_wheel_should_hop_letters(MOONLIT_WHEEL_LETTER_HOP_THRESHOLD_DEG_S + 1));
}

int main(void)
{
    test_step_bounds();
    test_step_is_monotonic_and_capped();
    test_hop_letters_threshold();

    printf("test_wheel: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    printf("test_wheel: 3 passed\n");
    return 0;
}
