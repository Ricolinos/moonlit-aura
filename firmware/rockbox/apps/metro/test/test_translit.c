/* Tests host de moonlit_translit.c (D-066). Modulo puro: se compila
 * solo, sin nada de Rockbox. `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../moonlit_translit.h"

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

static const char *tr(const char *in, char *buf, size_t n)
{
    return moonlit_translit(in, buf, n);
}

/* El caso del encargo: un titulo real con apostrofos y comillas
 * tipograficas, que hoy se dibuja con el caracter de reemplazo. */
static void test_caso_del_dueno(void)
{
    char b[128];

    CHECK_STR(tr("Don\xe2\x80\x99t Stop Believin\xe2\x80\x99", b, sizeof(b)),
              "Don't Stop Believin'");
    CHECK_STR(tr("\xe2\x80\x9cLive\xe2\x80\x9d", b, sizeof(b)), "\"Live\"");
    CHECK_STR(tr("Grandes \xe2\x80\xa6 exitos", b, sizeof(b)), "Grandes ... exitos");
    CHECK_STR(tr("Rock \xe2\x80\x93 Pop", b, sizeof(b)), "Rock - Pop");
    CHECK_STR(tr("A\xc2\xa0""B", b, sizeof(b)), "A B");
    CHECK_STR(tr("Kind of Blue\xe2\x84\xa2", b, sizeof(b)), "Kind of Blue(TM)");
}

/* Lo que NO se toca: ASCII puro y acentuadas Latin-1 (que SI estan en
 * el rango 32-383 de las fuentes). */
static void test_no_toca_lo_que_ya_esta(void)
{
    char b[128];

    CHECK(!moonlit_translit_needed("Analog Dreams"));
    CHECK(!moonlit_translit_needed("Canci\xc3\xb3n de cuna")); /* ó = 0xC3 0xB3 */
    CHECK(!moonlit_translit_needed(""));
    CHECK(!moonlit_translit_needed(NULL));

    CHECK_STR(tr("Canci\xc3\xb3n", b, sizeof(b)), "Canci\xc3\xb3n");
    CHECK_STR(tr("A\xc3\x91O", b, sizeof(b)), "A\xc3\x91O"); /* Ñ intacta */
}

/* Un caracter sin equivalente honesto se deja pasar tal cual: lo
 * resuelve el defaultchar de la fuente ('·', D-066), no una traduccion
 * inventada. */
static void test_sin_equivalente_pasa_intacto(void)
{
    char b[64];

    CHECK(moonlit_translit_needed("\xe2\x99\xaa")); /* ♪ arranca en 0xE2 */
    CHECK_STR(tr("\xe2\x99\xaa", b, sizeof(b)), "\xe2\x99\xaa");
    CHECK_STR(tr("\xe2\x98\x85 5", b, sizeof(b)), "\xe2\x98\x85 5"); /* ★ */
}

/* El buffer de destino manda: nunca se desborda y nunca se corta a
 * mitad de una secuencia UTF-8 (dejar bytes sueltos haria que el motor
 * de fuentes dibuje basura). */
static void test_no_desborda(void)
{
    char b[8];

    /* "..." son 3 bytes por cada …; con 8 bytes caben dos y el NUL. */
    CHECK_STR(tr("\xe2\x80\xa6\xe2\x80\xa6\xe2\x80\xa6", b, sizeof(b)), "......");

    /* Una acentuada de 2 bytes que no cabe entera no se parte. */
    {
        char c[4];
        CHECK_STR(tr("ab\xc3\xb1", c, sizeof(c)), "ab");
    }
    {
        char c[1];
        CHECK_STR(tr("hola", c, sizeof(c)), "");
    }
    /* outsz 0 no escribe nada; solo debe no reventar. */
    moonlit_translit("hola", b, 0);
    CHECK(1);
}

/* UTF-8 mal formado: se copia byte a byte, no es trabajo de este modulo
 * sanearlo -- pero no debe colgarse ni leer fuera de la cadena. */
static void test_utf8_malformado(void)
{
    char b[32];

    CHECK_STR(tr("\xe2\x80", b, sizeof(b)), "\xe2\x80");   /* truncada */
    CHECK_STR(tr("\xff\xfe", b, sizeof(b)), "\xff\xfe");   /* nunca valido */
    CHECK_STR(tr("a\xe2""b", b, sizeof(b)), "a\xe2""b");
}

/* La tabla misma: sin duplicados, ordenada por codepoint (asi se lee
 * mejor y asi la espera el lector de check_fonts.py), y ninguna entrada
 * vacia. */
static void test_tabla_coherente(void)
{
    int i;

    CHECK(moonlit_translit_count > 0);
    for (i = 0; i < moonlit_translit_count; i++)
    {
        CHECK(moonlit_translit_table[i].ascii != NULL);
        CHECK(moonlit_translit_table[i].ascii[0] != '\0');
        if (i > 0)
            CHECK(moonlit_translit_table[i].cp > moonlit_translit_table[i - 1].cp);
    }
}

int main(void)
{
    test_caso_del_dueno();
    test_no_toca_lo_que_ya_esta();
    test_sin_equivalente_pasa_intacto();
    test_no_desborda();
    test_utf8_malformado();
    test_tabla_coherente();

    printf("test_translit: %d/%d checks OK\n", checks - failures, checks);
    if (failures)
    {
        printf("%d FALLO(S)\n", failures);
        return 1;
    }
    return 0;
}
