/* Tests host-side de metro_firmware_families.c: sin dependencias de
 * Rockbox, compila y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_firmware_families.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

int main(void)
{
    int i, j, n = metro_fw_sibling_count();

    /* three families, two siblings (D-047) */
    CHECK(n == 2);

    for (i = 0; i < n; i++)
    {
        const struct metro_fw_family *f = metro_fw_sibling(i);

        CHECK(f != NULL);
        CHECK(f->dormant_dir != NULL);
        /* never ourselves: the own tree is not a row */
        CHECK(strcmp(f->dormant_dir, METRO_FW_OWN_DORMANT) != 0);
        /* contract v10 naming */
        CHECK(strncmp(f->dormant_dir, "/.firmware-", 11) == 0);
        CHECK(strlen(f->dormant_dir) > 11);
        /* distinct dirs and names between siblings */
        for (j = 0; j < i; j++)
        {
            const struct metro_fw_family *g = metro_fw_sibling(j);
            CHECK(strcmp(f->dormant_dir, g->dormant_dir) != 0);
            CHECK(f->name != g->name);
        }
    }

    /* out of range */
    CHECK(metro_fw_sibling(n) == NULL);
    CHECK(metro_fw_sibling(2) == NULL);
    CHECK(metro_fw_sibling(-1) == NULL);

    /* the own dormant tree follows the same naming rule */
    CHECK(strncmp(METRO_FW_OWN_DORMANT, "/.firmware-", 11) == 0);

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
