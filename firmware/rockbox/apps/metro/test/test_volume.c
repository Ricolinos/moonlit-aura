/* Tests host-side de metro_volume.c (R5-F3, M-083): el mapeo entre los
 * 16 niveles que muestra Metro y los dB de Rockbox. Sin Rockbox. */
#include <stdio.h>
#include "../metro_volume.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

/* Rango real del iPod 6G (cs42l55: -60..+12 dB). */
#define IPOD_MIN -60
#define IPOD_MAX  12

static void test_endpoints(void)
{
    CHECK(metro_volume_db_from_level(0, IPOD_MIN, IPOD_MAX) == IPOD_MIN);
    CHECK(metro_volume_db_from_level(15, IPOD_MIN, IPOD_MAX) == IPOD_MAX);
    CHECK(metro_volume_level_from_db(IPOD_MIN, IPOD_MIN, IPOD_MAX) == 0);
    CHECK(metro_volume_level_from_db(IPOD_MAX, IPOD_MIN, IPOD_MAX) == 15);
}

static void test_clamps(void)
{
    CHECK(metro_volume_db_from_level(-3, IPOD_MIN, IPOD_MAX) == IPOD_MIN);
    CHECK(metro_volume_db_from_level(99, IPOD_MIN, IPOD_MAX) == IPOD_MAX);
    CHECK(metro_volume_level_from_db(-200, IPOD_MIN, IPOD_MAX) == 0);
    CHECK(metro_volume_level_from_db(200, IPOD_MIN, IPOD_MAX) == 15);
    /* Rango degenerado: no divide por cero, todo cae a 0/min. */
    CHECK(metro_volume_db_from_level(7, 5, 5) == 5);
    CHECK(metro_volume_level_from_db(5, 5, 5) == 0);
}

/* La garantía que hace posible no guardar estado propio: cada nivel
 * vuelve a sí mismo tras pasar por dB, en varios rangos posibles. */
static void test_round_trip(void)
{
    int ranges[][2] = { { IPOD_MIN, IPOD_MAX }, { -90, 0 }, { -74, 6 }, { 0, 15 }, { 0, 100 }, { -30, 30 } };
    unsigned r;
    int level;

    for (r = 0; r < sizeof(ranges) / sizeof(ranges[0]); r++)
    {
        int lo = ranges[r][0], hi = ranges[r][1];
        int prev_db = -100000;
        for (level = 0; level < METRO_VOLUME_LEVELS; level++)
        {
            int db = metro_volume_db_from_level(level, lo, hi);
            CHECK(metro_volume_level_from_db(db, lo, hi) == level);
            /* Estrictamente creciente: dos niveles nunca comparten dB
             * (si lo hicieran, un paso de rueda no cambiaría nada). */
            CHECK(db > prev_db);
            prev_db = db;
        }
    }
}

/* Un dB cualquiera cae al nivel más cercano, nunca a uno lejano: el
 * ejemplo concreto del iPod, default de Rockbox -25 dB. */
static void test_nearest(void)
{
    int level = metro_volume_level_from_db(-25, IPOD_MIN, IPOD_MAX);
    int db = metro_volume_db_from_level(level, IPOD_MIN, IPOD_MAX);
    int diff = db - (-25);
    if (diff < 0) diff = -diff;
    /* paso ≈ 4.8 dB, así que el más cercano está a lo sumo a 2.4 -> 3 */
    CHECK(diff <= 3);
    CHECK(level == 7);
}

int main(void)
{
    test_endpoints();
    test_clamps();
    test_round_trip();
    test_nearest();
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
