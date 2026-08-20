/* Tests host-side de metro_artist_images_parse.c y metro_artist_images.c:
 * sin dependencias de Rockbox, compila y corre nativo. Ejecutar con
 * `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_artist_images_parse.h"
#include "../metro_artist_images.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_parse_normal_line(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    CHECK(metro_artist_images_parse_line("metro-qa.jpg: Metro QA",
                                          filename, sizeof(filename),
                                          artist, sizeof(artist)));
    CHECK(!strcmp(filename, "metro-qa.jpg"));
    CHECK(!strcmp(artist, "Metro QA"));
}

static void test_parse_artist_with_colon(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    /* El valor (artista) SI puede traer ':' -- justo la razon del
     * formato invertido (B.1). Solo se corta en el PRIMER ':'. */
    CHECK(metro_artist_images_parse_line("dj.jpg: DJ: The Remix Artist",
                                          filename, sizeof(filename),
                                          artist, sizeof(artist)));
    CHECK(!strcmp(filename, "dj.jpg"));
    CHECK(!strcmp(artist, "DJ: The Remix Artist"));
}

static void test_parse_trims_whitespace(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    CHECK(metro_artist_images_parse_line("  spaced.jpg  :   Spaced Artist   ",
                                          filename, sizeof(filename),
                                          artist, sizeof(artist)));
    CHECK(!strcmp(filename, "spaced.jpg"));
    CHECK(!strcmp(artist, "Spaced Artist"));
}

static void test_parse_comment_rejected(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    CHECK(!metro_artist_images_parse_line("# comment: not an entry",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
    CHECK(!metro_artist_images_parse_line("   # indented comment too",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
}

static void test_parse_blank_line_rejected(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    CHECK(!metro_artist_images_parse_line("", filename, sizeof(filename),
                                           artist, sizeof(artist)));
    CHECK(!metro_artist_images_parse_line("    ", filename, sizeof(filename),
                                           artist, sizeof(artist)));
}

static void test_parse_malformed_no_colon(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    CHECK(!metro_artist_images_parse_line("no colon here at all",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
}

static void test_parse_empty_fields_rejected(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];

    /* Sin nombre de archivo antes del ':'. */
    CHECK(!metro_artist_images_parse_line(": Artist Only",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
    /* Sin artista despues del ':'. */
    CHECK(!metro_artist_images_parse_line("file.jpg:",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
    CHECK(!metro_artist_images_parse_line("file.jpg:    ",
                                           filename, sizeof(filename),
                                           artist, sizeof(artist)));
}

static void test_parse_field_length_caps(void)
{
    char filename[METRO_ARTIST_IMAGES_FILE_LEN];
    char artist[METRO_ARTIST_IMAGES_ARTIST_LEN];
    char line[400];
    int i;

    /* Nombre de archivo de 130 bytes (excede el tope de 128) -- linea
     * completa descartada. */
    memset(line, 'a', 130);
    strcpy(line + 130, ".jpg: Someone");
    CHECK(!metro_artist_images_parse_line(line, filename, sizeof(filename),
                                           artist, sizeof(artist)));

    /* Artista de 70 bytes (excede el tope de 64) -- linea completa
     * descartada. */
    strcpy(line, "file.jpg: ");
    for (i = 0; i < 70; i++)
        line[10 + i] = 'b';
    line[10 + 70] = '\0';
    CHECK(!metro_artist_images_parse_line(line, filename, sizeof(filename),
                                           artist, sizeof(artist)));

    /* Justo en el limite (63 bytes de artista, cabe en 64 con el NUL)
     * si pasa. */
    strcpy(line, "file.jpg: ");
    for (i = 0; i < 63; i++)
        line[10 + i] = 'c';
    line[10 + 63] = '\0';
    CHECK(metro_artist_images_parse_line(line, filename, sizeof(filename),
                                          artist, sizeof(artist)));
    CHECK(strlen(artist) == 63);
}

static void test_index_lookup(void)
{
    struct metro_artist_images idx;
    metro_artist_images_init(&idx);

    CHECK(metro_artist_images_add_line(&idx, "metro-qa.jpg: Metro QA"));
    CHECK(metro_artist_images_add_line(&idx, "wheel.jpg: Wheel & Click"));
    CHECK(idx.count == 2);

    CHECK(!strcmp(metro_artist_images_lookup(&idx, "Metro QA"), "metro-qa.jpg"));
    CHECK(!strcmp(metro_artist_images_lookup(&idx, "Wheel & Click"), "wheel.jpg"));
    CHECK(metro_artist_images_lookup(&idx, "Nobody Here") == NULL);
}

static void test_index_ignores_malformed_lines(void)
{
    struct metro_artist_images idx;
    metro_artist_images_init(&idx);

    CHECK(!metro_artist_images_add_line(&idx, "# just a comment"));
    CHECK(!metro_artist_images_add_line(&idx, "no colon"));
    CHECK(idx.count == 0);
}

static void test_index_duplicate_value_first_wins(void)
{
    struct metro_artist_images idx;
    metro_artist_images_init(&idx);

    /* Dos archivos DISTINTOS apuntando al mismo tag de artista --
     * variantes de como quedo tageada la musica (B.1). La primera
     * linea gana. */
    CHECK(metro_artist_images_add_line(&idx, "old-photo.jpg: Metro QA"));
    CHECK(metro_artist_images_add_line(&idx, "new-photo.jpg: Metro QA"));
    CHECK(idx.count == 1);
    CHECK(!strcmp(metro_artist_images_lookup(&idx, "Metro QA"), "old-photo.jpg"));
}

static void test_index_caps_at_300(void)
{
    static struct metro_artist_images idx; /* grande -- estatico */
    char line[64];
    int i;

    metro_artist_images_init(&idx);

    for (i = 0; i < METRO_ARTIST_IMAGES_MAX + 20; i++)
    {
        snprintf(line, sizeof(line), "artist%04d.jpg: Artist Number %04d", i, i);
        CHECK(metro_artist_images_add_line(&idx, line));
    }

    CHECK(idx.count == METRO_ARTIST_IMAGES_MAX);
    /* Los primeros 300 SI quedaron indexados -- solo el excedente se
     * descarta, no un bloque arbitrario. */
    CHECK(metro_artist_images_lookup(&idx, "Artist Number 0000") != NULL);
    CHECK(metro_artist_images_lookup(&idx, "Artist Number 0299") != NULL);
    CHECK(metro_artist_images_lookup(&idx, "Artist Number 0300") == NULL);
}

int main(void)
{
    test_parse_normal_line();
    test_parse_artist_with_colon();
    test_parse_trims_whitespace();
    test_parse_comment_rejected();
    test_parse_blank_line_rejected();
    test_parse_malformed_no_colon();
    test_parse_empty_fields_rejected();
    test_parse_field_length_caps();
    test_index_lookup();
    test_index_ignores_malformed_lines();
    test_index_duplicate_value_first_wins();
    test_index_caps_at_300();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
