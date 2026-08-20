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
e instalación manual. Escribe la clave `firmware_family: metro` en
`aura.cfg` — clave propia de Metro, sin equivalente en el contrato de
Aura-Firmware/Aura-Studio.

**Riesgo abierto, no resuelto en este proyecto**: si el usuario tiene
Aura Studio instalado con el `rockbox.ipod` de Aura embebido y conecta
un iPod con Metro, `AuraUpdateChecker.swift` comparará hashes, no
coincidirán, y ofrecerá "actualizar" — lo que sobrescribiría Metro con
Aura si el usuario acepta. Corregir esto requiere que `Aura-Studio/`
aprenda a leer `firmware_family` y no ofrezca esa acción — trabajo en
otro repositorio, fuera del alcance de este proyecto (regla de
contención de la carpeta padre). Documentado como advertencia en
`docs/ESTADO_FINAL.md`.

**Actualización R2-F1/DD-6 (M-056, no se borra esta entrada)**: la
frase original decía que Metro "no escribe `.rockbox/aura/version.txt`
(evita aparentar ser una versión de Aura)". Eso cambió --
`package_dist.sh --release-tag` ahora SÍ lo escribe (ver M-056),
porque con `PLAN-metro-r2-maestro.md` DD-13 Aura Studio va a distinguir
familias por el contenido real de `firmware_family` en vez de por la
mera presencia de `version.txt` -- una vez que eso exista (`ST-045`,
Aura-Studio, fuera de esta sesión), `version.txt` deja de "aparentar
ser Aura" y pasa a ser el único canal para que Studio sepa qué versión
de **Metro** hay instalada. El riesgo de este M-004 **sigue vigente
sin cambios para cualquier versión de Aura Studio anterior a ST-045**
-- esas versiones no saben leer `firmware_family` y seguirán
comparando por hash igual que hoy.

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

## M-043 — F11: qué transición corre se decide diffeando `metro_nav_t` en `metro_main.c`, no con llamadas explícitas desde cada pantalla

`PLAN_MAESTRO.md` F11 lista `metro_screen_list/hub/nowplaying` como
"mod: llaman a transiciones", sugiriendo que cada pantalla anuncia su
propia transición en el sitio donde empuja/saca una página. En cambio,
`metro_main.c` captura `metro_nav_depth()`/`metro_nav_pivot()`/"¿es
Now Playing la página actual?" ANTES de llamar al `*_handle()` de
turno, y vuelve a leer los tres DESPUÉS -- la diferencia decide sola
qué transición corre (profundidad sube y la página nueva es el
centinela de NP -> FADE; profundidad sube sin más -> PUSH; profundidad
baja saliendo específicamente de NP -> FADE; profundidad baja sin más
-> POP; misma profundidad con el pivot cambiado -> SLIDE; nada de lo
anterior -> redibujo instantáneo). `metro_screen_hub.c`,
`metro_screen_list.c` y `metro_screen_nowplaying.c` quedan intocados
salvo por lo que ya tenían de F1-F10 -- ninguno sabe que
`metro_transitions.c` existe.

**Por qué**: la lógica "¿esto fue un push, un pop, o solo cambiar de
pivot?" ya vive por completo en `metro_nav_t` (F3, `metro_nav.c`) --
triplicarla como una llamada explícita en cada uno de los ~15 sitios
que empujan o sacan una página (contando `metro_screen_hub.c` solo)
sería repetir la misma pregunta que `metro_nav_t` ya puede responder
por diferencia de estado, y cada sitio nuevo (un pivot futuro, una
página nueva) tendría que acordarse de anotarlo. Un único punto de
diff no puede desincronizarse del comportamiento real del stack de
navegación porque LEE ese stack, no lo declara aparte.

**Caso límite real, resuelto por el orden de las comparaciones**: el
centinela de Now Playing SÍ se empuja como cualquier otra página
(F5, `metro_screen_nowplaying_push()` llama a
`metro_screen_list_push()`), así que entrar a NP sube la profundidad
Y cambia a NP en el mismo evento -- de ahí que el chequeo específico
de NP tenga que ganarle al genérico de "profundidad subió" en el
orden de `if`, o "push(NP)" saldría con SLIDE en vez de FADE.
`MACT_OPTIONS` desde NP tiene la relación inversa: profundidad sube,
pero la página nueva NO es el centinela (es la lista de opciones) --
cae al PUSH genérico a propósito, ver `docs/DESVIACIONES.md` F11-1.

## M-044 — F11: `metro_draw_text_cut_right()` (F10) ignoraba el framebuffer redirigido de una transición, dejando filas en blanco

`metro_fb_render()` redirige todo el dibujo de `lcd_*` a un buffer
fuera de pantalla (`viewport_set_buffer(NULL, &fb, SCREEN_MAIN)`) para
prerrenderizar el destino de una transición ANTES de animarla.
`metro_draw_text_cut_right()` (F10, recorte de títulos largos) crea su
propio viewport temporal para el recorte con
`viewport_set_defaults(&vp, SCREEN_MAIN)`, que deja `vp.buffer = NULL`
-- resuelto SIEMPRE contra el framebuffer real de la pantalla
(`lcd_init_viewport()`), sin importar qué buffer esté activo en ese
momento. Mientras `metro_draw_text_cut_right()` solo se llamaba
durante un dibujo normal (pantalla real == framebuffer real) esto era
invisible; F11 lo expuso porque el primer llamador real fuera de ese
caso es exactamente el prerrenderizado offscreen de una transición --
el título de cada fila se dibujaba contra la pantalla real (invisible
en ese instante, luego pisado por los cuadros de la animación) en vez
del framebuffer offscreen, así que el `to` capturado nunca tenía
texto de fila. Encontrado comparando capturas con `animations=off`
(filas visibles) contra `animations=all` (filas en blanco tras un
PUSH/SLIDE ya asentado) -- mismo síntoma en cualquier pantalla de
lista, no específico de música. Fix: una línea,
`vp.buffer = lcd_current_viewport->buffer;` justo después de
`viewport_set_defaults()`, heredando el buffer que esté activo de
verdad en vez de forzar el real -- sigue siendo la misma garantía de
M-027 (nunca un puntero de basura de stack), solo que ahora copia uno
ya válido en vez de inventar `NULL`.

## M-045 — F11: matriz `animations` (off/minimal/all) × `graphics` (lite/full) -- semántica real, no solo persistencia

`metro_settings.animations`/`.graphics` existían desde F6/F8 como
enteros persistidos "sin efecto visual hasta F11+" (comentario textual
en `metro_settings.h`). F11 les da tipo (`enum metro_anim_level`,
`enum metro_gfx_level`) y efecto real:
`metro_transitions_slide()`/`_fade()` calculan un nivel EFECTIVO
(`metro_settings.animations` menos las muescas de auto-degradación de
sesión, M-015) y lo traducen a cuadros/retardo
(`all`=8×3 ticks, `minimal`=4×3, `off`=instantáneo, sin animación).
`graphics` solo importa para FADE: `metro_fb_present_fade()` (blend
por píxel) está reservado a `graphics=full`
(`PLAN_MAESTRO.md` S3.1: "Solo con `graphics=full`") -- con
`graphics=lite`, incluso con `animations=all`, FADE cae al mismo
SLIDE que ya usa PUSH/POP/twist en vez de un segundo mecanismo de
respaldo. Verificado visualmente: `docs/screenshots/F11-anim-off.png`
(mismo tick que F11-push-mid.png, destino ya asentado),
`docs/screenshots/F11-fade-lite-mid.png` (borde nítido de slide, no
mezcla de píxeles, con `graphics=lite`). Default cambiado de las
constantes provisionales de F8 (`animations=1`, `graphics=1`) a
`METRO_ANIM_DEFAULT`=ALL/`METRO_GFX_DEFAULT`=FULL (M-015: "canon =
máxima fidelidad") en `metro_settings.c` y en "restablecer ajustes"
(`metro_screen_settings.c`).

## M-046 — F12: geometría del turnstile (eje central, distancia focal, tabla offline) y el asentado final tras el último cuadro

`metro_fb_present_turnstile()`/`metro_fb_draw_turnstile_layer()`
modelan una rotación RÍGIDA de toda la pantalla alrededor de su propio
centro vertical (`x = LCD_WIDTH/2`), bajo una cámara de perspectiva
simple a distancia focal `D = LCD_WIDTH*1.2` — ni el eje ni `D` están
confirmados contra una fuente primaria de WP7 (`INVESTIGACION.md` F.3
solo documenta el rango de ángulo/duración del turnstile real, no su
geometría de proyección exacta), así que ambos son **[ESTIMADO]**,
elección de diseño propia. La tabla (32 ángulos × 320 columnas,
`metro_turnstile_xs`/`metro_turnstile_step`) se resuelve hacia atrás
(backward-mapping, sin huecos) con Python
(`firmware/tools/gen_turnstile_table.py`, nunca trigonometría en el
dispositivo — sin FPU, `INVESTIGACION.md` B.5/B.6) y se consume en
tiempo real solo con lookup + acumulador de paso fijo por columna,
técnica de PictureFlow (B.5). Cada cuadro se redondea al ÍNDICE de
tabla más cercano en vez de interpolar entre dos filas — con solo 8
cuadros de animación bajo `animations=all`, el error de precisión no
es perceptible, y ahorra tener que mezclar 320 pares de valores por
cuadro en el dispositivo.

**Bug real encontrado y arreglado**: como la tabla solo tiene 32
muestras discretas, el último cuadro de la animación (ángulo real
exactamente 0°) nunca cae justo en una muestra de la tabla — el
resultado proyectado es una aproximación cercana a la pantalla final,
no un píxel a píxel exacto. Como `FEATHER` (M-047) redibuja SOLO el
área de filas después, el encabezado/pivots se habrían quedado
ligeramente deformados para siempre (nada los vuelve a dibujar).
Encontrado visualmente comparando el estado asentado antes/después de
agregar un blit final directo (`lcd_bitmap_part` sin proyección) al
cierre de `run_turnstile()` — mismo patrón que
`metro_fb_present_slide()` ya logra de forma exacta en su último
cuadro (`dx == LCD_WIDTH`), pero que el turnstile no puede replicar
por construcción (tabla discreta, no álgebra exacta).

**Aliasing esperado, sin arreglar a propósito**: en ángulos cercanos a
los extremos del rango (-80°/50°), columnas muy comprimidas (`step`
grande) pueden mostrar texto con un patrón de rayas visible
(sub-muestreo sin filtro) — exactamente la técnica que
`INVESTIGACION.md` B.5 documenta de PictureFlow ("el único costo por
píxel es un shift y una comparación", sin antialiasing). Visible solo
durante los ~240-360ms de la animación, nunca en el resultado asentado
(el blit final de arriba lo reemplaza por completo) — no vale la pena
un filtro de resampleo para un efecto que dura una fracción de
segundo. Ver `docs/screenshots/F12-turnstile-mid.png`.

## M-047 — F12: FEATHER como cascada por fila en `metro_screen_list.c`, no como transición de framebuffer completo

A diferencia de SLIDE/PUSH/POP/FADE (todas en `metro_transitions.c`,
operando sobre framebuffers completos de 320×240), FEATHER anima FILAS
INDIVIDUALES de una sola página de lista -- vive en
`metro_screen_list.c` en su lugar, con su propio bucle bloqueante de 6
cuadros que solo redibuja el área de filas
(`metro_draw_clear_rows_area()` + `metro_draw_rows_ex()` con un
arreglo de offsets Y, uno por fila visible). Escalonado de 1
cuadro/fila (fila N no empieza a caer hasta el cuadro N), cada fila
usando `metro_ease(OUT_QUAD, local_frame+1, local_total)` sobre los
cuadros que le quedan en el presupuesto -- así la fila SIEMPRE llega
exactamente a offset 0 en su propio último cuadro
(`metro_ease()`'s `i==frames` → 256) sin importar cuán pocos cuadros
le tocaron (incluso la última fila visible, con un solo cuadro
asignado). Disparado por una bandera (`s_feather_pending`) que
`metro_screen_list_push()` levanta en CADA push (incluido el del
centinela de Now Playing, que nunca la consume porque esa rama
siempre termina en FADE -- la siguiente vez que se empuje una lista de
verdad, la bandera ya se volvió a levantar de todas formas, así que no
hay arrastre real de estado viejo). Gate propio, más estricto que
PUSH/POP/twist: solo corre bajo el nivel EFECTIVO `all`
(`metro_transitions_effective_all()`, nueva función expuesta desde
`metro_transitions.c` para no duplicar el estado de auto-degradación
de sesión de M-015 en dos módulos) -- "off" bajo `minimal` y `off`,
igual que dicta la tabla de `PLAN_MAESTRO.md` S3.3.

## M-048 — F12: fondo de carátula atenuada en Now Playing -- buffer de ~300 KB, no 150 KB, misma razón que M-033

`metro_albumart.c` gana un segundo par caché/buffer
(`s_bg_scratch`/`s_bg_loaded_path`) independiente del tile pequeño ya
existente, decodificando la MISMA carátula a `LCD_WIDTH x LCD_HEIGHT`
en vez de `METRO_ALBUMART_SIZE`. `PLAN_MAESTRO.md` S3.3 estima este
buffer en "150 KB" (el tamaño final `320*240*2` sin margen) pero M-033
(F5) ya estableció, con un bug de corrupción de memoria real como
prueba, que `FORMAT_RESIZE` necesita espacio para el decode NATIVO
completo antes de reducir la imagen -- no solo el tamaño final. Este
buffer usa la misma fórmula de margen `*2*2` que el buffer del tile
pequeño (`METRO_ALBUMART_SCRATCH_SIZE`), dando ~300 KB reales, no 150.
Trivial sobre 64 MB de RAM total (`INVESTIGACION.md` B.3 ya lo
calificaba de "insignificante" incluso a 150 KB), así que la prioridad
es la misma que en M-033: seguridad de memoria por encima de fidelidad
al número exacto del plan. La composición en sí
(`metro_fb_blend_over_color()`, mismo blend por píxel que
`metro_fb_present_fade()`) NO llama `lcd_update()` -- a diferencia de
toda otra función `present_*`/`blend_*` de `metro_fb.c`, esta es un
fondo ESTÁTICO con más dibujo encima (tile pequeño, texto, controles)
antes de un único `lcd_update()` al final de
`metro_screen_nowplaying_show()`, no el único paso de una animación
por cuadro.

## M-049 — F13: `METRO_TRACE` gana `logf()` -- `DEBUGF` solo es visible en el simulador, nunca en hardware real sin equipo adicional

`PLAN_MAESTRO.md` S3.4 pide `METRO_TRACE` para medir el costo real de
cada transición en hardware (F13). Investigando cómo verla ahí se
encontró que `DEBUGF` (`firmware/debug.c`, target nativo) formatea el
mensaje a un buffer local y llama a `debug()` -- una función
`static inline` que **no hace nada** (`(void)msg;`) salvo que el
target tenga su propio `HAVE_GDB_API`/`breakpoint()` conectado a un
depurador JTAG real. En el simulador SÍ se ve (siempre va a `stderr`,
`firmware/target/hosted/debug-hosted.c`, ya usado para verificar F11 —
`docs/DESVIACIONES.md` no tiene entrada porque no era un hallazgo
nuevo ahí, solo una comprobación), pero en un iPod real la promesa del
plan de "medir en hardware con METRO_TRACE" habría sido letra muerta
sin cambiar nada. Rockbox sí tiene un mecanismo pensado exactamente
para esto: `logf()` (`firmware/export/logf.h`), un buffer circular de
16 KB en RAM (`ROCKBOX_HAS_LOGF`, definido para `ipod6g`) visible
desde el propio dispositivo, sin cable ni depurador -- menú de
depuración → "Show Log File" (`apps/debug_menu.c`, `logfdisplay`).
Requiere `#define LOGF_ENABLE` ANTES de `#include "logf.h"` en CADA
archivo que quiera usarlo de verdad (si no, `logf()` se compila a
no-op en ese archivo -- el propio header lo hace así para que el
buffer no se llene con el logging de todo el firmware a la vez);
`metro_transitions.c` es el único archivo de Metro que lo necesita, así
que es la única definición. `METRO_TRACE` ahora llama a ambos
(`DEBUGF` para el simulador, `logf` para hardware real) -- ver
`docs/ESTADO_FINAL.md` para el procedimiento completo de medición.

## M-050 — F13: `tools/version.sh` marca "M" (árbol sucio) incluso con todo comiteado -- estructura del repo, no un bug de Metro

`firmware/rockbox/tools/version.sh` decide si el árbol está "sucio"
corriendo `git -C "$1" diff --name-only HEAD` con
`GIT_WORK_TREE="$1"`, donde `$1` es la ruta al código fuente de
Rockbox (`firmware/rockbox/`, pasada como `ROOTDIR` desde
`firmware.make`). Ese mecanismo asume que esa ruta ES la raíz del
repo git -- cierto en un checkout de Rockbox standalone, **falso**
en Metro-Aura: el repo real es la carpeta padre completa (`DECISIONS.md`
M-006 y ss., todo el árbol de fases vive ahí). Con `GIT_WORK_TREE`
apuntando a un subdirectorio en vez de la raíz real, el comando falla
(`fatal: this operation must be run in a work tree`) y el resultado se
interpreta como "sucio" -- comprobado generando un release con el
árbol perfectamente limpio (`git status --porcelain` vacío) y viendo
igual una "M" en `rockbox-info.txt`. No es un bug introducido por
Metro-Aura ni algo que `apps/metro/` pueda arreglar -- es cómo
`tools/version.sh` fue escrito, asumiendo la estructura de un checkout
de Rockbox suelto.

**Fix, sin tocar ningún archivo de Rockbox**: `firmware.make` ya
respeta una variable `VERSION` pasada por línea de comandos con
prioridad sobre el cálculo de git (`SVNVERSION:=$(shell VERSION='$(VERSION)'
...)`, y `version.sh` mismo: `if [ -z $VERSION ]; then ... fi`).
`firmware/tools/build_target.sh` ahora propaga una `VERSION` opcional
del entorno a cada `make`; `firmware/tools/package_dist.sh` la calcula
correctamente ANTES de invocar `build_target.sh` -- hash corto real +
fecha, con la "M" solo si `git status --porcelain` (corrido desde la
raíz real del repo, sin el `GIT_WORK_TREE` mal alineado) dice de
verdad que hay cambios sin commitear. Mismo patrón que
Aura-Firmware's D-296/D-297 documentó como aceptado sin diagnosticar
la causa raíz (`package_dist.sh` de ese repositorio solo bloquea el
release si el árbol está sucio, nunca corrige el string en sí) -- acá
se corrigió la causa en vez de solo evitarla.

## M-051 — R2-F1: todo texto de `apps/metro/` dibuja bajo `DRMODE_FG`, nunca el `DRMODE_SOLID` por defecto de Rockbox

`INVESTIGACION-metro-r2.md` E.1 confirmó la causa raíz de las "placas
negras" que tapaban el cover art en Now Playing: `metro_draw.c` nunca
llamaba a `lcd_set_drawmode()`, así que cada `lcd_putsxy()` heredaba el
`DRMODE_SOLID` por defecto de Rockbox -- cada glifo se dibuja con una
caja de fondo opaca (`fg_pattern`/`bg_pattern` del viewport), tapando
lo que sea que estuviera detrás en vez de leerlo como transparente.

**Fix**: `metro_draw_text()` fija `lcd_set_drawmode(DRMODE_FG)` justo
antes de `lcd_putsxy()`; `metro_draw_text_cut_right()` fija
`vp.drawmode = DRMODE_FG` en el viewport temporal que ya arma para el
recorte de texto (M-027); `metro_draw_tile()` (letra inicial de un
tile) fija `DRMODE_FG` inline antes de su `lcd_putsxy()` propio. Las
tres llamadas directas a `lcd_putsxy()` que quedaban fuera de
`metro_draw_text()` (`metro_draw_header()` x2, `metro_draw_pivots()` y
el dígito del ícono de repetir en `metro_widgets.c`) ahora se canalizan
por `metro_draw_text()` en vez de duplicar la lógica de drawmode --
simplifica el llamador y garantiza el mismo comportamiento en un solo
lugar. `metro_main()` fija el mismo baseline una vez, justo después de
`metro_fonts_init()` (el LCD arranca en `DRMODE_SOLID` y nada dibujó
texto todavía en ese punto).

**Desviación del plan** (`PLAN-metro-r2-maestro.md` DD-1): el plan
asumía que las llamadas a `plugin_load()` vivían dentro de
`metro_main.c` y pedía restaurar `DRMODE_FG` ahí "al volver de
cualquier `plugin_load()`". En el código real esas llamadas están en
`metro_photos.c` (`imageviewer.rock`) y `metro_video.c`
(`mpegplayer.rock`), no en `metro_main.c` -- cada una restaura
`DRMODE_FG` inline justo después de su propio `plugin_load()`, antes
de que `metro_transitions_fade()` vuelva a dibujar la lista. Ver
`docs/DESVIACIONES.md` R2-F1-1 para el detalle completo.

**Por qué no restaurar `DRMODE_SOLID` en algún punto**: ningún dibujo
de `apps/metro/` quiere una caja de fondo opaca a propósito -- el
fondo ya se pinta explícitamente donde hace falta (`lcd_fillrect()` en
`metro_draw_tile()`, `metro_draw_clear_rows_area()`, etc.) antes de
dibujar el texto encima, así que no hay ningún caso real que necesite
volver a `SOLID`.

## M-052 — R2-F1/DD-2: `METRO_NP_BG_ALPHA256` se mantiene en 77 (30%) tras verificar contra una carátula casi blanca

`INVESTIGACION-metro-r2.md` E.4 estimaba, sin observación real, que el
peor caso de contraste para la línea de texto terciaria (álbum) de Now
Playing rondaría ~25/255 (~10%) contra una carátula casi blanca --
suficiente para preocupar por legibilidad. `PLAN-metro-r2-maestro.md`
DD-2 pedía generar esa carátula, capturar el tema oscuro real y decidir
entre mantener 77 o bajar a 51 (20%) según lo que se viera.

**Qué se hizo**: `gen_test_media.sh` ahora genera una carátula
`0xF2F2EC` (casi blanca) para el álbum "Wheel & Click/Analog Dreams"
(ver el comentario nuevo ahí). Se capturó Now Playing reproduciendo un
tema de ese álbum en el simulador, tema oscuro, `graphics=full`:
`docs/screenshots/R2-F1-np-worstcase.png`. Se comparó contra
`docs/screenshots/R2-F1-np-clean.png` (carátula naranja `0xCC7733`,
"First Light", caso típico).

**Resultado**: la línea terciaria ("Analog Dreams") queda legible en
ambas capturas. La razón, visible al mirar el propio código de
`metro_screen_nowplaying_show()`: `metro_fb_blend_over_color()` mezcla
la carátula sobre `metro_color_bg()` (un azul-marino oscuro) al 30% --
incluso con una carátula casi blanca (~242 de luminosidad), el
compuesto queda dominado por el 70% del fondo oscuro
(`0.3*242 + 0.7*25 ≈ 90`, un gris medio-oscuro, no un blanco real). La
estimación de E.4 asumía implícitamente comparar el texto contra la
carátula a color pleno, no contra su versión ya atenuada al 30% -- el
mecanismo de blend en sí ya actúa como salvaguarda de contraste antes
de llegar al texto.

**Decisión**: se mantiene `METRO_NP_BG_ALPHA256 = 77`, sin bajar a 51.
Ver `docs/DESVIACIONES.md` no aplica aquí -- no hubo desviación del
plan, DD-2 preveía explícitamente ambos desenlaces ("si es legible,
mantener 77; si no, bajar a 51") y este es el desenlace "mantener".

**Nota lateral, fuera de alcance de R2-F1**: verificando estas capturas
se encontró que el índice de fila seleccionado en pantalla NO siempre
coincide con la pista que efectivamente empieza a sonar al reproducir
desde "Canciones" (todas) o desde una lista de canciones de álbum --
confirmado con trazas temporales (`songs_on_select` recibe el índice
correcto, pero `metro_music_play_all_songs()`/
`metro_music_play_songs_of_album()` terminan reproduciendo una pista
distinta a la de esa fila). Es un bug real y reproducible en
`metro_music.c` (`insert_matching_tracks()` y su interacción con
`playlist_start()`), preexistente a esta ronda (F4/F6), no introducido
por DD-1/DD-2 ni relacionado con ellos. No se investigó ni se corrigió
aquí -- fuera del alcance declarado de R2-F1. Reportado al dueño en el
resumen de esta fase para decidir cuándo abrirlo como su propia fase o
ítem de trabajo.

## M-053 — R2-F1/DD-3: `metro_fsutil_list_by_ext()` descarta directorios vía `dir_get_info()`/`ATTR_DIRECTORY`

`INVESTIGACION-metro-r2.md` B.3 confirmó empíricamente que
`metro_fsutil_list_by_ext()` no distinguía directorios de archivos --
un directorio literalmente llamado `Folder.jpg` (herramientas de
escritorio de terceros lo crean) aparecía en la lista de fotos como si
fuera una imagen.

**Fix**: mismo patrón que `apps/filetree.c` (línea ~201) ya usa para su
propio escaneo filtrado por extensión -- `dir_get_info(d, entry)` por
cada entrada, y se descarta si `info.attribute & ATTR_DIRECTORY`.
`fs_attr.h` (que define `ATTR_DIRECTORY`) llega transitivo vía
`dir.h`, ya incluido en `metro_fsutil.c` -- sin includes nuevos.

**Verificado**: se recreó el caso de repro exacto de B.3 --
`Photos/Folder.jpg/` (directorio) junto a los fixtures reales de fotos
en el simulador -- y se capturó la lista "todos" de Fotos
(`docs/screenshots/R2-F1-photos-dirfilter.png`): el directorio no
aparece, solo los 6 archivos de imagen reales quedan listados.

## M-054 — R2-F1/DD-4: `metro_ensure_media_dirs()` crea `/Music`, `/Videos`, `/Photos`, `/Playlists` si faltan

Un disco que nunca pasó por un sync real de Aura Studio (música
copiada a mano por USB, o un iPod recién formateado) puede no tener
ninguna de las cuatro carpetas de medios de nivel superior que
`docs/contracts/library-layout-v1.md` (Aura-Firmware, consultado en
solo lectura) define -- `/Music/`, `/Videos/`, `/Photos/`,
`/Playlists/`. Sin ellas, las pantallas de Videos/Fotos muestran listas
vacías sin explicación y guardar una lista de reproducción nueva
fallaría en silencio.

**Qué se hizo**: `metro_ensure_media_dirs()` (`metro_settings.c`, único
módulo autorizado por `CLAUDE.md` a construir rutas del contrato) crea
las cuatro carpetas con `mkdir()` si `dir_exists()` dice que falta cada
una -- mismo patrón que `metro_settings_save()` ya usa para
`METRO_DIR`. Se llama desde `metro_disk_handoff()` en `metro_main.c`,
el único punto (arranque + cada vuelta de USB, comentario ya existente
ahí) donde el firmware "recupera" el disco.

**Verificado**: se apartaron las cuatro carpetas del simulador
(simulando un disco recién montado sin ellas), se arrancó el
simulador, y las cuatro reaparecieron vacías tras el boot antes de
restaurar los fixtures reales.

## M-055 — R2-F1/DD-5: `metro_manifest_t` parsea las 13 claves reales de `sync_summary.cfg`, no solo 3

Aura Studio escribe el mismo `sync_summary.cfg` de 13 claves para
cualquier dispositivo, sea Aura o Metro (`CatalogSummaryWriter`, no
distingue por `firmware_family`) -- pero `metro_manifest_t` solo leía
`music_count`/`video_count`/`photo_count`, ignorando las otras 10
claves reales que Studio ya escribe hoy (`music_bytes`, `video_bytes`,
`photo_bytes`, `playlist_count`, y el desglose por categoría de
video/foto).

**Qué se hizo**: `metro_manifest_t`/`metro_manifest_load()`
(`metro_manifest.c/.h`) ahora replican el mismo conjunto de campos que
`aura_manifest_t` de Aura-Firmware (consultado en solo lectura) --
incluyendo el parser `parse_i64()` propio (mismo motivo que el de Aura:
`atoll()` no está disponible en todos los targets de este árbol) y las
banderas `has_video_categories`/`has_photo_categories`, derivadas de
si las claves de desglose aparecieron en el archivo (no son claves en
sí mismas) -- un manifiesto escrito antes de que Studio agregara el
desglose no las tiene, y el firmware debe distinguir eso de "el conteo
real es cero".

`metro_screen_about.c` (`about_count()`/`about_get_row()`) agrega filas
para `playlist_count` (siempre que haya sync) y, condicionalmente, las
6 filas de desglose (películas/series/videoclips si
`has_video_categories`; imágenes/fotografías/IA si
`has_photo_categories`) -- mismo orden que la pantalla Acerca de real
de Aura-Firmware (`aura_screens.c`, consultada en solo lectura),
adaptado al motor de filas genérico de Metro (Aura dibuja esa pantalla
a mano; Metro reutiliza su lista de filas existente, sin copiar el
motor de dibujo de Aura). 7 strings nuevos ES/EN en `metro_lang.c`
(`LANG_ABOUT_PLAYLISTS`/`MOVIES`/`SERIES`/`CLIPS`/`IMAGES`/
`PHOTOS_TAKEN`/`AI`), minúsculas sin acentos -- mismo estilo que el
resto de `metro_lang.c` (ninguna cadena existente usa tildes).

**Verificado**: `sync_summary.cfg` de 13 claves colocado a mano en el
simulador (conteos: 21 canciones, 5 videos, 8 fotos, 1 lista, desglose
1/1/3 de video y 1/6/1 de foto) -- capturas
`docs/screenshots/R2-F1-about-full.png`,
`R2-F1-about-full-scrolled.png`, `R2-F1-about-full-end.png` muestran
las 13 filas de conteo más "basado en rockbox" al final, en el orden
correcto.

## M-056 — R2-F1/DD-6: `package_dist.sh --release-tag` escribe `.rockbox/aura/version.txt`, mismo contrato que Aura-Firmware

**Nota de numeración**: `PLAN-metro-r2-maestro.md` DD-6 sugería "Decisión
M-052" para este ítem, escrito en Fase 3 antes de saber el orden real
de ejecución de Fase 4. `DECISIONS.md` es una bitácora cronológica, no
una spec (encabezado del archivo) -- M-052 ya quedó asignado a la
decisión de DD-2 (la primera que necesitó número nuevo tras M-051 en
el orden real de ejecución). Este ítem es M-056, siguiente libre.

`package_dist.sh` acepta `--release-tag vX.Y.Z` como primer/segundo
argumento -- mismo contrato exacto que el `package_dist.sh` de
Aura-Firmware (consultado en solo lectura, D-297): exige árbol git
limpio (`ERROR` + `exit 1` si `--release-tag` se combina con cambios
sin commitear, ANTES de compilar nada), y solo con la bandera escribe
`$RELEASE_TAG` literal en `.rockbox/aura/version.txt` dentro del
`$STAGE` que arma `rockbox.zip`, justo antes del `zip -qr` (mismo punto
que Aura). Sin la bandera (build de desarrollo, uso normal) el archivo
no se escribe -- su ausencia sigue siendo una señal válida en sí misma.

**Verificado**:
- Guardia de árbol sucio: `package_dist.sh --release-tag v0.2.0-test`
  corrido con los 25 archivos de esta misma fase sin commitear ->
  `ERROR: hay cambios sin commitear -- un --release-tag necesita el
  árbol limpio`, `exit 1`, sin compilar nada -- confirmado en vivo.
- Build de desarrollo (sin la bandera): corrida completa real
  (`build_target.sh` + bootloader + `make zip` + `mks5lboot` +
  `rockbox.zip` armado + centinelas + checksums, terminó en
  `firmware/dist/`) -- confirmado que `.rockbox/aura/version.txt`
  **no** aparece en `rockbox.zip` (`unzip -l` sin esa entrada).
- Escritura de `version.txt` en sí (el bloque `mkdir -p`/`echo >`
  dentro de `$STAGE`): verificado en aislamiento contra un directorio
  de staging de prueba -- contenido exacto `v0.2.0-test` escrito en
  `$STAGE/.rockbox/aura/version.txt`. **No verificado end-to-end**
  dentro de una corrida real de `package_dist.sh --release-tag` sobre
  árbol limpio: probar eso exige que `package_dist.sh` mismo (el
  archivo que esta misma decisión modifica) ya esté comiteado --
  imposible antes del commit de cierre de R2-F1. Queda como
  verificación pendiente natural la primera vez que R2-F5 corra un
  release real (`v0.2.0`, con el árbol ya limpio en ese punto).

## M-057 — R2-F2/DD-7,8,9: cuadrícula de fotos (modelo declarativo + geometría 4×80 + caché propia de miniaturas)

**Nota de numeración**: `PLAN-metro-r2-maestro.md` sugería "M-053" para
DD-9 -- ese número ya quedó tomado por DD-3 (R2-F1) en el orden real de
ejecución, mismo caso que M-056 ya documentó. Bitácora cronológica, no
spec (encabezado de este archivo).

**DD-7 -- modelo extendido, no pantalla centinela**: `struct metro_pivot`
(`metro_page.h`) gana dos campos **al final** -- `int tile_cols;` (0 =
lista de filas, el comportamiento de siempre) y
`const fb_data *(*get_tile)(void *ctx, int index);` (NULL = tile de
acento con inicial, `metro_draw_tile()`). Todo inicializador posicional
existente en el árbol sigue compilando -- los dos campos nuevos caen en
0/NULL por defecto (warnings `-Wmissing-field-initializers` esperados y
aceptados, no tratados como error). `metro_screen_list_show()` llama
`metro_draw_tiles()` (nuevo, `metro_draw.c`) cuando `tile_cols > 0`;
`metro_screen_list_handle()` usa `metro_nav_move_sel_grid()` (nuevo,
`metro_nav.c`) en vez de `metro_nav_move_sel()` para ese mismo caso.
FEATHER (F12) y el índice flotante (F10) se omiten para pivots de
cuadrícula -- ninguno de los dos tiene sentido en un grid, ver los
comentarios en `metro_screen_list.c`.

**DD-8 -- geometría (320×240, header+pivots hasta y=84)**: 4 columnas ×
tiles de 80×80, sin separación, al ras de los bordes
(`METRO_TILE_SIZE`/`METRO_TILE_COLS`/`METRO_TILE_ROWS_VISIBLE`,
`metro_draw.h`). Fila 1 en y=84, fila 2 en y=164 -- se corta sola en
y=240 (asoma 76 de 80px), sin necesitar una fila extra de "peek" como
`metro_draw_rows_ex()`. Selección: borde de 3px `metro_color_accent()`
**dentro** del tile (no lo agranda). Verificado visualmente:
`docs/screenshots/R2-F2-photos-grid.png` (8 tiles, borde de selección,
segunda fila asomando cortada).

**DD-9 -- caché de miniaturas (`metro_photo_thumbs.c/.h`, nuevo)**:
- Directorio propio `.../aura/metrocache/photos/` (NO el `photocache/`
  de Aura -- formato/tamaño distintos, y la limpieza de convivencia lo
  borraría de todas formas). Ruta armada solo en
  `metro_settings_metro_cache_dir()` (`metro_settings.c`), regla de
  rutas de contrato del `CLAUDE.md`.
- Formato: `fb_data` crudo 80×80 (12 800 B/foto), nombre
  `<archivo>.<mtime>.mth` -- la mtime en el nombre ES la invalidación.
  `metro_photo_item_t` (`metro_photos.h`) gana un campo `mtime`, ya
  disponible gratis: `metro_fsutil_list_by_ext()` ya llamaba
  `dir_get_info()` por cada entrada (M-053, R2-F1) para el chequeo de
  `ATTR_DIRECTORY` -- se agregó `metro_fsutil_list_by_ext_mtime()`
  (variante con un `out_mtimes[]` opcional) en vez de un segundo scan.
- Generación bajo demanda, presupuestada: `metro_photo_thumbs_get()`
  devuelve de inmediato lo que ya esté en la ventana RAM (16
  miniaturas, ~205KB) o en caché de disco (lectura cruda, no
  decodificación -- barata, se intenta síncrona); si no hay nada,
  encola el nombre y devuelve `NULL` (el llamador cae al tile de acento
  con inicial). `metro_photo_thumbs_tick()`, llamado una vez por
  iteración ociosa de `metro_main()` (mismo poll que el redibujado del
  índice flotante de F10), decodifica **como máximo una** miniatura
  pendiente por llamada y pide redibujo solo si de verdad decodificó
  algo. Verificado: `docs/screenshots/R2-F2-photos-grid-loading.png`
  (8 placeholders con inicial, caché de disco vacía a propósito) vs.
  `R2-F2-photos-grid.png` (mismos 8, ya decodificados) tras más ticks.
- **"Cubrir" sin probe de dimensiones JPEG**: en vez de portar el
  `probe_jpeg_dimensions()` propio de Aura (`aura_photos.c`, parser de
  cabecera JPEG a mano, sin equivalente en Rockbox stock) para calcular
  el recorte de cobertura de antemano, se decodifica UNA vez con
  `read_jpeg_file(FORMAT_NATIVE|FORMAT_RESIZE|FORMAT_KEEP_ASPECT)` a un
  scratch de `80*80*2*2` bytes (margen M-033) -- el resultado siempre
  trae una dimensión exactamente en 80 y la otra ≤80 -- y luego
  `cover_crop()` (`metro_photo_thumbs.c`) hace un remuestreo nearest-
  neighbor puramente entero desde ESE bitmap ya chico hasta el recorte
  centrado 80×80 final, sin una segunda decodificación de mayor
  resolución. Sacrifica algo de nitidez en el eje recortado a cambio de
  quedarse en una sola decodificación barata -- aceptable a 80px; el
  visor propio de fotos (R2-F3, DD-10) sí necesita precisión real ahí
  y por eso ese sí lee el algoritmo Q16.16 de `aura_photos.c` como
  referencia, esta miniatura no.
- Limpieza de huérfanos: `remove_stale()` corre solo en el momento de
  una decodificación real (ya es el camino lento) -- escanea el
  directorio de caché una vez y borra cualquier otro `.mth` con el
  mismo nombre base que el que se acaba de escribir. Verificado en
  vivo: se tocó `beach.jpg` (mtime nuevo, simulando un re-sync),
  se re-entró a la cuadrícula, y el `.mth` viejo desapareció al
  terminar de decodificar el nuevo -- solo queda un archivo por foto.
- Persistencia entre arranques verificada: mismos mtimes de archivo
  exactos en los 14 `.mth` antes/después de un segundo arranque
  completo del simulador entrando a la cuadrícula -- ninguno se
  regeneró.

**Fixtures**: `gen_test_media.sh` pasa de 5 a 14 fotos `.jpg` reales
(sigue habiendo `.bmp`/`.png` no soportados, y una sin categoría a
propósito) -- "todos" abarca casi 2 pantallas completas de la
cuadrícula (14 > 8 tiles/pantalla), demostrando el asoma de la segunda
fila. `photo_categories.cfg` reparte las nuevas entre las 3 categorías.

**`test_nav.c`**: 5 casos nuevos para `metro_nav_move_sel_grid()` --
alineación de `first_visible` a filas, clamp en ambos extremos, `count`
no múltiplo de `cols` (21/4, caso real de biblioteca), lista vacía. 89
checks totales, 0 fallos (`metro_nav_move_sel()` intacto, cero
regresión en los casos ya existentes).

## M-058 — R2-F3/DD-10: visor propio de fotos (`metro_screen_photo_viewer.c`), reemplaza `imageviewer.rock`

**Motivación real, no solo la del plan**: verificando R2-F1/R2-F2 en
sesión interactiva, el dueño reportó que el simulador "se traba" al
abrir una foto desde la cuadrícula, sin poder salir. Investigado antes
de escribir código nuevo: no es un cuelgue -- `imageviewer.rock`
(plugin nativo, sin tocar) usa un esquema de botones propio para el
pad clickwheel del iPod (`apps/plugins/imageviewer/imageviewer_button.h`,
`IPOD_4G_PAD`/`IPOD_3G_PAD`/`IPOD_1G2G_PAD`): `IMGVIEW_QUIT = (BUTTON_SELECT
| BUTTON_REL)`, no MENU -- `MENU` está remapeado dentro del plugin a
`IMGVIEW_UP` (paneo). El plugin seguía vivo y esperando el botón
correcto; solo era indistinguible de un cuelgue real para quien espera
la convención de Metro (MENU = volver). El cuadro "Cargando..." que
también se reportó es UI nativa de Rockbox dentro de ese mismo plugin,
no de Metro. R2-F3 ya estaba planeado como la fase siguiente -- resulta
ser también el fix.

**Qué se hizo** (patrón centinela, igual que Now Playing F5):
- `metro_screen_photo_viewer.c/.h` (nuevos): página centinela en la
  pila (`metro_screen_photo_viewer_is_current()`), dibujo/input
  propios. `metro_screen_hub.c`'s `photo_pivot_on_select()` empuja el
  visor pasándole el MISMO array/`count` que ese pivot ya usa para
  `get_row()`/`get_tile()` -- SCROLL_FWD/BACK dentro del visor navegan
  la categoría exacta desde la que se entró, no "todos" sin importar
  el pivot de origen.
- `MCTX_VIEWER` nuevo en `metro_keymap.c/.h`: SCROLL_FWD/BACK reutilizan
  `MACT_PREV`/`MACT_NEXT` (mismo significado que en cualquier lista);
  SELECT es la única acción nueva, `MACT_TOGGLE_VIEW_MODE` (ajustar
  ↔ cubrir); MENU/MENU-sostenido/PLAY reutilizan `MACT_BACK`/`MACT_HOME`/
  `MACT_PLAYPAUSE` sin cambios.
- `metro_main.c`: `at_viewer` se suma a `at_root`/`at_player` en el
  bucle principal y en el diff de `metro_nav_t` que elige la transición
  (M-043) -- entrar/salir del visor siempre usa FADE, igual que Now
  Playing; ambos centinelas son mutuamente excluyentes así que
  `player_x || viewer_x` nunca es ambiguo. El motor de miniaturas
  (R2-F2/DD-9) y el redibujado del índice flotante (F10) se excluyen
  también mientras `at_viewer` -- ninguno tiene nada que hacer sobre
  una foto a pantalla completa.
- **Ajustar**: `FORMAT_NATIVE|FORMAT_RESIZE|FORMAT_KEEP_ASPECT` directo
  a una caja de 320×240, centrado, franjas del `metro_color_bg()` del
  tema activo.
- **Cubrir**: se leyó `aura_photos.c:943-1063` de Aura-Firmware como
  referencia de algoritmo (solo lectura, código propio escrito para
  Metro) -- `probe_jpeg_dimensions()` (parseo de marcadores JPEG hasta
  SOFn, sin decodificar un solo píxel, portado con nombres/comentarios
  de Metro) da el tamaño real de origen; `compute_decode_and_display_size()`
  calcula en Q16.16 (sin FPU) el factor `max(320/w, 240/h)` -- cubrir,
  no ajustar -- y decide si decodificar directo al tamaño final o al
  tamaño de origen (según si haría falta agrandar); `draw_scaled_centered()`
  hace el recorte centrado final por muestreo nearest-neighbor cuando
  el tamaño decodificado no coincide exactamente con el de pantalla.
  Al alternar modo se re-decodifica siempre (`s_loaded_index = -1`),
  nunca se re-muestrea un buffer viejo.
- Buffer propio `METRO_PHOTO_VIEW_SCRATCH_SIZE = LCD_WIDTH*LCD_HEIGHT*2*2`
  (~300KB, margen ×2 de M-033), no compartido con el de Now Playing
  (`metro_albumart.c`) -- ambos pueden ser el buffer más grande de su
  propia pantalla, pero Now Playing y el visor son mutuamente
  excluyentes, nunca compiten al mismo tiempo.
- `imageviewer.rock` sigue en el árbol sin tocar (`apps/plugins/imageviewer.make`
  sigue compilando el `.rock`) -- Metro solo dejó de lanzarlo.
  `metro_photos_view()` (la función que lo hacía, `metro_photos.c`) se
  eliminó por completo, sin llamadores.

**Bug real encontrado y corregido verificando esto mismo, no un puerto
directo de Aura**: `compute_decode_and_display_size()` (el cálculo de
memoria de Aura, portado primero tal cual) solo comparaba
`decode_w*decode_h*sizeof(fb_data)` contra el tamaño del scratch --
probado en vivo contra un fixture 640×300 en modo cubrir,
`read_jpeg_file()` devolvió error (`ret <= 0`, "formato no soportado"
en pantalla -- fallo correctamente detectado, no corrupción de memoria,
la lección de M-033 sí se sostuvo). Causa real: el tamaño de
decodificación pedido (512×240) caía **entre** dos pasos de escala DCT
del propio decodificador JPEG (1/1 = 640×300, 1/2 = 320×150) -- para
llegar a un tamaño intermedio, el decodificador necesita el buffer de
resolución NATIVA completa como paso intermedio antes de su propio
re-escalado, no solo el tamaño final pedido. Fix: el chequeo de
memoria ahora presupuesta contra `max(decode_w*decode_h, src_w*src_h)`
en vez de solo `decode_w*decode_h`, y resta `JPEG_DECODE_OVERHEAD`
(`apps/recorder/jpeg_load.h`, ~38-39KB) del presupuesto en vez de
asumir que sobra espacio para el overhead del decodificador. Verificado
tras el fix: mismo fixture 640×300 en cubrir decodifica y se ve
correctamente recortado (`docs/screenshots/R2-F3-viewer-cover.png`).
Este hallazgo no se reportó de vuelta a Aura-Firmware (regla de
contención de la carpeta padre, y ese repo está fuera de alcance de
esta ronda) -- queda documentado aquí por si aplica también allá.

**`docs/DESVIACIONES.md` R2-1**: qué se pierde de `imageviewer.rock`
(zoom, paneo, slideshow, soporte PNG/GIF/BMP) -- misma pérdida que
Aura-Firmware ya aceptó para su propio visor (C.3).

**Verificado** (simulador): `docs/screenshots/R2-F3-viewer-fit.png`
(640×300, franjas arriba/abajo), `R2-F3-viewer-cover.png` (mismo
fixture, cubierto sin franjas), `R2-F3-viewer-small-cover.png`
(200×200, más chica que la pantalla, agrandada por muestreo sin
franjas), `R2-F3-viewer-next.png` (SCROLL_FWD a la siguiente foto del
mismo pivot), `R2-F3-menu-back-to-grid.png` (MENU vuelve a la
cuadrícula con la misma selección -- el fix real del reporte del
dueño), `R2-F3-music-keeps-playing.png` (hub sigue mostrando una pista
en reproducción después de pasar por el visor), `R2-F3-fade-mid.png`
(fundido de entrada a mitad de transición). Builds limpios (sim +
target ipod6g), 271 checks host-side, 0 fallos.

## M-059 — R2-F4/DD-11: `mpegplayer` (video) con estilo Metro -- port mecánico + restilado, sin menú de inicio

**Metodología**: se generó primero el `diff -u` de los 7 archivos
(Metro upstream, intacto, vs. Aura-Firmware) en un directorio de
scratch, como pide DD-11 -- nunca se copió un archivo entero de Aura.
El diff mismo separa con claridad "mecanismo" (se porta) de "diseño
Aura" (se reemplaza por el de Metro): `video_out_rockbox.c` resultó
ser 100% mecanismo puro, cero branding, portado casi verbatim; los dos
archivos con UI propia (`mpegplayer.c`, `mpeg_settings.c`) sí
necesitaron restilado real. Ver `MODIFICATIONS.md` R2-F4 para el
listado archivo por archivo.

**Lo que se portó tal cual (mecanismo)**: modo "cubrir" en
`video_out_rockbox.c` (`vo_draw_frame_cover()`, recorte+escalado
nearest-neighbor sobre la memoria sobrante del arena de libmpeg2,
`stretch_image_plane()` ya existente reutilizada con un segundo
llamador real) -- incluye el guard `scale_mode_locked` contra
codificadores MPEG-2 de GOP corto que repiten la cabecera de secuencia
en reproducción normal, un bug real que Aura-Firmware encontró y
documentó primero (D-308, consultado en solo lectura). `SETTINGS_VERSION`
5→6, `MPEG_TOGGLE_SCALE = BUTTON_SELECT`, y la eliminación completa del
menú de inicio interactivo (`get_start_time()`/`show_start_menu()`/
`draw_slider()`/`display_thumb_image()`/`show_loading()`/
`increment_time()`/`resume_options()`, ~400 líneas de `#define
MPEG_START_TIME_*` por target) -- `mpeg_start_menu()` ahora resuelve
directo a `MPEG_START_SEEK`, ignorando `settings.resume_options` a
propósito (correcto incluso si un `.rockbox/mpegplayer.cfg` viejo
todavía tiene `MPEG_RESUME_MENU_ALWAYS` guardado).

**Lo que se restiló para Metro, no copiado de Aura**:
- Widget de menú propio (`metro_menu_draw()`/`metro_menu_pick()`,
  reemplaza `rb->do_menu()`/`rb->set_option()`/`rb->set_int_ex()`)
  -- geometría de `metro_draw_rows()` (pitch 28px, x=12, encabezado
  caption y=4, seleccionado en fg, resto en secundario), **sin** la
  píldora redondeada de selección que Aura dibuja (su propio lenguaje
  Apple2026) -- rectángulos planos, como pide DD-11.
- Barra de progreso del OSD: dos colores planos (pista terciaria +
  relleno acento), sin borde -- reemplaza tanto el rectángulo con
  borde de Rockbox stock como la píldora de Aura.
- `metro_load_personalization()` (`mpegplayer.c`, en `osd_init()`, una
  sola vez -- no en cada redibujado del menú, ver
  `metro_osd_colors()`/`metro_language()` como accesores baratos hacia
  ese estado ya cargado) lee el esquema de **Metro** (`theme:0/1`,
  `accent:0..9`, `language:0/1`) de `/.rockbox/aura/aura.cfg`, no el de
  Aura (`theme`/`theme_id`/`accent_rgb24` + un `theme.cfg` por estilo
  custom) -- Metro no tiene temas instalables (M-012), así que solo
  elige entre los 10 acentos compilados de `metro_palette.h` y los
  tonos base oscuro/claro, exactamente lo mismo que `metro_theme.c`
  usa para el resto de la app. Tabla de 10 colores de acento duplicada
  localmente (`metro_accent_colors[]`, mismo orden que
  `metro_theme.c`'s `accent_colors[]`) porque un plugin no puede
  enlazar contra `apps/metro/metro_theme.c`, solo incluir el header
  puro del que ambos leen.
- Tabla bilingüe ES/EN propia (`metro_str()`/`enum metro_str_id`,
  `mpeg_settings.h`) para los ~35 textos que este plugin muestra
  (menús + 12 splashes de error) -- no incluye `metro_lang.c` (build
  separado), elige idioma vía el mismo `aura.cfg` ya leído.
- Ícono de estado (play/pausa/stop): se mantuvo el bitmap compilado de
  Rockbox stock (`osd.icons`), solo recoloreado al acento del usuario
  -- DD-11 pedía un ícono 100% geométrico (triángulo/dos barras, como
  F10) pero reemplazar el bitmap por dibujo vectorial es trabajo de
  diseño adicional fuera del alcance razonable de esta fase; el
  recoloreo (mecanismo que Aura ya portó) es un compromiso fiel de
  bajo riesgo. Ver `docs/DESVIACIONES.md` R2-3.

**Bug real encontrado y corregido, no en el port mecánico de Aura**:
al toggle a "cubrir" con un fixture 640×300, `read_jpeg_file()` no
aplica aquí (ese es el visor de fotos) -- el equivalente de video es
puramente aritmético (`stretch_image_plane()`, sin decodificador de
por medio), así que este archivo no repite el bug de memoria de
`metro_screen_photo_viewer.c` (M-058). No se encontraron bugs nuevos
en el port de video -- el mecanismo de Aura para "cubrir" en video
(a diferencia del de fotos) no depende de un decodificador JPEG con
sus propios límites de DCT, por eso no hereda ese problema.

**Verificado en vivo** (simulador): `docs/screenshots/R2-F4-video-play.png`
(video entra directo, sin menú de inicio -- confirmado con los 3
fixtures de video existentes, todos con entradas de reanudación en un
`mpegplayer.cfg` **versión 5** preexistente en el simdisk que sigue
cargando sin error en cada corrida, D.8 verificado de forma natural,
sin necesidad de fabricar el escenario), `R2-F4-video-settings.png`
(menú "Reproductor de video" → Ajustes/Salir, estilo Metro),
`R2-F4-display-options.png` (submenú completo: Mostrar FPS/Limitar
FPS/Omitir fotogramas/Modo de ajuste/Brillo de la luz de fondo),
`R2-F4-scale-mode.png` (Ajustar/Cubrir), `R2-F4-light-red-accent.png`
(mismo menú con `aura.cfg` puesto a tema claro + acento rojo a mano --
confirma que el plugin lee el esquema en vivo, no algo hardcodeado).
Colores verificados por inspección directa de valores en tiempo de
ejecución (no solo visual): `metro_accent_colors[]`/`osd.accent`
calculan exactamente el RGB565 esperado para cada acento (verificado
con `DEBUGF` temporal, revertido antes del commit).

**No verificado visualmente**: el contenido del panel OSD (barra de
progreso + tiempos) no se pudo distinguir con claridad en las capturas
headless -- el área donde debería estar aparece negra en todas las
capturas probadas (40, 80, 150 ticks), independiente del tema/acento
usado, lo que descarta que sea el propio panel OSD mal coloreado
(confirmado por los valores de color correctos vía `DEBUGF`) y apunta
a un artefacto de decodificación MPEG-2 en progreso (franjas aún no
decodificadas en negro) o a una limitación de temporización similar a
las ya documentadas en este proyecto (`docs/DESVIACIONES.md` F2-2/F2-3,
D-307 de Aura-Firmware para el propio frame YUV). El código del OSD en
sí (`draw_scrollbar_draw()`, `osd_refresh_status()`) es una adaptación
mínima y fiel del mecanismo ya probado de Aura, y los valores de color
que usa están verificados correctos por inspección directa -- el
riesgo real aquí es bajo, pero la confirmación visual queda pendiente
para cuando el dueño lo revise en el simulador interactivo o en
hardware real.

## M-060 — R2-F4 (continuación): rediseño real "Zune HD" del OSD de video

**Contexto**: tras M-059, el dueño verificó en el simulador interactivo
y confirmó que el menú nativo de Rockbox en efecto ya no aparece, pero
que el OSD en sí "dista mucho de parecerse al reproductor que tenía el
Zune" -- pidió explícitamente ayuda de diseño real, "como Microsoft lo
hubiera hecho con el Zune", no solo un port mecánico recoloreado. Se
propuso una maqueta HTML (artefacto, 5 cambios concretos con su
justificación) que el dueño aprobó sin cambios ("sí, me gusta esa
propuesta procede").

**Los 5 cambios aprobados y cómo se implementaron** (todo en
`apps/plugins/mpegplayer/mpegplayer.c`, sin tocar `video_out_rockbox.c`
ni `mpeg_settings.c` de M-059):

1. **Panel flotando sobre el video, sin fondo opaco** -- este punto
   resultó **no alcanzable tal cual se maquetó**, por una razón
   arquitectónica descubierta al leer `osd_show()` con cuidado antes de
   tocar código: Rockbox no compone el OSD sobre el video en vivo, lo
   **recorta**. `osd_show(OSD_SHOW)` llama
   `stream_vo_set_clip(&rc)` con `rc = {0, 0, SCREEN_WIDTH, osd.y}` --
   el hilo de video literalmente deja de dibujar en el área del OSD
   mientras está visible; al ocultarse, `stream_draw_frame(false)`
   redibuja esa zona desde cero. No es alpha-blending con "stale" video
   detrás -- es una zona muerta que el video nunca toca. Lograr un
   panel de verdad transparente exigiría leer el framebuffer de vuelta
   para componer manualmente, una reescritura mucho más grande y con
   riesgo de rendimiento real en hardware iPod 6G que no se puede medir
   esta ronda (Barrera B3). **Resolución aplicada**: no un panel
   "flotante" literal, sino uno mucho más chico y completamente plano
   -- `osd_refresh_background()` pasó de dibujar un bisel "elevado" de
   4 líneas de brillo/sombra (M-059 heredó esto de Rockbox stock) a un
   solo relleno plano sin borde, y la fila completa del OSD (ícono +
   tiempos + barra) se redujo de dos niveles a **una sola fila**, del
   tamaño del propio texto -- de un panel de ~28-34px de alto a uno de
   ~18-20px. El efecto visual apuntado ("ligero, no un cajón de chrome
   sobre el video") se logra por tamaño y planitud, no por
   transparencia real. Este ajuste de alcance no estaba en la maqueta
   aprobada -- se documenta aquí como corrección factual encontrada
   durante la ejecución (ver también `docs/DESVIACIONES.md` R2-4).
2. **Ícono de estado geométrico, sin sombra** -- `draw_status_icon()`
   (nueva, junto a `draw_tri_stepped()`) dibuja triángulos/barras con
   `draw_fillrect()` en pasos de 1px, igual que la barra de progreso ya
   hacía -- reemplaza `osd_refresh_status()`'s bitmap de 3 tamaños
   (`mpegplayer_status_icons_8/12/16x...`) con sombra de 1px offset en
   negro-blend. Los 5 estados (parado/pausado/reproduciendo/
   avance/retroceso) tienen su propio glifo; el color es siempre el
   acento del usuario.
3. **Tiempos en los extremos de la barra, no en una fila separada** --
   `osd_text_init()` se reescribió por completo: antes eran dos filas
   (ícono+tiempos arriba, barra abajo, con el ícono centrado sobre la
   barra); ahora es una sola fila, izquierda a derecha: ícono, tiempo
   transcurrido, la barra, duración total -- exactamente el layout de
   un transport bar real (Zune HD incluido). Se usó `vo_rect_set_ext()`
   (posición absoluta) en vez del truco original de "ancho como .r,
   luego offset" -- más fácil de verificar a mano sin arrastrar bugs de
   desplazamiento.
4. **Tipografía Metro real, no `FONT_SYSFIXED`** -- `metro-caption-14.fnt`
   (el mismo archivo que `apps/metro/metro_fonts.c` carga para el rol
   `MFONT_CAPTION`) se carga una vez en `osd_init()` vía
   `rb->font_load()` (un plugin no tiene `font_load_ex` con presupuesto
   de glifos, solo el `rb->` genérico) y se expone a través de
   `draw_setfont_osd()`, con reserva a `FONT_UI` si la carga falla. El
   riesgo de rectángulo de redibujado que tenía pendiente el plan
   anterior no aplicó: `osd_text_init()` mide el texto con la MISMA
   fuente que `osd_refresh()` usa para dibujar (ambos llaman
   `draw_setfont_osd()`), así que `osd.time_rect`/`osd.dur_rect` ya
   encierran exactamente lo que se dibuja -- no se necesitó ampliar el
   área de limpieza a mano.
5. **Barra delgada de 2px con punta redondeada, pista al 28% blanco** --
   `draw_scrollbar_draw()` ahora dibuja una línea de
   `METRO_OSD_BAR_LINE=2`px (antes ocupaba toda la altura del rect,
   ~12px) centrada dentro de un rect de `METRO_OSD_BAR_H=4`px, más un
   "thumb" cuadrado de esos mismos 4px en el borde de lo reproducido
   (recortado para no salirse nunca del rect que `osd.prog_rect` le
   asignó, ya que ese rect es lo que se limpia antes de cada redibujado
   y lo que se une al rectángulo sucio del blit final). La pista sin
   reproducir pasó de `osd.prog_trackcolor = s_metro_tertiary` (bloque
   sólido) a `draw_blendcolor(osd.bgcolor, MYLCD_WHITE, 71)` -- ~28% de
   blanco mezclado sobre el fondo del panel (`draw_blendcolor` interpola
   linealmente 0→255 entre sus dos colores, así que 71/255≈0.28).

**Limpieza asociada**: `osd.icons` y las tres externs
`mpegplayer_status_icons_8/12/16x8x1` (bitmaps que ya no se referencian
en código) se quitaron del struct/decls -- los `.bmp` fuente en
`apps/plugins/bitmaps/mono/` se dejaron intactos (recurso genérico de
Rockbox, tocar su pipeline de generación de bitmaps es una superficie
sin relación con este cambio). La variante de
`draw_oriented_mono_bitmap_part()` no-portrait quedó sin llamadores
tras quitar el ícono de bitmap -- se eliminó (la otra copia, la
`#ifdef LCD_PORTRAIT`, sigue viva vía `draw_putsxy_oriented()` para el
render de glifos). `draw_hline()` quedó igual de huérfana al aplanar
`osd_refresh_background()` -- eliminada también. Build limpio, cero
warnings, en sim (`build-sim`) y target (`build-ipod6g`).

**No verificado visualmente (igual que M-059, mismo límite, más
investigado esta vez)**: se intentó activamente encontrar una ventana
de ticks donde la captura headless mostrara el panel del OSD con
contenido legible. Con `firmware/tools/sim_shot.sh` variando ticks
finos (40/44/46/48/50/55/60/65/70/75/80/90) sobre la secuencia
`SCROLL_FWD,SELECT,SELECT`: en ticks ~40-50 aparece una franja negra
grande (bastante más alta que los ~18-20px que calcula
`osd_text_init()`) que por tamaño no puede ser solo el panel -- es
consistente con la conclusión que ya dejó `docs/DESVIACIONES.md` R2-3
para M-059: un artefacto de decodificación MPEG-2 en progreso (parte
del frame aún sin decodificar, no el panel). Para ticks ~55 en
adelante el video ya se ve a color completo y el panel ya no es
visible en absoluto -- la ventana en la que el panel está mostrado
*y* el video ya terminó de decodificar parece ser más angosta que la
granularidad de captura de un solo proceso permite aislar, igual que
"Committing database" en `docs/SUPERFICIES.md`. Se intentó además
`DEBUGF` temporal en `osd_text_init()` para confirmar los valores de
`osd.height`/rects en tiempo de ejecución sin depender de leer
píxeles -- resultó ser un callejón sin salida distinto: `apps/plugin.h`
compila `DEBUGF` como no-operación para plugins en este build (a
diferencia de `apps/metro/`, que sí imprime -- de ahí que
`metro_fonts: ... loaded as font id N` apareciera en el log pero nunca
una línea propia de `mpegplayer`); no se persiguió más allá porque
forzar una build de depuración del plugin es una superficie
nueva y no crítica para este cambio. La geometría se verificó en
cambio a mano (aritmética de `osd_text_init()` trazada término a
término contra `vo_rect_set_ext()`) y por la propia compilación limpia
en sim y target. Confirmación visual real queda, como en M-059, para
el simulador interactivo (lanzado al cierre de esta fase) o hardware.

## M-061 — R2-F4 (cierre): el rediseño Zune sí era invisible -- causa raíz `make` sin `make install`; menús reconstruidos con la anatomía real de página Metro

**El reporte del dueño que destapó todo**: tras M-060, el dueño verificó
en el simulador interactivo y vio el OSD *viejo* (dos niveles, volumen
en "-34dB", bisel) y los menús sin estilo Metro -- "¿en qué parte
supuestamente hiciste los ajustes?". Tenía razón: **ninguno de los
cambios de M-060 había llegado al simulador**.

**Causa raíz (metodológica, no de código)**: en este árbol, `make` a
secas compila y enlaza `mpegplayer.rock` en
`build-sim/apps/plugins/mpegplayer/` pero **no lo copia a
`simdisk/.rockbox/rocks/viewers/`** -- eso lo hace `make install`
(`tools/root.make`, `buildzip.pl` con `RBPREFIX=simdisk`). El
`.rock` del simdisk llevaba horas desactualizado (16:47 vs 18:09):
toda la verificación de M-060 -- capturas headless *y* el simulador
interactivo lanzado al cierre -- corrió contra el binario de M-059.

**Consecuencia que hay que corregir por escrito**: la sección "No
verificado visualmente" de M-059/M-060 y la conclusión de
`docs/DESVIACIONES.md` R2-3 ("el panel del OSD no se distingue en
capturas headless, artefacto de decodificación o límite de
temporización") quedan **desmentidas en su parte central**: con el
binario correcto instalado, el panel del OSD se captura perfectamente
(`docs/screenshots/R2-F4-zune-osd.png`, pausa a 40 ticks). La franja
negra gigante de ticks 44-48 sí era decodificación en progreso (eso
se sostiene), pero "el panel no aparece nunca" era simplemente el
panel viejo de un binario viejo. Ver `docs/DESVIACIONES.md` R2-5.

**Lo que M-060 dejó bien y esta pasada confirmó visualmente** (ya
instalado): fila única de transporte (ícono geométrico en acento +
tiempos en los extremos + línea de 2px al ~28% blanco), sin bisel,
panel de ~20px. `docs/screenshots/R2-F4-zune-osd.png`.

**Lo que faltaba de verdad y esta pasada agregó** (la crítica del dueño
sobre los menús seguía vigente incluso con el binario correcto: M-059
copió la *geometría* de filas pero no la *anatomía de página* de Metro):

1. **Anatomía completa de página** (`metro_page_chrome()`,
   `mpeg_settings.c`): réplica 1:1 de lo que toda pantalla real de la
   app dibuja -- ceja en caption minúsculas a (12,4)
   ("reproductor de video"), reloj `%02d:%02d` a la derecha, glifo de
   batería 18x9+nub (copiado de `metro_draw_battery()`, F10), y el
   elemento firma que faltaba por completo: **título de página gigante
   en minúsculas con la fuente display de 48px a (12,28)** -- la misma
   posición y fuente con que el hub dibuja sus pivotes
   (`metro_draw_pivots()`). Filas desde y=84 (antes 32), pitch 28 --
   los offsets exactos de `metro_draw.c`.
2. **Fuentes reales en los menús**: la pasada anterior de esta misma
   sesión ya había cambiado FONT_UI por `metro-list-20`/`metro-listsel-20`/
   `metro-caption-14`; ahora se suman `metro-display-48` (título) y
   `metro-title-28` (valor de la pantalla de brillo). Las cinco cargas
   via `rb->font_load()` **deduplican por ruta** contra las que
   `apps/metro/metro_fonts.c` ya dejó residentes al arrancar
   (`firmware/font.c`, `find_font_index()` → "already loaded") -- costo
   de RAM cero, nada que descargar al salir del plugin.
3. **Interacción real de Metro -- ciclar en el lugar**: los selectores
   de dos filas por valor (patrón Rockbox) se eliminaron; seleccionar
   una fila de valor **cicla el valor en su lugar**, con el valor
   actual siempre visible como subtítulo caption/terciario alineado a
   la derecha -- exactamente `general_on_select()` de
   `metro_screen_settings.c` (idioma/animaciones/gráficos ciclan así).
   La fila "Borrar todas las reanudaciones" muestra el conteo actual
   como valor (baja a 0 en el acto, confirmación visual gratis).
4. **Títulos de página**: strings nuevos en minúsculas y cortos
   (`video`, `ajustes`, `pantalla`, `audio`, `brillo` -- ES y EN) para
   que quepan en la fuente de 48px; la ceja reutiliza
   MSTR_VIDEO_PLAYER, ahora en minúsculas como toda ceja de la app.
5. **Volumen sin decibeles** (pedido explícito del dueño): el "-34dB"
   se reemplazó por una barra de nivel de 28px con el mismo lenguaje
   de la barra de avance (pista al 28% blanco + relleno acento),
   normalizando `(volume-min)/(max-min)` del rango real de
   SOUND_VOLUME. `osd_refresh_volume()` reescrita; ya no se mide texto.
6. **Pantalla de brillo**: misma anatomía de página (ceja + "brillo"
   en display-48) con el valor en title-28 y color acento.

**Verificado en vivo** (simulador, binario instalado con `make
install`): `R2-F4-zune-menu-video.png` (menú raíz), 
`R2-F4-zune-menu-ajustes.png` (valores "Uno"/"3" en línea),
`R2-F4-zune-menu-pantalla.png` (5 filas con valores en vivo),
`R2-F4-zune-menu-audio.png`, `R2-F4-zune-brillo.png` (valor en acento),
`R2-F4-zune-osd.png` (OSD pausado: ícono acento + 0:00/0:01 en los
extremos + línea 2px + barra de volumen). Build limpio sin warnings en
sim y target; 271 checks de los 4 suites en verde.

**Regla operativa nueva para el repo** (guardada también en memoria de
sesión): toda verificación en simulador -- headless o interactiva --
va precedida de `make install`, no solo `make`. `firmware/tools/sim_shot.sh`
no lo hace por sí mismo.

## M-062 — R3-F1: motor de miniaturas genérico (`metro_thumbs.c`), base para fotos de artista y quickplay

**Contexto**: primera fase de la ronda 3 (`PLAN-metro-r3-maestro.md` DD-1).
Fotos de artista (R3-F3) y quickplay (R3-F4) necesitan, cada uno por su
lado, una caché de N miniaturas pequeñas con ventana en RAM +
persistencia a disco -- el mismo problema que `metro_photo_thumbs.c`
(R2-F2/M-057) ya resolvió para `/Photos/`. En vez de triplicarlo, esta
fase lo generaliza primero, sin ningún cambio de cara al usuario.

**Qué cambió**: `metro_photo_thumbs.c/.h` → `metro_thumbs.c/.h`. La API
pasó de estar keyed por `(filename, mtime)` -- shape específico de
fotos -- a `metro_thumbs_get(source, ctx, index)`/`metro_thumbs_tick()`/
`metro_thumbs_reset()`, con un `struct metro_thumb_source` de dos
callbacks (`cache_key(ctx, index, ...)`, `decode(ctx, index, dst)`) que
cada fuente implementa. Se eligió indexar por `(source, ctx, index)` en
la cola pendiente -- no solo por una clave de texto, como tenía R2-F2
-- porque `metro_thumbs_tick()` necesita poder volver a llamar al
`decode()` correcto de la fuente correcta más tarde; una clave de texto
sola no alcanza para eso.

**Ventana de RAM única y compartida**: los 16 slots (~205 KB) siguen
siendo los mismos de R2-F2, pero ahora una sola instancia sirve a
cualquier fuente -- nunca hay más de una cuadrícula en pantalla a la
vez, así que no hay razón para pagar tres ventanas. Cambiar de fuente
(dejar Fotos por Artistas, por ejemplo) es un `metro_thumbs_reset()`,
igual que antes al cambiar de pivot dentro de Fotos.

**Caché en disco por subdirectorio**: `metro_settings_metro_cache_dir()`
(único punto autorizado a armar esta ruta, regla del `CLAUDE.md`) ganó
un parámetro `subdir` -- de `.../aura/metrocache/photos` fijo a
`.../aura/metrocache/<subdir>`, uno por fuente. El esquema de clave
`<nombre>.<mtime>.mth` se conserva tal cual: `remove_stale()` (que
antes recibía el nombre de archivo como prefijo explícito) ahora deriva
el prefijo genéricamente cortando la clave en el **último** punto --
funciona igual para las tres fuentes porque las tres siguen esa misma
convención `<estable>.<variable>` en su propio `cache_key()`.

**Decodificación JPEG-a-tile compartida, no parte del motor**: el
motor en sí (`metro_thumbs.c`) no sabe qué es un JPEG -- `cover_crop()`
y el `read_jpeg_file()` de R2-F2 se expusieron como
`metro_thumbs_decode_jpeg_cover(path, out)`, un helper público que
cualquier fuente cuyo `decode()` sea "leer un JPEG y recortarlo a
cuadrado" puede llamar (Fotos y, en R3-F3, Artistas -- Quickplay en
R3-F4 no, porque primero necesita resolver qué track de un álbum tiene
la carátula antes de decodificar nada). El scratch buffer de decode
sigue siendo uno solo, estático, compartido -- válido porque el
presupuesto de una decodificación por tick sigue vigente sin cambios.

**Fotos migradas en la misma fase**: `metro_screen_hub.c` gana un
`photo_thumb_source` chico (cache_key = `"<filename>.<mtime>"`, decode
= construir `/Photos/<filename>` y llamar al helper compartido) y
`photo_pivot_get_tile()` pasa a delegar en `metro_thumbs_get()` con esa
fuente -- es la única consumidora del motor en esta fase, y sirve de
prueba de regresión: el comportamiento tiene que ser bit a bit el
mismo que R2-F2.

**Verificado**: sim y target compilan limpio (los warnings de
`tile_cols` que aparecen en el build de target son preexistentes de
R2-F2 -- confirmado comparando contra el árbol sin estos cambios via
`git stash`, no algo que esta fase introdujo). 4 suites de test de host
en verde (271 checks, sin relación directa pero corridos como parte
del criterio de "nunca dejar el build roto"). Verificación visual en
el simulador (`make install` primero, regla de M-061): con el
`metrocache/photos/` previo borrado a mano para forzar el camino
"nada en caché", la cuadrícula muestra los 8 tiles de acento con
inicial de "todos" (idéntico a `R2-F2-photos-grid-loading.png`); tras
dejar correr el ciclo ocioso, los 8 tiles reales aparecen (idéntico a
`R2-F2-photos-grid.png`); `SCROLL_FWD`×5 mueve el borde de selección a
la fila 2 (idéntico a `R2-F2-photos-grid-scrolled.png`); y
`ls metrocache/photos/` vuelve a mostrar 8 `.mth` con el mismo esquema
`<archivo>.<mtime>.mth`. Al ser una regresión exacta (nada cambia de
cara al usuario), no se generaron capturas `R3-F1-*.png` nuevas -- las
tres de R2-F2 siguen siendo la documentación visual vigente de esta
superficie.

## M-063 — R3-F2: letras `.lrc` sincronizadas, modo de pantalla completa en Now Playing

**Contexto**: segunda fase de la ronda 3 (`PLAN-metro-r3-maestro.md`
DD-2/DD-3). Primer ítem grande del backlog de v1. A diferencia de casi
todo el resto de Metro, no es un port de mecanismo de Aura-Firmware:
solo la IDEA (letras sincronizadas) viene de ahí, el parser y el
render son diseño propio -- ver `docs/DESVIACIONES.md` R3-1 para el
porqué del modelo de memoria distinto.

**Parser (`metro_lrc.c/.h`, nuevo, C99 puro, host-testeable)**: acepta
`[mm:ss]`/`[mm:ss.f]`/`[mm:ss.ff]`/`[mm:ss.fff]` (normalizado a
milisegundos: 1 dígito ×100, 2×10, 3+ trunca a milisegundo), múltiples
timestamps por línea (comparten el mismo `offset` de texto, sin
duplicar), descarta silenciosamente tags de metadata (`[ar:...]`) y
líneas sin marca válida (mismo criterio que el parser de Aura,
`INVESTIGACION-metro-r3.md` A.1). `metro_lrc_parse(lrc, len)` opera
**en el propio `lrc->buf`** que el llamador ya llenó (lectura de
archivo real, o un fixture de test escrito directo) -- corta cada
línea en su lugar con un `\0`, sin buffer intermedio. `metro_lrc_find_active()`
es búsqueda binaria sobre las entradas (asume orden cronológico, no
reordena -- mismo supuesto que cualquier reproductor de `.lrc` real).
`metro_lrc_sibling_path()` deriva la ruta hermana (mismo directorio,
mismo nombre base, `.lrc`) con la misma convención que
`derive_sibling_path()` de Aura (consultado read-only) y que
`library-layout-v1.md` §3 ya documenta. 45 checks en
`apps/metro/test/test_lrc.c`, cero fallas a la primera corrida.

**Integración (`metro_screen_nowplaying.c`)**: `ensure_lyrics_loaded(id3)`
es una "caché de 1" keyed por `id3->path`, mismo patrón exacto que
`load_art()` de `metro_albumart.c` -- recarga solo cuando la pista
cambió, nunca por redibujo. La fila "letra" de Options (nueva, entre
Repeat y la cola) muestra "no disponible" si la pista actual no tiene
`.lrc` (o está vacío/sin líneas válidas) y en ese caso `SELECT` sobre
ella es un no-op -- la única forma de dejar el modo "no alcanzable sin
letra" sin agregar un tipo de fila deshabilitada nueva al widget
compartido de listas (que habría tocado `metro_screen_list.c`/`metro_draw.c`,
usado por toda la app, fuera del alcance de esta fase).

**Diseño visual (DD-2)**: modo de **pantalla completa**, no un panel
en el hueco de ~160×56px que `INVESTIGACION-metro-r3.md` A.6 encontró
libre -- reemplaza TODO el contenido normal de Now Playing (carátula,
título/artista/álbum, tiempos, barra, íconos de modo), deja el header
y el overlay de volumen (retroalimentación transitoria de una acción,
no parte del layout que este modo reemplaza). Línea activa en
`MFONT_TITLE` (28px) en `metro_color_fg()`, hasta 2 líneas de contexto
cada lado en `MFONT_LIST` (20px), atenuadas por distancia
(secundario/terciario). Alineado a la izquierda (x=12) -- Metro no
centra texto. **No se porta el "vidrio" translúcido de Aura**
(`INVESTIGACION-metro-r3.md` A.4): es gradiente + translucidez,
lenguaje Apple2026 ajeno al diseño plano de Metro; el atenuado al 30%
que Now Playing ya aplica sobre la carátula (F12, `METRO_NP_BG_ALPHA256`)
es justo lo que hace innecesario ese mecanismo -- Aura compone sobre
carátula a color pleno, Metro no.

**Desviación real encontrada** (no solo el modelo de memoria, que ya
estaba en el plan): DD-2/DA-3 pedían que el modo también se conmutara
con `SELECT` directo en Now Playing -- pero `player_mapping[]` ya tiene
`SELECT|REL`→Options y `SELECT|REPEAT`→shuffle, sin gesto libre. Se
resolvió a favor de la alternativa que la propia DA-3 ya ofrecía: **solo
desde la fila de Options**. Ver `docs/DESVIACIONES.md` R3-2.

**Verificado en vivo** (simulador, `make install` primero): con
`metro-test.lrc` (3 líneas reales + 3 tags de metadata) reproduciendo
la pista de prueba, Options muestra "letra: desactivado" → tras
activarla y volver, Now Playing dibuja el modo de pantalla completa
con la línea activa en grande y el contexto atenuado por distancia,
confirmando que el parser descartó correctamente los tres tags de
metadata sin que aparecieran como líneas. Con una pista sin `.lrc`
("Wheel & Click/Analog Dreams"), Options muestra "letra: no disponible"
y `SELECT` sobre esa fila no cambia nada (verificado repitiendo la
captura). Build limpio en sim y target (warnings de `tile_cols`
presentes son preexistentes, no de esta fase). 5 suites de test de
host en verde (316 checks).
