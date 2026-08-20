/* Tests host-side de metro_lrc.c: sin dependencias de Rockbox, compila
 * y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_lrc.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void load(struct metro_lrc *lrc, const char *text)
{
    size_t len = strlen(text);
    memcpy(lrc->buf, text, len);
    metro_lrc_parse(lrc, len);
}

static void test_plain_mm_ss(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:00]Primera linea\n[00:05]Segunda linea\n");
    CHECK(lrc.count == 2);
    CHECK(lrc.entries[0].ms == 0);
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Primera linea"));
    CHECK(lrc.entries[1].ms == 5000);
    CHECK(!strcmp(metro_lrc_text(&lrc, 1), "Segunda linea"));
}

static void test_fractional_variants(void)
{
    struct metro_lrc lrc;
    /* 1, 2 y 3 digitos de fraccion -- 100ms/10ms/1ms de peso cada uno. */
    load(&lrc, "[00:01.5]uno\n[00:02.50]dos\n[00:03.500]tres\n");
    CHECK(lrc.count == 3);
    CHECK(lrc.entries[0].ms == 1500);
    CHECK(lrc.entries[1].ms == 2500);
    CHECK(lrc.entries[2].ms == 3500);
}

static void test_multiple_timestamps_share_text(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:12.00][00:45.00]Coro repetido\n");
    CHECK(lrc.count == 2);
    CHECK(lrc.entries[0].ms == 12000);
    CHECK(lrc.entries[1].ms == 45000);
    /* Mismo texto -- y de hecho el MISMO puntero, sin duplicar. */
    CHECK(metro_lrc_text(&lrc, 0) == metro_lrc_text(&lrc, 1));
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Coro repetido"));
}

static void test_metadata_tags_skipped(void)
{
    struct metro_lrc lrc;
    load(&lrc,
         "[ar:Artista]\n"
         "[ti:Titulo]\n"
         "[00:00.00]Letra real\n");
    /* [ar:...] y [ti:...] no son timestamps validos -- esas dos lineas
     * se descartan enteras (ninguna aporta una entrada). */
    CHECK(lrc.count == 1);
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Letra real"));
}

static void test_line_without_timestamp_dropped(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:00.00]Con marca\nSin marca de tiempo\n[00:10.00]Otra con marca\n");
    CHECK(lrc.count == 2);
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Con marca"));
    CHECK(!strcmp(metro_lrc_text(&lrc, 1), "Otra con marca"));
}

static void test_malformed_tag_recovers(void)
{
    struct metro_lrc lrc;
    /* Un tag con basura adentro no rompe el resto de la linea, solo se
     * salta hasta el ']' que lo cierra -- igual que un tag de
     * metadata. */
    load(&lrc, "[bogus][00:07.00]Sobrevive\n");
    CHECK(lrc.count == 1);
    CHECK(lrc.entries[0].ms == 7000);
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Sobrevive"));
}

static void test_bom_breaks_only_first_line(void)
{
    struct metro_lrc lrc;
    /* BOM UTF-8 (EF BB BF) antes del primer '[' -- la linea no empieza
     * con '[' asi que se descarta entera, igual que el parser de Aura
     * (INVESTIGACION-metro-r3.md A.1). La segunda linea, intacta, se
     * sigue leyendo bien. */
    const char text[] = "\xEF\xBB\xBF[00:00.00]Se pierde\n[00:05.00]Esta si\n";
    size_t len = sizeof(text) - 1;
    memcpy(lrc.buf, text, len);
    metro_lrc_parse(&lrc, len);
    CHECK(lrc.count == 1);
    CHECK(!strcmp(metro_lrc_text(&lrc, 0), "Esta si"));
}

static void test_no_valid_lines_returns_false(void)
{
    struct metro_lrc lrc;
    bool ok;
    size_t len;
    const char *text = "[ar:Nadie]\nSin marcas por ningun lado\n";
    len = strlen(text);
    memcpy(lrc.buf, text, len);
    ok = metro_lrc_parse(&lrc, len);
    CHECK(!ok);
    CHECK(lrc.count == 0);
}

static void test_exceeds_max_lines(void)
{
    static struct metro_lrc lrc; /* grande -- estatico, no en el stack */
    char *p = lrc.buf;
    int i;
    size_t remaining = sizeof(lrc.buf);

    /* METRO_LRC_MAX_LINES+50 lineas de "[00:00.00]x\n" (12 bytes cada
     * una) caben de sobra en los 8KB del buffer -- lo que se prueba
     * aqui es el tope de 600 ENTRADAS, no el tope de bytes. */
    for (i = 0; i < METRO_LRC_MAX_LINES + 50; i++)
    {
        int n = snprintf(p, remaining, "[00:00.00]x\n");
        if (n <= 0 || (size_t)n >= remaining)
            break;
        p += n;
        remaining -= (size_t)n;
    }

    metro_lrc_parse(&lrc, (size_t)(p - lrc.buf));
    CHECK(lrc.count == METRO_LRC_MAX_LINES);
}

static void test_find_active(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:00.00]cero\n[00:10.00]diez\n[00:20.00]veinte\n");
    CHECK(metro_lrc_find_active(&lrc, 0) == 0);
    CHECK(metro_lrc_find_active(&lrc, 5000) == 0);
    CHECK(metro_lrc_find_active(&lrc, 10000) == 1);
    CHECK(metro_lrc_find_active(&lrc, 15000) == 1);
    CHECK(metro_lrc_find_active(&lrc, 25000) == 2);
}

static void test_find_active_before_first_line(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:05.00]primera marca\n");
    /* Antes del primer timestamp -- todavia no hay linea activa. */
    CHECK(metro_lrc_find_active(&lrc, 0) == -1);
    CHECK(metro_lrc_find_active(&lrc, 4999) == -1);
    CHECK(metro_lrc_find_active(&lrc, 5000) == 0);
}

static void test_text_out_of_range(void)
{
    struct metro_lrc lrc;
    load(&lrc, "[00:00.00]unica\n");
    CHECK(metro_lrc_text(&lrc, -1) == NULL);
    CHECK(metro_lrc_text(&lrc, 1) == NULL);
    CHECK(metro_lrc_text(&lrc, 0) != NULL);
}

static void test_sibling_path(void)
{
    char out[64];
    CHECK(metro_lrc_sibling_path("/Music/Artist/song.mp3", out, sizeof(out)));
    CHECK(!strcmp(out, "/Music/Artist/song.lrc"));

    CHECK(metro_lrc_sibling_path("/Music/track.flac", out, sizeof(out)));
    CHECK(!strcmp(out, "/Music/track.lrc"));
}

static void test_sibling_path_no_extension(void)
{
    char out[64];
    CHECK(!metro_lrc_sibling_path("/Music/noext", out, sizeof(out)));
    /* Punto en un directorio pero no en el nombre de archivo -- no hay
     * extension que reemplazar. */
    CHECK(!metro_lrc_sibling_path("/Music/Album.2026/track", out, sizeof(out)));
}

static void test_sibling_path_buffer_too_small(void)
{
    char out[10];
    CHECK(!metro_lrc_sibling_path("/Music/Artist/song.mp3", out, sizeof(out)));
}

int main(void)
{
    test_plain_mm_ss();
    test_fractional_variants();
    test_multiple_timestamps_share_text();
    test_metadata_tags_skipped();
    test_line_without_timestamp_dropped();
    test_malformed_tag_recovers();
    test_bom_breaks_only_first_line();
    test_no_valid_lines_returns_false();
    test_exceeds_max_lines();
    test_find_active();
    test_find_active_before_first_line();
    test_text_out_of_range();
    test_sibling_path();
    test_sibling_path_no_extension();
    test_sibling_path_buffer_too_small();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
