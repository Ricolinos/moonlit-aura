/* Tests host-side de apps/metro/moonlit_tokens.h (M1, D-027, D-028).
 * Solo incluye el header generado -- sin lcd.h: LCD_RGBPACK cae al
 * fallback del propio header (ver moonlit_tokens.h). Verifica que cada
 * rol MD3 tiene su _RGB24 y que, en cada nivel de elevacion, el borde
 * de luz es mas claro que la base y el de sombra mas oscuro, canal por
 * canal (D-012). Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include "../moonlit_tokens.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CH_R(rgb24) (((rgb24) >> 16) & 0xFF)
#define CH_G(rgb24) (((rgb24) >> 8) & 0xFF)
#define CH_B(rgb24) ((rgb24) & 0xFF)

/* Los 16 roles MD3 (D-028) existen como _RGB24 en ambos esquemas. */
static void test_roles_have_rgb24(void)
{
    unsigned night[] = {
        MOONLIT_NIGHT_PRIMARY_RGB24, MOONLIT_NIGHT_ON_PRIMARY_RGB24,
        MOONLIT_NIGHT_PRIMARY_CONTAINER_RGB24, MOONLIT_NIGHT_ON_PRIMARY_CONTAINER_RGB24,
        MOONLIT_NIGHT_SURFACE_RGB24, MOONLIT_NIGHT_SURFACE_DIM_RGB24,
        MOONLIT_NIGHT_SURFACE_BRIGHT_RGB24,
        MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_RGB24, MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_RGB24,
        MOONLIT_NIGHT_SURFACE_CONTAINER_RGB24, MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_RGB24,
        MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_RGB24,
        MOONLIT_NIGHT_ON_SURFACE_RGB24, MOONLIT_NIGHT_ON_SURFACE_VARIANT_RGB24,
        MOONLIT_NIGHT_OUTLINE_RGB24, MOONLIT_NIGHT_OUTLINE_VARIANT_RGB24,
    };
    unsigned dawn[] = {
        MOONLIT_DAWN_PRIMARY_RGB24, MOONLIT_DAWN_ON_PRIMARY_RGB24,
        MOONLIT_DAWN_PRIMARY_CONTAINER_RGB24, MOONLIT_DAWN_ON_PRIMARY_CONTAINER_RGB24,
        MOONLIT_DAWN_SURFACE_RGB24, MOONLIT_DAWN_SURFACE_DIM_RGB24,
        MOONLIT_DAWN_SURFACE_BRIGHT_RGB24,
        MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_RGB24, MOONLIT_DAWN_SURFACE_CONTAINER_LOW_RGB24,
        MOONLIT_DAWN_SURFACE_CONTAINER_RGB24, MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_RGB24,
        MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_RGB24,
        MOONLIT_DAWN_ON_SURFACE_RGB24, MOONLIT_DAWN_ON_SURFACE_VARIANT_RGB24,
        MOONLIT_DAWN_OUTLINE_RGB24, MOONLIT_DAWN_OUTLINE_VARIANT_RGB24,
    };
    CHECK(sizeof(night) / sizeof(night[0]) == 16);
    CHECK(sizeof(dawn) / sizeof(dawn[0]) == 16);
    for (unsigned i = 0; i < sizeof(night) / sizeof(night[0]); i++)
        CHECK(night[i] <= 0xFFFFFF);
    for (unsigned i = 0; i < sizeof(dawn) / sizeof(dawn[0]) ; i++)
        CHECK(dawn[i] <= 0xFFFFFF);
}

/* light_edge > base > shadow_edge por canal (D-012), en los 5 niveles
 * de superficie, en ambos esquemas. */
#define CHECK_EDGE_ORDER(base, light, shadow) do { \
    CHECK(CH_R(light) >= CH_R(base)); \
    CHECK(CH_G(light) >= CH_G(base)); \
    CHECK(CH_B(light) >= CH_B(base)); \
    CHECK(CH_R(base) >= CH_R(shadow)); \
    CHECK(CH_G(base) >= CH_G(shadow)); \
    CHECK(CH_B(base) >= CH_B(shadow)); \
} while (0)

static void test_elevation_edge_order_night(void)
{
    CHECK_EDGE_ORDER(MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_EDGE_LIGHT_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_LOWEST_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_EDGE_LIGHT_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_LOW_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_NIGHT_SURFACE_CONTAINER_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_EDGE_LIGHT_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_EDGE_LIGHT_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGH_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_EDGE_LIGHT_RGB24,
                      MOONLIT_NIGHT_SURFACE_CONTAINER_HIGHEST_EDGE_SHADOW_RGB24);
}

static void test_elevation_edge_order_dawn(void)
{
    CHECK_EDGE_ORDER(MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_EDGE_LIGHT_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_LOWEST_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_DAWN_SURFACE_CONTAINER_LOW_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_LOW_EDGE_LIGHT_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_LOW_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_DAWN_SURFACE_CONTAINER_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_EDGE_LIGHT_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_EDGE_LIGHT_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_HIGH_EDGE_SHADOW_RGB24);
    CHECK_EDGE_ORDER(MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_EDGE_LIGHT_RGB24,
                      MOONLIT_DAWN_SURFACE_CONTAINER_HIGHEST_EDGE_SHADOW_RGB24);
}

/* Los 4 presets de acento existen en ambos esquemas (sustituyen a
 * enum metro_accent, D-028). */
static void test_presets_exist(void)
{
    unsigned presets[] = {
        MOONLIT_NIGHT_MOONSTONE_PRIMARY_RGB24, MOONLIT_DAWN_MOONSTONE_PRIMARY_RGB24,
        MOONLIT_NIGHT_TIDE_PRIMARY_RGB24, MOONLIT_DAWN_TIDE_PRIMARY_RGB24,
        MOONLIT_NIGHT_EMBER_PRIMARY_RGB24, MOONLIT_DAWN_EMBER_PRIMARY_RGB24,
        MOONLIT_NIGHT_MOSS_PRIMARY_RGB24, MOONLIT_DAWN_MOSS_PRIMARY_RGB24,
    };
    for (unsigned i = 0; i < sizeof(presets) / sizeof(presets[0]); i++)
        CHECK(presets[i] <= 0xFFFFFF);
}

int main(void)
{
    test_roles_have_rgb24();
    test_elevation_edge_order_night();
    test_elevation_edge_order_dawn();
    test_presets_exist();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
