# DECISIONS.md — Registro de decisiones técnicas (moonlit.aura)

Bitácora de decisiones de este repositorio, numeración **D-001** en
adelante. Es la **fuente de verdad**: ante discrepancia con cualquier
plan (`docs/plan/`, `docs/plans/`) manda este archivo.

- D-001…D-017 son las decisiones DM-001…DM-017 cerradas en fase 3
  (`docs/plan/00-decisiones-moonlit.md`, ahora histórico), copiadas sin
  cambio de fondo; solo se renumeró el prefijo `DM-` → `D-`.
- Referencias `M-NNN` apuntan a `DECISIONS-METRO-ARCHIVE.md` (bitácora
  heredada de Metro-Aura, solo lectura). Referencias `AF D-NNN` al
  `DECISIONS.md` de `Aura-Firmware`; `ST-NNN` al de `Aura-Studio`.
- Citas `ruta:línea` sin prefijo se refieren a este repo en
  `moonlit-fork-base` (2f1bd28a); `apps/metro/` abrevia
  `firmware/rockbox/apps/metro/`; `AF/` prefija rutas de Aura-Firmware
  en `7ec39edb` (`aura-upstream/main`).

---

# Fase 3 — decisiones previas al plan (D-001…D-017)

## Identidad y repositorio

**D-001 — Nombre y strings de familia.** Nombre visible del producto: **moonlit.aura**. Repositorio: `ricolinos/moonlit-aura` (público). `firmware_family: moonlit` en `/.rockbox/aura/aura.cfg` (mismo mecanismo que C3, `Metro-Aura/docs/COMPAT_STUDIO.md:16`). Árbol dormido: `/.firmware-moonlit/` (C27, D-326). Caché propia: `/.rockbox/moonlitcache/` — nunca `metrocache/`, para evitar la colisión de formatos descrita en C23 si ambos firmwares conviven en un mismo dispositivo.

**D-002 — Licencia y visibilidad.** GPL v2, repo público desde el primer commit, igual que los hermanos. `LICENSE` en raíz = copia de `firmware/rockbox/docs/COPYING` (patrón D-283). Avisos de terceros en `firmware/dist/THIRD-PARTY-NOTICES.txt` generados por `package_dist.sh` (§1.7) y pantalla "Acerca de" con URL de código fuente (GPL v2 §3), nota de no afiliación con Apple y créditos de fuentes e iconos.

**D-003 — Base del fork: Metro-Aura.** moonlit.aura se forkea de `Metro-Aura` (historial completo, no squash). Motivo: toda la investigación de fase 2 —checklist C1–C28, getter de acento, patrón de iconos binarios, `metro_settings_*`, `metro_sync.c`, `metro_lrc.c`— se validó contra esa base (§4). De `Aura-Firmware` se **cherry-pickean solo** estos módulos (commits exactos a determinar en el hito 0 del plan con `git log --follow`):
- `aura_flow.c/.h` (núcleo de proyección 1D, §6 H1),
- `aura_albumart.c/.h` (cfcache, §6 riesgo D-224),
- `design-system/generate.py` (supersampleo 16× + `MIN_INK_TONES`, §1.3) y el generador de tokens JSON→C (§5).
Todo lo demás de Aura-Firmware queda fuera.

## Tipografía

**D-004 — Familias (cierra P1).** Títulos: **Libre Baskerville** (SIL OFL 1.1). Texto y UI: **Montserrat** (SIL OFL 1.1, build **estática** `Montserrat-Regular.ttf` de github.com/JulietaUla/Montserrat — nunca la variable de Google Fonts, por el error de instancia peso-100 documentado en §1.6). Evidencia: x-height 0,530 vs 0,526 em (refuta H-T1), tamaños reales 9,6–34,8 KB por fuente (§1.2). Implementada en M2, commit `dfff6142`: `design-system/vendor/libre-baskerville/`, `design-system/vendor/montserrat/`.

**D-005 — Tamaño mínimo de rol.** Ningún rol de texto legible baja de **18 px nominal** (≈11,5 px cap-height proyectado, sobre el mínimo ISO 9241-303 de 10,5 px). El rol `caption` heredado a 14 px (9 px cap-height real medido, §2) **no se hereda**. Tamaños exactos por rol los fija el plan (sección C) dentro de esta regla. Implementada en M2, commit `33a3499e`: `apps/metro/moonlit_fonts.{c,h}` (7 roles, ninguno < 18 px), `firmware/tools/check_fonts.py --capheight`.

**D-006 — Umbral de trazo visible (cierra P9).** En la verificación mecánica de `.fnt`/capturas, cuenta como trazo un píxel con luminancia **> 60/255**, medido sobre captura del simulador con PIL — el método ya calibrado en el estudio de legibilidad. Implementada en M2, commit `33a3499e`: `firmware/tools/check_fonts.py` (`INK_THRESHOLD = 60`).

**D-007 — Slots de fuente y bug de rango (cierra P11/P12).** La jerarquía tipográfica se diseña con **≤ 12 roles** para no tocar `MAXUSERFONTS` (`firmware/export/font.h:51`). Solo si el plan demuestra que 12 no bastan se sube el define, con auditoría previa de buflib. `gen_fonts.sh` de moonlit corrige el bug `atoi("0x20")` (§1.1) usando rango **decimal 32–383**. La corrección en Metro-Aura y Aura-Firmware es trabajo de sesión propia por repo: el plan la lista como prompt aparte etiquetado `[Metro-Aura]` / `[Aura-Firmware]`, no la ejecuta. Implementada en M2, commit `33a3499e`: `design-system/generate.py` (`--fonts`, `convttf -s 32 -l 383`), `firmware/tools/gen_fonts.sh` (wrapper delgado).

## Iconos, temas y tokens

**D-008 — Iconos binarios compilados (cierra P4).** Dibujo con el patrón Metro: tabla C commiteada, máscara de cobertura de 8 bits, `metro_fb_plot_alpha()` (`metro_fb.c:116-127`), cero lecturas de disco en runtime. Generación con la metodología Aura: SVG → `rsvg-convert` → supersampleo 16× + filtro de caja → verificación `MIN_INK_TONES ≥ 4` que rompe el build (`generate.py:475`). Fuente de iconos: **Material Symbols** (Apache 2.0). Solo se generan los `icon_key × tamaño` que el plan enumere, no el Cartesiano 89×9 de Aura. Implementada en M3 (D-033): `design-system/generate.py` (`--icons`), `apps/metro/moonlit_icons.{c,h}`, `apps/metro/moonlit_icons_table.c`.

**D-009 — Formato de tema v1: omitido (cierra P3).** `aura.cfg` **no** declara `theme_format_supported`, igual que Metro (`COMPAT_STUDIO.md:16`). Consecuencia directa de D-008: cumplir v1 exigiría 801 máscaras en disco (§4, T6). Studio deshabilita instalación de temas limpiamente. Vigente sin cambio de código en M1…M9: es la ausencia heredada del fork (`moonlit-fork-base`), confirmada en H1 (`docs/screenshots/H1-aura-cfg.txt`, sin la clave) y documentada en `CONTRATO-moonlit-studio.md` §A.1/§A.7.

**D-010 — Tokens en un solo origen (cierra P6).** `design-system/tokens.json` es la única fuente de verdad de color, escala tipográfica, espaciado, radios y niveles de elevación. `design-system/generate.py` produce `firmware/.../moonlit_tokens.h` y las tablas de iconos. Ningún literal RGB fuera de `tokens.json`; el acento se lee en runtime por getter (patrón `metro_color_accent()`). Licencias de assets vendoreados en `design-system/vendor/<asset>/LICENSE*`. Implementada en M1 (origen de datos) y M4 (getter en runtime), commit `0173cf16`: `apps/metro/moonlit_palette.{c,h}`, `design-system/tokens.json`.

## Lenguaje visual

**D-011 — Subconjunto Material sin GPU.** Se aprueban exactamente los 15 principios confirmados con primitiva real en §1.5 (blend entero, elevación por tono, esquinas redondeadas paramétricas, transiciones slide/blend ≤ 300 ms bajo `lcd_active()`, easing por tabla, alfa plano constante para "vidrio", grilla 8 dp = 8 px a `LCD_DPI 160`). Quedan **prohibidos**: blur, ripple, sombras difusas, easing bezier en runtime, rasterización vectorial en runtime. "Listas con divisores" (el 16.º principio) se aprueba condicionado a que el hito correspondiente cite la primitiva de blit de 1 px. Implementada en M4, commit `a5660eec`: `apps/metro/metro_screen_hub.c`, `metro_screen_list.c`, `metro_screen_settings.c`, `metro_draw.c` (hub/lista/ajustes/barra de estado sobre tokens MD3).

**D-012 — Waning Crescent: luz desde la izquierda.** Toda elevación se simula con **dos tonos por nivel**: borde izquierdo/superior un paso más claro (luz), borde derecho/inferior un paso más oscuro (sombra), ambos derivados de la paleta por `tokens.json`, nunca calculados por cuadro. Sin gradientes por píxel fuera de `lcd_active()`. Implementada en M4, commit `6e0d7508`: `apps/metro/moonlit_elevation.{c,h}`.

**D-013 — Fondo del reproductor (cierra P5).** Plano tonal Material: superficie de elevación derivada de la paleta nocturna, sin decodificar ni promediar la portada. Costo por cuadro cero. Implementada en M5, commit `336af973`: `apps/metro/metro_screen_nowplaying.c`.

## Cover Flow vertical

**D-014 — "Marea" entra en el plan, con restricciones (cierra P7 y §6).** Nombre: **Marea**. Se implementa **sin reflejo** (`reflection_buf` eliminado → 33.800 B/slot en vez de 42.120) y **sin morphs**. Geometría vertical retuneada a ~5 tapas visibles en 220 px útiles. Módulo de dibujo reescrito en espejo (fila-por-fila), no transpuesto. Sin portada → monograma (inicial del álbum en Libre Baskerville) sobre color de acento. H3 (≤ 33 ms/cuadro) **no se da por cerrada en simulador**: el plan incluye un hito de medición en hardware real; hasta entonces Marea es funcionalidad "experimental" en `DECISIONS.md`. Implementada en M6 (motor), commit `390d6099`: `apps/metro/moonlit_flow.{c,h}`; y M8 (pantalla), commit `114b932d`: `apps/metro/moonlit_screen_marea.{c,h}`. Medición en hardware: pendiente, M12.

## Interacción

**D-015 — Play/Pause global (cierra P10).** No se decide sin cita. El hito 0 del plan incluye la lectura del mapeo de botones de Metro-Aura (`apps/keymaps/keymap-ipod.c` y el shell Metro) y el comportamiento resultante se cierra en `moonlit-aura/DECISIONS.md` con ruta:línea antes de cualquier cambio de UI. Cerrada sin cambio de código en Hito H0 (ver D-018: comportamiento ya heredado de Metro-Aura M-071, `metro_keymap.c:31-33,55,88`). Ningún hito M1…M9 lo modifica.

## Logotipo

**D-016 — Waning Crescent.** Especificación vectorial original: luna menguante por **sustracción de dos círculos**, iluminada desde la izquierda (coherente con D-012), monocroma, dibujada en color de acento dinámico. Incluye **wordmark "moonlit"** en Libre Baskerville **solo** en arranque y "Acerca de" (≥ 64 px); en tamaños 16/24/40 px se usa únicamente la geometría. Verificación mecánica de tonos ≥ 4 en cada tamaño exportado antes de integrarse. No deriva de ningún icono existente. Implementada en M9, commit `bff717f8`: `design-system/logo/moonlit-crescent.svg`, `apps/metro/moonlit_logo.{c,h}` (detalle de integración por pantalla en D-044).

## Contrato con Aura Studio

**D-017 — Cambios en Studio no se ejecutan en este plan.** Los cambios a `FirmwareFamily.swift`, `ExtrasView.swift`, `fetch-firmware.sh`, `project.yml` y `FIRMWARE_VERSION.example` listados en §4 (T7) se entregan como un prompt aparte etiquetado `[Aura Studio]`. moonlit.aura garantiza C1–C28 por herencia (con los strings de D-001) y no renegocia `CONTRATO-firmware-studio.md` v13 ni `CONTRATO-dispositivo.md` v2.

---

# Hito H0 — auditorías de dependencias (D-018…D-022)

**D-018 — Play/Pause global: se hereda M-071 (cierra D-015).**
Evidencia leída: `apps/metro/metro_keymap.c:31-33` (`hub_mapping[]`,
comentario "R4/FA-8 (M-071)"), `:55` (`list_mapping[]`) y `:88`
(`player_mapping[]`) mapean `MACT_PLAYPAUSE` a
`BUTTON_PLAY | BUTTON_REL`; el manejo vive en
`metro_screen_list.c:252` y `metro_screen_hub.c:951`. `lock_mapping[]`
lo deja sin mapear a propósito (`metro_keymap.c:108-112`, M-068).
**Decisión**: PLAY alterna reproducción desde cualquier pantalla salvo
el candado y el visor de fotos (plugin). Marea (H6) añade su contexto
con el mismo mapeo. Sin cambio de código en H0.

**D-019 — `aura_wheel` se copia en H6 (cierra la HIPÓTESIS de A.1).**
`grep -rn "wheel_velocity\|wheel_step\|velocity" apps/metro/*.c
apps/metro/*.h` → **cero resultados**: Metro no expone velocidad de
rueda en grados/s. `AF/apps/aura/aura_wheel.c` tiene 51 líneas y su
único include es `aura_wheel.h` (`aura_wheel.c:23`); expone
`aura_wheel_step(int velocity_deg_s)` (`aura_wheel.h:48`) y
`aura_wheel_should_hop_letters` (`:56`). **Decisión**: copia de archivo
(`git show aura-upstream/main:…`) como `apps/metro/moonlit_wheel.{c,h}`
con prefijo renombrado, dependencias cero, en el hito H6 junto con
`moonlit_flow`. HIPÓTESIS pendiente para H6: de dónde sale
`velocity_deg_s` en Metro (en Aura lo calcula
`aura_main_wheel_velocity()`, `AF/aura_musicflow.c:1240`); si Metro no
tiene la fuente, H6 la porta o Marea usa pasos discretos de rueda.

**D-020 — Capa `.pfraw`: opción (a) de PA-1, alcance acotado.**
Inventario de `AF/apps/aura/aura_art.{c,h}` (109 + 206 líneas).
Includes de `aura_art.c:23-27`: `file.h`, `aura_art.h`,
`aura_settings.h`, `apple2026_shell.h`. Funciones que Marea necesita
(`aura_art.h:94-107`): `aura_art_read_pfraw`, `aura_art_write_pfraw`,
`aura_art_pfraw_is_cached`, `aura_art_transpose`,
`aura_art_mask_corners_transposed`. Símbolos externos que arrastran:
`aura_settings.theme` (campo `theme` del encabezado `.pfraw`,
`aura_art.c:91-105`), `a26_shell_isqrt256` y `a26_shell_blend`
(`aura_art.c:181,203`, solo en `mask_corners_transposed`).
**Decisión**: en H6 se copian esas 5 funciones a
`apps/metro/moonlit_art.{c,h}` con prefijo renombrado; `theme` del
encabezado se fija con un identificador de paleta de moonlit (única
paleta "night" en v1 — PA-5 recomendación (a), se cierra en H3);
`a26_shell_blend` se sustituye por `metro_fb_blend_color()`
(`apps/metro/metro_fb.h:82`, M-083) y `a26_shell_isqrt256` por una
copia local de la raíz entera (sin dependencia del shell Apple2026).
Ninguna otra parte de `aura_albumart.c` entra. Esto es la desviación
acotada de D-003 ("byte-idéntico") que PA-1 pedía cerrar antes de H6.

**D-021 — Riesgo M-004 heredado íntegro.** Leído
`DECISIONS-METRO-ARCHIVE.md` M-004 (cuerpo completo, incluida la
actualización M-056): Aura Studio anterior a ST-045 compara hashes del
`rockbox.ipod` y puede ofrecer "actualizar" el iPod a Aura; Studio con
ST-045 distingue familias por `firmware_family`, pero **no conoce la
familia `moonlit`** hasta que se ejecute el prompt `[Aura Studio]`
(D-017). **Decisión**: la advertencia se hereda y se amplía en
`docs/GUIA_FLASHEO.md` y `README.md` en H1 ("un Studio que no conozca
`moonlit` puede ofrecer volver a Aura o a Metro"). No hay mitigación
posible desde este repo.

**D-022 — `MAXUSERFONTS` no se toca.** `firmware/rockbox/firmware/
export/font.h:51` define `MAXUSERFONTS 12`. Metro usa 5 roles
(`apps/metro/metro_fonts.h:31-36`, `MFONT_COUNT`). La jerarquía de
moonlit (plan §C.3) tiene 8 roles (7 si PA-6 elimina `MFONT_ACCENT`)
≤ 12. **Decisión**: se conserva el define; H2 no requiere auditoría de
buflib.

---

# Hito H1 — identidad en runtime y contrato (D-023…)

**D-023 — Ruta exacta de la caché propia: `/.rockbox/aura/moonlitcache/`
(precisa D-001).** D-001 abrevia la ruta como `/.rockbox/moonlitcache/`;
el plan (§F.1 §A.4, `CLAUDE.md`) y el código heredado la sitúan bajo
`.rockbox/aura/`: `metro_settings_metro_cache_dir()` compone
`METRO_DIR "/…"` con `METRO_DIR = ROCKBOX_DIR "/aura"`
(`apps/metro/metro_settings.c:39,216`). **Decisión**: se conserva el
padre `.rockbox/aura/` (COMPAT C23: árbol interno que Studio ignora y
que la limpieza entre familias borra junto con `photocache/`/
`cfcache/`) y solo cambia el nombre de la hoja: `metrocache/` →
`moonlitcache/`. Misma pasada: `firmware_family: moonlit`
(`metro_settings.c:131`) y árbol dormido `/.firmware-moonlit`
(`metro_settings.c:246`, contrato v10) — el cambio a Aura por renombre
(M-090) queda intacto salvo por el nombre del árbol saliente. La única
función que compone la ruta de caché sigue siendo
`metro_settings_metro_cache_dir()` (regla de rutas de `CLAUDE.md`);
`metro_thumbs.c` no cambia de código, solo de comentario.

**D-024 — `CONTRATO-moonlit-studio.md` v1: cierra PA-4 y PA-7.** El
contrato propio referencia (no copia) `CONTRATO-firmware-studio.md`
v13, `CONTRATO-dispositivo.md` v2 y `library-layout-v1.md` v1.3 de
`Aura-Firmware`. Decisiones que fija: (a) **PA-4 → opción (a)**: la
frontera GPL se versiona con la etiqueta `BOOT-1` además del SHA-256,
porque el hash cambia en cada recompilación (RBVERSION embebido) y no
sirve como versión de fuente; sube a `BOOT-2` solo si cambia
`bootloader/ipod-s5l87xx.c` o `utils/mks5lboot/`. (b) **PA-7 → opción
(a)**: centinela de árbol instalado `/.rockbox/fonts/moonlit-body-18.fnt`
(patrón "familia por fuente" que Studio ya usa); H2 materializa el
archivo. (c) Los cambios en Studio siguen siendo prompt aparte
(`docs/plans/PROMPT-aura-studio.md`, D-017); la corrección del rango
de `convttf` en los hermanos, `docs/plans/PROMPT-hermanos-gen-fonts.md`
(D-007). Numeración: este registro desplaza en uno los D-0xx que el
plan §G cita como orientativos para H6/H7 (D-024 → D-025, etc.).

**D-025 — Ruptura latente de `make.dep` en `mpegplayer` (heredada, corregida
en H1).** `firmware/rockbox/tools/make.inc:42` genera dependencias con
`$(CC) -MG -MM … $(CFLAGS)`; `apps/plugins/mpegplayer/mpegplayer.make:31`
añade `-I$(APPSDIR)/metro` solo a `MPEGCFLAGS`, no a `CFLAGS`. Resultado:
`#include "metro_palette.h"` (`mpegplayer.c:108`, M-059) se anota como
`$(BUILDDIR)/metro_palette.h` y un build desde cero falla ("No rule to
make target"). Metro-Aura no lo vio porque su `make.dep` (19 ago 04:02)
es anterior a M-059 (19 ago 16:47) y `make` no lo regenera. **Decisión**:
include relativo `"../../metro/metro_palette.h"` (un solo archivo, ya
listado en `MODIFICATIONS.md`), no un `-I` global. H3 vuelve a tocar
esta línea al sustituir `metro_palette.h` por `moonlit_palette.h`; el
prompt `[Metro-Aura]` de `docs/plans/PROMPT-hermanos-gen-fonts.md`
debería recibir este hallazgo como ítem aparte (no se ejecuta aquí).

**D-026 — Wordmark provisional "moonlit.aura" hasta H5 (decisión del
dueño, cierre de H1).** El par "metro"/"aura" de M-092 se retira ya, no
en H5: `gen_logo.py` genera `rockboxlogo.320x98x16.bmp` con la única
línea "moonlit.aura" (todavía en Selawik Light, que H2 elimina junto con
`fonts-src/`; en ese hito el generador pasa a Libre Baskerville o se
retira si H5 ya está cerca), `metro_screen_splash.c` dibuja
`LANG_WORDMARK` = "moonlit.aura" en una sola línea, y la pantalla USB
usa el mismo bitmap por máscara (`metro_screen_usb.c`). D-016 sigue
vigente: el wordmark definitivo en Libre Baskerville y el logotipo
llegan en H5. Además, el prompt `[Aura Studio]` incorpora
`moonlitcache/` a la limpieza de convivencia entre familias (C23).

# Hito M1 — tokens MD3 y generador JSON→C (D-027…D-031)

Cierran `docs/plan/04-auditoria-brecha.md` (H2 nunca ejecutado) y abren
`docs/plan/05-plan-correctivo.md`. Fijadas en la sesión que redactó ese
plan (§II.0), registradas aquí al ejecutar M1.

**D-027 — Dos esquemas MD3 desde el primer hito.** `night` (oscuro,
predeterminado) y `dawn` (claro). Mapean al ajuste `theme` existente
(`metro_theme.h:30-33`, resolvers `metro_theme.c:67-87`; persistencia en
`metro_settings.c` queda para M4). Sustituye a PA-5 del plan 03.
Implementada en M1: `design-system/tokens.json` (`color.night`,
`color.dawn`).

**D-028 — Vocabulario MD3 obligatorio en tokens.** 16 roles por esquema:
`primary`, `on_primary`, `primary_container`, `on_primary_container`,
`surface`, `surface_dim`, `surface_bright`, `surface_container_lowest`,
`surface_container_low`, `surface_container`, `surface_container_high`,
`surface_container_highest`, `on_surface`, `on_surface_variant`,
`outline`, `outline_variant`. El acento dinámico de Metro **es**
`primary` (los 4 presets de `color.primary_presets` —moonstone, tide,
ember, moss— sustituyen a los 10 acentos WP7 de `enum metro_accent`,
`metro_theme.h:35-47`; el remplazo del enum en C queda para M4). La
escala `bg/surface_0..2` del plan 03 §B.1 se descarta. Elevación tonal =
niveles `surface_container_*` (MD3) más el par luz/sombra de D-012,
precalculado por `generate.py` (`MOONLIT_<esquema>_<nivel>_EDGE_LIGHT` /
`_EDGE_SHADOW`, delta de `tokens.json:elevation`). Implementada en M1:
`design-system/tokens.json`, `design-system/generate.py`
(`generate_header`, `rgb_defines`, `edge_rgb`), verificado en
`firmware/rockbox/apps/metro/test/test_tokens.c`.

**D-029 — Marea convive con Álbumes.** Pivote nuevo en `music_page`
(`music_pivots[]`, `apps/metro/metro_screen_hub.c:641-663` — la ruta
`:606-623` citada originalmente quedó desfasada); la rejilla se
conserva. Se registra en M1 (decisión de alcance); se implementa en M8
como último pivote; D-051 lo mueve al primero.

**D-030 — Layout de Marea.** Columna de portadas a la izquierda
(x ∈ [0,152)), información del álbum a la derecha (x ∈ [160,320)):
título en `MFONT_HEADLINE`, artista en `MFONT_BODY`, "N canciones" en
`MFONT_LABEL`. Sustituye la geometría centrada de plan 03 §D.1. Se
registra en M1 (decisión de alcance); se implementa en M8.

**D-031 — Nombres de archivo de fuente por rol.** `moonlit-<rol>-<px>.fnt`
(el centinela `moonlit-body-18.fnt` ya está en `CONTRATO-moonlit-studio.md`
§A.8 y en `docs/plans/PROMPT-aura-studio.md`; cambiarlo rompería el
contrato). El nombre de familia va en la cabecera del `.fnt` y en
`tokens.json`, no en el nombre de archivo. Se registra en M1 (decisión
de alcance); se implementa en M2.

# Hito M2 — Pipeline de fuentes: vendor OFL, `.fnt` y `moonlit_fonts` (D-032)

**D-032 — Cierra dos hipótesis de tamaño de `05-plan-correctivo.md` §M2
con datos reales, y documenta una discrepancia resuelta con el dueño.**

1. **Glifo faltante en Libre Baskerville-Regular.** El plan exige
   `firstchar=32 size=352` para los 7 `.fnt` (rango decimal 32–383,
   D-007). Al generar, `moonlit-display-40.fnt` (y title-28, headline-22,
   los 3 roles en Libre Baskerville) salieron con `size=351`, no 352.
   Causa verificada: `design-system/vendor/libre-baskerville/LibreBaskerville-Regular.ttf`
   (descargada de `github.com/impallari/Libre-Baskerville`, la fuente
   exacta que cita D-004) no trae el glifo U+017F ("ſ", *long s*,
   carácter histórico sin uso en español ni en ninguna cadena de
   `metro_lang.c`). `firmware/rockbox/tools/convttf.c:693`
   (`if ( !(charindex) ) continue;`) salta cualquier código sin glifo
   al barrer `32..383`, así que `lastchar` topa en U+017E (382) y
   `firmware/rockbox/tools/convttf.c:726`
   (`size = lastchar - firstchar + 1`) da 351. Montserrat sí trae
   U+017F completo (los 4 roles en esa familia dan 352). **Decisión
   (confirmada con el dueño, no unilateral):** se acepta `size=351`
   para los 3 roles Libre Baskerville como excepción documentada, no
   como umbral relajado sin registro — `design-system/generate.py`
   (`FONT_SIZE_EXCEPTIONS`) la aplica y falla si cualquier otro valor
   aparece. Sin impacto funcional.
2. **Tamaño real en bytes, incluida una segunda excepción no prevista
   por el plan.** `05-plan-correctivo.md` §M2 esperaba "todos < 60000
   salvo display-40". Medido (`stat -f %z`): `moonlit-body-18.fnt`
   26434, `moonlit-headline-22.fnt` 41375, `moonlit-label-18.fnt`
   27190, `moonlit-list-20.fnt` 34048, `moonlit-listsel-20.fnt` 36006 —
   los 5 bajo 60000 como esperaba el plan. Pero **`moonlit-title-28.fnt`
   también excede el umbral: 72187 bytes**, no anticipado por el plan
   (solo excluía display-40, que da 137979). Causa: la estimación de
   D-004 ("tamaños reales 9,6–34,8 KB por fuente") se midió con un
   conjunto de prueba más chico que el rango decimal 32–383 completo
   que D-007 exige aquí; a Libre Baskerville 28px con el charset
   completo le corresponden más glifos reales que a esa muestra. Sin
   consecuencia práctica: `font_load_ex(path, 0, 400)` (`moonlit_fonts.c`)
   carga el archivo completo sin el límite de 10 KB de `MAX_FONT_SIZE`
   (M-010 ya lo estableció para Selawik-display-48, 189 KB, mayor
   todavía). HIPÓTESIS C.1 de `04-auditoria-brecha.md` queda cerrada
   con estos números reales, no con la estimación original.

3. **Consumidor fuera de `apps/metro/` con rutas de `.fnt` hardcodeadas
   (hallado en revisión adversarial, no por el plan).** `apps/plugins/mpegplayer/mpegplayer.c:1400,1409-1412`
   (M-060, R2-F4) carga `metro-{caption-14,list-20,listsel-20,display-48,title-28}.fnt`
   por `#define` de ruta fija, para imitar la tipografía de Metro en su
   OSD y en `mpeg_settings.c`. El borrado de esos 5 archivos (M2, punto
   2 de arriba) los dejaba señalando a rutas inexistentes —
   `rb->font_load()` cae a -1 y los accessors devuelven `FONT_UI`
   (`mpegplayer.c:1428-1453`), así que no crashea, pero es una
   regresión visual silenciosa que ninguna definición de hecho de M2
   detecta. **Corrección:** los 5 `#define` pasan a
   `moonlit-{body-18,list-20,listsel-20,display-40,title-28}.fnt`
   (mapeo por rol: `MFONT_CAPTION` ahora es `MFONT_BODY` en
   `apps/metro/moonlit_fonts.h`, así que `metro-caption-14.fnt` mapea a
   `moonlit-body-18.fnt`, no a un archivo "caption" nuevo; `display`
   pasa de 48px a 40px, el tamaño MD3 real). Comentario inline
   `moonlit (D-032)` en el archivo. Sin cambios de geometría: el título
   en `metro_font_display()` (`mpeg_settings.c:272`) se posiciona por
   coordenadas fijas, no por altura de fuente, así que el cambio
   48px→40px es cosmético (coherente con el resto de la UI de Metro,
   que ya usa 40px en todos lados).

Implementada en M2: `design-system/generate.py` (`--fonts`,
`FONT_SIZE_EXCEPTIONS`), `firmware/tools/check_fonts.py`,
`firmware/assets/fonts/moonlit-*.fnt`,
`apps/plugins/mpegplayer/mpegplayer.c` (punto 3).

# Hito M3 — Iconos Material Symbols compilados con verificación de tonos (D-033)

**D-033 — Fuente exacta de los 20 SVG, tamaños generados y cierre de
una hipótesis de `05-plan-correctivo.md` §M3 con datos reales (dos
iconos, no el previsto).**

1. **Fuente de los SVG.** `github.com/google/material-design-icons`,
   rama `master`, variante `materialsymbolsrounded`, peso 400 / relleno
   0 / grado 0 por defecto (`symbols/web/<nombre>/materialsymbolsrounded/<nombre>_24px.svg`
   — el atributo `width`/`height` del SVG es solo una pista de tamaño;
   la geometría real vive en el `viewBox`, así que un único archivo por
   icono sirve para las tres tallas). `LICENSE` del mismo repo (Apache
   2.0) vendoreada junto a los SVG. Los 20 nombres son los de
   `03-plan-implementacion.md` §B.4, en ese mismo orden en
   `tokens.json:icon.names` y en `enum moonlit_icon_id`
   (`apps/metro/moonlit_icons.h`).

2. **`sync` a 16px, el riesgo que el plan preveía (§B.4: "si falla
   tonos a 16px, se usa variante Rounded weight 500"), pasó limpio: 34
   tonos.** Los que fallaron con el peso 400 por defecto fueron otros
   dos, **no anticipados por el plan**: `pause@24px` (3 tonos) y
   `battery_full@24px` (2 tonos) — ambos con `MIN_INK_TONES = 4`
   (D-008). Diagnóstico (`design-system/generate.py --icons`, tabla de
   cobertura impresa a mano): no es un bug del pipeline de
   supersampleo/filtro de caja (`generate_icons()`,
   `design-system/generate.py`) — es geometría real. Ambos íconos son
   casi enteramente bordes rectos (dos barras verticales; un contorno
   de batería con esquinas redondeadas mínimas) que, a 24px
   específicamente, caen casi exacto sobre la grilla de píxeles: el
   filtro de caja de 16× produce entonces una rampa de 2-3 valores en
   vez de una docena, porque casi no hay borde parcialmente cubierto.
   A 16px y 40px los mismos SVG pasan sin problema (la grilla no
   coincide igual de exacto). **Corrección, aplicando la misma
   mitigación que el plan ya preveía para `sync` (no una nueva
   política):** `pause.svg` y `battery_full.svg` se vendorean en su
   variante `wght500` (trazo más grueso) en vez de peso 400 — desplaza
   los bordes lo suficiente para que el filtro de caja capture rampa
   real en las tres tallas (verificado: 12/15/23 tonos y 11/10/15
   tonos respectivamente a 16/24/40px). Los otros 18 iconos quedan en
   peso 400. Sin cambio de `MIN_INK_TONES` ni de la metodología de
   generación — la verificación mecánica hizo exactamente lo que tenía
   que hacer.

3. **Enum y estructura de datos.** `struct moonlit_icon_mask {int
   width; int height; const uint8_t *cov;}`; tabla
   `moonlit_icons[MOONLIT_ICON_COUNT][MOONLIT_ICON_SIZE_COUNT]` (20×3 =
   60 máscaras); `moonlit_icon_draw(id, size_px, x, y, color)` en
   `apps/metro/moonlit_icons.c`, mismo cuerpo que
   `metro_widgets_draw_glyph()` (M-089) sobre `metro_fb_plot_alpha()`.
   Sustituye a `metro_icons.h`/`metro_icons_table.c` (máscaras
   monocromas de 1 bit, Fluent System Icons): 9 de los 20 iconos tienen
   consumidor hoy (`metro_screen_nowplaying.c`, `metro_draw.c`, vía
   `metro_widgets_draw_icon()`/`metro_widgets_draw_icon_in_circle()`,
   sin cambio de firma salvo el tipo del enum); los 11 restantes
   quedan compilados sin consumidor, reservados para M4/M5/M8.

4. **Alcance del grep de verificación "sin Fluent".** El propio hito
   M3 (`05-plan-correctivo.md` §M3) trae dos cláusulas en tensión: su
   definición de hecho exige
   `grep -rn 'METRO_ICON_\|metro_icons.h\|Fluent' apps/metro/ firmware/tools/`
   vacío, pero su "NO tocar" protege `metro_glyphs_table.c` explícitamente
   "hasta M5" — y ese archivo trae "Fluent System Icons (Microsoft), MIT"
   en su propia cabecera (`apps/metro/metro_glyphs_table.c:3`), igual que
   sus dos comentarios de contexto en `metro_screen_usb.c:38,129`
   (pantalla USB, alcance de M5, no de M3). El grep tal como está
   escrito no puede dar vacío mientras esa protección siga vigente.
   **Resolución (consultada con el dueño):** el grep se interpreta
   acotado al pipeline de iconos que M3 posee — excluye
   `metro_glyphs_table.c` y `metro_screen_usb.c` (ambos de M5). Las
   únicas dos menciones sueltas de "Fluent" en archivos que M3 sí
   posee (comentario de layout en `metro_draw.c:157`, comentario de
   `moonlit_icons.h:28`) se reescribieron para no nombrar la familia
   anterior, ya que no estaban protegidas. M5 hereda la obligación de
   dejar el grep sin acotar en vacío al reemplazar el glifo USB.

Implementada en M3: `design-system/tokens.json` (`icon`),
`design-system/generate.py` (`--icons`, `generate_icons`,
`_rasterize_icon_alpha`), `design-system/vendor/material-symbols/`,
`apps/metro/moonlit_icons.{c,h}`, `apps/metro/moonlit_icons_table.c`,
`firmware/tools/check_tones.py`. Elimina: `firmware/assets/icons/`,
`firmware/tools/gen_icons.py`, `apps/metro/metro_icons.h`,
`apps/metro/metro_icons_table.c`.

# Hito M4 — Paleta MD3, elevación tonal y pantallas base (D-034…D-038)

**D-034 — Los 4 tonos WP7 heredados se resuelven por rol MD3, no se retiran.**
`metro_color_bg()`/`metro_color_fg()`/`metro_color_secondary()`/`metro_color_tertiary()`
(`metro_theme.c`) siguen existiendo con el mismo nombre — los llaman
`metro_screen_nowplaying.c`, `metro_screen_lock.c` y `metro_screen_usb.c`
(M5, fuera de alcance de M4) sin cambios — pero ya no leen
`metro_palette.h`: delegan en `moonlit_color()` (`moonlit_palette.c`)
con el mapeo `bg→surface`, `fg→on_surface`, `secondary→on_surface_variant`,
`tertiary→outline`. `metro_color_accent()` (que sí tenía cero razón de
sobrevivir como función propia, D-028 ya lo resuelve como
`moonlit_color(MROLE_PRIMARY)`) se retira como función y queda como
`#define metro_color_accent moonlit_color_accent` en `moonlit_palette.h`
— mismo patrón de alias que `moonlit_fonts.h` usó con `MFONT_CAPTION`
en M2 — hasta que M11 lo retire. `metro_accent_color(enum metro_accent)`
(cero llamadores, verificado por grep) se elimina sin reemplazo.

**D-035 — Excepción de "único includer" para plugins de Rockbox
(discrepancia consultada con el dueño).** El plan (`05-plan-correctivo.md`
§M4) fija dos cosas en tensión: "`apps/metro/moonlit_palette.c/.h` —
único includer de `moonlit_tokens.h`" y, dos párrafos después, "el
include de `mpegplayer.c:113` [metro_palette.h] → `moonlit_palette.h`".
`moonlit_palette.h` no es un header puro (declara funciones
implementadas en `moonlit_palette.c`) y un plugin de Rockbox
(`mpegplayer.rock`) no enlaza objetos de `apps/metro/` — solo su propio
código más la tabla `rb->` (`mpegplayer.make:27-31`, comentario
preexistente de M-059/D-025) — así que seguir la instrucción al pie de
la letra rompe el link del plugin. **Resolución (consultada con el
dueño, tres opciones ofrecidas):** `mpegplayer.c` incluye
`../../metro/moonlit_tokens.h` directo — sigue siendo un header puro de
`#define` (ningún `.c`), igual que `metro_palette.h` antes — y la regla
de "único includer" se entiende acotada a `apps/metro/`: dentro de ese
árbol solo `moonlit_palette.c` lo incluye (`grep -rln 'moonlit_tokens.h'
apps/metro/` → únicamente `moonlit_palette.c` cuenta como `#include`;
`metro_theme.h`, `moonlit_palette.h`, `moonlit_elevation.h`,
`metro_transitions.c`, `test/test_tokens.c` y `test/Makefile` solo
*mencionan* el nombre del archivo en un comentario o regla de Make, no
lo incluyen). `test/test_tokens.c` ya incluía `moonlit_tokens.h` desde
M1 (host test, excepción ya vigente). Colores de `mpegplayer.c`
reescritos a los 4 presets MD3 (`metro_accent_colors_night/dawn[4]`,
D-028) y a los roles de superficie noche/dawn — antes leía los 10
acentos WP7 planos de `metro_palette.h`.

**D-036 — Cierre de D-011 (listas con divisores): primitiva de blit
citada.** `metro_screen_list.c:56` (`draw_row_dividers()`),
`lcd_hline(METRO_DRAW_LEFT_X, LCD_WIDTH - METRO_DRAW_LEFT_X, y)` en
`outline_variant` (D-028), un borde por cada límite entre filas
visibles salvo el que cae dentro de la tarjeta de la fila seleccionada
(la tapa moonlit_draw_surface() de esa fila, D-012, ya la delimita).
D-011 pasa de condicionada a **aprobada**.

**D-037 — Duración de PUSH/POP desde `motion.transition_ms`.**
`metro_transitions.c` no puede incluir `moonlit_tokens.h` (D-035); la
constante `METRO_TRANSITION_MS 220` se repite ahí como literal
documentado. Con `HZ=100` fijo (`firmware/kernel/include/tick.h`) y
`frame_delay=3` ticks (30 ms/cuadro) sin cambiar, `animations=all` pasa
de 8 a 7 cuadros (240 ms → 210 ms, más cerca de los 220 ms del token
que el valor anterior, que no venía de ningún token). `animations=minimal`
no cambia (4 cuadros).

**D-038 — Identidad en español de los esquemas y presets; retiro de
`MFONT_CAPTION`.** `LANG_VALUE_DARK/LIGHT` pasan de "oscuro"/"claro" a
**"noche"/"amanecer"** (`metro_lang.c`) — los dos esquemas MD3 llevan la
identidad Waning Crescent, no solo una polaridad de contraste; en
inglés, "night"/"dawn". Los 4 presets de acento (D-028) se nombran
**piedra lunar / marea / ascua / musgo** (`LANG_ACCENT_MOONSTONE/TIDE/
EMBER/MOSS`); en inglés, moonstone/tide/ember/moss. `MFONT_CAPTION`
(compat temporal de M2) se retira de `moonlit_fonts.h`; los 22 sitios
que lo usaban pasan a `MFONT_LABEL` (header, subtítulos, valores,
tiempos, cejas volando en CONTINUUM) salvo el mensaje de lista vacía de
`metro_widgets_draw_empty_state()`, que pasa a `MFONT_BODY` (es una
oración completa, no un valor corto).

Implementada en M4: `apps/metro/moonlit_palette.{c,h}` (roles MD3,
presets, único includer de `moonlit_tokens.h` dentro de `apps/metro/`),
`apps/metro/moonlit_elevation.{c,h}` (tarjetas de elevación tonal con
esquinas por cobertura y borde luz/sombra de 1px), `metro_theme.{c,h}`
(D-034), `metro_draw.c` (barra de estado 20px sobre
`surface_container_lowest`; capa de estado de foco/selección en
`metro_draw_rows_ex()`), `metro_screen_list.c` (D-036),
`metro_screen_hub.c` (misma capa de estado en su propio bucle de
dibujo), `metro_screen_settings.c` (4 presets), `metro_lang.{c,h}`
(D-038), `metro_transitions.c` (D-037), `moonlit_fonts.h` (retiro de
`MFONT_CAPTION`), `firmware/tools/check_tones.py` (`--edge`),
`firmware/tools/sim_matrix.sh` (2 esquemas × 4 presets × 3 pantallas),
`apps/plugins/mpegplayer/*` (D-035). Elimina: `apps/metro/metro_palette.h`.

# Hito M5 — Ahora suena, candado, USB y splash sobre el sistema nuevo (D-039…D-040)

**D-039 — Ahora suena: fondo plano, tarjeta tonal de carátula (136 px,
no 120) y jerarquía tipográfica invertida.**

1. **Fondo (implementa D-013).** `metro_screen_nowplaying_show()`
   mezclaba 30% de la carátula (o foto del artista) sobre
   `metro_color_bg()` (F12/R4-FA-7, M-078). D-013 ya lo cerraba como
   "plano tonal Material... costo por cuadro cero" desde H1; M5 lo
   ejecuta: `load_background()` se retira, el fondo pasa a
   `metro_draw_clear()` liso. Los helpers de fondo de
   `metro_albumart.c` quedan sin llamador (D-013 permite conservarlos
   hasta M11).

2. **Tamaño de la carátula: discrepancia consultada con el dueño.**
   `05-plan-correctivo.md` §M5 pide "carátula 120×120", pero la
   constante real es `METRO_ALBUMART_SIZE 136`
   (`apps/metro/metro_albumart.h:35`), compartida con el decodificador
   de miniaturas del hub (`metro_albumart.c:205-221`, downscale a
   `METRO_TILE_SIZE 80`) — y el propio M5 dice "NO tocar hub (M4)".
   **Resolución (consultada con el dueño):** se mantiene 136, no se
   toca `metro_albumart.h`. La tarjeta de elevación
   (`moonlit_draw_surface(..., MSURFACE_BASE, corner_s)`) se dibuja a
   136×136, no 120×120.

3. **La tarjeta solo se ve sin carátula.** Con arte real,
   `lcd_bitmap()` cuadrado tapa la tarjeta redondeada por completo —
   `metro_fb.c` no tiene una primitiva de blit con esquinas
   redondeadas. Se dibuja en ambos casos de todos modos (el costo es
   un `fillrect` a ~1 Hz, no un bucle de animación) porque es la que sí
   se ve en el respaldo sin carátula: reemplaza el "acento sólido +
   inicial" de `metro_draw_tile()` (M-076) por tarjeta tonal + inicial
   en `primary`, mismo lenguaje que el monograma de Marea (`05-plan-correctivo.md`
   §M8 D.5).

4. **Jerarquía tipográfica.** El plan solo nombra fuente/color para
   "título" y "artista"; se interpreta que el título (no nombrado)
   toma el rol fuerte por default (`on_surface`) ya que solo "artista"
   se marca explícitamente `on_surface_variant`. Título:
   `MFONT_LIST`/secundario → `MFONT_TITLE`/`on_surface`. Artista:
   `MFONT_LIST_SEL`/`on_surface` (versalitas) → `MFONT_BODY`/
   `on_surface_variant` (conserva las versalitas de `metro_lang_upper()`,
   el plan no pide retirarlas). Álbum, fuera de alcance de M5, no
   cambia. Invierte a propósito el orden WP7/Zune de M-083 ("la línea
   fuerte es quién") — MD3 encabeza con QUÉ suena; posiciones Y sin
   cambiar (el plan no pide reordenar líneas).

5. **Barra de progreso.** `metro_draw_progress()` (compartida por
   Ahora suena y el splash) pintaba la pista en `metro_color_tertiary()`
   (`outline`) — un tono de contorno, no de superficie. Pasa a
   `surface_container_highest`, el relleno sigue en `primary`.

**D-040 — Retiro de `metro_glyphs_table.c`/`metro_widgets_draw_glyph()`;
USB usa `moonlit_icon_draw()`.** El glifo `arrow_sync` de Fluent System
Icons (M-089), la única razón por la que `metro_glyphs_table.c` y
`metro_glyphs.h` sobrevivieron a M3 (D-033 punto 4, "se conserva hasta
M5"), se sustituye por el ícono `usb` de Material Symbols a 40 px
(`moonlit_icons.h`, ya compilado desde M3 sin consumidor). Retirados
sin reemplazo: `metro_glyphs_table.c`, `metro_glyphs.h`,
`metro_widgets_draw_glyph()` (`metro_widgets.c/.h`) — ningún llamador
les sobrevive. `apps/SOURCES` pierde la entrada
`metro/metro_glyphs_table.c` (fuera de `apps/metro/`, ver
`MODIFICATIONS.md`). El grep "sin Fluent" de D-033 punto 4, acotado en
M3 porque este archivo estaba protegido, ahora da vacío sin acotar.

Implementada en M5: `apps/metro/metro_screen_nowplaying.c` (D-013,
D-039), `apps/metro/metro_draw.c` (pista de `metro_draw_progress()`,
D-039), `apps/metro/metro_screen_usb.c` (D-040),
`apps/metro/metro_widgets.{c,h}` (retiro de `metro_widgets_draw_glyph()`),
`docs/moonlit-design-system/componentes/{ahora-suena,candado,usb}.md`.
`metro_screen_lock.c` y `metro_screen_splash.c` no cambiaron de código
(ya resolvían color/fuente por rol MD3 desde M2/M4); solo se
verificaron con captura. Elimina: `apps/metro/metro_glyphs_table.c`,
`apps/metro/metro_glyphs.h`.

# Hito M6 — Motor `moonlit_flow` vertical + pruebas en host (D-041)

**D-041 — Copia de `aura_flow.c`/`aura_wheel.c` desde
`aura-upstream/main` @ `7ec39edb` (2026-08-23), giro de eje mínimo,
cierre de la HIPÓTESIS de D-019.**

1. **Trazabilidad.** `git show aura-upstream/main:firmware/rockbox/apps/aura/aura_flow.c`
   (229 líneas) y `aura_flow.h` (136), `aura_wheel.c` (51) y
   `aura_wheel.h` (58) — tamaños idénticos a los citados en
   `04-auditoria-brecha.md` I.4 y `05-plan-correctivo.md` §M6. Cada
   archivo nuevo lleva en cabecera `moonlit: derived from <archivo>
   @ aura-upstream 7ec39edbf7cbe8547afa55880336ecdf2f890104`.

2. **Giro de eje en `moonlit_flow` (`aura_flow.c` → `moonlit_flow.c`).**
   Solo los símbolos atados a la geometría horizontal cambian de
   nombre/valor: `AURA_FLOW_SCREEN_W 320` → `MOONLIT_FLOW_AXIS_LEN 220`
   (240 − 20 px de barra de estado, D-030); `AURA_FLOW_DISPLAY_W 128`
   → `MOONLIT_FLOW_DISPLAY_LEN 120` (tapa central, D-030);
   `DISPLAY_LEFT_R`/`MAXSLIDE_LEFT_R` → `..._TOP_R`; el campo
   `proj->screen_x` → `screen_y`; `aura_flow_source_column()` →
   `moonlit_flow_source_row()`; `aura_flow_vertical_scale()` →
   `moonlit_flow_cross_scale()`. `MOONLIT_FLOW_CAM_DIST 240` se
   conserva sin retunear — **HIPÓTESIS** abierta hasta M8, cuando haya
   pantalla real donde compararlo. `diff` contra el original
   sed-renombrado: 58 líneas (`<` + `>`), bajo el tope de 60 del plan.
   El resto del módulo (punto fijo, tabla de seno, `allowed_shift`,
   la nota D-219 sobre `clz32`) es copia literal — no se reescribió
   nada que no dependiera del eje.

3. **`moonlit_wheel` — copia literal (cierra D-019).** `aura_wheel.c`
   no depende de orientación de pantalla (opera sobre grados/seg
   escalares); prefijo `aura_wheel_`→`moonlit_wheel_` es el único
   cambio.

4. **HIPÓTESIS de D-019 cerrada: mismas unidades que Aura.**
   `firmware/target/arm/ipod/button-clickwheel.c:194` (compartido por
   ipod6g, `HAVE_WHEEL_ACCELERATION` en `config/ipod6g.h:72`):
   `v = (v * 360) / WHEELCLICKS_PER_ROTATION; /* conversion to
   degree/sec */`, acumulado en `wheel_velocity` y empaquetado en el
   dato del botón como `(1<<31)|(1<<24)|wheel_velocity`
   (`button-clickwheel.c:237-238`). `button_apply_acceleration()`
   (`firmware/drivers/button.c:632-641`) extrae la velocidad con
   `data & 0xffffff` — mismo campo y misma unidad que
   `button_get_data() & 0xFFFFFF` en `aura_main_wheel_velocity()`
   (`AF/aura_musicflow.c:1240`). **Resolución: PC-1(a)** — mismas
   unidades, sin tabla de conversión. `apps/metro/metro_input.c` gana
   `metro_input_last_wheel_velocity()`: guarda `get_action_data() &
   0xffffff` en cada `MACT_PREV`/`MACT_NEXT` bajo
   `HAVE_WHEEL_ACCELERATION` (0 en target sin esa macro o sin evento de
   rueda todavía). Sin consumidor en M6 (Marea llega en M8) — expuesto
   para que `moonlit_wheel_step()` lo use entonces.

5. **`test_flow.c`/`test_wheel.c`.** Los 7 casos de `test_flow.c` se
   adaptan al eje (`screen_y`/`AXIS_LEN`); `test_realistic_side_slide_layout`
   pasa de 3 tapas (centro + 1 por lado) a 5 (centro + 2 por lado,
   D-030 "2 tapas por lado"), con offsets reescalados por
   `MOONLIT_FLOW_AXIS_LEN/AURA_FLOW_SCREEN_W = 220/320`.
   `test_wheel.c` es nuevo (Aura no tenía test host para
   `aura_wheel.c`): límites de `moonlit_wheel_step` (1..3, monótono) y
   del umbral de hojeo por letras.

Implementada en M6: `apps/metro/moonlit_flow.{c,h}`,
`apps/metro/moonlit_wheel.{c,h}`, `apps/metro/metro_input.{c,h}`
(`metro_input_last_wheel_velocity()`), `apps/metro/test/test_flow.c`,
`apps/metro/test/test_wheel.c`, `apps/metro/test/Makefile`,
`apps/SOURCES` (fuera de `apps/metro/`, ver `MODIFICATIONS.md`). Sin
pantalla ni consumidor todavía (M8).

# Hito M7 — Caché de portadas `moonlit_art` (D-042)

**D-042 — Cinco desviaciones acotadas de `05-plan-correctivo.md` §M7,
todas necesarias para que `apps/metro/test/test_art.c` compile y
enlace standalone con `cc` de host (mismo criterio que
`test_flow.c`/`test_wheel.c`, M6), registradas antes de codificar tras
confirmarlas con el dueño (`AskUserQuestion`, sesión de este hito).**

1. **Split `moonlit_art.c/.h` (D-020) vs. `moonlit_art_cache.c/.h`
   (nuevo).** `05-plan-correctivo.md:179-181` pone
   `moonlit_art_load_for_album()`/`moonlit_art_precache()` en el mismo
   archivo que las 5 funciones puras de D-020, pero esas dos llaman a
   `metro_music_albums()`/`metro_albumart_decode_track_cover_sized()`/
   `metro_settings_metro_cache_dir()` — módulos reales de Rockbox, no
   compilables con `cc` de host — mientras que `:190` exige
   `make -C apps/metro/test test → verde incl. test_art`, y el patrón
   ya establecido en `test/Makefile` (`test_flow`, `test_wheel`,
   `test_tokens`) enlaza solo el `.c` bajo prueba, sin mocks.
   **Decisión**: `moonlit_art.c/.h` se queda exactamente con el alcance
   de D-020 (formato `.pfraw` + horneado de esquinas, cero
   dependencias más allá de `file.h`/`lcd.h`); `moonlit_art_cache.c/.h`
   (nuevo, no host-testado, mismo criterio que `metro_thumbs.c`) recibe
   la resolución álbum→pista→píxeles y la precarga D-224. El propio
   `05-plan-correctivo.md:189` ("`grep -n 'moonlitcache'
   apps/metro/moonlit_art.c → vacío`") ya asume que la ruta de caché no
   vive en ese archivo — compatible con el split.

2. **`isqrt256()`/`blend()` copiados localmente en `moonlit_art.c`,
   no `metro_fb_blend_color()`.** D-020 decía "`a26_shell_blend` se
   sustituye por `metro_fb_blend_color()`" — pero esa función vive en
   `metro_fb.h`, que incluye `lcd.h` real (depende de `cpu.h`/
   `config.h`/`events.h`, no compilable con `cc` de host). **Decisión**:
   mismo criterio que D-020 ya aplicaba a la raíz entera ("copia local,
   sin dependencia del shell Apple2026") extendido al blend de un
   canal: `moonlit_art.c` trae su propia `isqrt256()`/`blend()`
   (fórmula RGB565 idéntica a `_RGB_UNPACK_*`/`LCD_RGBPACK` de
   `lcd.h:317-328`, ipod6g es `RGB565` plano, `config/ipod6g.h:85`, no
   `RGB565SWAPPED`).

3. **Cabecera `.pfraw`: campo 3 pasa de `theme` a `layout` (fijo en 1),
   campo 4 (`extra`) pasa a llevar el tema.** Mismo tamaño (4×int32,
   16 bytes) que `aura_art_pfraw_header`, sentido distinto: D-030 fija
   Marea en fila-contigua para siempre (nunca se necesita un segundo
   layout), así que el campo que en Aura discriminaba temas ahora
   discrimina layout (`MOONLIT_ART_LAYOUT_ROW_MAJOR`), y el campo
   `extra` (que en Aura era una segunda llave de invalidación de uso
   libre, D-020) lleva el tema activo (`metro_theme_get()`, D-027) —
   la única llave que Marea necesita. Cambiar de esquema invalida la
   caché (las esquinas se hornean contra `moonlit_color(MROLE_SURFACE)`,
   que sí varía night/dawn).

4. **PC-2 resuelta: opción (a).** `metro_albumart_decode_track_cover()`
   (`metro_albumart.c`) se generaliza a
   `metro_albumart_decode_track_cover_sized(path, out, size)` —
   decodifica siempre a `METRO_ALBUMART_SIZE` (136, el tamaño ya
   probado) y remuestrea las píxeles YA decodificados a `size` en vez
   de arriesgar un segundo decode de JPEG cerca de 120px (mismo riesgo
   de `JPEG_DECODE_OVERHEAD` que R3-F4/DD-5 ya evitaba para 80px).
   `metro_albumart_decode_track_cover()` original queda como
   envoltorio con `size=METRO_TILE_SIZE`; sus 3 llamadores no cambian.

5. **PC-3 resuelta: opción (a), enganchada en `metro_music.c`, no en
   una pantalla.** `05-plan-correctivo.md` sugería enganchar la
   precarga a la pantalla "Actualizando biblioteca…" o a la entrada a
   Marea — pero M7 prohíbe tocar pantallas (`§M7 "NO tocar: ...
   pantallas"`) y Marea no existe hasta M8. El propio "Parte de" de
   §M7 cita `AF/aura_music.c:221-300` como el patrón D-224: ese
   enganche vive en `aura_music_db_ready()` (capa de datos), no en una
   pantalla. **Decisión**: `moonlit_art_cache_on_db_ready()` se llama
   desde `metro_music_db_ready()` (`metro_music.c`, en el mismo bloque
   `!s_update_triggered` que ya dispara `tagcache_start_scan()`), con
   su propia bandera de una-vez-por-arranque. Sin pantalla de progreso
   en M7 (`progress_cb` queda `NULL` desde este enganche) —
   `moonlit_art_precache()` sí acepta un callback para cuando M8 (o una
   revisión posterior) decida mostrar progreso.

**Dos cachés de portadas distintas, a propósito (§M7 "NO tocar:
`metro_thumbs.c`").** `metro_thumbs.c` (`.mth`, 80 px, fila-contigua
simple sin esquinas horneadas) sigue siendo la única caché de la
rejilla de Álbumes/Quickplay (`metro_screen_hub.c`, `album_thumb_*`) —
M7 no la toca ni la reemplaza. `moonlit_art`/`moonlit_art_cache`
(`.pfraw`, 120 px, cabecera propia + esquinas horneadas al radio de
Marea) es una caché nueva y separada, solo para Marea (M8). Coexisten
bajo el mismo padre (`.../aura/moonlitcache/{albums,art}/`) porque
sirven tamaños y formatos de archivo distintos (D-020 ya fijaba que
`.pfraw` trae cabecera + esquinas horneadas, algo que `.mth` no
necesita) — unificarlas forzaría a la rejilla de 80px a cargar con el
costo de una cabecera y un horneado que no usa, o a Marea a decodificar
sin esquinas. `metro_thumbs.c` no gana ningún cambio de código en M7,
solo la constancia de por qué no se lo tocó.

Verificado en `firmware/build-sim`: `METRO_INSTALL_MUSIC_FIXTURES=1`
más `gen_test_media.sh` deja 8 álbumes con carátula resoluble
(`find_albumart()` también encuentra `Music/cover.jpg` como ancestro
de los álbumes sin `cover.jpg` propio) — primera corrida produce 8
`.pfraw` de 28 816 B (`16 + 120*120*2`) con 8 líneas `DEBUGF
"moonlit_art: decode"`; segunda corrida, mismo disco: 0 `decode`,
8 `hit`, mismo conteo de archivos.

Implementada en M7: `apps/metro/moonlit_art.{c,h}`,
`apps/metro/moonlit_art_cache.{c,h}`, `apps/metro/test/test_art.c`,
`apps/metro/test/file.h`, `apps/metro/test/lcd.h` (sustitutos de host),
`apps/metro/test/Makefile`, `apps/metro/metro_albumart.{c,h}`
(`metro_albumart_decode_track_cover_sized`), `apps/metro/metro_music.c`
(enganche en `metro_music_db_ready()`), `apps/SOURCES` (fuera de
`apps/metro/`, ver `MODIFICATIONS.md`). Sin pantalla ni consumidor
todavía (M8 la usa desde Marea).

# Hito M8 — Pantalla Marea (D-043)

**D-043 — Marea es funcionalidad experimental hasta la medición en hardware real de
M12: `MOONLIT_FLOW_CAM_DIST` (D-041) sigue sin retunear, y los tres
desvíos de este hito lo confirman.**

1. **Ángulo/separación lateral re-derivados de `aura_musicflow.c`,
   escalados a 120px.** `MF_ITILT`/`MF_OFFSETX_R`/`MF_SLIDE_SPACING_R`
   de Aura (`AF/aura_musicflow.c:581-583`) están calibrados contra
   `MF_COVER_SIZE=130`; D-030 fija la tapa de Marea en 120px.
   `MAREA_ITILT=199` se conserva sin cambio (documentado en Aura como
   "no depende del tamaño del slide"); `MAREA_OFFSETX_R=84900` y
   `MAREA_SLIDE_SPACING_R=26800` son `92000`/`29000` escalados por
   `120/130` (`apps/metro/moonlit_screen_marea.c`). Sin medición en
   dispositivo real que los confirme — HIPÓTESIS a retunear en M12
   junto con `CAM_DIST`, mismo criterio que D-041.

2. **Sin zoom-al-scrollear ni reflejo.** El zoom de Aura
   (`MF_ZOOM_SCALE_SHRUNK`/D-245/D-246/D-247) fue un encargo específico
   del dueño de *ese* producto, ausente de `05-plan-correctivo.md` §M8;
   el reflejo ya quedó fuera desde D-020/M7 (`moonlit_art` no lo
   genera). Ninguno de los dos se porta: `slide.distance` es siempre 0
   en `moonlit_screen_marea.c`, y `draw_slide_perspective()` muestrea
   solo la carátula, sin banda de reflejo.

3. **Tapas laterales sin carátula: relleno liso proyectado, no un
   segundo buffer de 120×120.** `05-plan-correctivo.md` no especifica
   qué dibujar cuando una tapa lateral (no la central) carece de arte.
   Reservar un `s_placeholder_cover[120*120]` (mismo patrón que
   `moonlit_art_cache.c:s_precache_cover`) sumaría 28 800 B de `.bss`
   extra — con el límite de hecho de M8 ya ajustado (`MAREA_CACHE_SLOTS`
   × 28 800 B ≈ 1 065 600 B de los 1 100 000 B permitidos), esos 28 800 B
   no caben. `draw_slide_flat()` reusa la misma proyección de fila
   (`moonlit_flow_begin_projection`/`_cross_scale`) pero rellena con
   `lcd_hline()` a un color plano (`primary_container` fundido hacia
   `surface` con el mismo `fade` que la carátula real usaría) — cero
   buffer adicional. La tapa CENTRAL sin arte sigue el camino que sí
   especifica D-030 D.5: tarjeta plana + inicial, dibujada directo (sin
   pasar por el motor), únicamente cuando `offset256==0` exacto.

**`.bss`:** `arm-none-eabi-size` — base (M7, `moonlit-fork-base..HEAD`
antes de M8) 7 474 076 B, con M8 8 569 948 B — crecimiento 1 095 872 B,
bajo el límite de 1 100 000 B de `05-plan-correctivo.md` §M8 por 4 128 B.

**Panel derecho, ancho 152 (no 160).** D-030 describe la región como
`x ∈ [160,320)`; el propio `05-plan-correctivo.md` §M8 "Crea" da la
llamada exacta `moonlit_draw_surface(160, 20, 152, 220, ...)`. Se
implementa el ancho literal de esa llamada (152, hasta x=312) — deja un
margen de 8px a la derecha simétrico al que separa la columna de
portadas (`[0,152)`) del panel (`[160,...)`), lectura no contradictoria
con el rango descriptivo de D-030.

**Decode nunca dentro de `show()`.** `get_slot_for()` solo lee el
`.pfraw` ya horneado por M7 (`moonlit_art_read_pfraw()`); un cache-miss
(tema recién cambiado, álbum sin precachear) cae al monograma y encola
el álbum. `moonlit_screen_marea_tick()` — llamada desde la rama ociosa
de `metro_main.c`, nunca desde el bucle de animación — decodifica como
mucho UNO por vuelta (mismo presupuesto que `metro_thumbs_tick()`,
DD-9) vía `moonlit_art_load_for_album()`.

Implementada en M8: `apps/metro/moonlit_screen_marea.{c,h}` (nuevo),
`apps/metro/metro_screen_hub.{c,h}` (pivote Marea, D-029;
`metro_screen_hub_albums()`/`metro_screen_hub_open_album_songs()`),
`apps/metro/metro_music.{c,h}` (`metro_music_song_count_of_album()`),
`apps/metro/metro_lang.{c,h}` (`LANG_MAREA_SONGS_FMT`),
`apps/metro/metro_main.c` (cuarto centinela `at_marea`, mismos 4
enganches que Now Playing/el visor de fotos), `apps/SOURCES` (fuera de
`apps/metro/`, ver `MODIFICATIONS.md`).

# Hito M9 — Logotipo Waning Crescent y wordmark (D-044)

**D-044 — Geometría de consumidores no fijada por D-016 ni por
`05-plan-correctivo.md` §M9: tres desvíos documentados aquí porque
tocan geometría compartida con otras pantallas.**

1. **Creciente del hub (40px): cabecera de marca propia, no una fila
   más.** `05-plan-correctivo.md` §M9 dice "hub = 40 px" sin coordenadas
   (a diferencia de D-030/M8, que fue exacta). El hub (`metro_screen_hub.c`)
   es una lista de filas WP7 sin iconos en ningún lado; la barra de
   estado mide 20px fijos en las cinco pantallas que la usan (D-034) y
   solo quedaban 12px libres entre ella y la primera fila. Decisión del
   dueño (pregunta abierta de esta sesión, opción "cabecera de marca
   propia"): una franja de 40px con el creciente entre la barra de
   estado y la primera fila. Costo aceptado explícitamente:
   `METRO_HUB_FIRST_Y` pasa de 32 a 84 (exactamente un
   `METRO_HUB_PITCH` más) y `METRO_HUB_VISIBLE` baja de 4 a 3 filas sin
   scroll — con música sonando o sin ella, "ajustes" ahora requiere
   desplazar. Mismo patrón "asoma cortado" que ya usan filas y pivots
   (`metro_draw.h`), solo que empieza una fila más tarde. Ningún otro
   `metro_screen_*.c` toca `METRO_HUB_FIRST_Y`/`METRO_HUB_PITCH`/
   `METRO_HUB_VISIBLE` — el costo es local al hub raíz.

   **Corrección tras revisión adversarial:** la primera versión de este
   punto dejaba `metro_screen_hub_show()` con `metro_draw_header("")`
   (la misma cadena vacía que candado/pantalla principal, que sí piden
   el creciente de 16px en la barra de estado) — el hub terminaba
   mostrando DOS crecientes apilados (16px arriba + 40px debajo,
   visible en la primera versión de `docs/screenshots/M9-hub.png`).
   Corregido distinguiendo `page_title == NULL` (sin texto NI marca —
   el hub, que ya trae la suya) de `page_title == ""` (sin texto, con
   marca de 16px — candado y pantalla principal) en
   `metro_draw_header()` (`metro_draw.c`); el hub pasa `NULL`.

2. **Creciente + wordmark de "Acerca de" (64px): bucle de filas propio,
   no `metro_draw_rows()`.** D-016 exige el wordmark en "Acerca de" a
   ≥ 64px; `metro_draw_rows()`/`metro_draw_rows_ex()` (`metro_draw.c`)
   dibujan siempre desde el `#define METRO_DRAW_ROWS_FIRST_Y 84` fijo
   (`metro_draw.h:45`), eje que además usa CONTINUUM para el cálculo
   `from_y` de la transición de entrada (`metro_screen_list.c:247`) y
   que comparten *todos* los demás pivotes de Ajustes (general,
   pantalla). Mover ese eje para un solo pivote habría exigido
   parametrizar `metro_draw_rows_ex()`/`metro_draw_tiles()` (8+ sitios
   de llamada) y auditar CONTINUUM — fuera de alcance de "solo el
   logotipo". En vez de eso, "Acerca de" recibe su propio bucle de
   dibujo (mismo patrón que ya usa el hub, "su propio bucle de dibujo",
   comentario junto a `draw_hub_row_card()`): `draw_about_hero()` +
   `draw_about_rows()` en `metro_screen_list.c`, identificando el
   pivote por `pivot->name == LANG_PIVOT_ABOUT` (comparar el puntero
   contra `&metro_screen_about_pivot` NO funciona:
   `metro_screen_settings.c:503` copia ese struct por valor dentro de
   `all_pivots[2]`, así que la dirección nunca coincide). Costo
   aceptado: sin cascada FEATHER ni divisores de fila en "Acerca de";
   las filas arrancan en y=160 en vez de 84, así que se ven menos filas
   sin desplazar (créditos y conteos de biblioteca ya asumían scroll
   con contenido largo, así que el costo es consistente con cómo esa
   pantalla ya se comporta).

3. **`sim_shot.sh … 30` de la definición de hecho no alcanza el splash
   de Metro.** Confirmado, no es hipótesis: `apps/main.c:294`
   (`sleep(HZ); /* sim is too fast to see logo */`, código stock de
   Rockbox, sin tocar por ningún commit moonlit) mantiene la pantalla
   de arranque *de Rockbox* — no la de Metro — visible varios cientos
   de ticks simulados antes de que `metro_main()` arranque. A 30 ticks
   el dump captura ese splash de Rockbox (con el logo oficial
   restaurado, ver abajo), no el creciente. Las capturas de evidencia
   de este hito (`M9-splash.png`) se tomaron a 100 ticks, donde el
   splash de Metro ya está activo y estable.

**Wordmark provisional D-026 retirado.** `firmware/tools/gen_logo.py`
eliminado; `LANG_WORDMARK` retirado de `metro_lang.h`/`.c` (sin más
consumidores). `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` — que
`gen_logo.py` sobrescribía con el wordmark de texto provisional — se
restaura al bitmap oficial de Rockbox (`git show
5c6da72d:firmware/rockbox/apps/bitmaps/native/rockboxlogo.320x98x16.bmp`,
el import sin modificar de F0): es el splash que dibuja `apps/main.c`
*antes* de que Metro tenga el control de la pantalla (punto 3 arriba),
ajeno al sistema de diseño de moonlit — no vale la pena mantenerlo
"neutro" a mano cuando el árbol ya trae el original correcto en su
propia historia. Registrado en `MODIFICATIONS.md` (fuera de
`apps/metro/`).

**Pantalla USB: creciente de 40px reemplaza el wordmark de bitmap.** El
mismo patrón de máscara de cobertura de 8 bits que ya usaba el ícono
"usb" de Material Symbols (D-040) — `moonlit_logo_draw_crescent()`
en vez del `draw_wordmark()` a mano que leía `bm_rockboxlogo` pixel a
pixel (retirado con el bitmap provisional).

Implementada en M9: `design-system/logo/moonlit-crescent.svg` (dos
círculos vía `<mask>`, D-016), `design-system/logo/moonlit-wordmark.svg`
("moonlit" en Libre Baskerville Regular a contornos, generado una vez
con fontTools y commiteado como asset estático), `design-system/tokens.json`
(`logo{crescent_sizes, wordmark_size}`), `design-system/generate.py`
(`--logo`: mismo pipeline de supersampleo 16x + filtro de caja que
`--icons`, más verificación de cobertura/cúspides a 16px, E.3),
`apps/metro/moonlit_logo.{c,h}` + `moonlit_logo_table.c` (generado),
`apps/metro/metro_screen_splash.c` (creciente 64 + wordmark, retira el
texto de D-026), `apps/metro/metro_screen_list.c` (hero + filas propias
de "Acerca de"), `apps/metro/metro_screen_hub.c` (cabecera de marca de
40px), `apps/metro/metro_draw.c` (creciente de 16px en la barra de
estado vacía), `apps/metro/metro_screen_usb.c` (creciente de 40px),
`apps/metro/metro_lang.{c,h}` (retira `LANG_WORDMARK`), `apps/SOURCES`
y `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` (fuera de
`apps/metro/`, ver `MODIFICATIONS.md`); elimina
`firmware/tools/gen_logo.py`.

# Hito M11 — Revisión adversarial global

**D-045 — Lectura de disco dentro del bucle de animación de scroll de
Marea, pendiente (hallazgo real de la revisión adversarial, no
corregido en este hito).** Un subagente independiente de M11, encargado
de refutar activamente "las restricciones vinculantes de `CLAUDE.md`"
(regla: "Ninguna lectura de disco dentro de un bucle de animación"),
encontró una violación real y reproducible:
`run_scroll_animation()` (`apps/metro/moonlit_screen_marea.c:512-538`)
está correctamente gateada por `lcd_active() &&
metro_settings.animations != METRO_ANIM_OFF` **antes** de entrar al
`for` (líneas 519-524, patrón de `hub_row_animates()`), pero el cuerpo
del `for` (línea 533) llama a `moonlit_screen_marea_show()` en cada
cuadro, que llama a `draw_slide()` (`:413`), que llama a
`get_slot_for()` (`:192`). Cuando el álbum que entra a la ventana
visible durante el scroll no está en los 37 slots de caché LRU
(`s_slots`), `get_slot_for()` (`:226`) ejecuta
`moonlit_art_read_pfraw()`, que en `moonlit_art.c:51,72` hace
`open()`+`read()` reales sobre el `.pfraw` cacheado en disco —
**dentro del bucle de animación por cuadro**, no antes de él.

El comentario del propio código (`:170-173`) distingue "decodificar un
JPEG" (prohibido en el bucle; el decode real vive en
`moonlit_screen_marea_tick()`, llamado desde `metro_main.c:458`, fuera
de cualquier bucle) de "leer un `.pfraw` ya horneado" (que sí ocurre
en el bucle) y cita "D-030/M8" como respaldo — pero D-030
(`:240-243` de este archivo) solo describe el layout de columnas de
Marea, no concede esa excepción. El texto vinculante de `CLAUDE.md` no
distingue tipos de lectura de disco.

**Por qué queda pendiente y no se corrige en este mismo hito (regla de
M11: corrección inline solo si es ≤ 20 líneas):** una corrección
correcta no es un parche mecánico de una línea. Requiere precalcular,
**antes** de entrar al `for` de `run_scroll_animation()`, la unión de
todos los índices de álbum que podrían volverse visibles en cualquier
punto entre `from_x256` y `to_x256` (no solo en el punto de llegada) y
forzar su lectura de `.pfraw` ahí — o, alternativamente, separar
`get_slot_for()` en una variante de solo-lectura-de-caché usada dentro
del bucle (que nunca golpea disco y cae a monograma en un miss, igual
que ya hace con un miss de JPEG) y una variante con lectura real usada
solo en la precarga previa al bucle y en `moonlit_screen_marea_tick()`.
Ambas rutas exigen decidir la ventana de precarga contra el
presupuesto fijo de `MAREA_CACHE_SLOTS` (37, D.3 del plan 03) y
verificarse contra scroll rápido de rueda (`step > 1`, D-019), algo
que ninguna definición de hecho de M8 ni M11 ejercita mecánicamente —
no es una decisión de diseño ya cerrada en `docs/plan/00-decisiones-moonlit.md`
ni en este archivo, así que no se inventa aquí. Severidad acotada en
la práctica: una sola pantalla (Marea, ya D-043 "experimental hasta
M12"), lectura de un archivo de tamaño fijo (28 816 B) ya horneado
(nunca un decode de JPEG), gateada por `lcd_active()`. Queda como
trabajo pendiente para un hito posterior o para M12 junto con el
retuneo de `MOONLIT_FLOW_CAM_DIST`.

**Cerrada en v0.1.1 (commit "Marea preloads visible covers before the
scroll loop", junto a D-049).** Se tomó la primera de las dos rutas de
arriba, con la segunda como red: `preload_range(from_x256, to_x256)`
(`apps/metro/moonlit_screen_marea.c:537`) carga en `s_slots` — con
disco permitido, ANTES del `for` de `run_scroll_animation()` (`:576`)
— la unión de las ventanas visibles entre origen y destino (cada
cuadro dibuja centro ± `MAREA_VISIBLE_RADIUS+1`; con
`moonlit_wheel_step()` ≤ 3 son a lo sumo 10 de los 37 slots, y la LRU
desaloja por distancia a `s_target_index`, que ya apunta al destino).
Dentro del bucle, `s_in_scroll_loop` (`:200`) veda a `get_slot_for()`
cualquier `moonlit_art_read_pfraw()`: un miss (que la precarga hace
imposible salvo tema cambiado a mitad de scroll) cae al slot LRU que
tocaba, dejado libre (`album_index = -1`) solo con el monograma — sin
buffer aparte: un `marea_slot_t` más son 28 816 B de `.bss` y D-043
fija el techo: `arm-elf-eabi-size` M11 = 8 569 948, v0.1.1 completa =
8 570 044 (+96 B de banderas, bajo el límite de 7 474 076 + 1 100 000 =
8 574 076; un primer intento con slot propio dio 8 598 844 y se
descartó) —, marca `s_scroll_missed` y el cuadro final se
repinta una vez fuera del bucle con disco permitido. La
lectura de `.pfraw` sigue existiendo en un solo sitio (`:248`,
`get_slot_for`), fuera de `run_scroll_animation()`
(`grep -n 'read_pfraw\|open(' moonlit_screen_marea.c` → solo `:248`).
`MAREA_CACHE_SLOTS` (37) no cambia. Verificado en el simulador con
`SELECT,SELECT,SCROLL_FWD×3` (Marea ya primer pivote, D-051): la pila
scrollea y la tapa central muestra portada, no monograma.

**Refutaciones de M11 que NO prosperaron (confirmadas por subagentes
independientes, uno por tema, sin ver el código/hallazgos del otro):**
1. *"moonlit no tiene sistema de diseño propio"* — no prospera. Cero
   `#define METRO_*` de color, cero `MFONT_CAPTION` restante, cero
   fuga de "metro" en UI (`metro_lang.c`), cero literal RGB fuera de
   `moonlit_tokens.h`. `design-system/tokens.json`/`generate.py`
   definen 16 roles MD3 propios con decisiones documentadas y
   verificadas contra el código (D-027…D-038).
2. *"Marea no parte de Aura-Firmware"* — no prospera. Trazabilidad
   real confirmada: cabeceras citan el sha de `aura-upstream`, el diff
   mecánico de `moonlit_flow.c` contra `aura_flow.c` da 58 líneas
   (< 60), y las adaptaciones documentadas en D-014/D-019/D-020/D-041/
   D-042/D-043 (proyección vertical, ejes renombrados, límites de
   `.bss`, sin zoom/reflejo) están realmente presentes en el código.
3. *"La frontera GPL del bootloader no se sostiene"* — no prospera.
   `git log --oneline moonlit-fork-base..HEAD -- firmware/rockbox/bootloader/
   firmware/rockbox/utils/mks5lboot/` vacío en ambas rutas. El único
   commit histórico que toca `ipod-s5l87xx.c` fuera de esa ventana
   (`7e7593c5`) es *anterior* a `moonlit-fork-base` y ya forma parte de
   la fuente congelada que la cláusula BOOT-1 de
   `CONTRATO-moonlit-studio.md` §B declara como base.
4. *"Las restricciones vinculantes no se cumplen"* — no prospera en
   general (idioma español por defecto, tonos de iconos/logo
   verificados sin `FAIL`, cero literal RGB real, ningún rol de fuente
   < 18px, cero uso real de las APIs prohibidas de Rockbox), **salvo**
   el hallazgo real de disco-en-bucle-de-animación de Marea documentado
   arriba en D-045.

---

# Release v0.1.0 — cambio de sistema entre tres familias (D-046…D-048)

**D-047 — "Cambiar sistema": submenú con una fila por familia hermana
(tabla de familias, tres familias, invariantes del contrato v10
intactas).** Hasta M11 moonlit solo sabía despertar a Aura ("cambiar a
Aura", heredado de Metro M-090) y las constantes se llamaban
`METRO_FW_DORMANT_METRO` aunque apuntaban a `/.firmware-moonlit`. Con
tres familias (Aura `/.firmware-aura`, Metro `/.firmware-metro`,
moonlit.aura `/.firmware-moonlit`; contrato canónico v14, §A bis
"registro de familias") la fila única no alcanza. **Decisión**:
1. Tabla pura de hermanas, sin I/O, en
   `firmware/rockbox/apps/metro/metro_firmware_families.h:34-46`
   (`struct metro_fw_family { dormant_dir; name }`,
   `metro_fw_sibling_count()`, `metro_fw_sibling(i)` → `NULL` fuera de
   rango) y `metro_firmware_families.c:27-30`
   (`{"/.firmware-aura", LANG_FAMILY_AURA}`,
   `{"/.firmware-metro", LANG_FAMILY_METRO}`). El propio árbol dormido
   es `METRO_FW_OWN_DORMANT` (`metro_firmware_families.h:40`,
   `"/.firmware-moonlit"`, D-001) y **nunca** es una fila: no se
   cambia a uno mismo. Registrada en `firmware/rockbox/apps/SOURCES:361`.
2. `metro_settings.c` generaliza sin cambiar la secuencia:
   `metro_firmware_sibling_installed(i)` (`:255`) y
   `metro_firmware_switch_to(i)` (`:287-324`) ejecutan EXACTAMENTE los
   seis pasos de M-090/M-091 — guardas (`sibling == NULL`, no
   instalado, ya existe el propio dormido → `false` sin tocar nada);
   guardar todo y vaciar a disco; `rename(/.rockbox →
   METRO_FW_OWN_DORMANT)` (`:303`); `rename(hermana → /.rockbox)` con
   rollback (`:307-311`); `refresh_root_binary()` (`:317`); marcador
   condicional `metro_sync_switch_needs_rebuild(METRO_FW_OWN_DORMANT)`
   (`:318-319`); `system_reboot()` (`:322`). Retiradas
   `METRO_FW_DORMANT_AURA/_METRO`, `metro_firmware_aura_installed()` y
   `metro_firmware_switch_to_aura()`; contrato actualizado en
   `metro_settings.h:133-168`.
3. UI: la fila 8 de Ajustes › General pasa a `METRO_ROW_NAV`
   "cambiar sistema" (`metro_screen_settings.c:295-303`) y empuja
   `switch_page` (`:374-375`), página local de un pivote (patrón
   `options_page` de `metro_screen_nowplaying.c:385-388`) con una fila
   `METRO_ROW_ACTION` por hermana (`:194-233`): subtítulo "no instalado"
   si falta el dormido (inerte), y si existe → `snprintf` con
   `LANG_DIALOG_SWITCH_FMT` → `metro_widgets_confirm()` →
   `metro_firmware_switch_to(i)`. Misma lista genérica, mismas
   primitivas y roles de color/fuente que el resto de Ajustes: cero
   dibujo propio, cero literal.
4. Strings al final del catálogo (patrón M-009):
   `LANG_SETTING_SWITCH_SYSTEM`, `LANG_FAMILY_AURA/_METRO/_MOONLIT`,
   `LANG_DIALOG_SWITCH_FMT` (`metro_lang.h:184-188`,
   `metro_lang.c:179-183` / `:335-339`); retirados
   `LANG_SETTING_SWITCH_TO_AURA` y `LANG_DIALOG_SWITCH_TO_AURA_TITLE`;
   `LANG_VALUE_NOT_INSTALLED` se conserva.
5. Test host `metro/test/test_firmware_families.c` (Makefile `:16,43`):
   count == 2, ningún dir es el propio, prefijo `/.firmware-`, dirs y
   nombres distintos, `metro_fw_sibling(2) == NULL`.
Invariantes v10 que NO cambian: el activo es siempre `/.rockbox`; el
saliente se renombra primero; `/rockbox.ipod` := el del entrante;
Studio garantiza "nunca dos de la misma familia" y repara un cambio a
medias al conectar. Añadir una cuarta familia = una línea en la tabla +
un `LANG_FAMILY_*`. Capturas:
`docs/screenshots/v0.1.0-cambiar-sistema-{vacio,instalados,confirmar}.png`
(secuencia: `WAIT,SCROLL_FWD×3,SELECT,WAIT,SCROLL_FWD×8,SELECT,WAIT`
desde el hub; `…,SCROLL_FWD,SELECT,WAIT` para el diálogo de Metro).

**D-048 — `__TIME__`/`__DATE__` fuera de los plugins SDL (reproducibilidad
de `rockbox.zip` para la actualización selectiva del contrato v11).**
La actualización selectiva compara CRC32 por archivo del `rockbox.zip`
nuevo contra el manifiesto de lo instalado; cualquier binario cuya
salida cambie con la hora de compilación viaja entero en cada release.
Los `.rock` de Quake y Duke3D embebían la hora/fecha de build:
`firmware/rockbox/apps/plugins/sdl/progs/quake/host.c:884` y
`quake/host_cmd.c:958` (`Con_Printf ("Exe: "__TIME__" "__DATE__"\n")`)
y `duke3d/Engine/src/display.c:711` (`printf("Compiled %s …", __DATE__)`),
más el guardia `#if (!defined __DATE__)` de `display.c:698-700` que
solo existía para esa línea. Eso son ~2,2 MB de delta espurio por
release (los dos plugins más grandes del árbol) aunque no cambie una
línea de ellos. **Decisión**: ambos `Con_Printf` pasan a
`"Exe: rockbox build\n"`, el `__DATE__` de Duke3D se sustituye por el
literal `"rockbox build"` y el guardia se elimina con su único usuario;
cada sitio lleva el comentario `moonlit (D-048)` y queda registrado en
`MODIFICATIONS.md` (regla de `CLAUDE.md`: cambios fuera de
`apps/metro/`). Verificación: `grep -rn '__TIME__\|__DATE__'
firmware/rockbox/apps/plugins/sdl/progs/{quake,duke3d}` devuelve solo
esos comentarios. Los mensajes son de consola interna del juego, sin
efecto en jugabilidad ni en la UI de moonlit.

**D-046 — Versión inicial `v0.1.0`; los tags heredados de Metro no se
publican.** El clon local arrastra desde el fork los 31 tags de
Metro-Aura (`git tag`: `v0.1.0`, `v0.1.0-beta`, `v0.2.0`…`v0.5.6`, más
`moonlit-fork-base`); apuntan a commits de Metro (p. ej. `v0.5.6` =
`69d93dd3` "M-092: wordmark becomes 'metro / aura'") y sus assets
(`rockbox.ipod`, `rockbox.zip`…) existen solo en los releases de
`Ricolinos/Metro-Aura`, nunca en este repo. El remoto
`https://github.com/Ricolinos/moonlit-aura.git` solo tiene
`moonlit-fork-base` (`git ls-remote --tags origin`). **Decisión**: el
primer release de moonlit.aura es **`v0.1.0`** (`CONTRATO-moonlit-studio.md`
v2 §A.7; `firmware/tools/package_dist.sh --release-tag v0.1.0`);
ningún tag `v*` heredado se empuja (`git push --tags` queda prohibido;
solo `git push origin v0.1.0`). Como el nombre `v0.1.0` ya existe
localmente apuntando a `a3693675` (Metro F13), antes de etiquetar el
release hay que **borrar el tag local heredado** (`git tag -d v0.1.0`)
— y, para no repetir la colisión con `v0.2.x`…`v0.5.x` en releases
futuros, borrar en bloque los 30 `v*` heredados del clon local. Esto
no se hace en la pasada que cierra esta decisión (regla de la corrida:
sin crear ni tocar tags); queda como paso 0 del checklist de release
de `CLAUDE.md` §Releases. `docs/COMPAT_STUDIO.md` C22 (`version.txt`
en el zip, camino feliz de `--release-tag`) se verifica con este mismo
release.

---

# v0.1.1 — biblioteca grande en hardware real (D-049…D-051)

**D-049 — Pantalla "preparando biblioteca": bloqueante a propósito,
interrumpible, con progreso para tagcache y para la caché de carátulas.**
Medido en el iPod del dueño (4 556 pistas, ~1 083 álbumes, 1 028
`cover.jpg`): al pulsar Música el aparato quedó 4 min 18 s sin pantalla
ni botones (979 álbumes × 264 ms). Diagnóstico (cadena real, antes de
esta decisión): `metro_screen_hub.c:834 hub_on_select case 0` →
`metro_music.c:202 metro_music_db_ready()` →
`moonlit_art_cache_on_db_ready()` → `moonlit_art_cache.c:111-141
moonlit_art_precache(NULL)` síncrona, con `yield()` como único
paliativo y llamando `moonlit_art_load_for_album()` por CADA álbum
(open+read completo aun en hit). `moonlit_art_pfraw_is_cached()`
(`moonlit_art.c:66`) existía y no se usaba — regresión frente a
`../Aura-Firmware/firmware/rockbox/apps/aura/aura_music.c:345-384`, que
sí la usa, corta con `pending == 0` y repinta cada 4. `updating_page`
(`metro_screen_hub.c:760-779`) era una fila estática sin refresco y
`music_lists_refresh()` (`:303-333`) se repetía en cada entrada.
**Decisión:**
1. `apps/metro/moonlit_screen_library.c/.h` (`apps/SOURCES`):
   `bool moonlit_screen_library_prepare(void)`, bucle propio calcado de
   `metro_run_sync_screen_if_needed()` (`metro_main.c:161-193`):
   `metro_input_next(MCTX_DIALOG, HZ/10)` atiende `SYS_EVENT` (USB:
   `metro_screen_usb_show()` + `default_event_handler()`, y devuelve
   false) y `MACT_BACK` (pospone: devuelve false; lo pendiente queda
   para la próxima entrada — idempotente). Visual moonlit: creciente
   64 px `on_surface` centrado (`moonlit_logo_draw_crescent`, como el
   splash), `MFONT_HEADLINE` "preparando biblioteca", `MFONT_BODY`
   `on_surface_variant` con la fase, `metro_draw_progress()` 120×2 y
   `MFONT_LABEL` "N de M" (minúsculas: convención de todo
   `metro_lang.c`). Fase 1 (`run_phase_db`, `:156`) solo si
   `!(metro_music_db_ready() && tagcache_is_fully_initialized())`:
   "construyendo la base de música" con
   `tagcache_get_commit_step()/_max_commit_step()` (patrón
   `apps/main.c:380-388`; 0 hasta que el escaneo termina y arranca el
   commit), refresco cada HZ/10; se sondea `metro_music_db_ready()`
   porque es quien dispara `tagcache_rebuild()`/`tagcache_start_scan()`.
   Fase 2 (`run_phase_art`): `moonlit_art_pending_count()`; 0 → no
   dibuja nada; si no, "preparando carátulas" y `moonlit_art_precache()`
   con `progress_cb` que repinta cada 4 (`LIB_REPAINT_EVERY`, `:64`;
   `lcd_update()` cuesta más que un decode chico, AF `:376-380`) y
   `should_abort` que sondea sin bloquear (`poll_interrupt(0)`, `:112`).
   Strings `LANG_LIBRARY_PREPARING/_PHASE_DB/_PHASE_ART/_COUNT_FMT`
   (`metro_lang.h:193-196`, `metro_lang.c:186-189`/`:348-351`).
2. `moonlit_art_cache.c`: `moonlit_art_precache(progress_cb,
   should_abort)` (`:135`) cuenta primero los pendientes y salta con
   `moonlit_art_pfraw_is_cached()` (`:164`, cabecera de 16 bytes) antes
   de cualquier `read()` completo; `should_abort` se consulta entre
   álbumes, nunca a mitad de un decode (ningún `.pfraw` a medias);
   devuelve false si se cortó. Nueva `int moonlit_art_pending_count()`
   (`:124`). El conteo puro vive en `moonlit_art.c:81
   moonlit_art_count_uncached()` (host-testeable,
   `test/test_art.c:108 test_count_uncached`: 2 válidos + 1 cabecera de
   otro tema + 1 ausente → 2 pendientes; 16/16 checks). Se retira
   `moonlit_art_cache_on_db_ready()` y su enganche de
   `metro_music.c:202` (`grep on_db_ready metro_music.c` → vacío;
   comentario D-049 en `:195`).
3. `metro_screen_hub.c hub_on_select case 0` (`:875`): llama
   `moonlit_screen_library_prepare()` y DESPUÉS decide la página con
   `metro_music_db_ready()` como antes: si la base sigue sin estar
   lista (usuario pospuso en fase 1) empuja `updating_page`; si solo se
   pospusieron carátulas, Música abre y Marea decodifica el resto una
   por tick ocioso. Listas de Música cacheadas (`s_music_lists_valid`,
   `:314-350`): se reconstruyen solo si cambió el sello
   `tagcache_get_stat()->total_entries` (lectura de struct, cero disco;
   se mueve con la primera construcción, el `tagcache_start_scan()` por
   arranque y la reconstrucción de un sync) o tras
   `metro_screen_hub_music_lists_invalidate()` desde
   `metro_disk_handoff()` (`metro_main.c:213`: arranque y cada retorno
   de USB — lo único que cambia playlists y `artist_images.cfg`, que
   tagcache no ve). `db_stamp.txt` (M-091) se descartó: lectura de
   archivo por entrada y solo cambia con un sync de Studio, se perdería
   el escaneo por arranque. Quickplay (`metro_music_recent_albums`, 8
   filas) y `metro_thumbs_reset()` siguen por entrada: reproducir
   reordena Quickplay y ningún sello lo cubre.
4. D-045 cerrada en su propio bloque (arriba).
Verificación: `make -C firmware/rockbox/apps/metro/test test` verde
(test_art 16/16); `build_sim.sh` sin warnings nuevos (los de
`-Wmissing-field-initializers`/`-Wformat-truncation` en
`metro_screen_hub.c` son previos a este cambio);
`docs/screenshots/v0.1.1-preparando-biblioteca.png` (simdisk con la
biblioteca de `gen_test_media.sh`, `.pfraw` de `moonlitcache/art/`
borrados; `sim_shot.sh … 1 "WAIT,SELECT"`: fase 2 "4 de 11"); fase 1
verificada borrando `database_*.tcd` del simdisk (mismo comando,
"construyendo la base de música", "0 de 10", la base se reconstruye y
Música abre). **Hipótesis abiertas:** (a) si la reconstrucción de una
biblioteca vacía nunca deja `tagcache_is_usable()` en true, la fase 1
espera hasta MENU (antes: la fila "actualizando biblioteca…" para
siempre — mismo estado final, ahora con salida); (b) un USB durante la
pantalla no rehace `metro_disk_handoff()` (igual que la pantalla de
sync hoy), solo devuelve al hub; (c) el tiempo por álbum en fase 2
sigue siendo el del decode JPEG (~264 ms medidos), la ganancia real es
que solo se paga una vez por álbum y con pantalla/botones — pendiente
de medir en el iPod con la biblioteca completa.

**D-050 — Creciente Waning Crescent desde el primer cuadro de arranque.**
Hasta aquí `apps/main.c:256-294 show_logo_boot()` (stock, llamado en
`:332,:398,:421,:544`) dibujaba `apps/bitmaps/native/rockboxlogo.320x98x16.bmp`
— restaurado al original de Rockbox en M9/D-044 (md5 `5bc5004b…`) — más
"Ver. …" en `FONT_SYSFIXED` durante los segundos de `init()`, antes de
que `metro_main.c:268` mostrara el creciente: el primer cuadro visible
del aparato era el logo amarillo de Rockbox. Patrón Aura:
`../Aura-Firmware/firmware/rockbox/apps/main.c:256-299` (D-051/D-210:
bitmap propio, sin texto de versión, centrado bajo `#elif
defined(IPOD_6G)`). **Decisión:**
1. `design-system/generate.py --bootlogo` (`generate_bootlogo`):
   rasteriza `design-system/logo/moonlit-crescent.svg` a 72 px con el
   mismo pipeline de `--logo` (`_rasterize_alpha`: rsvg-convert,
   supersampleo 16× + filtro de caja) y lo compone centrado sobre un
   lienzo 320×98 con fondo `color.night.surface` (#14161F) y tinta
   `color.night.on_surface` (#E7E5EA). **Tinta `on_surface` y no
   `primary`:** cuando `apps/main.c` dibuja esto aún no se ha leído el
   preset de acento del usuario (`metro_settings_load()` corre dentro de
   `metro_main()`), así que `primary` de moonstone sería una suposición
   que falla para tide/ember/moss; además el splash que sigue
   (`metro_screen_splash.c:50`) dibuja el mismo creciente con
   `metro_color_fg()` == `on_surface`, con lo que primer cuadro y splash
   son la misma figura del mismo color — sin salto. Sin texto.
   Verificación de tonos ≥ 4 con la misma `MIN_INK_TONES` (104 tonos),
   reporte en `docs/screenshots/v0.1.1-bootlogo-tones.txt`. Formato de
   salida: BMP Windows 3.x 24 bpp sin compresión, idéntico al original
   (`file` del original: "320 x 98 x 24"; el "x16" del nombre es la
   profundidad nativa que `bmp2rb` produce para el LCD, no la del
   archivo), así la regla de `apps/bitmaps/bitmaps.make` no cambia.
   md5 nuevo `d03480c5a8c73b4639ad0b581ebed6e4`. `MODIFICATIONS.md`:
   entrada del bmp reescrita.
2. `apps/main.c show_logo_boot()`: bloque `#if defined(IPOD_6G)`
   (comentario `moonlit (D-050)`): `lcd_set_background(LCD_RGBPACK(0x14,
   0x16, 0x1F))` — literal RGB documentado como excepción: `main.c` no
   puede incluir `moonlit_palette.h` (la regla de único includer de
   `moonlit_tokens.h` está acotada a `apps/metro/`, D-035, y este código
   corre antes de que `metro_main()` tenga la pantalla), el valor es
   `tokens.json color.night.surface` copiado a mano y anotado —,
   `lcd_clear_display()`, `lcd_bmp` centrado en ambos ejes, sin
   `lcd_putsxy` de versión, `lcd_update()`. El `#else` conserva el
   código stock para cualquier otro target. `MODIFICATIONS.md`:
   entrada nueva. Nada en `bootloader/` (BOOT-1 intacto, D-045 punto 3
   de las refutaciones de M11).
Verificación: `md5` ≠ `5bc5004b8be813dddad1c88d5735bfb6`; Pillow
confirma 320×98 y 103 colores distintos del fondo; `build_target.sh
--firmware` produce `rockbox.ipod`; `build_sim.sh` sin warnings en
`main.c`; `docs/screenshots/v0.1.1-boot-logo.png` con `sim_shot.sh … 5`
(sin botones) muestra el creciente sobre `surface` night — el
simulador pasa por `apps/main.c:421` (variante hosted) y `IPOD_6G` está
definido en el build `--target=ipod6g --type=s`, así que ese camino
también toma la rama nueva. `CLAUDE.md` §Comandos gana `--bootlogo`.

**D-051 — Marea es el primer pivote de Música.** Metro (DA-1, M-065)
dejó Quickplay como pivote de aterrizaje "abierto a que el dueño lo
cambie en la PARADA"; en moonlit el dueño lo cambió: la superficie de
llegada de Música es el Cover Flow vertical. **Decisión:** en
`apps/metro/metro_screen_hub.c music_pivots[]` la entrada
`{ LANG_MAREA_TITLE, marea_count, marea_get_row, marea_on_select, NULL }`
pasa de la última posición a la primera; `npivots` sigue en 7 y ningún
otro pivote cambia (Quickplay queda segundo con su rejilla). Comentario
"DA-1: first pivot" reescrito ahí mismo. Actualizados: D-029 (ruta
real y nota), `docs/ESTADO_FINAL.md` (Quickplay ya no es el primero),
`docs/moonlit-design-system/componentes/marea.md` §Entrada,
`docs/moonlit-design-system/00-INDICE.md`, `README.md`, y la secuencia
de botones de `docs/screenshots/M11-verificacion.txt` (antes
`SELECT,RIGHT×6,SELECT`, ahora `SELECT,SELECT`).
`docs/screenshots/M8-marea-{0,1,mono}.png` regenerados con la
secuencia nueva (`WAIT,SELECT,WAIT,SELECT,WAIT` para -0, `+SCROLL_FWD,WAIT`
para -1, `+SCROLL_FWD×8,WAIT` para -mono: "Night Drive", monograma "N").
Verificación: la captura de Música (`WAIT,SELECT`) muestra el
encabezado del pivote "marea" primero.

**D-052 — Motion Waning: el movimiento de moonlit tiene dirección y
luz.** Metro heredaba el *turnstile* de F12 (giro por columnas,
`metro_fb_draw_turnstile_layer()` + tabla `metro_turnstile_table.c/.h`
generada por `tools/gen_turnstile_table.py`, ~230 k px/cuadro) y un
FADE lineal de 6 cuadros; nada de eso respondía al lenguaje Waning
Crescent (luz desde la izquierda, D-012; elevación por tono, no por
sombra). **Decisión:** un paquete de cuatro piezas, todas en
`apps/metro/`, ninguna con lectura de disco dentro del bucle:

1. **C1 Luz de canto** (`8aabc8a2`): PUSH/POP = deslizamiento; la
   página nueva **entra desde la izquierda**, la saliente se retira
   hacia la izquierda al volver. `metro_transitions_push()` corre
   `run_slide()` en todo nivel: 7 cuadros × 3 ticks bajo `all`, 4 bajo
   `minimal`, `METRO_EASE_OUT_QUAD`. CONTINUUM monta sobre este bucle
   (`all`+`full`, solo push). Primitiva
   `metro_fb_compose_slide(from, to, dx, seam)` (blit sin `lcd_update()`,
   para componer capas encima) y `metro_fb_present_slide()` (con
   update). Costo: 76 800 px/cuadro.
2. **C3 Filo de luna** (`8aabc8a2`): `lcd_vline` de 1 px en la
   costura entre ambas páginas, color `moonlit_surface(MSURFACE_HIGH,
   MEDGE_LIGHT)` — el mismo borde-luz de las tarjetas —, omitida en el
   último cuadro. Costo: 240 px/cuadro. Token `motion.seam: true`.
3. **C2 Menguante** (`8e036151`): FADE 7 × 3 ticks (210 ms, el mismo
   presupuesto que PUSH, D-037) con `METRO_EASE_OUT_QUAD` en vez de
   6 lineales. Sigue reservado a `all`+`graphics=full`; `minimal`/`lite`
   caen al deslizamiento C1. Costo: 76 800 blends/cuadro.
4. **C4 Marea que sube** (este commit): al mover la selección **una**
   fila en listas (`metro_screen_list.c run_selection_rise()`) y en el
   hub (`metro_screen_hub.c run_selection_rise()`), la tarjeta de la
   fila nueva pasa de `surface` a `surface_container_high` en 4 cuadros
   × 2 ticks (80 ms, `motion.selection_ms`), `METRO_EASE_OUT_QUAD`,
   tono interpolado con `metro_fb_blend_color()`; el marcador `primary`
   de 3 px crece de 0 a la altura completa desde arriba; los bordes
   luz/sombra D-012 solo en el último cuadro. Solo se repintan las dos
   filas afectadas (`metro_draw_row_slot()` / `draw_hub_row()`) con
   `lcd_update_rect()`; `redraw_current()` asienta la pantalla después
   como siempre. Primitiva única: `moonlit_draw_selection_card(y, h,
   card_alpha, marker_h, edges)` en `moonlit_elevation.c`, que también
   dibuja la tarjeta asentada de `metro_draw_rows_ex()`, del hub y de
   "Acerca de" (una sola definición de la tarjeta de selección). Costo:
   ~18 000 px/cuadro (2 bandas de 320×28; 320×52 en el hub). Puerta:
   `lcd_active() && metro_settings.animations != METRO_ANIM_OFF`
   (corre también bajo `minimal`); **nunca** con `steps > 1`
   (aceleración de rueda), ni si la ventana desplazó, ni en rejillas,
   ni en "Acerca de", ni en listas vacías, ni mientras flota la letra
   de índice (F10). Marea no cambia.

**Eliminado:** `run_turnstile()`, `metro_fb_draw_turnstile_layer()`,
`metro_turnstile_table.c/.h`, `tools/gen_turnstile_table.py` y su
entrada en `apps/SOURCES` (`grep -rn turnstile firmware/rockbox/apps/
firmware/tools/` vacío).
**Tokens:** `design-system/tokens.json motion` gana `selection_ms: 80`,
`ease_selection: "out_quad"`, `seam: true` → `MOONLIT_MOTION_SELECTION_MS`,
`MOONLIT_MOTION_EASE_SELECTION`, `MOONLIT_MOTION_SEAM`
(`moonlit_tokens.h`); `metro_transitions.h` cita el literal
`METRO_SELECTION_FRAMES 4` × `METRO_SELECTION_FRAME_TICKS 2` (patrón
D-037, porque D-035 impide incluir el header fuera de
`moonlit_palette.c`).
**Trazas:** `metro_transitions_trace()` exportada; cada bucle (slide,
fade, select) emite `METRO_TRACE("<nombre> frame i/n at +t ticks")`.
**`.bss`:** `arm-elf-eabi-size` antes = 8 570 044 (v0.1.1, D-051),
después = 8 570 044 — sin cambio (ninguna tabla nueva; la tabla del
turnstile vivía en `.rodata`), bajo el techo D-043 de 8 574 076.
**Sin medición real:** los costos son conteos de píxeles; la checklist
H de hardware (32/32) sigue sin responder y M12 mide `present_slide`,
`present_fade` y C4 en el iPod con las trazas por cuadro.
Verificación: `make -C firmware/rockbox/apps/metro/test test` verde;
`build_sim.sh` y `build_target.sh` sin warnings nuevos;
`docs/screenshots/v0.1.1-motion-{push,fade,select}-mid.png` (secuencias
en `docs/moonlit-design-system/sistema/05-movimiento.md` §Paquete
Waning; la del push verificada con PIL: columna x=83 entera del color
borde-luz; la de C4: marcador 48/52 px, tono intermedio, sin bordes).

---

# v0.1.2 — Marea no bloqueante y base compartida (D-053…D-055)

**D-053 — Marea anima por reloj, no bloquea (modelo Music Flow de
Aura-Firmware).** Diagnóstico (v0.1.1): `moonlit_screen_marea.c:580-590`
animaba **bloqueando** (`for frame 1..7 { show() completa; sleep(3) }`),
sin `cpu_boost` (S5L8702 a 54 MHz), redibujando en los 8 cuadros (7 + el
`redraw_current()` de `metro_main.c:533-534`) el panel derecho (33 440 px
+ 576 `plot_alpha` + 144 `isqrt`) y la cabecera, llamando en cada cuadro
`metro_music_song_count_of_album()` (`:469-470`, barrido de tagcache) y
leyendo hasta 3 `.pfraw` de disco antes del primer cuadro
(`preload_range()`, `:537-551,576`). El render en sí (42 057 px/cuadro)
es más barato que el de Aura (95 591): `moonlit_flow.c` no se toca.
Aura (`aura_musicflow.c:316-321,1238-1259`; `aura_main.c:606-614`)
resuelve la posición como función del reloj y el bucle principal solo
pide cuadros. **Decisión** (commit `79171a86`):

1. Estado por reloj: `MAREA_SCROLL_ANIM_MS 220` (= `motion.transition_ms`,
   literal documentado, patrón D-037), `s_anim_from_x256`, `s_anim_since`
   (`current_tick`), `anim_pos_x256()` = `metro_ease(METRO_EASE_OUT_EXPO,
   elapsed_ms, 220)` interpolando origen → destino×256;
   `moonlit_screen_marea_animating()` = "se pidió destino y aún no se
   dibujó el asentamiento".
2. `scroll_step()` solo fija el destino desde la posición animada
   **actual** (retarget real) y regresa. Eliminados
   `run_scroll_animation()`, `s_in_scroll_loop`, `s_scroll_missed`,
   `preload_range()`, `MAREA_SCROLL_FRAMES/FRAME_DELAY`, `sleep()` y el
   drenaje de botones (el bucle principal ya lee botones entre cuadros).
   `moonlit_wheel_step()` (tope 3) se conserva. Con `animations=off` o
   LCD dormido el origen es el destino: el primer cuadro es el de
   asentamiento.
3. Redibujo acotado: `moonlit_screen_marea_show_carousel()` = relleno de
   la banda (0,20,152,220) + 7 `draw_slide()` + `lcd_update_rect()` de
   esa banda, bajo `cpu_boost(true/false)`. `moonlit_screen_marea_show()`
   completa (clear + cabecera + panel + `lcd_update()`) solo al entrar, al
   cambiar tema/idioma (`redraw_current()`) y en el **cuadro de
   asentamiento** — ahí y solo ahí `metro_music_song_count_of_album()`,
   cacheado por índice (`s_songs_for_index`, patrón `target_artist()` de
   Aura `:845-856`).
4. `metro_main.c`: timeout `HZ/20` mientras `at_marea && animating()`
   (patrón `metro_screen_hub_wants_ticks()`); en `MACT_NONE` si anima →
   `show_carousel()`; tras `MACT_NEXT/PREV` en Marea se dibuja el primer
   cuadro de inmediato en vez de `redraw_current()`. Cadencia por reloj =
   salto de cuadros implícito: un cuadro lento no alarga la animación.
5. Disco fuera del cuadro: `get_slot_for()` **nunca abre archivos** — un
   miss reclama el slot (`MAREA_ART_PENDING`, monograma/relleno) y
   `moonlit_screen_marea_tick()` (una carga por vuelta ociosa, **solo
   cuando no anima**) lo rellena: `moonlit_art_load_for_album()` lee el
   `.pfraw` o decodifica; `MAREA_ART_MISSING` fija el monograma sin
   reintentos. Sin pendientes, el tick precarga el álbum más cercano sin
   slot dentro de `MAREA_PREFETCH_RADIUS` (= 2+1+3: ventana visible + paso
   máximo de rueda) — sustituye a `preload_range()` sin bloquear. Con
   ello desaparece el bug "slot −1 reutilizado dentro del mismo cuadro"
   de D-045.
6. Verificación: tests host verdes; `build_sim.sh` sin warnings nuevos;
   `docs/screenshots/v0.1.2-marea-mid.png` (`sim_shot.sh … 2
   "WAIT,SELECT,WAIT,SELECT,WAIT,SCROLL_FWD"`), comparada con PIL contra
   una captura en reposo del mismo build: banda 0..152 = 24 270 px
   distintos, panel 160..320 = **0**, cabecera = **0** (`M8-marea-0.png`
   es de otra paleta y no sirve de base). `grep -n 'sleep(\|song_count'`
   → `sleep` vacío, `song_count` solo en `draw_panel()` bajo
   `s_songs_for_index != s_target_index`; `grep -n 'read_pfraw\|open('`
   → vacío (la lectura vive en `moonlit_art_cache.c`, llamada solo desde
   `tick()`).

**Hipótesis abiertas:** sin medición en hardware (M12): el costo real del
cuadro bajo `cpu_boost` y si `HZ/20` alcanza para 7 tapas a 120 px; el
radio de precarga 6 es un valor de partida.

**D-054 — Base de datos tagcache compartida en `/.aura/tagcache` y sello
compartido (contrato v15).** Hechos: `apps/tagcache.c/.h` son
byte-idénticos en los tres repos (md5 `1e7a6754…`/`d9e5f97d…`,
`TAGCACHE_MAGIC 0x54434810`); la ruta es runtime
(`global_settings.tagcache_db_path`, `settings_list.c:1817`,
`tagcache.c:5624 tc_stat.db_path`; `open_db_fd()` hace `mkdir`). El sello
v12 (`metro_sync.c:117-180`) solo se escribía en `finish_ok()`: el
rebuild de bootstrap de `metro_music_db_ready()` (`metro_music.c:179`)
no sellaba y cada cambio de firmware tras una biblioteca copiada a mano
reconstruía 5 minutos. **Decisión** (commit `65d909d1`):

1. `AURA_SHARED_DB_DIR "/.aura/tagcache"` y `AURA_SHARED_DB_STAMP_PATH`
   en `metro_settings.h` (único dueño de rutas del contrato junto con
   `metro_settings.c`/`metro_sync.c`). `metro_force_shared_db_path()`
   (`metro_settings.c`) hace `strmemccpy(global_settings.tagcache_db_path,
   …)`; se llama en los **dos** cuerpos de `init()` de `apps/main.c`
   justo después de `metro_apply_hygiene()` — es decir, después de
   `settings_load()` (que sobreescribiría la ruta con `config.cfg`) y
   antes de `init_dircache()`/`init_tagcache()`. Corrección al
   diagnóstico del encargo: `metro_apply_hygiene()` **no** corre en
   `metro_main()` sino dentro de `init()` (`main.c:481` y `:755`, M-019),
   así que el punto correcto ya existía; lo que "llega tarde" es
   `metro_main()` (`main.c:252`), que corre después de `init_tagcache()`
   (`main.c:489`/`:798`). `MODIFICATIONS.md` actualizado.
2. Migración sin rebuild: si no existe `/.aura/tagcache/database_idx.tcd`
   y sí `ROCKBOX_DIR/database_idx.tcd`, `mkdir` + `rename()` de todo
   `database_*.tcd|.txt` del árbol (misma partición FAT, atómico, nunca
   copia). Si el compartido **ya** existe, los del árbol se **borran**:
   son restos muertos que ningún hermano leerá (v15) y solo ocupan disco.
   Al migrar una base sin sello se sella (misma regla que el arranque en
   frío de `metro_sync_switch_needs_rebuild()`: la base que el firmware
   activo venía usando está al día).
3. Sello: `db_stamp.txt` pasa a `/.aura/tagcache/db_stamp.txt`;
   `ROCKBOX_DIR/aura/db_stamp.txt` se migra por `rename` si el compartido
   no existe (y se borra si sí). `metro_sync_switch_needs_rebuild()` ya
   **no** recibe árbol saliente: compara siempre el sello compartido con
   `/.aura/library-stamp` (`metro_sync_db_stamp_is_current()`); sin sello
   de biblioteca lo crea, sella la base y devuelve **false** (la
   entrante usa la misma base). `metro_music_db_ready()` sella tras el
   rebuild de bootstrap la primera vez que `tagcache_is_usable()`
   (`s_scan_triggered`), nunca una base que ya existía.
4. Studio ignora el directorio salvo para borrarlo al forzar rebuild
   (v15); nada que hacer en firmware.
5. Verificación (simulador): con `simdisk/.rockbox/database_*.tcd`
   (11 archivos) y sin `/.aura/` → tras el arranque
   `ls simdisk/.aura/tagcache/` = 11 `.tcd` + `db_stamp.txt`,
   `simdisk/.rockbox/` solo conserva `database.ignore`; segundo arranque:
   `stat` de los `.tcd` idéntico (sin rebuild);
   `cat db_stamp.txt` == `cat library-stamp`
   (`fw-20260826T104732-0000006b`). `grep -rn 'database_\|db_stamp'
   apps/metro/ apps/main.c` → solo `metro_settings.{c,h}`, `metro_sync.c`
   y las llamadas a `metro_sync_record_db_stamp()`. Test host del sello:
   no se separó — la lógica pura es `strcmp` de dos archivos de una
   línea; todo el valor está en el I/O, que se verificó en el simulador.

**Hipótesis abiertas:** el cambio a una hermana que **todavía no** adopte
v15 (base propia dentro de su árbol) recibirá marcador solo si el sello
compartido difiere del de biblioteca — su base propia puede estar vieja
sin que moonlit lo sepa; se cierra cuando Aura/Metro implementen v15.
`tagcache_ram` recarga la base desde la ruta nueva sin cambio de código
(`tc_stat.db_path`), verificado en simulador, no en el iPod.

**D-055 — Claves estables de caché de carátulas y thumbs compartidos
`/.aura/thumbs`.** Hechos: las carátulas se indexaban por `album_seek`
(`moonlit_art_cache.c:42-48` `<seek>-120.pfraw`, `metro_screen_hub.c`
`album-<seek>.mth`), que tagcache renumera en cada rebuild → caché
huérfana/equivocada y otros 4 min de decodificación; `finish_ok()` no
limpiaba nada. Fotos y artistas ya usaban `<archivo>.<mtime>.mth`.
**Decisión** (commit `05b7f159`):

1. Clave de álbum = `a-<crc32 hex8 de la ruta de la pista
   representativa>.<tag_mtime de esa pista>`
   (`metro_music_album_art_key()`, `metro_music.c`): la pista es el
   primer resultado de `metro_music_songs_of_album()`, la ruta viene de
   `tagcache_retrieve(tag_filename)`, el mtime de
   `tagcache_get_numeric(tag_mtime)` (no hay `stat()` de archivo en el
   API de Rockbox y tagcache ya lo guarda), el CRC de `crc_32()` de
   `firmware/include/crc32.h`. **Desviación deliberada** respecto al
   encargo (`a-<crc>-<mtime>`): el separador es `.` para que la clave
   tenga la forma `<estable>.<mtime>` que `metro_thumbs.c remove_stale()`
   ya entiende — un cambio de carátula deja el `.mth` viejo huérfano y
   el motor lo borra solo. Memo de 48 entradas (1 344 B `.bss`),
   invalidado cuando cambia `tagcache total_entries` (mismo indicador
   que `music_lists_are_valid()`) o por `metro_music_album_art_key_reset()`
   al terminar un sync. Aplica a `moonlitcache/art/<clave>-120.pfraw`
   (sigue en el árbol) y a `/.aura/thumbs/albums/<clave>.mth`. Marea ya
   no calcula rutas (D-053): la clave se resuelve solo en `tick()`.
   `moonlit_art_pending_count()` memoriza la respuesta "0 pendientes"
   por (`total_entries`, tema): con claves estables es la respuesta
   estable, y así entrar a Música no paga una búsqueda de tagcache por
   álbum cada vez.
2. `AURA_SHARED_THUMBS_DIR "/.aura/thumbs"` (solo en `metro_settings.c`);
   `metro_settings_shared_thumbs_dir(subdir)` sustituye a
   `metro_settings_metro_cache_dir()` en `metro_thumbs.c` (formato crudo
   80×80 idéntico en Metro y moonlit, `METRO_TILE_SIZE`).
   `metro_settings_migrate_shared_thumbs()` (desde `metro_main()`, tras
   `metro_settings_load()`): `rename` de `moonlitcache/{albums,artists,
   photos}` si el compartido no existe; devuelve true y entonces se pide
   una limpieza (esquema de claves anterior).
3. Limpieza de huérfanos: `moonlit_art_request_gc()` (bandera
   `moonlitcache/art/.gc-pending`, sobrevive reinicios) la pide
   `finish_ok()` del sync con `music: true` y la migración;
   `moonlit_art_gc()` la ejecuta la pantalla "preparando biblioteca"
   tras la precarga, con la pantalla puesta: tabla de `crc32` de las
   claves vigentes en el scratch estático de la precarga
   (`s_precache_cover`, 28 800 B = 7 200 entradas, cero `.bss` nuevo) y
   un barrido de `art/*-120.pfraw` y `thumbs/albums/*.mth` borrando lo
   que no esté en la tabla — incluidos los nombres pre-D-055. Elegido
   sobre la limpieza en `finish_ok()` (que corre sin pantalla y a veces
   en segundo plano) y sobre un GC por presupuesto (necesitaría la tabla
   viva entre vueltas).
4. Verificación (simulador): tras entrar a Música,
   `ls simdisk/.aura/thumbs/albums/` → `a-031b464b.1787718316.mth` …;
   `moonlitcache/` solo conserva `art/` con `a-<crc>.<mtime>-120.pfraw`
   y los nombres viejos (`108-120.pfraw`, `album-108.mth`) desaparecieron
   por la limpieza de migración. Borrado `simdisk/.aura/tagcache/*` y
   reinicio (rebuild real, base y sello regenerados): `stat` de los 8
   `.pfraw` y 7 `.mth` **idéntico** — nada se decodificó de nuevo y la
   pantalla "preparando biblioteca" no entró en fase 2.

**`.bss`:** `arm-elf-eabi-size` antes = 8 570 044 (v0.1.1, D-052),
después = 8 571 388 (+1 344 B: el memo de 48 claves de `metro_music.c`; los slots de Marea crecen 148 B y el estado del bucle bloqueante desaparece) — bajo el techo D-043 de 8 574 076. `build_target.sh --firmware` sin warnings nuevos en `apps/metro/`.

**Hipótesis abiertas:** el costo de `metro_music_album_art_key()` en
frío (una búsqueda filtrada de tagcache + un `retrieve` de disco por
álbum) no está medido en el iPod; con `tagcache_ram` la búsqueda es en
RAM y el memo/la respuesta cacheada evitan repetirlo, pero la primera
pasada de precarga de 979 álbumes lo paga entero. La pista representativa
es el primer resultado de tagcache para el álbum (orden de escaneo): si
esa pista se borra, el álbum cambia de clave y se decodifica una vez.

# v0.1.3 — Caché negativa de carátulas (D-056)

**D-056 — Caché negativa `<clave>.none`: los álbumes sin carátula
resoluble dejan de reabrir la pantalla "preparando biblioteca".**
Diagnóstico (iPod del dueño, 1 083 álbumes, 979 `.pfraw` en
`moonlitcache/art/`): cada entrada a Música —sin reiniciar ni cambiar de
familia— mostraba "preparando biblioteca / preparando carátulas" con
**57** pendientes. Esos 57 álbumes no tienen carátula que
`metro_albumart_decode_track_cover_sized()` acepte (sin `cover.jpg`/APIC,
o JPEG que el decodificador rechaza). `moonlit_art_load_for_album()`
(`moonlit_art_cache.c:78-100` en v0.1.2) devolvía `false` sin escribir
nada; D-042 dejó fuera la caché negativa a propósito (criterio heredado de
`AF/aura_music.c:221-300`); y `moonlit_art_pending_count()`
(`moonlit_art_cache.c:151-170` en v0.1.2) solo memorizaba el resultado
cuando era **0** (D-055). Cadena por entrada: pre-pase recorre tagcache y
cuenta 57 → pantalla → 57 decodes fallidos → nada cambia en disco → la
próxima entrada repite. **Decisión** (este commit):

1. **Marcador negativo.** Cuando `moonlit_art_load_for_album()` no
   consigue arte (sin pista, sin ruta o decode fallido —
   `moonlit_art_cache.c:126-131`, `give_up()` `:91`) escribe
   `moonlitcache/art/<clave>.none` (0 bytes) con la **misma clave
   estable D-055** `a-<crc32 ruta>.<mtime>` del `.pfraw`
   (`moonlit_art_none_path()`, `moonlit_art.c`: `"<dir>/<clave>-120.pfraw"`
   → `"<dir>/<clave>.none"`; sin tamaño ni tema, "no hay carátula" no
   depende de ninguno). En la siguiente llamada, tras el miss del
   `.pfraw`, ve el `.none` y devuelve `false` sin abrir la pista ni
   decodificar (`:123`); Marea (`moonlit_screen_marea_tick()`) marca el
   slot `MAREA_ART_MISSING` → monograma, igual que antes pero sin el
   decode. Un decode posterior que sí produce `.pfraw` borra el `.none`
   por si acaso (`:141`).
2. **El pre-pase trata `.none` como resuelto.**
   `moonlit_art_is_resolved()` = `.pfraw` válido **o** `.none` presente;
   la usan `moonlit_art_count_uncached()` (puro, host-testable),
   `count_uncached_now()` y el salto por álbum de
   `moonlit_art_precache()`. La precarga deja los `.none` en el mismo
   pase, así que a la segunda entrada `pending == 0`.
3. **Memo permanente del pre-pase.** `moonlit_art_pending_count()`
   memoriza el resultado **siempre** (no solo 0) por (`tagcache
   total_entries`, tema, generación); `moonlit_art_pending_invalidate()`
   sube la generación desde `finish_ok()` del sync con música (vía
   `moonlit_art_request_gc()`), desde el sellado de bootstrap en
   `metro_music_db_ready()` (`metro_music.c:207`) y tras una precarga
   abortada; una precarga completa guarda 0 directamente. Entrar a
   Música con la biblioteca sin cambios ya no recorre tagcache.
4. **GC.** `gc_sweep()` estática pasa a `moonlit_art_sweep()` en
   `moonlit_art.c` (con `test/dir.h` como stand-in de host) y
   `moonlit_art_gc()` barre también `art/*.none`: un álbum que
   desaparece o cambia de clave se lleva su marcador.
5. Tests host (`test/test_art.c`, 46 checks): derivación de la ruta
   `.none`, round-trip del marcador, `count_uncached` con `.none` como
   cacheado, barrido que borra `.none`/`.pfraw` huérfanos y respeta
   `.gc-pending`.

**Verificación (simulador, biblioteca de `gen_test_media.sh`, 11 álbumes,
3 sin carátula: `SinArte/`, `Wheel & Click/Sin Portada`,
`Aura Test Combo/Night Drive`).** `rm moonlitcache/art/*` +
`sim_shot.sh docs/screenshots/v0.1.3-prepare-first.png 1 "WAIT,SELECT"`
→ fase 2 con "11 de 11"; `ls moonlitcache/art/` → 8 `.pfraw` + 3
`.none`. Segunda corrida (`v0.1.3-prepare-second.png`, 40 ticks) → Música
directamente; PIL: 240 px de la barra `primary` en la región y 160–185 de
la primera captura, **0** en la segunda. Marea con un `SCROLL_FWD`
(`v0.1.3-marea-none-monogram.png`): "Album sin portada" con monograma
"A". `make -C firmware/rockbox/apps/metro/test test` verde.

**`.bss`:** `arm-elf-eabi-size` antes = 8 571 388 (v0.1.2, D-055),
después = 8 571 420 (+32 B: memo de pendientes y contexto del GC) — bajo
el techo D-043 de 8 574 076. `build_target.sh --firmware` en 0, sin
warnings nuevos en `apps/metro/`.

**Limitación documentada / hipótesis abierta (misma que Aura D-338):** la
clave incluye el `mtime` de la pista representativa, así que una carátula
añadida desde Studio **reescribiendo la pista** (APIC) o tocando su mtime
cambia la clave y se reintenta sola; si Studio solo deja un `cover.jpg`
junto a una pista intacta, la clave no cambia y el `.none` sigue
mandando hasta el próximo GC que lo deje huérfano (un sync con música
que re-clave el álbum) — no hay `stat()` de carpeta en la clave a
propósito (costo por álbum en el pre-pase). `COMPAT_STUDIO.md` no existe
en este repo y `CONTRATO-moonlit-studio.md` v3 es inmutable: el
comportamiento queda anotado aquí, no en el contrato.

**D-057 — Marea carga las carátulas como Music Flow: una lectura
acotada por cuadro, tick presupuestado a HZ/20, precarga direccional.**
Reporte del dueño en hardware (iPod 6G, HDD, 1 083 álbumes, 979
`.pfraw` + 57 `.none` — es decir, la biblioteca **entera ya horneada**):
tras D-053 Marea "se desliza rápido y fluido, igual que en Aura", pero
"las carátulas tardan en aparecer (llegan monogramas y después las
portadas)", mientras que en el Music Flow de Aura aparecen al instante.

**Diagnóstico (verificado contra el código de ambos repos):**

- D-053 dejó **toda** lectura de `.pfraw` en `moonlit_screen_marea_tick()`
  (`moonlit_screen_marea.c`), que cargaba **1 slot por vuelta** del
  bucle principal y solo cuando `!moonlit_screen_marea_animating()`
  (`metro_main.c:476-482` antes de este commit). En reposo (sin
  animar), el bucle principal despierta cada `HZ/10` = 100 ms
  (`metro_main.c`, la condición de `timeout` de `metro_input_next()`
  solo caía a `HZ/20` mientras `at_marea && animating()`) — 5 portadas
  visibles tardaban **≥ 500 ms** en llenarse tras cada asentamiento, y
  durante el desplazamiento mismo nunca se cargaba nada (regla dura de
  D-053, "ninguna lectura de disco dentro de un bucle de animación").
  La precarga ociosa (`next_slot_to_load()`, segunda mitad) barría un
  radio **parejo** `MAREA_PREFETCH_RADIUS = MAREA_VISIBLE_RADIUS+1+3 = 6`
  sin sesgo de dirección.
- Aura (`../Aura-Firmware/firmware/rockbox/apps/aura/aura_musicflow.c`):
  `get_slot_for()` (`:448-494`) llama a `aura_albumart_load_for_album()`
  **dentro del propio `draw`** (`aura_musicflow_draw()`, invocada por
  slide visible en `:1103`/`:1116`/`:1397`) — un `read()` plano de
  ~42 KB (`MF_COVER_SIZE²×2 + reflejo`) cuando el `.pfraw` ya existe,
  ~5-10 ms con el disco despierto; sin acotar a "una por cuadro", así
  que puede decodificar JPEG y tocar tagcache ahí mismo si hace falta.
  `MF_CACHE_SLOTS = 2×(MF_VISIBLE_RADIUS+15)+3 = 39` (`:91`) guarda 15
  álbumes más allá de lo visible **a cada lado**, parejo (sin sesgo de
  dirección tampoco — la asimetría 10/4 de este commit es una decisión
  de moonlit, no un calco de Aura). Resultado: al asentar, casi todo ya
  está en RAM porque el propio `draw` lo fue completando cuadro a
  cuadro sin la restricción que D-053 impone aquí.
- La ruta a un `.pfraw` (`moonlit_art_pfraw_path()` →
  `metro_music_album_art_key()`, `metro_music.c:625-661`) **sí** toca
  tagcache la primera vez que se resuelve un álbum en la sesión
  (`compute_album_art_key()`, `:597-623`: `metro_music_songs_of_album()`
  + `tagcache_retrieve()`/`_get_numeric()`) — memoizada después en un
  anillo de 48 entradas, pero ese primer cómputo es justo la clase de
  trabajo que D-030/D-053 prohíben dentro de un cuadro de animación.
  Aura no tiene esa restricción (su `get_slot_for()` puede tocar
  tagcache y decodificar JPEG dentro del `draw`); moonlit sí, así que
  la solución no puede ser "copiar a Aura tal cual" — de ahí que la
  lectura dentro del cuadro (item 1) esté acotada a claves **ya
  conocidas** de antemano, nunca a resolver una nueva.

**Decisión** (este commit, D-057):

1. **Lectura acotada dentro del cuadro** (`moonlit_screen_marea_show_carousel()`
   → `try_frame_bounded_read()`): mientras anima, a lo sumo **una**
   lectura de `.pfraw` por cuadro — `moonlit_art_read_pfraw()` plano,
   nunca `metro_albumart_decode_track_cover_sized()` ni tagcache.
   Requiere que la clave del álbum ya esté memoizada
   (`metro_music_album_art_key_peek()`, `metro_music.c` — variante de
   solo-memo de `metro_music_album_art_key()`, sin el `compute_album_art_key()`
   que toca tagcache; `moonlit_art_pfraw_path_peek()`,
   `moonlit_art_cache.c`, construye la ruta con esa clave). Prioriza el
   slot central del destino y luego sus visibles
   (`moonlit_marea_prefetch_order(target, n, dir=1, MAREA_VISIBLE_RADIUS,
   MAREA_VISIBLE_RADIUS, …)`). Un `.pfraw` inexistente o un `.none` ya
   conocido **no** gastan el cupo del cuadro (D-056 ya distinguía
   `open()` fallido de un miss real); solo un `read()` completo lo
   gasta. Medido con `current_tick` y trazado con `MAREA_TRACE`
   (macro local nueva, `DEBUGF`+`logf`, mismo patrón que
   `metro_transitions.c:METRO_TRACE` — este módulo no tenía una propia
   antes de D-057).
2. **Tick con presupuesto** (`moonlit_screen_marea_tick()`): hasta
   `MAREA_TICK_BUDGET_MAX_LOADS` = 4 cargas o `MAREA_TICK_BUDGET_MS` =
   15 ms medidos con `current_tick`, lo que ocurra primero (antes: 1
   carga fija). `moonlit_screen_marea_wants_ticks()` (patrón
   `metro_screen_hub_wants_ticks()`) devuelve true mientras quede un
   índice sin slot o `PENDING` en la ventana visible
   (`±(MAREA_VISIBLE_RADIUS+1)`); `metro_main.c` ahora sondea a `HZ/20`
   con `at_marea && (animating() || wants_ticks())`, no solo mientras
   anima.
3. **Prefetch direccional** (`moonlit_marea_prefetch_order()`, módulo
   puro nuevo `moonlit_marea_prefetch.c/.h`, host-testado en
   `test/test_marea_prefetch.c`, 35 checks): tras el asentamiento,
   `next_slot_to_load()` recorre primero el propio destino, luego
   alterna `target ± d` acotando cada lado por separado —
   `MAREA_PREFETCH_FWD_RADIUS` = 10 en `s_last_scroll_dir` (actualizado
   en cada `scroll_step()` real), `MAREA_PREFETCH_BACK_RADIUS` = 4 en
   la contraria. Como `MAREA_VISIBLE_RADIUS` (2) ≤ ambos radios, la
   ventana visible siempre queda cubierta en los primeros pasos del
   recorrido. Techo de slots: 1 (destino) + 10 + 4 = 15 ≤
   `MAREA_CACHE_SLOTS` (37) con margen de sobra — **no hizo falta subir
   el conteo de slots** para esta precarga más ancha; se documenta el
   cálculo aquí en vez de proponer un valor nuevo.
4. **Disco despierto — hipótesis abierta, sin implementar.** Se buscó
   `storage_spin()`/`call_storage_idle_notifys()`/`ata_spin` en
   `aura_musicflow.c`/`aura_music.c`/`aura_albumart.c`: **ningún**
   resultado (el único `call_storage_idle_notifys()` del árbol de Aura
   vive en `aura_firmware_switch.c`, la pantalla de conmutación de
   familia por USB, sin relación). Aura tampoco hace nada explícito
   para mantener el HDD despierto durante un barrido de Music Flow, así
   que moonlit tampoco inventa nada nuevo aquí — queda como hipótesis
   sin verificar en hardware real (M12): si el HDD se duerme entre una
   lectura y la siguiente durante un scroll largo, el costo de
   "despertarlo" podría dominar sobre cualquier mejora de este commit,
   y ni Aura ni moonlit lo evitan hoy.
5. **Prioridad de LRU** (`claim_slot()`): al desalojar, el primer barrido
   ignora cualquier candidato a distancia `≤ MAREA_VISIBLE_RADIUS+1` del
   destino actual (nunca desaloja lo visible) aunque sea el "más lejano"
   visto hasta ese punto; solo si los 37 slots caen enteros dentro de esa
   ventana (biblioteca minúscula) un segundo barrido cae al criterio
   viejo, sin la exclusión, para no dejar la función sin slot.

**Antes / después:**

| | Antes (D-053) | Después (D-057) |
|---|---|---|
| Lecturas por cuadro (animando) | 0 | ≤ 1, plana, solo con clave ya conocida |
| Cadencia del tick ocioso | 1 carga cuando `!animating()` | hasta 4 cargas / 15 ms por vuelta |
| Cuándo sondea `HZ/20` | solo `animating()` | `animating() \|\| wants_ticks()` |
| Radio de precarga ociosa | 6, parejo | 10 adelante / 4 atrás (`s_last_scroll_dir`) |
| `MAREA_CACHE_SLOTS` | 37 | 37 (sin cambio — 15 caben de sobra) |
| `.bss` (`arm-elf-eabi-size`) | 8 571 420 | 8 571 420 (+0 B) |

**Verificación:**

- `make -C firmware/rockbox/apps/metro/test test`: verde, 14 suites
  (incluye `test_marea_prefetch`, nueva, 35/35 checks —
  `moonlit_marea_prefetch_order()` es lógica pura de índices → lista
  ordenada por distancia/dirección, sin E/S).
- `firmware/tools/build_sim.sh`: sin warnings nuevos (los
  `-Wmissing-field-initializers` de `tile_cols`/`empty_message` son
  previos, presentes en 8+ archivos ajenos a este cambio).
- `grep -n 'decode\|song_count\|tagcache' apps/metro/moonlit_screen_marea.c`
  dentro de los cuerpos de `show_carousel()`/`get_slot_for()`: vacío
  (confirmado acotando por rango de línea de cada función, no solo por
  archivo completo — las coincidencias reales caen en `load_pending_slot()`,
  `try_frame_bounded_read()`, `draw_panel()` y comentarios).
- Capturas (simulador, biblioteca de `gen_test_media.sh`, 11 álbumes,
  moonlitcache pre-horneada — 5 `.pfraw` + 6 `.none`, mismo estado
  "todo ya resuelto en disco" que el reporte del dueño):
  `docs/screenshots/v0.1.3-marea-settle-3ticks.png`/`-30ticks.png`
  (`sim_shot.sh … {3,30} "WAIT,SELECT,WAIT,SELECT,WAIT,SCROLL_FWD,SCROLL_FWD,SCROLL_FWD"`).
  PIL sobre la región del carrusel (`(0,0,152,240)`): **21 654 / 36 480**
  px distintos. Cifra alta **por geometría, no por carga**: se verificó
  por separado (variando los ticks de asentamiento de 3 a 30 de uno en
  uno) que el asentamiento de la 3ª rueda cae entre los ticks 16 y 20
  (~160-190 ms, consistente con `MAREA_SCROLL_ANIM_MS` = 220 ms) — la
  captura de 3 ticks es de facto **a medio deslizar**, no en reposo, así
  que casi todo el carrusel ocupa una posición de perspectiva distinta
  a la de 30 ticks; el diff de posición domina el conteo. Lo que sí se
  verificó visualmente en la captura de 3 ticks (y en una prueba
  adicional con 8 `SCROLL_FWD`): **ningún monograma transitorio** —
  las tapas visibles a medio deslizar muestran color plano real (las
  carátulas de prueba son de color sólido, D-030), nunca la tarjeta con
  inicial; el único monograma observado en cualquier corrida ("M",
  álbum real `Cultura Profética/M.O.T.A`, sin `cover.jpg` ni APIC) es
  un `.none` genuino, no un efecto de carga tardía. **Limitación de esta
  verificación de simulador:** con solo 11 álbumes y ~1-2 s de espera
  ociosa antes de cada scroll, el radio de precarga **viejo** (6, parejo)
  ya alcanzaba a cubrir toda la biblioteca antes de que el usuario
  empezara a scrollear — esta prueba no distingue el comportamiento
  viejo del nuevo a esta escala; la ganancia real (biblioteca de 1 083
  álbumes, `HZ/10` en reposo) solo se puede medir en hardware (M12,
  igual que el resto de Marea).
- `RBDEV_TOOLCHAIN=… firmware/tools/build_target.sh --firmware`: exit 0,
  24 warnings (idénticos a los de antes de este commit — `tile_cols`/
  `empty_message`, ninguno nuevo). `.bss`: antes = 8 571 420 (v0.1.3,
  D-056, confirmado reconstruyendo con `git stash` antes de este
  commit), después = 8 571 420 (**+0 B** — `moonlit_marea_prefetch.c`
  no añade estado estático, el peek de `metro_music.c` reutiliza el
  memo de 48 entradas ya existente, y los `order[]` de la precarga
  direccional son locales de pila) — bajo el techo D-043 de 8 574 076,
  con el mismo margen que D-056 dejó (2 656 B).
- `git status --short`: limpio al final (ver commit).

**Hipótesis abiertas:** disco despierto durante un barrido largo (item
4, sin verificar, ni en Aura ni aquí); si el radio 10/4 es el óptimo
para una biblioteca de 1 083 álbumes o si conviene ensancharlo una vez
medido en hardware real (mismo estatus "experimental hasta M12" que el
resto de Marea, D-014/D-043); si `ART_KEY_MEMO_N` = 48
(`metro_music.c`) alcanza para que la precarga direccional de 15
álbumes no se pise con otros consumidores de esa clave
(`metro_screen_hub.c` grid de álbumes) en una sesión larga.

**D-058 — un solo barrido de biblioteca, responsivo, con conteo de
una sola sesión de tagcache por álbum.** Reporte del dueño en hardware
real (iPod 6G, 1 083 álbumes, 979 `.pfraw`, ~57 pendientes): al entrar
a Música, "preparando biblioteca/carátulas" mostró "0/10" (fase 1,
tagcache), tras ~5 min cambió a "0/57" (fase 2) y quedó **10 minutos**
sin avanzar y sin responder a MENU -- forzó reinicio (SELECT+MENU).
Peor que antes de D-049, que al menos mostraba progreso.

**Diagnóstico (verificado leyendo el código en el commit anterior a
este, `3ae50600`):**

1. **Barrido completo duplicado.** `run_phase_art()`
   (`moonlit_screen_library.c:175-185`) llamaba
   `moonlit_art_pending_count()` (`moonlit_art_cache.c:238-254`), que
   recorre TODOS los álbumes vía `count_uncached_now()`
   (`:219-236`) para decidir el total de la pantalla. `moonlit_art_precache()`
   (`:256-313`) volvía a llamar `count_uncached_now(count, theme)`
   desde cero en su propia línea 271 -- el mismo barrido de ~1083
   álbumes otra vez, antes de procesar un solo álbum. Cada álbum,
   cacheado o no, necesita `metro_music_album_art_key()`
   (`metro_music.c:625-661`) para construir la ruta a comprobar
   (`precache_path_at()`, `moonlit_art_cache.c:173-181`), y esa
   función, en frío (memo de 48 entradas insuficiente para 1083
   álbumes), cae a `compute_album_art_key()` (`:597-623`): DOS
   sesiones `tagcache_search` separadas -- una vía
   `metro_music_songs_of_album()` (`:549-552` → `run_search()`,
   `:315-373`, con `tag_title` filtrado por `tag_album`) solo para
   obtener un `track.seek`, y otra vía `tagcache_search(&tcs, tag_filename)`
   + `tagcache_retrieve()` + una asignación manual `tcs.idx_id = track.seek`
   (`:614`) + `tagcache_get_numeric()`. En hardware real esto puede
   tardar minutos para 1083 álbumes, y se paga **dos veces completas**
   antes de que aparezca cualquier progreso.
2. **Sin capacidad de interrupción durante el conteo.**
   `count_uncached_now()` solo hacía `yield()` cada 32 iteraciones
   (`:232-233` de la versión vieja) -- no redibujaba, no sondeaba
   botones. `precache_should_abort()`
   (`moonlit_screen_library.c:149-154`) solo se consultaba ENTRE
   álbumes dentro del loop principal de `moonlit_art_precache()`
   (`moonlit_art_cache.c:303` de la versión vieja), nunca durante el
   conteo -- si el barrido es lento, la pantalla se ve congelada y
   MENU no puede abortar hasta que termina TODO el barrido (dos veces,
   por el punto 1).
3. **`compute_album_art_key()` con dos sesiones donde bastaba una.**
   Ver punto 1 -- la sesión A (`tag_title` + filtro `tag_album`, vía
   `metro_music_songs_of_album()`) solo servía para aprender un
   `track.seek` que la sesión B (`tag_filename` + `tagcache_retrieve()`
   + `tcs.idx_id` manual) volvía a resolver por separado.

**Decisión** (este commit, D-058):

1. **Barrido único.** `moonlit_art_precache()` ya no recalcula
   `pending` -- ahora recibe el total ya conocido como parámetro:
   `bool moonlit_art_precache(int pending, moonlit_art_progress_fn
   progress_cb, moonlit_art_abort_fn should_abort)`
   (`moonlit_art_cache.h`/`.c`). `run_phase_art()`
   (`moonlit_screen_library.c`) pasa el `pending` que ya obtuvo de
   `moonlit_art_pending_count()`. El `for` de `moonlit_art_precache()`
   SIGUE llamando `moonlit_art_is_resolved()` por álbum -- es la única
   forma de saber cuáles procesar, `pending` solo alimenta el
   denominador de la barra. `grep -n 'count_uncached_now'
   apps/metro/moonlit_art_cache.c` da 4 líneas: la definición, un
   comentario en `precache_path_at()`, la única llamada real (dentro
   de `moonlit_art_pending_count()`) y un comentario en
   `moonlit_art_precache()` documentando por qué esa llamada YA NO
   está ahí.
2. **Barrido interrumpible con heartbeat.** `count_uncached_now()`
   (única función que recorre TODA la biblioteca, ahora llamada una
   sola vez) recibe `progress_cb`/`should_abort` (mismos tipos que
   `moonlit_art_precache()` -- `moonlit_art_progress_fn` reutilizado
   con la forma `(checked, total_albums)` en vez de `(done, pending)`,
   que todavía no se conoce durante el conteo) y los consulta cada
   `SWEEP_HEARTBEAT_ALBUMS` = 8 álbumes o `SWEEP_HEARTBEAT_TICKS` =
   `HZ/4` (lo que ocurra primero) -- nunca más de ~250 ms sin sondear
   MENU, muy por debajo del margen de ~1 s pedido. Si `should_abort()`
   devuelve true, el barrido corta y `moonlit_art_pending_count()`
   devuelve **-1** (no memoiza un conteo parcial); `run_phase_art()`
   trata `pending < 0` igual que un abort de la precarga (`return
   false`). Pantalla nueva `LANG_LIBRARY_PHASE_SCAN` ("revisando
   carátulas"/"checking covers", `metro_lang.h`/`.c`) distingue esta
   fase de conteo de `LANG_LIBRARY_PHASE_ART` ("preparando
   carátulas", la precarga real) y muestra `checked/total_albums` --
   el total REAL de la biblioteca, no el pendiente, que aún no se
   sabe. `s_abort_requested` (`moonlit_screen_library.c`) se resetea
   ANTES del conteo, no solo antes de la precarga, porque ahora el
   conteo también puede tardar y abortarse solo.
3. **`compute_album_art_key()` de una sola sesión.** En vez de
   `metro_music_songs_of_album()` + una segunda búsqueda por
   `tag_filename`, ahora: `tagcache_search(&tcs, tag_filename)` +
   `tagcache_search_add_filter(&tcs, tag_album, album_seek)` + UNA
   `tagcache_get_next(&tcs, path, sizeof(path))` que ya entrega la
   ruta, y `tagcache_get_numeric(&tcs, tag_mtime)` sobre la MISMA
   sesión antes de `tagcache_search_finish()`. **No hace falta la
   asignación manual `tcs.idx_id = track.seek`** que la versión vieja
   necesitaba: `tagcache_get_next()` (`tagcache.c:2058-2075` →
   `get_next()`, `:1909-2011`) entra por la rama de "relative fetch"
   cuando `tcs->filter_count > 0` (nuestro caso, por el filtro de
   `tag_album`) y ahí mismo hace `tcs->idx_id = seeklist->idx_id`
   (`:1951`) antes de devolver el resultado -- exactamente lo que
   `tagcache_get_numeric()` (`:1338-1352`) necesita leer.
   **Mismo resultado (ruta, mtime) que el método viejo, verificado por
   inspección de código, no por test host (no existe stub de tagcache
   para host, ver más abajo):** `build_lookup_list()`
   (`tagcache.c:1663-1712` para el camino RAM, análogo para disco)
   recorre el índice maestro en orden de posición aplicando los
   filtros (`idx->tag_seek[filter_tag] == filter_seek`) -- ese
   predicado no mira `tcs->type` para nada, así que la primera entrada
   del índice maestro que cumple `tag_album == album_seek` es la misma
   sin importar si el tipo de búsqueda es `tag_title` (método viejo,
   sesión A) o `tag_filename` (método nuevo); el `uniqbuf` que la
   sesión A activaba (`run_search()`, `metro_music.c:315-330`) no
   cambia esto porque nunca hay nada que deduplicar en la PRIMERA
   coincidencia. La sesión B vieja retomaba ese mismo `idx_id`
   (`track.seek`) a mano; la nueva lo obtiene automáticamente del
   mismo camino.
4. **Salvaguarda de reloj -- documentada, no implementada aparte.**
   No se agregó un timeout explícito de ~20 s para el conteo: el
   mecanismo del punto 2 (heartbeat cada ~250 ms con sondeo de MENU no
   bloqueante) ya le da al usuario una salida en <1 s en cualquier
   punto del barrido, sea cual sea su duración total -- un timeout
   automático adicional sería redundante con "el usuario puede salir
   cuando quiera" y añadiría un segundo criterio de corte a
   coordinar con el primero sin beneficio claro.

**Hipótesis del reporte de producción, documentada y NO
implementada:** un directorio plano con 1000+ archivos en
`moonlitcache/art/` y `/.aura/thumbs/albums/` podría ser lento de
listar/abrir en FAT real (fragmentación de directorio, cada `open()`
recorriendo entradas linealmente) y contribuir al estancamiento
observado, más allá del barrido duplicado del punto 1. **No se
implementa sharding de directorios en esta pasada** -- es un cambio
más grande, que además tocaría el directorio `/.aura/thumbs/`
COMPARTIDO con Metro-Aura/Aura-Studio (D-054/D-055, contrato
`CONTRATO-moonlit-studio.md` v3) y requeriría coordinar el formato de
ruta entre ambos repos antes de tocarlo. Queda como decisión futura,
solo si el punto 1 (que por sí solo explica una tardanza de minutos)
no basta para resolver el reporte en hardware real.

**Antes / después:**

| | Antes (D-049/D-056) | Después (D-058) |
|---|---|---|
| Barridos completos de la biblioteca por entrada a Música | 2 (`moonlit_art_pending_count()` + `moonlit_art_precache()`) | 1 |
| Sesiones `tagcache_search` por álbum en frío (fuera del memo de 48) | 2 (`compute_album_art_key()`) | 1 |
| MENU durante el conteo | no se sondea (congelado hasta terminar) | sondeado cada ≤8 álbumes / `HZ/4` ticks |
| Pantalla durante el conteo | ninguna (pantalla previa sin cambios) | "revisando carátulas N/total_albums" |
| `moonlit_art_pending_count()` firma | `(void)` | `(progress_cb, should_abort)`, puede devolver -1 (abortado) |
| `moonlit_art_precache()` firma | `(progress_cb, should_abort)` | `(pending, progress_cb, should_abort)` |
| `.bss` (`arm-elf-eabi-size`) | 8 571 420 | 8 571 420 (+0 B) |

**Verificación:**

- `make -C firmware/rockbox/apps/metro/test test`: verde, 14 suites
  (mismas de D-057 -- `moonlit_art_cache.c`/`metro_music.c` no son
  host-testables, sin stub de `tagcache.h`/`metro_settings.h`/
  `metro_albumart.h` para host, igual que antes de este commit; sus
  cabeceras ya documentaban esto. No se agregaron tests host nuevos
  por esta razón -- el cambio de firma de `moonlit_art_precache()` y
  la reescritura de `compute_album_art_key()` solo se pudieron
  verificar por inspección de código (punto 3 de la decisión) y en
  simulador (abajo), no con `cc` de host).
- `firmware/tools/build_sim.sh --reconfigure`: 30 warnings, ninguno en
  `moonlit_art_cache.c`/`metro_music.c`/`moonlit_screen_library.c`/
  `metro_lang.c` (confirmado filtrando el log por esos archivos);
  mismas categorías preexistentes que D-057 documentó (`tile_cols`/
  `empty_message`/`-Wformat-truncation` de `metro_thumbs.c`).
- **Responsividad de MENU durante el conteo, verificada en
  simulador:** se generaron 300 álbumes sintéticos de una pista
  (`ffmpeg -c copy` reetiquetando `metro-test.mp3`, sin arte --
  `firmware/test-media/Music/Load Test Artist/`, no versionado) sobre
  los ~11 álbumes de fixtures existentes (312 en total) y se copiaron
  a `simdisk/`. Con un `DEBUGF("moonlit_art: sweep %d/%d")` temporal
  en el heartbeat de `count_uncached_now()` se confirmó que dispara
  cada 8 álbumes en punto (`0/312, 8/312, 16/312, ... 304/312`, 39
  latidos para el barrido completo) sin cortes. Para forzar una
  ventana de abort observable (el sim, sin HDD real, barre 312 álbumes
  en bien menos de 1 s -- no reproduce la escala de minutos del
  reporte), se insertó temporalmente un `usleep(3000)` por álbum
  (`#ifdef MOONLIT_DEBUG_SLOW_SWEEP`, compilado solo con `make
  EXTRA_DEFINES=... -DMOONLIT_DEBUG_SLOW_SWEEP`, revertido antes del
  build final -- no está en el árbol commiteado, `grep -n
  'MOONLIT_DEBUG_SLOW_SWEEP\|usleep' apps/metro/moonlit_art_cache.c`
  vacío). Con esa build, inyectando `METRO_SIM_BUTTONS=SELECT,MENU`
  (`firmware/tools/sim_shot.sh`-style automation), el log mostró
  `moonlit_art: sweep aborted at 16/312` y el screendump capturado en
  ese instante muestra la pantalla nueva a medio dibujar ("revisando
  carátulas", "16 de 312") superpuesta a la transición de salida; un
  dump posterior confirma que la app vuelve limpiamente al pivote de
  Música (pestañas marea/reproducir), nunca se queda colgada ni entra
  a Marea con datos a medias. **Limitación de esta verificación:** el
  `usleep()` es un hack de host, no reproduce el costo real de
  E/S en HDD -- prueba que el mecanismo de aborto FUNCIONA
  correctamente cuando se le da tiempo de dispararse, no cuánto tarda
  en hardware real (eso solo se puede medir en el iPod del dueño,
  M12). Sin el `usleep`, la build final normal se re-verificó de
  extremo a extremo (312 álbumes, sin MENU) llegando limpiamente hasta
  Marea con los 39 latidos de heartbeat en el log.
- `grep -n 'count_uncached_now' apps/metro/moonlit_art_cache.c`: 4
  líneas -- definición, comentario en `precache_path_at()`, la única
  llamada real (`moonlit_art_pending_count()`), comentario en
  `moonlit_art_precache()` explicando la que ya no está.
- `RBDEV_TOOLCHAIN=… firmware/tools/build_target.sh --firmware`:
  exit 0 tanto en el commit anterior (`3ae50600`, baseline vía `git
  stash`) como con este cambio. 26 warnings en ambos (ninguno nuevo;
  la cifra de 24 que D-057 documentó ya no coincide con el HEAD actual
  por cambios ajenos a este commit -- reconfirmado comparando
  baseline vs. D-058 lado a lado, ambos 26). `.bss`: antes = 8 571 420,
  después = 8 571 420 (**+0 B** -- sin estáticos nuevos, solo
  reutilización de los typedefs/scratch existentes); `.text` +212 B.
  Bajo el techo D-043 de 8 574 076, mismo margen de 2 656 B que D-057.
- `git status --short`: limpio (ver commit); la biblioteca sintética
  de 300 álbumes vive en `firmware/test-media/` (gitignored) y se
  borró al terminar la verificación.

**Hipótesis abiertas:** directorio plano en `moonlitcache/art/`/
`/.aura/thumbs/albums/` como factor adicional de lentitud en FAT real
(documentada arriba, no implementada); si `SWEEP_HEARTBEAT_ALBUMS` = 8
/ `SWEEP_HEARTBEAT_TICKS` = `HZ/4` es la cadencia óptima para hardware
real o conviene ajustarla una vez medido en el iPod del dueño (mismo
estatus "experimental hasta M12" que Marea, D-014/D-043); si el
conteo de 1083 álbumes en hardware real, ahora en un solo barrido en
vez de dos, cae dentro de un tiempo que el usuario considere
razonable, o si además hace falta paralelizar/cachear más agresivo el
conteo mismo (fuera del alcance de este commit).

**D-059 — caché maestra compartida `/.aura/art/` (contrato v16):
constructor en segundo plano reemplaza la pantalla de "preparando
carátulas".** D-058 hizo el conteo/precarga de D-049 responsivo y de
una sola sesión de tagcache por álbum, pero no cambió la naturaleza
del problema: la pantalla "preparando biblioteca" seguía teniendo una
FASE 2 completa (contar + decodificar) delante de Música, y **cada
familia (Aura, Metro, moonlit) decodifica y cachea sus propias
carátulas por separado** aunque las tres lean del mismo disco --
Studio ya define un contrato v16 de imagen maestra compartida
(`/.aura/art/`) para que un JPEG se decodifique UNA vez, sin importar
cuál familia lo hizo primero. Este commit implementa ese contrato del
lado de moonlit y usa su llegada para retirar la fase 2 por completo
en vez de optimizarla más.

**Formato en disco** (`moonlit_master_art.h`, módulo puro, D-042):
`/.aura/art/{albums,artists,photos}/<clave>.art` -- cabecera LE de 16 B
(`magic 'MAST'`, `width`, `height`, `flags=0`, `reserved=0`) + píxeles
RGB565 LE fila-contigua, cuadrados SIN esquinas ni tema (recorte
fill-and-center-crop puro de la fuente): álbumes/artistas 130×130,
fotos 80×80. `<clave>.none` (0 B) es el marcador negativo compartido,
mismo esquema de clave que D-055/D-056
(`a-<crc32 hex8 ruta pista representativa>.<mtime>` = la clave que ya
memoiza `metro_music_album_art_key()`; `r-`/`p-` análogas para
artistas/fotos vía `metro_music_album_art_source()`, sin memo, D-059).
Cada familia DERIVA su tamaño de trabajo desde la maestra al cargarla
-- nunca al revés: moonlit reduce 130→120 con filtro de caja entero +
`moonlit_art_mask_corners()` para Marea, y 130→80 para la rejilla
(`metro_thumbs.c`). Escritura atómica (`<ruta>.tmp` + `rename()`) para
que una familia hermana nunca lea una maestra a medio escribir.

**Decisión hilo vs. plan B -- HILO real, con evidencia citada en el
propio código (`moonlit_master_art_builder.c`, comentario de cabecera,
verificado leyendo las tres fuentes, no supuesto):**

- `tagcache_search()` incrementa un `write_lock` global compartido por
  todas las sesiones abiertas, así que un commit concurrente espera a
  que termine cualquier búsqueda en curso; todo el estado de una
  búsqueda vive dentro del `struct tagcache_search` del llamador; la
  copia en RAM (`tagcache_ram`) es de solo lectura para búsquedas.
  Rockbox reparte CPU cooperativamente (un cambio de hilo solo ocurre
  en `yield()`/`sleep()`/una llamada bloqueante), así que el
  incremento no atómico de `write_lock` es seguro entre hilos. El
  constructor usa `metro_music_album_art_source()` (sin memo) --
  precisamente para que la memoización de 48 entradas que la UI lee
  sin candado dentro de un cuadro de Marea (`metro_music_album_art_key_peek()`,
  D-053/D-057) nunca tenga un segundo escritor.
- El decodificador JPEG (`apps/recorder/jpeg_load.c`) guarda su estado
  en UN `static struct jpeg` y cede CPU por fila de MCU -- dos decodes
  en dos hilos SÍ se entrelazarían y corromperían el resultado. Por
  eso todo `read_jpeg_file()`/`clip_jpeg_file()` bajo `apps/metro/`
  ahora corre bajo `moonlit_master_art_lock()` (mutex recursivo):
  `metro_albumart.c` (UI y constructor comparten el mismo
  `decode_file_into()`/`decode_embedded_into()`) y
  `metro_screen_photo_viewer.c`. El hilo de buffering de audio nunca
  decodifica JPEG aquí (moonlit no tiene skin engine, así que la
  reproducción nunca pide arte al buffering).
- El constructor tiene su propio scratch (`METRO_ALBUMART_SCRATCH_SIZE`)
  y su propio `struct mp3entry` -- nunca toca `s_scratch`/`s_track_id3`
  de `metro_albumart.c` (UI-only, y Ahora Suena guarda un puntero
  dentro del suyo).

Con las tres condiciones verificadas, un hilo Rockbox real de baja
prioridad (patrón `apps/plugins/pictureflow.c`) es seguro y más simple
que el plan B (un elemento por vuelta ociosa tras ≥2 s sin input): un
hilo dedicado no compite por el presupuesto de `metro_thumbs_tick()`/
`moonlit_screen_marea_tick()` en la vuelta ociosa de `metro_main.c`, y
`BUILDER_PRIORITY = PRIORITY_BACKGROUND + 2` (por debajo del propio
hilo de tagcache y de `PRIORITY_BUFFERING`) es lo más parecido que
ofrece este kernel a "suspendido mientras el audio bufferea" -- no
existe un bit `audio_status()` para "bufferea ahora mismo".
`sleep(HZ/20)` entre elementos con trabajo real (`yield()` si el
elemento ya estaba resuelto) más `moonlit_master_art_builder_pause()`
(consultado por el hilo entre elementos, nunca a mitad de uno) dejan
el disco y la CPU libres durante el scroll de Marea y las transiciones
de pantalla.

**Qué se integró en el hilo principal (`metro_main.c`)**, quedaba
pendiente del commit anterior a este:
`moonlit_master_art_builder_init()` una vez, antes de que cualquier
decode de carátula sea posible; `moonlit_master_art_builder_poll()` en
cada vuelta ociosa (no-op de un `bool` tras crear el hilo la primera
vez que `metro_music_db_ready()` y `tagcache_is_fully_initialized()`
se sostienen); `moonlit_master_art_builder_pause(true/false)`
alrededor de todo el bloque de `metro_transitions_*` y mientras
`moonlit_screen_marea_animating()` es cierto (recalculado cada vuelta,
nunca queda pegado en `true` si el usuario sale de Marea a mitad de
scroll); y el emparejamiento `metro_thumbs_take_waiting()` +
`moonlit_master_art_builder_generation()` para repintar la rejilla
solo cuando el generador realmente avanzó, no en cada vuelta ociosa
mientras el constructor sigue ocupado (evita una tormenta de
repintados completos sin nada nuevo que mostrar).

**Qué encontró este commit ya hecho** (agente anterior, cortado a
mitad de sesión): `moonlit_master_art.{c,h}` (formato puro, 71/71
checks en `test_master_art.c`, ya en `apps/SOURCES`/`test/Makefile`),
`moonlit_master_art_builder.{c,h}` (hilo completo: `init/poll/kick/
pause/active/generation/hint_album/lock/unlock`, migración de
`moonlit_art.none` heredados, gc reubicado como `run_gc()` al final de
cada pasada), `metro_albumart.{c,h}` (decodes `_raw`/`_ui` compartidos
entre UI e hilo constructor, con `moonlit_master_art_lock()` alrededor
de cada `read_jpeg_file()`/`clip_jpeg_file()`), `metro_thumbs.{c,h}`
(`metro_thumbs_decode_via_master()` + `METRO_THUMB_OK/FAIL/WAITING`),
`metro_music.{c,h}` (`metro_music_album_art_source()` sin memo),
`metro_settings.{c,h}` (`metro_settings_shared_art_dir()`),
`metro_photos.h` (`METRO_PHOTOS_DIR`/`_EXT_*` exportados),
`moonlit_art.{c,h}`/`moonlit_art_cache.{c,h}` ya reducidos al nuevo
contrato (`moonlit_art_load_for_album()` devuelve
`enum moonlit_art_result { LOADED, NONE, WAITING }`), `test/dir.h`
(stand-ins `dir_exists()`/`mkdir()` de un argumento) y `test/test_art.c`
ya migrados.

**Qué completó este commit** (verificado leyendo cada consumidor de
las funciones que D-059 cambió de firma, no solo los archivos que la
nota del encargo señalaba):

1. `moonlit_screen_library.c`/`.h`: se retira la FASE 2 completa
   (`run_phase_art()`, `precache_progress()`, `count_progress()`,
   `precache_should_abort()`, `s_abort_requested`, el barrido de gc al
   final) -- la pantalla ahora es solo la fase 1 (tagcache);
   `LANG_LIBRARY_PHASE_SCAN`/`LANG_LIBRARY_PHASE_ART` se retiran de
   `metro_lang.{c,h}` (quedaban sin ningún llamador). `moonlit_art_gc()`
   ya no existía en `moonlit_art_cache.h` (el gc vive en
   `moonlit_master_art_builder.c` desde el punto anterior) -- este
   archivo todavía la llamaba: no compilaba.
2. `metro_screen_hub.c`: sus tres `metro_thumb_source.decode()`
   (álbumes, artistas, fotos) todavía llamaban
   `metro_thumbs_decode_jpeg_cover()`/`metro_albumart_decode_track_cover()`,
   ninguna de las dos existe ya -- no compilaba. Reescritos sobre
   `metro_thumbs_decode_via_master()` + `metro_albumart_decode_file_ui()`/
   `_decode_track_cover_ui()`, con la clave de maestra resuelta vía
   `moonlit_art_master_path()`/`moonlit_art_master_file_path()`.
3. `moonlit_screen_marea.c` -- el más grave, silencioso (compilaba,
   pero con la carga de carátula rota):
   - `load_pending_slot()` hacía `if (moonlit_art_load_for_album(...))`
     tratando el resultado como `bool`. `moonlit_art_load_for_album()`
     devuelve el nuevo `enum moonlit_art_result` con `MOONLIT_ART_LOADED
     == 0` -- exactamente invertido: el `if` era **falso** en el caso de
     ÉXITO. Sin este fix, Marea jamás mostraría una carátula cargada
     con éxito por esta vía (aunque `try_frame_bounded_read()`, más
     abajo, sí podía enmascararlo en parte). Corregido a un `switch`
     explícito sobre el enum, con un tercer estado nuevo
     `MAREA_ART_WAITING` para `MOONLIT_ART_WAITING` (constructor
     ocupado, ya hinteado) -- antes solo existían PENDING/LOADED/MISSING.
   - `try_frame_bounded_read()` llamaba `moonlit_art_pfraw_path_peek()`/
     `moonlit_art_read_pfraw()`/`moonlit_art_none_path()`/
     `moonlit_art_none_exists()` -- ninguna existe desde que D-059
     retiró el `.pfraw` privado. No compilaba. Reescrito sobre
     `moonlit_art_master_path_peek()` + `moonlit_art_derive_from_master()`
     (lectura + remuestreo en una llamada, ya no hace falta el
     parámetro `theme` -- el horneado de esquinas lo aplica
     `moonlit_art_cache.c` con el tema vigente en cada derivación) +
     `moonlit_master_art_none_path()`/`_none_exists()`.
   - Nuevo `requeue_waiting_slots_if_generation_moved()`: un slot
     WAITING no se reintenta solo en la siguiente vuelta (se
     confundiría con un PENDING recién reclamado) -- se reintenta
     cuando `moonlit_master_art_builder_generation()` avanza, señal de
     que el constructor escribió (o marcó `.none`) algún elemento.
     Llamado desde `moonlit_screen_marea_tick()` y
     `moonlit_screen_marea_wants_ticks()` (que ahora también cuenta un
     slot WAITING como "todavía falta", igual que PENDING, para que
     `metro_main.c` siga pidiendo HZ/20 mientras se espera al
     constructor). El dibujo (`slot->art == MAREA_ART_LOADED`) no
     necesitó cambios: cualquier estado que no sea LOADED ya mostraba
     el monograma/relleno.
4. `metro_main.c`: no llamaba a `moonlit_master_art_builder_init()`/
   `_poll()`/`_pause()` en ningún lado -- el hilo constructor nunca se
   habría creado en el firmware real (aunque el módulo compilara y sus
   tests pasaran). Se agregó `_init()` al principio de `metro_main()`
   (antes de que cualquier `decode_file_into()` sea alcanzable),
   `_poll()` en la vuelta ociosa, `_pause()` alrededor del bloque de
   transiciones y espejado del estado de scroll de Marea, y el
   emparejamiento generación/`metro_thumbs_take_waiting()` descrito
   arriba.

**Verificación:**

- `make -C firmware/rockbox/apps/metro/test test`: 17 suites, todas en
  verde, incluida `test_master_art` (71/71, ya existía) y `test_art`
  (17/17, ya migrado).
- `firmware/tools/build_sim.sh`: compila limpio (solo warnings
  preexistentes de `-Wformat-truncation`/`-Wmissing-field-initializers`,
  ninguno nuevo de lógica). Con `/.aura/art/`, `/.aura/thumbs/` y
  `moonlitcache/` vacíos, arranque de 1200 ticks SIN entrar a Música:
  `find .aura/art -name '*.art' | wc -l` → 21, `*.none` → 307 (312
  álbumes de la biblioteca sintética de prueba, la mayoría sin
  carátula por diseño de la fixture); cabecera verificada con
  `python3`/`struct`: `MAST`, 130×130 en álbumes, 80×80 en fotos.
  Entrar a Música (`SELECT`) aterriza directo en el pivote "marea" sin
  ninguna barra de progreso de por medio (capturado en pantalla,
  comparado con el bug de D-058: cero fases entre el hub y Marea).
  Dentro de Marea, dos álbumes con carátula real (`Analog Dreams` de
  `Wheel & Click`, `First Light` de `Aura Test Combo`) muestran su
  imagen decodificada -- captura `docs/screenshots/v0.1.5-marea-from-master.png`
  -- sin ninguna línea `moonlit_art: decode` en el log (`grep decode`
  sobre stdout de `rockboxui`): la maestra ya estaba escrita por el
  constructor antes de entrar a Música, Marea solo la leyó y derivó.
  La rejilla de Fotos (`fotos` → `todos`) muestra las miniaturas reales
  de la fixture, también sin placeholders. Segundo arranque: `stat -f
  "%m %z"` de tres maestras (dos álbumes, una foto) idéntico byte a
  byte y tick a tick frente al primer arranque -- nada se reescribe.
- `RBDEV_TOOLCHAIN=… firmware/tools/build_target.sh`: exit 0 (firmware
  y bootloader). `.bss` (`arm-elf-eabi-size firmware/build-ipod6g/rockbox.elf`):
  **antes = 8 571 420** (valor limpio conocido, D-058), **después =
  8 456 508** (**-114 912 B**), bajo el techo D-043 de 8 574 076 con
  **117 568 B de margen** -- el ahorro viene de consolidar los scratch
  de decode JPEG que antes duplicaban `metro_albumart.c` (`s_scratch`,
  METRO_ALBUMART_SCRATCH_SIZE) y `metro_thumbs.c` (`SCRATCH_MAX_SRC_PX`
  128×128×2×2 = 65 536 B) en un solo scratch compartido más pequeño
  (`moonlit_master_art_builder.c`: `s_decode` = el mismo
  `METRO_ALBUMART_SCRATCH_SIZE`, `s_master` = 130×130 fb_data =
  33 800 B) mas el retiro completo del `.pfraw` privado y su
  maquinaria de precarga (D-042/D-058), pese a sumar la pila del hilo
  nuevo (`BUILDER_STACK_SIZE` = `DEFAULT_STACK_SIZE + 0x2000`).
- `git status --short`: limpio tras los commits de este cambio.

**Hipótesis abiertas:** si el conteo de álbumes que ve el constructor
en una pasada (`enumerate_albums()`, 312 en la fixture de prueba) varía
entre arranques por razones no relacionadas con D-059 (orden de commit
de tagcache en un `tagcache_start_scan()` desde cero) -- observado en
la fixture sintética, no en la biblioteca real del dueño, fuera del
alcance de este commit; si el ritmo `BUILDER_ELEMENT_PACE = HZ/20`
resulta demasiado lento/rápido en hardware real con 1083 álbumes +
fotos + artistas (mismo estatus "experimental hasta M12" que Marea,
D-014/D-043) -- el reporte original de D-058 (10 min bloqueados) ya no
puede repetirse (no hay pantalla que bloquear), pero el TIEMPO TOTAL
hasta que la última carátula real llega sigue sin medirse en ese
hardware; si conviene que el constructor también compita por
prioridad con el hilo de escaneo de tagcache durante un
`tagcache_start_scan()` en curso (hoy ambos pueden correr a la vez,
sin que este commit lo haya necesitado forzar de otro modo).
