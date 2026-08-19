# DESVIACIONES.md — Registro de desviaciones respecto a `PLAN_MAESTRO.md`

Cada entrada: fase · qué decía el plan · qué se encontró en la práctica · qué se hizo. Las decisiones de diseño (`M-NNN`) no se cambian por preferencia — solo se corrigen hechos falsos o se documentan ajustes técnicos necesarios para que el plan funcione.

---

## F0-1 — `tools/configure` no estaba en la lista de 27 archivos modificados por Aura

**Plan decía** (`PLAN_MAESTRO.md` §0.2, citando `INVESTIGACION.md` D.4): que `tools/configure` probablemente era idéntico al upstream porque no aparecía en la lista de 27 archivos de `MODIFICATIONS.md` de Aura-Firmware — quedó marcado `NO RESUELTO` en la investigación.

**Qué se encontró**: al hacer `diff -rq` completo del árbol de Aura-Firmware contra una copia limpia del upstream `0726ec93` (verificación previa a sembrar F0), aparecieron **28** archivos distintos, no 27. Los dos no documentados en `MODIFICATIONS.md` de Aura-Firmware:
- `tools/configure`: sí tiene una modificación real (D-007) — detección del gcc de Homebrew más reciente disponible con fallback 16→15→14→13, en vez de fijar `gcc-16` a secas. Es una laguna real de la documentación de Aura-Firmware (no se toca ese repo desde aquí).
- `apps/plugins/mpegplayer/mpegplayer.h`: agrega la declaración de `aura_osd_colors()` (D-306/D-307) — pertenece al mecanismo de OSD de mpegplayer, ya clasificado en el backlog de `PLAN_MAESTRO.md` (ítem 1), no afecta F0.

**Qué se hizo**: se agregó `tools/configure` a la lista de archivos portados en F0 (ahora 10, no 9) — es exactamente el tipo de fix de build que la Fase Cero busca. `mepgplayer.h` no se porta en F0 (el backlog que lo necesita no es v1); se revisará su diff cuando/si se ejecuta ese ítem del backlog.

**Impacto en `PLAN_MAESTRO.md`**: §0.2 pasa de 9 a 10 archivos portados.

---

## F0-2 — Comentarios de atribución GPL en los archivos portados no se reescriben

**Plan decía** (§0.2, columna "Marca inline"): sustituir `Aura (D-NNN)` por `Metro (from Aura D-NNN)` dentro de cada archivo portado.

**Qué se decidió**: los 10 archivos se portan **byte-idénticos** a Aura-Firmware, sin tocar sus comentarios. Los comentarios narrativos que dicen "Aura (D-293): ..." son, en varios de estos archivos, el propio aviso de modificación GPL v2 §2a (qué se cambió, cuándo, por quién) — reescribirlos para decir "Metro" atribuiría a Metro-Aura una modificación que en realidad hizo Aura-Firmware. La atribución correcta (que este archivo llegó modificado desde Aura-Firmware, con referencia cruzada a sus decisiones D-NNN) queda en `MODIFICATIONS.md` de este repo, que es el mecanismo estándar para documentar el historial de modificaciones sin alterar el código fuente heredado.

**Impacto en `PLAN_MAESTRO.md`**: ninguno en el resultado final (mismo comportamiento); la columna "Marca inline" de §0.2 queda reemplazada por "sin cambios en el archivo; atribución en MODIFICATIONS.md".

---

## F1-1 — La higiene de `global_settings` (M-019) no puede correr al principio de `metro_main()`

**Plan decía** (`PLAN_MAESTRO.md` §1.2, fila `metro_main.c/.h`): "higiene M-019" como lo primero que hace `metro_main()`, la función que se llama una sola vez desde `apps/main.c` en reemplazo de `root_menu()`, **después** de que `init()` termina.

**Qué se encontró**: con la higiene puesta ahí, la primera captura de pantalla (`F1-boot.png`) mostró el backdrop stock de Rockbox (gradiente gris con el wordmark "ROCKbox" rotado, `backdrops/cabbiev2.320x240x16.bmp`) detrás del texto "metro", en vez de un fondo negro limpio. Investigando la ruta de código (`firmware/drivers/lcd-16bit-common.c:lcd_clear_viewport()`): cuando hay un backdrop activo (`lcd_backdrop` no nulo, seteado por `apps/gui/skin_engine/skin_backdrops.c:skin_backdrop_show()`), **cualquier** `lcd_clear_display()` — incluida la de mi propio `metro_screen_splash_show()` — pinta el backdrop en vez de un color sólido; no hay forma de "limpiar" ese comportamiento después del hecho sin desactivar el backdrop en el origen. Y el backdrop se activa en `settings_apply_skins()` (`apps/gui/skin_engine/skin_engine.c:165`), que lee `global_settings.backdrop_file` **dentro de `init()`**, antes de que `metro_main()` — y por lo tanto mi higiene — llegue a correr.

**Qué se hizo**: se movió la higiene completa a una función exportada, `metro_apply_hygiene()` (declarada en `metro_main.h`), llamada desde **ambos** cuerpos de `init()` en `apps/main.c` (uno para el build de simulador/hosted, otro para target real — el archivo tiene dos definiciones de `init()` bajo `#if (CONFIG_PLATFORM & PLATFORM_HOSTED)` / `#else`), en el mismo punto exacto que usa Aura-Firmware: justo después de `settings_load()`, antes de `settings_apply(true)`/`settings_apply_skins()`. Confirmado por captura: con este cambio, `F1-boot.png` muestra fondo negro limpio.

**Impacto en `PLAN_MAESTRO.md`**: §1.2 (tabla de `metro_main.c/.h`) y el pseudo-código de M-019 deben leerse como "higiene llamada desde `init()` en `apps/main.c`, no desde `metro_main()`" — `metro_main()` sigue siendo el punto de entrada de la UI, pero ya no es responsable de este paso. Cualquier fase futura que agregue una nueva pantalla de "primer arranque" debe recordar que la higiene ya corrió antes de que exista ninguna UI.

---

## F2-1 — `metro_draw_rows()`/`metro_draw_pivots()`/`metro_draw_tile()`/`metro_draw_progress()` no se implementan en F2

**Plan decía** (`PLAN_MAESTRO.md` §1.2, fila `metro_draw.c/.h`, fase "F2, F3"): que F2 ya deja escritas las primitivas de dibujo de filas y encabezado de pivots (`draw_rows`, `draw_pivots`), junto con `metro_draw_tile()`/`metro_draw_progress()`.

**Qué se encontró**: esas cuatro primitivas operan sobre tipos que el propio plan define recién en F3 (`struct metro_page`/`struct metro_pivot`/`struct metro_row`, tabla de `metro_page.h` en §1.2, fase F3). Escribirlas en F2 exigía inventar firmas de datos provisionales que F3 probablemente tendría que rehacer, contra el criterio de "no diseñar para hipotéticos" del propio proyecto.

**Qué se hizo**: F2 implementa solo lo que el propio criterio de "hecho" de F2 pide y puede probar de forma aislada: `metro_draw_clear()`, `metro_draw_text()`, `metro_draw_text_cut_right()`, `metro_draw_header()`, `metro_draw_battery()`. Las otras cuatro quedan explícitamente para F3, cuando `metro_page.h` ya exista.

**Impacto en `PLAN_MAESTRO.md`**: §1.2, fila `metro_draw.c/.h` — la fase de `draw_rows`/`draw_pivots`/`draw_tile`/`draw_progress` pasa de "F2, F3" a "F3" únicamente.

---

## F2-2 — Capturas headless del simulador no son fiables una vez `metro_main()` entra a su loop de botones (macOS ≥26.4)

**Plan decía** (`PLAN_MAESTRO.md` §5, criterio de "hecho" de F2): capturar `F2-type-specimen.png` con `sim_shot.sh` de la forma estándar (como ya había funcionado en F0 y F1).

**Qué se encontró**: con el código de F2 (que agrega carga de 5 fuentes + dibujo del espécimen antes del loop), `sim_shot.sh docs/screenshots/F2-type-specimen.png 100` produce un `dump.bmp` truncado (66 bytes, solo cabecera) y el proceso termina con una excepción no capturada de AppKit:

```
*** Terminating app due to uncaught exception 'NSInternalInconsistencyException',
reason: 'nextEventMatchingMask should only be called from the Main Thread!'
...
7   rockboxui   button_get_w_tmo + 56
8   rockboxui   metro_main + 44
```

Investigado a fondo: **no es un bug de Metro**. `tools/configure` (heredado de Aura-Firmware, ver `MODIFICATIONS.md`) ya trae un comentario `FIXME` explícito sobre esto:

> `# FIXME: sigaltstack fails with "Operation not permitted" in make_context (firmware/asm/thread-unix.c) on latest versions of macOS (≥ 26.4). Fall back to SDL threads.`

Este Mac corre macOS 26.5.2 -- dentro del rango que ese `FIXME` ya marca como problemático. Con `HAVE_SDL_THREADS` (confirmado activo en `autoconf.h` de este build), el "hilo del dispositivo" donde corre `metro_main()` es un hilo creado por SDL, no el hilo principal real del proceso -- y en macOS ≥26.4, `SDL_PumpEventsInternal`/AppKit ya no tolera que ese hilo secundario bombee eventos (`nextEventMatchingMask`) mientras el mecanismo de captura headless (`uisimulator/common/sim_tasks.c`, en un hilo aparte) intenta volcar la pantalla a disco concurrentemente. El barrido de valores de `METRO_SIM_AUTODUMP_TICKS` confirma que la corrupción es prácticamente determinista para cualquier tick ≥30 (15/15 intentos fallidos a `ticks=100`) -- y como `show_logo_boot()` (stock de Rockbox) ya bloquea los primeros ~100 ticks con su propio `sleep(HZ)`, no existe ningún valor de tick donde la captura sea a la vez confiable y posterior al arranque de `metro_main()`. Se probaron sin éxito: forzar `SDL_VIDEODRIVER=dummy` (el proceso se cuelga en vez de crashear), terminar el proceso externamente tras estabilizar el archivo (la corrupción ya ocurrió antes de que el poll externo detecte el archivo), adjuntar `lldb` para invocar `screen_dump()` manualmente con todos los hilos pausados (requiere una autorización interactiva de macOS no disponible en esta sesión en segundo plano), y el hint `SDL_MAC_BACKGROUND_APP=1` de SDL3 (documentado como "don't force the SDL app to become a foreground process" -- exactamente el escenario, pero no cambió el resultado).

**Qué se hizo**: no se alteró `metro_main()` ni ningún código de producto para "arreglar" esto -- degradar la responsividad real de la UI (p.ej. agregando un `sleep()` fijo antes del loop de botones) solo para que un capturador de pantalla de desarrollo funcione sería resolver el problema equivocado. En su lugar:
- Se verificó la carga de las 5 fuentes por otra vía, sólida y determinista: la salida de `DEBUGF` (activo por defecto en builds de simulador) corriendo `rockboxui` de forma interactiva sin autodump, que muestra las 5 líneas `metro_fonts: <rol> (<ruta>) loaded as font id N` sin ningún fallback a `FONT_SYSFIXED` -- ver evidencia completa en el reporte de cierre de F2.
- `F2-type-specimen.png` **no se pudo capturar en esta sesión**. Queda pendiente de que el dueño la tome en su propia máquina corriendo `firmware/tools/sim_shot.sh` interactivamente (fuera de una sesión headless en segundo plano, que es probablemente donde esta restricción de AppKit se manifiesta con más fuerza) -- o, si el problema persiste ahí también, de investigar un fix real en `uisimulator/common/sim_tasks.c`/`firmware/target/hosted/sdl/` en una fase dedicada.

**Impacto en `PLAN_MAESTRO.md`**: el criterio de "hecho" de F2 se cumple parcialmente -- carga de fuentes verificada por log, compilación limpia para simulador y target real verificada, pero sin la captura visual. **Riesgo real para fases futuras**: F3 en adelante depende de `sim_shot.sh` con secuencias de botones inyectadas (`METRO_SIM_BUTTONS`), que necesariamente mantienen `metro_main()` corriendo su loop por muchos ticks -- exactamente el escenario que dispara esta falla. Si el problema persiste en sesiones futuras (headless o no), es un bloqueante real para el criterio "simulador primero" del proyecto y merece una fase de investigación dedicada antes de F3, no un parche apurado.

**Confirmado por el dueño en sesión interactiva** (no en segundo plano): el mismo crash ocurre corriendo `sim_shot.sh` directamente en Terminal.app en este Mac. Descarta la hipótesis de "solo pasa en sesiones headless" -- es un problema real del entorno (macOS 26.5.2 + este SDL3 + Rockbox), no de esta sesión en particular.

---

## F2-3 — Intento de arreglo real (mover `screen_dump()` al hilo de `metro_main()`): revertido, cambia el crash por un deadlock peor

Tras la confirmación de F2-2 en sesión interactiva, se intentó una solución real en vez de solo documentar el bloqueo, ya que F3 depende de esto.

**Diagnóstico más profundo**: `screen_dump()` (`firmware/screendump.c`) en sí no tiene ninguna dependencia de SDL -- es E/S de archivos POSIX pura (`creat`/`write`/`close`) sobre `lcd_framebuffer`. El crash de F2-2 ocurre porque se **invoca desde `sim_thread`** (un hilo Rockbox real bajo `HAVE_SDL_THREADS`, creado en `uisimulator/common/sim_tasks.c`), mientras `metro_main()` corre en un hilo *distinto* y está bloqueado dentro de `button_get_w_tmo()` -> `SDL_PumpEventsInternal`. Confirmado con un experimento de control: `timeout 20 ./rockboxui` sin ninguna variable de autodump corre 20 segundos completos sin crashear -- `button_get_w_tmo()` por sí solo es inofensivo; el crash exige la actividad concurrente de `sim_thread`.

**El intento**: se separó el disparo (`sim_thread` sigue detectando *cuándo* volcar, vía tick o tecla F5/KP_0) de la ejecución real (`screen_dump()`+`exit()` movidos a una función nueva, `sim_tasks_service_dump()`, declarada en `sim_tasks.h` y llamada desde el propio loop de `metro_main()` en cada iteración -- el único hilo que además ya pasa por `button_get_w_tmo()` de forma segura). Cambios: `uisimulator/common/sim_tasks.c`/`.h`, `apps/metro/metro_main.c` (agregado `sim_tasks_service_dump()` al loop, y `button_get_w_tmo(HZ)` -> `button_get_w_tmo(HZ/10)` para no encolar la captura hasta un segundo completo).

**Resultado**: el crash de AppKit desapareció (0/varios intentos), pero apareció un **deadlock nuevo y peor**: el archivo de volcado queda truncado en exactamente 66 bytes (la cabecera BMP) en el 100% de los intentos -- incluso en los casos que antes SÍ funcionaban (`ticks=20`/`25`, capturando el logo de arranque). Instrumentación temporal con `DEBUGF` en `screendump.c` (revertida) localizó el punto exacto: la cabecera se escribe bien, se entra al bucle de líneas de barrido, se ejecuta la primera iteración (`memset` + copia de píxeles desde `FBADDR`) -- y el proceso se queda colgado ahí mismo, nunca llega ni al primer `write()` de una línea de barrido ni a ningún mensaje de error. `ps`/`lsof` durante el cuelgue confirman: el fd real (`6`, distinto del "fd" virtual `1` que reporta la capa hosted de Rockbox) queda abierto con exactamente 66 bytes escritos, y el proceso está en estado `S` (esperando, no en una syscall bloqueada tipo `U`) -- consistente con un deadlock real en algún primitivo de sincronización (probablemente en la capa de filesystem simulado, `firmware/target/hosted/filesystem-unix.c`, no tocada por Metro ni por Aura), no con un cuelgue de la copia de píxeles en sí (que es aritmética de punteros pura, sin ninguna razón para bloquearse).

**Qué se hizo**: se revirtió el intento por completo (`git checkout --` sobre los 4 archivos tocados) -- un deadlock del 100% de las veces es estrictamente peor que un crash que al menos funcionaba parcialmente (`ticks≤25`, antes de que `metro_main()` entrara a su loop). El árbol quedó exactamente como en el commit de F2.

**Impacto en `PLAN_MAESTRO.md`**: ninguno adicional a F2-2 -- el bloqueo para F3 sigue en pie. **Pista real para quien retome esto**: el problema no es únicamente "qué hilo bombea eventos SDL" (F2-2) -- hay una **segunda** condición de carrera independiente en la capa de filesystem simulado de Rockbox, que se dispara específicamente cuando `screen_dump()` se llama desde el hilo "device" (`metro_main`) en vez de `sim_thread`. Cualquier solución real necesita investigar `firmware/target/hosted/filesystem-unix.c` y qué primitivo de sincronización usa alrededor de `open`/`write`/`close`, no solo el problema de AppKit. Candidato con mejor relación esfuerzo/resultado si se retoma: correr `sim_shot.sh` sin `METRO_SIM_AUTODUMP_QUIT` y con `lldb`/Instruments interactivo (no headless) para ver el backtrace real de ambos hilos en el momento del cuelgue -- esta sesión no pudo usar `lldb` por falta de autorización interactiva de macOS (ver F2-2).
