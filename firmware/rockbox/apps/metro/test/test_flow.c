/* moonlit: derived from test_flow.c @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104
 * (ver MODIFICATIONS.md, DECISIONS.md D-041). Adaptado al eje vertical
 * de moonlit_flow (screen_y/AXIS_LEN en vez de screen_x/SCREEN_W, D-030).
 *
 * Tests host-side del nucleo matematico de Cover Flow (moonlit_flow.c).
 * Sin dependencias de Rockbox: compila y corre nativo en el Mac.
 * Ejecutar con `make -C apps/metro/test`.
 *
 * No hay una implementacion de referencia para comparar pixel a pixel
 * (el original vive dentro de un plugin que no corre en el host) -- los
 * tests verifican propiedades estructurales de la formula portada
 * (limites, terminacion, valores trigonometricos conocidos), no una
 * salida exacta contra pictureflow.c. */
#include <stdio.h>
#include "../moonlit_flow.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define ABS(x) ((x) < 0 ? -(x) : (x))

static void test_fixed_point_basics(void)
{
    CHECK(moonlit_flow_fmul(MOONLIT_FLOW_ONE, MOONLIT_FLOW_ONE) == MOONLIT_FLOW_ONE);
    CHECK(moonlit_flow_fmul(MOONLIT_FLOW_ONE, 0) == 0);
    CHECK(moonlit_flow_fmul(MOONLIT_FLOW_ONE * 2, MOONLIT_FLOW_HALF) == MOONLIT_FLOW_ONE);

    CHECK(moonlit_flow_fdiv(MOONLIT_FLOW_ONE, MOONLIT_FLOW_ONE) == MOONLIT_FLOW_ONE);
    CHECK(moonlit_flow_fdiv(0, MOONLIT_FLOW_ONE) == 0);

    /* fmul(fdiv(a,b), b) ~= a, con tolerancia de redondeo de punto fijo. */
    {
        int a = MOONLIT_FLOW_ONE * 7;
        int b = MOONLIT_FLOW_ONE * 3;
        int roundtrip = moonlit_flow_fmul(moonlit_flow_fdiv(a, b), b);
        CHECK(ABS(roundtrip - a) < MOONLIT_FLOW_ONE / 32);
    }
}

static void test_trig_known_values(void)
{
    const int TOL = 3; /* cuantizacion de la tabla de 33 muestras */

    CHECK(ABS(moonlit_flow_fsin(0) - 0) <= TOL);
    CHECK(ABS(moonlit_flow_fsin(256) - MOONLIT_FLOW_ONE) <= TOL);   /* 90 grados */
    CHECK(ABS(moonlit_flow_fsin(512) - 0) <= TOL);                /* 180 grados */
    CHECK(ABS(moonlit_flow_fsin(768) - (-MOONLIT_FLOW_ONE)) <= TOL); /* 270 grados */

    CHECK(ABS(moonlit_flow_fcos(0) - MOONLIT_FLOW_ONE) <= TOL);
    CHECK(ABS(moonlit_flow_fcos(256) - 0) <= TOL);
    CHECK(ABS(moonlit_flow_fcos(512) - (-MOONLIT_FLOW_ONE)) <= TOL);

    /* Identidad pitagorica aproximada en varios angulos (sin^2+cos^2 ~= 1). */
    {
        int angles[] = { 0, 100, 256, 400, 512, 700, 768, 900 };
        size_t i;
        for (i = 0; i < sizeof(angles) / sizeof(angles[0]); i++)
        {
            int s = moonlit_flow_fsin(angles[i]);
            int c = moonlit_flow_fcos(angles[i]);
            int sum = moonlit_flow_fmul(s, s) + moonlit_flow_fmul(c, c);
            CHECK(ABS(sum - MOONLIT_FLOW_ONE) < MOONLIT_FLOW_ONE / 16);
        }
    }
}

static void test_flat_centered_slide_is_visible_and_terminates(void)
{
    moonlit_flow_slide_t slide = { 0, 0, 0 };
    moonlit_flow_projection_t proj;
    int width = 100;
    int steps = 0;
    int last_row = -1;
    int monotonic = 1;

    moonlit_flow_begin_projection(&proj, &slide, width);
    CHECK(proj.screen_y < MOONLIT_FLOW_AXIS_LEN);

    do
    {
        int row = moonlit_flow_source_row(&proj);
        CHECK(row >= 0 && row < width);
        if (last_row >= 0 && row < last_row)
            monotonic = 0;
        last_row = row;
        steps++;
    }
    while (moonlit_flow_advance_column(&proj) && steps < MOONLIT_FLOW_AXIS_LEN + 10);

    CHECK(steps > 0);
    CHECK(steps <= MOONLIT_FLOW_AXIS_LEN); /* termina, no se cuelga */
    CHECK(monotonic); /* de frente y sin rotar, la fuente avanza siempre hacia adelante */
}

static void test_offscreen_slide_is_not_visible(void)
{
    /* "Muy abajo" pero dentro de un rango realista para punto fijo de
     * 32 bits -- CAM_DIST*cx no debe desbordar (cx en unidades de
     * pantalla, nunca cerca de INT_MAX/CAM_DIST en un flujo real). */
    moonlit_flow_slide_t slide = { 0, 0, MOONLIT_FLOW_ONE * 1000 };
    moonlit_flow_projection_t proj;

    moonlit_flow_begin_projection(&proj, &slide, 100);
    CHECK(proj.screen_y >= MOONLIT_FLOW_AXIS_LEN);
}

static void test_rotated_slide_terminates(void)
{
    /* Un slide lateral (angulo ~45 grados, IANGLE_MAX/8=128) tambien
     * debe terminar en un numero acotado de filas -- es el caso que
     * usa la recurrencia de Mobius (has_rotation), no el paso fijo.
     * `distance` es un entero simple (NO preescalado por ONE, ver
     * moonlit_flow.h) -- 5 es un valor de prueba chico, no un numero
     * real de ningun layout. */
    moonlit_flow_slide_t slide = { 128, 5, MOONLIT_FLOW_ONE * 40 };
    moonlit_flow_projection_t proj;
    int width = 80;
    int steps = 0;

    moonlit_flow_begin_projection(&proj, &slide, width);
    if (proj.screen_y < MOONLIT_FLOW_AXIS_LEN)
    {
        do
        {
            int row = moonlit_flow_source_row(&proj);
            CHECK(row >= 0 && row < width);
            steps++;
        }
        while (moonlit_flow_advance_column(&proj) && steps < MOONLIT_FLOW_AXIS_LEN + 10);

        CHECK(steps > 0);
        CHECK(steps <= MOONLIT_FLOW_AXIS_LEN);
    }
}

static void test_vertical_scale_is_positive(void)
{
    moonlit_flow_slide_t slide = { 0, 0, 0 };
    moonlit_flow_projection_t proj;

    moonlit_flow_begin_projection(&proj, &slide, 100);
    CHECK(proj.screen_y < MOONLIT_FLOW_AXIS_LEN);
    CHECK(moonlit_flow_cross_scale(&proj) > 0);
}

/* Layout de Marea en reposo (D-030: 5 tapas visibles en 220 px utiles
 * -- 1 central + 2 por lado). Parametros de inclinacion/offset del
 * coverflow clasico (pictureflow.c reset_slides()), reescalados del
 * eje de 320 px de aura_flow.c al eje vertical de 220 px de moonlit
 * (MOONLIT_FLOW_AXIS_LEN/AURA_FLOW_SCREEN_W = 220/320 = 0.6875) --
 * sirve de documentacion viva ademas de test: si alguien cambia
 * MOONLIT_FLOW_DISPLAY_LEN o el alto de pantalla, este test es la
 * primera senal de que itilt/offset tambien hay que recalcularlos. */
#define TEST_ITILT    199    /* 70 grados: 70*1024/360, sin cambio por eje */
#define TEST_OFFSET_NEAR  99041  /* tapa adyacente: TEST_OFFSETX_320 * 0.6875 */
#define TEST_OFFSET_FAR  173072  /* tapa extrema: 1.75x la adyacente, doc SS0 */

static void test_realistic_side_slide_layout(void)
{
    moonlit_flow_slide_t center    = { 0, 0, 0 };
    moonlit_flow_slide_t near_l    = { TEST_ITILT, 0, -TEST_OFFSET_NEAR };
    moonlit_flow_slide_t near_r    = { -TEST_ITILT, 0, TEST_OFFSET_NEAR };
    moonlit_flow_slide_t far_l     = { TEST_ITILT, 0, -TEST_OFFSET_FAR };
    moonlit_flow_slide_t far_r     = { -TEST_ITILT, 0, TEST_OFFSET_FAR };
    moonlit_flow_slide_t slides[5];
    moonlit_flow_projection_t proj;
    int width = 56; /* CF_COVER_SIZE de aura_coverflow.c */
    size_t i;

    slides[0] = center;
    slides[1] = near_l;
    slides[2] = near_r;
    slides[3] = far_l;
    slides[4] = far_r;

    moonlit_flow_begin_projection(&proj, &center, width);
    CHECK(proj.screen_y < MOONLIT_FLOW_AXIS_LEN);

    /* Las 5 tapas, inclinadas y corridas, tienen que seguir siendo
     * procesables sin colgarse -- si la compensacion de inclinacion
     * (zo) estuviera mal, esto facilmente diverge o nunca marca
     * "visible". */
    for (i = 0; i < sizeof(slides) / sizeof(slides[0]); i++)
    {
        moonlit_flow_begin_projection(&proj, &slides[i], width);
        if (proj.screen_y < MOONLIT_FLOW_AXIS_LEN)
        {
            int steps = 0;
            do { steps++; } while (moonlit_flow_advance_column(&proj) && steps < MOONLIT_FLOW_AXIS_LEN + 10);
            CHECK(steps <= MOONLIT_FLOW_AXIS_LEN);
        }
    }
}

int main(void)
{
    test_fixed_point_basics();
    test_trig_known_values();
    test_flat_centered_slide_is_visible_and_terminates();
    test_offscreen_slide_is_not_visible();
    test_rotated_slide_terminates();
    test_vertical_scale_is_positive();
    test_realistic_side_slide_layout();

    printf("test_flow: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    printf("test_flow: 7 passed\n");
    return 0;
}
