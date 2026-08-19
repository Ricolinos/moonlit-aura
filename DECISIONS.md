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

### M-025 — F2: captura visual de `F2-type-specimen.png` no lograda en esta sesión (limitación de entorno, no de código) — **SUPERADA por M-027: el diagnóstico era incorrecto, SÍ era código de Metro**

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

**Actualización — confirmado en sesión interactiva**: el dueño
reprodujo el mismo crash corriendo `sim_shot.sh` directamente en
Terminal.app. No es un artefacto de sesión en segundo plano — es un
problema real de este entorno (macOS 26.5.2 + SDL3 + Rockbox).

### M-026 — F2: intento de mover `screen_dump()` al hilo de `metro_main()`, revertido (cambia el crash por un deadlock) — **SUPERADA por M-027 (misma causa raíz equivocada)**

Ver `docs/DESVIACIONES.md` F2-3. Se separó el disparo del volcado
(`sim_thread`, sin cambios) de su ejecución real (`screen_dump()`/
`exit()`, movidos a una función nueva llamada desde el propio loop de
`metro_main()`, el único hilo que ya pasa de forma segura por
`button_get_w_tmo()`). Eliminó el crash de AppKit, pero introdujo un
**deadlock nuevo, 100% determinista** — el volcado queda colgado a
mitad del bucle de copia de píxeles del framebuffer (antes de la
primera escritura de línea), incluso en los ticks bajos que antes sí
funcionaban. Diagnosticado con instrumentación `DEBUGF` temporal
(revertida) hasta ubicar el punto exacto del cuelgue; descarta que sea
el `exit()` (el mismo cuelgue ocurre sin `METRO_SIM_AUTODUMP_QUIT`).
**Revertido por completo** (`git checkout --` sobre los 4 archivos:
`uisimulator/common/sim_tasks.c`/`.h`, `apps/metro/metro_main.c`,
`firmware/screendump.c`) — un deadlock del 100% de las veces es
estrictamente peor que un crash parcial. El árbol de F2 quedó
exactamente como en su commit, ningún cambio de este intento llegó a
comitearse.

**Pista para quien retome esto**: hay una segunda condición de carrera
independiente de la de M-025, en la capa de filesystem simulado de
Rockbox (`firmware/target/hosted/filesystem-unix.c`, sin tocar por
Metro ni Aura) — se dispara específicamente al llamar `screen_dump()`
desde el hilo "device" en vez de `sim_thread`. Una solución real
necesita depurar ambos hilos con `lldb`/Instruments en una sesión
interactiva (no disponible en esta sesión por falta de autorización
de macOS), no solo resolver el problema de AppKit de M-025.

### M-027 — F2: causa real del crash de captura — `struct viewport` sin inicializar en `metro_draw_text_cut_right()`; M-025/M-026 quedan superadas

Ver `docs/DESVIACIONES.md` F2-4 (detalle completo, evidencia y
lección metodológica). Resumen: el simulador de **Aura-Firmware**
captura sin problema en esta misma máquina con el mismo entorno — el
problema era de Metro. `metro_draw_text_cut_right()` declaraba un
`struct viewport` en la pila sin inicializar y llamaba
`viewport_set_fullscreen()` directamente; `lcd_init_viewport()` lee
`vp->buffer` (basura) antes de que nadie lo asigne y lo desreferencia/
escribe a través de él (UB) — corrompiendo el estado del LCD que
`screen_dump()` usa después vía `FBADDR()` → `buffer->get_address_fn`
(llamada indirecta que saltó a código arbitrario: a `metro_main`, cuyo
`button_get_w_tmo()` hace `SDL_PumpEvents()` en el hilo llamante por
diseño en `__APPLE__`, de ahí la excepción de AppKit desde
`sim_thread`). **Arreglo**: `viewport_set_defaults()` (idioma de
Rockbox, pone `buffer = NULL` primero). Verificado: captura
determinista a 100/200/300 ticks y con botones inyectados; simulador y
target compilan limpio. **Regla nueva para `apps/metro/`**: todo
`struct viewport` local se inicializa con `viewport_set_defaults()`
(o `memset` 0 + `viewport_set_fullscreen()`), nunca con
`viewport_set_fullscreen()` a secas. Se añade a `CLAUDE.md`.

### M-028 — F2: `gen_fonts.sh` sin `-x` (trim horizontal) — recortaba el glifo del espacio a ~1 px

Detectado en la primera captura real de F2: el texto aparecía sin
espacios ("title28px"). `convttf -x` recorta hasta 2 px por lado de
todo glifo "casi vacío", incluido el espacio (0x20): a 20 px pasa de
~5 px a ~1 px. Aura-Firmware no usa `-x` (`design-system/generate.py`
invoca solo `-p <size>`). Quitado; las 5 fuentes se regeneraron y se
versionaron de nuevo. M-010 sigue vigente en todo lo demás.

## M-029 — F3: `get_custom_action()` con contexto propio exige el bit `CONTEXT_PLUGIN`

Detalle load-bearing de M-007, no obvio por la documentación de
`apps/action.h`: `action_code_lookup()` (`apps/action.c`) solo llama al
callback `get_context_map` que se le pasa a `get_custom_action()`
**si** `context & CONTEXT_PLUGIN` es cierto — si no, usa
`get_context_mapping(context)` (la tabla interna del core, que no
conoce `MCTX_HUB`/`MCTX_LIST`/`MCTX_DIALOG` y devolvería `NULL`).
`metro_input_next()` siempre llama
`get_custom_action((int)ctx | CONTEXT_PLUGIN, ...)`; las tablas de
`metro_keymap.c` terminan con `LAST_ITEM_IN_LIST` (no la variante
`__NEXTLIST`) para que un botón sin mapear resuelva a `ACTION_NONE` en
vez de caer a `CONTEXT_STD` — el espacio de acciones de Metro
(`MACT_*`, arrancando en `LAST_ACTION_PLACEHOLDER+1`) es disjunto del
del core, no hay nada útil a lo que encadenar.

## M-030 — F4: `config.h` antes que `tagcache.h` en cualquier archivo nuevo que use tagcache

`tagcache.h` comprueba `#ifdef HAVE_TAGCACHE` antes de incluir `config.h`
él mismo (esa macro la define `config.h` vía `config/ipod6g.h`). Si
`tagcache.h` es el primer header de un archivo, `HAVE_TAGCACHE` todavía
no existe y todo el contenido del header desaparece en silencio -- el
archivo compila "bien" pero cada función de tagcache queda sin
declarar, con errores de compilación confusos y lejos de la causa real.
Mismo gotcha ya documentado por Aura-Firmware (su D-021); `metro_music.c`
lo replica con `#include "config.h"` primero. Cualquier archivo nuevo de
`apps/metro/` que use tagcache respeta este orden.

## M-031 — `metro_page.h`: `title_dynamic` para encabezados que nombran datos del usuario

Extensión de M-009/F3-1 necesaria en F4 (detalle completo en
`docs/DESVIACIONES.md` F4-1): `struct metro_page.title` sigue siendo
`enum metro_lang_id` (cadena de UI, resuelta en tiempo de dibujo), pero
una página cuyo encabezado nombra un artista/álbum/género real de la
biblioteca del usuario no puede usar ese campo -- no es una cadena de
UI traducible. `title_dynamic` (un `const char *`, NULL por defecto)
la sustituye cuando no es NULL. Cualquier pantalla futura cuyo
encabezado necesite mostrar un dato del usuario (F7 fotos/videos por
categoría, etc.) usa este mismo mecanismo en vez de inventar uno
nuevo.

## M-032 — F4: `metro_music_db_ready()` también dispara `tagcache_start_scan()` sobre una base ya usable

Encontrado verificando C8 (`docs/COMPAT_STUDIO.md`): una base de
tagcache construida en una sesión anterior del simulador no se enteraba
de pistas agregadas después (mismo bug real que Aura-Firmware documentó
en D-206: "archivos copiados por USB, biblioteca vacía en el
aparato") -- Rockbox solo refresca la base desde la pantalla
"Base de datos > Actualizar ahora" del navegador de carpetas, que
Metro no tiene por diseño. `metro_music_db_ready()` ahora dispara
`tagcache_start_scan()` una vez por arranque cuando la base YA es
usable (además de `tagcache_rebuild()` para el caso de base
inexistente, ya presente desde el primer commit de F4) -- mismo patrón
que `aura_music_db_ready()`, con el mismo cuidado de esperar a
`tagcache_is_fully_initialized()` antes de decidir (D-206: `is_usable()`
puede volverse verdadero antes de que la determinación de fondo
termine).

## M-033 — F5: el buffer de decodificación JPEG necesita margen real sobre el tamaño final, no solo `width*height*sizeof(fb_data)`

Bug real encontrado verificando la carátula de Now Playing: con
`s_scratch` dimensionado exactamente a `METRO_ALBUMART_SIZE^2 *
sizeof(fb_data)` (136×136×2 = 36 992 bytes), `read_jpeg_file()` con
`FORMAT_RESIZE` escribía **fuera de los límites** de `s_scratch` antes
de que su propio chequeo de tamaño lo detectara, corrompiendo la
siguiente variable estática del binario (`s_vol_overlay_until` en
`metro_screen_nowplaying.c`, un `long` de 8 bytes, terminó con un
valor de 51 539 607 555 — ni cero ni nada relacionado con volumen).
`JPEG_DECODE_OVERHEAD` (`apps/recorder/jpeg_load.h`) es ~39 KB por sí
solo, sin contar el espacio real que pide el downscale intermedio
antes de reducir a 136×136. Mismo hallazgo y misma fórmula que
Aura-Firmware ya documentó en `aura_albumart.c` (margen `x2` sobre el
tamaño final en bytes) tras un bug parecido (imagen fallando en
silencio, no corrompiendo memoria — la variante de Metro fue peor
porque el buffer nunca antes se había ejercitado con una carátula
real). `metro_albumart.c` reserva `METRO_ALBUMART_SIZE^2 * 2 * 2`
bytes ahora. Cualquier decodificador de imagen nuevo en `apps/metro/`
reserva scratch con el mismo margen, nunca el tamaño final a secas.

## M-034 — `firmware/tools/gen_test_media.sh`: las carátulas de prueba se generan vía PNG + `sips`, nunca `ffmpeg` directo a `.jpg`

Bug real encontrado en la misma verificación: incluso con `-pix_fmt
yuvj420p` (D-030, ya documentado), el encoder `mjpeg` de `ffmpeg`
produce archivos que el decodificador JPEG de Rockbox lee "con éxito"
(`read_jpeg_file()` devuelve `>0`, dimensiones correctas) pero **sin
color** — la crominancia se pierde y el resultado sale gris plano (el
nivel de gris coincide con la luminancia real del color pedido,
confirmado a mano: `RGB(202,119,51)` esperado, `RGB(139,137,139)`
decodificado — casi exactamente el luma de ese naranja). Verificado
que el archivo fuente SÍ tiene el color correcto (`sips -g all`,
Pillow) — el defecto está específicamente en cómo Rockbox interpreta
el JPEG que produce el encoder `mjpeg` de `ffmpeg`, no en el color de
origen. La misma imagen recodificada con Pillow o con `sips -s format
jpeg` decodifica perfecto. `gen_test_media.sh` genera ahora toda
carátula de prueba como PNG (sin subsampling, formato sin pérdida) y
la convierte a `.jpg` con `sips` — nunca `ffmpeg -f lavfi ... .jpg`
directo. `firmware/tools/gen_fonts.sh`/`gen_logo.py` no generan JPEG,
no les aplica.

## M-035 — `playlist_get_track_info()` devuelve 0 en éxito / -1 en error, no un booleano

Bug real encontrado armando la cola de "próximas" de Now Playing:
`if (!playlist_get_track_info(...))` se salta silenciosamente **cada
pista resuelta con éxito** (que devuelve `0`, falsy en C) y solo
"acierta" en el caso de error (`-1`, truthy) — exactamente al revés de
la intención. A diferencia de `tagcache_search()`/`find_albumart()`
(booleanos de verdad, `true`=éxito), esta función usa una convención
estilo `errno`: comparar explícitamente contra `!= 0`, nunca negar el
resultado directamente. Cualquier código nuevo en `apps/metro/` que
llame `playlist_get_track_info()` sigue esta misma comparación.

## M-036 — F6: `metro_music_db_ready()` cede mientras `metro_sync_job_active()`

Con `metro_sync.c` existiendo desde F6, hay dos caminos que podrían
llamar `tagcache_rebuild()`/`tagcache_update()` sobre la misma base:
el disparado por un marcador real de Aura Studio (`metro_sync.c`) y el
disparo de emergencia de `metro_music_db_ready()` (M-032, para cuando
nunca hubo marcador -- música copiada a mano por USB). Sin la guarda,
ambos podrían competir por la base en el mismo arranque. Regla: mientras
`metro_sync_job_active()` es verdadero, `metro_music_db_ready()` solo
devuelve `tagcache_is_usable()` sin tocar tagcache -- mismo criterio
que `aura_music_db_ready()` ya usa (`if (aura_sync_job_active()) return
tagcache_is_usable();`).

## M-037 — F9: gancho de una línea en `apps/gui/splash.c` para traducir mensajes que Metro no genera

Puerto directo del mecanismo de Aura-Firmware (`aura_splash_lang.c`,
D-055 en ese repo): `splash()`/`splashf()` del core de Rockbox son el
único punto por el que pasan los mensajes de tagcache ("Committing
database..."), de carga de playlist/plugin ("Loading..."), y de
apagado por batería baja — ninguno de esos call sites vive en
`apps/metro/`, así que traducirlos uno por uno significaría tocar
varios archivos del core. En cambio, `metro_splash_translate()`
(`apps/metro/metro_splash_lang.c`, código nuevo) reescribe el mensaje
YA resuelto (con sus argumentos dinámicos ya sustituidos por
`vsnprintf()`) contra una tabla de reglas conocidas, justo antes del
ajuste de línea de `splash()` — un solo gancho en `apps/gui/splash.c`
cubre los cuatro. Ver `MODIFICATIONS.md` F9.

## M-038 — F9: la pantalla USB de Metro no llega a verse en el simulador; `gui_usb_screen_run()` sigue dueño de la pantalla mientras dura la conexión

`default_event_handler()` (`apps/misc.c`) llama a `gui_usb_screen_run()`
(la pantalla USB nativa de Rockbox, `apps/gui/usb_screen.c`) de forma
bloqueante hasta que se desconecta el cable -- Metro no puede
interceptar ese bloqueo sin reimplementar el propio manejo de
almacenamiento masivo/HID, fuera de alcance y de riesgo real en
hardware. `metro_screen_usb_show()` dibuja un cuadro con wordmark +
"conectado" ANTES de llamar a `default_event_handler()` -- pero
verificado con el token `USB_INSERT` (M-039), `gui_usb_screen_run()`
hace su propio `lcd_update()` de inmediato, sin ceder el hilo, así que
en el simulador el cuadro de Metro nunca llega a mostrarse ni un solo
frame (`docs/screenshots/F9-usb.png` ya muestra el ícono nativo de
Rockbox). Se conserva de todas formas por ser inofensivo y porque
[ESTIMADO] podría verse brevemente en hardware real, donde el refresco
del LCD físico no es instantáneo -- sin confirmar, requiere el
dispositivo. Alcance más limitado que lo que Aura-Firmware documentó
para la misma superficie (`docs/superficies-rockbox.md` de ese repo,
fila "Pantalla USB": 🟡, con logo+wordmark SÍ visibles ahí porque
Aura modificó `usb_screen.c` directamente en vez de dibujar antes de
llamarlo). Ver `docs/DESVIACIONES.md` F9-1.

## M-039 — F9: token `USB_INSERT` en `METRO_SIM_BUTTONS` (`sim_tasks.c`)

Necesario para capturar `metro_screen_usb.c` de forma headless (mismo
mecanismo que toda otra captura de F0-F8): `METRO_SIM_BUTTONS` ya
soportaba botones reales y el token `WAIT`; agregar `USB_INSERT` que
llama `sim_trigger_usb(true)` directamente (declarado en
`sim_tasks.h`, ya usado por el menú interactivo del simulador) evita
inventar una variable de entorno nueva (`PLAN_MAESTRO.md` F9 proponía
`METRO_SIM_FORCE_USB=1`) cuando el mecanismo de inyección existente ya
alcanzaba con un token más. Ver `MODIFICATIONS.md` F9.

## M-040 — F10: la letra flotante del índice no se puede disparar por botones inyectados en el simulador headless

`metro_screen_list.c`'s índice flotante (S1.4) exige `steps >= 3`,
donde `steps` sale de `button_apply_acceleration(get_action_data())`
(`firmware/drivers/button.c`) -- una función que solo devuelve más de
1 cuando el bit 31 de `data` viene encendido con una velocidad de
rueda real. `uisimulator/common/sim_tasks.c` inyecta cada botón vía
`button_queue_post(codigo, 0)` (dato siempre 0, ver `MODIFICATIONS.md`
F0/F9): sin ese bit, `button_apply_acceleration()` devuelve 0 y
`metro_input.c` cae siempre a `steps=1`, sin importar cuántos
`SCROLL_FWD` se inyecten seguidos ni con qué separación. Verificado
bajando `METRO_INDEX_LETTER_MIN_STEPS` a 1 de forma temporal, tomando
la captura, y devolviéndolo a 3 antes de compilar la versión final --
mismo patrón que la instrumentación de depuración temporal de F4/F5,
nunca llega a un commit. El umbral real de 3 queda sin verificar
visualmente en este entorno (misma categoría que la pantalla USB de
F9, M-038): [ESTIMADO] correcto por inspección de código
(`metro_nav_move_sel()` recibe el mismo `steps` que ya usa para mover
la selección, así que si la selección salta 3+ filas la letra
correspondiente es la de la fila donde aterriza), pero requiere
hardware real (o una rueda simulada con velocidad, fuera de alcance
de este sim) para confirmar el disparo natural. Ver
`docs/DESVIACIONES.md` F10-1.

## M-041 — F10: `metro_draw_tile()` con etiqueta en blanco no dibuja nada, en vez de depender de un glifo de espacio

`metro_widgets_draw_empty_state()` reutiliza `metro_draw_tile()` para
el cuadro acento sin letra (S1.4), pasándole `" "` como `label` con la
intención de que el glifo de espacio no dibuje nada visible. La fuente
bitmap propia de `MFONT_DISPLAY` no tiene un glifo real para el
carácter espacio -- `lcd_putsxy()` cae a otro glifo de la tabla en su
lugar (visto en el simulador como una "I" mayúscula grande, sin
relación con el contenido esperado; capturado antes del fix
vaciando `Playlists/` para forzar la pivote vacía). Encontrado
verificando visualmente el estado vacío recién implementado, no antes
de esta fase porque `metro_draw_tile()` no tenía ningún llamador con
una etiqueta en blanco hasta ahora. Fix: `metro_draw_tile()` ahora
salta el `lcd_putsxy()` por completo cuando el carácter resuelto es
un espacio, en vez de confiar en que la fuente lo dibuje en blanco.

## M-042 — F10: iconos geométricos reales para batería/aleatorio/repetir, reemplazando los textos de M-018/F5-1

`metro_draw_battery()` pasa de texto "`N%`" a un ícono real (rectángulo
+ pestaña, relleno proporcional al nivel vía `lcd_fillrect`) -- mismo
ancho reservado en el encabezado (`metro_draw_header()`), sin cambios
ahí. Las insignias de texto "aleatorio"/"repetir todo" de Now Playing
(F5-1, deferidas explícitamente a F10) se reemplazan con
`metro_widgets_draw_shuffle_icon()` (trazos cruzados con puntas de
flecha) y `metro_widgets_draw_repeat_icon()` (cuadro-lazo con una
punta de flecha rompiendo la esquina superior derecha, más un "1"
superpuesto cuando `REPEAT_ONE` está activo) -- ambos construidos solo
con primitivas `lcd_drawline`/`lcd_drawrect`/`lcd_fillrect` (M-018:
nunca se lee un bitmap de disco), verificados visualmente en
`docs/screenshots/F10-np-icons.png`.
