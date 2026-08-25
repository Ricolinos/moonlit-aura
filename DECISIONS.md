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

**D-008 — Iconos binarios compilados (cierra P4).** Dibujo con el patrón Metro: tabla C commiteada, máscara de cobertura de 8 bits, `metro_fb_plot_alpha()` (`metro_fb.c:116-127`), cero lecturas de disco en runtime. Generación con la metodología Aura: SVG → `rsvg-convert` → supersampleo 16× + filtro de caja → verificación `MIN_INK_TONES ≥ 4` que rompe el build (`generate.py:475`). Fuente de iconos: **Material Symbols** (Apache 2.0). Solo se generan los `icon_key × tamaño` que el plan enumere, no el Cartesiano 89×9 de Aura.

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
