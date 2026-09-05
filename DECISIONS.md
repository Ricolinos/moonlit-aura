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

---

**D-060 — Esperar un `commit()` de tagcache en curso antes de rebotar
por un cambio de familia (misma corrección en paralelo en
`Aura-Firmware` AF D-342 y en `Metro-Aura` M-099).** `metro_settings.c`
comparte base con Metro-Aura desde `moonlit-fork-base` (D-003):
`metro_firmware_switch_to()` heredaba sin cambios el defecto M-090 ya
presente ahí.

**Diagnóstico.** `metro_firmware_switch_to()`
(`apps/metro/metro_settings.c:409-448` antes de este commit) llamaba
`tagcache_shutdown()` (`apps/tagcache.c:5508-5517`) y de inmediato
`rename()` + `system_reboot()`. `tagcache_shutdown()` solo vacía la
cola de escrituras numéricas diferidas (`run_command_queue(true)`,
playcount/lastplayed/rating) -- **nunca** espera un `commit()` en
curso. Un `commit()` (`apps/tagcache.c`, función `commit()`) marca el
header maestro **compartido** (`/.aura/tagcache/database_idx.tcd`,
contrato v15/D-054) `dirty = true` y lo escribe a disco de inmediato
(`current_tcmh.dirty = true; update_master_header();`, `:3538-3540`) --
antes incluso de reconstruir un solo índice -- y solo lo vuelve a
marcar limpio (`tcmh.dirty = false;`) y lo persiste
(`write_master_header()`) al final del todo (`:3593-3597`), después de
reconstruir los índices de texto y numéricos. Si el reboot del cambio
de familia cae en cualquier punto de esa ventana, el header queda con
`dirty=1` en disco. `check_all_headers()` en el siguiente arranque de
**cualquier** familia (Aura, Metro o moonlit -- el header es
compartido) ve el flag sucio, `tc_stat.ready` se queda en `false`, y
esa familia reconstruye la base entera desde cero aunque los datos en
sí estuvieran íntegros.

En moonlit este defecto es alcanzable en la práctica, a diferencia de
Metro-Aura (M-099): `moonlit_master_art_builder_poll()` sondea
`metro_music_db_ready()` desde el arranque (D-059,
`metro_main.c:438`, dentro del bucle ocioso), no solo al entrar a
Música como en Metro. Cualquier lectura de carátula que dispare una
escritura de tag numérico, o un `tagcache_start_scan()` en curso por
un marcador de sync pendiente, puede dejar un `commit()` a mitad de
camino en el instante en que el dueño pide "cambiar sistema" desde
Ajustes.

**Corrección.** Antes del primer `rename()`, `metro_firmware_switch_to()`
espera a que `tagcache_get_commit_step()` (API pública ya existente,
`apps/tagcache.h:194`) vuelva a `0`, con tope de **8 s**
(`METRO_SWITCH_COMMIT_WAIT_TICKS`, `metro_settings.c:432`), sondeando
cada `HZ/10` -- mismo patrón que `wait_for_tagcache_with_splash()`
(`metro_main.c:227-241`, tope `HZ*5`) y que el propio
`init_tagcache()` de Rockbox (`apps/main.c:379`, sondea el mismo
getter). Se eligió 8 s por encima del tope del splash (5 s) porque
aquí no hay pantalla ni feedback -- un `sleep` silencioso, ya que el
caso (una familia distinta pidiendo el cambio justo cuando hay un
commit en vuelo) es raro y no amerita una pantalla nueva -- y conviene
dar más margen que en el arranque, donde el usuario ya está mirando la
barra de progreso del splash. Si el tope se agota, el switch **procede
de todas formas**: no bloquear indefinidamente un cambio de familia
por un hilo de tagcache atascado es más importante que blindar al
100 % contra la reconstrucción ocasional -- la misma decisión que ya
toma `tagcache_prepare_shutdown()` (`apps/tagcache.c:5495-5504`, la
función de apagado limpio que Rockbox stock sí usa) al limitarse a
**negarse** si `commit_step > 0` en vez de esperarlo: aquí no podemos
negarnos (el switch ya se confirmó en el diálogo), así que esperamos
con tope en su lugar.

**Ventana residual (no cerrada, y no cerrable con la API pública).**
`tagcache_get_commit_step()` vuelve a `0` en `apps/tagcache.c:3579`,
**antes** de que el header realmente se reescriba limpio en disco
(`:3593-3597`, abrir el master fd, actualizar el conteo de entradas,
`tcmh.dirty = false`, `write_master_header()`). Entre esas dos líneas
hay un puñado de syscalls (`open`, `lseek`, `write`, `close`), no un
recorrido de biblioteca completo -- del orden de milisegundos, no de
segundos. Un reboot que cayera justo ahí seguiría dejando el header
sucio. Cerrar esta ventana exigiría una API interna de tagcache que
señalice "header ya persistido" (no existe hoy; `commit_step` es lo
único público) -- fuera de alcance de este commit, que solo puede usar
`apps/tagcache.h`. La mitigación reduce la ventana de "toda la
duración de un commit" (potencialmente segundos con una biblioteca
grande) a "unas pocas syscalls", que es la misma exposición residual
que ya acepta `tagcache_prepare_shutdown()` aguas arriba.

**Cadena de arranque hasta el primer sondeo -- verificada, NO
modificada a propósito.** `metro_main()` (`metro_main.c`):
`metro_screen_splash_show()` → `wait_for_tagcache_with_splash()`
(`:291-292`) → `metro_screen_list_init()` → `metro_screen_lock_init()`
+ `metro_screen_lock_run_if_active()` (candado, `:304-305`, R3-F7/DD-8)
→ `metro_disk_handoff()` (`:307`, que a su vez llama
`metro_run_sync_screen_if_needed()`, F6) → `redraw_current()` → bucle
principal, donde recién en la rama `MACT_NONE` corre el primer
`moonlit_master_art_builder_poll()` (`:438`). Si el candado está
armado, o si hay un marcador de sync pendiente que dispara la pantalla
F6 de "actualizando biblioteca", el primer sondeo del constructor de
carátulas (y por lo tanto el primer punto en que ese hilo podría
empezar a generar tráfico hacia tagcache) se retrasa lo que tarde el
dueño en desbloquear o lo que tarde el sync. Esto es una limitación
conocida, dejada **sin corregir a propósito**: mover el arranque del
constructor de carátulas (o cualquier trabajo de tagcache) a antes del
candado sería un cambio de superficie de seguridad distinto (el
candado existe para que nada corra antes de desbloquear, R3-F7/DD-8) y
de alcance distinto al de esta decisión -- no se hace sin que se pida
explícitamente. En la práctica no agrava el riesgo que D-060 corrige:
retrasa cuándo puede *empezar* un commit, no lo que pasa si uno ya
estaba en vuelo cuando llega un cambio de familia.

**Verificación:**
- `make -C firmware/rockbox/apps/metro/test test`: 17 suites, todas en
  verde (el defecto y su corrección viven enteros dentro de
  `metro_firmware_switch_to()`, que ningún test host ejercita --
  necesita `rename()`/`system_reboot()`/tagcache reales -- pero
  ninguna suite existente se rompió por el cambio de firma/includes).
- `firmware/tools/build_sim.sh`: compila limpio. Un switch normal (sin
  commit en curso, caso de todo el día a día) no se retrasa: la
  condición del `while` evalúa `tagcache_get_commit_step() != 0`
  primero -- con el valor en `0` el cuerpo del bucle (el único `sleep`)
  nunca corre, así que el costo agregado es una lectura de campo, no
  medible. "Cambiar sistema" del hub sigue funcionando: capturado en
  `docs/screenshots/v0.1.5-d060-cambiar-sistema-vacio.png`
  (`SCROLL_FWD×3,SELECT,SCROLL_FWD×8,SELECT` desde el hub, patrón de
  D-047) -- la fila 8 de Ajustes › General empuja `switch_page` con
  "Aura"/"Metro" en "no instalado" (ninguna familia hermana instalada
  en el disco simulado), igual que antes del cambio.
- `RBDEV_TOOLCHAIN=… firmware/tools/build_target.sh`: exit 0 (firmware
  y bootloader). `.bss` (`arm-elf-eabi-size firmware/build-ipod6g/rockbox.elf`):
  **8 456 508**, sin cambio frente al valor limpio de D-059 -- el
  arreglo solo agrega variables locales de pila (`wait_start`) y un
  `#define`, cero estáticas nuevas. Bajo el techo D-043 de 8 574 076.
- `git status --short`: limpio tras este commit (más el screenshot
  nuevo, agregado al repo).

## D-061 — "actualizar biblioteca" prepara también las carátulas; el aviso no cabía en el diálogo

**Encargo del dueño (2026-08-27).** Una opción de Ajustes **"Actualizar Biblioteca"** en las tres familias, con aviso de que puede tardar varios minutos según los archivos y el estado del disco, y que la preparación ocurra solo tras actualizar el firmware, tras un sync de Studio, o a pedido. El síntoma que la motiva es de moonlit: *"en el music flow de moonlit (marea) se tarda un poco en cargarse las imágenes de los albums; el movimiento sí es fluido pero se tarda en mostrar la imagen."*

**Lo que ya existía.** La caché maestra compartida `/.aura/art/` (D-059) y la base compartida `/.aura/tagcache/` (D-054) ya viven fuera de `/.rockbox/`, con las mismas claves (`crc_32(ruta, 0xffffffff)` + mtime) y los mismos lados (130/130/80) que Aura y Metro — verificado leyendo los tres. La fila de Ajustes también existía (`LANG_SETTING_LIBRARY` → `metro_sync_request_manual()`). Lo que faltaba: que esa preparación **incluyera las imágenes** y terminara antes de devolver el control. D-059 reemplazó la fase bloqueante de D-049/D-058 por un constructor puramente en segundo plano, y eso es exactamente lo que produce la espera reportada: entras a Marea y las carátulas van apareciendo mientras el hilo camina.

**Decisión.** Terminado el trabajo de tagcache, `metro_sync.c` entra en `METRO_SYNC_BUILDING_ART`: espera a que la base sea consultable, corre una pasada **completa** del constructor y recién entonces cierra la pantalla, con el avance en la misma pantalla de espera. Solo cuando la música estuvo en juego. MENU la pospone: el constructor no se cancela, vuelve a su cadencia de fondo. La fila pasa a "actualizar biblioteca" y su aviso lleva la advertencia de duración.

Esto **no** reabre lo que D-059 cerró. Lo que ahí bloqueaba aparecía solo, sin que nadie lo pidiera; aquí el usuario lo pidió y se le advirtió cuánto tarda.

`moonlit_master_art_builder` gana lo mínimo: `progress()` (fase + hechos/total; total 0 en artistas y fotos, cuyos recorridos son en streaming), `pass_done()` (apoyado en el `complete` que `run_pass()` ya devolvía — una pasada interrumpida nunca cuenta), `is_running()`, `set_foreground()` (sin la pausa de animación ni el paso de `HZ/20`, solo `yield()`) y `begin_full_pass()`, que además llama a `poll()`: el hilo lo crea `poll()` la primera vez que la base está lista, así que pedir la preparación sin haber entrado nunca a Música habría dejado `s_kick` puesto sin hilo que lo atendiera.

**Detalle que se corrigió al instrumentar**: el contador NO se incrementa dentro de `pace()`, que también corre desde `service_hints()` — un hint de Marea no es un elemento del recorrido y habría movido el contador de la pantalla sin motivo. Se cuenta en los tres sitios reales del recorrido.

**Bug colateral corregido: el aviso no cabía, y el diálogo se encimaba.** `draw_question()` está topada en dos líneas a propósito. Se agregó `metro_widgets_confirm_detail()`, igual que en Metro (M-100), pero aquí **no bastó con meter una línea**: la tipografía de moonlit es mucho más grande (Baskerville 28 px en la pregunta), así que *"¿actualizar biblioteca ahora?"* ya ocupa dos líneas por sí sola y termina justo donde empezaría el detalle — el texto se dibujaba encima de la segunda línea de la pregunta y encima de "sí". Con detalle, el bloque entero se reacomoda (pregunta en 62, detalle en 112, "sí"/"no" en 178/206) y el detalle admite **tres** líneas en vez de dos; sin detalle, las posiciones no cambian ni un píxel y ningún otro diálogo se mueve. El envoltorio del detalle también cortaba la **última** línea en su último espacio y perdía la palabra final ("…y cómo esté el" en vez de "…el disco."): si lo que queda entra entero, va entero.

**Verificado.**
- Build ARM (`firmware/build-ipod6g`): **0 errores**; sin warnings en los archivos tocados. Los `-Wmissing-field-initializers` de `metro_screen_hub.c` son preexistentes y no se tocaron.
- `make -C firmware/rockbox/apps/metro/test test`: **11 suites, 0 fallos**.
- Simulador (build limpio + `make install`), recorrido real con inyección de botones: la fila **"actualizar biblioteca"** aparece en Ajustes › general; el aviso muestra la pregunta y **la advertencia completa** en tres líneas, sin encimarse (captura); al aceptar corre la base y sigue con **"preparando carátulas 83/312"** bajo "actualizando biblioteca…" (captura), y la pantalla **se cierra sola** devolviendo a Ajustes. Con la caché maestra de álbumes vaciada antes de la corrida, se reconstruyeron las **312** maestras.
- A diferencia de Aura (D-344) y Metro (M-100), aquí la captura del progreso **no** necesitó ralentizar nada: la biblioteca del simulador de moonlit tiene 312 álbumes y la fase dura lo suficiente para observarla con el volcado automático.
- **Límite documentado**: no se probó en hardware, ni el disparo por marcador de un sync de Studio real (exige Mac + iPod); los dos entran por el mismo `finish_ok()`, que es el punto modificado.

---

# v0.2.0 — ronda "pulido" (D-062…)

## D-062 — Pila del hilo principal de 8 a 12 KB, y una herramienta que lo vigile

**Encargo** (`PLAN-ronda-3-firmwares-maestro.md` §E, plan hijo Fase 1).
El panic que motiva la ronda es de Aura (`Stkov main`, `sp` 528 B por
debajo del inicio de una pila de 8 KB), pero **los tres firmwares
comparten `app.lds`**: el mismo desbordamiento es alcanzable aquí en
cuanto un camino de UI se estire lo suficiente. La corrección de raíz se
aplica a las tres familias.

**Lo que se cambió.** `firmware/target/arm/s5l8702/app.lds`, sección
`.stack`: `. += 0x2000` → `. += 0x3000`. Archivo de Rockbox fuera de
`apps/metro/` → anotado en `MODIFICATIONS.md` en la misma pasada, con
comentario inline `moonlit (D-062)`.

**Que cabe no es una estimación, es el `.map`.** IRAM de core = 48 KB
(`IRAMSIZE`, `0xC000`). Antes: `stackbegin` 0x07d30, `stackend` 0x09d30,
`_fiqstackend` **0x0a530**. Después: `stackend` 0x0ad30,
`_fiqstackend` **0x0b530** — 2 768 B libres hasta 0xC000. Enlaza sin
error (`build_target.sh`, exit 0, firmware y bootloader).

**La medida que justifica los 4 KB.** `firmware/tools/stack_report.py`
(abajo) estima el peor camino estático desde `main` en **7 688 B**. Con
la pila vieja de 8 192 B eso es el **93.8 %** — sin margen para el
inlining de un compilador distinto ni para una rama que la herramienta
no puede seguir (llamadas por puntero). Con 12 288 B baja al **62.6 %**,
por debajo del tope del 75 % que fija §E.3. El camino que manda no es
de `apps/metro/` sino de Rockbox base, y termina donde ya lo había
localizado AF D-343: `skin_get_gwps → skin_load → skin_data_load →
font_load_ex → glyph_cache_load` (2 088 B él solo) y de ahí a la pila de
FAT/ATA.

**`firmware/tools/stack_report.py` — portada, no reescrita.** Es la
herramienta de `../Aura-Firmware/firmware/tools/` (AF D-345), el repo
canónico según §E.3, con tres adaptaciones y ninguna más: el grupo
propio pasa de `apps/aura/` a `apps/metro/`, la raíz propia de
`aura_main` a `metro_main`, y el toolchain sale de `RBDEV_TOOLCHAIN`
(M-002). Mide el **desensamblado del binario que se publica**
(`objdump -d`, prólogos `push`/`sub sp`) en vez de recompilar con
`-fstack-usage` — desviación ya documentada y justificada en la propia
herramienta por Aura; se conserva `--su-dir` como contraste opcional.
Corre en `package_dist.sh` **antes** de empaquetar, con `--quiet`, y
detiene el empaquetado si falla (`set -e`).

Dos diferencias deliberadas respecto a la copia de Aura, ambas porque
copiarlas tal cual habría dado un número FALSO en este repo:

1. **`GUARDED_EDGES` se vacía.** Aura corta la arista
   `skin_get_gwps → skin_load` apoyándose en que AF D-345 vació
   `settings_apply_skins()`. moonlit **no** tiene ese cambio: que
   `apps/metro/` tenga prohibido el skin engine (CLAUDE.md) no borra la
   arista de `apps/settings.c` de Rockbox base. Contarla es lo correcto
   — y de hecho es el camino que domina el reporte.
2. **`BIG_FRAMES`, lista declarada de marcos grandes** (mecanismo nuevo,
   propuesto también para la copia canónica de Aura). La regla de §E.3
   "ninguna función de `apps/<familia>/` sobre 1 024 B" es inalcanzable
   como regla ciega aquí: ocho funciones la superan por dos idiomas de
   Rockbox, no por descuido — `struct tagcache_search` en la pila
   (~1.2 KB; volverla estática rompería el anidamiento entre el hilo de
   UI y el hilo constructor de D-059, que abren búsquedas a la vez) y
   varios `char path[MAX_PATH]` (260 B c/u). En vez de subir el tope
   (que dejaría pasar crecimiento nuevo) o de fallar siempre (que
   bloquearía el empaquetado), cada excepción se declara **con su
   motivo**, el reporte las imprime una por una, y una entrada obsoleta
   se señala para que se quite. Cualquier función **nueva** por encima
   del tope sigue fallando, que es para lo que sirve la regla.
   Declaradas: `run_pass` (corre en el hilo constructor, con su propia
   pila `DEFAULT_STACK_SIZE + 0x2000` — no en la de 12 KB que este
   reporte mide), `run_search`, `insert_matching_tracks`,
   `metro_music_song_count_of_album`, `metro_music_recent_albums`,
   `moonlit_art_load_for_album`, `import_ratings`, `write_marker`.
   Ninguna está en el peor camino reportado.

**§E.4 — marca de agua de la pila en "Acerca de".** Es la única forma
de que el dueño confirme en hardware que 12 KB alcanzan sin esperar otro
panic. Rockbox ya la calcula para su menú de depuración
(`thread_get_debug_info()` → `stack_usage`, porcentaje: cuenta las
palabras que ya no valen `DEADBEEF`). La fila está **oculta**: SELECT
sostenido sobre la fila de versión la revela/oculta
(`MACT_SELECT_HOLD`, acción nueva en `MCTX_LIST`). El par
`{SELECT|REL, prereq SELECT}` + `{SELECT|REPEAT, prereq NONE}` es el
mismo que ya usan `MACT_OPTIONS`/`MACT_TOGGLE_SHUFFLE` en `MCTX_PLAYER`:
cuando dispara el REPEAT, el REL posterior ya no cumple su
prerrequisito y `MACT_SELECT` no se dispara detrás. Antes de este
cambio, SELECT sostenido en una lista no producía **ninguna** acción,
así que no le quita el gesto a nadie. En el simulador la fila arranca
visible (`#ifdef SIMULATOR`) y muestra "pila principal n/d": ahí los
hilos son de SDL y el campo no existe (`HAVE_SDL_THREADS`) — se dibuja
igual porque lo que el simulador verifica es la geometría, y un número
inventado sería peor que decir que no se sabe.

**Warning latente corregido de paso.** `metro_screen_lock.c:340` usaba
`strlcpy()` sin declararla (`-Wimplicit-function-declaration`). No era
visible porque el archivo llevaba tiempo sin recompilarse; el cambio de
`metro_settings.h` de D-063 lo forzó. Se agrega `#include
"string-extra.h"`.

**Verificación.**
- `RBDEV_TOOLCHAIN=… build_target.sh`: exit 0 (firmware + bootloader),
  **cero warnings nuevos** (solo los preexistentes
  `-Wmissing-field-initializers` de `metro_screen_hub.c`/
  `metro_screen_settings.c` y `-Wformat-truncation`).
- `stack_report.py --quiet`: `OK -- peor camino 7688 B (62.6 % de
  12288 B), ninguna funcion de apps/metro/ sobre 1024 B fuera de las 8
  declaradas.` Exit 0.
- `.bss` (`arm-elf-eabi-size rockbox.elf`): **8 460 732**. Los +4 096 B
  frente a D-061 (8 456 636) **son la pila misma**: `.stack` es
  `NOLOAD` en IRAM y `size` la cuenta dentro de `bss`. No hay ni una
  estática nueva por este cambio. Bajo el techo D-043 de 8 574 076, con
  **113 344 B de margen**.
- Sin probar en hardware: la marca de agua real (el simulador no la
  tiene) y que 12 KB basten en el iPod del dueño. Va a la lista de
  verificación de la ronda.

### D-062, addendum — el camino de skins se cierra en la raíz

Cerrada ya la decisión, la sesión de Aura-Firmware (vía la supervisora)
avisó de dos cosas que cambian el número de arriba. Se verificaron
ambas antes de actuar:

**1. La premisa del maestro sobre los skins era falsa, y el camino se
cierra en la raíz.** Dejar `wps_file`/`sbs_file` vacíos no sirve:
`skin_load()` cae al skin por defecto **compilado**. Aura lo cerró
haciendo que `settings_apply_skins()` no cargue skins
(`apps/gui/skin_engine/skin_engine.c`, AF D-345). Aquí aplica igual y
por una razón más fuerte: `apps/metro/` tiene **prohibido** el motor de
skins (CLAUDE.md), moonlit dibuja su propia barra de estado
(`metro_draw_header()`) y su propio "Ahora suena", y ningún tema de
moonlit es un `.wps`/`.sbs` — el motor solo estaba cargando skins que
nadie iba a mirar.

Se portó el mismo cambio, del mismo archivo, leído de
`../Aura-Firmware` (commit `7705b4a3`, solo lectura): se retira el bucle
de carga de arranque y `skins_initialised` se queda en `false`, con lo
que `skin_get_gwps()` sale de inmediato para `CUSTOM_STATUSBAR` — la
única pantalla skinneable a la que moonlit puede llegar. Todo lo demás
de la función (backdrops, `THEME_STATUSBAR`, `skin_backdrop_show()`)
queda intacto. Anotado en `MODIFICATIONS.md` con comentario inline
`moonlit (D-062)`.

**El criterio que se aplicó, y por qué el número no bajó solo.** El
umbral que dio la supervisora era "si el camino de skins supera 6 KB,
pórtalo": medido, `skin_get_gwps` cuesta **5 136 B**, por debajo. Se
portó igual —instrucción explícita de la supervisora y, sobre todo,
porque ese subárbol es **la cola del peor camino completo**— y el
resultado justifica la decisión mucho mejor que el umbral:

| | Antes | Después |
|---|---|---|
| Peor camino desde `main` | 7 688 B (62.6 %) | **5 520 B (44.9 %)** |
| Peor camino desde `gui_usb_screen_run()` | 7 056 B | **4 376 B** |

Ese segundo número es el que importa: es el camino de ~9.5 KB que
AF D-343 dejó anotado sin corregir, medido en este binario.

Recompilar solo, sin más, movió el total apenas 8 B (7 688 → 7 680): la
arista `skin_get_gwps → skin_load` **sigue existiendo en el
desensamblado**, porque la guarda es una variable de runtime
(`skins_initialised`), no una constante de compilación. Por eso vuelve a
`GUARDED_EDGES` —vaciada en la primera versión de esta decisión
justamente porque entonces la guarda **no** era cierta aquí— ahora con
su motivo propio de moonlit. El peor camino pasa a ser de código
propio: `metro_main → moonlit_screen_marea_tick →
moonlit_art_load_for_album (1 184) → metro_music_songs_of_album →
run_search (1 592) → tagcache → FAT/ATA`.

**Lo que esto NO cambia: la pila se queda en 12 KB.** Con 5 520 B, los
8 192 B viejos darían 67.4 %, bajo el tope del 75 %. Aun así los 4 KB se
quedan, por tres razones que el número no ve: la guarda es de runtime y
un cambio aguas arriba puede reactivar la arista; la herramienta **no
sigue llamadas por puntero** (275 funciones del binario tienen alguna) y
su estimación es por lo tanto un piso, no un techo; y el panic del dueño
existió de verdad. El margen es barato: 2 768 B de IRAM que nadie más
estaba usando.

**Verificación funcional en simulador** (lo que el maestro pide
comprobar al apagar los skins):
- Rejilla de fotos → visor → MENU de vuelta: la rejilla vuelve con su
  barra de estado propia dibujada
  (`f1-skins-02-fotos.png`, `f1-skins-03-visor.png`,
  `f1-skins-04-visor-vuelta.png`).
- `mpegplayer.rock`: **se carga de verdad** (el log del simulador
  muestra sus `mpeg_alloc_internal: … MPEG_ALLOC_MPEG2_BUFFER/PCMOUT/
  DISKBUF/YUV`) y devuelve el control a la lista de videos con la barra
  intacta, sin panic ni assert (`f1-skins-05-video.png`).
- **Pendiente de hardware**: la pantalla USB. Es propia
  (`metro_screen_usb.c`), pero se dibuja **desde** `usb_screen.c` de
  Rockbox después de `font_disable_all()`, y el arnés headless no puede
  inyectar una conexión USB (solo pulsaciones de rueda/botón). Va a la
  lista de verificación de la ronda; es justo el camino que más baja
  (7 056 → 4 376 B).

**2. La carrera del `uniqbuf` de tagcache NO existe en moonlit** (en
Aura sí). Verificado leyendo los dos hilos, no supuesto:
`s_uniqbuf` (`metro_music.c:56`) lo usan solo `run_search()`,
`metro_music_recent_albums()` e `insert_matching_tracks()`, y las tres
corren únicamente en el hilo de UI. El hilo constructor de maestras
llama exactamente a dos cosas de este módulo —
`metro_music_album_art_source()` y `metro_music_db_ready()`— y ninguna
pide `tagcache_search_set_uniqbuf()`: `enumerate_albums()`
(`moonlit_master_art_builder.c:274`) abre su propia
`struct tagcache_search` en su pila, sin uniqbuf, y escribe en el
`s_seeks[]` privado del constructor. Es la separación que D-059 ya había
razonado al elegir la variante **sin memo** para el constructor. No hace
falta decisión nueva.

Nota relacionada, del mismo tipo y por construcción: la lectura de
directorio que D-063 agrega dentro de `metro_music_album_art_source()`
—que sí corre en los dos hilos— usa `metro_fsutil_mtime_in_dir()`, que
trabaja **solo con locales**. Deliberadamente no se reutilizó
`metro_fsutil_list_by_ext_mtime()`, que sí tiene un `s_scan[]` estático
y habría introducido exactamente la carrera que este punto buscaba.

### D-062, addendum 2 — el build LIMPIO, y un pie en el que este repo no cayó

Aviso de la sesión de Metro (R7-3, vía la supervisora): allí un build
**limpio** del target llevaba roto desde el 19-ago sin que nadie lo
notara, porque `mkdepfile` (`tools/functions.make:57`) arma su línea con
`PPCFLAGS + OTHER_INC` y **no** con `MPEGCFLAGS`, así que el
`-I$(APPSDIR)/metro` de M-059 no le llegaba; `-MG` convertía el header
de paleta en un fantasma, y `build-ipod6g/` arrastraba un `make.dep`
anterior que lo tapaba.

**Verificado: aquí no pasa, y por una razón concreta.**
`firmware/build-ipod6g` y `-boot` se borraron enteros y se reconstruyó
desde cero: **exit 0, cero errores**. moonlit se salva porque
`mpegplayer.c:120` incluye `"../../metro/moonlit_tokens.h"` con **ruta
relativa**, que el preprocesador resuelve contra el directorio del
propio archivo que la incluye — no depende de que ningún `-I` llegue a
`mkdepfile`. El `MPEGCFLAGS += -I$(APPSDIR)/metro` de M-059 sigue ahí
(heredado del fork) pero no es lo que sostiene la compilación.

Build limpio vs. incremental: `text`/`data`/`bss` **byte a byte
idénticos** (1 243 156 / 12 308 / 8 460 732); `rockbox.bin` difiere en
**22 bytes**, que son la cadena `RBVERSION` (el hash de git del commit
de cada corrida). Nada más.

**Lo que sí estaba roto, y es de este repo.** El build limpio falló en
el primer intento con `arm-elf-eabi-gcc: No such file or directory`.
Causa: `build_target.sh` hace `cd` al directorio de build **antes** de
usar `PATH="$TC_BIN:$PATH"`, así que un `RBDEV_TOOLCHAIN` **relativo**
se resolvía contra el directorio equivocado. Y relativo es exactamente
como lo escriben los planes de esta ronda
(`RBDEV_TOOLCHAIN=../Metro-Aura/firmware/toolchain/bin`). No se notaba
nunca en un build incremental, porque el `Makefile` ya generado lleva
las rutas absolutas del `configure` anterior: **solo rompía el build
limpio**, que es justo del que tiene que salir el paquete. Corregido en
`build_target.sh` y en `package_dist.sh` (que hace `cd` por el mismo
motivo, para `make zip` y `mks5lboot`): `TC_BIN` se vuelve absoluto
apenas se resuelve. Confirmado reconstruyendo desde cero **con la ruta
relativa**: exit 0.

## D-063 — Caché de carátulas con versión de formato y clave de álbum v18

**Encargo** (maestro §A.2/§A.3, plan hijo Fase 1.2). Dos agujeros que
las claves actuales no pueden tapar solas:

1. Un tile mal derivado por una versión anterior del código **sobrevive
   para siempre** aunque el código ya esté corregido: su clave no
   cambió, así que nadie lo vuelve a generar.
2. Una `cover.jpg` reescrita sin tocar ningún archivo de audio **no
   cambia la clave** (`a-<crc32 ruta pista>.<tag_mtime>`), así que la
   maestra vieja —o peor, su marcador `.none`— queda pegada. Es la
   hipótesis (a) que D-055/D-056 dejaron abierta.

**Decisión 1 — `/.aura/art/format.txt` (contrato v18).** Un entero
decimal, hoy `2`. Al arrancar, `metro_settings_purge_stale_art_caches()`
lo lee; si falta o es menor que `MOONLIT_MASTER_ART_FORMAT_VERSION`,
borra las cachés **derivadas** y escribe la versión nueva. El
constructor de fondo (D-059) las rehace. Studio nunca lo toca.

Se borra **por sufijo conocido** (`.art`, `.none`, `.mth`, `.pfraw`) en
`/.aura/art/{albums,artists,photos}`, `/.aura/thumbs/{…}` y
`moonlitcache/{albums,artists,photos,art}`, reutilizando
`moonlit_art_sweep()` (D-056, host-testable) con un `keep` que no
conserva nada — **no** un borrado recursivo del árbol: `/.aura/art` y
`/.aura/thumbs` son directorios COMPARTIDOS con Aura y Metro, y un
barrido ciego podría llevarse algo que escribió otra familia. El
`.pfraw` entra en la lista aunque D-059 lo retiró: un disco actualizado
desde v0.1.4 todavía lo tiene.

Dónde vive cada mitad, según la regla de rutas de CLAUDE.md: el
**formato del archivo** (`_format_read()`/`_format_write()`) en
`moonlit_master_art.c`, que es el módulo del formato en disco y es
host-testable; la **ruta** y la orquestación de la purga en
`metro_settings.c`, el único archivo que puede escribir `/.aura/…`.

Se llama desde `metro_main()` justo después de
`metro_settings_migrate_shared_thumbs()` y **antes** de que exista el
hilo constructor (lo crea `moonlit_master_art_builder_poll()`, más
abajo en la vuelta ociosa). Sin pantalla: lo que recorre son tres
directorios planos borrando por sufijo, no la biblioteca.

**Decisión 2 — clave de álbum v18.** El `<mtime>` de
`a-<crc32>.<mtime>` pasa a ser `max(mtime de la pista representativa,
mtime de la `cover.jpg` hermana si existe)`. Cuesta **una lectura de
directorio por álbum** en la resolución de clave: Rockbox no expone un
`stat()` de un archivo suelto, así que `metro_fsutil_mtime_in_dir()`
hace `opendir()` + `readdir()` hasta la primera coincidencia
(comparación sin distinguir mayúsculas, como el FAT) y sale. La UI la
paga una sola vez por álbum gracias al memo de D-055; el constructor,
una vez por álbum y por pasada.

La parte pura —de qué carpeta hermana hablamos— es
`metro_fsutil_parent_dir()`, `static inline` en el header por el mismo
motivo que `metro_fsutil_is_hidden_name()`: así la cubre el arnés de
host sin arrastrar dependencias de Rockbox.

**Contrato.** `CONTRATO-moonlit-studio.md` pasa a **v5**, referenciando
el canónico v18: §A.11 gana el párrafo de versión de formato y purga
más el de la clave; §A.10 hereda la clave nueva.

**Verificación (simulador, `make install` hecho por `build_sim.sh`).**
- Estado previo: 21 `.art` + 307 `.none`, **sin** `format.txt`.
- Primer arranque: `format.txt` = `2` y el árbol purgado — a los 400
  ticks va en 5 `.art` + 43 `.none`, reconstruyendo.
  (`docs/screenshots/ronda-pulido/f1-purga-01-primera.png`)
- Segundo arranque: `format.txt` sigue en `2` y **no** se purga nada.
- Pasada completa (3 000 ticks): **21 `.art` + 307 `.none`** — el mismo
  estado exacto que D-059 documentó. La purga no pierde nada, solo lo
  rehace.
- Clave v18, evidencia directa en la fixture: en
  `Music/Wheel & Click/Analog Dreams/`, las pistas tienen mtime
  1787718315 y `cover.jpg` 1787718316; la maestra escrita es
  `a-031b464b.**1787718316**.art` — el mtime de la carátula, no el de la
  pista. Con v17 habría sido `.1787718315`.
- Invalidación, la prueba que cierra la hipótesis (a): `touch` de esa
  `cover.jpg` a 2027-01-01 **sin tocar ninguna pista** → la pasada
  siguiente escribe `a-031b464b.**1798826400**.art`. Antes de D-063 la
  clave no se habría movido y la maestra vieja habría quedado servida
  para siempre. (La vieja convive hasta que el `run_gc()` del final de
  la pasada la barre como huérfana, que es el comportamiento de D-059.)
- Tests host: **16 suites, 0 fallos**; `test_fsutil` 20→**33 checks**
  (`metro_fsutil_parent_dir()`: raíz, sin barra, buffer justo, NULL,
  ruta terminada en barra) y `test_master_art` 71→**85 checks**
  (`format.txt` ausente / vacío / con basura / con cola = versión 0 o el
  número, que es donde se decide si hay purga).
- `.bss`: sin cambio atribuible (ver D-062: el delta es la pila).

**Límite conocido.** La purga de moonlit no toca las cachés privadas de
Aura (`.rockbox/aura/cfcache`, `photocache`) — no son suyas y este repo
no escribe fuera de lo suyo; cada familia purga las propias al ver el
mismo `format.txt`, que es como el contrato lo define.

## D-064 — "Acerca de": el hero se desplaza, y las filas dejan de perderse

**El defecto, confirmado antes de tocar nada.** `metro_screen_list.c`
navegaba "Acerca de" con `METRO_DRAW_ROWS_VISIBLE` (5), pero
`draw_about_rows()` empezaba en y=160 con paso 28, así que en 240 px de
alto solo cabían **dos** filas. En los desplazamientos relativos 3 y 4
la selección se salía de la pantalla, y había filas —los créditos, las
licencias OFL/Apache, el aviso de no afiliación con Apple, que la GPL
§3 obliga a poder leer— que **nunca** se veían. El hero fijo de 64 px
(creciente + wordmark, y=76..140) era lo que se comía el espacio.

**Decisión.** El hero pasa a ser **compacto y desplazable**: creciente
de 40 px + wordmark en una sola línea de 40 px, y es la "fila −1" de la
lista — solo se dibuja mientras la ventana está arriba del todo
(`first == 0`), y en cuanto el usuario baja desaparece y las filas usan
toda la pantalla. Al volver al tope, vuelve.

El número de filas visibles se **deriva de la geometría real**
(`about_visible_rows()`: `(LCD_HEIGHT − primera_y) / paso`), nunca de la
constante global, y **el mismo número** lo usan el bucle de dibujo y
`metro_nav_move_sel()` — que era exactamente lo que no pasaba. Con hero:
4 filas (124/152/180/208). Sin hero: 5 (80/108/136/164/192) más una
asomando en 220. Una fila "asomando" solo se dibuja si se le ven al
menos 12 px de texto; un sliver de 4 px es ruido, no una pista de que
hay más lista.

**Enmienda a D-016.** D-016 decía "el wordmark solo se dibuja junto al
creciente de 64 px (arranque, Acerca de)". Este hero es la **segunda**
pareja admitida: creciente de 40 px + wordmark de 140×28 centrado
verticalmente contra él. El de 64 px sigue siendo el del arranque. Los
tamaños 16/24 siguen siendo solo geometría del creciente, sin wordmark.

**Fila de versión, nueva.** El maestro §B.2 da por sentado que "el
firmware muestra su propia versión en Acerca de" — y no estaba. Se
agrega como fila fija (índice 1, bajo el nombre del aparato) con
`rbversion`, la cadena que ya genera el build (`genversion.sh`) y que
`package_dist.sh` fija al tag en un release; sin `__DATE__`/`__TIME__`
de por medio (D-048). Es además el ancla de la fila oculta de
diagnóstico de D-062.

**Desviación respecto al plan hijo, deliberada.** El plan pedía el hero
compacto "en y=28..68, primera fila en y=80". Los 28..68 son la banda de
los **pivotes** (`METRO_PIVOT_Y` 28, `MFONT_DISPLAY` 40 px): ahí es donde
se dibujan "general · pantalla · acerca de", que es como se navega entre
ellos. Poner el hero encima habría borrado esa navegación. El hero vive
entonces en el área de contenido (y=80 cuando está arriba del todo) y
**las posiciones de fila del plan se cumplen exactamente** —
80/108/136/164/192 más una asomando— en cuanto el hero se va, que es el
estado en el que el plan las pedía.

**Verificación (simulador, capturas en `docs/screenshots/ronda-pulido/`).**
- `f1-about-00-hero.png` — entrada: hero compacto + 4 filas
  ("mi ipod" seleccionada, "versión 7eeafd24feM-260904", "pila principal
  n/d", "sin sincronizar todavía").
- `f1-about-04.png` — tras 4 pasos: el hero se fue, 5 filas completas +
  una asomando, selección dentro de pantalla.
- `f1-about-08/12/16/20.png` — el recorrido entero. `f1-about-20.png`
  muestra la **última** fila ("moonlit no está afiliado a apple")
  seleccionada y completa: antes de D-064 era inalcanzable.
- `f1-about-back-to-top.png` — 6 pasos abajo y 6 arriba: el hero vuelve.
- Sin probar: el gesto de SELECT sostenido (el arnés headless de
  `sim_shot.sh` solo inyecta pulsaciones cortas). Va a la lista de
  hardware.

## D-065 — Marea deja de tener fases visibles: un solo camino de dibujo

**El defecto** (§2 del informe de la ronda, plan hijo Fase 2). Marea
tenía **tres** caminos de dibujo, y un álbum sin carátula los recorría
todos al desplazarse:

1. `draw_slide_flat()` — un relleno liso proyectado, para una tapa sin
   arte que no cayera exactamente al centro. Existía por una razón
   concreta y honesta: no reservar un segundo buffer de 120×120 solo
   para un color plano.
2. `draw_monogram()` — exactamente en el centro (`offset256 == 0`), una
   tarjeta plana con la inicial en `primary`, dibujada directo, sin
   pasar por el motor de flujo (en offset 0 no hay perspectiva que
   aplicar).
3. `draw_slide_perspective()` — la carátula real, proyectada.

Girando la rueda sobre una biblioteca sin carátulas, cada álbum
**cambiaba de forma tres veces**: barra lisa al acercarse, tarjeta con
letra al pasar por el centro, barra lisa al alejarse. Y cuando la
carátula real llegaba, cambiaba una cuarta.

**Decisión: el monograma se RASTERIZA dentro de `slot->cover`.** Un
slot nace dibujable. `claim_slot()` llama a `rasterize_monogram()`, que
produce exactamente el mismo formato que una carátula real —120×120
fila-contigua, relleno `primary_container`, inicial en
`on_primary_container` (`MFONT_HEADLINE`), esquinas horneadas contra
`surface` por `moonlit_art_mask_corners()`, la misma que se aplica a
una maestra derivada (D-020/D-059)—. `MAREA_ART_PENDING` deja de
significar "no dibujable" y pasa a significar solo "todavía no se
intentó cargar la de verdad". `draw_slide()` queda en una línea:
`draw_slide_perspective(slot->cover, offset256)`. `draw_slide_flat()` y
`draw_monogram()` se retiran.

Es el criterio de `aura_albumart_load_default()` que el plan cita: **el
consumidor no tiene por qué distinguir de dónde salió el píxel**.

**Cómo se dibuja fuera de pantalla.** `metro_fb_render_tile()`
(`metro_fb.c`, no duplicado en Marea): arma un `struct frame_buffer_t`
con `stride = size` apuntando al buffer del slot, inicializa un
viewport con `viewport_set_defaults()` **antes** de asignarle ese buffer
(M-027) y lo acota a `size × size`. Así el relleno y el texto pasan por
las mismas primitivas que todo lo demás —`lcd_fillrect()` y
`metro_draw_text()` (M-051), colores por rol— y cualquier dibujo que se
saliera de la tapa lo recorta el LCD, en vez de escribir fuera del
buffer del slot. El puntero y el stride son estáticos de módulo por la
misma razón que `s_render_target` de `metro_fb_render()`:
`get_address_fn` recibe solo `(x, y)`, sin contexto propio; igual de
seguro, porque los llamadores son síncronos y no reentrantes.

**Calentamiento síncrono al entrar.** `moonlit_screen_marea_push()`
intenta hasta 7 slots (destino ±3) **antes del primer cuadro**, leyendo
solo la maestra ya escrita. Deliberadamente **no** usa
`moonlit_art_load_for_album()`, que además decodificaría el JPEG si el
constructor está ocioso: un decode ahí costaría cientos de ms con el
usuario mirando una pantalla vacía. El álbum sin maestra se queda con
su monograma —que ya es dibujable— y entra por el tick de siempre. Esto
sí puede tocar tagcache (`moonlit_art_master_path()`, no la variante
`_peek()`) y es correcto: `push()` no es un bucle de animación, es el
momento anterior al primero; la regla dura de D-030/D-053 acota los
bucles, no la entrada.

**LRU: una tapa cargada dentro de ±15 no se desaloja.**
`MAREA_KEEP_LOADED_RADIUS` = 15, sobre la protección de D-057 (lo
visible en el destino). Volver a leer una tapa cuesta 34 KB, y es justo
la que el usuario va a volver a ver si sigue girando en esa dirección.
Los slots **sin cargar** de esa franja siguen siendo desalojables: no
hay nada que perder en ellos. Cabe con margen — como mucho 31 de 37
slots protegidos, y solo si están todos cargados.

**Medición (simulador, `#ifdef SIMULATOR`).** `current_tick` tiene
10 ms de grano (HZ = 100), inservible frente a un presupuesto de 33 ms,
así que en el simulador el cronómetro es el del host en microsegundos
(`gettimeofday`); en el target no se agrega ni un ciclo. 60 cuadros de
scroll continuo, mismo recorrido (`SELECT,SELECT` + 20 `SCROLL_FWD`),
mismo simdisk:

| | Antes (D-057) | Después (D-065) | Después, caché vacía |
|---|---|---|---|
| media | 1.30 ms | 1.25 ms | 1.55 ms |
| p50 | 0.46 ms | 0.43 ms | 0.53 ms |
| p90 | 8.19 ms | 7.19 ms | 7.52 ms |
| máx | 9.03 ms | **9.35 ms** | 8.89 ms |

Calentamiento: **1.43 ms** con la caché maestra completa (3 tapas
leídas + 4 resueltas por `.none`), **0.91 ms** con la caché recién
purgada. Muy por debajo de los 150 ms que el plan puso de tope — en el
simulador.

**Lo que esta medición NO dice, y hay que decirlo.** El simulador corre
sobre el sistema de archivos del host y una CPU tres órdenes de
magnitud más rápida que un S5L8702 a 54 MHz: el piso de cada cuadro lo
pone el relleno de la banda + `lcd_update_rect` (33 440 px), y contra
ese piso la diferencia entre proyectar textura y pintar un relleno liso
se pierde en el ruido. Analíticamente el cambio **sí** tiene costo: una
tapa sin carátula pasaba de ~120 `lcd_hline()` a la misma cadena de
muestreo que una carátula real (~14 400 px). Lo que no cambia es el
**peor caso**: una pantalla llena de carátulas reales cuesta hoy
exactamente lo que costaba antes. Lo que sube es el caso mejor, hasta
igualarlo. El criterio de los 33 ms sigue siendo de hardware, y sigue
pendiente — que es literalmente lo que D-014/D-043 ya dicen de Marea
("experimental hasta la medición en hardware real de M12").

**Verificado además por lectura, no por suposición**: el constructor de
fondo sigue pausado durante el scroll — `metro_main.c` espeja
`moonlit_screen_marea_animating()` en
`moonlit_master_art_builder_pause()` y este commit no toca ninguna de
las dos (D-057/D-059).

**Un susto que resultó no serlo.** Una captura mostró una banda gris
clara de 6 px sobre la tapa más lejana, que no aparecía antes. Se
sospechó de un desbordamiento de fila al proyectar (ahora TODAS las
tapas pasan por el muestreo, también las de los extremos, donde antes
solo iba el relleno liso). Se descartó por dos vías: `moonlit_flow_source_row()`
acota la fila a `[0, slide_width_px-1]`, y un volcado temporal de los
píxeles del propio buffer confirmó relleno correcto en las filas 0, 1,
60 y 119 de cada monograma. La banda es una carátula de la fixture que
es literalmente gris claro, proyectada en la posición más lejana. Se
deja anotado porque el volcado es la clase de verificación que conviene
repetir si aparece algo parecido.

**Verificación.**
- `build_target.sh` (firmware + bootloader): exit 0, **0 warnings
  nuevos**. `build_sim.sh`: exit 0. Tests host: 16 suites, 0 fallos.
- `.bss`: **8 460 764** (+32 B: `s_monogram_initial[5]` y los dos
  estáticos de `metro_fb_render_tile()`; ningún buffer nuevo de tapa —
  el monograma se escribe **dentro** del slot que ya existía). Bajo el
  techo D-043 de 8 574 076, margen **113 312 B**.
- `stack_report.py`: OK, peor camino 5 512 B (44.9 %).
- Capturas: `f2-marea-01-entrada.png` (carátula real al centro),
  `f2-marea-02-monogramas.png`, `f2-marea-03-monograma-centro.png` —
  monograma al centro **y** monograma en perspectiva arriba, la prueba
  de que ya no hay dos formas distintas para lo mismo.

## D-066 — Glifos faltantes: el rango se queda, la puntuación se translitera

**El encargo** (§7 del informe, plan hijo Fase 3): los metadatos reales
traen comillas tipográficas, guiones largos y puntos suspensivos que las
fuentes de moonlit no tienen, y salen como carácter de reemplazo.

**Primero, medir qué falta.** `firmware/tools/check_fonts.py --coverage`
(modo nuevo) lee la **tabla de anchos** de cada `.fnt` —no solo el rango
de la cabecera: un código dentro de `[firstchar, firstchar+size)` puede
seguir sin glifo, que es lo que D-032 ya documentaba para U+017F— y la
compara con (a) todas las cadenas de `metro_lang.c`, (b) una lista curada
de codepoints frecuentes en metadatos de música, y (c) opcionalmente un
volcado de tags. Falla si falta algo de (a): la UI propia no puede tener
huecos.

Al escribir el lector de la tabla me equivoqué de desplazamiento
—`font.c:224-238` **alinea** a 16 o 32 bits entre el bitmap y la tabla de
offsets, según `bits_size`— y el reporte salió corrido un byte: decía que
el espacio medía 0 px en dos roles. Se detectó porque el final calculado
de la tabla no coincidía con el tamaño real del archivo. La herramienta
lleva ahora ese cruce como aserción: si el cálculo no coincide con
`font.c`, aborta en vez de mentir.

**Lo que encontró, y es un defecto real de producto:** `U+2026 '…'` falta
en **los siete roles** y `metro_lang.c` ya lo usa
(`LANG_LIST_TRUNCATED = "…y más: la lista está llena"`). Se dibujaba como
`?` desde siempre.

**La opción preferida del plan, rechazada CON NÚMEROS.** Ampliar el rango
de convttf a `-s 32 -l 8482`, medido generando los 7 roles:

| | 32–383 | 32–8482 | delta |
|---|---|---|---|
| Los 7 `.fnt` en disco | 375 219 B | 1 385 797 B | **+1 010 578 B** |
| Tablas offset+ancho en RAM (7 roles residentes) | 8 787 B | 295 785 B | **+286 998 B** |

El presupuesto que fijaba el plan era **≤ 40 KB de RAM**: se pasa por
7×. La causa es el formato: RB12 es un rango **denso**, una entrada de
offset + una de ancho por código, exista o no el glifo. Llegar a 8482
paga **8 071 entradas vacías por fuente** para ganar ~30 glifos útiles.

**La alternativa del plan tampoco servía, por un motivo que el plan no
podía prever.** "Un codepoint fuera del rango del rol Baskerville se
dibuja con el rol Montserrat más cercano" presupone que Montserrat sí lo
tiene — y no: los `.fnt` de Montserrat llevan **el mismo** rango 32–383.
Los TTF tienen los glifos; los `.fnt` generados, no. Ampliar solo los
cuatro roles Montserrat seguiría costando ~164 KB, cuatro veces el
presupuesto.

**Lo que sí se hizo.** `apps/metro/moonlit_translit.c`, módulo puro: 24
codepoints con equivalente ASCII honesto (espacio duro, los seis guiones
de U+2010–U+2015, comillas simples y dobles tipográficas, viñetas,
puntos suspensivos, primas, angulares simples, ™). Se aplica en
`metro_draw_text()`/`metro_draw_text_clipped()` — el único sitio que hace
falta, porque M-051 obliga a que **todo** el texto pase por ahí, así que
ni las pantallas ni `metro_lang.c` saben del asunto. El camino común no
copia nada: `moonlit_translit_needed()` es un recorrido de bytes que sale
en falso salvo que aparezca un 0xC2/0xE2.

Lo que **no** tiene equivalente honesto (corcheas, estrellas, corazones,
CJK, emoji) **no se inventa**: cae en el `defaultchar`, que pasa de `?`
(63) a **`·` (183, U+00B7)**. Un punto medio no parece un error de
lectura; un signo de interrogación sí. Traducir ♪ como "b" sería peor que
no traducirlo.

**Efecto secundario que hubo que atender:** medir una cadena con
`lcd_getstringsize()` ya no da el ancho con el que se dibuja (un `…` mide
un glifo y se dibuja como tres). Se agrega
`metro_draw_text_width()`, que translitera antes de medir, y es la que
usa la marquesina de D-067. Los llamadores existentes de
`lcd_getstringsize()` que miden cadenas de `metro_lang.c` (nombres de
pivot, subtítulos) no se tocaron: ninguna de esas cadenas lleva
puntuación tipográfica, y el checker lo verifica en cada corrida.

**Verificación de punta a punta, con una fixture nueva.**
`gen_test_media.sh` gana el álbum *"Don’t Stop — “Live”…"* de
*Journey’s Edge*, con apóstrofo curvo, comillas dobles curvas, raya,
puntos suspensivos, espacio duro y una corchea ♪; sin `cover.jpg` a
propósito, para que sirva también de monograma (D-065). En el simulador:
- Cuadrícula de álbumes: `Don't Stop - "Live"…` / `Journey's Edge`
  (`f3-lista-albumes.png`).
- Lista de canciones: la ceja transliterada, `Believin'`,
  `Any Way You Want It` (con el espacio duro ya normal) y
  **`Wheel · in the Sky`** — la corchea cayendo en el `·` nuevo
  (`f3-canciones.png`). Antes: `?` en los cinco sitios.
- "Ahora suena" (`f3-nowplaying.png`) y panel de Marea
  (`f3-marea-panel.png`), igual.
- `check_fonts.py --coverage`: *"la UI esta cubierta en todos los
  roles"*. De los 42 codepoints de metadatos, 24 se transliteran y 18
  quedan al `·` a propósito.
- Tests host: `test_translit` **94 checks**, incluidos UTF-8 truncado,
  buffer justo (nunca corta a mitad de secuencia) y la coherencia de la
  tabla.

### D-066, addendum — la fuente de puntuación aparte: medida, y por qué NO se implementa en dos roles

La supervisora pidió explorar la única variante que respeta el formato
RB12 y que el dueño pidió literalmente ("construirlos desde la fuente
correcta"): un `.fnt` **extra por rol** con el rango contiguo
**8208–8482** (U+2010–U+2122), dibujado por tramos solo cuando una
cadena trae un codepoint de ese bloque. Criterio escrito de antemano:
implementar si dos roles caben en ≤ 25 KB de RAM y ≤ 2 ranuras.

**Medido con `convttf` de verdad, los siete roles:**

| rol | archivo | `size` | tablas |
|---|---|---|---|
| display 40 | 13 242 B | 272 | 816 B |
| title 28 | 7 468 B | 272 | 816 B |
| headline 22 | 4 570 B | 272 | 816 B |
| list 20 | 7 527 B | 275 | 825 B |
| list_sel 20 | 7 857 B | 275 | 825 B |
| body 18 | 5 949 B | 275 | 825 B |
| label 18 | 6 131 B | 275 | 825 B |
| **7 roles** | **52 744 B** | | **5 748 B** |

Los dos roles del criterio (headline 22 + list 20): **12 097 B en disco,
1 641 B de tablas, 2 ranuras**. El criterio **se cumple con holgura** —
1.6 KB contra un tope de 25 KB.

**Y aun así no se implementa así, por dos hechos que el criterio no
tenía delante:**

1. **La inconsistencia no es hipotética, es de una sola pantalla.**
   "Ahora suena" dibuja el álbum en `MFONT_LIST` (20) y el título en
   `MFONT_TITLE` (28). Con los dos roles del criterio, el álbum saldría
   con comilla curva y el título del mismo disco con comilla recta,
   **uno debajo del otro**. Eso es peor que la transliteración uniforme:
   parece un error de codificación, no una decisión.
2. **La versión uniforme no cabe en las ranuras.** `MAXUSERFONTS` = 12
   (`firmware/export/font.h:51`) y moonlit ya carga 7. Los seis roles
   que muestran metadatos (todos menos `display`, que solo dibuja
   nombres de pivote) serían 13; los siete, 14. Para que sea uniforme
   hay que subir `MAXUSERFONTS` a 16 — un cambio en un header de Rockbox
   que cuesta **16 B** de `.bss` (`buflib_allocations[MAXFONTS]`, 4 B
   por ranura) y su entrada en `MODIFICATIONS.md`.

**Estado en el momento de esta medición: NO implementado.** La versión
que vale la pena es la uniforme (seis roles + `MAXUSERFONTS` a 16:
~44 KB de disco, ~4.9 KB de tablas, 16 B de `.bss`), y su costo real no
está en esos números sino en el **dibujo por tramos dentro de
`metro_draw_text()`**: partir la cadena en tramos por bloque de
codepoint, avanzar la x por el ancho medido de cada tramo, y que todo
eso siga cuadrando con el viewport de recorte, con la medición de la
marquesina (D-067) y con la transliteración como respaldo. Es el camino
de texto más caliente del firmware y un error ahí se ve en todas las
pantallas a la vez.

La transliteración ya cierra el **defecto reportado** (el dueño veía el
carácter de reemplazo). Lo que quedaba era estético: `'` en vez de `’`.
Se dejó como decisión propia para el final de la ronda, con los números
ya hechos y el criterio claro: **uniforme o nada** — la supervisora
condicionó implementarla a que todo lo demás cerrara en verde primero.
Cerró: es **D-074**, al final de esta misma ronda, con los números de
aquí confirmados casi exactos por la medición real.

## D-067 — Marquesina: el texto que no cabe se mueve, y solo ese

**Referencia** (maestro §G, `aura_patterns.c`/`aura_marquee.c` leídos de
`../Aura-Firmware`, solo lectura): si cabe, estático; si no, **2 000 ms
quieto, 5 000 ms desplazando de derecha a izquierda de forma lineal,
24 px de hueco entre copias, en bucle**, con **dos copias** del texto
para que no haya costura. Tokens nuevos en `tokens.json`
(`motion.marquee_static_ms`, `marquee_scroll_ms`, `marquee_loop_gap_px`)
→ `moonlit_tokens.h` por `generate.py --header`; ningún número a mano.

**Lineal a propósito.** Con easing se lee peor: el ojo sigue el texto, no
el movimiento.

**Dónde se aplica**, y dónde deliberadamente no:
- Fila **seleccionada** de una lista (`metro_draw.c`). Las demás se
  cortan como siempre — una lista entera moviéndose sería ilegible.
- Rótulo del tile seleccionado (único texto de una cuadrícula).
- Las **tres** líneas de "Ahora suena": ahí no hay selección que mover,
  todas tienen el foco por igual.
- Título del panel de Marea. El artista y el conteo **no**: dos textos
  moviéndose a la vez en un panel de 152 px compiten entre sí, y el
  título es el que de verdad se corta.
- Fila seleccionada de "Acerca de" (las filas más largas de moonlit: URL
  del repositorio, licencias).

**Puerta de energía.** `moonlit_marquee_draw()` decide en cada dibujo si
esa ranura está desplazando; `moonlit_marquee_wants_ticks()` es lo que
`metro_main.c` consulta para bajar la espera a `HZ/20` y repintar. Se
apaga sola en cuanto el texto cabe, el LCD se duerme o las animaciones
están apagadas — con `animations=off` se corta a la derecha como siempre,
sin un solo tick de más. La puerta la decide el dibujo, no una pantalla
declarando "yo animo".

**Estado por ranura, no por fila.** Siete ranuras
(`enum moonlit_marquee_slot`), una por SITIO de la UI. El ciclo se
reinicia solo cuando cambia el texto de esa ranura: mover la selección
empieza otra vez por el tramo quieto, que es lo que se espera —poder leer
la fila nueva antes de que empiece a desfilar—. Se vio en vivo en la
captura: al cambiar de pista, la línea de título reinició su ciclo
mientras la de álbum seguía a mitad del suyo.

**El reloj vive aparte.** `moonlit_marquee_cycle.c` es un módulo puro sin
una sola dependencia de Rockbox — mismo patrón que
`moonlit_marea_prefetch.c` respecto de Marea (D-057) — para que el arnés
de host lo enlace solo. Ahí está todo lo que se puede equivocar sin que
se note: que el tramo quieto sea de verdad quieto, que el barrido llegue
**exactamente** a `span_px` (si se queda corto o se pasa, el bucle tiene
costura) y que un tiempo grande no desborde. `test_marquee`: **591
checks**, incluido un barrido de tres ciclos comprobando que el
desplazamiento nunca sale de `[0, span]`.

**Verificación.** Capturas de "Ahora suena" a dos instantes distintos del
mismo recorrido (`f3-marquesina-np-1.png` tick 450,
`f3-marquesina-np-2.png` tick 600): la línea de álbum pasa de
`- "Live"…   Don't S` a `"…   Don't Stop - "L` — las dos copias con su
hueco de 24 px, avanzando. Build target y simulador en 0 errores y **0
warnings nuevos** (uno propio, `-Wtype-limits` por comparar un enum sin
signo contra 0, corregido antes de commitear). Tests host: **18 suites,
0 fallos**. `.bss` **8 461 404** (+640 B: las 7 ranuras de 48 bytes de
clave más el estado; bajo el techo D-043 de 8 574 076, margen
**112 672 B**). `stack_report.py`: OK, 5 512 B (44.9 %).

**Pendiente de hardware**: que 5 000 ms de barrido se lean cómodos en el
iPod con un título de 40 caracteres, y que el repintado a `HZ/20` de una
marquesina no se note en la batería. Van a la lista de la ronda.

## D-068 — La barra de estado se alinea por la caja de tinta, medida por el pipeline

**El defecto** (maestro §H). El texto de 18 px de la barra iba en `y=4`:
su caja de fuente ocupaba **4..22 dentro de una barra de 20 px** —tocaba
el borde inferior y lo desbordaba en 3 px— y su altura de mayúsculas
quedaba centrada en **y=14** en vez de en el eje de la barra, **y=10**.
La batería y los íconos estaban a medio píxel; el texto, a cuatro.

**La regla**: todo se centra en `alto/2` por su **caja de tinta**, no por
su caja nominal. Y las tres medidas que eso necesita las produce el
pipeline, no la mano:

1. **Texto — altura de mayúsculas.** `generate.py --header` emite
   `MOONLIT_FONT_<ROL>_CAP_TOP/_CAP_H` midiendo el glifo **`H` del propio
   `.fnt`**. Decodificar ese bitmap tiene tres trampas, las tres
   documentadas en el código: 4 bits por píxel fila-contigua, **nibble
   bajo primero**, y el valor **invertido** (0 = tinta opaca, 15 =
   fondo). Se descubrió volcando el glifo como arte ASCII: leído sin
   invertir sale una 'H' en negativo. `text_y = 10 − CAP_H/2 − CAP_TOP`.
2. **Íconos — caja de tinta de la máscara.** `generate.py --icons` emite
   `ink_top`/`ink_h` por ícono y talla dentro de `struct
   moonlit_icon_mask`. Hacía falta por ícono, no una constante: un
   Material Symbol de 16 px dibuja entre 8 y 15 px de tinta según el
   símbolo (`play_arrow` 4..11, `lock` 0..14), así que centrar la
   **celda** no alinea nada.
3. **Batería** — su cuerpo de 9 px ya **es** su caja de tinta.

**El umbral de tinta tiene que ser el mismo en los dos lados, y no lo
era.** La máscara contaba cualquier cobertura > 0; la captura,
luminancia > 60/255. Con eso el candado salía medio píxel descuadrado y
`check_tones.py --align` fallaba por 1.5 px — la herramienta tenía razón
y el generador estaba midiendo otra cosa. El generador pasa a usar el
mismo > 60/255.

Y hubo que corregir el criterio de la herramienta también: "tinta" no es
"píxel claro" sino **"píxel que se aparta del fondo de la barra"**. Un
umbral absoluto de luminancia mide brillo — con el esquema `dawn` (fondo
claro, tinta oscura) daría justo al revés, y con un ícono en secundario
sobre `surface_container_lowest` dejaba fuera filas que sí se ven. El
fondo se deduce de la propia captura (el color más repetido de la barra)
y cuenta como tinta lo que se aparta de él más de 24 de luminancia.

**`check_tones.py --align`** (modo nuevo): agrupa las columnas con tinta
de la barra en elementos, mide la caja de tinta de cada uno y **falla**
si sus centros difieren más de 1 px.

| captura | dispersión | |
|---|---|---|
| `f3-canciones.png` (barra vieja) | **3.0 px** | FALLA |
| `f4-barra-candado.png` (barra nueva) | **1.0 px** | OK |

Que falle sobre una captura anterior es lo que demuestra que la
herramienta mide algo. La captura vieja además enseña el otro síntoma:
la tinta del texto llegaba a la fila **19**, la última de la barra.

**Matiz que queda documentado en la propia herramienta**: el firmware
centra por altura de **mayúsculas** y la herramienta mide **tinta**. En
un título con acentos ("música") la tilde sube por encima de la
mayúscula, así que la tinta queda medio píxel más arriba. Es esperado —
la tolerancia de 1 px existe para eso, y no hay que "corregir" el código
para que dé 0.0.

**Ícono de candado en la barra** (§H, §D.2): a la izquierda del
transporte, en **toda** pantalla con barra, leyendo `button_hold()` en
cada dibujo — el interruptor Hold del 6G no genera eventos
(`pmu_holdswitch_locked()` es sondeo). En **secundario**, no en acento:
el acento está reservado a la pausa (M-073), y el candado informa, no
reclama. Hasta ahora moonlit no lo reflejaba en ningún lado: se podía
tener el aparato bloqueado sin una sola pista en pantalla.

## D-070 — El acento puro es del estado activo; el respaldo usa su contenedor

**El defecto** (maestro §F): `metro_draw_tile()` rellenaba el tile de
respaldo con `metro_color_accent()` y `metro_draw_tiles()` dibujaba el
marco de selección **con ese mismo color**. Seleccionar un álbum sin
carátula no se veía: el marco desaparecía dentro del relleno.

**Decisión**: el relleno de respaldo pasa a `primary_container` con la
inicial en `on_primary_container` (los mismos roles que D-065 hornea en
el monograma de Marea, así que un álbum sin carátula se ve igual en la
cuadrícula y en Marea); el marco de selección se queda con `primary`
puro, 3 px, **más un anillo interior de 1 px en `surface`**.

El anillo no es adorno: separa el marco de la imagen que hay debajo.
Se ve en `f4-tiles-con-arte.png`, que selecciona a propósito la fixture
de contraste más difícil que existe en el repo —la carátula casi blanca
de *Analog Dreams* (R2-F1/DD-2)—: sin el anillo, un borde claro sobre
una carátula casi blanca se pierde en el borde de la propia carátula.

**Verificación**: `f4-tiles-albumes.png` (tile de respaldo seleccionado,
marco visible sobre el relleno) y `f4-tiles-con-arte.png` (marco visible
sobre la carátula más pálida). Build target y simulador en 0 errores, 0
warnings nuevos; 18 suites en verde; `.bss` **8 461 404**, sin cambio
frente a D-067 (ninguna estática nueva: el ink box viaja en la tabla de
íconos, que es `.rodata`), bajo el techo D-043 con **112 672 B** de
margen.

## D-069 — El bloqueo deja de servir solo al arrancar: Hold, y cuándo volver a pedir el código

**El defecto** (maestro §D). El bloqueo existía desde M-068, pero solo se
pedía **al arrancar**: una vez desbloqueado, el aparato quedaba abierto
hasta el siguiente encendido. Es decir, no servía para lo único que la
gente hace con un bloqueo — guardarse el aparato en el bolsillo. Y el
interruptor **Hold**, que es el gesto que el dueño ya hace, no se leía en
ningún lado de `apps/metro/`.

**Estado nuevo**: `screen_lock_require` ∈ {`al bloquear` (predeterminado),
`tras 1 minuto`, `tras 5 minutos`, `solo al encender`}, persistido en
`aura.cfg` junto a las otras dos claves del bloqueo y **solo cuando hay
bloqueo** (misma regla de M-068, para que la salida de emergencia siga
siendo "borra estas líneas"). Un valor fuera de rango cae al
predeterminado: mismo criterio de **fallar abierto** que el resto del
módulo — un archivo dañado no puede dejar el aparato inservible.

**La máquina de estados vive en el bucle principal**, no en una pantalla
— igual que el bloqueo mismo, tiene que alcanzar a todo el aparato:

- El Hold **no genera eventos** (`pmu_holdswitch_locked()` es sondeo),
  así que se compara su valor con el de la vuelta anterior.
- **No hizo falta tocar ningún timeout.** El maestro pedía bajar la
  espera a ≤ HZ/2; el bucle ya esperaba como mucho **HZ/10**, así que el
  flanco se ve dentro de los 100 ms. Se verificó antes de cambiar nada,
  en vez de aplicar la receta a ciegas.
- Flanco **OFF→ON**: se anota el tick. Con clave configurada queda la
  **pantalla en reposo**, redibujada **una vez por segundo** y solo bajo
  `lcd_active()` — lo único que cuesta energía aquí.
- Flanco **ON→OFF**: si el ajuste lo pide y el Hold duró lo suficiente,
  se rearma; la vuelta siguiente lo cobra
  `metro_screen_lock_run_if_active()`, que **sigue siendo el único punto
  de interceptación**. `solo al encender` devuelve −1 y nunca rearma.

**La pantalla en reposo no lleva casillas de clave.** Con el Hold puesto
no se puede teclear nada, y unas casillas vacías que no responden
invitan a probar. Lleva lo que uno sí quiere poder mirar con el aparato
en el bolsillo —reloj y batería, que ya dibuja `metro_draw_header()`— más
el creciente de 64 px y el candado como marca de "esto está guardado".

**Sub-página de Ajustes › bloqueo** (la fila deja de hacer una sola cosa
y empuja una página, mismo patrón que "cambiar sistema", D-047): sin
clave muestra **solo "activar"** —cambiar, quitar o elegir cuándo pedir
una clave que no existe son filas muertas—; con clave, las tres
restantes. La fila se renombra de **"candado" a "bloqueo"** (§C).

**Verificado headless, que es lo que importa aquí.** El Hold no es un
botón y no se puede postear a la cola de eventos, así que sin
instrumentación esto habría quedado "probado a mano". Se portó de Metro
(M-104, leído de `../Metro-Aura`) el token **`HOLD`** de
`METRO_SIM_BUTTONS`, que conmuta `hold_button_state` —la misma variable
que la tecla `h` del simulador— y queda anotado en `MODIFICATIONS.md`.
Y los umbrales de 1/5 minutos se escalan a **3/8 segundos bajo
`#ifdef SIMULATOR`**: lo que hay que verificar es la máquina de estados,
no la aritmética de `HZ`, y un minuto de espera real haría imposible
capturar las dos ramas.

| captura | qué prueba |
|---|---|
| `f4-bloqueo-subpagina.png` | las cuatro filas con clave puesta, con "pedir código · al bloquear" |
| `f4-bloqueo-reposo.png` | Hold puesto: creciente + candado, sin casillas, con reloj y batería |
| `f4-bloqueo-pide-codigo.png` | Hold soltado con `al bloquear`: pide la clave |
| `f4-bloqueo-1min-antes.png` | `tras 1 minuto`, soltado **antes** del umbral: vuelve al hub, **no** pide nada |
| `f4-bloqueo-1min-despues.png` | el mismo ajuste, soltado **después**: pide la clave |

Las dos últimas son el par que de verdad prueba la máquina: la diferencia
entre ellas es solo el tiempo con el Hold puesto.

**Verificación**: target y simulador en 0 errores, **0 warnings nuevos**;
18 suites host en verde; `.bss` **8 461 404**, sin cambio frente a D-067
(el estado del Hold son tres locales del bucle, no estáticas), bajo el
techo D-043 con **112 672 B** de margen; `stack_report.py` OK, 5 528 B
(45.0 %).

**Pendiente de hardware** (a la lista de la ronda): que el sondeo a
HZ/10 vea el flanco del interruptor real sin rebotes, y que la pantalla
en reposo no impida que la retroiluminación se apague por su
temporizador normal.

### D-067, addendum — la marquesina no reiniciaba su reloj al cambiar de pantalla

Hallazgo de Metro al portarla (M-106, avisado por la supervisora):
`moonlit_marquee_reset()` estaba **escrita, documentada y sin un solo
llamador**. El estado de la marquesina se guarda **por ranura**, no por
texto, y la ranura de "fila seleccionada" es una sola para todas las
listas: dos pantallas cuyo primer texto coincida en los 48 bytes de la
clave heredan el ciclo a medias, y la fila nueva arranca desplazándose
sin el tramo quieto que existe justo para poder leerla.

Se llama ahora en `metro_screen_list_push()`, `_pop()`, `_pop_to_root()`
y en los dos cambios de pivot. Los tres centinelas —Marea, "Ahora suena"
y el visor de fotos— entran y salen por `metro_screen_list_push()`/
`_pop()`, así que quedan cubiertos sin una línea más.

## D-071 — Ajustes homologados: apagado automático, clicker, avisos legales y dos pantallas de barra

**Encargo**: maestro §C, la matriz de Ajustes que las tres familias
deben tener parecida.

**Lo que faltaba, y lo que de verdad importaba.** De las filas nuevas la
que hacía falta es **apagado automático** {nunca, 10, 20, 60 min}: sin
ella, un aparato olvidado en un bolsillo se queda encendido hasta agotar
la batería, y Rockbox ya trae el ajuste (`global_settings.poweroff` +
`set_poweroff_timeout()`) sin que moonlit lo expusiera. También entran
**clicker** (`global_settings.keyclick`, aquí como interruptor: moonlit
no expone tres fuerzas de un clic de 10 ms) y **avisos legales**, que la
GPL v2 §3 exige tener a la vista del usuario y no solo en el repositorio.

**Un defecto de fondo que afectaba a filas que ya existían.**
`settings_save()` de Rockbox **no escribe en el acto**: registra un
callback en `DISK_EVENT_SPINUP`, y `call_storage_idle_notifys()` se
auto-bloquea 30 s entre corridas, así que el flush real llegaba con el
apagado limpio. Un aparato al que se le acaba la batería perdía el
cambio. Toda fila que toque `global_settings` termina ahora en
`save_global_settings_now()` — incluidas **brillo, retroiluminación y
límite de volumen**, que ya existían y ya tenían el problema. (Hallazgo
de Metro R7-5, estándar de los tres repos.)

**Brillo y retroiluminación dejan de ciclar valores con SELECT** y pasan
a una pantalla propia con barra (`metro_screen_adjust.c`, portada de
Metro M-103 con el idioma visual de moonlit). Con cuatro pasos de brillo
no se podía afinar; con seis de retroiluminación, llegar al que uno
quiere costaba cinco pulsaciones sin ver nunca el rango. El brillo pasa a
los **diez** pasos que pide el maestro, repartidos sobre el rango real
del aparato con el paso 0 en el mínimo (nunca 0: apagar la pantalla no
es un nivel de brillo). La retroiluminación conserva sus seis valores no
lineales, incluido "nunca", que no son una rejilla y no tendría sentido
interpolar. La rueda mueve **un paso por evento, sin aceleración** (un
control de 6 o 10 posiciones con la aceleración pensada para listas de
cientos de filas sería imposible de parar), se aplica **en vivo**, y hay
**un solo guardado, al salir**.

El subtítulo de la fila muestra la **posición del control**, no el crudo
de `global_settings`: si no, la fila y la barra pueden enseñar dos
porcentajes distintos para el mismo estado.

**Avisos legales** usa `metro_screen_text.c` (también portada de M-103):
texto corrido con ajuste de línea, la rueda desplaza **con** aceleración
—al revés que la de barra: aquí se recorre un texto largo, no se elige
entre pocas posiciones—, sin leer disco (la cadena vive en
`metro_lang.c`, bilingüe como todo lo demás).

**Fuera de esta ronda, por acuerdo**: fecha y hora, y ajuste de volumen
(replaygain) — el plan ya los marcaba P2.

**Verificación**: `f5-brillo.png` (barra a 33 %, valor en acento),
`f5-avisos-legales.png` (párrafos ajustados). Target y simulador en 0
errores, 0 warnings nuevos; 18 suites en verde.

### D-071, addendum — "ajuste de volumen" (replaygain) sí entra en la ronda

Yo lo había dejado fuera leyendo "P2" en el plan hijo; la supervisora
corrigió el alcance: **solo fecha y hora** queda fuera. Se agrega, con el
mismo idioma visual que las demás filas de valor.

Tres valores —**desactivado · por pista · por álbum**—, sobre
`global_settings.replaygain_settings.type` + `dsp_replaygain_set_settings()`
para que el DSP lo aplique a la pista **en curso** y no al siguiente
arranque, más el flush de `save_global_settings_now()`.

Rockbox tiene un cuarto valor, `REPLAYGAIN_SHUFFLE` ("por pista, pero
solo cuando el aleatorio está puesto"), que **no se expone**: es una
condición que hay que explicar para poder elegirla, y en una lista de
tres palabras no cabe explicarla. Quien lo traiga puesto desde otro
firmware ve la fila en "por pista", que es lo que ese valor hace la mitad
del tiempo — y es además el predeterminado de Rockbox, así que es el
estado en que la fila aparece de fábrica. Mismo criterio que Metro
(M-103).

Verificado en el simulador: la fila cicla `por pista` → `por álbum`
(`f5-ajuste-volumen.png`). `.bss` sin cambio (8 467 612): la fila no
agrega estáticas.

## D-072 — Fotos: la maestra ES el tile, y el visor se navega como se espera

**(a) La rejilla lee la maestra directo.** La maestra de una foto mide
80 px (`MOONLIT_MASTER_ART_PHOTO_SIZE`) y el tile mide **80**
(`METRO_TILE_SIZE`): el `.mth` era una **copia byte a byte** de un
archivo que ya estaba en disco, más una vuelta entera de cola por foto
(encolar → decodificar en el tick → escribir → releer) para no ganar
nada. Ahora la rejilla lee la maestra en el camino de dibujo, con
presupuesto de **4 lecturas por cuadro** (`metro_thumbs_begin_frame()`),
el mismo criterio de lectura acotada que Marea (D-057): el dibujo de una
rejilla no puede convertirse en ocho lecturas de disco seguidas.

Álbumes y artistas **no** cambian: su maestra es de 130 px y hay que
reducirla a 80, así que ahí el `.mth` sí evita trabajo real. La
distinción vive en un campo nuevo de `struct metro_thumb_source`
(`master_path`), que solo Fotos rellena.

**Un error propio, cazado en la primera captura.** Al agotarse el
presupuesto del cuadro devolvía `NULL` **sin encolar**, así que la
segunda fila de la rejilla se quedaba en monograma hasta que el usuario
la moviera — se ve en la primera corrida: cuatro tiles cargados arriba y
cuatro monogramas abajo. Corregido: agotado el presupuesto, el ítem cae
a la cola de siempre y `metro_thumbs_tick()` lo resuelve en la vuelta
ociosa, que es quien fuerza el repintado.

Verificado en disco, no de vista: tras dibujar la rejilla,
`/.aura/thumbs/photos` **no existe** y `/.aura/thumbs/albums` conserva
sus 7 archivos. `CONTRATO-moonlit-studio.md` pasa a **v6** con la nota de
compatibilidad en las dos direcciones (Metro puede seguir escribiéndolo;
moonlit lo ignora, y la purga de `format.txt` se lo lleva igual).

**(b) El visor.** LEFT/RIGHT pasan de foto — en el visor no hay pivotes
que torcer, así que estaban **sin mapear**, y son el gesto que cualquiera
prueba primero. Al cambiar de foto entra un deslizamiento horizontal:
`metro_transitions_photo_slide()`, el mismo slide de siempre (misma
captura, misma composición, misma costura, misma puerta de nivel de
animación) pero **topado a 5 cuadros = 150 ms** bajo `all`. Un twist de
pivote se hace una vez al entrar a una pantalla; pasar de foto se hace
en ráfaga, y a 210 ms el visor se siente pastoso justo en el gesto que
más se repite. Bajo `minimal` el tope no toca nada: sus 4 cuadros
(120 ms) ya cumplen.

**MENU vuelve conservando la selección**, que es lo que el plan pedía y
no pasaba: el visor podía haber avanzado cincuenta fotos y la rejilla se
quedaba en la que se abrió. Se usa `metro_nav_move_sel_grid()` y **no**
`metro_nav_set_sel()`: en una cuadrícula `first_visible` tiene que ser
múltiplo de `METRO_TILE_COLS` —`metro_draw_tiles()` mapea slot→fila/
columna suponiéndolo— y `_set_sel()` ventana por filas sueltas, lo que
dejaría la rejilla corrida.

**Verificación**: `f5-fotos-rejilla.png` (ocho tiles cargados, sin
`.mth` de fotos en disco), `f5-visor-derecha.png` (LEFT/RIGHT),
`f5-visor-vuelta.png` (MENU vuelve con la tercera foto seleccionada,
`dreamscape.jpg`, y la rejilla alineada). Target y simulador en 0
errores, **0 warnings nuevos**; 18 suites en verde; `.bss` **8 467 612**
(+6 208 B: las 96 líneas de 64 bytes del ajuste de texto de los avisos
legales, estáticas a propósito y no en la pila —el tope de marco de
`stack_report.py`—, más el presupuesto de la rejilla), bajo el techo
D-043 de 8 574 076 con **106 464 B** de margen; `stack_report.py` OK,
5 528 B (45.0 %).

## D-073 — El bootloader deja de arrancar en negro, y la marca no salta

**Encargo**: maestro §B. Desde D-050 el **firmware** pinta el creciente
desde su primer cuadro, pero el **bootloader** seguía arrancando en
negro absoluto (`verbose = false`). Es ~1 s de pantalla vacía y, sobre
todo, el bootloader es la pieza que se flashea en NOR y la única que
corre **antes de que exista un sistema de archivos**: el único sitio
donde se puede citar el origen del código sin depender del disco.

**La pantalla**: el creciente centrado en los dos ejes + dos leyendas
`FONT_SYSFIXED` en `on_surface_variant`, la última a 14 px del borde
inferior, interlineado 12: `moonlit - arranque <rbversion>` y
`Basado en Rockbox - GPL v2 - rockbox.org`. La versión es la del
**bootloader**, que es lo único que él conoce. Sin retardo artificial.

**Que la marca no salte no es un offset copiado a mano.**
`generate.py --bootloader-crop` calcula dónde cae la tinta del creciente
en pantalla cuando lo dibuja el firmware —centro del lienzo de 320×98 en
la pantalla, más la posición del creciente en el lienzo, más la caja de
tinta dentro del creciente: **(134, 94)**— y **resuelve** los márgenes
del recorte para que el centrado con **división entera** del bootloader
(`(LCD_WIDTH - crop_w) / 2`) caiga en ese mismo píxel. Después
recalcula ese centrado y **aborta antes de escribir el archivo** si no
coincide. Si alguien cambia el tamaño del creciente o del lienzo, esto
falla en generación en vez de dejar una marca que salta.

**El solver es el de Metro (M-107), no el de Aura (D-347), y era
necesario.** Aura ensancha el margen lejano 0 o 1 px con piso de 4;
Metro busca el **par más simétrico** con piso 2. Con el creciente el de
Aura habría abortado: la caja de tinta mide **45×52** dentro de un
cuadro de 72 y no está centrada en él —una luna es asimétrica por
definición—, así que el objetivo (134) no es el centro de pantalla para
45 px de tinta (137). El par mínimo posible es **izq 2 / der 8**
(asimetría 6, y no hay ninguno mejor: comprobado recorriendo el espacio
de soluciones). En vertical sale simétrico, 2/2.

Resultado: `bootwordmark.55x56x16.bmp`, **9 462 B** — muy por debajo del
presupuesto de 32 KB de bitmap que fija §B.1.

**Tamaño del bootloader**: 95 592 → **102 024 B**. `MOVE_AREA`
(`IRAM1_SIZE - IM3HDR_SZ` = 0x1F800 = 129 024 B) queda al **79.1 %**,
con **27 000 B libres**. Bajo el tope de 150 KB de §B.1.

**Lo que NO cambia**: `verbose` sigue en `false`;
`error()`/`fatal_error()`, la batería crítica y el modo USB del
bootloader escriben igual que siempre — `draw_boot_screen()` restaura el
primer plano a blanco antes de volver, para no teñir esos mensajes.

**`MODIFICATIONS.md`**: deja de ser cierto que «nada en `bootloader/`
(BOOT-1 intacto)», como decía la entrada de D-050. Anotado explícitamente.

**Verificación**: `build_target.sh --bootloader` enlaza en **0 errores,
0 warnings**; maqueta de aprobación en
`docs/screenshots/ronda-pulido/bootloader-maqueta.png`, dibujada con los
**glifos reales de sysfont** leídos de `fonts/08-Schumacher-Clean.bdf` —
una maqueta con una fuente parecida sería justo lo que no sirve: lo que
se aprueba tiene que ser lo que se va a ver. El simulador **no** ejecuta
el bootloader, así que la pantalla real queda para la lista de hardware
(y es lo que hay que mirar antes de flashear).


---

## D-075 — Reproducibilidad: `make dep` que nunca se refrescaba, y un `zip -r` que agregaba en vez de reemplazar

**Encargo** (maestro §I.5, nota de Aura D-348): la regla `$(DEPFILE) dep:`
de `tools/root.make` —heredada de Rockbox, **compartida por los tres
repos del fork**— no tiene prerrequisitos: `make.dep` se genera **una
vez**, al crear el directorio de build, y nunca se refresca solo. Un
`#include` agregado después queda invisible para `make`, y el `.o` que
lo usa no se recompila cuando esa cabecera cambia — el binario terminaría
dependiendo de **cuándo** se creó el directorio de build, no solo del
commit. Aura lo midió en su propio árbol: 7 bytes de diferencia entre un
build limpio y uno incremental del mismo commit.

**La corrección, en `build_target.sh`**: `build_one()` corre `make dep`
**siempre**, después de configurar y antes de compilar — no solo para un
release. Cuesta ~25 s por directorio en este árbol (medido), barato
frente al riesgo de un binario que no es reproducible.

**`BUILD_TARGET_CLEAN=1`**: `package_dist.sh` lo exporta cuando se pasa
`--release-tag`, y `build_one()` borra el directorio de build entero
antes de reconfigurar. Un release tiene que ser reproducible byte a
byte — es lo que sostiene la actualización selectiva por CRC32 de
Studio (contrato v18) — y `make dep` sobre un directorio viejo no lo
garantiza tanto como empezar de cero.

**Verificación de la reproducibilidad, con los tres pasos que importan
por separado:**

1. **`make dep` no rompe el incremental.** `build_target.sh` normal
   (sin `BUILD_TARGET_CLEAN`) sobre el árbol ya compilado: `make dep`
   corre, no encuentra nada nuevo que recompilar (`Nothing to be done`),
   exit 0.
2. **Limpio vs. incremental, byte a byte.** `BUILD_TARGET_CLEAN=1`
   reconstruyó `firmware/build-ipod6g` desde cero (`rm -rf` +
   reconfigurar + `make dep` + `make`, ~4m46s) sobre el mismo commit
   del incremental de arriba. `cmp` de los dos `rockbox.bin`:
   **idénticos byte a byte** (`cmp -l` → 0 diferencias). `text`/`data`/
   `bss` idénticos: `1 250 476 / 12 308 / 8 467 612`. Es un resultado
   **más fuerte** que el del addendum 2 de D-062 (que solo comparaba
   `.bss` y toleraba los 22 bytes de `RBVERSION`): aquí el árbol estaba
   limpio en las dos corridas, así que `RBVERSION` también coincidió.
3. **Los módulos nuevos de esta ronda SÍ quedan rastreados.**
   `make.dep` del build limpio tiene una entrada propia para cada `.o`
   nuevo de la ronda (`moonlit_translit.o`, `moonlit_marquee.o`,
   `moonlit_marquee_cycle.o`, `metro_screen_adjust.o`,
   `metro_screen_text.o`), cada uno con su `.c` y sus `.h` como
   prerrequisito — no es una entrada heredada de un `make dep` viejo
   que por casualidad seguía siendo válida.

**Un segundo defecto, propio de este repo, cazado verificando el
paquete real y no solo el binario.** `package_dist.sh` arma
`rockbox.zip` con `zip -qr "$DIST_DIR/rockbox.zip" .rockbox` dentro del
`$STAGE` temporal — y `zip -r` **agrega** a un archivo existente, no lo
reemplaza. Un `rockbox.zip` de una corrida anterior con `--release-tag`
dejó su `.rockbox/aura/version.txt` **sobreviviendo** dentro del zip de
la corrida siguiente, sin el flag: esa corrida nunca escribe
`version.txt` en `$STAGE`, así que no había nada que lo pisara, y
`zip -r` no borra lo que no vuelve a ver. Se detectó verificando el
contenido del zip recién armado (`unzip -l` + `grep aura`), no
suponiendo que "sin `--release-tag`, sin `version.txt`" bastaba —
justo la clase de verificación que esta ronda viene aplicando en vez de
confiar en la lectura del código.

**Corrección**: `rm -f "$DIST_DIR/rockbox.zip"` antes de armar el
nuevo. Verificado con dos corridas consecutivas de `package_dist.sh`
sin `--release-tag`: la primera (sin el `rm -f`) mostró el
`version.txt` fantasma; con la corrección, la segunda **no** lo tiene.

**Verificación completa de `package_dist.sh`** (sin `--release-tag`,
como exige esta ronda — nunca se pasó `--release-tag` con éxito: el
único intento abortó por el árbol sucio, que es el comportamiento
correcto de D-297/M-056 y no se tocó):
- Los 7 assets del contrato: `rockbox.ipod`, `rockbox.zip`,
  `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`,
  `MODIFICATIONS.md`, `THIRD-PARTY-NOTICES.txt`.
- 406 archivos en el árbol `.rockbox/` armado, centinelas verificados.
- `stack_report.py` corre **dentro** del script (antes de `make zip`)
  y sale en verde.
- `checksums.txt` con SHA-256 de los cuatro binarios.
- Sin `.rockbox/aura/version.txt` (correcto, sin `--release-tag`).

**`.bss` y `stack_report.py`, sobre el binario del build limpio**:
`.bss` **8 467 612** contra el techo D-043 (`8 574 076`) — margen
**106 464 B**. `stack_report.py`: OK, peor camino 5 528 B (45.0 % de
12 288 B).

## Fase 7 — cierre

Cerrada. `package_dist.sh` corre sin `--release-tag`, con
reproducibilidad verificada (D-075) y produce los 7 assets del
contrato. Ningún tag, ninguna release: eso lo dispara el dueño después
de probar en hardware.

### Lista de verificación en hardware — ronda "pulido"

Puntos ya anotados en cada decisión, reunidos aquí para que el dueño
tenga una sola lista. Ninguno se probó en el simulador — el simulador
verifica geometría, máquinas de estado y ausencia de regresión; lo que
sigue solo se puede confirmar en el iPod real.

**Arranque y bootloader (D-073)**
- [ ] La pantalla de arranque del bootloader se ve como
  `docs/screenshots/ronda-pulido/bootloader-maqueta.png` — creciente
  centrado, dos leyendas legibles, sin parpadeo.
- [ ] El creciente **no salta** en el paso bootloader → firmware (es
  el punto entero del recorte resuelto por `generate.py
  --bootloader-crop`; en el simulador no se puede ver porque no
  ejecuta el bootloader).
- [ ] Flashear con `mks5lboot` desde el paquete de `package_dist.sh` y
  confirmar que el dispositivo arranca.

**Pila y estabilidad (D-062)**
- [ ] Marca de agua de la pila principal en "Acerca de" (SELECT
  sostenido sobre la fila de versión) tras ≥ 10 minutos de uso
  intenso: Estilo/Temas, fotos, USB conectar/desconectar, Music Flow.
  Debe quedar por debajo del 75 % que exige el maestro §E.3. El
  simulador siempre muestra "n/d": los hilos SDL no tienen ese campo.
- [ ] Que 12 KB de pila principal (subida de 8 KB, D-062) sea
  suficiente sin que reaparezca el panic `Stkov main` original.

**Skins apagados (D-062, addendum)**
- [ ] La pantalla USB real (conectar el cable con el candado puesto y
  sin él) — es el camino que más bajó de costo de pila (7 056 →
  4 376 B) y el arnés headless no puede inyectar una conexión USB de
  verdad, solo pulsaciones.

**Caché de carátulas (D-063)**
- [ ] Purga de `format.txt` en un disco con miles de entradas (el
  maestro §A.3 pide medir si supera ~2 s; en el simulador, con tres
  directorios planos, es instantánea).

**"Acerca de" (D-064, D-062)**
- [ ] El gesto SELECT sostenido sobre la fila de versión revela/oculta
  la marca de agua de la pila (el arnés headless solo inyecta
  pulsaciones cortas).

**Marea (D-065, D-014/D-043 — "experimental hasta M12")**
- [ ] 60 cuadros de scroll continuo contra el techo de 33 ms del
  maestro §E.3 — el simulador corre sobre una CPU tres órdenes de
  magnitud más rápida y un filesystem de host, así que su medición
  (D-065) no puede resolver esto.
- [ ] Confirmar que el **peor caso** (pantalla llena de carátulas
  reales) no cambió respecto a antes de D-065 — es lo que el análisis
  predice, no lo que el simulador puede medir.
- [ ] Calentamiento síncrono al entrar (7 slots, destino ±3): que los
  ≤ 150 ms del maestro se cumplan con disco real, no con el del host.

**Marquesina (D-067)**
- [ ] Que 5 000 ms de barrido se lean cómodos con un título largo de
  verdad (~40 caracteres).
- [ ] Costo en batería del repintado a `HZ/20` mientras una marquesina
  desplaza — es el único mecanismo de esta ronda que pide cuadros
  fuera de una animación puntual.

**Barra de estado (D-068)**
- [ ] Alineación por caja de tinta (`check_tones.py --align`) en el
  panel real, no en una captura del simulador — el LCD físico puede
  tener su propio gamma/anti-aliasing.

**Bloqueo por Hold (D-069)**
- [ ] Que el sondeo a `HZ/10` vea el flanco del interruptor Hold real
  sin rebotes (el simulador conmuta `hold_button_state` de forma
  instantánea y limpia; un interruptor mecánico no).
- [ ] Que la pantalla en reposo (Hold puesto, con clave) no impida que
  la retroiluminación se apague por su temporizador normal.
- [ ] **Comparar autonomía en reposo con el bloqueo desactivado vs.
  activado** (verificado por lectura de código que el sondeo de Hold
  no agrega costo — D-062/D-069, nota de cierre—, pero la medición
  real de batería no se puede hacer en el simulador).

**Ajustes (D-071)**
- [ ] Apagado automático: 10 min sin actividad → el aparato se apaga
  de verdad.
- [ ] Brillo: los 10 pasos se notan como pasos reales en el panel
  físico, no solo como números.

**Fotos (D-072)**
- [ ] Deslizamiento del visor (topado a 150 ms) se siente fluido con
  fotos grandes decodificándose de disco real.

**Reproducibilidad y empaquetado (D-075)**
- [ ] Ninguno — verificado enteramente en esta sesión (build limpio
  byte-idéntico al incremental, paquete completo sin `--release-tag`).

**Fuente de puntuación uniforme (D-074)**
- [ ] Ninguno — verificado enteramente en esta sesión (simulador +
  tests host); es dibujo, no temporización ni hardware específico.

**Verificado en hardware por el dueño el 2026-09-04 (v0.2.0):** arranque,
Marea, fuentes, bloqueo, ajustes y fotos correctos; ajustes menores
pendientes para la siguiente ronda.

## D-074 — Fuente de puntuación uniforme: seis roles, medida por intersección

**Fase 8, condicionada por la supervisora al cierre en verde de todo lo
anterior (§I de la ronda)**: el resto cerró en verde, así que se
implementa la variante "uniforme o nada" que el addendum de D-066 dejó
medida y pendiente — comillas curvas, rayas y puntos suspensivos de
verdad, en vez de su equivalente ASCII, en los seis roles que muestran
metadatos.

**`MAXUSERFONTS` 12 → 16** (`firmware/export/font.h`). moonlit ya
cargaba 7 fuentes; seis roles más (todos salvo `MFONT_DISPLAY`, que
solo dibuja nombres de pivote) suman 13, con margen hasta 16 para no
volver a tocar este número. Costo medido: **+96 B de `.bss`** —
`buflib_allocations[MAXFONTS]` de `firmware/font.c` más los arreglos de
`apps/gui/skin_engine/skin_parser.c` (existen aunque moonlit no use el
motor de skins, D-062 addendum, porque el módulo se compila igual para
el target). Muy por debajo de los 16 B que estimaba el addendum —la
estimación solo contaba el primer arreglo— y de cualquier forma
trivial.

**Seis `.fnt` nuevos** (`design-system/generate.py --fonts`, extendido):
rango denso 8208–8482 (U+2010–U+2122, el mismo bloque que ya cubre
`moonlit_translit.c`), una por rol. `convttf` no siempre arranca
exactamente en 8208 — Libre Baskerville no trae U+2010 (HYPHEN), aunque
sí la mayoría del resto, mismo fenómeno que D-032 documenta para
U+017F pero en el extremo inicial del rango en vez de en medio — así
que a diferencia del rango primario (D-007) **no** se exige
`firstchar == 8208`; lo que decide qué se usa es la intersección de
abajo, no la cabecera de cada `.fnt`.

Medido, no estimado: **39 502 B en disco, 4 932 B de tablas en RAM**
(los seis roles residentes) — casi exacto al ~44 KB / ~4.9 KB que el
addendum de D-066 había calculado.

**La intersección, no la unión.** De los 24 codepoints de
`moonlit_translit_table` (23 dentro del rango 8208–8482; el espacio duro,
U+00A0, queda fuera y sigue transliterando siempre), `generate.py` lee
la tabla de anchos real de cada uno de los seis `.fnt` —mismo cruce
"la tabla termina donde el archivo" que evitó el error de desplazamiento
de D-066— y **solo** los codepoints que los **seis** dibujan con un
glifo real (ancho > 0) entran a la tabla generada
`moonlit_punct_table.c`. Medido: **20 de 23** — quedan fuera U+2010,
U+2011 y U+2012 (la familia de guiones cortos que Baskerville no trae),
que siguen transliterando a `-` como antes, sin que se note: ya
transliteraban a lo mismo.

Es la condición que el addendum de D-066 identificó como la razón real
para no hacerlo con dos roles: "Ahora suena" dibuja el álbum en
`MFONT_LIST` y el título en `MFONT_TITLE` — con la intersección, los
dos roles dibujan **exactamente** el mismo conjunto de codepoints con
fuente propia, así que un mismo texto nunca mezcla comilla curva en un
rol y comilla recta en otro dentro de la misma pantalla.

**Dibujo por tramos** (`moonlit_textseg.c`, módulo puro, mismo patrón
que `moonlit_marea_prefetch.c` y `moonlit_marquee_cycle.c`): recorre la
cadena codepoint a codepoint y clasifica cada uno en PRIMARY (rango
32–383, o transliterado si no hay fuente de puntuación que lo cubra) o
PUNCT (está en la intersección — sin transliterar, es justo lo que la
fuente de puntuación dibuja de verdad). Tramos contiguos del mismo tipo
se funden en uno solo. `metro_draw_text()`/`metro_draw_text_clipped()`
dibujan cada tramo con su fuente y avanzan la x por el ancho medido de
cada uno; `metro_draw_text_width()` sigue el mismo camino, así que la
medición que usa la marquesina (D-067) para decidir si desborda es
coherente con lo que de verdad se dibuja.

Sin fuente de puntuación (`MFONT_DISPLAY`), el resultado es **siempre**
un solo tramo transliterado entero — el comportamiento de D-066 sin
excepciones, verificado con su propio caso de test.

`lcd_setfont()` dentro de un viewport activo escribe directo
`lcd_current_viewport->font` (`firmware/drivers/lcd-bitmap-common.c`,
verificado leyendo el código, no supuesto): cambiar de fuente por tramo
dentro de `metro_draw_text_clipped()` no necesita ningún mecanismo
nuevo, el mismo `vp` que ya se armaba una vez ahora recibe un
`lcd_setfont()` por tramo.

**Verificación de punta a punta, en el mismo álbum de prueba de
D-066/D-067.** Capturas del recorrido completo:
- `f8-lista-albumes.png` — la cuadrícula: `Don't Stop — "Live"…`, con
  raya y comillas **curvas de verdad**, no su sustituto ASCII.
- `f8-canciones.png` — la ceja (mismo texto, rol `MFONT_LABEL`) y
  `Believin'` con apóstrofo curvo (`MFONT_LIST_SEL`); `Wheel · in the
  Sky` sigue con el `·` de D-066 — la corchea está fuera de
  8208–8482 por completo, ninguna fuente de puntuación la cubre y
  el defaultchar sigue resolviéndola.
- `f8-nowplaying.png` — el álbum (`MFONT_LIST`) con raya y comillas
  curvas.
- `f8-marea-panel.png` — el título del panel (`MFONT_HEADLINE`) con
  apóstrofo y raya curvos, y el artista `Journey's Edge`
  (`MFONT_BODY`) con su propio apóstrofo.
- `f8-marquesina-1.png`/`f8-marquesina-2.png` — mismo recorrido de
  D-067 a dos instantes distintos: la marquesina sigue desplazando
  (el texto avanzó entre los dos tics) y sigue dibujando con la fuente
  de puntuación en vez de la transliteración.

Cinco de los seis roles quedan documentados con evidencia visual directa
(falta solo `MFONT_TITLE` de Ahora Suena, que en este recorrido no tocó
mostrar un título con puntuación especial — el camino de código es el
mismo `build_segs()` que los otros cinco, así que no hay una razón
estructural para que se comporte distinto).

**Verificación.**
- Host: `test_textseg` nuevo, **45 checks** — el caso del encargo
  segmentado exactamente como se espera (8 tramos, alternando y
  fundiendo contiguos), lo que no tiene fuente de puntuación sigue
  transliterando, casos de borde (empieza/termina en PUNCT, buffer y
  `max_segs` agotados sin desbordar). Las 19 suites completas: **0
  fallos**.
- Un warning propio, corregido antes de commitear:
  `-Wunused-parameter` en el helper `load_one()` — sus dos parámetros
  de diagnóstico solo se usan dentro de `DEBUGF()`, que se compila a
  nada fuera de un build de depuración.
- Target y bootloader: `build_target.sh` (los dos) en **0 errores, 0
  warnings nuevos**. Simulador: **0 errores, 0 warnings nuevos**.
- `.bss`: **8 467 708** (+96 B sobre D-075, exactamente el costo de
  `MAXUSERFONTS`; ningún dato de fuente vive en `.bss` — se carga en el
  heap de buflib en runtime, igual que las siete fuentes primarias ya
  hacían). Bajo el techo D-043 de 8 574 076, margen **106 368 B**.
- `stack_report.py`: OK, 5 528 B (45.0 %) — sin cambio: `load_one()` no
  agrega marco significativo.
- `package_dist.sh` sin `--release-tag`: corrida completa, los seis
  `.fnt` de puntuación presentes en `rockbox.zip`, `stack_report.py`
  en verde dentro del script.
- **Reproducibilidad, con `font.h` tocado**: `firmware/export/font.h` es
  un archivo de core, así que se repitió la verificación de D-075 sobre
  este cambio en particular — no dar por sentado que un header
  compartido se comporta igual solo porque D-075 ya lo probó con otro
  diff. Build limpio (`BUILD_TARGET_CLEAN=1`) contra incremental, **mismo
  commit** en los dos (el primer intento comparó binarios de dos commits
  distintos por un descuido del propio proceso de verificación —
  detectado por el `cmp` mostrando 20 bytes de diferencia justo donde
  vive la cadena `RBVERSION`, no en el código; repetido con el commit
  fijo, **idénticos byte a byte**).

### Cierre de la ronda

Trece decisiones cerradas (D-061 a D-075, sin contar addenda): pila y
estabilidad, caché de carátulas v18, "Acerca de" navegable, Marea sin
fases visibles, glifos faltantes y marquesina, barra de estado
alineada, bloqueo por Hold, tiles con selección visible, ajustes
homologados, fotos como Aura, bootloader con pantalla de arranque,
reproducibilidad del empaquetado, y fuente de puntuación uniforme.

Tag sugerido para cuando el dueño termine la lista de hardware:
**`v0.2.0`**. Esta sesión no crea el tag ni el release — package_dist.sh
corrió únicamente sin `--release-tag`, tal como pide el protocolo de
la ronda.

## Ronda "ajustes 2"

Autorizada por la sesión supervisora tras el cierre de "ronda pulido"
(v0.2.0 liberado y verificado en hardware). Plan hijo
`docs/plans/PLAN-moonlit-ajustes-2.md`, maestro
`docs/plans/PLAN-ronda-ajustes-2-maestro.md` (4 repos en paralelo:
Aura-Firmware, Metro-Aura, moonlit-aura, Aura-Studio). Decisiones
**D-076+**. Mismo protocolo que la ronda anterior: cada PARADA se
reporta a la supervisora y la sesión continúa sin esperar; se detiene
solo ante build en rojo, contrato, `.bss` sobre el techo D-043 sin
arreglo claro, acción destructiva, o contradicción con este archivo.
Sin tag ni release.

## D-076 — La ceja queda delimitada: el título nunca choca con reloj, candado o transporte

**Motivo.** `metro_draw_header()` dibujaba el título con
`metro_draw_text()`, sin recorte, ANTES de calcular dónde iban a caer
el candado, el glifo de transporte y el reloj — un título largo (un
nombre de artista o álbum real, no una etiqueta fija de la interfaz)
podía superponerse con esos elementos. Nadie lo había notado porque
D-067 (marquesina) ya resolvía el caso general "texto largo que no
cabe en su propia franja" en listas y filas; la ceja nunca se sometió
a esa misma disciplina.

**Decisión.** `metro_draw_header()` calcula PRIMERO `right_edge`: el
borde izquierdo del elemento más a la izquierda entre los que
realmente van a dibujarse esta vez (batería — siempre presente, es el
límite por defecto si no hay nada más —, reloj si `get_time()` no es
NULL, glifo de transporte si hay audio, candado si `button_hold()`).
El título se dibuja despues, con `moonlit_marquee_draw()` (D-067) en
vez de `metro_draw_text()`, con un ancho de recorte de
`right_edge - 8 - METRO_DRAW_LEFT_X` — el hueco mínimo de 8 px que
pide el maestro (§E.1). Nueva ranura `MOONLIT_MARQUEE_HEADER` en
`moonlit_marquee.h`. El resto de la barra (candado, transporte, reloj,
batería) no cambia de posición ni de lógica — solo el orden en que se
calcula antes de dibujar.

**Verificación mecánica, no a ojo.** `firmware/tools/check_tones.py
--align` gana `--title-gap X0,W`: exige que la banda de columnas
`[X0, X0+W)` de la captura no tenga NI UN píxel de tinta. No intenta
adivinar cuál grupo de columnas es "el título" (una marquesina en
pleno barrido puede partirse en varios grupos — dos copias con el
hueco de bucle entre ellas —, y agrupar por heurística sería
adivinar); le basta con que la banda que el propio llamador calculó
(el hueco de 8 px) esté limpia. Corrido contra
`f1-03-header-delimitado.png` (candado + transporte + reloj + batería,
título largo desbordando) con `--title-gap 226,8`: **sin tinta,
OK**. El chequeo de alineación vertical preexistente (D-068) se
reverificó aparte contra una captura ESTÁTICA (`f1-01-marea-directo.png`,
título corto "marea" sin marquesina): dispersión **1.0 px**, dentro
del tope — confirma que el reordenamiento del cálculo no movió el eje
Y de nada. (Una captura a mitad de barrido de marquesina SÍ puede dar
más de 1.0 px de dispersión en ese chequeo preexistente porque el
tramo de texto visible en ese instante no es necesariamente el mismo
que se usó para calibrar `METRO_HEADER_TEXT_Y` — es ruido de contenido,
no del código; ya advertido en el docstring de la herramienta desde
D-068.)

**Capturas** (`docs/screenshots/ajustes-2/`): `f1-03-header-delimitado.png`
(título largo + Hold + reproduciendo + reloj, los cuatro elementos a
la vez, la marquesina del título recortada limpiamente antes del
candado).

## D-077 — Música entra directo a Marea; LEFT/RIGHT recorren los pivotes

**Motivo.** Música abría una página de pivotes cuyo primer pivote era
una sola fila ("Marea") que había que seleccionar para de verdad llegar
a la pantalla completa de Marea (D-051). Un paso intermedio sin
contenido propio, dado que Marea ya es el destino natural desde D-051.

**Decisión — mecanismo genérico, no un caso especial de Música.**
`struct metro_pivot` (`metro_page.h`) gana un campo `is_launcher`
(booleano, al final, default `false` — todo inicializador posicional
existente sigue compilando igual, mismo patrón que `tile_cols`/
`empty_message`). Un pivote `is_launcher` nunca se dibuja como lista:
en cuanto el cursor aterriza en él —al empujar la página por primera
vez (`metro_screen_list_push()` siempre arranca en pivote 0) o al
retroceder con `MACT_PIVOT_PREV` desde el pivote 1—,
`metro_screen_list.c` dispara su `on_select(ctx, 0)` de inmediato en
vez de mostrar `count()`/`get_row()`. `music_pivots[0]` (Marea,
`metro_screen_hub.c`) se marca así; su `on_select` ya era
`moonlit_screen_marea_push()`, sin cambios. Resultado: abrir Música
entra directo a Marea, y `metro_screen_hub.c` no necesita saber que
Marea existe como caso especial — es el mismo mecanismo que usaría
cualquier otro pivote de una sola acción.

**LEFT/RIGHT en Marea.** Marea reusa `MCTX_LIST` desde D-030 (candado
+ transporte + reloj ya lo dejaban dicho: sin tocar `metro_keymap.c`).
`moonlit_screen_marea_handle()` gana los casos `MACT_PIVOT_NEXT`/
`MACT_PIVOT_PREV`: ambos hacen `metro_screen_list_pop()` (Marea es un
centinela sobre el frame de `music_page`, que quedó en pivote 0 antes
de mostrarse) y luego mueven ese frame — RIGHT con
`metro_nav_pivot_next()` normal (pivote 0→1, Quickplay); LEFT con la
función nueva `metro_nav_pivot_set()` (`metro_nav.h`/`.c`) directo al
**último** pivote (Playlists). `metro_nav_pivot_set()` es un escape
deliberado, PURO y probado en host (`test_nav.c`, `test_pivot_set()`,
+9 checks): salta y recorta a `[0, npivots-1]` sin pasar por
`_next()`/`_prev()`, que **siguen** sin envolver en cualquier otra
pantalla (F.1, `metro_nav.h`, intacto — verificado con
`test_pivot_no_wrap()`, que no cambió). Simétrico: LEFT desde el
primer pivote de lista (Quickplay, pivote 1) llega a pivote 0 vía
`metro_nav_pivot_prev()` normal, y como pivote 0 es `is_launcher`,
`metro_screen_list.c` dispara Marea en el acto — mismo mecanismo que
la entrada inicial, sin código nuevo para "volver".

`MACT_BACK` y `MACT_HOME` en Marea quedan iguales entre sí
(`metro_screen_list_pop_to_root()`, directo al hub): ya no existe una
página de pivotes intermedia útil que un solo `pop()` revelara — antes
de esta ronda un `pop()` simple sí tenía sentido (mostraba la fila
"Marea" seleccionable); ahora revelaría el mismo pivote 0 launcher, que
nunca debe verse.

**Verificación.** `test_nav`: 93 → **102 checks**, 0 fallos (9 nuevos de
`metro_nav_pivot_set()`). Capturas
(`docs/screenshots/ajustes-2/`): `f1-01-marea-directo.png` (Música →
Marea directo, sin pasar por ninguna lista), `f1-05-marea-right-quickplay.png`
(RIGHT desde Marea → "reproducir ya"), `f1-04-marea-left-wrap-playlists.png`
(LEFT desde Marea → "listas", el único punto de todo el esquema de
pivotes que envuelve), `f1-06-quickplay-left-marea.png` (LEFT desde
Quickplay → de vuelta a Marea). Documentado en
`docs/moonlit-design-system/componentes/marea.md` (secciones
"Navegación" y "Entrada").

## D-078 — Marquesina en el panel de Marea: título y subtítulo, desfasados medio ciclo

**Motivo.** Solo el título del panel de Marea usaba marquesina
(D-067); el artista/álbum se cortaba con `metro_draw_text_cut_right()`.
El maestro (§E.3) pide que los dos desplacen, pero con cuidado: si los
dos desbordan a la vez y barren juntos, compiten por la mirada y
ninguno se termina de leer con comodidad.

**Decisión.** `moonlit_marquee_draw()` gana una variante,
`moonlit_marquee_draw_offset()`, con un parámetro `phase_ms` que se
suma a `elapsed_ms` antes de calcular el desplazamiento — el reloj de
la ranura (`since`) no se toca, así que el efecto es "esta ranura
arranca su ciclo ya adelantado ese tanto", sin coordinarse con ninguna
otra ranura ni con la pantalla. `moonlit_marquee_draw()` pasa a ser
este mismo camino con `phase_ms=0`. Nueva ranura
`MOONLIT_MARQUEE_MAREA_SUBTITLE`. El panel de Marea
(`moonlit_screen_marea.c:draw_panel()`) dibuja el subtítulo con
`phase_ms = MAREA_SUBTITLE_MARQUEE_PHASE_MS` = 3 500 ms — literal
documentado (`(marquee_static_ms + marquee_scroll_ms) / 2`, mismo
patrón D-037 que `MAREA_SCROLL_ANIM_MS`, este archivo no incluye
`moonlit_tokens.h`, D-035). El conteo de canciones no cambia: sigue
sin marquesina, siempre cabe.

**El panel ahora también repinta en el tick ocioso.** Antes de esto no
existía ningún camino que repintara el panel de Marea mientras estaba
asentada y sin tapas pendientes — el título ya tenía marquesina desde
D-067 pero solo avanzaba de cuadro cuando coincidía por casualidad con
una carga de tapa. `moonlit_screen_marea_show_panel()` (nueva,
exportada) repinta SOLO el panel (`lcd_update_rect()`, nunca la banda
ni la cabecera, mismo nivel de finura que
`moonlit_screen_marea_show_carousel()` con la banda) — `metro_main.c`
la llama desde su rama ociosa cuando `moonlit_marquee_wants_ticks()`
es cierto y ni la animación del carrusel ni una tapa recién cargada ya
repintaron ese cuadro.

**Verificación.** `test_marquee`: 596 checks totales (`test_desfase_subtitulo()`,
nuevo, documenta con la matemática pura por qué las dos ranuras no
compiten: en `t=0` el título está quieto mientras el subtítulo, con el
mismo `since` pero desfasado, ya lleva 1 500 ms de barrido). Capturas
en dos tics distintos (`docs/screenshots/ajustes-2/`):
`f1-07-marea-marquee-tick-a.png`/`f1-08-marea-marquee-tick-b.png` —
título y subtítulo muestran tramos de texto distintos entre los dos
tics, y nunca coinciden en estar quietos o en movimiento al mismo
tiempo.

**Hallazgo aparte, fuera de esta fase.** Verificando estas capturas
con un álbum de prueba nuevo se encontró que `metro-test.aiff`
(`firmware/tools/gen_test_media.sh`) nunca tuvo tags de artista/álbum
—solo título— desde que existe: `ffmpeg` no los escribe al codificar a
AIFF con las opciones actuales del script (los otros cinco formatos del
mismo helper sí los llevan). Produce un álbum real "álbum desconocido"
de una sola pista en la biblioteca de prueba, que ordena primero
alfabéticamente. No es un bug de esta ronda ni de Marea — tagcache
etiqueta correctamente lo que de verdad no tiene tag — pero vale la
pena una pasada futura sobre `gen_test_media.sh` si estorba alguna
captura. Sin abrir una decisión propia por ahora: es un dato para la
siguiente sesión que toque fixtures, no un cambio de producto.

**PARADA 1 — Fase 1 cerrada.** Target (`rockbox.ipod` + bootloader) y
simulador compilan en 0 errores (mismos warnings preexistentes de
`-Wmissing-field-initializers` que ya arrastraban `tile_cols`/
`empty_message` en otros pivotes — `is_launcher` solo agrega una
instancia más de la misma clase, no una nueva). 19 suites de test
host, 0 fallos. `.bss`: **8 467 804 B** (+96 B sobre el cierre de la
ronda anterior — dos ranuras de marquesina nuevas,
`MOONLIT_MARQUEE_MAREA_SUBTITLE` y `MOONLIT_MARQUEE_HEADER`), techo
D-043 8 574 076 B, margen **106 272 B**. `make -C firmware/build-sim
install` corrido antes de las capturas. Capturas en
`docs/screenshots/ajustes-2/`. Sigue Fase 2 (ajustes compartidos).

## D-079 — Ajustes compartidos entre familias: `/.aura/settings.cfg`, contrato v19

**Motivo.** El maestro (§A) pide un archivo compartido en `/.aura/`
para los ajustes que Aura, Metro y moonlit ya exponen cada uno por su
lado (candado, brillo, retroiluminación, apagado automático, clicker,
límite de volumen, ajuste de volumen/replaygain, idioma, apariencia) —
hoy cambiar uno en una familia no se refleja al cambiar de sistema
(D-047) ni tras una sincronización con Aura Studio.

**Decisión — módulo puro, mismo vector de prueba, implementación
propia.** `moonlit_shared_settings.c/.h` (nuevo, cero dependencias de
Rockbox, mismo patrón que `metro_sync_marker.c`): parsea/serializa el
archivo texto-plano "clave: valor" del contrato (cabecera
`# aura-shared-settings v1` obligatoria y exacta -- sin ella el
archivo se rechaza entero, SS A.2.5), captura las 13 claves conocidas
en su forma literal (strings para los campos tipo-enum, `long` para
los numéricos) y **preserva verbatim** cualquier clave no reconocida
para la próxima escritura (`unknown_lines`, acotado a 512 B). Los
límites numéricos reales (`MAX_BRIGHTNESS_SETTING`, `sound_min/max`)
no los conoce este módulo -- los aplica el llamador
(`metro_settings.c`), que sí puede ver el target; el módulo solo
expone `moonlit_shared_settings_int_in_range()` para expresarlo. Tres
mapas palabra↔entero puros (`screen_lock_require`, `replaygain`,
`appearance`), en el mismo orden que los enums de moonlit para poder
castear directo. `metro_lang.c` gana `metro_lang_code_to_enum()`/
`_from_enum()` (código ISO 639-1 de dos letras) -- hoy solo
reconoce `es`/`en`; D-080 (Fase 3) agrega `fr`/`de`/`ru`/`it` a esta
misma tabla, no una nueva.

**`metro_settings.c` — aplicar y escribir.**
`metro_settings_apply_pending_shared()`: si `/.aura/settings.cfg`
existe, tiene la cabecera válida y su `rev` es mayor que
`metro_settings.shared_rev_applied` (nuevo campo, persistido en
`aura.cfg`), aplica las 13 claves -- `screen_lock_*`/`language`/
`appearance` a `metro_settings` (con `metro_theme_set()`/
`metro_lang_set()` en vivo), las otras seis a `global_settings`
directo (`brightness`, `backlight_timeout`, `idle_poweroff`,
`keyclick`, `volume_limit`, `replaygain_settings.type` -- este último
con su propio mapa 0/1/2↔`REPLAYGAIN_OFF/TRACK/ALBUM`, que **no**
coinciden en orden, D-071 ya evitaba exponer `REPLAYGAIN_SHUFFLE`) con
el flush de `save_global_settings_now()`. Una clave fuera de rango
(`brightness: 999`) se ignora sola; `language: fr` se reconoce como
clave *conocida* pero no cambia nada porque este build todavía no
soporta ese código (D-080 lo resuelve). `metro_settings_write_shared()`
hace el camino inverso: lee el archivo previo solo para heredar `rev`
y las líneas desconocidas, arma las 13 claves con el valor VIGENTE
ahora mismo (nunca con lo que decía el archivo viejo, SS A.2.3) y
reescribe con escritura atómica (`.tmp` + `rename()`, mismo patrón que
`moonlit_master_art_write()`). Se llama desde cada fila de Ajustes que
toca una de las 13 claves (idioma, brillo, retroiluminación, apagado
automático, clicker, límite de volumen, ajuste de volumen, apariencia,
candado activar/cambiar/quitar/cuándo pedir) y desde "restablecer
ajustes" -- que ahora también vuelve candado/retroiluminación/apagado
automático/clicker/replaygain a sus valores por defecto, algo que
D-071 había dejado fuera del botón.

**Dónde se aplica.** Mismo punto que la hora (`metro_settings_apply_
pending_clock()`): arranque y cada vuelta desde USB, dentro de
`metro_disk_handoff()` (`metro_main.c`).

**Hallazgo propio 1 — el candado se cobraba ANTES del archivo
compartido.** `metro_screen_lock_init()`/`_run_if_active()` corren
deliberadamente antes de `metro_disk_handoff()` (D-069: para que
"actualizando biblioteca" nunca tape el candado). Eso significa que un
`screen_lock_enabled: 0` que Aura Studio hubiera dejado escrito
mientras el aparato estaba **apagado** (conectado a la computadora sin
encender) se aplicaría demasiado tarde: el candado viejo, guardado
localmente la última vez que el aparato SÍ arrancó, se cobraría primero
y la salida de emergencia por USB del maestro (§A.2.6, "el PIN viaja en
claro... la salida de emergencia depende de que sea legible") quedaría
inalcanzable en ese arranque. Corregido llamando
`metro_settings_apply_pending_shared()` una vez más, en `metro_main()`,
antes de `metro_screen_lock_init()` -- idempotente (no hace nada si
`rev` ya está al día), así que el camino normal no cambia; solo importa
en el arranque en frío que sigue a una sincronización con el aparato
apagado. La llamada de siempre dentro de `metro_disk_handoff()` sigue
cubriendo la vuelta desde USB, que ya reevalúa el candado en la
siguiente vuelta del bucle principal (`metro_screen_lock_run_if_active()`
corre al principio de cada iteración).

**Hallazgo propio 2 — `keyclick` nunca sobrevivía un reinicio, desde
antes de esta ronda.** Verificando el vector A.3 con arranques REALES
(no solo mismo-sesión, que es como D-071 lo había probado) se encontró
que la fila "clicker" siempre volvía a "desactivado" al reiniciar, sin
importar quién la hubiera puesto en activado -- la UI local (D-071) o
esta ronda's ajustes compartidos. Causa: `metro_apply_hygiene()`
(`metro_main.c`, M-008, anterior a D-071) todavía forzaba
`global_settings.keyclick = 0` en **cada** arranque, con el comentario
"ninguno de estos ajustes está expuesto en la interfaz de Metro
(todavía)" -- cierto cuando se escribió, falso desde que D-071 agregó
la fila "clicker". Nadie lo notó porque D-071 solo se verificó
alternando el valor y mirándolo en la misma sesión, nunca contra un
reinicio real. Se quitó la línea (el valor por defecto de fábrica de
Rockbox para `keyclick` ya es 0, así que un aparato nuevo sigue
arrancando con el clic apagado -- solo deja de deshacer un valor que
alguien sí guardó a propósito). Las otras cinco claves de
`global_settings` (brillo, retroiluminación, apagado automático, límite
de volumen, replaygain) no estaban en la lista de `metro_apply_hygiene()`
y sí sobrevivían un reinicio de por sí -- confirmado con la misma
metodología de arranques reales antes de cerrar esta fase.

**Verificación.** `test_shared_settings` nuevo, **70 checks**: el
vector A.3 literal completo (13 claves parseadas exactas, `rev` 7,
`clave_futura` preservada y sobreviviendo un round-trip
parse→serialize→parse), el vector sin cabecera (archivo entero
rechazado, `out` sin tocar), `brightness: 999` (capturado pero fuera de
rango con el helper puro, el resto del archivo intacto), valores con
forma inválida ignorados clave por clave, serialize con buffer
insuficiente, los tres mapas palabra↔entero, PIN vacío. `test_lang`:
51 → **61 checks** (`metro_lang_code_to_enum()`/`_from_enum()`,
round-trip para cada idioma existente). Las 20 suites completas: **0
fallos**.

Capturas (`docs/screenshots/ajustes-2/`), las cuatro contra
**arranques reales** (simdisk limpio de `aura.cfg`/`config.cfg` antes
del primero, cada captura siguiente en un proceso nuevo del
simulador -- no la misma sesión repintándose): `f2-01-boot-shared.png`
(arranque con el vector A.3, tema pasa a claro de inmediato),
`f2-02-general.png`/`f2-03-general-scroll.png` (Ajustes › General:
idioma se queda en español porque "fr" no es soportado todavía,
apagado automático 20 min, clicker **activado** -- confirma el
hallazgo 2 corregido, ajuste de volumen por álbum), `f2-04-pantalla.png`
(Ajustes › Pantalla: tema amanecer, brillo 44 %, retroiluminación 10 s).

Target y bootloader compilan en 0 errores (mismos warnings
preexistentes de `-Wmissing-field-initializers`). `.bss`: **8 467 804
B**, sin cambio sobre el cierre de la Fase 1 -- el módulo nuevo vive en
`.text`/`.rodata`, no agrega estado estático propio (solo un `long`
más en `metro_settings_t`, absorbido por relleno de alineación ya
existente). Techo D-043 8 574 076 B, margen **106 272 B**.
`make -C firmware/build-sim install` corrido antes de cada captura.

**PARADA 2 — Fase 2 cerrada.** Sigue Fase 3 (idiomas y cirílico,
D-080/D-081).

## D-080 — Cuatro idiomas nuevos: francés, alemán, ruso, italiano

**Decisión.** `metro_lang.h` gana `METRO_LANG_FR/DE/RU/IT` (orden fijo,
`metro_lang_code_to_enum()`/`_from_enum()` y el selector de Ajustes lo
recorren igual). `metro_lang.c` gana `strings_fr/de/ru/it[LANG_COUNT]`
— las 137 cadenas completas, traducidas de cero (no máquina literal:
tono natural de cada idioma, revisado cadena por cadena contra el
significado real, no la forma de la frase en español/inglés). Reglas
del maestro (§D.1) aplicadas:
- Alemán: sustantivos (comunes y propios) con mayúscula inicial —
  ortografía real, no estilo — el resto sigue el mismo tono discreto
  que el resto de las tablas.
- Francés: el espacio fino antes de `?`/`:` se sustituye por un espacio
  normal (ninguna fuente de moonlit trae ese glifo, y agregar una
  fuente aparte solo para eso no vale la pena).
- Ruso: sin equivalente de pluralización en el motor de cadenas
  (`%d` es un entero puro) — `LANG_MAREA_SONGS_FMT` usa el genitivo
  plural ("%d песен"), la aproximación más común cuando no hay reglas
  de plural por cantidad; no es gramaticalmente exacto para N=1..4,
  limitación conocida y no nueva de este cambio (el mismo `%d` sin
  plurales ya existía para inglés "song(s)").
- `metro_lang_str()` pasa de un ternario ES/EN a `lang_tables[current_lang][id]`
  — un arreglo de punteros a tabla, para que agregar un idioma sea
  agregar una fila y una tabla, no anidar otro ternario.

**Selector con nombre nativo (maestro §D.2).** `metro_lang_native_name()`
(nueva) devuelve el nombre de un idioma EN ESE IDIOMA siempre —
"Español", "English", "Français", "Deutsch", "Русский", "Italiano" —
sin pasar por `current_lang`: una interfaz en alemán muestra "Русский"
tal cual, no una traducción de "ruso". La fila "idioma" en Ajustes
(`metro_screen_settings.c`) ahora la usa en vez de
`LANG_VALUE_SPANISH`/`LANG_VALUE_ENGLISH` (que quedan en las tablas,
sin uso — retirarlas movería el valor numérico de todo lo que viene
después en el enum sin necesidad real). `SELECT` recorre los seis en
orden y envuelve, en vez del toggle ES↔EN de antes.

**`metro_splash_lang.c` también a seis idiomas.** Las ocho reglas que
retraducen los mensajes de arranque de Rockbox (Metro no usa el
sistema de `.lang` de Rockbox, M-009) — "Cargando...", "Preparando el
disco...", batería baja/agotada, etc. — se habrían quedado en
español/inglés nada más si no se tocaban: un usuario ruso habría visto
"Preparando el disco..." en español en plena pantalla de arranque.
`splash_rule_t.es/en` se vuelve `tr[METRO_LANG_COUNT]`, indexado igual
que `metro_lang.c`.

**Verificación.** `test_lang`: 61 → **948 checks** —
`test_no_string_missing_in_any_language()` recorre las 137 claves × 6
idiomas (822 checks) confirmando que NINGUNA quedó sin traducir (un
hueco en un inicializador designado de arreglo no avisa en la
compilación, C99 permite arreglos dispersos — se habría visto como una
fila en blanco recién en el dispositivo); `test_six_languages_resolve()`
confirma que los formatos (`%d`/`%s`) sobreviven la traducción en los
seis; `test_native_names()` confirma que el nombre nativo no se mueve
con `current_lang`. Capturas en `docs/screenshots/ajustes-2/`
(`f3-01-hub-ruso.png`, `f3-02-acerca-de-ruso.png`) — ver D-081, la
verificación visual real depende de la fuente cirílica.

## D-081 — Cirílico: fuente aparte por rol, alfabeto ruso completo

**Decisión — rango, no el bloque Unicode completo.** `1025-1105`
(Ё U+0401, А-я U+0410-U+044F, ё U+0451: el alfabeto ruso completo, 66
letras) en vez de `1024-1279` (el bloque "Cyrillic" entero, 256
códigos) — medido por Metro sobre el mismo par de fuentes para su
propio porte: el bloque completo en cinco roles pesaba 280 KB, mucho
para alfabetos (ucraniano, serbio) que moonlit no ofrece todavía.
Ucraniano/serbio quedan FUERA a propósito; una ronda futura que agregue
esos idiomas amplía este rango, mide de nuevo, y ya.

**Fuente aparte, los SIETE roles.** A diferencia de la de puntuación
(D-074, excluye `MFONT_DISPLAY`), la cirílica se genera para los siete
— `MFONT_DISPLAY` dibuja nombres de pivote del hub, y en ruso son
cirílicos de punta a punta ("música" → "музыка"). Libre Baskerville
(display/title/headline) no trae cirílico: esos tres roles sustituyen
su cara por Montserrat Regular, al mismo tamaño de píxel, solo para
este archivo — Montserrat sí trae el alfabeto completo, verificado
(`CYRILLIC_REQUIRED_CODEPOINTS`, `design-system/generate.py`: el build
se detiene si a algún rol le falta una sola de las 66 letras — a
diferencia de la puntuación, aquí NO hay intersección: un alfabeto
incompleto no es aceptable en ningún rol). Los otros cuatro roles
(ya Montserrat) usan su misma cara, solo cambia el rango de códigos.
`design-system/generate.py --fonts` genera `moonlit-<rol>-<px>-cyr.fnt`
junto a la primaria y la de puntuación de siempre.

**Costo medido, no supuesto — y por qué `display` se queda.** Disco:
37 261 (display-40) + 18 597 (title-28) + 11 167 (headline-22) +
10 003 (list-20) + 10 763 (listsel-20) + 7 971 (body-18) + 8 131
(label-18) = **103 893 B** para los siete. RAM: medida con
`font_get(id)->buffer_size` en el simulador tras cargar cada una — 
**idéntica, byte a byte**, a su tamaño en disco en los siete casos: con
presupuesto de 96 glifos (`MOONLIT_CYRILLIC_GLYPH_BUDGET`) contra 81
entradas reales, `font_load_ex()` (`firmware/font.c`) cae en la rama
SIN caché (el archivo entero a buflib), confirmado leyendo la condición
`bufsize < file_size` del propio `font.c`, no solo midiendo. `display-40`
pesa 37 261/103 893 = **35.9 %** del total — Metro, con Inter en vez de
Montserrat, midió que su `display-48` pasaba el 52 % y lo dejó fuera
(los nombres de pivote en ruso caen a `title` ahí); moonlit no cruza
esa mitad, así que `display` se queda con cirílico propio, sin excepción
que documentar en `moonlit_fonts.c`.

**`moonlit_textseg.c` — tercera clase de tramo.** `MOONLIT_TEXTSEG_CYRILLIC`,
nuevo parámetro `has_cyrillic_font` en `moonlit_textseg_build()`. Un
codepoint en 1025-1105 con `has_cyrillic_font` en true es tramo
CYRILLIC, bytes tal cual (nunca transliterado — no hay ASCII razonable
para "я"). Encontrado al escribir esto: el atajo de D-074 ("sin fuente
de puntuación, un solo tramo PRIMARY transliterado entero") asumía que
`!has_punct_font` significa "este rol no tiene nada aparte" — cierto
hasta ahora, falso para `MFONT_DISPLAY` desde este momento (tiene
cirílico pero no puntuación). Corregido a
`!has_punct_font && !has_cyrillic_font`; sin la corrección, los nombres
de pivote en ruso se habrían transliterado (a nada, sin reemplazo) y
mostrado el defaultchar `·` en vez del texto real. `metro_draw.c` (único
punto de dibujo de texto, M-051) pasa `metro_font_has_cyrillic(role)` y
resuelve `MOONLIT_TEXTSEG_CYRILLIC` a `metro_font_cyrillic_id(role)` —
`metro_draw_text_width()` usa el mismo camino, así que mide cirílico
correcto sin cambio propio.

**`MAXUSERFONTS` 16 → 24.** 7 primarias + 6 puntuación + 7 cirílicas =
20 fuentes, con el mismo margen de 4 que D-074 dejó. Encontrado a la
mala (ver MODIFICATIONS.md): con 16 todavía, `moonlit_fonts.c` cargaba
`body`/`label` cirílicas como "failed to load" — silencioso,
`metro_font_cyrillic_id()` cae al id primario sin caerse, así que
invisible sin medir. `firmware/tools/sim_shot.sh` ahora falla
(`exit 1`, imprime la línea) si el log del simulador contiene
"failed to load" — cualquier captura de aquí en adelante es también,
gratis, una verificación de que las 20 fuentes cargaron.

**`check_fonts.py --coverage` extendido, sin romper lo que ya hacía.**
El chequeo agrupaba TODOS los `.fnt` bajo un mismo criterio ("cubre la
UI completa") — con las fuentes de puntuación (D-074) y ahora las
cirílicas conviviendo en el mismo directorio, eso reportaba cientos de
"faltantes" falsos (una fuente de puntuación, rango 8208+, nunca tuvo
que cubrir la UI general) y el propio chequeo nunca corrió limpio desde
D-074 hasta ahora. `_font_category()` clasifica por sufijo de archivo
(`-punct`/`-cyr`/primaria) y cada categoría se mide contra SU universo:
primaria contra la UI menos el rango cirílico (D-081 es trabajo de la
`-cyr`, no de esta), cirílica contra los codepoints cirílicos que la UI
de verdad usa (extraídos de `metro_lang.c`, las seis tablas a la vez —
`lang_codepoints()` ya recorría el archivo entero, sin cambio), y
puntuación solo informa metadatos (su cobertura real la decide la
intersección que `generate.py` ya calcula). Corrido limpio: los siete
roles primarios "UI: completa", los siete cirílicos "cirílico:
completo".

**`gen_test_media.sh`.** Álbum cirílico ("Лунный Свет" / "Ночная
Симфония", sin carátula a propósito — el monograma de Marea también
dibuja la inicial cirílica) y álbum alemán con ß/ü ("Käfer & Größe" /
"Weiße Straße") — este último ya en el rango primario 32-383 (D-007),
prueba de que NO necesita fuente aparte ni transliteración.

**Verificación.** `test_textseg`: 45 → **57 checks** (texto ruso puro
un solo tramo, ruso y latín alternando con fusión de tramos contiguos,
sin fuente cirílica no clasifica CYRILLIC, `MFONT_DISPLAY` sin
puntuación pero con cirílico). Las 20 suites: **0 fallos**. Target y
bootloader compilan en 0 errores. `.bss`: **8 467 964 B** (+160 B sobre
el cierre de la Fase 2 — ocho ranuras más de `MAXUSERFONTS`, mismo
orden de magnitud que los +96 B que D-074 midió para cuatro), techo
D-043 8 574 076 B, margen **106 112 B**.

Capturas contra arranques reales, mismo criterio de D-079
(`docs/screenshots/ajustes-2/`): `f3-01-hub-ruso.png` (hub y Ajustes ›
General en ruso, "язык: Рус[ский]"), `f3-02-acerca-de-ruso.png`
("Acerca de" en ruso, cirílico en `MFONT_LIST_SEL`/`MFONT_BODY`/
`MFONT_LABEL`), `f3-03-marea-cirilico.png` (Marea con el álbum ruso —
título en `MFONT_HEADLINE` desbordando en marquesina, monograma con la
inicial cirílica "Н" en vez de un placeholder o `?`), y una captura
intermedia del álbum alemán confirmando ß/ü sin fuente aparte.

**PARADA 3 — Fase 3 cerrada.** Sigue Fase 4 (visor de fotos, porta el
diff de Metro M-109 cuando exista).

### D-081, addendum — el dibujo por tramos también manda al MEDIR (mismo hallazgo que Metro M-116)

**Encargo**: la supervisora reporta M-116 de Metro — `metro_draw_tile()`
dibujaba la inicial del mosaico con `lcd_setfont()` + `lcd_putsxy()` a
pelo, saltándose el despacho por tramos, así que una inicial cirílica
salía como el defaultchar aunque el rótulo de abajo se leyera bien — y
pide revisar lo mismo aquí, más el monograma de Marea (D-065).

**Reproducido, no supuesto.** Cuadrícula de Álbumes con el álbum ruso
de las fixtures (`Ночная Симфония`, sin carátula, mosaico de respaldo):
la inicial salía **`·`** mientras el rótulo decía "Ночная Симфония" en
cirílico correcto — `f5-01-tile-cirilico-antes.png`. Es el mismo bug de
Metro, línea por línea: `metro_draw.c` era el ÚNICO `lcd_putsxy()` de
`apps/metro/` fuera de las dos funciones de dibujo por tramos que lo
tienen permitido.

**Y un segundo defecto, del mismo origen, que Metro no reportó: medir.**
El monograma de Marea sí **dibujaba** por tramos (`metro_draw_text()`,
por eso la captura `f3-03` de la Fase 3 mostraba la "Н" bien) pero
**medía** con `lcd_setfont(metro_font_id(rol))` + `lcd_getstringsize()`
— es decir, contra la fuente PRIMARIA del rol, que para un codepoint
cirílico devuelve el ancho del defaultchar. El glifo salía bien y
descentrado. El comentario de `metro_draw_text_width()` (D-066/D-067/
D-074) ya decía exactamente por qué eso está mal —"quien centra o decide
si hace falta marquesina se equivocaría por esa diferencia"— pero nueve
sitios seguían midiendo a mano; D-074 arregló el ancho y nadie barrió a
los llamadores.

**`metro_draw_text_size(role, str, &w, &h)`** (nuevo, `metro_draw.c`):
la forma completa de `metro_draw_text_width()` — recorre los mismos
tramos, suma anchos y toma el alto MAYOR (una cadena mixta se dibuja con
dos fuentes y su caja es la del tramo más alto). `metro_draw_text_width()`
pasa a delegar en él, sin duplicar el recorrido.

**Convertidos** (los que miden cadenas que HOY pueden traer cirílico —
dato de la biblioteca o cadena traducida de las seis lenguas de D-080):
mosaico (`metro_draw.c`, el del glifo equivocado), monograma de Marea
(`moonlit_screen_marea.c`), monograma de Ahora suena
(`metro_screen_nowplaying.c`), ciclo de la marquesina del hub
(`metro_screen_hub.c` — con un título cirílico el `span` salía corto y
las dos copias se encimaban), pregunta y mensaje vacío de los widgets
(`metro_widgets.c`), pantalla de biblioteca (`moonlit_screen_library.c`),
mensaje centrado de `metro_main.c`, caja de borrado de CONTINUUM
(`metro_transitions.c`) y las dos leyendas del visor de fotos
(`metro_screen_photo_viewer.c`, D-082 — un nombre de archivo cirílico).
Se dejan como estaban los que miden ASCII por construcción: un dígito
del candado, `"Ag"` para altura de línea, y el tiempo total formateado.

**Verificación.** `f5-02-tile-cirilico-despues.png`: mismo recorrido,
misma posición de la cuadrícula, la inicial **"Н"** dibujada con la
fuente cirílica de `MFONT_DISPLAY` (la que D-081 ya generaba desde la
Fase 3 — lo único que faltaba era pasar por el despachador que la
elige). Las 20 suites de host: **0 fallos**. Target y simulador: 0
errores, sin warnings nuevos. `.bss` **sin cambio** (8 484 604 B,
margen 89 472 B); `text` +32 B. `stack_report.py`: OK, 5528 B (45.0 %).

## D-082 — Visor de fotos responsivo: la misma fila de REPEAT que faltaba en Metro, más debounce

**Causa raíz, citada por comparación directa de código (no repetida
desde cero -- ya la diagnosticó Metro sobre el mismo código heredado,
M-109, commit `c9f8dbb` de `../Metro-Aura`, solo lectura).**
`viewer_mapping[]` (`metro_keymap.c`) nunca tuvo la fila `BUTTON_SCROLL_
{FWD,BACK} | BUTTON_REPEAT` que `hub_mapping[]`, `list_mapping[]`,
`player_mapping[]` y `lock_mapping[]` sí tienen desde antes. Un giro
CONTINUO de la rueda hace que el driver (`firmware/drivers/button.c` +
`button-clickwheel.c`) reporte el MISMO código con el bit `REPEAT`
puesto en cuanto pasan 300 ms -- indistinguible, para el driver, de un
botón sostenido. Sin esa fila, `action_code_worker()` (`apps/action.c`)
no encuentra el código con `REPEAT` puesto y, como la tabla termina en
`LAST_ITEM_IN_LIST` (no la variante `__NEXTLIST`), la acción caía en
`MACT_NONE` y se descartaba en silencio: un giro continuo solo avanzaba
una foto, la del primer evento, hasta soltar la rueda y volver a girar.

**Diagnóstico dinámico, reproducido.** El intento inicial con
`SCROLL_FWD×5` sin `WAIT` no sirvió: los tokens de inyección de
`uisimulator/common/sim_tasks.c` generaban un ciclo PRESS+REL discreto
por cada `SCROLL_FWD` -- no sintetizaban un `BUTTON_REPEAT` de verdad,
que depende de que el mismo código se siga reportando entre sondeos sin
soltarse (`REPEAT_START`, 300 ms). Se cerró el hueco portando el
mecanismo que Metro ya tenía para esto (M-101, `../Metro-Aura`, solo
lectura): un sufijo `+HOLD` sobre cualquier botón de `METRO_SIM_BUTTONS`
(p.ej. `SCROLL_FWD+HOLD`) hace que el injector postee press ->
`BUTTON_REPEAT` -> release en vez de solo press+release -- exactamente
la secuencia que produce el driver real al pasar el umbral. Portado a
la arquitectura propia de `sim_tasks.c` (`inject_phase` de 3 estados en
vez del `inject_release_pending` booleano que había antes, `inject_hold[]`
paralelo a `inject_codes[]`); comentario inline `moonlit (D-082)`,
entrada nueva en `MODIFICATIONS.md`.

Con el token, la reproducción real: desde el arranque, `SCROLL_FWD,
SCROLL_FWD, SELECT, SELECT` entra al visor en `beach.jpg` (índice 0 de
"Todas"); una vez asentado, un solo `SCROLL_FWD+HOLD` es el burst a
probar. **Revirtiendo solo las dos filas REPEAT** (comentadas
temporalmente, reconstruido, sin tocar nada más): el burst deja el
visor en `diagram.jpg`, **"2 / 16"** -- únicamente el press inicial
cuenta; el REPEAT se descarta en silencio, la rueda "sostenida" se
comporta como una sola pulsada. **Con la fila restaurada** (mismo
burst, mismo build salvo esa línea): el visor llega a `dreamscape.jpg`,
**"3 / 16"** -- press + REPEAT cuentan los dos, que es el
comportamiento correcto de un giro continuo. Capturas:
`f4-03-repro-m109-sin-repeat.png` (base, bug reproducido) y
`f4-04-repro-m109-con-repeat.png` (con la fila, corregido) en
`docs/screenshots/ajustes-2/`. Mismo criterio que usó Metro para cerrar
M-109: comparar el destino alcanzado por el mismo burst con y sin la
fila, no un MD5 (aquí más simple de leer: el nombre de archivo y el
contador "n / total" que el propio `draw_preview_caption()` ya pinta
durante el scrub bastan como evidencia, sin instrumentación aparte).

**Decisión — la fila, más el debounce del maestro §C.2, portado desde
Metro adaptado a la arquitectura propia.** `metro_keymap.c`:
`viewer_mapping[]` gana las dos filas REPEAT. moonlit ya tenía su
PROPIO mecanismo de deslizamiento entre fotos (`s_slide_dir`, D-072,
consumido DENTRO de `metro_screen_photo_viewer_show()`) -- distinto de
la arquitectura de Metro (que externaliza la dirección vía
`metro_screen_photo_viewer_take_slide()`, llamado desde `metro_main.c`,
porque M-106/M-109 en Metro sí tienen esa capa); el puerto respeta la
forma de moonlit en vez de traer la de Metro entera: nuevo
`s_nav_tick` (reloj de "último cambio de índice"), `METRO_PHOTO_
SETTLE_TICKS` (150 ms), y `metro_screen_photo_viewer_wants_ticks()`
(true mientras siga en la ventana de quietud, o mientras la foto
asentada todavía no esté decodificada). `handle()` sigue moviendo
`s_index` y armando `s_slide_dir` exactamente como antes (D-072) --
solo agrega reiniciar `s_nav_tick` bajo el mismo guard de "el índice
de verdad cambió" que ya protegía el deslizamiento. `show()` revisa la
ventana de quietud ANTES de tocar `s_slide_dir`: dentro de ella dibuja
`draw_scrub_preview()` (vista previa barata) y no consume nada; fuera
de ella, cae al camino de siempre sin cambios. El resultado: un giro
rápido reinicia el reloj en cada paso (barato, sin decodificar), y
recién cuando la rueda se detiene 150 ms el redibujo asentado
decodifica una vez y, si corresponde, desliza -- un solo decode por
gesto, no uno por paso.

**`draw_scrub_preview()`.** Lee la maestra compartida de 80 px de la
foto destino (`moonlit_art_master_file_path('p', "photos", ...)` +
`moonlit_master_art_read()`, LA MISMA que ya llena la cuadrícula desde
D-072 -- nunca decodifica un JPEG) y la amplía a 240×240 con
`draw_scaled_centered()` (la misma primitiva que ya dibuja el modo
"cubrir" de la foto completa, sin una segunda rutina de escalado). Sin
maestra todavía (constructor en segundo plano no llegó a esta foto) se
ve solo el nombre y la posición sobre el fondo limpio -- preferible a
forzar un decode que reintroduciría el bloqueo que este mecanismo
existe para evitar. A diferencia de Metro (que toma un lock sobre un
buffer compartido con su hilo constructor, `metro_master_art_lock()`),
la API de moonlit (`moonlit_master_art_read()`) escribe directo al
buffer del LLAMADOR sin necesitar lock -- `metro_thumbs.c` ya lee la
misma maestra así, sin lock, para la cuadrícula (D-072); se sigue ese
mismo criterio en vez de inventar uno nuevo. `MFONT_CAPTION` de Metro
no existe en moonlit (siete roles, no ocho) -- la leyenda usa
`MFONT_LABEL`, el rol que ya cubre subtítulos/valores cortos en el
resto de la interfaz.

**`metro_main.c`.** Mismos dos puntos de enganche que el resto de las
puertas de energía del repo (D-053/D-057/D-067): la espera de entrada
baja a `HZ/20` mientras `at_viewer && metro_screen_photo_viewer_wants_
ticks()`, y la rama ociosa llama `redraw_current()` bajo la misma
condición -- así el debounce vence solo por el paso del reloj, sin
necesitar que llegue un botón nuevo, exactamente igual que la
marquesina (D-067) o Marea (D-057) ya hacían para sus propias
ventanas de tiempo.

**Costo.** `.bss`: **8 480 764 B** (+12 800 B sobre el cierre de la
Fase 3 -- `s_preview_master[80×80]`, un `fb_data` por pixel, medido
exacto: 80×80×2 = 12 800 en este target de 16 bpp). Techo D-043
8 574 076 B, margen **93 312 B**.

**Verificación.** Target y bootloader compilan en 0 errores (mismo
warning preexistente de `-Wmissing-field-initializers`). Las 20 suites
de host: **0 fallos** (este módulo no es host-testable, depende de
Rockbox de punta a punta -- `jpeg_load.h`, `bmp.h`, `lcd.h`). Capturas
en `docs/screenshots/ajustes-2/`: `f4-01-visor-scrubbing.png` (a los
30 ms del último evento de rueda -- vista previa ampliada de la
maestra de 80 px, "dreamscape.jpg" y "3 / 16" sobre la franja opaca al
pie, sin decodificar nada), `f4-02-visor-asentado.png` (mismo destino,
500 ms después -- la foto completa decodificada, pantalla completa,
sin la franja de leyenda), `f4-03-repro-m109-sin-repeat.png` /
`f4-04-repro-m109-con-repeat.png` (reproducción del bug M-109 y su
cierre, ver diagnóstico dinámico arriba).

Suite de host completa tras el puerto del sufijo `+HOLD` a
`sim_tasks.c`: 20 suites, **0 fallos** (ninguna de las nuevas líneas es
host-testable -- viven en el simulador, guardadas por `SIMULATOR`).

**PARADA 4 — Fase 4 cerrada.** Sigue Fase 5 (cierre de la ronda).

## Fase 5 — Cierre

**`stack_report.py`, una FALLA real cazada por la propia verificación
mecánica del cierre.** Sobre el build limpio: `metro_settings_write_
shared()` (D-079) medía **2240 B** de marco y `metro_settings_apply_
pending_shared()` **1648 B** -- las dos muy por encima del tope de
1024 B de `apps/metro/`, sin que nada de las Fases 1-4 lo hubiera
notado (ninguna las ejercita en un camino que `stack_report.py` marque
como el peor; el reporte solo se había corrido para medir `.bss`, no
releído su veredicto completo hasta ahora). Causa: `write_shared()`
tenía DOS `moonlit_shared_settings_t` completos en la pila a la vez
(`s` y `old`) más el buffer de 1024 B; `apply_pending_shared()` uno
solo. Corregido con el mismo idioma que ya usa este archivo y
`metro_sync.c` (`write_marker()`) para el mismo problema -- estáticos en
vez de automáticos, seguro porque los cuatro puntos de llamada son
siempre el hilo de UI, nunca reentrante ni concurrente entre sí.
Commit `39371977`. `stack_report.py` tras la corrección: **OK, peor
camino 5528 B (45.0 % de 12288 B)**, sin cambio respecto a D-075 -- las
dos funciones movidas no estaban en ese camino tampoco.

**`.bss`.** Sobre el build limpio final: **8 484 604 B** (+3840 B sobre
el cierre de Fase 4 -- los dos `moonlit_shared_settings_t` que pasaron
de pila a `.bss`). Techo D-043 8 574 076 B, margen **89 472 B**.

**Reproducibilidad (mismo criterio de D-075).** `BUILD_TARGET_CLEAN=1`
contra el incremental de esta misma sesión, mismo commit (`39371977`):
`cmp` de los dos `rockbox.bin` -- **idénticos byte a byte**. `text`/
`data`/`bss` idénticos: `1 274 684 / 12 308 / 8 484 604`.

**`package_dist.sh` sin `--release-tag`.** Corrida completa (árbol con
`.serena/` sin trackear -- de siempre, no bloquea sin el flag, solo
agrega la `M` de árbol sucio a `rockbox-info.txt`, esperado en
desarrollo). Los 7 assets del contrato: `rockbox.ipod`, `rockbox.zip`,
`bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`,
`MODIFICATIONS.md`, `THIRD-PARTY-NOTICES.txt`. 419 archivos en el árbol
`.rockbox/` armado, centinelas verificados. `stack_report.py` corre
DENTRO del script (antes de `make zip`) y sale en verde. Sin
`.rockbox/aura/version.txt` dentro de `rockbox.zip` (correcto, sin
`--release-tag`; verificado con `unzip -l | grep aura`, no supuesto).

**`CONTRATO-moonlit-studio.md`.** Sube a **Versión 7**: referencia al
contrato canónico v19 (Aura-Firmware, leído en paralelo), §A.12 nueva
para `/.aura/settings.cfg` (consumo/escritura desde el lado de moonlit,
puntos exactos de aplicación en el handoff de disco y antes del
candado), fila de la tabla de contratos corregida de v16 a v19 (estaba
desactualizada desde antes de esta ronda -- el propio changelog del
archivo ya citaba v18 en Versión 5, la tabla nunca se había puesto al
día; se corrigió de paso al tocar la fila para v19, no una auditoría
completa del archivo).

### Lista de verificación en hardware — ajustes 2

Ninguno de estos puntos se puede confirmar en el simulador -- verifica
geometría, máquinas de estado y ausencia de regresión; lo que sigue
solo se prueba en el iPod real. Agrupado por decisión.

**Barra de estado y entrada directa a Marea (D-076/D-077/D-078)**
- [ ] Un título de pista/álbum largo en la barra nunca choca con el
  reloj, la batería, el icono de transporte ni el de candado (D-076);
  la marquesina del título corre dentro de su franja delimitada.
- [ ] Entrar a Música desde el hub va directo al panel de Marea (sin
  pasar por una lista intermedia), y MENU/`MACT_BACK` desde Marea
  vuelve directo a la raíz del hub, no a un nivel intermedio (D-077).
- [ ] Dentro de Marea, LEFT/RIGHT recorren los pivotes (Reproducción
  rápida, Listas) sin volver a envolver al llegar a la punta (D-077).
- [ ] Título y artista largos en el panel de Marea se desplazan cada
  uno con su propia marquesina, desfasadas entre sí (D-078) -- no
  arrancan ni cambian de dirección al mismo tiempo.

**Ajustes compartidos entre familias (D-079, contrato v19)**
- [ ] Cambiar brillo, apagado automático, volumen máximo o bloqueo en
  Metro (o Aura) y cambiar de sistema a moonlit: mismos valores, mismo
  comportamiento del candado -- y a la inversa, cambiando desde
  moonlit hacia las otras.
- [ ] Cambiar idioma en una familia -- las otras arrancan ya en ese
  idioma la próxima vez que se entre a ellas.
- [ ] Restablecer ajustes en moonlit escribe `/.aura/settings.cfg` con
  los valores por defecto y un `rev` nuevo; las otras familias los
  recogen al arrancar.
- [ ] Apagar el aparato con `screen_lock_enabled: 0` recién escrito por
  Studio (USB) y encenderlo de nuevo: el candado NO se activa (ruta de
  emergencia, D.6 del contrato canónico) -- confirma el segundo punto
  de aplicación antes de `metro_screen_lock_init()`.

**Sincronización de hora (contrato v19, D.4)**
- [ ] Sincronizar biblioteca desde Studio con moonlit conectado: la
  hora del iPod queda igual a la del Mac al terminar, sin reiniciar el
  aparato -- incluida una sincronización que no toca ningún archivo
  (biblioteca ya al día).

**Seis idiomas y cirílico (D-080/D-081)**
- [ ] Recorrer Ajustes, Ahora suena, el candado y "Acerca de" en ruso y
  en alemán completos, sin cortes de texto ni caer en `·`/`?` donde no
  debería.
- [ ] Un álbum con título/artista en cirílico se lee bien en la lista
  de música, en Marea y en Ahora suena -- glifos reales, no cuadros ni
  el `·` de transliteración.
- [ ] Un álbum con `ß`/`ü`/`ö`/`Ä` (alemán) se lee bien en los mismos
  tres lugares.
- [ ] La INICIAL de un álbum/artista cirílico sin carátula sale como
  letra cirílica en el mosaico de la cuadrícula, en el monograma de
  Marea y en el de Ahora suena -- no como `·` ni descentrada (D-081,
  addendum; el mosaico está verificado en simulador, falta confirmar
  que en el aparato no cambie por la fuente cargada de disco).

**Visor de fotos responsivo (D-082)**
- [ ] Girar la rueda sin levantar el dedo recorre varias fotos
  seguidas, no solo una -- la corrección real de M-109 en hardware, no
  solo en el simulador con el token `+HOLD` inyectado.
- [ ] Girar rápido de foto en foto se siente fluido -- solo un
  destello de la maestra de baja resolución por cada una, sin que el
  aparato se trabe intentando decodificar cada JPEG intermedio.
- [ ] La foto completa (decodificada) aparece al detenerse la rueda,
  dentro de una fracción de segundo perceptible como inmediata.

### Cierre de la ronda

Siete decisiones cerradas (D-076 a D-082, sin contar addenda): barra de
estado delimitada y entrada directa a Marea, marquesina doble
desfasada, ajustes compartidos entre las tres familias (contrato v19),
seis idiomas con soporte cirílico completo (más su addendum: el
despacho por tramos también manda al MEDIR, no solo al dibujar --
mismo hallazgo que Metro M-116, ampliado a los nueve sitios que medían
a mano), y visor de fotos responsivo con debounce -- este último con un
hallazgo propio de Fase 5
(la FALLA de `stack_report.py` en el módulo de ajustes compartidos de
D-079, corregida en la misma pasada) y una reproducción dinámica real
del bug M-109 después de portar el sufijo `+HOLD` de Metro a
`sim_tasks.c`, que sustituyó la limitación honesta anotada originalmente
en D-082.

Tag sugerido para cuando el dueño termine la lista de hardware:
**`v0.2.1`**.

**Publicado el 2026-09-04 con autorización directa del dueño** (pedida
en la terminal de esta sesión, no por relevo de la supervisora -- un
tag y un Release público son irreversibles y hacia afuera). Tag
`v0.2.1` sobre `c21674f4`, árbol limpio (`.serena/` apartado durante el
empaquetado y devuelto después, nunca comiteado ni agregado a
`.gitignore`), `package_dist.sh --release-tag v0.2.1` con build limpio,
`make dep` y `stack_report.py` en verde dentro del script.
`rockbox.zip` verificado antes de publicar: `version.txt` = `v0.2.1`,
nada más bajo `aura/` (sin fantasmas de corridas anteriores, D-075),
las 7 fuentes cirílicas, las 6 de puntuación y las 7 primarias
presentes. Los 7 assets exactos del contrato, sin `README.md`.

<https://github.com/Ricolinos/moonlit-aura/releases/tag/v0.2.1>

```
checksums.txt del release v0.2.1 (BOOT-1 sigue vigente: fuentes de bootloader y
mks5lboot sin cambios; el SHA del bootloader cambia solo por el RBVERSION embebido):
  3caef31def72184501999d7d4b7d9bcd64bf812c56c766bc9d785bf1cc3d004a  rockbox.zip
  47c96676c3af37b6a6d213dc5b8b0a2457401d7ae6f58eea95bf9c903ef3d251  rockbox.ipod
  035e14db2be7ce093a434a2dac8fff419c64a8be84644122710e2a1850fc3a91  mks5lboot
  252e90c0a0029280fe09731d33724e5ee9153961003b07c2344e957ece15a892  bootloader-ipod6g.ipod
```

La lista de verificación en hardware de arriba sigue **pendiente**: el
Release se publicó antes de recorrerla, por decisión del dueño. Llega
al iPod cuando Aura-Studio actualice su pin (`FIRMWARE_VERSION`), no
desde este repo.

**Nota para la próxima ronda que toque `.bss`.** El margen bajo el
techo D-043 quedó en **89 472 B**, el más bajo de los cerrados hasta
hoy -- esta ronda sola se comió unos 16 KB (12 800 B del preview de
scrubbing del visor, D-082, más los estáticos de D-079 addendum). Una
ronda de un idioma con alfabeto grande y sin mapeo 1:1 a Unicode básico
(japonés, con caché de glifos en vez de un `.fnt` denso como el
cirílico de D-081) necesita presupuestar ese costo ANTES de
implementar, no medirlo al final: a este ritmo de consumo, dos rondas
más de este tamaño agotan el margen actual.
