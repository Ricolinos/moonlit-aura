# moonlit.aura

Firmware para iPod Classic 6G (S5L8702, 320×240, 64 MB, sin GPU, sin FPU), fork de
Metro-Aura (tag `moonlit-fork-base`) sobre Rockbox (M-001). Nombre visible: "moonlit.aura".

## Comandos
- Target:      `firmware/tools/build_target.sh [--firmware|--bootloader]`  → `firmware/build-ipod6g/rockbox.ipod`
- Simulador:   `firmware/tools/build_sim.sh [--reconfigure] [--run]` (hace `make install` + copia .fnt; brew sdl2 gcc)
- Captura:     `firmware/tools/sim_shot.sh <out.png> [ticks] "SELECT,RIGHT,…"` (320×240, headless)
- Tokens/fuentes/iconos: `design-system/.venv/bin/python3 design-system/generate.py` (falla si un ícono tiene <4 tonos) — desde H2
- Tests host:  `make -C firmware/rockbox/apps/metro/test test`
- Release:     `firmware/tools/package_dist.sh --release-tag vX.Y.Z` (árbol limpio, sin __DATE__/__TIME__)

## Reglas que no se deducen del código
- Idiomas: `.md` en español de México sin voseo; código, comentarios y commits en inglés; UI en español (metro_lang.c).
- Toda decisión de diseño se cierra por escrito en `DECISIONS.md` (D-NNN) antes de ejecutarse. D-001…D-017 son vinculantes.
  Bitácora heredada de Metro (M-NNN): `DECISIONS-METRO-ARCHIVE.md`, solo lectura.
- Plan vigente: `docs/plan/03-plan-implementacion.md` (hitos H0–H7). `docs/plans/archivo/` es histórico de Metro, nunca trabajo pendiente.
- Ningún literal RGB fuera de `design-system/tokens.json`. En C solo se leen por `moonlit_palette.h` (hasta H3: `metro_palette.h`).
- Ninguna lectura de disco dentro de un bucle de animación; toda animación bajo `lcd_active()` y el nivel FX de `aura.cfg`.
- Prohibido desde `apps/metro/`: `root_menu()`, `do_menu()`, `gui_synclist`, `rockbox_browse()`, `kbd_input()`, skin engine.
- Todo `struct viewport` local se inicializa con `viewport_set_defaults()` (M-027). Texto siempre vía `metro_draw_text*()` (M-051).
- Rutas bajo `.rockbox/aura/` y `/.aura/` solo en `metro_settings.c`/`metro_sync.c`/`metro_device.c`/`metro_media_categories.c`.
  Contrato canónico: `Aura-Firmware/CONTRATO-firmware-studio.md` (v13) + `CONTRATO-moonlit-studio.md` de este repo. Inmutable desde aquí.
- `firmware_family: moonlit`, caché `/.rockbox/aura/moonlitcache/`, árbol dormido `/.firmware-moonlit/`. Nunca `metrocache/`.
- Cambios fuera de `apps/metro/` → `MODIFICATIONS.md` en la misma pasada + comentario `moonlit (D-NNN)`.
- Iconos y logo: compilados en tabla C (nunca disco); verificación mecánica de tonos ≥4 en `generate.py`, no visual.
- Fuentes: builds estáticas TTF, rango decimal 32–383 en convttf (nunca `0x…`), ningún rol < 18 px.
- Sin material de Apple ni Microsoft en el árbol ni en `firmware/dist/`.

## Diseño
Antes de tocar cualquier pantalla, animación, ícono o componente: skill `moonlit-design-system`.
Fuente viva: `docs/moonlit-design-system/00-INDICE.md` (se crea en H2–H3; hasta entonces, plan §B–§E).
