# Tests host-side de la logica pura de Metro (sin Rockbox).
# Uso: make -C apps/metro/test

CC ?= cc
CFLAGS := -std=c99 -Wall -Wextra -Werror -O0 -g
# D-042: -I. resuelve `#include "file.h"`/`#include "lcd.h"` desde
# ../moonlit_art.c contra los sustitutos de host de este directorio
# (test/file.h, test/lcd.h) -- solo se activa si el archivo no existe
# ya junto a moonlit_art.c (apps/metro/), que es el caso en build real.
CFLAGS += -I.

BUILD := build

.PHONY: test clean

test: $(BUILD)/test_nav $(BUILD)/test_sync_marker $(BUILD)/test_device $(BUILD)/test_motion $(BUILD)/test_lrc $(BUILD)/test_artist_images $(BUILD)/test_fsutil $(BUILD)/test_lang $(BUILD)/test_volume $(BUILD)/test_tokens $(BUILD)/test_flow $(BUILD)/test_wheel $(BUILD)/test_art $(BUILD)/test_firmware_families $(BUILD)/test_marea_prefetch $(BUILD)/test_master_art $(BUILD)/test_translit $(BUILD)/test_marquee $(BUILD)/test_textseg $(BUILD)/test_shared_settings
	@echo "== test_nav =="
	@$(BUILD)/test_nav
	@echo "== test_sync_marker =="
	@$(BUILD)/test_sync_marker
	@echo "== test_device =="
	@$(BUILD)/test_device
	@echo "== test_motion =="
	@$(BUILD)/test_motion
	@echo "== test_lrc =="
	@$(BUILD)/test_lrc
	@echo "== test_artist_images =="
	@$(BUILD)/test_artist_images
	@echo "== test_fsutil =="
	@$(BUILD)/test_fsutil
	@echo "== test_lang =="
	@$(BUILD)/test_lang
	@echo "== test_volume =="
	@$(BUILD)/test_volume
	@echo "== test_tokens =="
	@$(BUILD)/test_tokens
	@echo "== test_flow =="
	@$(BUILD)/test_flow
	@echo "== test_wheel =="
	@$(BUILD)/test_wheel
	@echo "== test_art =="
	@$(BUILD)/test_art
	@echo "== test_firmware_families =="
	@$(BUILD)/test_firmware_families
	@echo "== test_marea_prefetch =="
	@$(BUILD)/test_marea_prefetch
	@echo "== test_master_art =="
	@$(BUILD)/test_master_art
	@echo "== test_translit =="
	@$(BUILD)/test_translit
	@echo "== test_marquee =="
	@$(BUILD)/test_marquee
	@echo "== test_textseg =="
	@$(BUILD)/test_textseg
	@echo "== test_shared_settings =="
	@$(BUILD)/test_shared_settings

$(BUILD)/test_marquee: test_marquee.c ../moonlit_marquee_cycle.c | $(BUILD)
	$(CC) $(CFLAGS) test_marquee.c ../moonlit_marquee_cycle.c -o $@

$(BUILD)/test_textseg: test_textseg.c ../moonlit_textseg.c ../moonlit_textseg.h ../moonlit_translit.c ../moonlit_translit.h ../moonlit_punct_table.c ../moonlit_punct_table.h | $(BUILD)
	$(CC) $(CFLAGS) test_textseg.c ../moonlit_textseg.c ../moonlit_translit.c ../moonlit_punct_table.c -o $@

$(BUILD)/test_translit: test_translit.c ../moonlit_translit.c ../moonlit_translit.h | $(BUILD)
	$(CC) $(CFLAGS) test_translit.c ../moonlit_translit.c -o $@

$(BUILD)/test_nav: test_nav.c ../metro_nav.c ../metro_nav.h | $(BUILD)
	$(CC) $(CFLAGS) test_nav.c ../metro_nav.c -o $@

$(BUILD)/test_sync_marker: test_sync_marker.c ../metro_sync_marker.c ../metro_sync_marker.h | $(BUILD)
	$(CC) $(CFLAGS) test_sync_marker.c ../metro_sync_marker.c -o $@

$(BUILD)/test_device: test_device.c ../metro_device_name.c ../metro_device_name.h | $(BUILD)
	$(CC) $(CFLAGS) test_device.c ../metro_device_name.c -o $@

$(BUILD)/test_motion: test_motion.c ../metro_motion.c ../metro_motion.h | $(BUILD)
	$(CC) $(CFLAGS) test_motion.c ../metro_motion.c -o $@

$(BUILD)/test_lrc: test_lrc.c ../metro_lrc.c ../metro_lrc.h | $(BUILD)
	$(CC) $(CFLAGS) test_lrc.c ../metro_lrc.c -o $@

$(BUILD)/test_artist_images: test_artist_images.c ../metro_artist_images.c ../metro_artist_images.h ../metro_artist_images_parse.c ../metro_artist_images_parse.h | $(BUILD)
	$(CC) $(CFLAGS) test_artist_images.c ../metro_artist_images.c ../metro_artist_images_parse.c -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

$(BUILD)/test_fsutil: test_fsutil.c ../metro_fsutil.h | $(BUILD)
	$(CC) $(CFLAGS) test_fsutil.c -o $@

$(BUILD)/test_lang: test_lang.c ../metro_lang.c ../metro_lang.h | $(BUILD)
	$(CC) $(CFLAGS) test_lang.c ../metro_lang.c -o $@

$(BUILD)/test_volume: test_volume.c ../metro_volume.c ../metro_volume.h | $(BUILD)
	$(CC) $(CFLAGS) test_volume.c ../metro_volume.c -o $@

$(BUILD)/test_tokens: test_tokens.c ../moonlit_tokens.h | $(BUILD)
	$(CC) $(CFLAGS) test_tokens.c -o $@

$(BUILD)/test_flow: test_flow.c ../moonlit_flow.c ../moonlit_flow.h | $(BUILD)
	$(CC) $(CFLAGS) test_flow.c ../moonlit_flow.c -o $@

$(BUILD)/test_wheel: test_wheel.c ../moonlit_wheel.c ../moonlit_wheel.h | $(BUILD)
	$(CC) $(CFLAGS) test_wheel.c ../moonlit_wheel.c -o $@

$(BUILD)/test_art: test_art.c ../moonlit_art.c ../moonlit_art.h file.h dir.h lcd.h | $(BUILD)
	$(CC) $(CFLAGS) test_art.c ../moonlit_art.c -o $@

$(BUILD)/test_firmware_families: test_firmware_families.c ../metro_firmware_families.c ../metro_firmware_families.h | $(BUILD)
	$(CC) $(CFLAGS) test_firmware_families.c ../metro_firmware_families.c -o $@

$(BUILD)/test_marea_prefetch: test_marea_prefetch.c ../moonlit_marea_prefetch.c ../moonlit_marea_prefetch.h | $(BUILD)
	$(CC) $(CFLAGS) test_marea_prefetch.c ../moonlit_marea_prefetch.c -o $@

$(BUILD)/test_master_art: test_master_art.c ../moonlit_master_art.c ../moonlit_master_art.h file.h dir.h lcd.h | $(BUILD)
	$(CC) $(CFLAGS) test_master_art.c ../moonlit_master_art.c -o $@

$(BUILD)/test_shared_settings: test_shared_settings.c ../moonlit_shared_settings.c ../moonlit_shared_settings.h | $(BUILD)
	$(CC) $(CFLAGS) test_shared_settings.c ../moonlit_shared_settings.c -o $@
