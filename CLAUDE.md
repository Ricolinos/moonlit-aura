# moonlit.aura

Firmware para iPod Classic 6G (S5L8702, 320×240 @ LCD_DPI 160, 64 MB, sin GPU/FPU). Fork de Metro-Aura
(tag `moonlit-fork-base` = 2f1bd28a) sobre Rockbox (M-001). Nombre visible: "moonlit.aura". Repo: `Ricolinos/moonlit-aura`.

## Comandos (siempre desde la raíz del repo)
- Simulador: `firmware/tools/build_sim.sh [--reconfigure] [--run]` → `firmware/build-sim/rockboxui` (brew: sdl2 gcc).
  Captura: `firmware/tools/sim_shot.sh <out.png> [ticks] "SELECT,RIGHT,SCROLL_FWD,WAIT,…"` (320×240, headless); matriz `firmware/tools/sim_matrix.sh [dir]`
- Target: `firmware/tools/build_target.sh [--firmware|--bootloader]` → `firmware/build-ipod6g/rockbox.ipod` (`RBDEV_TOOLCHAIN=<bin/>` = toolchain externo)
- Tokens/fuentes/iconos/logo: `design-system/.venv/bin/python3 design-system/generate.py --header|--fonts|--icons|--logo|--bootlogo|--contrast`
  (venv: `python3 -m venv design-system/.venv && design-system/.venv/bin/pip install pillow`)
- Verificación mecánica: `firmware/tools/check_fonts.py <fnt|--capheight png>`, `firmware/tools/check_tones.py <png> [--region x,y,w,h] [--edge]`;
  tests host `make -C firmware/rockbox/apps/metro/test test`

## Reglas que no se deducen del código
- Idiomas: Markdown en español de México sin voseo; código, comentarios y commits en inglés; UI en español (`firmware/rockbox/apps/metro/metro_lang.c`).
- Toda decisión se cierra en `DECISIONS.md` (D-NNN) antes de codificar. D-001…D-057 vinculantes. Metro histórico: `DECISIONS-METRO-ARCHIVE.md` (solo lectura).
- Plan vigente: `docs/plan/05-plan-correctivo.md` (hitos M1–M12). `docs/plans/archivo/` es histórico, nunca trabajo pendiente.
- Colores: solo `design-system/tokens.json` (roles MD3, esquemas night/dawn). En C solo `firmware/rockbox/apps/metro/moonlit_palette.c`
  incluye `firmware/rockbox/apps/metro/moonlit_tokens.h`; todo el mundo llama `moonlit_color(rol)`/`moonlit_surface(nivel, borde)`. Cero literales RGB en `apps/metro/`.
- Elevación = tono (`surface_container_*`) + borde izq/sup claro y der/inf oscuro (luz desde la izquierda). Nunca blur, ripple ni sombras difusas.
- Animación solo bajo `lcd_active()` y `metro_settings.animations != METRO_ANIM_OFF`; ninguna lectura de disco dentro de un bucle de animación.
- Fuentes: 7 roles en `firmware/rockbox/apps/metro/moonlit_fonts.h`; ningún rol < 18 px; `convttf` con rango decimal 32–383 (nunca `0x…`); builds TTF estáticas.
- Iconos/logo: tablas C generadas (nunca disco); verificación de tonos ≥ 4 en `design-system/generate.py`; jamás "a ojo".
- Prohibido desde `apps/metro/`: `root_menu()`, `do_menu()`, `gui_synclist`, `rockbox_browse()`, `kbd_input()`, skin engine.
- `struct viewport` local → `viewport_set_defaults()` (M-027). Texto siempre vía `metro_draw_text*()` (M-051).
- Rutas `.rockbox/aura/`, `/.aura/`, caché `moonlitcache/`, árboles `/.firmware-*`: solo en `firmware/rockbox/apps/metro/metro_settings.c`,
  `metro_sync.c`, `metro_device.c`, `metro_media_categories.c` y la tabla de familias `metro_firmware_families.c` (D-047).
  Contratos inmutables: `../Aura-Firmware/CONTRATO-firmware-studio.md` v15 + `CONTRATO-moonlit-studio.md` v3 de este repo.
  Rutas compartidas entre familias (v15, D-054/D-055): `/.aura/tagcache` (base + `db_stamp.txt`) y `/.aura/thumbs/` (`.mth` 80 px).
- Cambios fuera de `apps/metro/` → `MODIFICATIONS.md` en la misma pasada + comentario `moonlit (D-NNN)`.
- Nunca escribir en `../Aura-Firmware`, `../Metro-Aura`, `../Aura-Studio`. Sin material de Apple ni Microsoft en el árbol.
- Diseño: antes de tocar pantalla, animación, ícono o componente, skill `moonlit-design-system` → `docs/moonlit-design-system/00-INDICE.md`.

## Releases (contrato v11/v14, D-046/D-048)
Release = borrar tags `v*` heredados de Metro del clon local (D-046; nunca `git push --tags`) + árbol limpio + tag `vX.Y.Z`
(primero `v0.1.0`) + `firmware/tools/package_dist.sh --release-tag vX.Y.Z` + GitHub Release en `Ricolinos/moonlit-aura` con
exactamente 7 assets: `rockbox.ipod`, `rockbox.zip`, `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`, `MODIFICATIONS.md`,
`THIRD-PARTY-NOTICES.txt` — nunca `firmware/dist/README.md`. La actualización selectiva la calcula Studio (CRC32 por archivo);
lo único que cuida este repo es la reproducibilidad: nada de `__DATE__`/`__TIME__`, timestamps ni aleatoriedad dentro de `rockbox.zip`.
El release llega al iPod cuando Aura-Studio actualiza su pin (`FIRMWARE_VERSION`: `moonlit.tag=` + 4 hashes `moonlit.*`), no desde aquí.
