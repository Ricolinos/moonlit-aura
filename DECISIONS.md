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
