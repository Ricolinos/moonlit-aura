# moonlit.aura — Plan de implementación (fase 3)

**Fecha:** 2026-08-25 · **Estado:** ACTIVO (sin encabezado `ESTADO:` = pendiente de ejecución).
**Entradas vinculantes:** `docs/plan/00-decisiones-moonlit.md` (DM-001…DM-017) y `docs/plan/02-investigacion.md`. Ante discrepancia entre este plan y `00-decisiones-moonlit.md`, mandan las decisiones.
**Alcance:** este documento se ejecuta en sesiones nuevas **dentro del repositorio `moonlit-aura`** (que se crea en el hito H0). Ninguna sesión que ejecute este plan escribe en `Aura-Firmware/`, `Metro-Aura/` ni `Aura-Studio/`; esos repos se leen como referencia y se citan por ruta.

**Estado del repo de referencia al redactar (para reproducir citas):** `Metro-Aura` HEAD `2f1bd28a693b52a3554ecd8b4524ae1afa8e975d` (`main`, limpio); `Aura-Firmware` HEAD `7ec39edbf7cbe8547afa55880336ecdf2f890104` (`main`, limpio). Toda cita `ruta:línea` sin prefijo de repo se refiere a `Metro-Aura/`; las de Aura-Firmware llevan prefijo `AF/`. Rutas dentro del firmware abrevian `firmware/rockbox/apps/metro/` como `apps/metro/`.

Convenciones de este plan:
- **CONCLUSIÓN** = verificado con cita en la investigación o en los informes de lectura de esta sesión. **HIPÓTESIS** = no verificado; no se ejecuta sin cerrarse en el hito que la nombra.
- Cada hito cierra con commits atómicos en `moonlit-aura`, **sin push**, y con la evidencia listada en su tabla "Cierre".

---

## A. Arquitectura del repositorio moonlit.aura

### A.1 Estrategia de fork (DM-003)

**CONCLUSIÓN.** Base: `Metro-Aura` con historial completo.

```
git clone /Volumes/Ricolinos/Codigo/GitHub/Aura/Metro-Aura moonlit-aura
cd moonlit-aura
git remote rename origin metro-upstream
git remote add origin git@github.com:ricolinos/moonlit-aura.git   # sin push en este plan
git remote add aura-upstream /Volumes/Ricolinos/Codigo/GitHub/Aura/Aura-Firmware
git fetch aura-upstream
git tag moonlit-fork-base 2f1bd28a693b52a3554ecd8b4524ae1afa8e975d
```

El tag `moonlit-fork-base` fija el punto de divergencia; la decisión M-001 de Metro (`DECISIONS.md:15`, base Rockbox `0726ec93517a61f602679ab052b083217ec9c96d`) se hereda tal cual como base Rockbox de moonlit.

**Qué se cherry-pickea de Aura-Firmware y desde qué commits** (historial leído con `git log --follow`, informe de lectura de esta sesión):

| Módulo | Commit que lo introduce | Último commit que lo toca | Modo de traída | Dependencias reales (costo) |
|---|---|---|---|---|
| `AF/firmware/rockbox/apps/aura/aura_flow.{c,h}` | `14261906` (Fase 31.1) | `22f44275` (D-317) | **Copia de archivo** desde `aura-upstream/main` (`git show aura-upstream/main:<ruta> > apps/metro/moonlit_flow.c`), no `git cherry-pick` (los 5 commits tocan además archivos que no existen en Metro) | **Cero**: `aura_flow.c:23` solo incluye `aura_flow.h`. Es punto fijo + sin/cos + proyección; cosecha de `pictureflow.c` (`aura_flow.h:25`). |
| `AF/design-system/generate.py` | `8245d71a` (Fase 2) | `83ffcc41` (D-289 4/4) | Copia de archivo + **poda** (ver A.2, hito H2): se conservan `generate_header`, la generación de fuentes vía `convttf` y el bloque de iconos con `MIN_INK_TONES` (`generate.py:475,575-583,626-632`); se elimina `--swift-out`, `theme_format_json`, `panel_backgrounds`, `tile_icons` | Pillow en `design-system/.venv` (gitignorado en AF, `.gitignore:23`); `convttf` del fork. Ninguna dependencia de C. |
| `AF/design-system/tokens.json` | `8245d71a` | `22f44275` | Se toma como **plantilla de esquema**, no como contenido: claves `screen`, `layout`, `color{light,dark}`, `spacing`, `type_scale`, `font`, `icon` (`tokens.json:1-104`). El contenido se reescribe entero (sección B). | — |
| `AF/firmware/rockbox/apps/aura/aura_albumart.{c,h}` | `916e161e` (Fase 5) | `f81c2297` (D-322) | **NO se copia entero.** `aura_albumart.c:39-46` arrastra `aura_settings.h`, `apple2026_shell.h`, `apple2026_tokens.h`, `aura_style.h`, `aura_art.h`, `aura_music.h`, `aura_artist_images.h` y 12 símbolos `aura_*`/`a26_*`. Se copia únicamente la **capa de caché `.pfraw`** que Marea necesita: `aura_art_write_pfraw`, `aura_art_read_pfraw`, `aura_art_pfraw_is_cached`, `aura_art_transpose`, `aura_art_mask_corners_transposed` (viven en `AF/…/aura_art.{c,h}`, HIPÓTESIS: no se leyó ese archivo; el hito H0 lo inventaría). | Ver pregunta abierta **PA-1**: esto es una desviación acotada de DM-003 ("byte-idéntico") que el ejecutor debe cerrar antes de H6. |
| `AF/firmware/rockbox/apps/aura/aura_wheel.{c,h}` | (no leído) | (no leído) | Copia de archivo si Metro no tiene equivalente. `aura_wheel_step(int velocity_deg_s)` (`aura_wheel.h:48`, `aura_wheel.c:25`) y `aura_main_wheel_velocity()` (usada en `aura_musicflow.c:1240`). **HIPÓTESIS**: Metro no expone velocidad de rueda en grados/s; el hito H0 lo verifica con `grep -rn wheel_velocity apps/metro/`. | Por verificar en H0. |

Todo lo demás de Aura-Firmware queda fuera (DM-003). El directorio `AF/design-system/out/` y `apple2026_tokens.h` están gitignorados (`AF/.gitignore:22,39`): **nada generado se cherry-pickea, todo se regenera** en moonlit.

Archivos heredados de Metro que se **renombran** en H1 (prefijo `metro_` → `moonlit_`) solo si su cabecera o nombre público aparece en pantalla o en el contrato; el resto conserva el prefijo `metro_` para no romper el historial de `git blame` ni `MODIFICATIONS.md`. Ver PA-2.

### A.2 Árbol de carpetas y archivos

Estructura basada en la de Aura-Firmware para `.claude/skills/`, `docs/<sistema-de-diseño>/` y `design-system/` (Metro-Aura **no tiene** `.claude/` ni `docs/design/` — informe de lectura §2), y en la de Metro-Aura para `firmware/` y `docs/`.

```
moonlit-aura/
├── CLAUDE.md                     ← A.3 (≤40 líneas)
├── DECISIONS.md                  ← D-001…D-017 = DM-001…DM-017 copiadas en H0; nuevas D-018+
├── MODIFICATIONS.md              ← heredado de Metro; se extiende en la misma pasada de cada cambio fuera de apps/metro/ (GPL v2 §2a)
├── LICENSE                       ← copia de firmware/rockbox/docs/COPYING (DM-002, patrón D-283)
├── README.md                     ← reescrito: qué es moonlit.aura, hardware, cómo instalar
├── CONTRATO-moonlit-studio.md    ← F: contrato cross-repo propio, v1 (no copia el de Aura; lo referencia)
├── .claude/skills/moonlit-design-system/SKILL.md   ← A.3: todo lo que no cabe en CLAUDE.md
├── .claude/skills/moonlit-design-system/reference/ ← copias de docs/moonlit-design-system/*.md que la skill cita
├── design-system/
│   ├── tokens.json               ← B.1: ÚNICO origen de color/escala/espaciado/radios/elevación/iconos
│   ├── generate.py               ← A.1: podado de AF; produce out/moonlit_tokens.h, out/fonts/*.fnt, out/icons_table.c
│   ├── vendor/libre-baskerville/{LibreBaskerville-Regular,-Bold,-Italic}.ttf + OFL.txt   ← C.2
│   ├── vendor/montserrat/{Montserrat-Regular,-Medium,-SemiBold}.ttf + OFL.txt            ← C.2 (build estática, DM-004)
│   ├── vendor/material-symbols/<nombre>.svg + LICENSE (Apache 2.0)                        ← B.4
│   ├── logo/moonlit-crescent.svg + moonlit-wordmark.svg                                   ← E
│   └── out/                      ← gitignorado; regenerado por generate.py
├── docs/
│   ├── moonlit-design-system/    ← equivalente a AF/docs/aura-design-system/: 00-INDICE.md, sistema/{01-color,02-tipografia,03-geometria,04-movimiento,05-elevacion}.md, componentes/{lista,hub,ahora-suena,marea,acerca-de}.md
│   ├── COMPAT_STUDIO.md          ← heredado de Metro; se reetiqueta con los strings DM-001
│   ├── GUIA_FLASHEO.md, guia-desarrollo.md, SUPERFICIES.md   ← heredados, texto adaptado
│   ├── plans/                    ← planes activos de moonlit (este archivo se copia aquí como PLAN-implementacion.md en H0)
│   ├── plans/archivo/            ← históricos de Metro (INVESTIGACION.md, PLAN_MAESTRO.md…) con encabezado ESTADO:
│   └── screenshots/              ← capturas de criterio de "hecho" por hito (320×240)
└── firmware/
    ├── assets/fonts/*.fnt        ← salida de generate.py (commiteada, viaja en rockbox.zip)
    ├── assets/icons/             ← se vacía: los SVG viven en design-system/vendor/material-symbols/
    ├── rockbox/                  ← árbol Rockbox (M-001), byte-idéntico salvo MODIFICATIONS.md
    ├── rockbox/apps/metro/       ← código propio; nuevos archivos con prefijo moonlit_ (ver lista abajo)
    ├── rockbox/utils/mks5lboot/  ← heredado, frontera GPL A.4
    ├── rockbox/bootloader/ipod-s5l87xx.c ← heredado, frontera GPL A.4
    └── tools/{build_target.sh, build_sim.sh, sim_shot.sh, sim_matrix.sh, gen_fonts.sh, package_dist.sh, gen_logo.py, check_tones.py}
```

Archivos nuevos en `apps/metro/` (todos con cabecera GPL v2):

| Archivo | Propósito | Hito |
|---|---|---|
| `moonlit_tokens.h` | Generado por `generate.py` (gitignorado en AF; en moonlit **se commitea** para que `build_target.sh` no dependa de Python — ver PA-3) | H2 |
| `moonlit_palette.c/.h` | Sustituye `metro_palette.h`: getters `moonlit_color(enum moonlit_color_role)`, `moonlit_color_accent()`, `moonlit_surface(level, edge)` — únicos lectores de `moonlit_tokens.h` | H3 |
| `moonlit_elevation.c/.h` | Primitiva "luz desde la izquierda": `moonlit_draw_surface(x,y,w,h,level,radius)` (B.3) | H3 |
| `moonlit_fonts.c/.h` | Reemplaza `metro_fonts.c`: roles C.3, carga con `font_load_ex` | H2 |
| `moonlit_icons.h` + `moonlit_icons_table.c` | Tabla C generada (máscaras 8-bit) | H2 |
| `moonlit_logo_table.c` | Máscaras del logo a 16/24/40/64 px + wordmark | H5 |
| `moonlit_flow.c/.h` | Copia de `aura_flow.{c,h}` con `AURA_FLOW_*` → `MOONLIT_FLOW_*` y `SCREEN_W`/`SCREEN_H` intercambiados para el eje vertical | H6 |
| `moonlit_art.c/.h` | Capa `.pfraw` (PA-1) | H6 |
| `moonlit_screen_marea.c` | Pantalla Marea (D) | H6 |
| `moonlit_lang_extra.c` | Strings nuevos (Marea, créditos, wordmark) añadidos al final de `metro_lang.c` — patrón Metro M-009 (`CLAUDE.md:9-10`) | H1+ |

### A.3 Borrador de `CLAUDE.md` (≤40 líneas)

```markdown
# moonlit.aura

Firmware para iPod Classic 6G (S5L8702, 320×240, 64 MB, sin GPU, sin FPU), fork de
Metro-Aura (tag `moonlit-fork-base`) sobre Rockbox (M-001). Nombre visible: "moonlit.aura".

## Comandos
- Target:      `firmware/tools/build_target.sh [--firmware|--bootloader]`  → `firmware/build-ipod6g/rockbox.ipod`
- Simulador:   `firmware/tools/build_sim.sh [--reconfigure] [--run]` (hace `make install` + copia .fnt; brew sdl2 gcc)
- Captura:     `firmware/tools/sim_shot.sh <out.png> [ticks] "SELECT,RIGHT,…"` (320×240, headless)
- Tokens/fuentes/iconos: `design-system/.venv/bin/python3 design-system/generate.py` (falla si un ícono tiene <4 tonos)
- Tests host:  `make -C firmware/rockbox/apps/metro/test test`
- Release:     `firmware/tools/package_dist.sh --release-tag vX.Y.Z` (árbol limpio, sin __DATE__/__TIME__)

## Reglas que no se deducen del código
- Idiomas: `.md` en español de México sin voseo; código, comentarios y commits en inglés; UI en español (metro_lang.c).
- Toda decisión de diseño se cierra por escrito en `DECISIONS.md` (D-NNN) antes de ejecutarse. D-001…D-017 son vinculantes.
- Ningún literal RGB fuera de `design-system/tokens.json`. En C solo se leen por `moonlit_palette.h`.
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
Fuente viva: `docs/moonlit-design-system/00-INDICE.md`.
```

(38 líneas.) Todo lo demás —lenguaje Waning Crescent, tokens, jerarquía tipográfica, elevación, subconjunto Material, Marea, logo— va a `.claude/skills/moonlit-design-system/SKILL.md`, con cabecera YAML `name`/`description` idéntica en forma a `AF/.claude/skills/apple2026-design-system/SKILL.md:1-4` y un puntero a `docs/moonlit-design-system/00-INDICE.md` (patrón `SKILL.md:10`). El cuerpo de la skill se redacta en H2–H3 a partir de las secciones B–E de este plan.

### A.4 Frontera GPL: mks5lboot y bootloader

**CONCLUSIÓN (informes de lectura, §4 Metro y §7 AF): no existe un número de versión propio para `mks5lboot` ni para `bootloader-ipod6g.ipod` en ningún repo hermano.** Se identifican por:
- Base Rockbox: commit `0726ec93517a61f602679ab052b083217ec9c96d` (`DECISIONS.md:15`, M-001).
- Bootloader: `firmware/rockbox/bootloader/ipod-s5l87xx.c` (`docs/GUIA_FLASHEO.md:18`); imprime `RBVERSION` del build (`ipod-s5l87xx.c:864`, `build-ipod6g/rbversion.h:2`). En Aura-Firmware, último cambio `72dbf778` (2026-08-10, "bootloader silencioso") y "sin cambios desde" esa fecha (`AF/DECISIONS.md:88,190,234`).
- `mks5lboot`: `firmware/rockbox/utils/mks5lboot/mks5lboot.h:41` `#define IM3_VERSION "1.0"` (formato DFU, no versión de herramienta); modificación registrada `MODIFICATIONS.md:56` (backend libusb opcional en macOS).
- Identidad distribuida: SHA-256 en `firmware/dist/checksums.txt` (`package_dist.sh:207-212`), y `CONTRATO-firmware-studio.md:96-100` (§B) que declara los cuatro binarios derivados GPL v2.

**Lo que hereda moonlit (H0, sin cambio de código):** `firmware/rockbox/bootloader/ipod-s5l87xx.c`, `firmware/rockbox/utils/mks5lboot/*` y el `Makefile` modificado, tal como están en `moonlit-fork-base`. `package_dist.sh` los recompila (`:105-106`) y produce `bootloader-ipod6g.ipod` + `mks5lboot` + `checksums.txt`.

**Versionado explícito que este plan introduce (requisito de la tarea):** `CONTRATO-moonlit-studio.md` §B declara
```
bootloader: fuente ipod-s5l87xx.c @ moonlit-fork-base (2f1bd28a), Rockbox base 0726ec93, RBVERSION "<hash>-<fecha>" del build de release, SHA-256 en checksums.txt
mks5lboot:  fuente utils/mks5lboot @ moonlit-fork-base, IM3_VERSION 1.0, Makefile modificado (MODIFICATIONS.md:56), SHA-256 en checksums.txt
Versión de frontera GPL: BOOT-1  (sube a BOOT-2 solo si cambia cualquiera de los dos fuentes)
```
Ver PA-4 (si se adopta la etiqueta `BOOT-n` o se deja solo el SHA-256 como hoy).

---

## B. Sistema de diseño

### B.1 Tokens: un solo archivo fuente (DM-010)

`design-system/tokens.json` (esquema derivado de `AF/design-system/tokens.json:1-104`; contenido nuevo):

```jsonc
{
  "$schema": "moonlit-tokens-1",
  "screen": { "width": 320, "height": 240, "dpi": 160 },          // LCD_DPI 160 → 1 dp = 1 px
  "layout": { "statusbar_height": 20, "list_inset": 16, "corner_radius_card": 8, "corner_radius_pill": 12 },
  "spacing": { "xs": 4, "s": 8, "m": 16, "l": 24, "xl": 32 },       // grilla 8 px (DM-011)
  "color": {
    "night": {                                                       // único tema en v1 (PA-5)
      "bg":            [ 12,  14,  22],
      "surface_0":     [ 18,  20,  30],  "surface_1": [ 24,  27,  39],  "surface_2": [ 31,  35,  50],
      "on_surface":    [226, 228, 235],  "on_surface_dim": [150, 154, 168],  "outline": [ 52,  57,  76],
      "light_edge_delta":  12,             // sumado por canal al borde izquierdo/superior (DM-012)
      "shadow_edge_delta": -10,            // restado al borde derecho/inferior
      "accent_default": "moonstone",
      "accent_presets": { "moonstone": [168,184,214], "tide": [ 96,146,186], "ember": [214,150,110], "moss": [128,168,140] }
    }
  },
  "type_scale":  { … C.3 … },
  "font":        { … C.1 … },
  "elevation":   { "levels": 3, "edge_px": 1 },                     // B.3
  "icon":        { "family": "Material Symbols Rounded", "license": "Apache-2.0", "sizes": [16, 24, 40], "names": [ … B.4 … ] },
  "motion":      { "transition_ms": 220, "ease": "out_expo" }        // B.5
}
```

Reglas mecánicas (se verifican en H2 con un `grep` en el cierre del hito):
- `grep -rn "LCD_RGBPACK\|0x[0-9A-Fa-f]\{6\}" apps/metro/*.c apps/metro/*.h | grep -v moonlit_tokens.h` debe devolver **cero** líneas propias de moonlit (las heredadas de `metro_palette.h` se eliminan en H3).
- `generate.py` escribe `moonlit_tokens.h` con `#define MOONLIT_COLOR_<ROL> LCD_RGBPACK(r,g,b)` y `#define MOONLIT_ACCENT_<NOMBRE>`; solo `moonlit_palette.c` los incluye.
- El acento en runtime: `moonlit_color_accent()` (equivalente a `metro_color_accent()`, `metro_theme.c:89-92`) lee `metro_settings.accent` y devuelve el preset; ningún otro módulo indexa la tabla.

### B.2 Lenguaje visual Waning Crescent

Principios (van al skill, no a CLAUDE.md):
1. **Minimalismo**: una sola superficie por nivel de jerarquía, sin bordes de 1 px salvo los de elevación (B.3). Contenido sobre `bg`; tarjetas/filas seleccionadas sobre `surface_1`; modales sobre `surface_2`.
2. **Contrastes suaves**: el texto principal nunca es blanco puro (`on_surface` = 226/228/235); texto secundario `on_surface_dim`. Contraste mínimo `on_surface`/`bg` ≈ 13:1, `on_surface_dim`/`bg` ≈ 6:1 (HIPÓTESIS: calcular en H2 con `generate.py --contrast` y fallar si `on_surface_dim` < 4.5:1).
3. **Iluminación lateral desde la izquierda** (DM-012): toda superficie elevada tiene el borde izquierdo y superior aclarado (`light_edge_delta`) y el derecho e inferior oscurecido (`shadow_edge_delta`). Es lo único que expresa profundidad; coherente con la luna menguante, iluminada por la izquierda (E).
4. **Sombras sutiles**: no existe sombra difusa. La "sombra" es el borde oscuro de 1 px (`elevation.edge_px`) del punto anterior. Nunca blur (DM-011).
5. **Calma nocturna**: sin animaciones de atención (sin pulsos, sin rebotes); el acento aparece solo en selección, progreso y logo.

### B.3 Elevación y sombras sin GPU

**Primitiva** `moonlit_draw_surface(x, y, w, h, level, radius)` en `moonlit_elevation.c` (H3):

1. Rellena el rectángulo redondeado con `surface_<level>` usando `lcd_fillrect()` por filas + esquinas paramétricas (mismo principio que `a26_shell_fill_rounded_rect()`, `AF/apple2026_shell.c:546`; en Metro, el anillo antialias de `metro_widgets.c:195,223` vía `metro_fb_plot_alpha()`, `metro_fb.c:116`).
2. Dibuja el borde **izquierdo y superior** de 1 px con `surface_<level> + light_edge_delta` y el **derecho e inferior** con `surface_<level> + shadow_edge_delta`. Los seis colores resultantes (3 niveles × 2 bordes) los precalcula `generate.py` en `moonlit_tokens.h` — **cero aritmética de color por cuadro**.
3. Esquinas: el píxel de esquina toma el color del borde superior (izquierda) o inferior (derecha), sin blend.

**Costo**: 4 `lcd_hline`/`lcd_vline` + un `fillrect`. Sin buffers temporales (pila de 8 KB, D-226/D-227 citados en `02-investigacion.md` §1.5).

**Qué queda bajo `lcd_active()`** (puerta obligatoria, `CLAUDE.md` Metro:28-30; sitios existentes `metro_transitions.c:170,366,402`):
- Transiciones de pantalla (B.5).
- Desplazamiento animado de listas y de Marea.
- Cualquier redibujo de progreso (barra de reproducción).
- **No** va bajo `lcd_active()` el dibujo estático de superficies: se pinta una vez por cambio de estado.

### B.4 Subconjunto de Material Design aprobado (DM-011)

Aprobados los 15 principios con primitiva confirmada (`02-investigacion.md` §1.5) más el 16.º condicionado:

| Principio | Primitiva en el fork | Módulo moonlit |
|---|---|---|
| Grilla 8 dp = 8 px | `LCD_DPI 160` (`firmware/export/config/ipod6g.h:83`) | tokens `spacing` |
| Escala tipográfica discreta | `font_load_ex` (`metro_fonts.c:62`) | C.3 |
| Superficies por tono (elevación) | `lcd_fillrect` + bordes | `moonlit_elevation.c` |
| Estados seleccionado/deshabilitado por tinta | `on_surface`/`on_surface_dim` | `moonlit_palette.c` |
| Iconografía Material Symbols, cobertura fija | `metro_fb_plot_alpha()` (`metro_fb.c:116`) | `moonlit_icons_table.c` |
| Listas con divisores | `lcd_hline` de 1 px en `outline` — **condición DM-011**: el hito H3 cita la línea de `metro_screen_list.c` donde se dibuja | `metro_screen_list.c` |
| Chips/píldoras rectángulo redondeado ≤ 8 px | radio paramétrico | `moonlit_elevation.c` |
| Fade-through / shared-axis ≤ 300 ms | `metro_transitions_fade()` (`metro_transitions.c:396`), `metro_transitions_slide()` (`:165`) | heredado |
| Easing por tabla | `ease_out_expo_table[16]` (`metro_motion.c:25-27`), `metro_ease()` (`:58`) | heredado |
| "Vidrio" = alfa plano constante, solo en controles | `metro_fb_blend_over_color()` (`metro_fb.c:151`) | barra de estado |
| Esquinas redondeadas paramétricas | anillo antialias `metro_widgets.c:195,223` | heredado |
| Blend entero sin FPU | `metro_fb.c:98-104,116-126` | heredado |
| Selección por tinta de acento, no por relleno | `moonlit_color_accent()` | H3 |
| Barra de estado de 20 px | `layout.statusbar_height` | H3 |
| Matriz Animaciones × Gráficos (M-015) | `aura.cfg` → `animations`/`graphics` (`metro_settings.c:136-137`) | heredado |
| Iluminación lateral (extensión propia, DM-012) | B.3 | `moonlit_elevation.c` |

**Prohibidos** (DM-011): blur, ripple, sombras difusas, easing bezier en runtime, rasterización vectorial en runtime, `spring` con sobrepaso (`aura_motion_spring`, `AF/aura_motion.c:44-50`, **no se copia**: contradice "calma nocturna").

Iconos requeridos en v1 (Material Symbols Rounded, Apache 2.0; `design-system/vendor/material-symbols/`): `play_arrow`, `pause`, `skip_next`, `skip_previous`, `repeat`, `shuffle`, `volume_up`, `favorite`, `sync`, `album`, `person`, `queue_music`, `photo`, `movie`, `settings`, `info`, `lock`, `battery_full`, `battery_charging_full`, `usb`. Tamaños 16/24/40 → máx. 20 × 3 = 60 máscaras; a 8 bit/px: 20×(256+576+1600) ≈ 48,6 KB de flash (contra los 461 KB del Cartesiano de Aura, `02-investigacion.md` §1.3). `sync` se prueba primero (riesgo alto, §1.3); si falla tonos a 16 px, se usa variante *Rounded weight 500* — ver H2.

### B.5 Motion

- Duración única `motion.transition_ms = 220` (≤ 300, DM-011), easing `METRO_EASE_OUT_EXPO`.
- Pantalla → pantalla: `metro_transitions_fade()` (fade-through). Lista → detalle: `metro_transitions_slide()` (shared-axis horizontal). Marea: desplazamiento vertical propio (D.4).
- Toda transición respeta `metro_transitions_effective_all()` (`metro_transitions.c:347`) y el nivel FX.

---

## C. Tipografía

### C.1 Pipeline TTF → `.fnt` (DM-004, DM-007)

**Herramienta**: `firmware/rockbox/tools/convttf.c` (byte-idéntico entre hermanos, `02-investigacion.md` §1.1), compilado por `gen_fonts.sh:20-25` (`cc -lm -std=c99 -O2 -Wall -g convttf.c -o convttf $(pkg-config --cflags --libs freetype2)`).

**Comando exacto por rol** (corrige el bug `atoi("0x20")` de `gen_fonts.sh:47-48` → rango decimal, §1.1):

```
firmware/rockbox/tools/convttf -p <px> -s 32 -l 383 -D 63 -c <sep> -o firmware/assets/fonts/moonlit-<rol>-<px>.fnt design-system/vendor/<familia>/<archivo>.ttf
```
- `-s 32 -l 383`: Latin básico + Latin-1 + Latin Extended-A (317 glifos reales medidos en el estudio, contra 1309 con el bug).
- `-D 63`: glifo por defecto `?`.
- `-c <sep>`: separación extra entera; `1` para roles ≥ 28 px (evita la fusión "ll", M-082, `gen_fonts.sh:52-60`), `0` para el resto.
- **Sin `-x`** (trim horizontal): rompe el ancho del espacio (M-028, `gen_fonts.sh:39-45`).
- **Hinting**: `convttf.c` carga con `FT_LOAD_DEFAULT` (hinting automático de FreeType, antialias 256 niveles → 4 bpp, `convttf.c:102,115,626-629`). **HIPÓTESIS**: no existe bandera de hinting en `convttf`; el hito H2 lo confirma con `grep -n "FT_LOAD" convttf.c` y, si el autohinter degrada las serifas de Baskerville a 22 px, evalúa parchear `convttf.c` para `FT_LOAD_NO_AUTOHINT` (cambio fuera de `apps/metro/` → `MODIFICATIONS.md`).
- **Fuentes variables prohibidas** (DM-004, `02-investigacion.md` §1.6): solo builds estáticas.
- Carga en runtime: `font_load_ex(path, 0, MOONLIT_FONT_GLYPH_BUDGET)` con presupuesto 400 glifos (`metro_fonts.c:33,62`). `MAX_FONT_SIZE 60000` (M-010/M-011). HIPÓTESIS a verificar en H2: Libre Baskerville a 40 px supera 60 KB (extrapolando 34,8 KB a 20 px); si `font_load_ex` con caché de glifos lo absorbe (como Selawik 48 px hoy, `metro_fonts.c:41`), no hay problema; si no, el rol `display` baja a 32 px.

`generate.py` invoca exactamente ese comando por rol leyendo `tokens.json:type_scale` (reemplaza la lista `ROLES=` de `gen_fonts.sh:28-34`); `gen_fonts.sh` queda como wrapper que llama a `generate.py --fonts-only`.

### C.2 Licencias (obligatorio para redistribuir bajo GPL)

| Familia | Archivos TTF | Licencia | Ruta del texto de licencia | Llega a `THIRD-PARTY-NOTICES.txt` |
|---|---|---|---|---|
| Libre Baskerville | `LibreBaskerville-Regular.ttf`, `-Bold.ttf`, `-Italic.ttf` | SIL OFL 1.1 | `design-system/vendor/libre-baskerville/OFL.txt` | Sí (bloque nuevo) |
| Montserrat (estática, github.com/JulietaUla/Montserrat) | `Montserrat-Regular.ttf`, `-Medium.ttf`, `-SemiBold.ttf` | SIL OFL 1.1 | `design-system/vendor/montserrat/OFL.txt` | Sí |
| Material Symbols Rounded (SVG) | `*.svg` | Apache 2.0 | `design-system/vendor/material-symbols/LICENSE` | Sí |
| Rockbox | — | GPL v2 | `LICENSE` (raíz) = `firmware/rockbox/docs/COPYING` | No (no es tercero vendoreado; va en "Acerca de") |

`package_dist.sh` reescribe el bloque `THIRD-PARTY-NOTICES.txt` (Metro `:179-205` → patrón AF `:202-233`) concatenando los tres archivos anteriores. Los directorios `fonts-src/` (Selawik) e `icons/` (Fluent) de Metro **se eliminan** en H2 junto con `LICENSE-fluent-system-icons.txt` y `fonts-src/LICENSE.txt`; "cero material Microsoft" (M-020) se mantiene.

Pantalla "Acerca de" (`metro_screen_about.c`): se añade un bloque de créditos (patrón `AURA_STR_ABOUT_CREDITS_BODY`, `AF/aura_lang.c:258`) con: autor, "Basado en Rockbox… GPL v2… Código fuente: github.com/ricolinos/moonlit-aura", "Tipografía e íconos: Libre Baskerville y Montserrat (SIL OFL), Material Symbols (Apache 2.0)", nota Apple sin afiliación. En Metro hoy solo existe `LANG_ABOUT_BASED_ON_ROCKBOX` (`metro_lang.h:80`, `metro_lang.c:71,210`) — se agrega `LANG_ABOUT_CREDITS_BODY` al final de la tabla (H1).

### C.3 Jerarquía (≤ 12 roles, DM-007; ningún rol < 18 px, DM-005)

Calibración del estudio (`02-investigacion.md` §7.6, un punto real): cap-height ≈ 0,64 × px nominal. Umbrales ISO 9241-303 (`01-plan-investigacion.md:92`): mínimo 10,5 px, cómodo 13–14 px.

| Rol (`enum moonlit_font_role`) | Familia / cara | px | cap-height proy. | `-c` | Uso | Justificación |
|---|---|---|---|---|---|---|
| `MFONT_DISPLAY` | Libre Baskerville Regular | 40 | 25,6 | 1 | Wordmark en arranque/Acerca de (DM-016), título del hub | Solo texto corto; serif a tamaño grande es donde luce (H-T1 refutada, pero la serif se reserva a títulos por DM-004) |
| `MFONT_TITLE` | Libre Baskerville Regular | 28 | 17,9 | 1 | Título de Ahora suena, encabezado de pivote | Equivale al rol `title` 28 de Metro (`metro_fonts.h:32`) |
| `MFONT_HEADLINE` | Libre Baskerville Regular | 22 | 14,1 | 0 | Nombre de álbum/artista en cabecera de lista, monograma de Marea | Umbral "cómodo" 13–14 px cumplido |
| `MFONT_LIST` | Montserrat Regular | 20 | 12,8 | 0 | Filas de lista no seleccionadas | Igual que `MFONT_LIST` Metro (`metro_fonts.h:33`) |
| `MFONT_LIST_SEL` | Montserrat SemiBold | 20 | 12,8 | 0 | Fila seleccionada | Igual que Metro `:34` |
| `MFONT_BODY` | Montserrat Regular | 18 | 11,5 | 0 | Subtítulos, valores de ajustes, letras | **Sustituye a `caption` 14** (9 px real, bajo ISO; §2) |
| `MFONT_LABEL` | Montserrat Medium | 18 | 11,5 | 0 | Barra de estado, contadores, etiquetas de botón | Medium distingue UI de contenido sin subir tamaño |
| `MFONT_ACCENT` | Libre Baskerville Italic | 18 | ~11,5 | 0 | Artista bajo el título en Ahora suena | Único uso de itálica; da el "susurro" del lenguaje (PA-6: opcional, puede eliminarse) |

**8 roles** (< 12, `MAXUSERFONTS` intacto, `firmware/export/font.h:51`). Tamaño estimado en RAM: 40 px ≈ 55–65 KB (HIPÓTESIS C.1), 28 ≈ 48 KB, 22 ≈ 38 KB, 20+20 ≈ 62 KB, 18×3 ≈ 75 KB → ~290 KB, trivial sobre 64 MB. Archivos: `moonlit-display-40.fnt`, `moonlit-title-28.fnt`, `moonlit-headline-22.fnt`, `moonlit-list-20.fnt`, `moonlit-listsel-20.fnt`, `moonlit-body-18.fnt`, `moonlit-label-18.fnt`, `moonlit-accent-18.fnt`.

Verificación mecánica de legibilidad (DM-006) en H2: captura del especímen (`metro_screen_specimen.c`, F2, se conserva como pantalla oculta de QA) con cada rol, medición de cap-height real con PIL (píxeles > 60/255 en la columna de una "H") y comparación con la tabla; el hito falla si algún rol queda < 10,5 px.

---

## D. Cover Flow vertical — "Marea" (DM-014; veredicto VIABLE CON RESTRICCIONES)

### D.1 Módulo

`apps/metro/moonlit_screen_marea.c` (pantalla) + `moonlit_flow.{c,h}` (proyección) + `moonlit_art.{c,h}` (caché `.pfraw`).

- **Proyección**: copia de `aura_flow.{c,h}` (autocontenido). Cambios: renombrar prefijos; `AURA_FLOW_SCREEN_W 320`/`SCREEN_H 240` (`AF/aura_flow.h:72-73`) → `MOONLIT_FLOW_AXIS_LEN 220` (alto útil = 240 − barra 20) y `MOONLIT_FLOW_CROSS_LEN 320`; `DISPLAY_W 128` (`:74`) → tamaño de tapa central **120** (§6 H2 de la investigación); `CAM_DIST 240` (`:75`) se conserva.
- **Dibujo**: `draw_slide_perspective()` de Aura lee columna por columna y escribe tiras verticales (`AF/aura_musicflow.c:605-606,673-710`). Marea implementa `draw_slide_vertical()` **en espejo**: itera filas de destino (`y`), obtiene la fila fuente con `moonlit_flow_source_column()` (renombrada `_source_row`) y escala horizontalmente con `moonlit_flow_vertical_scale()` (renombrada `_cross_scale`). Escritura por tiras horizontales con `lcd_bitmap_part()` / acceso directo al framebuffer como en `metro_fb.c`.
- **Sin reflejo, sin morphs** (DM-014): no se copia `aura_art_generate_reflection`, `MF_FLIP_MS`, ni el zoom 256/216 (`aura_musicflow.c:209-212,342`).

### D.2 Modelo de datos

```c
typedef struct {
    int album_index;                              /* índice en la lista de álbumes de tagcache (metro_music) */
    char title[MOONLIT_ITEM_LEN];                 /* nombre del álbum */
    char artist[MOONLIT_ITEM_LEN];
    bool has_art;
    fb_data cover_buf[120 * 120];                 /* 28 800 B, sin reflection_buf */
    unsigned char initial;                        /* para el monograma */
} marea_slot_t;
static marea_slot_t s_slots[MAREA_CACHE_SLOTS];   /* estático, nunca en pila (D-226/227) */
static int s_target_index, s_current_index_fp;    /* posición en punto fijo AURA_FLOW_SHIFT 10 */
```
Fuente de la lista de álbumes: `metro_music.c` (envoltorio de tagcache, informe §14) — Marea no llama a tagcache directamente.

### D.3 Caché de portadas

- `MAREA_VISIBLE_RADIUS 2` (5 tapas visibles: central + 2 por lado, §6 H2) → `MAREA_CACHE_SLOTS = 2*(2+15)+3 = 37` (misma fórmula que `MF_CACHE_SLOTS`, `aura_musicflow.c:91`) × 28 800 B ≈ **1,07 MB** (contra 1,60 MB de Aura).
- `get_slot_for(album_index)` se copia lógicamente de `aura_musicflow.c:448` (LRU + precarga síncrona, D-224).
- Disco: carátula decodificada una vez a `/.rockbox/aura/moonlitcache/art/<hash>.pfraw` (120×120, esquinas horneadas radio 8) vía `moonlit_art_write_pfraw()`; lecturas posteriores son un `read()` plano. **Nunca dentro del bucle de animación**: la precarga ocurre en `scroll_step()` antes de armar la animación, patrón `aura_musicflow.c:1238-1240`.
- La primera visita a un álbum sin `.pfraw` decodifica JPEG con `recorder/jpeg_load.h` (ya usado por `metro_albumart.c`/`metro_thumbs.c`, informe §14).

### D.4 Mapeo de rueda

- `aura_wheel_step(velocity_deg_s)` (`AF/aura_wheel.h:48`) devuelve pasos por evento a partir de la velocidad angular; opera sobre índice 1D (`02-investigacion.md` §3). Se copia como `moonlit_wheel_step()` si H0 confirma que Metro no tiene equivalente (A.1).
- Sentido: `SCROLL_FWD` (horario) → siguiente álbum = **la columna sube** (la tapa central se desplaza hacia arriba y entra una nueva por abajo, como una marea que sube). `SCROLL_BACK` → baja.
- `SELECT` sobre la tapa central → lista de pistas del álbum (`metro_screen_list`). `PLAY` → reproduce el álbum (M-071, global). `MENU` → vuelve al hub.
- Animación de desplazamiento: 220 ms, `METRO_EASE_OUT_EXPO`, bajo `lcd_active()` y nivel FX; con FX en "ninguna", salto directo sin animación.

### D.5 Estados vacíos

- **Sin portada** → monograma: rectángulo redondeado 120×120 en `surface_1` con iluminación lateral (B.3) y la inicial del álbum en `MFONT_HEADLINE` (Libre Baskerville 22 px) centrada, color = `moonlit_color_accent()`. `initial` = primer carácter alfabético del título en mayúscula; si no hay ninguno, `♪` no está en el rango 32–383 → se usa `#`.
- **Biblioteca sin álbumes** → texto `MFONT_BODY` "No hay álbumes en la biblioteca" centrado, mismo tratamiento que la lista vacía de Metro.
- **Base de datos en construcción** → Marea cede a la pantalla "Actualizando biblioteca…" existente (`metro_sync.c`), no muestra nada propio.

### D.6 Restricción de hardware (H3 de la investigación)

Marea queda marcada **experimental** en `DECISIONS.md` hasta que el hito H7 mida en el iPod: `DEBUGF("marea frame %ld ms", current_tick - t0)` por cuadro (patrón D-300), promedio y máximo sobre 60 cuadros de desplazamiento continuo. Criterio: máx ≤ 33 ms. Si falla: (a) bajar a 3 tapas visibles; (b) desactivar animación bajo FX "reducidas". Decisión a cerrar en H7 con el dato.

---

## E. Logotipo Waning Crescent (DM-016)

### E.1 Especificación vectorial original

`design-system/logo/moonlit-crescent.svg`, `viewBox 0 0 100 100`, sin trazos, una sola forma por sustracción booleana:

```
Círculo A (disco lunar):   centro (50, 50), radio 36
Círculo B (sombra):        centro (62, 46), radio 32     ← desplazado a la derecha y ligeramente arriba
Forma = A − B              → creciente iluminado por la IZQUIERDA (menguante vista desde el hemisferio norte)
Color: currentColor (el firmware lo pinta con moonlit_color_accent())
Grosor mínimo del creciente: 36 − (32 − 12) = 16 unidades en el eje horizontal → a 16 px de exportación = 2,6 px ≥ 2 px (legible)
Puntas: los cúspides caen en y ≈ 22 y ≈ 78; a 16 px son ~1 px de grosor — se aceptan como antialias (verificación E.3)
```
Ajuste fino permitido en H5: mover B en ±4 unidades para que el conteo de tonos pase; **no** se permite añadir estrellas, texto dentro del icono, ni derivarlo de ningún glifo existente.

`moonlit-wordmark.svg`: la palabra "moonlit" compuesta con Libre Baskerville Regular convertida a contornos (`fonttools` o Inkscape → path), minúsculas, tracking +2 %, altura de x alineada al centro del creciente cuando se componen juntos (creciente a la izquierda, 1 espacio de 8 px, wordmark). Solo se usa a ≥ 64 px (DM-016).

### E.2 Tamaños de exportación

| Uso | Tamaño | Fuente | Salida |
|---|---|---|---|
| Barra de estado / fila de hub | 16 px | crescent | máscara 8-bit en `moonlit_logo_table.c` |
| Cabecera Acerca de, ícono de ajuste | 24 px | crescent | ídem |
| Hub (tile "moonlit") | 40 px | crescent | ídem |
| Pantalla de arranque | 64 px crescent + wordmark 140×28 px | ambos | ídem (2 máscaras) |
| Acerca de, primera pantalla | 64 px + wordmark | ambos | reutiliza las anteriores |

Pipeline: `generate.py --logo` → `rsvg-convert -w <16·S> -h <16·S>` (S = 16, supersampleo) → filtro de caja 16× (mismo código que los iconos, `AF/generate.py:372-391`) → máscara 8-bit → tabla C. Reemplaza `firmware/tools/gen_logo.py` de Metro (M-092 wordmark "metro / aura"), que se elimina.

### E.3 Verificación mecánica (obligatoria antes de integrar)

Por cada máscara exportada, `generate.py` cuenta los tonos distintos > 0 (`len(tones) < MIN_INK_TONES` con `MIN_INK_TONES = 4`, `AF/generate.py:475,582`) y además, para el logo:
- **Tonos ≥ 4** en 16, 24, 40, 64 px y en el wordmark.
- **Cobertura del creciente** a 16 px: al menos 3 píxeles con valor ≥ 200/255 en la columna más ancha (garantiza cuerpo, no solo antialias).
- **Cúspides**: ningún píxel aislado con valor < 60/255 sin vecino ≥ 60/255 (evita "polvo").
Fallo en cualquiera → `die()` y el build se detiene (`AF/generate.py:46,626-632`). La salida del chequeo se guarda en `docs/screenshots/H5-logo-tones.txt` como evidencia.

---

## F. Contrato con Aura Studio (DM-017)

### F.1 Archivo `CONTRATO-moonlit-studio.md` — Versión 1 — (fecha del hito H1)

Referencia, no copia, los contratos canónicos de `Aura-Firmware/`: `CONTRATO-firmware-studio.md` v13 (`:3`), `CONTRATO-dispositivo.md` v2, `docs/contracts/library-layout-v1.md` v1.3. moonlit.aura **consume** esos contratos sin modificarlos (`CLAUDE.md` Metro:66-71).

**§A — Lo que moonlit.aura garantiza**
1. `/.rockbox/aura/aura.cfg` regenerado entero en cada guardado (`metro_settings.c`, C2) con `firmware_family: moonlit` (`metro_settings.c:131`, valor cambiado), `sync_marker_supported: 1` (`:132`) y **sin** `theme_format_supported` (DM-009).
2. Marcador `/.aura/sync-pending.json` procesado y borrado; versión no soportada se deja intacta (C4–C5).
3. Árbol dormido `/.firmware-moonlit/` (contrato v10, D-326; `FirmwareFamily.dormantTreeName`).
4. Caché privada en `/.rockbox/aura/moonlitcache/` (`metro_settings.c:214-217`, string cambiado); Studio la ignora (C23).
5. `install_manifest.cfg` se ignora (C28). `version.txt` dentro de `rockbox.zip` solo con `--release-tag` (C22, M-056).
6. Build reproducible: sin `__DATE__`/`__TIME__` en `rockbox.zip`.
7. Release en GitHub `ricolinos/moonlit-aura` con exactamente los assets de la tabla §A del contrato v13: `rockbox.ipod`, `rockbox.zip`, `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`, `MODIFICATIONS.md`, `THIRD-PARTY-NOTICES.txt`.
8. Centinela de árbol instalado: `/.rockbox/fonts/moonlit-body-18.fnt` (archivo que solo moonlit escribe) — PA-7.

**§B — Frontera GPL** (A.4): identidad de bootloader y mks5lboot, etiqueta `BOOT-1`.

**§C — Lo que moonlit.aura requiere de Studio** (no ejecutado en este plan): reconocer `firmware_family: moonlit` como familia instalable/actualizable; hasta entonces Studio degrada de forma segura (`SyncMarker.swift:68-99`, `02-investigacion.md` §4) pero **no** ofrece actualizaciones. Riesgo M-004 (`DECISIONS.md:41-71`): para versiones de Studio anteriores a ST-045 el riesgo de sobrescritura por hash persiste; moonlit lo documenta en `docs/GUIA_FLASHEO.md` con la misma advertencia que Metro (C21).

### F.2 Prompt aparte — `[Aura Studio]` (se guarda en `docs/plans/PROMPT-aura-studio.md`; no se ejecuta en fase 4)

```
[Aura Studio] Añadir la familia de firmware "moonlit" (repo ricolinos/moonlit-aura).
Solo lectura de moonlit-aura/CONTRATO-moonlit-studio.md v1. Cambios, todos en Aura-Studio:
- Models/FirmwareFamily.swift:28 → case moonlit; :35-40 configValue "moonlit"; :43-49 displayName "moonlit.aura";
  :56-62 releaseRepository "ricolinos/moonlit-aura"; :66-68 installable += .moonlit; :75-81 bundleSubdirectory "moonlit";
  :88-93 installedTreeSentinel "/.rockbox/fonts/moonlit-body-18.fnt"; :105-110 dormantTreeName ".firmware-moonlit";
  :114-121 parse case "moonlit".
- Views/ExtrasView.swift:75-97 entrada de UI (patrón Metro).
- scripts/fetch-firmware.sh:49-57,157 bloque moonlit. project.yml:72,119-121 recurso de bundle moonlit.
- FIRMWARE_VERSION.example: bloque moonlit.* (mismo formato que metro.*, líneas 13-20).
No requieren cambio: GitHubReleaseChecker, AuraUpdateChecker, BundledArtifacts (genéricos por familia).
Registrar como ST-NNN. Verificar con un iPod que tenga moonlit instalado (C20).
```

### F.3 Prompts aparte para los hermanos (DM-007, no se ejecutan aquí)

`[Metro-Aura]` y `[Aura-Firmware]`: "Corregir `gen_fonts.sh` (Metro `:47-48`) / `generate.py` (AF, equivalente) para pasar a `convttf` el rango en decimal (32–383): `atoi("0x20")` devuelve 0 y convierte el charset completo (1309 glifos en vez de ~317). Regenerar `.fnt`, medir diferencia de tamaño, registrar decisión y `MODIFICATIONS.md` si aplica." Se guardan en `docs/plans/PROMPT-hermanos-gen-fonts.md`.

---

## G. Orden de ejecución y verificación

Cada hito ≤ 1 sesión. Comandos siempre desde la raíz de `moonlit-aura`. "Cierre" = evidencia que Claude debe mostrar (salida de comando o captura) antes de commitear. Ningún hito hace `git push`.

### H0 — Fork, identidad documental y auditoría de dependencias
**Archivos:** clone + remotes + tag (A.1); `DECISIONS.md` (reemplazar cabecera: numeración D-001+, copiar DM-001…DM-017 como D-001…D-017, mover el histórico de Metro a `DECISIONS-METRO-ARCHIVE.md` de solo lectura); `CLAUDE.md` (A.3); `.claude/skills/moonlit-design-system/SKILL.md` (esqueleto); `docs/plans/PLAN-implementacion.md` (copia de este archivo); `docs/plans/archivo/*` con `ESTADO:`; `README.md`; `LICENSE`.
**Auditorías (solo lectura, resultado a `DECISIONS.md` D-018…):**
- D-018 Play/Pause global (DM-015): **evidencia ya citada** — `apps/metro/metro_keymap.c:31-33` (hub, M-071), `:55` (lista), `:88` (player) mapean `MACT_PLAYPAUSE` a `BUTTON_PLAY|BUTTON_REL`; manejo en `metro_screen_list.c:252-253` y `metro_screen_hub.c:951`; lock lo deja sin mapear (`:110`). Recomendación: **heredar M-071** (Play global salvo en lock y visor de fotos). Solo falta cerrar por escrito.
- D-019 `aura_wheel` (A.1 HIPÓTESIS): `grep -rn "wheel_velocity\|wheel_step" apps/metro/` y decidir copia.
- D-020 capa `.pfraw` (PA-1): inventariar `AF/…/aura_art.{c,h}` (funciones, includes, tamaño) y cerrar el alcance de la copia.
- D-021 M-004 cuerpo completo leído (`DECISIONS.md:41-71`, ya en el informe): heredar advertencia en `GUIA_FLASHEO.md`.
- D-022 `MAXUSERFONTS`: confirmar que 8 roles ≤ 12 → no se toca `font.h:51`.
**Verificación:** `firmware/tools/build_target.sh` (target compila sin cambios), `firmware/tools/build_sim.sh`, `firmware/tools/sim_shot.sh docs/screenshots/H0-baseline.png 150` (debe verse Metro intacto — línea base).
**Cierre:** salida de `git log --oneline -1 moonlit-fork-base`; `git status --short` limpio tras commits; captura `H0-baseline.png` 320×240; `DECISIONS.md` con D-001…D-022. Commits: `chore: fork moonlit-aura from Metro-Aura 2f1bd28a`, `docs: DECISIONS D-001..D-022, CLAUDE.md, skill skeleton`.

### H1 — Identidad en runtime y contrato
**Archivos:** `metro_settings.c:131` (`firmware_family: moonlit`), `:214-217` (`moonlitcache`), `metro_settings.h:109`; nombre de árbol dormido (grep `firmware-metro` en `apps/metro/` y `package_dist.sh`); `metro_lang.c`/`.h` (+`LANG_ABOUT_CREDITS_BODY`, +`LANG_MAREA_*`, wordmark "moonlit"); `metro_screen_about.c` (créditos C.2); `firmware/tools/package_dist.sh` (bloque THIRD-PARTY C.2, nombre de repo, assets §A); `docs/COMPAT_STUDIO.md` (strings DM-001); `docs/GUIA_FLASHEO.md` (advertencia M-004); `CONTRATO-moonlit-studio.md` v1 (F.1); `docs/plans/PROMPT-aura-studio.md`, `PROMPT-hermanos-gen-fonts.md` (F.2, F.3); tests `apps/metro/test/test_sync_marker.c`, `test_device.c` si contienen el string `metro`.
**Verificación:** `make -C firmware/rockbox/apps/metro/test test` (todo verde); `build_sim.sh`; `sim_shot.sh docs/screenshots/H1-about.png 200 "<botones hasta Acerca de>"`; `grep -rn "metrocache\|firmware_family: metro\|firmware-metro" apps/metro/ firmware/tools/` → cero resultados; `cat firmware/build-sim/simdisk/.rockbox/aura/aura.cfg` tras la captura → contiene `firmware_family: moonlit` y no `theme_format_supported`.
**Cierre:** salida de los tres comandos anteriores + captura de Acerca de con créditos legibles. Commits: `feat: firmware_family moonlit, moonlitcache, dormant tree (D-001)`, `docs: CONTRATO-moonlit-studio v1, COMPAT_STUDIO, prompts [Aura Studio]/[hermanos]`, `feat: about credits + THIRD-PARTY-NOTICES (D-002)`.

### H2 — design-system: tokens, fuentes, iconos, verificación mecánica
**Archivos:** `design-system/tokens.json` (B.1, C.3), `design-system/generate.py` (A.1 podado; `--fonts-only`, `--icons-only`, `--logo`, `--contrast`), `design-system/vendor/{libre-baskerville,montserrat,material-symbols}/` + licencias (C.2), `firmware/tools/gen_fonts.sh` (wrapper), `firmware/tools/build_sim.sh:49-53` (copia de `.fnt` con nuevos nombres), `apps/metro/moonlit_tokens.h` (generado, commiteado — PA-3), `moonlit_fonts.c/.h` (8 roles), `moonlit_icons.h` + `moonlit_icons_table.c`, `firmware/tools/check_tones.py` (reutiliza la función de conteo de `generate.py` sobre cualquier PNG/BMP: uso `check_tones.py <archivo> --min 4`), eliminación de `firmware/assets/fonts-src/`, `firmware/assets/icons/`, `gen_icons.py`, `metro_fonts.c/.h`, `metro_icons_table.c`; `.gitignore` (`design-system/out/`, `.venv/`); skill `SKILL.md` secciones B y C.
**Verificación:**
1. `design-system/.venv/bin/python3 design-system/generate.py` → termina en 0; imprime tabla de tonos por ícono (los 20 × 3 ≥ 4) y tamaño de cada `.fnt`.
2. `ls -la firmware/assets/fonts/` → 8 archivos `moonlit-*.fnt`, cada uno ≤ 60 000 B (o registrar la HIPÓTESIS C.1 resuelta: si `display-40` > 60 KB, medir carga real en el simulador y decidir 32 px).
3. `grep -n "FT_LOAD" firmware/rockbox/tools/convttf.c` (HIPÓTESIS hinting C.1).
4. `build_sim.sh` + `sim_shot.sh docs/screenshots/H2-specimen.png 200 "<botones al especímen>"`; medición de cap-height con PIL por rol (script en `firmware/tools/check_tones.py --capheight`) → todos ≥ 10,5 px.
5. `grep -rn "LCD_RGBPACK" apps/metro/*.c apps/metro/*.h | grep -v "moonlit_tokens.h\|metro_palette.h"` → cero (la paleta vieja se retira en H3).
**Cierre:** salida íntegra de `generate.py`, tabla de tamaños, captura del especímen, tabla de cap-height medida, resultado de `sync` a 16 px (si falló y se cambió de peso, decisión D-023). Commits: `feat(design-system): tokens.json + generate.py (D-010)`, `feat: vendor Libre Baskerville, Montserrat, Material Symbols + licenses (D-004, D-008)`, `feat: moonlit_fonts 8 roles, decimal charset range (D-005, D-007)`, `feat: compiled icon masks + check_tones.py`.

### H3 — Paleta, elevación y pantallas base (hub, lista, ajustes, barra de estado)
**Archivos:** `moonlit_palette.c/.h` (sustituye `metro_palette.h`; `metro_theme.c:23-34,89-92` pasa a leer `moonlit_tokens.h`), `moonlit_elevation.c/.h` (B.3), `metro_screen_hub.c`, `metro_screen_list.c` (divisor de 1 px: **citar línea**, DM-011), `metro_screen_settings.c`, barra de estado (donde viva en Metro: grep `statusbar` en `apps/metro/`), `metro_widgets.c` (modal sobre `surface_2`), `metro_transitions.c` (duración 220 ms desde tokens), `docs/moonlit-design-system/sistema/*.md`, skill.
**Verificación:** `build_target.sh` y `build_sim.sh` sin warnings nuevos (`-Werror` en tests); `sim_matrix.sh docs/screenshots/H3-matrix` (hub/lista/ajustes × acentos de `tokens.json`, adaptando `sim_matrix.sh:41-54` a los presets moonlit); `grep -rn "LCD_RGBPACK\|metro_palette.h" apps/metro/` → solo `moonlit_tokens.h`; verificación de iluminación lateral: `check_tones.py --edge docs/screenshots/H3-matrix/list-*.png` comprueba que en la fila seleccionada el píxel de borde izquierdo es más claro que el interior y el derecho más oscuro (script mecánico, no visual).
**Cierre:** matriz de capturas, salida del grep, salida de `--edge`. Commits: `feat: moonlit_palette + elevation (D-010, D-012)`, `feat: hub/list/settings restyle Waning Crescent (D-011)`, `chore: remove metro_palette.h`.

### H4 — Ahora suena y transiciones
**Archivos:** `metro_screen_nowplaying.c` (fondo plano tonal DM-013: eliminar el uso de `metro_fb_blend_over_color()` sobre carátula en `:631` si pinta la portada como fondo; título `MFONT_TITLE`, artista `MFONT_ACCENT`, controles con iconos compilados), `metro_screen_lock.c`, `metro_screen_usb.c`, `metro_screen_splash.c` (solo color, el logo llega en H5), `docs/moonlit-design-system/componentes/ahora-suena.md`.
**Verificación:** `build_sim.sh`; `sim_shot.sh docs/screenshots/H4-nowplaying.png 300 "<botones hasta reproducir metro-test.mp3>"` (medios de prueba: `firmware/tools/gen_test_media.sh`); captura con FX "ninguna" y "todas" (editar `simdisk/.rockbox/aura/aura.cfg` como hace `sim_matrix.sh:41-54`); `grep -n "lcd_active" apps/metro/metro_screen_nowplaying.c` → la barra de progreso animada está gateada.
**Cierre:** dos capturas (FX on/off), salida del grep. Commits: `feat: now playing tonal surface + typography (D-013)`, `feat: lock/usb/splash palette`.

### H5 — Logotipo y arranque
**Archivos:** `design-system/logo/moonlit-crescent.svg`, `moonlit-wordmark.svg` (E.1), `generate.py --logo` (E.2–E.3), `apps/metro/moonlit_logo_table.c`, `metro_screen_splash.c` (64 px + wordmark, M-092 reemplazado), `metro_screen_about.c` (64 px + wordmark), hub (40 px), barra de estado (16 px), eliminación de `firmware/tools/gen_logo.py` y de cualquier asset "metro / aura".
**Verificación:** `generate.py --logo` → tabla de tonos por tamaño ≥ 4 + chequeos de cobertura/cúspides (E.3) guardada en `docs/screenshots/H5-logo-tones.txt`; `build_sim.sh`; `sim_shot.sh docs/screenshots/H5-splash.png 30` (arranque) y `H5-about.png`; `check_tones.py docs/screenshots/H5-splash.png --region <x,y,w,h del logo> --min 4`.
**Cierre:** `H5-logo-tones.txt`, dos capturas, salida de `check_tones.py`. Commits: `feat: Waning Crescent logo spec + generated masks (D-016)`, `feat: splash/about/hub/statusbar use logo`.

### H6 — Marea (simulador)
**Archivos:** `moonlit_flow.c/.h` (copia de `aura_flow` @ `aura-upstream/main`, renombrada, ejes intercambiados), `moonlit_art.c/.h` (D-020), `moonlit_wheel.c/.h` si D-019 lo requiere, `moonlit_screen_marea.c` (D.1–D.5), entrada en el hub, `metro_music.c` (getter de lista de álbumes si falta), `metro_keymap.c` (contexto Marea), `metro_lang.c` (strings), `MODIFICATIONS.md` si se toca algo fuera de `apps/metro/`, `docs/moonlit-design-system/componentes/marea.md`, `DECISIONS.md` D-024 "Marea experimental hasta H7".
**Verificación:** `build_target.sh` (compila para ARM, tamaño de `.bss` reportado por `arm-elf-eabi-size firmware/build-ipod6g/rockbox.elf` — +1,07 MB esperado); `build_sim.sh`; `sim_shot.sh docs/screenshots/H6-marea-{0,1,2}.png 300 "<hub→Marea>,SCROLL_FWD,…"` con biblioteca de prueba que incluya un álbum **sin** carátula (`firmware/test-media/SinArte/`) → captura del monograma; `ls firmware/build-sim/simdisk/.rockbox/aura/moonlitcache/art/` → `.pfraw` generados; segunda ejecución sin decodificar JPEG (log `DEBUGF` de `moonlit_art`).
**Cierre:** 3 capturas (central, desplazada, monograma), salida de `size`, listado de la caché, log de segunda ejecución. Commits: `feat: moonlit_flow (vertical axis) from aura_flow@22f44275`, `feat: moonlit_art pfraw cache (D-020)`, `feat: Marea screen, experimental (D-014, D-024)`.

### H7 — Medición en hardware, empaquetado y cierre
**Archivos:** instrumentación temporal `DEBUGF` en `moonlit_screen_marea.c` (se retira en el último commit), `DECISIONS.md` (D-025 resultado de la medición y decisión final de Marea; D-026 primer release), `docs/GUIA_FLASHEO.md`, `firmware/dist/README.md`, `CONTRATO-moonlit-studio.md` (SHA-256 reales en §B).
**Verificación:** `build_target.sh`; flasheo en iPod según `docs/GUIA_FLASHEO.md` (usuario); lectura de log serie/`DEBUGF` con 60 cuadros de Marea → promedio y máximo en ms; `firmware/tools/package_dist.sh` (sin `--release-tag`, árbol limpio) → `firmware/dist/{rockbox.ipod,rockbox.zip,bootloader-ipod6g.ipod,mks5lboot,checksums.txt,MODIFICATIONS.md,THIRD-PARTY-NOTICES.txt}`; `unzip -l firmware/dist/rockbox.zip | grep -c "fonts/moonlit-"` → 8; `cat firmware/dist/THIRD-PARTY-NOTICES.txt | grep -c "SIL OPEN FONT LICENSE"` → 2 y `Apache` → 1; reproducibilidad: dos `package_dist.sh` consecutivos → `checksums.txt` idénticos.
**Cierre:** tabla ms/cuadro (usuario aporta el log si no hay serie; si no es posible medir, D-025 registra "pendiente" y Marea sigue experimental), listado de `firmware/dist/`, diff vacío de los dos `checksums.txt`. Commits: `perf: Marea frame budget measured on device (D-025)`, `chore: package_dist assets + GPL boundary BOOT-1 (D-026)`. **Sin tag y sin push**: el release real es una sesión aparte (flujo dual-familia documentado en los CLAUDE.md hermanos).

---

## H. Preguntas abiertas

| # | Pregunta | Opciones | Recomendación |
|---|---|---|---|
| PA-1 | DM-003 dice "`aura_albumart.c` byte-idéntico", pero el archivo arrastra 7 cabeceras Aura y 12 símbolos (`AF/aura_albumart.c:39-46`). | (a) copiar solo la capa `.pfraw` de `aura_art.{c,h}` como `moonlit_art` (H0 lo inventaría); (b) portar `aura_albumart.c` + `aura_art` + `aura_style` + `aura_music` + `aura_settings` (~5 módulos, contradice DM-008/DM-009); (c) escribir la caché desde cero sobre `metro_thumbs.c` | **(a)** — respeta la intención de DM-003 (reutilizar la caché probada, D-224) sin importar el motor de temas. Registrar como D-020 en H0. |
| PA-2 | Prefijo de los archivos heredados de Metro: ¿renombrar `metro_*` → `moonlit_*` en masa? | (a) solo los nuevos y los que exponen nombre público (`metro_screen_about`, `metro_lang` strings), conservar el resto; (b) renombrar todo en H0 | **(a)** — preserva `git blame`, `MODIFICATIONS.md` y las 10 cabeceras heredadas byte-idénticas (`CLAUDE.md` Metro:56-60); el renombrado masivo no aporta funcionalidad. |
| PA-3 | `moonlit_tokens.h` generado: ¿commiteado o gitignorado como en AF (`.gitignore:39`)? | (a) commiteado (build del target sin Python; el diff muestra cambios de tokens); (b) gitignorado y regenerado en `build_*.sh` | **(a)** — `build_target.sh` de Metro no invoca Python (`build_target.sh:45-58`) y la reproducibilidad del `rockbox.zip` es más fácil de auditar con el header versionado. |
| PA-4 | Frontera GPL: ¿introducir la etiqueta `BOOT-n` en el contrato o seguir identificando bootloader/mks5lboot solo por SHA-256 como los hermanos? | (a) `BOOT-1` + SHA-256; (b) solo SHA-256 | **(a)** — la tarea exige versionado explícito; el SHA-256 cambia con cada recompilación aunque el fuente no cambie (RBVERSION embebido), así que no sirve como versión de fuente. |
| PA-5 | Tema claro: ¿moonlit tiene solo tema "night" en v1? `metro_settings.c:133` escribe `theme` y `sim_matrix.sh` itera tema×acento. | (a) solo "night", `theme` fijo, ajuste oculto; (b) "night" + "dawn" (claro) desde H3 | **(a)** — "calma nocturna" es el lenguaje; un tema claro duplica la paleta y las 6 superficies de elevación sin decisión de diseño escrita. Dejar "dawn" como D-0xx futura. |
| PA-6 | ¿Se conserva el rol `MFONT_ACCENT` (Libre Baskerville Italic 18) o se elimina para quedar en 7 roles y una licencia menos de archivo TTF? | (a) conservar; (b) eliminar | **(b)** — la itálica a 18 px no fue evaluada en el estudio de legibilidad (solo Regular); sin dato real, viola el espíritu de DM-005. Añadir después si H2 la mide ≥ 10,5 px. |
| PA-7 | Centinela de árbol instalado para Studio (`installedTreeSentinel`): ¿`fonts/moonlit-body-18.fnt` u otro archivo? | (a) `.fnt` del rol body; (b) un archivo dedicado `/.rockbox/aura/moonlit.id` | **(a)** — sigue el patrón de familia por fuente que ya usa Studio (`02-investigacion.md` §4, T7 "depende de la fuente centinela"); no añade archivos al contrato. |
| PA-8 | `convttf` sin control de hinting (HIPÓTESIS C.1): si el autohinter deforma las serifas a 22 px, ¿se parchea `convttf.c` (fuera de `apps/metro/`, `MODIFICATIONS.md`) o se sube `MFONT_HEADLINE` a 24 px? | (a) parche `FT_LOAD_NO_AUTOHINT` opcional por bandera; (b) subir a 24 px | **(a)** solo si H2 muestra deformación medible (tonos/cap-height); por defecto **no tocar** `convttf.c`. |
