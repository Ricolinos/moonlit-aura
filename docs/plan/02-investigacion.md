# moonlit.aura — Investigación (fase 2)

**Fecha:** 2026-08-25 · **Alcance:** ejecución de las tareas T1–T9 y del estudio de legibilidad §4 de `01-plan-investigacion.md`, solo lectura de repos hijos (más una copia aislada en scratchpad para el estudio de legibilidad, nunca escrita dentro de un repo). Ningún archivo de Aura-Firmware, Metro-Aura ni Aura-Studio fue modificado — verificado con `git status --short` en ambos repos públicos tras la investigación (limpio en los dos).

Todas las tareas fueron ejecutadas por subagentes de solo lectura, uno por tarea. Cada hallazgo cita ruta:línea; donde no fue posible confirmar por lectura directa, queda marcado **HIPÓTESIS** explícitamente.

---

## 1. Hallazgos técnicos (renderizado, fuentes, iconos, memoria, build)

### 1.1 Formato `.fnt` y antialias real (T1)

**CONCLUSIÓN.** La tensión aparente entre "`antialias = 1`" y "Metro-Aura documenta 1bpp" es un error de documentación, no del código. `convttf.c:102,115` renderiza con FreeType antialiasado real (256 niveles de gris, sin `FT_LOAD_MONOCHROME`). El empaquetado (`convttf.c:626-629,845-856`) usa 4 bits/píxel — **16 niveles de gris reales**, no 1 bit — con reducción `cur_col = (src2+8)/17` (`:850`), un redondeo, no una binarización.

El campo `header.depth = 1` (`convttf.c:682`) que Metro-Aura llamó "1bpp" **no es el bpp real**: es una bandera que `firmware/font.c:212` (`glyph_bytes()`) usa para elegir la fórmula de tamaño — `depth` verdadero → 4bpp (coincide con convttf); `depth` falso → 1bpp clásico. Generador y lector son consistentes entre sí; solo la documentación de Metro-Aura (M-011) nombró mal el campo.

Guardarraíl relevante: `firmware/font.c:441-445` rechaza fuentes antialiasadas solo si `LCD_DEPTH < 16`. En ipod6g, `LCD_DEPTH 16` (`ipod6g.h:84`) — la condición se compila fuera, las fuentes antialiasadas cargan sin restricción en el target real. `convttf.c` y `font.c` son byte-idénticos entre Metro-Aura y Aura-Firmware (diff vacío verificado), por lo que esta conclusión aplica a moonlit sin cambio si hereda el árbol Rockbox intacto (M-022).

**Hallazgo colateral con cita (encontrado durante el estudio de legibilidad, pertenece a esta tarea):** `gen_fonts.sh:47-48` define `START=0x20`/`LIMIT=0x17F` como *strings literales*; `convttf.c:1101,1113` los parsea con `atoi()`/`atol()`, que leen `"0x20"` como `0` (se detienen en la `x`). `convttf.c:674` trata `limit_char == 0` como "sin límite". **CONCLUSIÓN verificada empíricamente**: la invocación tal como está escrita en `gen_fonts.sh` no limita a ~352 glifos Latin — convierte el charset completo del TTF de entrada (replicado con Libre Baskerville: 1309 glifos, no ~352). Esto es un bug real y activo en el pipeline heredado (M-028), no solo una curiosidad de moonlit — afecta también a Metro-Aura y Aura-Firmware hoy. Se anota aquí y se traslada a preguntas abiertas (P9) por su impacto en presupuesto de flash/RAM.

**Implicación para el estudio de legibilidad:** el umbral de "trazo visible" en binarización (§4.3 del plan) debe evaluarse contra 16 niveles reales, no contra una binarización 2-tono — pero qué nivel de gris cuenta como "visible" sigue siendo una decisión de diseño no derivable del código (P9 en preguntas abiertas).

### 1.2 Presupuesto de fuentes (T2, refinado con datos reales del estudio de legibilidad)

**CONCLUSIÓN — techo de slots.** `MAXUSERFONTS` es 12 en Metro-Aura (`firmware/export/font.h:51`) y 14 en Aura-Firmware (`:64`), con `MAXFONTS = FONT_FIRSTUSERFONT + MAXUSERFONTS` en ambos. Metro usa 5/12 slots (`metro_fonts.c:39-46`). Aura-Firmware usa **14/14, al límite exacto** — comentario explícito en `aura_style.c:274` y enum de 14 roles en `apple2026_shell.h:65-105`. D-289 (`Aura-Firmware/DECISIONS.md:60-68`) documenta que el cambio de tema descarga *todas* las fuentes antes de cargar el candidato precisamente porque no hay cupo para tener ambas activas.

**Implicación directa para moonlit:** si hereda el léxico de 14 roles tipo Aura sobre una base Metro (12 slots, 7 libres), **hay que subir `MAXUSERFONTS`** en el fork — decisión de fase 3, anotada aquí como dependencia dura.

**CONCLUSIÓN — sin tope duro de tamaño por fuente.** `MAX_FONT_SIZE = 60000` (60 KB) es el techo real (M-010/M-011, `Metro-Aura/DECISIONS.md:125-137`), sin restricción práctica sobre 64 MB de RAM.

**Datos REALES (no extrapolados) obtenidos por el estudio de legibilidad §4**, que sustituyen la tabla hipotética original de T2 para las dos familias candidatas — `.fnt` generados con `convttf` real, 316-318 glifos, rango decimal correcto (32–383), `-c 0`:

| Familia | 10px | 12px | 14px | 16px | 18px | 20px |
|---|---|---|---|---|---|---|
| Libre Baskerville | 10,8 KB | 14,7 KB | 20,3 KB | 22,9 KB | 29,4 KB | 34,8 KB |
| Montserrat | 9,6 KB | 12,1 KB | 16,2 KB | 19,6 KB | 24,4 KB | 31,0 KB |

Todas caben holgadamente bajo 60 KB/fuente. Con 14 roles en el rango 8–22 px, el total en RAM (buflib, carga simultánea) ronda **~220–280 KB** — trivial sobre 64 MB. El techo real sigue siendo el conteo de *slots* (`MAXUSERFONTS`), no los bytes. **HIPÓTESIS ya cerrada por dato real** (T2 original especulaba con extrapolación desde Selawik 64px; el dato real da valores *menores*, razonable porque Selawik es una UI sans más gruesa a tamaño grande que estas dos candidatas a tamaños de body-text).

### 1.3 Pipeline de iconos: disco vs binario (T3)

**CONCLUSIÓN — moonlit debe usar el patrón Metro (binario compilado), no el patrón Aura (disco).**

Dos patrones reales, ambos leídos con cita:
- **Aura (disco, 8-bit coverage):** `design-system/generate.py` genera máscaras BMP de un canal por supersampleo 16× (línea 391) + filtro de caja (372-390), guardadas en disco. En runtime, `aura_widgets.c:167-208` (`draw_icon_mask_2`) llama `aura_style_read_icon_bmp()` **por cada dibujo**. Verificación mecánica: `MIN_INK_TONES = 4` (`generate.py:475`), falla el build si algún ícono queda binarizado.
- **Metro (binario, compilado):** `gen_icons.py` documenta explícitamente (líneas 6-17) por qué NO usa disco: *"CLAUDE.md prohíbe lectura de disco dentro de un bucle de animación"*. SVG → `rsvg-convert` → tabla C commiteada, 32 B/ícono monocromo o máscara de cobertura de 8 bits compilada (no en disco) para íconos grandes, dibujada con `metro_fb_plot_alpha()` (`metro_fb.c:116-127`).

**Evidencia decisiva:** D-315 (`Aura-Firmware/DECISIONS.md:452-457`) ya diagnosticó, en producción, que `aura_style_read_icon_bmp()` leyendo disco por cuadro (~250-285 aperturas en 330ms) es el **sospechoso dominante** de la lentitud del morph del Modo 4. Es la misma función que usa el patrón de disco de Aura — Aura-Firmware ya midió su propio patrón como causa de un problema de rendimiento real.

**Decisión:** pipeline de generación de Aura (supersampleo 16×, filtro de caja, verificación `MIN_INK_TONES ≥ 4` — superior a Metro, que no verifica tonos) + formato de salida compilado de Metro (tabla C, máscara de cobertura de 8 bits). Fuente: Material Symbols (Apache 2.0, decisión cerrada) vía `rsvg-convert`.

**Costo estimado (HIPÓTESIS de dimensionamiento, Material Symbols no vendoreado aún):** ícono monocromo 16×16 = 32 B/ícono en flash; cobertura 8-bit = `size_px²` bytes/variante (24px→576B, 40px→1600B). Si moonlit heredara el Cartesiano completo de Aura (89 icon_key × 9 tamaños) a 24px promedio: ≈461 KB de flash — no crítico, pero **no verificado** cuánto de ese Cartesiano es realmente necesario para moonlit (fase 3).

Prueba de tonos aplicada conceptualmente (sin ejecutar, Material Symbols no vendoreado) a 3 íconos de muestra: `play_arrow` y `favorite` con riesgo bajo (geometría análoga a casos que ya pasan en Metro); `sync` con riesgo alto (trazos finos, análogo a `arrow_repeat_1` que Metro descartó por ilegible a 16px, `gen_icons.py:66-72`) — recomendación: verificar `sync` específicamente una vez vendoreado, usar variante Regular si falla.

### 1.4 Memoria — caché de portadas del Flow vertical (T4, detalle de memoria)

**CONCLUSIÓN.** El costo por slot de caché es exacto: `cover_buf` (130×130×2B) + `reflection_buf` (130×32×2B) = **42.120 B/slot** (`aura_musicflow.c:76-79`). Presupuesto real en producción: `MF_CACHE_SLOTS = 2×(MF_VISIBLE_RADIUS+15)+3 = 39` slots × 42.120 B ≈ **1,60 MB** (~2,5% de 64 MB), confirmado también en `Aura-Firmware/DECISIONS-ARCHIVE.md:2629`. El mecanismo de carga (`get_slot_for()`, `aura_musicflow.c:447-490`) es completamente agnóstico del eje — sirve tal cual para Flow vertical sin tocar una línea. Detalle completo de geometría visible en §6 (veredicto Cover Flow vertical).

### 1.5 Primitivas de renderizado sin GPU (T8)

**CONCLUSIÓN — blend entero confirmado en ambos repos.** `metro_fb_plot_alpha()`/`metro_fb_blend_over_color()` (`metro_fb.c:98-104,116-126,151-168`) y `a26_shell_blend()` (`apple2026_shell.c:665-678`) — misma fórmula de interpolación lineal entera (`from + ((to-from)*a)>>8` / `/256`), sin float. **Sin FPU confirmado**: D-076 (`Aura-Firmware/DECISIONS-ARCHIVE.md:676-680,3063`) documenta ARM926EJ-S sin FPU; `aura_motion_spring()` usa tabla de 24 muestras precomputada en vez de curva bezier en runtime — evidencia directa para el descarte de easing "emphasized". D-226/D-227 (`:2645-2661`) confirman pila de 8 KB — cualquier buffer temporal de "elevación por tono" o máscara debe ser estático, no en pila, si supera unos KB. `LCD_DPI 160` real (`ipod6g.h:83`, no 163 como se aclaró en el plan §0) → a esa densidad 1dp = 1px exacto, la grilla de 8dp es literalmente 8px sin conversión.

Validación completa de los 16 principios de la sección §6 del plan contra primitivas reales: **15/16 confirmados con cita directa** (blend, sin-FPU, esquinas redondeadas vía `a26_shell_fill_rounded_rect()` en `apple2026_shell.c:546`, transiciones bajo `lcd_active()` vía `metro_transitions_slide()` en `metro_transitions.c:165-172` con easing por tabla `metro_ease(METRO_EASE_OUT_EXPO,...)`, ausencia de blur/ripple/rasterización vectorial confirmada por ausencia de funciones `blur`/`gaussian` en el código). **1 hueco menor**: "listas con divisores" no tiene cita directa de primitiva en esta tarea (se asume trivial — un blit de 1px con las mismas funciones de framebuffer ya citadas — pero no se verificó con grep dedicado). Matiz: "esquinas redondeadas por máscara precalculada" del plan en realidad se implementa como función paramétrica (cálculo de radio en cada llamada), no como máscara bitmap — viable igual, mismo principio de "sin blur/SDF", pero la descripción del plan no es literal.

### 1.6 Build / simulador — confirmación operativa

El simulador `rockboxui` de Metro-Aura, ya compilado (`firmware/build-sim/`), se usó para capturas headless reales sin modificar el repo original: se copió (nunca movió) el binario y el `simdisk/` de prueba a una carpeta aislada de scratchpad, se generaron `.fnt` reales de las candidatas con una copia del binario `convttf`, y se sustituyó solo el archivo de fuente dentro de la copia aislada del `simdisk`, nunca en el repo. `git status --short` en Metro-Aura y Aura-Firmware quedó limpio antes y después — confirma que la restricción de escritura se cumplió íntegramente incluso durante el build/captura. Detalle completo del método y resultados en §5 y en las capturas adjuntas (`docs/plan/capturas/`).

**Aviso técnico real, hallado durante la generación:** al cargar fuentes variables (`.ttf` con eje `wght`) sin fijar instancia, Montserrat cargó como peso 100 (Thin) en vez de 400 (Regular) — la tabla `OS/2` estática de la build variable de Google Fonts no coincide con el eje por defecto real. Se corrigió usando una build estática (`Montserrat-Regular.ttf`, github.com/JulietaUla/Montserrat, MIT+OFL). **Implicación para fase 3:** si moonlit vendorea variable fonts, el pipeline `convttf`/FreeType necesita fijar explícitamente la instancia deseada o usar builds estáticas — el error no genera ningún aviso en el log de conversión.

### 1.7 Avisos legales de terceros (T9)

**CONCLUSIÓN — patrón existente, dos precedentes.** Ambos firmwares hermanos generan `firmware/dist/THIRD-PARTY-NOTICES.txt` en tiempo de empaquetado dentro de `package_dist.sh`, concatenando el `LICENSE`/`LICENSE.txt` de cada asset vendoreado: Aura-Firmware (`firmware/tools/package_dist.sh:197-233`) concatena Inter (tipografía), Lucide (ISC, corregido en D-290 §1.2, `Aura-Firmware/DECISIONS.md:82`) y Phosphor; Metro-Aura (`:179-205`) concatena Selawik (SIL OFL 1.1) y Fluent System Icons (MIT). Además del archivo de texto, D-283 (`Aura-Firmware/DECISIONS-ARCHIVE.md:3500-3521`) documenta la pantalla "Acerca de" → créditos, con atribución legible en el dispositivo, nota de marca Apple sin afiliación, y el requisito GPL v2 §3 (URL de código fuente) — usa el string `AURA_STR_ABOUT_CREDITS_BODY`. `LICENSE` en la raíz del repo (copia de `firmware/rockbox/docs/COPYING`) vive junto al repo, no dentro de `dist/`.

**Lista de avisos requeridos para moonlit.aura, por asset:**

| Asset | Licencia | Dónde vive el LICENSE fuente | THIRD-PARTY-NOTICES.txt | Pantalla "Acerca de" |
|---|---|---|---|---|
| Material Symbols (iconos, decisión cerrada) | Apache 2.0 | `design-system/vendor/material-symbols/LICENSE` (convención Aura, ver P6) | Sí, bloque nuevo | Sí, línea nueva en el párrafo de tipografía/íconos |
| Familia tipográfica candidata (P1, PENDIENTE) | SIL OFL 1.1 (ambas candidatas) | `.../vendor/<familia>/LICENSE.txt` | Sí, bloque nuevo | Sí |
| Rockbox (base del firmware) | GPL v2 | `firmware/rockbox/docs/COPYING` → `LICENSE` en raíz (patrón D-283) | No aplica (no es "third-party" vendoreado) | Sí, obligatorio por GPL v2 §3, texto adaptado a "moonlit" |
| Marca Apple (hardware) | N/A | — | No | Sí, nota de no afiliación (patrón D-283) |

**HIPÓTESIS**: no se pudo confirmar si moonlit usará `design-system/vendor/` (patrón Aura) o `firmware/assets/*-src/` (patrón Metro) como ubicación de los `LICENSE` fuente — depende de P6 (ya inclinada hacia el patrón Aura, ver §5).

**Hallazgo fuera de alcance** (anotado, no investigado): no se verificó si Material Symbols exige atribución de Google explícita más allá del texto de la licencia Apache 2.0 (que no exige atribución visible en UI, solo preservar el aviso de copyright) — es verificación legal puntual, no técnica.

---

## 2. Hallazgos visuales — qué del lenguaje de cada firmware hermano es transferible

De la tabla "Reutilizar / Adaptar / Rediseñar" del plan (§1), confirmado y ampliado por T3/T8/legibilidad:

- **Transferible byte-idéntico**: árbol Rockbox + toolchain + captura headless + tests de host + pipeline de fuentes `gen_fonts.sh`→`convttf` (con el bug de rango anotado en §1.1) + carga `font_load_ex` con buflib. Todo confirmado operativo en esta investigación (§1.6).
- **Transferible con cambio de contenido, no de mecánica**: identidad de familia (`firmware_family: moonlit`), acento dinámico (getter heredado de Metro, paleta propia en un solo archivo), matriz Animaciones×Gráficos.
- **Arquitectura de iconos**: moonlit debe adoptar el **patrón de dibujo binario de Metro** (§1.3) pero la **metodología de generación de Aura** (supersampleo + verificación de tonos) — es un híbrido, no una elección binaria entre "copiar Aura" o "copiar Metro".
- **Fondo del reproductor** (P5 del plan): sin captura real generada para esta pantalla en esta ronda (fuera del texto navegable alcanzado); se mantiene como decisión de diseño abierta, ver §8.
- **Hallazgo visual de alto valor (legibilidad real, no hipotética):** las tres fuentes evaluadas al tamaño nominal 14px que usa hoy el rol `caption` de Metro-Aura (incluida Selawik, ya en producción) rasterizan a **9px de cap-height real medido**, por debajo del mínimo ISO 9241-303 (10,5px) — ver §5 y capturas `docs/plan/capturas/03-06`. Esto es un hallazgo transferible sobre el *lenguaje tipográfico heredado* en sí, no específico de la elección de familia para moonlit: el rol `caption` compartido por los tres firmwares hermanos ya está al límite de legibilidad.
- **Material Design sin GPU** (§6 del plan): 15/16 principios confirmados con primitiva real compartida entre ambos firmwares hermanos (§1.5) — es el lenguaje visual más directamente transferible, ya que ninguno de los dos firmwares hermanos lo implementa hoy tal cual (es nuevo para los tres), pero las primitivas que lo soportan ya existen en ambos.

---

## 3. Hallazgos de interacción (rueda, botones, Play/Pause global)

**Rueda — CONFIRMADA reutilizable sin cambio (H4, T4).** `aura_wheel_step()` (`aura_wheel.h:48`, invocado en `aura_musicflow.c:1239`) opera exclusivamente sobre un índice 1D abstracto (`s_target_index`), sin referencia a coordenadas de pantalla — agnóstico de layout, reutilizable byte-idéntico para Flow vertical.

**Botones — confirmado indirectamente, sin cita dedicada.** El mecanismo de inyección de botones sin interacción humana (`*_SIM_AUTODUMP_BUTTONS`, patrón usado por `sim_shot.sh`/`sim_matrix.sh` y por el propio estudio de legibilidad para navegar el simulador headless, `uisimulator/common/sim_tasks.c:39-100`) confirma que el mapeo botón→acción es data-driven y utilizable desde el simulador, pero **no se leyó el mapeo de botones específico de moonlit** porque ninguna tarea del plan lo pedía con esa granularidad.

**Play/Pause global — NO INVESTIGADO.** Ninguna tarea T1–T9 del plan (`01-plan-investigacion.md` §2) cita rutas para el comportamiento de Play/Pause global (p. ej. si el botón central/Play responde igual en cualquier pantalla, fuera del reproductor). Esto es un hueco del plan mismo, no una omisión de esta ejecución: "ejecuta exactamente las tareas listadas" no incluye este ítem pese a que la estructura del entregable lo pide. Se traslada a preguntas abiertas (P10) en vez de investigarse fuera de alcance.

---

## 4. Contrato con Aura Studio: archivo, versión, campos; qué cambiaría en Studio (descrito, no ejecutado — repo privado)

**Archivo y campos (confirmado, T5/T6):** `/.rockbox/aura/aura.cfg`, regenerado entero cada guardado (`metro_settings_save()`, nunca edita en sitio). Campos relevantes: `firmware_family: moonlit` (D-324, cambia solo el string), `sync_marker_supported: 1` (mecanismo idéntico a Metro), `theme_format_supported` (**condicional**, ver recomendación abajo). Contratos vigentes sin renegociar: `CONTRATO-firmware-studio.md` v13, `CONTRATO-dispositivo.md` v2 (`device_name` solo lectura), `CONTRATO-formato-tema.md` v1 (condicional).

**Checklist de compatibilidad C1–C28 (T5), fuente `Metro-Aura/docs/COMPAT_STUDIO.md:12-41`, aplicado a moonlit.aura como fork de Metro-Aura — 22 Sí por herencia directa, 4 Parcial/heredado (mismo estado que Metro), 1 requiere hardware (C20), 1 pendiente de redacción propia (C21), 0 brechas duras en el contrato de datos en disco:**

| # | Requisito (resumen) | Estado moonlit | Mecanismo heredado |
|---|---|---|---|
| C1 | `/.rockbox/aura/` existe tras primer arranque | Sí (heredado) | `metro_settings_load/save()` — `COMPAT_STUDIO.md:14` |
| C2 | `aura.cfg` se regenera entero cada guardado | Sí (heredado) | `metro_settings_save()` (`creat()`, nunca edita en sitio) — `:15` |
| C3 | `aura.cfg` con `sync_marker_supported: 1` y `firmware_family: <propia>` | Sí, con cambio de valor: `firmware_family: moonlit` (D-324) | `:16` — mecanismo idéntico, solo cambia el string |
| C4 | Marcador `/.aura/sync-pending.json` procesado y borrado | Sí (heredado) | `:17` |
| C5 | Marcador con `version` no soportada se deja intacto | Sí (heredado, verificado por test-suite) | `:18` |
| C6 | `metro_disk_handoff()` corre al volver de USB | Parcial — heredado, sin verificar en sim (requiere `METRO_SIM_FORCE_USB=1`) | `:19` |
| C7 | Claves `rtc_sync_*` se aplican y descartan | Sí (heredado) | `:20` |
| C8 | Tagcache indexa los 3 layouts de `/Music/` | Sí (heredado) | `:21` |
| C9 | `cover.jpg` y APIC se muestran | Parcial — heredado (APIC no ejercitado con fixture en Metro) | `:22` |
| C10 | `.lrc` no rompe nada | Sí (heredado) | `:23` |
| C11 | `Playlists/*.m3u8` se listan y reproducen | Sí (heredado) | `:24` |
| C12 | `/Videos/*.mpg` con nombre 95 bytes | Sí (heredado) | `:25` |
| C13 | `/Photos/*.jpg` ≤640px, hasta 500 | Parcial — heredado | `:26` |
| C14 | Categorías video/foto: pivots si hay .cfg | Sí (heredado) | `:27` |
| C15 | `sync_summary.cfg`: presente→conteos, ausente→guiones | Sí (heredado) | `:28` |
| C16 | `device.cfg`: presente→nombre, ausente→genérico | Sí (heredado) | `:29` |
| C17 | Archivos no usados en v1 no rompen nada | Sí (heredado) | `:30` |
| C18 | Ninguna ruta/clave ajena se escribe/borra | Sí (heredado, regla dura en `Metro-Aura/CLAUDE.md`) | `:31` |
| C19 | Descriptor USB sin cambios | Sí (heredado, moonlit no toca `usb-s5l8702.c`) | `:32` |
| C20 | `AuraDeviceProbe` clasifica con Studio real | ⬜ Requiere hardware (igual que Metro) | `:33` |
| C21 | Advertencia sobre `AuraUpdateChecker` documentada | Pendiente — moonlit necesita su propia advertencia; mecanismo genérico | `:34` |
| C22 | `version.txt` en `rockbox.zip` solo en releases reales | Sí (heredado; requiere `package_dist.sh` propio, fase 3) | `:35` |
| C23 | `metrocache/<fuente>/` ajeno al contrato, Studio lo ignora | Sí (heredado); **brecha potencial**: si moonlit reusa el nombre `metrocache/` en el mismo dispositivo que Metro-Aura, colisionarían cachés de formato distinto | `:36` |
| C24 | `.lrc` hermano se consume para letras sincronizadas | Sí (heredado, `metro_lrc.c`) | `:37` |
| C25 | `artist_images.cfg` + `artists/<archivo>.jpg` se consumen | Sí (heredado) | `:38` |
| C26 | `ratings.cfg` se reimporta a `tag_rating` | Sí (heredado, `metro_sync.c`) | `:39` |
| C27 | Contrato v10: árbol dormido `/.firmware-<familia>/` | Sí, con cambio de nombre: `/.firmware-moonlit/` (D-326) | `:40` |
| C28 | Contrato v11: `install_manifest.cfg` es de Studio, firmware lo ignora | Sí (heredado) | `:41` |

Confirmado con cita: `LibrarySync.swift:81,193-233` no ramifica por familia — la biblioteca sincroniza igual para cualquier familia declarada. `SyncMarker.swift:68-99` (`declaredFamily()`) trata un `firmware_family` desconocido con degradación segura (sin actualizaciones ofrecidas, sin romper sync). **Brecha real** (no es un "No" del checklist, es una condición externa): Studio no reconocerá moonlit como firmware instalable/actualizable en su UI hasta que se agregue un `case` en `FirmwareFamily.swift:28` (ver T7 abajo).

**Sistema de temas v1 (P3, T6) — RECOMENDACIÓN: omitir `theme_format_supported`, igual que Metro.** Cumplir v1 íntegro exige: (1) portar el motor completo de `aura_style.c` (parser de manifiesto, escaneo de `themes/<id>/`, recarga de 14 fuentes con fallback); (2) subir `MAXUSERFONTS` de 12 a 14 (dependencia con §1.2); (3) construir un pipeline de **801 máscaras BMP en disco** (89 icon_key × 9 tamaños) — que **contradice directamente** la decisión de §1.3 de usar iconos binarios compilados por rendimiento (D-315). Cumplir v1 empujaría hacia el patrón de disco para iconos, revirtiendo esa decisión. El patrón "omitir" (Metro, `COMPAT_STUDIO.md:16`) tiene costo cero y precedente probado en producción: Studio deshabilita instalación de temas de forma limpia sin la clave.

**Qué tendría que cambiar en Aura-Studio para reconocer moonlit y ofrecer actualizaciones (T7 — descrito, NO ejecutado, es trabajo de repo privado en sesión propia):**

| Archivo | Cambio |
|---|---|
| `Models/FirmwareFamily.swift:28` | Nuevo `case moonlit` |
| `:35-40` (`configValue`) | `"moonlit"` |
| `:43-49` (`displayName`) | Nombre de producto (pendiente) |
| `:56-62` (`releaseRepository`) | Repo de moonlit (no existe aún, fase 3) |
| `:66-68` (`static let installable`) | Agregar `.moonlit` |
| `:75-81` (`bundleSubdirectory`) | `"moonlit"` |
| `:88-93` (`installedTreeSentinel`) | Depende de la fuente centinela elegida (P1) |
| `:105-110` (`dormantTreeName`) | `".firmware-moonlit"` (ya decidido, D-326) |
| `:114-121` (`static func parse`) | `case "moonlit": return .moonlit` |
| `Views/ExtrasView.swift:75-97` | Entrada de UI (patrón ya usado para Metro) |
| `scripts/fetch-firmware.sh:49-57,157` | Bloque de fetch para `moonlit` |
| `project.yml:72,119-121` | Recurso de bundle `moonlit` |
| `FIRMWARE_VERSION.example` | Bloque `moonlit.*` (mismo formato de prefijo que `metro.*`, líneas 13-20 confirmadas) |

**No requieren cambio** (confirmado leyendo el código completo): `GitHubReleaseChecker`, `AuraUpdateChecker` (ya cachea por familia desde ST-046), `BundledArtifacts` — los tres ya son genéricos por `FirmwareFamily`, diseñados para N familias, no para 2.

**Riesgo M-004** (`Metro-Aura/DECISIONS.md:41-46,68`): "vigente, no resuelto" según cita textual de la línea de estado; el detalle completo del riesgo residual (líneas 46-67) no se leyó por estar fuera del rango asignado a T7 — **HIPÓTESIS pendiente**.

---

## 5. Propuesta de lineamientos del sistema de diseño moonlit.aura

- **Color.** Acento dinámico vía getter heredado del patrón de Metro (`metro_color_accent()`-style), paleta propia de moonlit en un único archivo (`moonlit_palette.h` o equivalente) — nunca valores RGB fuera de ese archivo, siguiendo la restricción vinculante ya heredada (M-020/D-286 equivalente).
- **Tipografía.** Familia definitiva PENDIENTE (P1) — pero el estudio de legibilidad (§1.2, capturas reales) ya establece dos hechos verificados con evidencia, no hipótesis: (1) Baskerville y Montserrat tienen x-height casi idéntica (0,530 vs 0,526 em) — **refuta H-T1** del plan, que asumía una brecha grande; (2) el rol `caption` compartido (14px nominal) ya rasteriza a 9px de cap-height real en las tres fuentes evaluadas, por debajo del mínimo ISO 9241-303. **Recomendación de lineamiento**: subir el tamaño mínimo de rol de "caption" a al menos 18px nominal (proyecta ~11,5px cap-height, ver tabla calibrada en el hallazgo de legibilidad) antes de fijar la familia — el problema de legibilidad actual no es de familia, es de tamaño de rol.
- **Espaciado.** Grilla de 8dp = 8px exactos a `LCD_DPI 160` (§1.5) — sin conversión, aritmética entera directa.
- **Elevación simulada.** Superficies planas con colores precalculados por nivel ("elevación por tono"), cero sombras difusas — primitiva ya existente (`metro_fb_blend_over_color()`/`a26_shell_blend()`). Nunca blur/glassmorphism real; "hoja de vidrio" solo como alfa plano constante (patrón `M4_GLASS_ALPHA` de Aura, `aura_nowplaying.c:129`, es alfa plano, no blur).
- **Motion bajo `lcd_active()`.** Transiciones "fade-through"/"shared-axis" ≤300ms como slide/blend de framebuffer, gateadas por `lcd_active()` (patrón `metro_transitions_slide()`), con easing por tabla precomputada (`metro_ease(METRO_EASE_OUT_EXPO,...)`) — nunca curvas bezier en runtime (sin FPU, D-076).
- **Tokens.** Recomendación (P6, ya inclinada en el plan): generador JSON→C tipo Aura (`design-system/tokens.json` → header), no header manual tipo Metro — un lenguaje nuevo con paleta/escala/iconos se beneficia de un solo origen de verdad y del chequeo de tonos ya integrado en `generate.py` (§1.3).

---

## 6. Veredicto Cover Flow vertical ("Flow vertical" / candidato de nombre "Marea", P7)

### VIABLE CON RESTRICCIONES

Veredicto por hipótesis (T4):

- **H1 (proyección independiente del eje) — PARCIALMENTE CONFIRMADA.** El núcleo matemático (`aura_flow.c:125-229`) es reutilizable byte-idéntico — es una proyección 1D genérica sin noción de eje. **Pero no basta con transponer**: `AURA_FLOW_SCREEN_W 320` está hardcodeado como constante de barrido (`aura_flow.h:66`), y el módulo de dibujo (`draw_slide_perspective()`, `aura_musicflow.c:673-710`) lee la portada columna-por-columna y escribe tiras verticales — un carrusel vertical necesita el patrón espejo (fila-por-fila, tiras horizontales). **Restricción: reescritura del módulo de dibujo, no un cambio de parámetro.**
- **H2 (caché reutilizable) — CONFIRMADA para la caché; geometría requiere retuning.** Los 39 slots × 42.120 B ≈ 1,60 MB (§1.4) sirven tal cual, sin tocar una línea. Pero la geometría visible en pantalla **no es directamente portable**: hoy caben 7 tapas en 320px de ancho; en 220px de alto útil (240 − 20px de barra de estado) con una portada central de 110-120px, el espacio remanente alcanza para **~5 tapas (2 laterales por lado), no 7**, salvo que también se reduzca `MF_OFFSETX_R` para el eje vertical. **Restricción: retuning de constantes de layout, cálculo aritmético ya hecho, no medido en simulador.**
- **H3 (cuadro ≤33ms) — NO VERIFICABLE ni en simulador.** `Aura-Firmware/DECISIONS.md:465` documenta explícitamente que las optimizaciones de rendimiento del Modo 4 (mismo tipo de medición ms/cuadro) quedaron sin implementar porque requieren perfilado en hardware real (D-300) — las lecturas de simulador en un host x86/ARM64 a GHz no son representativas del ARM926EJ-S a ~216MHz sin FPU. **Corrección explícita a la premisa del plan §5: esta hipótesis queda PENDIENTE DE VERIFICACIÓN EN HARDWARE, no confirmable en fase 2.**
- **H4 (rueda reutilizable) — CONFIRMADA**, ver §3.
- **Riesgo D-224** ("coverflow muy lento", `DECISIONS-ARCHIVE.md:2613`): el cuello de botella histórico no fue la proyección sino la decodificación de portadas desde disco durante el desplazamiento — se resolvió con precarga + caché de 39 slots, ya contada en H2. Mismo riesgo aplica a Flow vertical, misma mitigación portable. Metro-Aura hoy **no tiene** `cfcache` (`metro_sync.h:28` confirmado sin la referencia) — hay que portar `aura_albumart.c` byte-idéntico (F0-2).

**Restricciones del veredicto, resumidas:** (1) reescritura del módulo de dibujo (espejo, no transponer); (2) retuning de geometría a ~5 tapas visibles; (3) el presupuesto de cuadro real queda pendiente de hardware, sin ese dato no se puede cerrar la decisión de "sin reflejo ni morphs" con certeza; (4) requiere portar `aura_albumart.c` a la base Metro.

---

## 7. Lista HIPÓTESIS pendientes

1. **Umbral de "trazo visible"** en el conteo de tonos del `.fnt` (16 niveles reales, no 2) — decisión de diseño, no derivable del código (§1.1).
2. **Peso por glifo de una fuente serif** a tamaños grandes — parcialmente resuelto por datos reales de Baskerville/Montserrat (§1.2), pero sin dato real de una tercera candidata si P1 cambia de familia.
3. **Costo de flash del Cartesiano completo de iconos** (89 × 9 a 24px ≈ 461 KB) — no verificado cuánto de ese Cartesiano es realmente necesario para moonlit (§1.3).
4. **Prueba de tonos en los 3 íconos de muestra** (`play_arrow`, `favorite`, `sync`) — aplicada solo conceptualmente, no ejecutada; Material Symbols no vendoreado aún (§1.3). **No cumple literalmente el criterio de "listo" de T3** ("prueba de tonos ≥4 en 3 iconos de muestra"), porque ejecutarla exige vendorear assets dentro de un repo — código/fase 3, fuera del alcance de escritura de esta investigación. Pendiente de ejecución real en fase 3.
5. **H3 del Flow vertical** (presupuesto de cuadro ≤33ms) — pendiente de hardware real, no de simulador (§6).
6. **Factor de calibración cap-height (0,64×px nominal)** — un solo punto de medición real (14px); no confirmado que sea constante entre tamaños (estudio de legibilidad).
7. **Prueba de separación "ll"/"rn" (M-082)** — sin cadena de prueba disponible en la biblioteca del simulador durante esta ronda; pendiente de repetir con pantalla de prueba dedicada.
8. **Ancho útil en caracteres/línea** — proyectado desde tamaño total de archivo `.fnt`, no medido carácter por carácter (requeriría parsear el binario RB12).
9. **Alcance exacto del riesgo residual M-004** — solo se leyó el título y la línea de estado ("vigente"), no el cuerpo completo (§4).
10. **Ruta de vendoreo de licencias de terceros** (`design-system/vendor/` vs `firmware/assets/*-src/`) — depende de P6, ya inclinado pero no cerrado (T9).

---

## 8. Preguntas abiertas

| # | Pregunta | Opciones | Recomendación |
|---|---|---|---|
| P1 | Familia tipográfica definitiva | (a) Libre Baskerville + Montserrat; (b) una sola sans y serif solo ≥24px; (c) otra pareja OFL | **H-T1 del plan queda REFUTADA por datos reales** (x-height casi idéntica, 0,530 vs 0,526 em) — el riesgo real no es la brecha de x-height sino que el rol `caption` compartido ya está al límite de legibilidad a 14px en las tres fuentes evaluadas, incluida la que ya está en producción. Recomendación revisada: (a) sigue siendo viable, pero condicionada a subir el tamaño mínimo de rol `caption` a ≥18px (§5), no a elegir una familia distinta |
| P2 | Ruta del entregable | (a) `docs/plan/`; (b) `docs/plans/` | Ya resuelto por uso: este documento vive en `docs/plan/` (a) |
| P3 | ¿moonlit implementa el formato de tema v1? | (a) cumplir íntegro; (b) omitir; (c) extender (descartada) | **(b) con evidencia de costo (T6)**: cumplir v1 exige 801 máscaras en disco, que contradice directamente la decisión de iconos binarios de P4 — son incompatibles entre sí, no independientes |
| P4 | Iconos en disco vs binario | (a) binario; (b) disco | **(a) confirmado con evidencia (T3, D-315)**: el patrón de disco es la causa dominante ya diagnosticada de un problema de rendimiento real en Aura-Firmware — **P4 y P3 están acopladas**: elegir (a) en P4 obliga a (b) en P3, y viceversa |
| P5 | Fondo del reproductor | (a) imagen 30%; (b) promedio RGB; (c) plano tonal Material | Sin captura real de esta pantalla en esta ronda (texto navegable no llegó tan lejos) — se mantiene la recomendación original del plan, (c), sin evidencia adicional |
| P6 | Tokens: generador JSON→C vs header manual | (a) JSON+generador; (b) header | (a), reforzado por T9: la ruta de vendoreo de licencias también se resuelve más limpio con la convención de Aura (`design-system/vendor/`) si se adopta (a) |
| P7 | Nombre propio del Flow vertical | p. ej. "Marea", "Fases", "Columna" | Sin cambios — "Marea" |
| P9 *(nueva)* | ¿Qué nivel de gris (de 16 reales) cuenta como "trazo visible" en la verificación mecánica de legibilidad? | (a) cualquier valor >0; (b) umbral ≥25% (>63/255); (c) umbral calibrado por captura real como hizo el estudio (>60/255, PIL) | (c) — es el único método ya validado con evidencia real en esta investigación |
| P10 *(nueva)* | Comportamiento de Play/Pause global no tiene tarea asignada en `01-plan-investigacion.md` — ¿se agrega como tarea a una fase 2.5, o se decide sin investigación de código previa? | (a) tarea adicional dedicada; (b) decisión de diseño sin precedente de código a citar | (a) — es un ítem pedido explícitamente en la estructura del entregable pero ausente del plan de tareas; conviene cerrarlo con la misma disciplina de cita que el resto antes de fase 3 |
| P11 *(nueva)* | Subir `MAXUSERFONTS` de 12 a 14+ en el fork de moonlit — ¿algún costo colateral no visto en T2/T6? | (a) ninguno esperado (RAM trivial); (b) requiere auditar buflib completo antes de decidir | (a) como hipótesis de trabajo, pero **verificar en fase 3** antes de comprometerse — ninguna tarea de esta ronda auditó el impacto de subir ese define fuera de la RAM directa de fuentes |
| P12 *(nueva)* | El bug de `atoi()` en `gen_fonts.sh` (§1.1) afecta hoy a Metro-Aura y Aura-Firmware en producción, no solo a moonlit — ¿se corrige ahí (fuera de este plan, sesión propia por repo) o se hereda deliberadamente para moonlit? | (a) reportar y corregir en cada repo hermano por separado; (b) heredar el bug en moonlit y decidir después | (a) — es un bug real con impacto de flash/RAM ya medido (1309 vs ~352 glifos), corregirlo no es una decisión de diseño de moonlit, es una corrección de pipeline compartido |

---

*Autoevaluado y corregido contra `01-plan-investigacion.md` por un subagente de revisión antes de la entrega — ver nota final de contención: ningún archivo de Aura-Firmware, Metro-Aura o Aura-Studio fue tocado en ningún momento de esta investigación.*
