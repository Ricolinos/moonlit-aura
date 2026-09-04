/* Tests host de moonlit_textseg.c (D-074). Modulo puro: enlaza junto
 * con moonlit_translit.c y moonlit_punct_table.c, sin nada de Rockbox.
 * `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../moonlit_textseg.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define BUFSZ 128

/* Sin fuente de puntuacion (MFONT_DISPLAY): siempre UN tramo PRIMARY,
 * transliterado entero -- el comportamiento de D-066 sin cambios. */
static void test_sin_fuente_de_puntuacion(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    n = moonlit_textseg_build("Don\xe2\x80\x99t Stop", false, buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "Don't Stop"));
}

/* El caso del encargo: un titulo con varias marcas de puntuacion
 * distintas, alternando con texto normal -- cada tramo con su tipo, y
 * los contiguos del mismo tipo fundidos en uno. */
static void test_caso_del_dueno(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[16];
    int n;

    /* Don't Stop -- "Live"... con apostrofo, raya, comillas y puntos
     * suspensivos tipograficos. */
    n = moonlit_textseg_build(
        "Don\xe2\x80\x99t Stop \xe2\x80\x94 \xe2\x80\x9cLive\xe2\x80\x9d\xe2\x80\xa6",
        true, buf, sizeof(buf), segs, 16);

    CHECK(n == 8);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "Don"));
    CHECK(segs[1].kind == MOONLIT_TEXTSEG_PUNCT);
    CHECK(!strcmp(segs[1].text, "\xe2\x80\x99")); /* ' */
    CHECK(segs[2].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[2].text, "t Stop "));
    CHECK(segs[3].kind == MOONLIT_TEXTSEG_PUNCT);
    CHECK(!strcmp(segs[3].text, "\xe2\x80\x94")); /* -- */
    CHECK(segs[4].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[4].text, " "));
    CHECK(segs[5].kind == MOONLIT_TEXTSEG_PUNCT);
    CHECK(!strcmp(segs[5].text, "\xe2\x80\x9c")); /* " abierta */
    CHECK(segs[6].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[6].text, "Live"));
    CHECK(segs[7].kind == MOONLIT_TEXTSEG_PUNCT);
    /* " cerrada + ... FUNDIDOS en un solo tramo PUNCT contiguo */
    CHECK(!strcmp(segs[7].text, "\xe2\x80\x9d\xe2\x80\xa6"));
}

/* Lo que NO tiene fuente de puntuacion (fuera de la interseccion, o
 * fuera del rango 8208-8482 por completo) sigue transliterando en el
 * tramo PRIMARY -- nunca se inventa un tramo PUNCT para lo que la
 * tabla generada no cubre. */
static void test_translitera_lo_que_el_punct_no_cubre(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    /* U+2010 (guion HYPHEN) quedo fuera de la interseccion (Baskerville
     * no lo trae) -- translitera a '-' en el tramo PRIMARY, como antes
     * de D-074. */
    n = moonlit_textseg_build("A\xe2\x80\x90" "common\xe2\x80\x90" "word", true,
                              buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "A-common-word"));

    /* Una corchea (fuera de 8208-8482 por completo) tampoco es PUNCT:
     * sigue sin equivalente, pasa intacta para que el defaultchar del
     * rol primario la resuelva (D-066). */
    n = moonlit_textseg_build("Wheel \xe2\x99\xaa in the Sky", true,
                              buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "Wheel \xe2\x99\xaa in the Sky"));
}

/* Texto ASCII puro, con fuente de puntuacion disponible: un solo tramo
 * PRIMARY, sin gastar ni un tramo PUNCT de mas. */
static void test_ascii_puro_un_tramo(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    n = moonlit_textseg_build("Analog Dreams", true, buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "Analog Dreams"));
}

/* Empieza o termina en un tramo PUNCT -- no hay primario que abrir o
 * cerrar alrededor. */
static void test_punct_al_borde(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    n = moonlit_textseg_build("\xe2\x80\x9cLive\xe2\x80\x9d", true,
                              buf, sizeof(buf), segs, 8);
    CHECK(n == 3);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PUNCT);
    CHECK(!strcmp(segs[0].text, "\xe2\x80\x9c"));
    CHECK(segs[1].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[1].text, "Live"));
    CHECK(segs[2].kind == MOONLIT_TEXTSEG_PUNCT);
    CHECK(!strcmp(segs[2].text, "\xe2\x80\x9d"));
}

/* Degenerados: nunca desreferencia NULL, nunca revienta con buffers o
 * limites de tramo en cero. */
static void test_degenerados(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];

    CHECK(moonlit_textseg_build(NULL, true, buf, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("", true, buf, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, NULL, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, buf, 0, segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, buf, sizeof(buf), segs, 0) == 0);
}

/* El buffer y el arreglo de tramos mandan: nunca se desborda ninguno
 * de los dos, aunque el texto de entrada de para mas. */
static void test_no_desborda(void)
{
    char buf[8];
    struct moonlit_textseg segs[8];
    int n, i;

    /* Buffer chico: se trunca sin escribir fuera de el. */
    n = moonlit_textseg_build("Don\xe2\x80\x99t Stop \xe2\x80\x94 \xe2\x80\x9cLive\xe2\x80\x9d\xe2\x80\xa6",
                              true, buf, sizeof(buf), segs, 8);
    CHECK(n >= 1);
    for (i = 0; i < n; i++)
        CHECK(segs[i].text >= buf && segs[i].text < buf + sizeof(buf));

    /* max_segs chico: nunca escribe mas de los que se le piden. */
    {
        char buf2[BUFSZ];
        struct moonlit_textseg segs2[2];

        n = moonlit_textseg_build(
            "Don\xe2\x80\x99t Stop \xe2\x80\x94 \xe2\x80\x9cLive\xe2\x80\x9d\xe2\x80\xa6",
            true, buf2, sizeof(buf2), segs2, 2);
        CHECK(n <= 2);
    }
}

int main(void)
{
    test_sin_fuente_de_puntuacion();
    test_caso_del_dueno();
    test_translitera_lo_que_el_punct_no_cubre();
    test_ascii_puro_un_tramo();
    test_punct_al_borde();
    test_degenerados();
    test_no_desborda();

    printf("test_textseg: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
