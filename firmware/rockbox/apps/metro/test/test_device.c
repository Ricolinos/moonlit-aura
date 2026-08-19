/* Tests host-side de metro_device_name.c: sin dependencias de Rockbox,
 * compila y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_device_name.h"

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
    char out[METRO_DEVICE_NAME_BUF];

    CHECK(metro_device_name_sanitize("iPod de Ricardo", out, sizeof(out)) == 15);
    CHECK(!strcmp(out, "iPod de Ricardo"));

    /* leading/trailing whitespace, collapsed internal */
    CHECK(metro_device_name_sanitize("   iPod   de  Ana \t ", out, sizeof(out)) == 11);
    CHECK(!strcmp(out, "iPod de Ana"));

    /* control chars dropped */
    metro_device_name_sanitize("iPod\x01 de\r\n Ana", out, sizeof(out));
    CHECK(!strcmp(out, "iPod de Ana"));

    /* UTF-8 (2 bytes) is preserved */
    metro_device_name_sanitize("iPod de \xc3\x91o\xc3\xb1o", out, sizeof(out));
    CHECK(!strcmp(out, "iPod de \xc3\x91o\xc3\xb1o"));

    /* truncated to 48 bytes WITHOUT splitting a sequence: 46 'a' + 2-byte char = 48 -> fits */
    {
        char in[128];
        memset(in, 'a', 46); in[46] = '\0'; strcat(in, "\xC3\xB1" "zzz");
        CHECK(metro_device_name_sanitize(in, out, sizeof(out)) == 48);
        CHECK(out[46] == (char)0xC3 && out[47] == (char)0xB1 && out[48] == '\0');
        /* 47 'a' + a 2-byte char doesn't fit whole: cut BEFORE it */
        memset(in, 'a', 47); in[47] = '\0'; strcat(in, "\xC3\xB1");
        CHECK(metro_device_name_sanitize(in, out, sizeof(out)) == 47);
        CHECK(out[47] == '\0');
    }

    /* empty / NULL / only whitespace */
    CHECK(metro_device_name_sanitize("", out, sizeof(out)) == 0 && out[0] == '\0');
    CHECK(metro_device_name_sanitize(NULL, out, sizeof(out)) == 0);
    CHECK(metro_device_name_sanitize("   ", out, sizeof(out)) == 0);

    /* small buffer: outsz is respected */
    {
        char small[6];
        CHECK(metro_device_name_sanitize("abcdefgh", small, sizeof(small)) == 5);
        CHECK(!strcmp(small, "abcde"));
    }

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
