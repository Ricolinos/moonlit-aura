# Tests host-side de la logica pura de Metro (sin Rockbox).
# Uso: make -C apps/metro/test

CC ?= cc
CFLAGS := -std=c99 -Wall -Wextra -Werror -O0 -g

BUILD := build

.PHONY: test clean

test: $(BUILD)/test_nav
	@echo "== test_nav =="
	@$(BUILD)/test_nav

$(BUILD)/test_nav: test_nav.c ../metro_nav.c ../metro_nav.h | $(BUILD)
	$(CC) $(CFLAGS) test_nav.c ../metro_nav.c -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
