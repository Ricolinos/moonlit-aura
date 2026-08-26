# moonlit.aura — Plan correctivo (fase 4)

**Fecha:** 2026-08-25 · **Estado:** ACTIVO (sin encabezado `ESTADO:` = pendiente de ejecución). Sustituye a `03-plan-implementacion.md` para todo lo posterior a H1. Diagnóstico de partida: `04-auditoria-brecha.md`. Entradas vinculantes: `00-decisiones-moonlit.md` (DM-001…DM-017), `moonlit-aura/DECISIONS.md` (D-001…D-026) y las decisiones D-027…D-031 fijadas en §II.0.

Convención: rutas sin prefijo = `moonlit-aura/`; `AF/` = `Aura-Firmware/`; `apps/metro/` abrevia `firmware/rockbox/apps/metro/`. CONCLUSIÓN / HIPÓTESIS como en `04-auditoria-brecha.md`.

---


## II.0 Reglas del plan

- Se ejecuta en `moonlit-aura/` en sesiones nuevas de Sonnet, una por hito, sin acceso a esta conversación. Ninguna sesión escribe en `Aura-Firmware/`, `Metro-Aura/` ni `Aura-Studio/`.
- Cada hito: ≤ 1 sesión; un entregable; **definición de hecho = comandos con salida esperada**; commits atómicos **sin push**; `DECISIONS.md` recibe una D-NNN por decisión nueva antes de codificar.
- Decisiones ya cerradas que este plan hereda sin volver a preguntar: DM-001…DM-017 (= D-001…D-017), D-018…D-026 de `moonlit-aura/DECISIONS.md`, PA-1(a) PA-2(a) PA-3(a) PA-4(a) PA-6(b) PA-7(a) PA-8(a) del plan 03.
- **Decisiones nuevas cerradas en esta sesión (registrar como D-027…D-030 en el hito M1):**
  - **D-027 Dos esquemas MD3 desde el primer hito**: `night` (oscuro, predeterminado) y `dawn` (claro). Mapean al ajuste `theme` existente (`metro_theme.h:49-50`, `metro_settings.c:133`). Sustituye PA-5.
  - **D-028 Vocabulario MD3 obligatorio en tokens**: roles `primary`, `on_primary`, `primary_container`, `on_primary_container`, `surface`, `surface_dim`, `surface_bright`, `surface_container_lowest`, `surface_container_low`, `surface_container`, `surface_container_high`, `surface_container_highest`, `on_surface`, `on_surface_variant`, `outline`, `outline_variant`. El acento dinámico de Metro **es** `primary` (los presets sustituyen a los 10 acentos WP7). La escala `bg/surface_0..2` del plan 03 B.1 se descarta. Elevación tonal = niveles `surface_container_*` (MD3), más el par luz/sombra de DM-012.
  - **D-029 Marea convive con Álbumes**: pivote nuevo en `music_page` (`metro_screen_hub.c:606-623`); la rejilla se conserva.
  - **D-030 Layout de Marea**: columna de portadas **a la izquierda** (x ∈ [0,152)), información del álbum **a la derecha** (x ∈ [160,320)): título en `MFONT_HEADLINE`, artista en `MFONT_BODY`, "N canciones" en `MFONT_LABEL`. Sustituye la geometría centrada de plan 03 D.1.
  - **D-031 Nombres de archivo de fuente por rol**: `moonlit-<rol>-<px>.fnt` (el centinela `moonlit-body-18.fnt` ya está en el contrato v1 §A.8 y en `PROMPT-aura-studio.md`; cambiarlo rompería el contrato). El nombre de familia va en la cabecera del `.fnt` y en `tokens.json`, no en el nombre de archivo.

## II.1 Orden y dependencias

```
M1 tokens MD3 + generate.py (header)      ─┐
M2 fuentes: vendor + .fnt + moonlit_fonts  ─┼─▶ M4 paleta+elevación+lista/hub/ajustes ─▶ M5 Ahora suena/lock/usb/splash
M3 iconos Material Symbols (tabla C)      ─┘                                              │
M6 moonlit_flow vertical + test host (motor puro, sin UI) ──▶ M7 moonlit_art .pfraw + precarga ──▶ M8 pantalla Marea
M9 logotipo Waning Crescent                                                                     │
M10 DECISIONS/docs/design/skill al día  ◀──────────────────────────────────────────────────────┘
M11 revisión adversarial global (re-ejecuta todas las definiciones de hecho)
M12 [hardware, usuario] medición de Marea + empaquetado (= H7 del plan 03, se conserva)
```
M1, M2, M3 y M6 son independientes entre sí (pueden ir en cualquier orden o en paralelo por sesiones distintas). M4 requiere M1+M2+M3. M8 requiere M4+M6+M7. M9 requiere M3 (mismo pipeline de máscaras).

## II.2 Hitos

Formato de cada hito: **Parte de** (ruta:línea que se copia/lee) · **Crea/Modifica** · **NO tocar** · **Definición de hecho** (comandos, desde la raíz de `moonlit-aura/`) · **Commits**.

### M1 — Tokens MD3 y generador JSON→C

**Parte de:** `AF/design-system/generate.py:124` (`generate_header`), `:197-262` (`generate_aura_ds_defines`: aplanado dict→`#define`, `"#RRGGBB"`→`LCD_RGBPACK(r,g,b)` + `_RGB24`), `:37` (entrada `tokens.json`). Esquema de `AF/design-system/tokens.json:1-104` solo como forma. **No** copiar `generate_fonts`, `generate_icons`, `render_symbol_shapes` (AppKit), `--swift-out`, `theme_format_json`, `panel_backgrounds`, `tile_icons`: se escribe `generate.py` nuevo de ≤ 250 líneas con solo el header (fuentes e iconos llegan en M2/M3 como subcomandos).

**Crea:** `design-system/tokens.json` (D-027, D-028: `screen{320,240,dpi 160}`, `spacing{4,8,16,24,32}`, `shape{corner_none 0, corner_xs 4, corner_s 8, corner_m 12, corner_full 999}`, `color.night{16 roles MD3}`, `color.dawn{16 roles}`, `color.primary_presets{moonstone,tide,ember,moss}` con `primary/on_primary/primary_container/on_primary_container` por preset y esquema, `elevation{light_edge_delta 12, shadow_edge_delta -10, edge_px 1}`, `type_scale{}` (vacío, M2 lo llena), `motion{transition_ms 220, ease "out_expo"}`); `design-system/generate.py` (`--header` → `apps/metro/moonlit_tokens.h`, commiteado PA-3(a); `--contrast` calcula contraste WCAG `on_surface`/`surface` y `on_surface_variant`/`surface` por esquema y falla si < 4,5:1); `design-system/README.md` (10 líneas: cómo correr); `design-system/.venv/` gitignorado; `.gitignore` (+`design-system/.venv/`, `design-system/out/`); `apps/metro/moonlit_tokens.h` generado (**solo `#define`, incluido únicamente por `moonlit_palette.c` a partir de M4** — en M1 nadie lo incluye aún; sí se compila en un test host `test/test_tokens.c` que verifica que cada rol tiene su `_RGB24` y que `light_edge` > base > `shadow_edge` por canal); `DECISIONS.md` D-027…D-031.

**NO tocar:** nada bajo `apps/metro/*.c` existente, `metro_palette.h`, fuentes, iconos.

**Definición de hecho:**
```
test -f design-system/tokens.json && test -f design-system/generate.py && test -f apps/metro/moonlit_tokens.h
python3 -c "import json;t=json.load(open('design-system/tokens.json'));print(sorted(t['color']['night']))" \
  → imprime los 16 roles de D-028, y lo mismo para 'dawn'
grep -c 'SURFACE_CONTAINER' apps/metro/moonlit_tokens.h        → ≥ 10  (5 niveles × 2 esquemas)
grep -c 'MOONLIT_NIGHT_\|MOONLIT_DAWN_' apps/metro/moonlit_tokens.h → ≥ 32
grep -c 'EDGE_LIGHT\|EDGE_SHADOW' apps/metro/moonlit_tokens.h    → ≥ 20  (5 niveles × 2 bordes × 2 esquemas)
design-system/.venv/bin/python3 design-system/generate.py --header && git diff --exit-code apps/metro/moonlit_tokens.h → 0 (determinista)
design-system/.venv/bin/python3 design-system/generate.py --contrast → tabla con todos los pares ≥ 4.5 y salida 0
grep -rn '0x[0-9A-Fa-f]\{6\}\|LCD_RGBPACK(' apps/metro/*.c apps/metro/*.h | grep -v 'moonlit_tokens.h\|metro_palette.h\|metro_fb.c:104\|nowplaying.c:548\|metro_music.c' → vacío
make -C apps/metro/test test → todo verde incluido test_tokens
grep -c 'D-02[7-9]\|D-03[01]' DECISIONS.md → ≥ 5
```
**Commits:** `feat(design-system): tokens.json MD3 dual scheme + generate.py --header (D-027, D-028)`, `docs: D-027..D-031`.

### M2 — Pipeline de fuentes: vendor OFL, `.fnt` y `moonlit_fonts`

**Parte de:** `firmware/tools/gen_fonts.sh:20-25,64-65` (compilación y comando de `convttf`); `apps/metro/metro_fonts.c:40-83` y `metro_fonts.h:30-37` (tabla rol→archivo, `font_load_ex`, fallback); `metro_screen_specimen.c:21-30` (especímen).

**Crea:** `design-system/vendor/libre-baskerville/{LibreBaskerville-Regular.ttf, OFL.txt}` (github.com/impallari/Libre-Baskerville, OFL 1.1); `design-system/vendor/montserrat/{Montserrat-Regular.ttf, Montserrat-Medium.ttf, Montserrat-SemiBold.ttf, OFL.txt}` (**build estática** de github.com/JulietaUla/Montserrat `fonts/ttf/`, DM-004 — nunca la variable); `tokens.json:type_scale` con los **7 roles** (PA-6(b) elimina ACCENT): `display` LibreBaskerville-Regular 40 `-c 1` · `title` LibreBaskerville-Regular 28 `-c 1` · `headline` LibreBaskerville-Regular 22 · `list` Montserrat-Regular 20 · `list_sel` Montserrat-SemiBold 20 · `body` Montserrat-Regular 18 · `label` Montserrat-Medium 18 — escala MD3 display/headline/title/body/label (`list`/`list_sel` son "body-large" en MD3; el nombre se conserva por compatibilidad con los 19 sitios de llamada); `generate.py --fonts` que ejecuta por rol `firmware/rockbox/tools/convttf -p <px> -s 32 -l 383 -D 63 -c <sep> -o firmware/assets/fonts/moonlit-<rol>-<px>.fnt <ttf>` (**decimal**, D-007) y luego lee la cabecera RB12 del `.fnt` producido y **falla si `firstchar != 32` o `size != 352`**; `firmware/tools/gen_fonts.sh` reescrito como wrapper de `generate.py --fonts`; `apps/metro/moonlit_fonts.c/.h` (enum `MFONT_DISPLAY, MFONT_TITLE, MFONT_HEADLINE, MFONT_LIST, MFONT_LIST_SEL, MFONT_BODY, MFONT_LABEL, MFONT_COUNT`; **`#define MFONT_CAPTION MFONT_BODY`** temporal para que los 19 sitios compilen sin tocarlos — se retira en M4); `apps/SOURCES` (`metro/moonlit_fonts.c` en lugar de `metro/metro_fonts.c`); `metro_screen_specimen.c` (una línea por rol, con la cadena "Hll rn ÁÉÑ ¿? — pantalla"); `firmware/tools/check_fonts.py` (lee cabecera RB12 de un `.fnt` e imprime `firstchar/size/height/maxwidth`; `--capheight <png> --rows` mide con PIL, umbral > 60/255 DM-006, la altura en píxeles de la "H" en cada fila del especímen); `package_dist.sh:145-161` (centinelas → `moonlit-body-18.fnt`, `moonlit-list-20.fnt`) y `:200` (OFL de las dos familias); `build_sim.sh` sin cambio (copia `*.fnt`).
**Elimina:** `firmware/assets/fonts/metro-*.fnt`, `firmware/assets/fonts-src/` (Selawik, M-020 "cero Microsoft"), `apps/metro/metro_fonts.c/.h`.

**NO tocar:** `convttf.c` (PA-8: solo si `check_fonts.py --capheight` muestra headline-22 < 10,5 px, y entonces registrar D-0xx + `MODIFICATIONS.md`), `metro_draw.c`, pantallas.

**Definición de hecho:**
```
ls firmware/assets/fonts/ → exactamente: moonlit-display-40.fnt moonlit-title-28.fnt moonlit-headline-22.fnt moonlit-list-20.fnt moonlit-listsel-20.fnt moonlit-body-18.fnt moonlit-label-18.fnt
ls design-system/vendor/libre-baskerville/OFL.txt design-system/vendor/montserrat/OFL.txt → ambos existen
head -c 200 design-system/vendor/montserrat/Montserrat-Regular.ttf | grep -c fvar → 0 (estática: sin tabla fvar; verificar también con python3 fontTools si está)
for f in firmware/assets/fonts/*.fnt; do python3 firmware/tools/check_fonts.py $f; done → cada uno firstchar=32 size=352
stat -f %z firmware/assets/fonts/*.fnt → todos < 60000 salvo display-40 (registrar tamaño real; HIPÓTESIS C.1 cerrada en D-032)
grep -n '0x' firmware/tools/gen_fonts.sh → vacío
grep -rn 'Selawik\|fonts-src' --include='*.sh' --include='*.c' --include='*.h' --include='*.py' . | grep -v build- → vacío
firmware/tools/build_sim.sh && firmware/tools/sim_shot.sh docs/screenshots/M2-specimen.png 200 "<botones al especímen, documentados en el commit>"
python3 firmware/tools/check_fonts.py --capheight docs/screenshots/M2-specimen.png → 7 filas, todas ≥ 10.5 px
ls firmware/build-sim/simdisk/.rockbox/fonts/ | grep -c moonlit- → 7
grep -c 'moonlit-body-18.fnt' firmware/tools/package_dist.sh → ≥ 1
make -C apps/metro/test test → verde
```
**Commits:** `feat: vendor Libre Baskerville + Montserrat static (OFL) (D-004)`, `feat: generate.py --fonts, decimal charset 32-383, check_fonts.py (D-005, D-007)`, `feat: moonlit_fonts 7 MD3 roles, remove Selawik`.

### M3 — Iconos Material Symbols compilados con verificación de tonos

**Parte de:** `AF/design-system/generate.py:372-391` (supersampleo 16× + filtro de caja), `:475` (`MIN_INK_TONES = 4`), `:575-583,626-632` (conteo de tonos y `die()`); patrón de tabla C 8-bit de Metro `apps/metro/metro_glyphs_table.c` (M-089) y consumidor `metro_widgets_draw_glyph` (`metro_widgets.h:94`); tabla actual `metro_icons_table.c` / `metro_icons.h` (Fluent) y su generador `firmware/tools/gen_icons.py`.

**Crea:** `design-system/vendor/material-symbols/{20 svg de plan 03 B.4}` + `LICENSE` (Apache 2.0, de github.com/google/material-design-icons, variante Rounded, peso 400); `generate.py --icons`: `rsvg-convert -w 16·S` → PIL box 16× → máscara 8-bit → cuenta tonos > 0 y falla si < 4 → escribe `apps/metro/moonlit_icons_table.c` + `moonlit_icons.h` (`enum moonlit_icon`, `struct moonlit_icon_mask {w,h,const uint8_t *cov}`, tamaños 16/24/40); `firmware/tools/check_tones.py` (misma función de conteo sobre PNG/BMP arbitrario, `--region x,y,w,h --min 4`); `apps/metro/moonlit_icons.c` con `moonlit_icon_draw(id, size, x, y, color)` implementado sobre `metro_fb_plot_alpha()` (`metro_fb.c:116`) — mismo cuerpo que `metro_widgets_draw_glyph`; `tokens.json:icon{family,license,sizes,names}`. **Sustituye** los consumidores de `metro_icons.h` (grep `METRO_ICON_` en `apps/metro/`) por el enum nuevo con un mapa nombre→nombre en el commit.
**Elimina:** `firmware/assets/icons/`, `LICENSE-fluent-system-icons.txt`, `gen_icons.py`, `metro_icons_table.c`, `metro_icons.h`.

**NO tocar:** paleta, fuentes, `metro_glyphs_table.c` (glifo USB M-089 se conserva hasta M5).

**Definición de hecho:**
```
ls design-system/vendor/material-symbols/*.svg | wc -l → 20 ; test -f design-system/vendor/material-symbols/LICENSE
design-system/.venv/bin/python3 design-system/generate.py --icons → tabla 20×3 con tonos ≥ 4 cada uno, salida 0; guardar en docs/screenshots/M3-icon-tones.txt
grep -c 'static const uint8_t' apps/metro/moonlit_icons_table.c → 60
grep -rn 'METRO_ICON_\|metro_icons.h\|Fluent' apps/metro/ firmware/tools/ → vacío
firmware/tools/build_sim.sh && firmware/tools/sim_shot.sh docs/screenshots/M3-hub.png 150
python3 firmware/tools/check_tones.py docs/screenshots/M3-hub.png --region <x,y,w,h de un ícono del hub> --min 4 → OK
grep -rn 'read(\|open(' apps/metro/moonlit_icons.c → vacío (cero disco en runtime, DM-008)
```
**Commits:** `feat: vendor Material Symbols Rounded (Apache-2.0) (D-008)`, `feat: generate.py --icons + compiled 8-bit masks + check_tones.py`, `chore: remove Fluent icons`.

### M4 — Paleta MD3, elevación tonal y pantallas base (hub, lista, ajustes, barra de estado)

**Parte de:** `metro_theme.c:23-34,61-99` (tabla de acentos, setters, resolvers) y `metro_theme.h:35-50`; `metro_widgets.c:195-223` (anillo antialias: la técnica de cobertura para esquinas redondeadas); `metro_fb.c:98-127`; `metro_screen_list.c` (fila seleccionada; grep `metro_color_accent` para los puntos de tinta); `metro_screen_hub.c`; `metro_screen_settings.c`; barra de estado (grep `statusbar\|status_bar` en `apps/metro/`; HIPÓTESIS: vive en `metro_draw.c:120-170`, confirmar); `metro_transitions.c` (duración desde tokens).

**Crea:** `apps/metro/moonlit_palette.c/.h` — único includer de `moonlit_tokens.h`; API: `moonlit_color(enum moonlit_role)` (16 roles D-028, resuelve por `metro_settings.theme` night/dawn), `moonlit_color_accent()` = `moonlit_color(MROLE_PRIMARY)` (conserva el nombre `metro_color_accent` como `#define` de compatibilidad hasta que M11 lo retire), `moonlit_surface(level 0..4, edge MEDGE_NONE|LIGHT|SHADOW)`; presets de `primary` (moonstone/tide/ember/moss) reemplazan `enum metro_accent` (`metro_theme.h:35-47`) manteniendo el ajuste `accent` de `aura.cfg` con los nombres nuevos; `apps/metro/moonlit_elevation.c/.h` — `moonlit_draw_surface(x,y,w,h,level,radius)`: `lcd_fillrect` por filas + esquinas por cobertura (`metro_fb_plot_alpha`, radio de `tokens.shape`) + borde izquierdo/superior `EDGE_LIGHT` y derecho/inferior `EDGE_SHADOW` (DM-012), colores precalculados en `moonlit_tokens.h`, cero aritmética por cuadro; capa de estado de foco/selección = fila seleccionada sobre `surface_container_high` con texto `on_surface` y marcador de 3 px en `primary` (MD3 "state layer" sin alfa: se resuelve por token); divisores de lista = `lcd_hline` en `outline_variant` (DM-011 condicionado: **citar la línea** de `metro_screen_list.c` donde se dibuja); barra de estado 20 px sobre `surface_container_lowest`; `MFONT_CAPTION` → `MFONT_BODY`/`MFONT_LABEL` en los 19 sitios (retirar el `#define` temporal de M2); `docs/moonlit-design-system/00-INDICE.md` + `sistema/{01-color,02-tipografia,03-forma,04-elevacion,05-movimiento}.md` (breves, con los valores de `tokens.json`); `sim_matrix.sh:41-54` adaptado a `theme ∈ {night,dawn}` × `accent ∈ {4 presets}`; `firmware/tools/check_tones.py --edge <png> --row y --x0 x --x1 x` (verifica píxel borde izq. más claro que interior y derecho más oscuro).
**Elimina:** `apps/metro/metro_palette.h` (y su include en `mpegplayer.c:113` → `moonlit_palette.h`, actualizar `MODIFICATIONS.md`).

**NO tocar:** `metro_screen_nowplaying.c`, `metro_screen_lock.c`, `metro_screen_usb.c`, `metro_screen_splash.c` (M5); Marea; logo.

**Definición de hecho:**
```
test -f apps/metro/moonlit_palette.c && test -f apps/metro/moonlit_elevation.c && ! test -e apps/metro/metro_palette.h
grep -rln 'moonlit_tokens.h' apps/metro/ firmware/rockbox/apps/plugins/ → solo apps/metro/moonlit_palette.c
grep -rn '0x[0-9A-Fa-f]\{6\}\|LCD_RGBPACK(' apps/metro/*.c apps/metro/*.h | grep -v 'moonlit_tokens.h\|metro_fb.c:104\|nowplaying.c:548\|metro_music.c' → vacío
grep -c 'surface_container' design-system/tokens.json → ≥ 10 ; grep -c 'SURFACE_CONTAINER' apps/metro/moonlit_palette.c → ≥ 5
grep -rn 'MFONT_CAPTION' apps/metro/ → vacío
grep -n 'lcd_hline' apps/metro/metro_screen_list.c → ≥ 1 línea (divisor, citada en DECISIONS D-0xx)
firmware/tools/build_sim.sh 2>&1 | grep -c 'warning:' → 0 nuevos respecto a la base (anotar cifra base en el commit)
firmware/tools/sim_matrix.sh docs/screenshots/M4-matrix → 3 pantallas × 2 temas × 4 presets = 24 PNG de 320×240
python3 firmware/tools/check_tones.py --edge docs/screenshots/M4-matrix/list-night-moonstone.png --row <y fila sel> → "left lighter, right darker: OK" (y lo mismo para dawn)
make -C apps/metro/test test → verde ; firmware/tools/build_target.sh → rockbox.ipod producido (con RBDEV_TOOLCHAIN)
```
**Commits:** `feat: moonlit_palette MD3 roles, dual scheme, primary presets (D-010, D-027, D-028)`, `feat: moonlit_elevation tonal surfaces with left light (D-012)`, `feat: hub/list/settings/statusbar on MD3 tokens (D-011)`, `chore: remove metro_palette.h`, `docs: moonlit-design-system sistema/*`.

### M5 — Ahora suena, candado, USB y splash sobre el sistema nuevo

**Parte de:** `metro_screen_nowplaying.c:438` (puerta `lcd_active`), `:619` (fuente), `:631` (HIPÓTESIS del plan 03: blend sobre carátula — confirmar con grep `metro_fb_blend_over_color` y `metro_albumart_background_bitmap`); `metro_screen_lock.c:172`; `metro_screen_usb.c:144`; `metro_screen_splash.c:34-71`.

**Modifica:** fondo del reproductor = `surface` plano (DM-013: retirar `metro_albumart_load_background*` de la ruta de dibujo; la función puede quedar sin llamar hasta M11); carátula 120×120 con esquinas `corner_s` sobre `moonlit_draw_surface(level 2)`; título `MFONT_TITLE`, artista `MFONT_BODY` en `on_surface_variant`, tiempos `MFONT_LABEL`; controles con `moonlit_icon_draw` (play_arrow/pause/skip_*); barra de progreso en `primary` sobre `surface_container_highest`, redibujo animado solo bajo `lcd_active()` (`:438` se conserva); lock/usb/splash: colores por rol, glifo USB (`metro_glyphs_table.c`) → ícono `usb` 40 px de M3 y `metro_glyphs_table.c` se elimina; `docs/moonlit-design-system/componentes/{ahora-suena,candado,usb}.md`.

**NO tocar:** hub/lista/ajustes (M4), Marea, logo (splash solo cambia colores/tipografía; el logo llega en M9).

**Definición de hecho:**
```
grep -n 'metro_albumart_load_background\|metro_albumart_background_bitmap' apps/metro/metro_screen_nowplaying.c → vacío
grep -n 'lcd_active' apps/metro/metro_screen_nowplaying.c → ≥ 1 (progreso gateado)
grep -rn 'metro_glyphs' apps/metro/ → vacío ; ! test -e apps/metro/metro_glyphs_table.c
firmware/tools/build_sim.sh && firmware/tools/gen_test_media.sh (si el simdisk no tiene medios)
firmware/tools/sim_shot.sh docs/screenshots/M5-nowplaying-night.png 300 "<botones hasta reproducir>" ; ídem -dawn tras editar theme en simdisk/.rockbox/aura/aura.cfg ; M5-lock.png ; M5-usb.png (METRO_SIM_FORCE_USB=1)
python3 firmware/tools/check_tones.py docs/screenshots/M5-nowplaying-night.png --region <x,y,w,h del ícono play> --min 4 → OK
python3 - <<'EOF'  → el píxel (4,120) del fondo del reproductor == color surface del esquema night en tokens.json (RGB565 cuantizado ±8)
EOF
make -C apps/metro/test test → verde
```
**Commits:** `feat: now playing tonal surface + MD3 type + icons (D-013)`, `feat: lock/usb/splash on MD3 tokens`, `chore: remove metro_glyphs_table`.

### M6 — Motor `moonlit_flow` vertical + pruebas en host (sin UI)

**Parte de:** `AF/firmware/rockbox/apps/aura/aura_flow.c` (229 líneas, cero dependencias) y `aura_flow.h` (136) tomados de `aura-upstream/main` con `git show aura-upstream/main:firmware/rockbox/apps/aura/aura_flow.c > apps/metro/moonlit_flow.c`; `AF/.../test/test_flow.c` (211) → `apps/metro/test/test_flow.c`; `AF/.../aura_wheel.{c,h}` (51/58, cero dependencias) → `apps/metro/moonlit_wheel.{c,h}` (D-019).

**Modifica (en la copia, documentando cada cambio en la cabecera del archivo con "moonlit: derived from aura_flow.c @ aura-upstream <sha>"):** prefijo `AURA_FLOW_`→`MOONLIT_FLOW_`, `aura_flow_`→`moonlit_flow_`; `AURA_FLOW_SCREEN_W 320` (`aura_flow.h:72`) → `MOONLIT_FLOW_AXIS_LEN 220` (240 − barra 20); `DISPLAY_LEFT_R`/`MAXSLIDE_LEFT_R` (`:77-78`) → `_TOP_R`; `proj->screen_x` (`:109`) → `screen_y`; corte en `aura_flow.c:192,207` contra `AXIS_LEN`; `aura_flow_source_column()` → `moonlit_flow_source_row()`; `aura_flow_vertical_scale()` → `moonlit_flow_cross_scale()`; `DISPLAY_W 128` (`:74`) → tapa central 120; `CAM_DIST 240` (`:75`) se conserva y se anota como HIPÓTESIS a retunear en M8. `test_flow.c`: los 7 casos adaptados al eje (el caso `test_realistic_side_slide_layout` `:160` reescrito con 5 tapas en 220 px); `test/Makefile:11` añade `test_flow` y `test_wheel`; `apps/SOURCES` añade `metro/moonlit_flow.c`, `metro/moonlit_wheel.c`. `moonlit_wheel_step()` recibe velocidad de `get_action_data()` (`metro_input.c:36` ya la lee bajo `HAVE_WHEEL_ACCELERATION`): añadir `metro_input_last_wheel_velocity()` en `metro_input.c` que guarde `get_action_data()` del último `MACT_PREV/NEXT` (HIPÓTESIS: unidades de `button_get_data()` en ipod6g son grados/s como en Aura `aura_main.c:62-74`; verificar en `firmware/target/arm/s5l8702/ipod6g/button-6g.c` y anotar en D-0xx).

**NO tocar:** ninguna pantalla; `metro_keymap.c`; `.pfraw` (M7).

**Definición de hecho:**
```
test -f apps/metro/moonlit_flow.c && test -f apps/metro/moonlit_flow.h && test -f apps/metro/moonlit_wheel.c && test -f apps/metro/test/test_flow.c
head -30 apps/metro/moonlit_flow.c | grep -c 'aura_flow.c @ aura-upstream' → 1 (trazabilidad)
diff <(git show aura-upstream/main:firmware/rockbox/apps/aura/aura_flow.c | sed 's/aura_flow/moonlit_flow/g;s/AURA_FLOW/MOONLIT_FLOW/g') apps/metro/moonlit_flow.c | grep -c '^[<>]' → < 60 líneas (solo el cambio de eje; anotar la cifra)
grep -c 'screen_x\|SCREEN_W' apps/metro/moonlit_flow.[ch] → 0
grep -c 'screen_y\|AXIS_LEN' apps/metro/moonlit_flow.[ch] → ≥ 6
make -C apps/metro/test test → verde, con líneas "test_flow: 7 passed" y "test_wheel: N passed"
firmware/tools/build_sim.sh → compila (símbolos enlazados aunque sin llamador)
```
**Commits:** `feat: moonlit_flow vertical projection from aura_flow@<sha> + host tests (D-014)`, `feat: moonlit_wheel from aura_wheel (D-019)`.

### M7 — Caché de portadas `moonlit_art` (.pfraw fila-contigua, 120 px) y precarga

**Parte de:** `AF/.../aura_art.c:91-125,141+` (cabecera `{size,radius,theme,extra}`, `read/write/is_cached`), `:170-206` (máscara de esquinas: sustituir `a26_shell_isqrt256`/`a26_shell_blend` por `metro_widgets` isqrt/`metro_fb_blend_color` `metro_fb.c:107`); D-020 ya fija las 5 funciones a copiar. Decodificación: **no** se porta `aura_albumart.c`; se usa `metro_albumart_decode_track_cover()` (`metro_albumart.h:94`) — HIPÓTESIS: decodifica a `METRO_TILE_SIZE 80`; M7 añade una variante `metro_albumart_decode_track_cover_sized(path, out, size)` o confirma que `read_jpeg_file(..., FORMAT_RESIZE)` acepta 120 (leer `metro_albumart.c` y `recorder/jpeg_load.h`). Resolución álbum→pista→ruta: `album_thumb_decode` (`metro_screen_hub.c:363-379`: `metro_music_songs_of_album` + `metro_music_track_path`). Precarga: patrón D-224 (`AF/aura_music.c:221-300`), pero en Metro se engancha a la pantalla "Actualizando biblioteca…" existente (`metro_sync.c`, HIPÓTESIS: función de progreso; localizar) o a la entrada a Marea con cápsula de espera (`metro_widgets`).

**Crea:** `apps/metro/moonlit_art.c/.h`: `moonlit_art_pfraw_path(seek, size, out)` bajo `metro_settings_metro_cache_dir()/art/` (= `/.rockbox/aura/moonlitcache/art/`, D-023); `moonlit_art_read_pfraw / write_pfraw / is_cached` con cabecera `{size=120, radius=8, layout=1 (fila-contigua), extra=theme}`; `moonlit_art_mask_corners(buf, size, radius, bg)` **sin transponer**; `moonlit_art_load_for_album(seek, fb_data *out120)` = hit `.pfraw` → `read()` plano; miss → decode → mask → write; `moonlit_art_precache(progress_cb)` recorre `metro_music_albums()` una vez por arranque de DB (`metro_music_db_ready()`), `yield()` por álbum; `apps/SOURCES`; `test/test_art.c` (host: write→read round-trip, rechazo de cabecera con size distinto, máscara de esquinas deja 4 píxeles de esquina == bg).

**NO tocar:** `metro_thumbs.c` (la rejilla de Álbumes sigue con `.mth` 80 px — dos cachés distintas, documentar en D-0xx), pantallas.

**Definición de hecho:**
```
test -f apps/metro/moonlit_art.c && grep -c 'aura_art.c @ aura-upstream' apps/metro/moonlit_art.c → 1
grep -n 'transpose' apps/metro/moonlit_art.[ch] → vacío (fila-contigua)
grep -n 'moonlitcache' apps/metro/moonlit_art.c → vacío (la ruta viene de metro_settings_metro_cache_dir(), CLAUDE.md regla de rutas)
make -C apps/metro/test test → verde incl. test_art
firmware/tools/build_sim.sh && firmware/tools/sim_shot.sh docs/screenshots/M7-precache.png 400 "<botones a Música>" (si la precarga muestra progreso)
ls firmware/build-sim/simdisk/.rockbox/aura/moonlitcache/art/ | grep -c '\.pfraw$' → == número de álbumes de gen_test_media.sh con carátula
stat -f %z firmware/build-sim/simdisk/.rockbox/aura/moonlitcache/art/*.pfraw | sort -u → un solo valor = 16 + 120*120*2 = 28816
segunda ejecución del sim con DEBUGF activo (build sim ya define DEBUG): grep -c 'moonlit_art: decode' log → 0 ; grep -c 'moonlit_art: hit' log → == álbumes
```
**Commits:** `feat: moonlit_art row-major pfraw cache from aura_art@<sha> (D-020)`, `feat: album art precache on db ready (D-224 pattern)`.

### M8 — Pantalla Marea (portadas a la izquierda, información a la derecha)

**Parte de:** `AF/aura_musicflow.c:605-713` (`draw_slide_perspective`, se reescribe en espejo), `:448-494` (`get_slot_for`, LRU por distancia), `:1238-1259` (`scroll_step`), `:316-321` (`anim_pos_x256`), `:1321-1362` (despacho IDLE); patrón de pantalla completa de Metro `metro_screen_photo_viewer.{h,c}` (`push/is_current/show/handle`) y sus 4 enganches en `metro_main.c:92-100,305-312,453-461,466-467`; contexto de rueda `metro_keymap.c:41-45` (LIST: PREV/NEXT con REPEAT); easing `metro_motion.c:25-27,58`.

**Crea:** `apps/metro/moonlit_screen_marea.c/.h`: `marea_slot_t {album_index; fb_data cover[120*120]; bool has_art; char initial;}` × `MAREA_CACHE_SLOTS = 2*(2+15)+3 = 37` estático (37 × 28 800 = 1 065 600 B, D.3 del plan 03; **límite de hecho: `.bss` crece ≤ 1,1 MB**); geometría D-030: eje vertical x-centro de columna = 76, tapa central 120 px en y=[70,190), 2 tapas por lado escaladas por `moonlit_flow_cross_scale`; `draw_slide_vertical()` itera `screen_y`, obtiene `row = moonlit_flow_source_row()`, escala horizontal y **`lcd_bitmap(row_buf, x0, screen_y, n_cols, 1)`**; fade lateral por LUT hacia `surface` (copiar `build_fade_lut` `:513-518`); panel derecho x∈[160,320): `moonlit_draw_surface(160, 20, 152, 220, level 1, corner_m)` + título `MFONT_HEADLINE` (cortado con `metro_draw_text_cut_right`), artista `MFONT_BODY` `on_surface_variant`, "N canciones" `MFONT_LABEL`; sin portada → monograma: `moonlit_draw_surface(level 2)` 120×120 + inicial `MFONT_HEADLINE` en `primary` (D.5); rueda: `MACT_NEXT` → `scroll_step(+1 × moonlit_wheel_step(vel))` = la columna sube; `MACT_SELECT` → `metro_screen_list_push(album_songs_page)` (reutilizar `album_songs_pivots` `metro_screen_hub.c:655`); `MACT_PLAYPAUSE` → `metro_music_play_songs_of_album(seek, 0)`; `MACT_BACK` → pop; animación 220 ms `METRO_EASE_OUT_EXPO` **solo si `lcd_active() && metro_settings.animations != METRO_ANIM_OFF`** (patrón `metro_screen_hub.c:818`), si no salto directo; nunca decode dentro de `show()`: `get_slot_for` solo lee `.pfraw` (M7) y si falta usa monograma y encola. Entrada: pivote "Marea" en `music_page` (`metro_screen_hub.c:606-623`, D-029) con `on_select` → `moonlit_screen_marea_push()`; `metro_main.c` añade `at_marea` en los 4 enganches (contexto `MCTX_LIST` reutilizado — no se toca `metro_keymap.c`); strings `LANG_MAREA_*` ya existentes (`metro_lang.h:181-182`) + `LANG_MAREA_SONGS_FMT`; `DECISIONS.md` D-0xx "Marea experimental hasta medición en hardware (M12)"; `docs/moonlit-design-system/componentes/marea.md`.

**NO tocar:** `metro_thumbs.c`, rejilla de Álbumes, `metro_keymap.c`, `moonlit_flow.c` (si el retune de `CAM_DIST` lo exige, cambiar solo constantes en `.h` y registrar).

**Definición de hecho:**
```
test -f apps/metro/moonlit_screen_marea.c ; grep -c 'lcd_bitmap(' apps/metro/moonlit_screen_marea.c → ≥ 1 y ninguna con ", 1, " como ancho (grep -c ', 1, n_rows' → 0)
grep -n 'lcd_active' apps/metro/moonlit_screen_marea.c → ≥ 1 ; grep -n 'read_jpeg\|decode_track_cover' apps/metro/moonlit_screen_marea.c → vacío
grep -n 'LANG_MAREA' apps/metro/metro_screen_hub.c → ≥ 1 (pivote)
firmware/tools/build_target.sh ; arm-none-eabi-size firmware/build-ipod6g/rockbox.elf (o el size del toolchain) → bss ≤ base + 1 100 000 B (anotar base y resultado)
firmware/tools/build_sim.sh
firmware/tools/sim_shot.sh docs/screenshots/M8-marea-0.png 400 "<hub→Música→pivote Marea>"      → 320×240, portada central en x<152, texto en x≥160
firmware/tools/sim_shot.sh docs/screenshots/M8-marea-1.png 400 "<…>,SCROLL_FWD,WAIT"           → el título del panel derecho cambió respecto a -0 (verificar con `cmp` de la región 160..320 entre ambos PNG → distinto) y la región 0..152 también cambió
firmware/tools/sim_shot.sh docs/screenshots/M8-marea-mono.png 400 "<… hasta el álbum SinArte>"  → monograma (check_tones --region de la tapa central: ≥ 4 tonos, y píxel central == color primary del esquema)
python3 - → en M8-marea-0.png, columna x=76: ≥ 3 bloques de píxeles ≠ surface separados (5 tapas visibles, D.3)
make -C apps/metro/test test → verde ; grep -c 'experimental' DECISIONS.md → ≥ 1 en la D de Marea
```
**Commits:** `feat: Marea screen -- vertical music flow, covers left / info right (D-014, D-029, D-030)`, `feat: Marea pivot in Music page`.

### M9 — Logotipo Waning Crescent y wordmark

**Parte de:** plan 03 §E.1–E.3 íntegro (dos círculos: A c(50,50) r36, B c(62,46) r32; forma A−B; wordmark "moonlit" en Libre Baskerville a contornos); pipeline de M3 (`generate.py --icons` → reutilizar para `--logo`); consumidores actuales `metro_screen_splash.c:34-71`, `metro_screen_usb.c:29`, `metro_screen_about.c`, `firmware/tools/gen_logo.py` (M-092, se elimina), `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` (tocado en `49973dc1`: restaurar el de Rockbox o dejar uno neutro, registrar en `MODIFICATIONS.md`).

**Crea:** `design-system/logo/moonlit-crescent.svg`, `moonlit-wordmark.svg`; `generate.py --logo` → `apps/metro/moonlit_logo_table.c` (máscaras 16/24/40/64 + wordmark 140×28) con chequeos E.3 (tonos ≥ 4; ≥ 3 píxeles ≥ 200 en la columna más ancha a 16 px; sin píxel aislado < 60 sin vecino ≥ 60); splash = creciente 64 + wordmark; Acerca de = ídem; hub = 40 px; barra de estado = 16 px; USB = 40 px; `docs/screenshots/M9-logo-tones.txt`.
**Elimina:** `gen_logo.py`, wordmark provisional D-026 (registrar D-0xx que lo sustituye).

**Definición de hecho:**
```
test -f design-system/logo/moonlit-crescent.svg && grep -c '<circle' design-system/logo/moonlit-crescent.svg → 2 (o 1 path con mask; anotar)
design-system/.venv/bin/python3 design-system/generate.py --logo | tee docs/screenshots/M9-logo-tones.txt → 5 máscaras, todas "tones>=4 OK", cobertura 16px OK, cúspides OK, salida 0
! test -e firmware/tools/gen_logo.py ; grep -rn 'metro / aura\|gen_logo' apps/metro/ firmware/tools/ → vacío
firmware/tools/build_sim.sh && firmware/tools/sim_shot.sh docs/screenshots/M9-splash.png 30 && firmware/tools/sim_shot.sh docs/screenshots/M9-about.png 200 "<a Acerca de>"
python3 firmware/tools/check_tones.py docs/screenshots/M9-splash.png --region <x,y,64,64> --min 4 → OK
python3 - → en M9-splash.png, dentro de la región del creciente, la columna más a la izquierda con tinta está más iluminada (más píxeles == primary) que la más a la derecha (luz desde la izquierda, DM-012/DM-016)
```
**Commits:** `feat: Waning Crescent logo spec + generated masks (D-016)`, `feat: splash/about/hub/statusbar/usb use logo; remove provisional wordmark`.

### M10 — DECISIONS.md, docs/design y skill al día

**Modifica:** `DECISIONS.md` (verificar que cada D-004…D-016 tiene línea "Implementada en M<n>, commit <sha>"), `README.md` (sin rutas inexistentes; sección "Estado" real), `CLAUDE.md` (versión de II.3), `.claude/skills/moonlit-design-system/SKILL.md` (cuerpo real: tokens, roles, elevación, iconos, Marea, logo — ≤ 150 líneas, apunta a `docs/moonlit-design-system/`), `docs/moonlit-design-system/00-INDICE.md` completo, `docs/plan/03-plan-implementacion.md` → mover a `docs/plans/archivo/` con `ESTADO: SUPERADO por 05-plan-correctivo.md (H0–H1 ejecutados)`, `CONTRATO-moonlit-studio.md` (§A.8 centinela ahora real), `docs/COMPAT_STUDIO.md`, `MODIFICATIONS.md` (cada archivo fuera de `apps/metro/` tocado en M1–M9).

**Definición de hecho:**
```
for p in $(grep -oh '`[a-zA-Z0-9_./-]*\.\(md\|json\|py\|sh\|h\|c\|fnt\|txt\)`' CLAUDE.md README.md .claude/skills/moonlit-design-system/SKILL.md docs/moonlit-design-system/*.md | tr -d '`' | sort -u); do test -e "$p" || echo "MISSING $p"; done → sin líneas MISSING
grep -c 'Implementada en M' DECISIONS.md → ≥ 12
head -1 docs/plans/archivo/03-plan-implementacion.md | grep -c 'ESTADO:' → 1 ; ! test -e docs/plan/03-plan-implementacion.md
wc -l CLAUDE.md → ≤ 40
git diff --stat moonlit-fork-base..HEAD -- 'firmware/rockbox/*' ':!firmware/rockbox/apps/metro' | tail -1 → N archivos; grep -c '<cada uno>' MODIFICATIONS.md → ≥ 1 por archivo
```
**Commits:** `docs: DECISIONS/README/CLAUDE/skill/design-system in sync with M1..M9`.

### M11 — Revisión adversarial global

Sesión de solo lectura + correcciones mínimas. Re-ejecuta **todas** las definiciones de hecho de M1–M10 en orden y guarda la salida en `docs/screenshots/M11-verificacion.txt`. Luego, con subagentes independientes (uno por tema, sin ver el código del otro):
1. "Refuta que moonlit tiene un sistema de diseño propio": busca cualquier `#define METRO_*` de color, cualquier `metro_font_id(MFONT_CAPTION)`, cualquier string "metro" visible en UI (`grep -n '"[^"]*[Mm]etro[^"]*"' apps/metro/metro_lang.c` → solo el nombre de la familia hermana en textos de cambio de firmware, M-090).
2. "Refuta que Marea parte de Aura-Firmware": `diff` de `moonlit_flow.c` contra `aura_flow.c` (< 60 líneas de cambio) y de `moonlit_art.c` contra `aura_art.c`.
3. "Refuta la frontera GPL": `CONTRATO-moonlit-studio.md` §B `BOOT-1` coincide con `git log -1 -- firmware/rockbox/bootloader/ipod-s5l87xx.c firmware/rockbox/utils/mks5lboot` = `moonlit-fork-base` (ningún cambio).
4. "Refuta las restricciones vinculantes": `grep -rn 'lcd_active' apps/metro/*.c` cubre cada bucle con `current_tick` (lista los bucles sin puerta); texto en inglés en UI (`metro_lang.c` tabla es solo español + inglés como segundo idioma, verificar que el predeterminado es es); iconos verificados por conteo (M3-icon-tones.txt, M9-logo-tones.txt existen y sin FAIL).
Cada refutación que prospere se corrige en el mismo hito si es ≤ 20 líneas, o se registra como D-0xx pendiente.

**Definición de hecho:** `docs/screenshots/M11-verificacion.txt` existe, contiene una línea `OK M<n>` por hito (M1…M10) y ninguna `FAIL`; `git status --short` limpio; `make -C apps/metro/test test` verde; `build_target.sh` y `build_sim.sh` en 0.

### M12 — Hardware, empaquetado y cierre (= H7 del plan 03, sin cambios de fondo)

Usuario flashea; `DEBUGF("marea frame %ld ms")` 60 cuadros → promedio/máximo; criterio máx ≤ 33 ms; si falla, (a) 3 tapas visibles o (b) sin animación bajo FX reducidas → D-0xx. `package_dist.sh` → 7 assets; `unzip -l firmware/dist/rockbox.zip | grep -c 'fonts/moonlit-'` → 7; `grep -c 'SIL OPEN FONT LICENSE' firmware/dist/THIRD-PARTY-NOTICES.txt` → 2; `grep -c Apache` → 1; dos `package_dist.sh` consecutivos → `checksums.txt` idénticos. Sin tag, sin push.

## II.3 Borrador de `CLAUDE.md` para moonlit-aura (≤ 40 líneas; se instala en M10, y M1 puede adelantar las líneas de comandos que ya sean ciertas)

```markdown
# moonlit.aura

Firmware para iPod Classic 6G (S5L8702, 320×240 @ LCD_DPI 160, 64 MB, sin GPU/FPU). Fork de Metro-Aura
(tag `moonlit-fork-base` = 2f1bd28a) sobre Rockbox (M-001). Nombre visible: "moonlit.aura". Repo: `moonlit-aura`.

## Comandos (siempre desde la raíz del repo)
- Simulador:  `firmware/tools/build_sim.sh [--reconfigure] [--run]` → `firmware/build-sim/rockboxui` (brew: sdl2 gcc freetype librsvg)
- Captura:    `firmware/tools/sim_shot.sh <out.png> [ticks] "SELECT,RIGHT,SCROLL_FWD,WAIT,…"` (320×240, headless)
- Matriz:     `firmware/tools/sim_matrix.sh <dir>` (pantallas × tema × preset)
- Target:     `RBDEV_TOOLCHAIN=../Metro-Aura/firmware/toolchain/bin firmware/tools/build_target.sh` → `firmware/build-ipod6g/rockbox.ipod`
- Tokens/fuentes/iconos/logo: `design-system/.venv/bin/python3 design-system/generate.py --header|--fonts|--icons|--logo|--contrast`
  (venv: `python3 -m venv design-system/.venv && design-system/.venv/bin/pip install pillow`)
- Verificación mecánica: `firmware/tools/check_fonts.py <fnt|--capheight png>`, `firmware/tools/check_tones.py <png> [--region x,y,w,h] [--edge]`
- Tests host: `make -C firmware/rockbox/apps/metro/test test`
- Release:    `firmware/tools/package_dist.sh --release-tag vX.Y.Z` (árbol limpio; sin push desde sesiones de plan)

## Reglas que no se deducen del código
- Idiomas: `.md` en español de México sin voseo; código, comentarios y commits en inglés; UI en español (`metro_lang.c`).
- Toda decisión se cierra en `DECISIONS.md` (D-NNN) antes de codificar. D-001…D-031 vinculantes. Metro histórico: `DECISIONS-METRO-ARCHIVE.md` (solo lectura).
- Plan vigente: `docs/plan/05-plan-correctivo.md` (hitos M1–M12). `docs/plans/archivo/` es histórico, nunca pendiente.
- Colores: solo `design-system/tokens.json` (roles MD3, esquemas night/dawn). En C solo `moonlit_palette.c` incluye `moonlit_tokens.h`;
  todo el mundo llama `moonlit_color(rol)` / `moonlit_surface(nivel, borde)`. Cero literales RGB en `apps/metro/`.
- Elevación = tono (`surface_container_*`) + borde izq/sup claro y der/inf oscuro (luz desde la izquierda). Nunca blur, ripple ni sombras difusas.
- Animación solo bajo `lcd_active()` y `metro_settings.animations != METRO_ANIM_OFF`; ninguna lectura de disco dentro de un bucle de animación.
- Fuentes: 7 roles en `moonlit_fonts.h`; ningún rol < 18 px; `convttf` con rango decimal 32–383 (nunca `0x…`); builds TTF estáticas.
- Iconos/logo: tablas C generadas (nunca disco); verificación de tonos ≥ 4 en `generate.py`; jamás "a ojo".
- Prohibido desde `apps/metro/`: `root_menu()`, `do_menu()`, `gui_synclist`, `rockbox_browse()`, `kbd_input()`, skin engine.
- `struct viewport` local → `viewport_set_defaults()` (M-027). Texto siempre vía `metro_draw_text*()` (M-051).
- Rutas `.rockbox/aura/`, `/.aura/`, caché `moonlitcache/`: solo en `metro_settings.c`/`metro_sync.c`/`metro_device.c`/`metro_media_categories.c`.
  Contratos inmutables: `Aura-Firmware/CONTRATO-firmware-studio.md` v13 + `CONTRATO-moonlit-studio.md` v1.
- Cambios fuera de `apps/metro/` → `MODIFICATIONS.md` en la misma pasada + comentario `moonlit (D-NNN)`.
- Nunca escribir en `../Aura-Firmware`, `../Metro-Aura`, `../Aura-Studio`. Sin material de Apple ni Microsoft en el árbol.

## Diseño
Antes de tocar pantalla, animación, ícono o componente: skill `moonlit-design-system` → `docs/moonlit-design-system/00-INDICE.md`.
```
(37 líneas.)

## II.4 Preguntas abiertas restantes (no bloquean; se cierran en el hito indicado)

| # | Pregunta | Opciones | Recomendación | Hito |
|---|---|---|---|---|
| PC-1 | Unidades de `get_action_data()` para la rueda en ipod6g (¿grados/s como `button_get_data()` en Aura?) | (a) mismas; (b) distintas → tabla de conversión | Leer `button-6g.c` y decidir | M6 |
| PC-2 | `metro_albumart_decode_track_cover` solo a 80 px | (a) parámetro `size`; (b) segunda función | (a) | M7 |
| PC-3 | Precarga de Marea: ¿al `db_ready` (arranque) o al entrar a Marea? | (a) arranque, patrón D-224; (b) entrada con cápsula | (a) si la UI de "Actualizando biblioteca" admite un paso más; si no, (b) | M7 |
| PC-4 | `display-40` de Libre Baskerville > 60 KB | (a) aceptar (carga completa igual, `font.c:365-377`); (b) bajar a 32 px | (a) salvo que `check_fonts.py` muestre > 120 KB | M2 |
| PC-5 | Tema `dawn`: ¿presets de `primary` idénticos a night o tonos propios? | (a) propios (MD3 primary40 vs primary80); (b) idénticos | (a) — es lo que hace MD3; `tokens.json` los lleva por esquema | M1 |
