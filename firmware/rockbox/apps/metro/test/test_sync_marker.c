/* Tests host-side de metro_sync_marker.c: sin dependencias de Rockbox,
 * compila y corre nativo. Ejecutar con `make -C apps/metro/test`. */
#include <stdio.h>
#include <string.h>
#include "../metro_sync_marker.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        failures++; \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_init(void)
{
    metro_sync_marker_t m;
    metro_sync_marker_init(&m);
    CHECK(m.version == -1);
    CHECK(m.timestamp[0] == '\0');
    CHECK(!m.music && !m.video && !m.images);
    CHECK(m.attempts == 0);
    CHECK(!metro_sync_marker_has_work(&m));
}

static void test_canonical_marker(void)
{
    metro_sync_marker_t m;
    const char *json =
        "{\n"
        "  \"version\": 1,\n"
        "  \"timestamp\": \"2026-08-17T20:15:00Z\",\n"
        "  \"changes\": { \"music\": true, \"video\": false, \"images\": true }\n"
        "}\n";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(m.version == 1);
    CHECK(!strcmp(m.timestamp, "2026-08-17T20:15:00Z"));
    CHECK(m.music);
    CHECK(!m.video);
    CHECK(m.images);
    CHECK(m.attempts == 0);
    CHECK(metro_sync_marker_has_work(&m));
}

static void test_compact_and_reordered(void)
{
    metro_sync_marker_t m;
    /* No whitespace, keys reordered, explicit attempts. */
    const char *json = "{\"attempts\":2,\"changes\":{\"images\":false,\"video\":true,\"music\":false},\"version\":1,\"timestamp\":\"x\"}";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(!m.music && m.video && !m.images);
    CHECK(m.attempts == 2);
    CHECK(!strcmp(m.timestamp, "x"));
}

static void test_unknown_keys_ignored(void)
{
    metro_sync_marker_t m;
    const char *json =
        "{ \"version\": 1, \"source\": \"Aura Studio 1.2\", \"studio_build\": 44,"
        "  \"changes\": { \"music\": true, \"themes\": true }, \"note\": \"music: false\" }";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(m.music);
    CHECK(!m.video && !m.images);
}

static void test_key_inside_string_value_is_not_a_key(void)
{
    metro_sync_marker_t m;
    /* "video" appears as a text VALUE before it appears as a real key. */
    const char *json =
        "{ \"version\": 1, \"timestamp\": \"video\", \"changes\": { \"video\": true } }";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(!strcmp(m.timestamp, "video"));
    CHECK(m.video);
}

static void test_missing_version(void)
{
    metro_sync_marker_t m;
    CHECK(metro_sync_marker_parse("{ \"changes\": { \"music\": true } }", &m)
          == METRO_SYNC_MARKER_MISSING_VERSION);
    CHECK(m.version == -1);
    CHECK(!m.music); /* out is left as init() left it: nothing gets rebuilt */
    CHECK(metro_sync_marker_parse("{ \"version\": \"uno\" }", &m)
          == METRO_SYNC_MARKER_MISSING_VERSION);
}

static void test_malformed(void)
{
    metro_sync_marker_t m;
    CHECK(metro_sync_marker_parse("", &m) == METRO_SYNC_MARKER_MALFORMED);
    CHECK(metro_sync_marker_parse("garbage", &m) == METRO_SYNC_MARKER_MALFORMED);
    CHECK(metro_sync_marker_parse(NULL, &m) == METRO_SYNC_MARKER_MALFORMED);
    CHECK(metro_sync_marker_parse("   \n[1,2]", &m) == METRO_SYNC_MARKER_MALFORMED);
    CHECK(!metro_sync_marker_has_work(&m));
}

static void test_newer_version_is_unsupported_but_readable(void)
{
    metro_sync_marker_t m;
    const char *json = "{ \"version\": 7, \"changes\": { \"music\": true } }";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_UNSUPPORTED);
    CHECK(m.version == 7); /* so the screen can say "version 7" */
    CHECK(m.music);        /* read, but the caller must NOT act on it */
}

static void test_bool_values_strict(void)
{
    metro_sync_marker_t m;
    /* 1/"yes"/null are not true: only the literal JSON true is. */
    const char *json = "{ \"version\": 1, \"changes\": { \"music\": 1, \"video\": \"yes\", \"images\": null } }";
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(!m.music && !m.video && !m.images);
    CHECK(!metro_sync_marker_has_work(&m));
}

static void test_serialize_roundtrip(void)
{
    metro_sync_marker_t m, back;
    char buf[256];
    int n;

    metro_sync_marker_init(&m);
    m.version = 1;
    strcpy(m.timestamp, "2026-08-17T21:00:00Z");
    m.music = true;
    m.images = true;
    m.attempts = 2;

    n = metro_sync_marker_serialize(&m, buf, sizeof(buf));
    CHECK(n > 0);
    CHECK((size_t)n == strlen(buf));
    CHECK(metro_sync_marker_parse(buf, &back) == METRO_SYNC_MARKER_OK);
    CHECK(back.version == 1);
    CHECK(!strcmp(back.timestamp, "2026-08-17T21:00:00Z"));
    CHECK(back.music && !back.video && back.images);
    CHECK(back.attempts == 2);
}

static void test_serialize_defaults_and_too_small(void)
{
    metro_sync_marker_t m, back;
    char big[256];
    char tiny[16];

    metro_sync_marker_init(&m); /* version -1 -> serializes the supported one */
    CHECK(metro_sync_marker_serialize(&m, big, sizeof(big)) > 0);
    CHECK(metro_sync_marker_parse(big, &back) == METRO_SYNC_MARKER_OK);
    CHECK(back.version == METRO_SYNC_MARKER_VERSION_SUPPORTED);
    CHECK(metro_sync_marker_serialize(&m, tiny, sizeof(tiny)) == -1);
}

static void test_timestamp_truncates_safely(void)
{
    metro_sync_marker_t m;
    char json[256];
    char longts[80];
    memset(longts, 'a', sizeof(longts) - 1);
    longts[sizeof(longts) - 1] = '\0';
    snprintf(json, sizeof(json), "{ \"version\": 1, \"timestamp\": \"%s\" }", longts);
    CHECK(metro_sync_marker_parse(json, &m) == METRO_SYNC_MARKER_OK);
    CHECK(strlen(m.timestamp) == METRO_SYNC_MARKER_TIMESTAMP_LEN - 1);
}

int main(void)
{
    test_init();
    test_canonical_marker();
    test_compact_and_reordered();
    test_unknown_keys_ignored();
    test_key_inside_string_value_is_not_a_key();
    test_missing_version();
    test_malformed();
    test_newer_version_is_unsupported_but_readable();
    test_bool_values_strict();
    test_serialize_roundtrip();
    test_serialize_defaults_and_too_small();
    test_timestamp_truncates_safely();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
