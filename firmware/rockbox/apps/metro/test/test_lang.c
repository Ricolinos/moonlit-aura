/* Tests host-side de metro_lang_initial() (R4/FA-5a, M-076): sacar el
 * PRIMER CARÁCTER de una cadena UTF-8, no su primer byte.
 * Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_lang.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_INITIAL(input, expected) do { \
    char buf[5]; \
    metro_lang_initial((input), buf, sizeof(buf)); \
    checks++; \
    if (strcmp(buf, (expected)) != 0) { \
        failures++; \
        printf("FALLO %s:%d: initial(\"%s\") = \"%s\", esperado \"%s\"\n", \
               __FILE__, __LINE__, (input), buf, (expected)); \
    } \
} while (0)

static void test_ascii(void)
{
    CHECK_INITIAL("canciones", "C");
    CHECK_INITIAL("Canciones", "C");
    CHECK_INITIAL("2 Unlimited", "2");
    CHECK_INITIAL("_borrador", "_");
}

/* El caso que motivó todo esto: una letra acentuada ocupa DOS bytes,
 * así que `label[0]` entregaba medio carácter. */
static void test_acentos(void)
{
    CHECK_INITIAL("álbum desconocido", "Á");
    CHECK_INITIAL("Ángela", "Á");
    CHECK_INITIAL("éxitos", "É");
    CHECK_INITIAL("índice", "Í");
    CHECK_INITIAL("ópera", "Ó");
    CHECK_INITIAL("último", "Ú");
    CHECK_INITIAL("ñu", "Ñ");
    /* Ya en mayúscula: se conserva tal cual. */
    CHECK_INITIAL("Éxitos", "É");
    CHECK_INITIAL("Ñandú", "Ñ");
}

/* No todo 0xC3 xx es una letra: hay que no "mayusculizar" lo que no lo
 * es, ni salirse de Latin-1. */
static void test_no_letras_latin1(void)
{
    CHECK_INITIAL("÷ dividir", "÷");   /* 0xC3 0xB7, signo, no letra */
    CHECK_INITIAL("ÿ rara", "ÿ");      /* su mayúscula Ÿ no está en Latin-1 */
    CHECK_INITIAL("© 2026", "©");      /* 0xC2 xx, otro bloque */
}

/* Multibyte de 3 y 4 bytes: se copia el carácter entero aunque no haya
 * regla de mayúscula que aplicarle. */
static void test_multibyte_largo(void)
{
    CHECK_INITIAL("東京", "東");        /* 3 bytes */
    CHECK_INITIAL("😀 emoji", "😀");    /* 4 bytes */
}

/* Entradas degeneradas: nunca leer de más ni desreferenciar NULL. */
static void test_degenerado(void)
{
    char buf[5];

    CHECK_INITIAL("", "");

    metro_lang_initial(NULL, buf, sizeof(buf));
    CHECK(buf[0] == '\0');

    /* Secuencia truncada: byte guía de 2 bytes sin su continuación. */
    metro_lang_initial("\xC3", buf, sizeof(buf));
    CHECK(buf[0] == '\xC3' && buf[1] == '\0');

    /* Byte de continuación suelto: se trata como 1 byte, sin leer más. */
    metro_lang_initial("\xA1x", buf, sizeof(buf));
    CHECK(buf[0] == '\xA1' && buf[1] == '\0');

    /* Buffer que no alcanza para el carácter completo: trunca, pero
     * siempre termina en NUL y nunca escribe fuera. */
    metro_lang_initial("álbum", buf, 2);
    CHECK(buf[1] == '\0');

    /* outsz 0 no debe escribir nada -- centinela alrededor. */
    buf[0] = 'Z';
    metro_lang_initial("hola", buf, 0);
    CHECK(buf[0] == 'Z');
}

/* R4 (M-079): ordenamiento con acentos plegados. */
#define LT(a, b) do { \
    checks++; \
    if (!(metro_lang_collate((a), (b)) < 0)) { \
        failures++; \
        printf("FALLO %s:%d: esperaba \"%s\" < \"%s\"\n", \
               __FILE__, __LINE__, (a), (b)); \
    } \
} while (0)

static void test_collate(void)
{
    /* El bug que motivó esto: una inicial acentuada caía tras la Z. */
    LT("Ángela", "Beto");
    LT("Ángela", "Zoé");
    LT("Andrés", "Ángela");     /* And < Áng: la 'd' pliega antes que 'g' */
    LT("Ángela", "Antonio");    /* Áng < Ant */
    LT("Éxitos", "Fuego");
    LT("Último", "Vals");

    /* La caja no cambia DÓNDE cae una etiqueta: ambas formas aterrizan
     * en el mismo sitio respecto al resto. */
    LT("abba", "Beto");
    LT("ABBA", "Beto");
    LT("abba", "Zzz");
    /* ...pero sí desempatan entre ellas, de forma determinista: 0 solo
     * para cadenas idénticas byte a byte. */
    checks++;
    if (metro_lang_collate("abba", "ABBA") == 0)
    {
        failures++;
        printf("FALLO %s:%d: abba/ABBA deberían desempatar\n",
               __FILE__, __LINE__);
    }
    checks++;
    if (metro_lang_collate("abba", "abba") != 0)
    {
        failures++;
        printf("FALLO %s:%d: cadenas idénticas deben dar 0\n",
               __FILE__, __LINE__);
    }

    /* La ñ es letra propia: va DESPUÉS de toda la N y ANTES de la O. */
    LT("Nuevo", "Ñu");
    LT("Ñu", "Oasis");
    LT("Nz", "Ñu");             /* incluso tras la última n+consonante */

    /* Los dígitos siguen antes que las letras. */
    LT("2 Unlimited", "Abba");

    /* Empate al plegar: desempata determinista, sin quedar "igual". */
    checks++;
    if (metro_lang_collate("Angela", "Ángela") == 0)
    {
        failures++;
        printf("FALLO %s:%d: Angela/Ángela deberían desempatar\n",
               __FILE__, __LINE__);
    }

    /* Prefijo: lo más corto va primero. */
    LT("Sol", "Solar");

    /* Degenerados: no debe reventar. */
    checks++;
    if (metro_lang_collate("", "") != 0) { failures++; printf("FALLO vacias\n"); }
    LT("", "a");
}

int main(void)
{
    test_ascii();
    test_acentos();
    test_no_letras_latin1();
    test_multibyte_largo();
    test_degenerado();
    test_collate();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
