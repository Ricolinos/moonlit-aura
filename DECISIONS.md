# DECISIONS.md — Registro de decisiones técnicas (Metro-Aura)

Bitácora de decisiones tomadas en este repositorio, numeración
**M-001** en adelante. Referencias `D-NNN` apuntan al `DECISIONS.md`/
`DECISIONS-ARCHIVE.md` de `Aura-Firmware`; referencias `ST-NNN` al de
`Aura-Studio`.

⚠️ Esto es una BITÁCORA, no una spec. La fuente de verdad del diseño
vigente es `docs/plans/PLAN_MAESTRO.md` §D (resumen tabulado) y este
archivo (detalle y decisiones nuevas posteriores a la Fase 3); para el
estado del código, el propio código.

---

## M-001 — Commit base: Rockbox `0726ec93517a61f602679ab052b083217ec9c96d`

Mismo commit base que usa `Aura-Firmware`. **Por qué**: el toolchain, el
simulador SDL y todos los fixes de hardware que se portan en F0 ya
están probados contra ese commit exacto en este entorno; un commit
upstream más nuevo introduce una variable no probada sin beneficio
concreto documentado (`docs/plans/INVESTIGACION.md` D.7).

## M-002 — Toolchain propio, no compartido con Aura-Firmware

`firmware/toolchain/` se compila localmente con `firmware/tools/build_toolchain.sh`
(mismo procedimiento que D-032 de Aura-Firmware). Nunca se referencia
por ruta relativa un toolchain de otro checkout — regla de contención
de la carpeta padre (`Aura/CLAUDE.md`).

## M-003 — Rutas en disco: se conserva el prefijo `.rockbox/aura/`

Metro-Aura escribe y lee `.rockbox/aura/aura.cfg`,
`.rockbox/aura/sync_summary.cfg`, `/.aura/sync-pending.json`, etc. —
los mismos nombres de ruta que usa Aura-Firmware. **Por qué**: es la
única señal que `AuraDeviceProbe.swift` de Aura Studio usa para
reconocer un dispositivo compatible (existencia de `.rockbox/aura/` +
`aura.cfg`) — no depende del nombre visible del firmware. Cambiar el
prefijo rompería la detección automática sin ninguna ganancia
(`docs/plans/INVESTIGACION.md` E.2).

## M-004 — Distribución propia; riesgo de `AuraUpdateChecker` documentado, no resuelto

Metro-Aura publica sus propios releases (tag, `rockbox.ipod`,
`rockbox.zip`, `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`)
e instalación manual. **No** escribe `.rockbox/aura/version.txt`
(evita aparentar ser una versión de Aura). **Sí** escribe la clave
nueva `firmware_family: metro` en `aura.cfg` — clave propia de Metro,
sin equivalente en el contrato de Aura-Firmware/Aura-Studio.

**Riesgo abierto, no resuelto en este proyecto**: si el usuario tiene
Aura Studio instalado con el `rockbox.ipod` de Aura embebido y conecta
un iPod con Metro, `AuraUpdateChecker.swift` comparará hashes, no
coincidirán, y ofrecerá "actualizar" — lo que sobrescribiría Metro con
Aura si el usuario acepta. Corregir esto requiere que `Aura-Studio/`
aprenda a leer `firmware_family` y no ofrezca esa acción — trabajo en
otro repositorio, fuera del alcance de este proyecto (regla de
contención de la carpeta padre). Documentado como advertencia en
`docs/ESTADO_FINAL.md`.

## M-005 — Marcador de sincronización desde el día uno

Se portan `metro_sync.c`/`metro_sync_marker.c` (adaptados de
`aura_sync.c`/`aura_sync_marker.c` de Aura-Firmware) en F6, no se
depende del fallback de Aura Studio (borrar `.tcd` y reconstruir
completo). `aura.cfg` anuncia `sync_marker_supported: 1` desde F6.

## M-006 — Toda la UI en C propio, bucle propio reemplaza `root_menu()`

`apps/main.c` llama a `metro_main()` en vez de `root_menu()`, una sola
vez, sin retorno — mismo patrón que D-001/D-014/D-015 de Aura-Firmware.
`root_menu()`, el skin engine, `gui_synclist`, `do_menu()`, `tree.c`,
`tagtree.c` quedan compilados pero inalcanzables. **Por qué**: el skin
engine no tiene primitivas de animación ni de movimiento horizontal
(`docs/plans/INVESTIGACION.md` A.4); `gui_synclist` tiene altura de
fila fija y sin soporte de pivots (A.2).

## M-007 — Input vía `get_custom_action()` con keymap propio, no lectura cruda

A diferencia de Aura-Firmware (que lee botones crudos con
`button_get()`), Metro-Aura usa `get_custom_action(ctx, timeout, ...)`
con tablas `button_mapping[]` propias (`apps/metro/metro_keymap.c`).
**Por qué**: el filtrado de `BUTTON_REL` espurio ya lo resuelve
`action.c` con el patrón "prereq button" — evita reimplementar a mano
el fix que Aura tuvo que escribir (`next_button()`,
`docs/plans/INVESTIGACION.md` C.3/C.4); las tablas declarativas
encajan mejor con la jerarquía de profundidad del twist.

## M-008 — Piezo de clic apagado por defecto

`global_settings.keyclick` queda en su valor de fábrica (0 = off), sin
ajuste expuesto en la UI v1. **Por qué**: el pad del Zune 30 no tenía
el mismo feedback táctil-sonoro que el clickwheel del iPod
(`docs/plans/INVESTIGACION.md` F.8); además es la consecuencia directa
de usar `get_custom_action()` con `keyclick=0` (C.7).

## M-009 — Idioma: tabla propia bilingüe ES/EN, español por defecto

`apps/metro/metro_lang.c`, sin el sistema `.lang` de Rockbox — mismo
mecanismo que `aura_lang.c` de Aura-Firmware (D-013). Ajuste "Idioma"
en Ajustes generales.

## M-010 — Tipografía: Selawik (SIL OFL), 5 roles, carga completa

Fuente: Microsoft Selawik, licencia SIL Open Font License 1.1 —
redistribuible, sin restricción, cero material Zegoe/Segoe en el
árbol. Vendoreada en `firmware/assets/fonts-src/`, convertida con
`tools/convttf` por `firmware/tools/gen_fonts.sh` a
`firmware/assets/fonts/metro-*.fnt` (versionados).

Roles: `display` (Light 48px), `title` (Light 28px), `list` (Regular
20px), `list_sel` (Semibold 20px), `caption` (Regular 14px). Rango de
caracteres 0x20–0x17F (latín básico + Latin-1 + Latin Extended-A).

Carga con `font_load_ex(path, 0, N)` (N ≥ número de glifos de la
fuente) — carga **completa** desde buflib, sin modo caché. **Nota
sobre una corrección de la Fase 2**: la investigación original (Fase
2, `INVESTIGACION.md` A.7) concluyó erróneamente que había un límite
duro de 10 KB por fuente (`MAX_FONT_SIZE`) por asumir que `MEMORYSIZE`
no estaba definido para `ipod6g`. Verificado en la Fase 3: `MEMORYSIZE=64`
sí se inyecta vía `-D` desde el `Makefile` que genera `tools/configure`
(`--ram=$(MEMORYSIZE)`), así que `MAX_FONT_SIZE=60000` y no hay tope de
10 KB — ver la errata explícita en `docs/plans/INVESTIGACION.md` (A.7,
G.3). Selawik Light a 64px pesa 284 KB en disco, ~1 950 B/glifo — carga
completa sin problema sobre 64 MB de RAM.

Flujo opcional "el usuario aporta Zegoe/Segoe": documentado en
`docs/guia-desarrollo.md` — mismos parámetros de `convttf`, mismos
nombres de archivo, colocados en `.rockbox/fonts/`.

## M-011 — `MAXUSERFONTS` se queda en 12 (valor de upstream, sin tocar)

Metro necesita 5 slots de fuente; el valor de upstream (12) alcanza de
sobra. `firmware/export/font.h` no se modifica — menos diff contra
upstream que Aura-Firmware (que lo subió a 14 para sus 12+2 estilos).

## M-012 — Paleta: base oscura por defecto, 10 acentos WP7, magenta por defecto

Ver `docs/plans/PLAN_MAESTRO.md` §1.4 para la especificación completa
y `docs/plans/INVESTIGACION.md` F.5 para la tabla de conversión a
RGB565 (error de cuantización ≤ 1% en los 10 acentos). Acento por
defecto: magenta `#FF0097`.

## M-013 — Buffers de transición: 2 framebuffers estáticos en BSS, 150 KB c/u

Mismo patrón verificado en `aura_transitions.c` de Aura-Firmware
(`docs/plans/INVESTIGACION.md` B.3) — sin alocación dinámica.

## M-014 — Presupuesto de animación: 30 fps nominal, transiciones ≤ 300 ms, boost por transición

Ver `docs/plans/PLAN_MAESTRO.md` §3.2. Cifras derivadas
(`docs/plans/INVESTIGACION.md` B.2), no medidas en hardware real hasta
F13 — sujetas a ajuste.

## M-015 — Niveles de FX: matriz de 2 ejes (`animations` × `graphics`), con auto-degradación por sesión

Mismo sistema que Aura-Firmware (`docs/plans/INVESTIGACION.md` B.10),
persistido en `aura.cfg`. Umbral de auto-degradación (> 2× presupuesto
en 3 mediciones consecutivas) es una estimación inicial, ajustable con
datos reales de F13.

## M-016 — Video/fotos con plugins stock en v1; visor propio y modo "cubrir" al backlog

`mpegplayer.rock`/`imageviewer.rock` sin modificar en v1 (con
`plugin_set_silent_open_errors(true)`). El modo "cubrir pantalla" y un
visor de fotos propio quedan en el backlog de
`docs/plans/PLAN_MAESTRO.md`.

## M-017 — Ajustes: `global_settings` de Rockbox para lo genérico, `aura.cfg` para lo propio de Metro

Ver tabla completa de claves en `docs/plans/PLAN_MAESTRO.md` §1.2
(`metro_settings.c`) y §4 (checklist de compatibilidad).

## M-018 — Iconografía mínima compilada en el binario, nunca leída de disco por cuadro

Ver `docs/plans/INVESTIGACION.md` A.11/B.10 (regla de oro: nunca leer
bitmaps de disco dentro de un bucle de animación).

## M-019 — Higiene de `global_settings` al primer arranque de `metro_main()`

`statusbar=STATUSBAR_OFF`, `backdrop_file="-"`, `show_shutdown_message=false`,
`talk_menu=false`, `clear_settings_on_hold=false`, `usb_hid=false`
(bajo `USB_ENABLE_HID`), `tagcache_ram=true`, `keyclick=0` (M-008).

## M-020 — Nombre visible "Metro"; cero material Microsoft en pantalla o en el árbol

Wordmark "metro" en minúsculas (Selawik Light). Cero logotipos o
nombres de Zune/Windows Phone/Zegoe/Segoe en la UI, en el árbol de
código, ni en el `.zip` distribuido. Documentación de proyecto usa
"Metro-Aura"; la UI del dispositivo usa "Metro".

---

## Decisiones de la Fase 4 (ejecución)

### M-021 — Fase Cero: se agrega `tools/configure` a los archivos portados (no eran 9, son 10)

Ver `docs/DESVIACIONES.md` F0-1. `tools/configure` tiene una
modificación real de Aura-Firmware (D-007, detección de gcc de
Homebrew con fallback) que no estaba documentada en la lista de 27
archivos de `MODIFICATIONS.md` de ese repositorio. Se verificó con
`diff -rq` completo del árbol de Aura-Firmware contra una copia limpia
del upstream antes de sembrar este repositorio.

### M-022 — Fase Cero: los archivos portados de Aura-Firmware se copian byte-idénticos, sin reescribir su atribución GPL inline

Ver `docs/DESVIACIONES.md` F0-2. Los comentarios `Aura (D-NNN)` dentro
de los 10 archivos portados **no** se reescriben a `Metro (from Aura
D-NNN)` — son avisos de modificación GPL v2 §2a que documentan quién
hizo el cambio real (Aura-Firmware). La atribución de que además
llegaron a Metro-Aura por copia vive en `MODIFICATIONS.md`, no en el
código. Única excepción: `uisimulator/common/sim_tasks.c` sí se
modificó (variables de entorno `AURA_SIM_*` → `METRO_SIM_*`, son
identificadores de proyecto, no aviso legal).

### M-023 — F1: la higiene M-019 corre dentro de `init()` (apps/main.c), no al principio de `metro_main()`

Ver `docs/DESVIACIONES.md` F1-1. El backdrop Cabbie v2 stock se pinta
sobre el LCD en `settings_apply_skins()`, llamado dentro de `init()`
**antes** de que `metro_main()` corra — esperar a `metro_main()` para
desactivar `global_settings.backdrop_file` es demasiado tarde, el
backdrop ya quedó activo y `lcd_clear_display()` lo redibuja en cada
limpieza de pantalla (comportamiento nativo de
`lcd_clear_viewport()`, no un bug). `metro_apply_hygiene()` (declarada
en `metro_main.h`, ya no `static`) se llama ahora desde ambos cuerpos
de `init()` en `apps/main.c`, justo después de `settings_load()` —
mismo punto exacto que usa Aura-Firmware.

### M-024 — F2: `metro_draw_rows()`/`draw_pivots()`/`draw_tile()`/`draw_progress()` se difieren a F3

Ver `docs/DESVIACIONES.md` F2-1. Esas cuatro primitivas operan sobre
tipos (`struct metro_page`/`metro_pivot`/`metro_row`) que el propio
plan define recién en F3 — escribirlas antes exigía inventar firmas
provisionales. `metro_draw.c` de F2 solo trae lo que su propio
criterio de "hecho" necesita: `clear`, `text`, `text_cut_right`,
`header`, `battery`.

### M-025 — F2: captura visual de `F2-type-specimen.png` no lograda en esta sesión (limitación de entorno, no de código)

Ver `docs/DESVIACIONES.md` F2-2 — hallazgo importante para toda fase
futura que dependa de `sim_shot.sh` con `metro_main()` corriendo su
loop de botones. Causa raíz identificada con alta confianza: falla de
AppKit (`nextEventMatchingMask should only be called from the Main
Thread!`) al bombear eventos SDL desde el hilo del "dispositivo"
(`HAVE_SDL_THREADS`, activo por el propio `FIXME` ya presente en
`tools/configure` para macOS ≥ 26.4 — este Mac corre 26.5.2)
concurrente con el hilo de volcado headless (`uisimulator/common/sim_tasks.c`).
Determinista para cualquier `METRO_SIM_AUTODUMP_TICKS` ≥ 30 en esta
sesión; sin ventana segura posible porque el propio arranque de
Rockbox (`show_logo_boot()`) ya ocupa los primeros ~100 ticks.
Verificado por 6+ técnicas distintas sin éxito (detalle completo en
`docs/DESVIACIONES.md` F2-2). No se alteró `metro_main()` para
maquillar el síntoma (un `sleep()` fijo antes del loop degradaría la
responsividad real del producto). Verificación de F2 se apoyó en el
log `DEBUGF` de carga de fuentes (evidencia sólida y completa) más
compilación limpia para simulador y target real. **Pendiente**: el
dueño debe intentar la captura en una sesión interactiva propia (no en
segundo plano) — si el problema persiste ahí, amerita una fase de
investigación dedicada antes de F3, dado que F3+ depende de captura
con botones inyectados.
