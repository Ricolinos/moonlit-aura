/* Tests host-side de moonlit_shared_settings.c (D-079, maestro SS A):
 * sin dependencias de Rockbox, compila y corre nativo. Ejecutar con
 * `make -C apps/metro/test`. El vector de SS A.3 es LITERAL -- el
 * mismo texto que Aura y Metro usan en sus propios tests host. */
#include <stdio.h>
#include <string.h>
#include "../moonlit_shared_settings.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_STR(got, want) do { \
    checks++; \
    if (strcmp((got), (want)) != 0) { \
        failures++; \
        printf("FALLO %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, (got), (want)); \
    } \
} while (0)

/* SS A.3, vector canonico -- 13 claves conocidas + una futura. */
static const char *VECTOR_A3 =
    "# aura-shared-settings v1\n"
    "rev: 7\n"
    "updated_by: metro\n"
    "screen_lock_enabled: 1\n"
    "screen_lock_pin: 0427\n"
    "screen_lock_require: 1min\n"
    "brightness: 32\n"
    "backlight_timeout: 10\n"
    "idle_poweroff: 20\n"
    "keyclick: 1\n"
    "volume_limit: -6\n"
    "replaygain: album\n"
    "language: fr\n"
    "appearance: light\n"
    "clave_futura: lo que sea\n";

static void test_vector_a3(void)
{
    moonlit_shared_settings_t s;
    char buf[1024];
    int n;

    CHECK(moonlit_shared_settings_parse(VECTOR_A3, &s));

    /* Las 13 claves conocidas, parseadas exactas. */
    CHECK(s.have_rev && s.rev == 7);
    CHECK(s.have_updated_by); CHECK_STR(s.updated_by, "metro");
    CHECK(s.have_screen_lock_enabled && s.screen_lock_enabled);
    CHECK(s.have_screen_lock_pin); CHECK_STR(s.screen_lock_pin, "0427");
    CHECK(s.have_screen_lock_require); CHECK_STR(s.screen_lock_require, "1min");
    CHECK(s.have_brightness && s.brightness == 32);
    CHECK(s.have_backlight_timeout && s.backlight_timeout == 10);
    CHECK(s.have_idle_poweroff && s.idle_poweroff == 20);
    CHECK(s.have_keyclick && s.keyclick);
    CHECK(s.have_volume_limit && s.volume_limit == -6);
    CHECK(s.have_replaygain); CHECK_STR(s.replaygain, "album");
    CHECK(s.have_language); CHECK_STR(s.language, "fr");
    CHECK(s.have_appearance); CHECK_STR(s.appearance, "light");

    /* language "fr" no es un codigo soportado TODAVIA en este build
     * (D-080 lo agrega en Fase 3) -- el parser igual la captura como
     * clave CONOCIDA (arriba); el mapa a enum es quien la rechaza,
     * por separado, sin que eso invalide el resto del vector. */
    CHECK(moonlit_shared_settings_lock_require_from_str(s.screen_lock_require) == 1);
    CHECK(moonlit_shared_settings_replaygain_from_str(s.replaygain) == 2);
    CHECK(moonlit_shared_settings_appearance_from_str(s.appearance) == 1);

    /* "clave_futura" no es una de las 13 -- se preserva verbatim. */
    CHECK(strstr(s.unknown_lines, "clave_futura: lo que sea") != NULL);

    /* Reescribir preserva rev y la clave futura. */
    n = moonlit_shared_settings_serialize(&s, buf, sizeof(buf));
    CHECK(n > 0);
    CHECK(strstr(buf, "rev: 7\n") != NULL);
    CHECK(strstr(buf, "clave_futura: lo que sea") != NULL);
    CHECK(strstr(buf, MOONLIT_SHARED_SETTINGS_HEADER) == buf); /* primera linea */

    /* El resultado vuelve a parsear igual (round-trip). */
    {
        moonlit_shared_settings_t s2;
        CHECK(moonlit_shared_settings_parse(buf, &s2));
        CHECK(s2.have_rev && s2.rev == 7);
        CHECK_STR(s2.language, "fr");
        CHECK(strstr(s2.unknown_lines, "clave_futura: lo que sea") != NULL);
    }
}

/* SS A.3, segundo vector: sin la cabecera exacta -> el archivo entero
 * se rechaza, nunca "casi lo parsea". */
static void test_sin_cabecera_rechazado(void)
{
    moonlit_shared_settings_t s;
    static const char *sin_cabecera =
        "rev: 7\n"
        "updated_by: metro\n";

    CHECK(!moonlit_shared_settings_parse(sin_cabecera, &s));
    CHECK(!s.have_rev); /* init() lo dejo asi, no a medio llenar */

    /* Una cabecera de otra version tampoco cuenta -- coincidencia
     * exacta, no "empieza con". */
    {
        static const char *otra_version =
            "# aura-shared-settings v2\n"
            "rev: 1\n";
        CHECK(!moonlit_shared_settings_parse(otra_version, &s));
    }
}

/* SS A.3, tercer vector: brightness fuera de rango -> solo esa clave
 * se ignora, el resto del archivo se sigue aplicando. El LIMITE real
 * (MAX_BRIGHTNESS_SETTING) es del target -- este test usa uno
 * representativo via moonlit_shared_settings_int_in_range(), que es
 * lo que metro_settings.c llama de verdad con el limite real. */
static void test_brightness_fuera_de_rango(void)
{
    moonlit_shared_settings_t s;
    static const char *vector =
        "# aura-shared-settings v1\n"
        "rev: 3\n"
        "brightness: 999\n"
        "idle_poweroff: 15\n";

    CHECK(moonlit_shared_settings_parse(vector, &s));
    /* El parser SI capturo el numero (es sintacticamente valido) --
     * el rechazo por rango es responsabilidad del llamador. */
    CHECK(s.have_brightness && s.brightness == 999);
    CHECK(!moonlit_shared_settings_int_in_range(s.brightness, 1, 100));
    /* Las demas claves del mismo archivo no se contaminan. */
    CHECK(s.have_rev && s.rev == 3);
    CHECK(s.have_idle_poweroff && s.idle_poweroff == 15);
}

static void test_valores_con_forma_invalida_se_ignoran(void)
{
    moonlit_shared_settings_t s;
    static const char *vector =
        "# aura-shared-settings v1\n"
        "rev: no-es-numero\n"
        "screen_lock_enabled: quizas\n"
        "brightness: 40\n";

    CHECK(moonlit_shared_settings_parse(vector, &s));
    CHECK(!s.have_rev);
    CHECK(!s.have_screen_lock_enabled);
    CHECK(s.have_brightness && s.brightness == 40);
}

static void test_serialize_solo_las_have(void)
{
    moonlit_shared_settings_t s;
    char buf[256];
    int n;

    moonlit_shared_settings_init(&s);
    s.have_rev = true; s.rev = 1;
    s.have_appearance = true; strcpy(s.appearance, "dark");

    n = moonlit_shared_settings_serialize(&s, buf, sizeof(buf));
    CHECK(n > 0);
    CHECK(strstr(buf, "rev: 1\n") != NULL);
    CHECK(strstr(buf, "appearance: dark\n") != NULL);
    /* Ninguna otra clave -- no se inventan valores por defecto aqui,
     * eso es responsabilidad del llamador que arma `in`. */
    CHECK(strstr(buf, "brightness") == NULL);
    CHECK(strstr(buf, "language") == NULL);
}

static void test_serialize_buffer_chico(void)
{
    moonlit_shared_settings_t s;
    char buf[8]; /* mas chico que la sola cabecera */

    moonlit_shared_settings_init(&s);
    s.have_rev = true; s.rev = 1;
    CHECK(moonlit_shared_settings_serialize(&s, buf, sizeof(buf)) == -1);
}

static void test_mapas_palabra_entero(void)
{
    CHECK(moonlit_shared_settings_lock_require_from_str("hold") == 0);
    CHECK(moonlit_shared_settings_lock_require_from_str("1min") == 1);
    CHECK(moonlit_shared_settings_lock_require_from_str("5min") == 2);
    CHECK(moonlit_shared_settings_lock_require_from_str("boot") == 3);
    CHECK(moonlit_shared_settings_lock_require_from_str("nunca") == -1);
    CHECK_STR(moonlit_shared_settings_lock_require_to_str(0), "hold");
    CHECK_STR(moonlit_shared_settings_lock_require_to_str(3), "boot");
    CHECK(moonlit_shared_settings_lock_require_to_str(99) == NULL);

    CHECK(moonlit_shared_settings_replaygain_from_str("off") == 0);
    CHECK(moonlit_shared_settings_replaygain_from_str("track") == 1);
    CHECK(moonlit_shared_settings_replaygain_from_str("album") == 2);
    CHECK(moonlit_shared_settings_replaygain_from_str("nada") == -1);
    CHECK_STR(moonlit_shared_settings_replaygain_to_str(1), "track");

    CHECK(moonlit_shared_settings_appearance_from_str("dark") == 0);
    CHECK(moonlit_shared_settings_appearance_from_str("light") == 1);
    CHECK(moonlit_shared_settings_appearance_from_str("night") == -1); /* nombre interno, no el de SS A.1 */
    CHECK_STR(moonlit_shared_settings_appearance_to_str(0), "dark");
}

/* screen_lock_pin: vacio es valido (D-079: "4 digitos o vacio"); la
 * FORMA de 4 digitos la valida metro_screen_lock.c como siempre, este
 * modulo solo exige que quepa en el buffer. */
static void test_pin_vacio(void)
{
    moonlit_shared_settings_t s;
    static const char *vector =
        "# aura-shared-settings v1\n"
        "screen_lock_pin: \n";

    CHECK(moonlit_shared_settings_parse(vector, &s));
    CHECK(s.have_screen_lock_pin);
    CHECK_STR(s.screen_lock_pin, "");
}

int main(void)
{
    test_vector_a3();
    test_sin_cabecera_rechazado();
    test_brightness_fuera_de_rango();
    test_valores_con_forma_invalida_se_ignoran();
    test_serialize_solo_las_have();
    test_serialize_buffer_chico();
    test_mapas_palabra_entero();
    test_pin_vacio();

    printf("test_shared_settings: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
