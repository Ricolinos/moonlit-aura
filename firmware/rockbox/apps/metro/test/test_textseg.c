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

    n = moonlit_textseg_build("Don\xe2\x80\x99t Stop", false, false, buf, sizeof(buf), segs, 8);
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
        true, false, buf, sizeof(buf), segs, 16);

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
    n = moonlit_textseg_build("A\xe2\x80\x90" "common\xe2\x80\x90" "word", true, false,
                              buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[0].text, "A-common-word"));

    /* Una corchea (fuera de 8208-8482 por completo) tampoco es PUNCT:
     * sigue sin equivalente, pasa intacta para que el defaultchar del
     * rol primario la resuelva (D-066). */
    n = moonlit_textseg_build("Wheel \xe2\x99\xaa in the Sky", true, false,
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

    n = moonlit_textseg_build("Analog Dreams", true, false, buf, sizeof(buf), segs, 8);
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

    n = moonlit_textseg_build("\xe2\x80\x9cLive\xe2\x80\x9d", true, false,
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

    CHECK(moonlit_textseg_build(NULL, true, false, buf, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("", true, false, buf, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, false, NULL, sizeof(buf), segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, false, buf, 0, segs, 8) == 0);
    CHECK(moonlit_textseg_build("hola", true, false, buf, sizeof(buf), segs, 0) == 0);
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
                              true, false, buf, sizeof(buf), segs, 8);
    CHECK(n >= 1);
    for (i = 0; i < n; i++)
        CHECK(segs[i].text >= buf && segs[i].text < buf + sizeof(buf));

    /* max_segs chico: nunca escribe mas de los que se le piden. */
    {
        char buf2[BUFSZ];
        struct moonlit_textseg segs2[2];

        n = moonlit_textseg_build(
            "Don\xe2\x80\x99t Stop \xe2\x80\x94 \xe2\x80\x9cLive\xe2\x80\x9d\xe2\x80\xa6",
            true, false, buf2, sizeof(buf2), segs2, 2);
        CHECK(n <= 2);
    }
}

/* moonlit (D-081): texto ruso puro, con fuente cirilica disponible --
 * un solo tramo CYRILLIC, sin gastar tramos de mas (mismo criterio que
 * test_ascii_puro_un_tramo() para el rango primario). */
static void test_cirilico_puro_un_tramo(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    /* "музыка" (musica) -- las seis letras dentro de 1025-1105. */
    n = moonlit_textseg_build("\xd0\xbc\xd1\x83\xd0\xb7\xd1\x8b\xd0\xba\xd0\xb0",
                              true, true, buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_CYRILLIC);
    CHECK(!strcmp(segs[0].text, "\xd0\xbc\xd1\x83\xd0\xb7\xd1\x8b\xd0\xba\xd0\xb0"));
}

/* moonlit (D-081): cirilico y latin alternando -- p.ej. un titulo con
 * una palabra en ingles adentro -- cada tramo con su tipo, contiguos
 * del mismo tipo fundidos (mismo criterio que test_caso_del_dueno()). */
static void test_cirilico_y_latin_alternan(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    /* "рок - live" ("rock - live"): cirilico, luego PRIMARY -- " - " y
     * "live" son el MISMO tipo (PRIMARY) y se funden en un solo tramo,
     * igual que dos PUNCT contiguos en test_caso_del_dueno(). */
    n = moonlit_textseg_build("\xd1\x80\xd0\xbe\xd0\xba - live",
                              true, true, buf, sizeof(buf), segs, 8);
    CHECK(n == 2);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_CYRILLIC);
    CHECK(!strcmp(segs[0].text, "\xd1\x80\xd0\xbe\xd0\xba"));
    CHECK(segs[1].kind == MOONLIT_TEXTSEG_PRIMARY);
    CHECK(!strcmp(segs[1].text, " - live"));
}

/* moonlit (D-081): sin fuente cirilica (has_cyrillic_font=false, hoy
 * teorico -- los siete roles la tienen, pero el modulo no debe asumirlo)
 * un codepoint cirilico NO se clasifica CYRILLIC -- cae al mismo camino
 * que cualquier otro codepoint sin cobertura (translit o defaultchar). */
static void test_sin_fuente_cirilica_no_clasifica_cyrillic(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    n = moonlit_textseg_build("\xd0\xbc\xd1\x83\xd0\xb7\xd1\x8b\xd0\xba\xd0\xb0",
                              true, false, buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_PRIMARY);
}

/* moonlit (D-081): MFONT_DISPLAY (has_punct_font=false) SI tiene
 * cirilico -- dibuja nombres de pivote del hub, cirilicos de punta a
 * punta en ruso ("настройки"). El atajo de un solo tramo transliterado
 * de test_sin_fuente_de_puntuacion() NO debe aplicar aqui. */
static void test_display_sin_punct_pero_con_cirilico(void)
{
    char buf[BUFSZ];
    struct moonlit_textseg segs[8];
    int n;

    /* "настройки" (ajustes). */
    n = moonlit_textseg_build(
        "\xd0\xbd\xd0\xb0\xd1\x81\xd1\x82\xd1\x80\xd0\xbe\xd0\xb9\xd0\xba\xd0\xb8",
        false, true, buf, sizeof(buf), segs, 8);
    CHECK(n == 1);
    CHECK(segs[0].kind == MOONLIT_TEXTSEG_CYRILLIC);
}


/* moonlit (D-081, addendum 2): una frase RUSA entera cabe en el tope de
 * tramos que usa metro_draw.c. Un espacio ASCII es PRIMARY y parte la
 * corrida cirilica, asi que el ruso gasta ~2 tramos por palabra: con el
 * tope viejo de 12 esta frase -- el detalle del dialogo de biblioteca,
 * la cadena mas larga que se dibuja de una sola vez -- se cortaba en
 * "это может занять несколько минут, в " y el resto NO se dibujaba, en
 * silencio. Este test fija el contrato que metro_draw.c necesita: con
 * su tope actual, lo que sale reconstruye la cadena COMPLETA. */
#define TEXTSEG_MAX_DE_METRO_DRAW 128

static void test_frase_rusa_larga_no_se_trunca(void)
{
    static const char ru[] =
        "\xd1\x8d\xd1\x82\xd0\xbe\x20\xd0\xbc\xd0\xbe\xd0\xb6\xd0\xb5\xd1"
        "\x82\x20\xd0\xb7\xd0\xb0\xd0\xbd\xd1\x8f\xd1\x82\xd1\x8c\x20\xd0"
        "\xbd\xd0\xb5\xd1\x81\xd0\xba\xd0\xbe\xd0\xbb\xd1\x8c\xd0\xba\xd0"
        "\xbe\x20\xd0\xbc\xd0\xb8\xd0\xbd\xd1\x83\xd1\x82\x2c\x20\xd0\xb2"
        "\x20\xd0\xb7\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x81\xd0\xb8\xd0\xbc\xd0"
        "\xbe\xd1\x81\xd1\x82\xd0\xb8\x20\xd0\xbe\xd1\x82\x20\xd0\xba\xd0"
        "\xbe\xd0\xbb\xd0\xb8\xd1\x87\xd0\xb5\xd1\x81\xd1\x82\xd0\xb2\xd0"
        "\xb0\x20\xd1\x84\xd0\xb0\xd0\xb9\xd0\xbb\xd0\xbe\xd0\xb2\x20\xd0"
        "\xb8\x20\xd1\x81\xd0\xbe\xd1\x81\xd1\x82\xd0\xbe\xd1\x8f\xd0\xbd"
        "\xd0\xb8\xd1\x8f\x20\xd0\xb4\xd0\xb8\xd1\x81\xd0\xba\xd0\xb0\x2e";
    char buf[1024];
    struct moonlit_textseg segs[TEXTSEG_MAX_DE_METRO_DRAW];
    char rearmado[1024];
    int n, i;

    n = moonlit_textseg_build(ru, true, true, buf, sizeof(buf),
                              segs, TEXTSEG_MAX_DE_METRO_DRAW);
    /* Que de verdad necesita mas de los 12 de antes -- si algun dia la
     * segmentacion deja de partir en cada espacio, este numero baja y
     * el test lo dice en vez de quedarse callado. */
    CHECK(n > 12);
    CHECK(n < TEXTSEG_MAX_DE_METRO_DRAW);

    rearmado[0] = '\0';
    for (i = 0; i < n; i++)
        strcat(rearmado, segs[i].text);
    CHECK(strcmp(rearmado, ru) == 0);
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
    test_cirilico_puro_un_tramo();
    test_cirilico_y_latin_alternan();
    test_sin_fuente_cirilica_no_clasifica_cyrillic();
    test_display_sin_punct_pero_con_cirilico();
    test_frase_rusa_larga_no_se_trunca();

    printf("test_textseg: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
