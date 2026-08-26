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

**D-004 — Familias (cierra P1).** Títulos: **Libre Baskerville** (SIL OFL 1.1). Texto y UI: **Montserrat** (SIL OFL 1.1, build **estática** `Montserrat-Regular.ttf` de github.com/JulietaUla/Montserrat — nunca la variable de Google Fonts, por el error de instancia peso-100 documentado en §1.6). Evidencia: x-height 0,530 vs 0,526 em (refuta H-T1), tamaños reales 9,6–34,8 KB por fuente (§1.2).

**D-005 — Tamaño mínimo de rol.** Ningún rol de texto legible baja de **18 px nominal** (≈11,5 px cap-height proyectado, sobre el mínimo ISO 9241-303 de 10,5 px). El rol `caption` heredado a 14 px (9 px cap-height real medido, §2) **no se hereda**. Tamaños exactos por rol los fija el plan (sección C) dentro de esta regla.

**D-006 — Umbral de trazo visible (cierra P9).** En la verificación mecánica de `.fnt`/capturas, cuenta como trazo un píxel con luminancia **> 60/255**, medido sobre captura del simulador con PIL — el método ya calibrado en el estudio de legibilidad.

**D-007 — Slots de fuente y bug de rango (cierra P11/P12).** La jerarquía tipográfica se diseña con **≤ 12 roles** para no tocar `MAXUSERFONTS` (`firmware/export/font.h:51`). Solo si el plan demuestra que 12 no bastan se sube el define, con auditoría previa de buflib. `gen_fonts.sh` de moonlit corrige el bug `atoi("0x20")` (§1.1) usando rango **decimal 32–383**. La corrección en Metro-Aura y Aura-Firmware es trabajo de sesión propia por repo: el plan la lista como prompt aparte etiquetado `[Metro-Aura]` / `[Aura-Firmware]`, no la ejecuta.

## Iconos, temas y tokens

**D-008 — Iconos binarios compilados (cierra P4).** Dibujo con el patrón Metro: tabla C commiteada, máscara de cobertura de 8 bits, `metro_fb_plot_alpha()` (`metro_fb.c:116-127`), cero lecturas de disco en runtime. Generación con la metodología Aura: SVG → `rsvg-convert` → supersampleo 16× + filtro de caja → verificación `MIN_INK_TONES ≥ 4` que rompe el build (`generate.py:475`). Fuente de iconos: **Material Symbols** (Apache 2.0). Solo se generan los `icon_key × tamaño` que el plan enumere, no el Cartesiano 89×9 de Aura. Implementada en M3 (D-033): `design-system/generate.py` (`--icons`), `apps/metro/moonlit_icons.{c,h}`, `apps/metro/moonlit_icons_table.c`.

**D-009 — Formato de tema v1: omitido (cierra P3).** `aura.cfg` **no** declara `theme_format_supported`, igual que Metro (`COMPAT_STUDIO.md:16`). Consecuencia directa de D-008: cumplir v1 exigiría 801 máscaras en disco (§4, T6). Studio deshabilita instalación de temas limpiamente.

**D-010 — Tokens en un solo origen (cierra P6).** `design-system/tokens.json` es la única fuente de verdad de color, escala tipográfica, espaciado, radios y niveles de elevación. `design-system/generate.py` produce `firmware/.../moonlit_tokens.h` y las tablas de iconos. Ningún literal RGB fuera de `tokens.json`; el acento se lee en runtime por getter (patrón `metro_color_accent()`). Licencias de assets vendoreados en `design-system/vendor/<asset>/LICENSE*`.

## Lenguaje visual

**D-011 — Subconjunto Material sin GPU.** Se aprueban exactamente los 15 principios confirmados con primitiva real en §1.5 (blend entero, elevación por tono, esquinas redondeadas paramétricas, transiciones slide/blend ≤ 300 ms bajo `lcd_active()`, easing por tabla, alfa plano constante para "vidrio", grilla 8 dp = 8 px a `LCD_DPI 160`). Quedan **prohibidos**: blur, ripple, sombras difusas, easing bezier en runtime, rasterización vectorial en runtime. "Listas con divisores" (el 16.º principio) se aprueba condicionado a que el hito correspondiente cite la primitiva de blit de 1 px.

**D-012 — Waning Crescent: luz desde la izquierda.** Toda elevación se simula con **dos tonos por nivel**: borde izquierdo/superior un paso más claro (luz), borde derecho/inferior un paso más oscuro (sombra), ambos derivados de la paleta por `tokens.json`, nunca calculados por cuadro. Sin gradientes por píxel fuera de `lcd_active()`.

**D-013 — Fondo del reproductor (cierra P5).** Plano tonal Material: superficie de elevación derivada de la paleta nocturna, sin decodificar ni promediar la portada. Costo por cuadro cero.

## Cover Flow vertical

**D-014 — "Marea" entra en el plan, con restricciones (cierra P7 y §6).** Nombre: **Marea**. Se implementa **sin reflejo** (`reflection_buf` eliminado → 33.800 B/slot en vez de 42.120) y **sin morphs**. Geometría vertical retuneada a ~5 tapas visibles en 220 px útiles. Módulo de dibujo reescrito en espejo (fila-por-fila), no transpuesto. Sin portada → monograma (inicial del álbum en Libre Baskerville) sobre color de acento. H3 (≤ 33 ms/cuadro) **no se da por cerrada en simulador**: el plan incluye un hito de medición en hardware real; hasta entonces Marea es funcionalidad "experimental" en `DECISIONS.md`.

## Interacción

**D-015 — Play/Pause global (cierra P10).** No se decide sin cita. El hito 0 del plan incluye la lectura del mapeo de botones de Metro-Aura (`apps/keymaps/keymap-ipod.c` y el shell Metro) y el comportamiento resultante se cierra en `moonlit-aura/DECISIONS.md` con ruta:línea antes de cualquier cambio de UI.

## Logotipo

**D-016 — Waning Crescent.** Especificación vectorial original: luna menguante por **sustracción de dos círculos**, iluminada desde la izquierda (coherente con D-012), monocroma, dibujada en color de acento dinámico. Incluye **wordmark "moonlit"** en Libre Baskerville **solo** en arranque y "Acerca de" (≥ 64 px); en tamaños 16/24/40 px se usa únicamente la geometría. Verificación mecánica de tonos ≥ 4 en cada tamaño exportado antes de integrarse. No deriva de ningún icono existente.

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
(`metro_screen_hub.c:606-623`); la rejilla se conserva. Se registra en
M1 (decisión de alcance); se implementa en M8.

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
