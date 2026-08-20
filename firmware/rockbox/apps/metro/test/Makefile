# Tests host-side de la logica pura de Metro (sin Rockbox).
# Uso: make -C apps/metro/test

CC ?= cc
CFLAGS := -std=c99 -Wall -Wextra -Werror -O0 -g

BUILD := build

.PHONY: test clean

test: $(BUILD)/test_nav $(BUILD)/test_sync_marker $(BUILD)/test_device $(BUILD)/test_motion $(BUILD)/test_lrc
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

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
