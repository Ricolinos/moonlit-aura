# PLAN MAESTRO — Metro-Aura

**Fase 3 del proyecto Metro-Aura.** Este documento convierte `INVESTIGACION.md` en un plan de ejecución: qué se construye, en qué archivos, en qué orden y con qué criterio de "hecho". La Fase 4 lo ejecuta fase por fase sin re-explorar el código; si un dato del plan resulta falso (archivo inexistente, API distinta), la Fase 4 corrige y anota la desviación en `docs/DESVIACIONES.md` — nunca cambia una decisión de diseño por preferencia propia.

Fecha: 2026-08-20. Insumo: `docs/PLAN_INVESTIGACION.md`, `docs/INVESTIGACION.md` (con la errata A.7 añadida en esta fase). Español en documentos; inglés en código, comentarios y commits.

---

## Índice

- **§D — Decisiones** (resuelve la sección G de la investigación, M-001…M-020)
- **§0 — Fase Cero de separación** (qué se porta de Aura-Firmware, archivo por archivo)
- **§1 — Capa de diseño** (arquitectura, módulos, archivos nuevos/modificados)
- **§2 — Navegación** (esqueleto twist, tabla gesto Zune → clickwheel)
- **§3 — Transiciones y animaciones** (sistema, primitivas, niveles, fallback)
- **§4 — Compatibilidad con Aura Studio** (checklist verificable)
- **§5 — Orden de ejecución** (fases F0…F13 + backlog, criterio de hecho por fase)
- **§6 — Reglas transversales para la Fase 4**

---

## §D — Decisiones

Cada decisión tiene identificador `M-NNN` (se copian a `Metro-Aura/DECISIONS.md` en F0 y se extienden desde ahí; la fuente de verdad de decisiones es ese archivo, no este plan). Marca `[ESTIMADO]` donde la justificación no se pudo verificar en código.

| ID | Decisión | Justificación (evidencia) |
|---|---|---|
| **M-001** | **Commit base**: Metro-Aura se siembra desde upstream Rockbox `0726ec93517a61f602679ab052b083217ec9c96d`, el mismo de Aura-Firmware. Layout del repo idéntico al de Aura-Firmware: `firmware/rockbox/` (árbol Rockbox versionado in-repo, sin `.git` interno), `firmware/tools/`, `firmware/toolchain/` (ignorado), `firmware/build-sim/`, `firmware/build-ipod6g/`, `firmware/build-ipod6g-boot/` (ignorados), `firmware/assets/`, `firmware/test-media/` (ignorado), `docs/`. | D.7, D.4: toolchain, los 27 parches auditados y el simulador ya están probados contra ese commit exacto; misma estructura → los scripts de Aura se portan con renombres mínimos. |
| **M-002** | **Toolchain propio**: `firmware/toolchain/` se compila con `rockboxdev.sh` dentro de Metro-Aura (mismo procedimiento que D-032 de Aura); `build_*.sh` acepta `RBDEV_TOOLCHAIN=<ruta>` para apuntar a otro (p. ej. el de Aura-Firmware) como atajo de desarrollo, nunca por defecto. | D.4; regla de contención de la carpeta padre (ningún script asume un checkout hermano). |
| **M-003** | **Rutas en disco**: Metro conserva **`/.rockbox/aura/`** como prefijo, el archivo **`aura.cfg`** con su formato `clave: valor`, y **`/.aura/sync-pending.json`**. El nombre "Aura" en rutas es contrato de compatibilidad, no branding. | E.2: `AuraDeviceProbe` decide compatibilidad por existencia de `.rockbox/aura/` + `aura.cfg`; cambiar el prefijo rompe la detección sin ganancia. G.2. |
| **M-004** | **Distribución y `AuraUpdateChecker`**: Metro-Aura tiene su propio canal (GitHub Release de `Metro-Aura` con `rockbox.ipod`, `rockbox.zip`, `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`) e instalación **manual** documentada. Metro **no** escribe `.rockbox/aura/version.txt` (evita hacerse pasar por una versión de Aura) y **sí** escribe la clave nueva `firmware_family: metro` en `aura.cfg`. Se documenta como **trabajo de seguimiento fuera de este proyecto** (sesión dedicada al repo `Aura-Studio/`): que Studio reconozca `firmware_family` y no ofrezca "actualizar" ni instalar temas sobre un iPod Metro. Mientras eso no exista, `ESTADO_FINAL.md` advertirá al usuario del riesgo. | E.3, G.1: solo avisa, pero el flujo de instalación normal sobrescribiría Metro con Aura. Studio ignora claves desconocidas de `aura.cfg` (lee claves concretas por nombre) `[ESTIMADO]`. |
| **M-005** | **Marcador de sincronización desde el día uno**: se portan `aura_sync.c`/`aura_sync_marker.c` (renombrados `metro_sync*.c`), `aura.cfg` anuncia `sync_marker_supported: 1`. | E.4, G.6: código ya escrito y testeado; el fallback (Studio borra `.tcd`) es más lento sin beneficio. |
| **M-006** | **Toda la UI en C, en `apps/metro/`**, con el mismo gancho de arranque que Aura (`root_menu()` → `metro_main()`, una línea en `apps/main.c`; `root_menu.c` compilado pero inalcanzable). Skin engine, `gui_synclist`, `do_menu()`, `tree.c`, `tagtree.c`, `yesno`, `kbd_input` **no se usan** desde `apps/metro/`. | A.1–A.5: el skin engine no tiene animación ni twist; `gui_synclist` tiene altura de fila fija y sin movimiento horizontal. |
| **M-007** | **Input vía `get_custom_action()`** con tablas de keymap propias (`apps/metro/metro_keymap.c`), **no** lectura cruda de botones. Velocidad de rueda vía `get_action_data()` (`apps/action.h:418`, devuelve `intptr_t`); aceleración vía `button_apply_acceleration()` (`firmware/export/button.h:90`). | C.3/C.4/G.4: el filtrado de `BUTTON_REL` viene resuelto por el patrón "prereq button"; tablas declarativas encajan con la jerarquía del twist; no toca `keymap-ipod.c` ni `action.h`. |
| **M-008** | **Piezo apagado por defecto** (`global_settings.keyclick` queda en 0 = off; sin ajuste en la UI v1). | C.7, F.8: fidelidad al pad silencioso del Zune 30; el default de Rockbox ya es off. |
| **M-009** | **Idioma**: tabla de cadenas propia (`metro_lang.c`, sin `.lang` de Rockbox), **bilingüe ES/EN, español por defecto**, ajuste "Idioma" en Ajustes. Encabezados y pivots en minúsculas en ambos idiomas (lenguaje Metro). | A.9, G.8: mecanismo de Aura (que ya es ES/EN); el dueño y Aura Studio operan en español; el inglés preserva la fidelidad al Zune para quien lo prefiera. |
| **M-010** | **Tipografía**: **Selawik** (SIL OFL) vendoreada en `firmware/assets/fonts-src/`, convertida con `tools/convttf` a `.fnt` por `firmware/tools/gen_fonts.sh` (salida versionada en `firmware/assets/fonts/`). Escala: `display` = Selawik Light 48px, `title` = Light 28px, `list` = Regular 20px, `list_sel` = Semibold 20px, `caption` = Regular 14px (5 fuentes). Carga con **`font_load_ex(path, 0, 400)`** (carga completa desde buflib, sin caché de glifos), rango de caracteres `0x20–0x17F` (latín básico + Latin-1 + Latin Extended-A; suficiente para español). Fallback a `FONT_SYSFIXED` si falta cualquier `.fnt`. Flujo opcional "el usuario aporta Zegoe/Segoe" = documentación de cómo generar `.fnt` con los mismos parámetros y colocarlos en `.rockbox/fonts/` con los nombres convenidos (Metro prefiere esos archivos si existen y pasan `font_load_ex`). | Errata A.7 (`MAX_FONT_SIZE=60000`, `font_load_ex` carga completa), F.6 (convttf probado a 24/42/64px sin errores), F.7 (licencia). Presupuesto de RAM estimado ~380 KB en buflib para las 5 fuentes `[ESTIMADO]`. |
| **M-011** | **`MAXUSERFONTS` se queda en 12 (upstream)** — Metro necesita 5 slots. `firmware/export/font.h` no se modifica. | D.1 (font.h era MIXTO); menos diff contra upstream. |
| **M-012** | **Paleta**: base oscura por defecto (bg `#000000`, fg `#FFFFFF`, secundario `#999999`, terciario `#666666`), tema claro opcional (bg `#FFFFFF`, fg `#000000`, secundario `#666666`, terciario `#999999`). 10 acentos WP7 (F.5), **acento por defecto magenta `#FF0097`**. Selección en listas = texto blanco (Semibold) vs. gris (Regular), sin caja de resalte; el acento se usa con moderación: tiles, barra de progreso, letra índice, estado activo (shuffle/repeat), título de página. Colores solo desde `metro_palette.h` (cero RGB hardcodeado en pantallas). | F.5 (error RGB565 ≤ 1%), F.2 ("content before chrome"). Fidelidad exacta a la selección del Zune 30 `[ESTIMADO]`. |
| **M-013** | **Buffers de transición**: 2 framebuffers estáticos en BSS de 320×240×2 bytes (150 KB c/u) — `s_fb_from`, `s_fb_to` — más el framebuffer del LCD. Sin alocación dinámica. | B.3 (patrón de Aura verificado), A.10. |
| **M-014** | **FPS objetivo 30 fps nominal (33 ms/cuadro), piso 20 fps**; transiciones de **≤ 300 ms, cuadros fijos con `sleep()`** entre cuadros (patrón Aura), `cpu_boost(true/false)` **por transición individual**, guard `lcd_active()`, `drain_button_queue_if_full()` por cuadro, no interrumpibles (≤ 300 ms). | B.2, B.5–B.10: 4× de boost tiene costo real; transiciones puntuales, no sesión larga. Cifras a validar en hardware (B.11). |
| **M-015** | **Niveles de FX en matriz de 2 ejes** persistidos en `aura.cfg`: `animations = all \| minimal \| off` (cómo se mueve) × `graphics = full \| lite` (qué se dibuja: fondo de carátula atenuado, blends por píxel). Canon = máxima fidelidad; los niveles inferiores son sustracciones sobre el mismo código. **Auto-degradación por sesión**: si una transición mide > 2× su presupuesto en 3 ejecuciones consecutivas (`METRO_TRACE`), baja un nivel de `animations` solo para esa sesión y lo registra. | B.10 (sistema de Aura validado en producción), B.11 (método de medición). El umbral 2×/3 es `[ESTIMADO]`. |
| **M-016** | **Videos** se reproducen con el plugin `mpegplayer` stock (con `plugin_set_silent_open_errors(true)`); el modo "cubrir pantalla" de Aura (D-304/305/308) se porta como **fase opcional** (backlog), no en v1. **Fotos** se abren con el plugin `imageviewer` stock en v1 (visor propio en backlog). | D.1 (mpegplayer es MIXTO), E.6 (formato MPEG-1/2 320×240 fijo), reglas del encargo (no reemplazar el reproductor). Superficies de Rockbox del plugin quedan documentadas como pendientes en `ESTADO_FINAL.md`. |
| **M-017** | **Ajustes de Rockbox reutilizados como almacén** (`global_settings` + `settings_save()`) para volumen, brillo, tiempo de retroiluminación, shuffle, repeat; **ajustes propios en `aura.cfg`** (`metro_settings.c`): tema, acento, animaciones, gráficos, idioma, `sync_marker_supported`, `firmware_family`, `tz_local_quarters`, `rtc_sync_*` (transitorias). Cada pantalla de Ajustes es una lista Metro que llama al setter correspondiente. | A.9, E.1 (claves que Studio lee/escribe), D.2 (`aura_settings.c` es INFRA parcial). |
| **M-018** | **Iconografía mínima, compilada en el binario** (`apps/bitmaps/native/metro_*.bmp` vía `bmp2rb`): batería (3 estados), play/pausa, shuffle/repeat, y el wordmark de arranque `rockboxlogo.320x98x16.bmp` (mismo nombre, wordmark "metro" en Selawik Light). Nunca se leen bitmaps de disco por cuadro. | A.11, B.10 regla 1, D.1 (mecanismo del logo). |
| **M-019** | **Higiene de `global_settings` al arrancar** (en `metro_main()`, una vez por arranque): `statusbar=STATUSBAR_OFF`, `backdrop_file="-"`, `show_shutdown_message=false`, `talk_menu=false`, `clear_settings_on_hold=false`, `usb_hid=false` (bajo `USB_ENABLE_HID`), `tagcache_ram=true`, `keyclick=0`. | D.1 (`apps/main.c` MIXTO: la higiene es política de producto cerrado, no estética). |
| **M-020** | **Nombre visible**: "Metro" en la UI (wordmark minúsculas), "Metro-Aura" en documentación/releases; cero logotipos/nombres Zune, Windows Phone, Zegoe/Segoe en pantalla o en el árbol. Licencia GPL v2 (`COPYING` de Rockbox como `LICENSE`), `MODIFICATIONS.md` desde F0. | F.9, D.3. |

Decisiones menores resueltas en el cuerpo del plan (sin ID propio): pantalla USB re-vestida en C (no la de Rockbox), splash re-vestido por gancho en `apps/gui/splash.c` (mecanismo D-055), sin bloqueo por PIN en v1 (`misc.c` D-238 no se porta), sin presets de EQ propios en v1 (`settings.h` no se porta), sin `solitaire.c`.

---

## §0 — Fase Cero de separación

**Objetivo**: un Rockbox limpio para ipod6g en `Metro-Aura/`, con **solo** los fixes de hardware/build/compatibilidad de Aura-Firmware, la UI stock de Rockbox intacta y funcional, compilando simulador y target. Cero código de UI de Aura ni de Metro. Es la base "usable desde el día 0".

### 0.1 Siembra del repositorio

| Paso | Detalle |
|---|---|
| 0.1.1 | Copiar el upstream `0726ec93` a `Metro-Aura/firmware/rockbox/` (fuente: tarball de GitHub `archive/0726ec93517a61f602679ab052b083217ec9c96d.tar.gz`; sin `.git` interno). Verificar hash del tarball o del árbol contra el de Aura para los archivos no modificados (`diff -rq` de una muestra: `apps/gui/`, `firmware/target/arm/s5l8702/`). |
| 0.1.2 | `LICENSE` = `firmware/rockbox/docs/COPYING` (GPL v2). `MODIFICATIONS.md` (GPL §2a): origen, commit base, fecha, lista de archivos modificados (inicialmente los de 0.2). `README.md` (qué es, cómo compilar, estado). `CLAUDE.md` del repo (reglas: código/commits en inglés, docs en español, todo cambio a un archivo de Rockbox fuera de `apps/metro/` va a `MODIFICATIONS.md` en la misma pasada y se marca `Metro (M-NNN)` inline; cabecera GPL en todo `.c/.h` nuevo; ningún RGB fuera de `metro_palette.h`; ninguna lectura de disco en bucles de animación; ninguna llamada a `apps/gui/*`, `do_menu()`, `tree.c`, `tagtree.c`, `kbd_input()`, `gui_syncyesno()` desde `apps/metro/`). `DECISIONS.md` con M-001…M-020. `.gitignore`: `firmware/build-*/`, `firmware/toolchain/`, `firmware/test-media/`, `firmware/dist/*` salvo `README.md`, `.DS_Store`, `*.dSYM`. |
| 0.1.3 | `docs/` ya contiene `PLAN_INVESTIGACION.md`, `INVESTIGACION.md`, `PLAN_MAESTRO.md`; se añaden `docs/DESVIACIONES.md` (vacío con plantilla) y `docs/screenshots/` (capturas de criterio de hecho por fase, versionadas). |

### 0.2 Archivos de Rockbox que se portan desde Aura-Firmware (lista cerrada)

Rutas relativas a `firmware/rockbox/`. "Portar intacto" = copiar el archivo de Aura-Firmware tal cual y verificar con `diff` que el único delta contra upstream es el documentado.

| Archivo | Qué se porta | Cómo | Marca inline |
|---|---|---|---|
| `apps/tagcache.c` | Todo el delta de Aura (D-021/D-244/D-293): contador `tc_build_jobs_done`, temporal huérfano, fix de fuga en simulador, fix de `load_ramcache()`, `commit()` con buffer temporal, confirmación silenciosa del temporal al arrancar | Portar intacto | Sustituir comentarios `Aura (D-NNN)` por `Metro (M-005, from Aura D-NNN)` |
| `apps/tagcache.h` | `tagcache_get_build_jobs_done()`, `tagcache_has_pending_temp()`, `tagcache_discard_pending_temp()` | Portar intacto | Ídem |
| `firmware/export/config/ipod6g.h` | `CONFIG_BACKLIGHT_FADING BACKLIGHT_FADING_SW_HW_REG`; `USBPOWER_BTN_IGNORE` fuera de `#ifdef BOOTLOADER` | Portar intacto (solo esos dos bloques) | `Metro (from Aura D-06x/D-061)` |
| `bootloader/ipod-s5l87xx.c` | `verbose = false` bajo `#ifdef IPOD_6G` | Portar intacto | `Metro (from Aura D-064)` |
| `lib/rbcodec/codecs/aiff.c` | Tolerancia a AIFF de Music/iTunes | Portar intacto | `Metro (from Aura, 2026-08-12)` |
| `uisimulator/common/sim_tasks.c` | Inyección de botones + autodump headless | Portar y **renombrar** identificadores/variables de entorno `AURA_*` → `METRO_*` (`METRO_SIM_AUTODUMP_TICKS`, `METRO_SIM_AUTODUMP_QUIT`, `METRO_SIM_BUTTONS`) | `Metro (from Aura D-008/D-017)` |
| `utils/mks5lboot/Makefile` | Backend libusb opcional en macOS | Portar intacto | Aviso de modificación en cabecera |
| `apps/plugin.c` | `plugin_set_silent_open_errors()` | Portar intacto | `Metro (from Aura D-298)` |
| `apps/plugin.h` | Declaración | Portar intacto | Ídem |

**No se portan en F0** (y quedan fuera de la Fase Cero por diseño): `apps/main.c` (el gancho llega en F1), `apps/SOURCES` (F1), `apps/gui/splash.c` y `apps/gui/usb_screen.c` (F9), bitmaps (F1/F10), `apps/misc.c/.h` (D-238, no se necesita), `apps/settings.h` (EQ, no), `firmware/export/font.h` (M-011), `apps/plugins/mpegplayer/*` (backlog opcional), `apps/plugins/solitaire.c` (no), `firmware/target/hosted/filesystem-unix.c` (byte-idéntico).

### 0.3 Tooling portado (fuera del árbol Rockbox)

| Archivo en Metro-Aura | Origen en Aura-Firmware | Cambios |
|---|---|---|
| `firmware/tools/build_sim.sh` | `firmware/tools/build_sim.sh` | Quitar todo lo de `design-system/generate.py`/tokens/íconos; tras `make install`, copiar `firmware/assets/fonts/*.fnt` a `simdisk/.rockbox/fonts/` (a partir de F2, no hace daño en F0); soportar `--reconfigure`, `--run` |
| `firmware/tools/sim_shot.sh` | `apple2026_sim_shot.sh` | Renombrar; variables `METRO_SIM_*` |
| `firmware/tools/sim_matrix.sh` | `apple2026_sim_matrix.sh` | Renombrar; la matriz concreta de pantallas se define en F10 (aquí solo el esqueleto) |
| `firmware/tools/gen_test_media.sh` | mismo nombre | Renombrar `aura-test.*` → `metro-test.*`; añadir `cover.jpg` en la carpeta del álbum, un `.lrc`, un `.mpg` de 3 s (ffmpeg, MPEG-1 320×240), 3 JPEG ≤640px en `Photos/`, un `.m3u8` en `Playlists/`, y `video_categories.cfg`/`photo_categories.cfg` de muestra en `simdisk/.rockbox/aura/` |
| `firmware/tools/build_target.sh` | (nuevo, según `docs/guia-desarrollo.md` de Aura) | `configure --target=ipod6g --type=N` en `build-ipod6g/`, `--type=B` en `build-ipod6g-boot/`; `PATH` al toolchain (M-002) |
| `firmware/tools/build_toolchain.sh` | (nuevo, D-032) | Envuelve `rockboxdev.sh` con `RBDEV_PREFIX/TARGET=a/DOWNLOAD/BUILD` dentro de `firmware/toolchain/` |
| `firmware/tools/package_dist.sh` | mismo nombre | Se porta en F13 (release), no en F0 |
| `firmware/dist/README.md` | mismo | F13 |

### 0.4 Criterio de "hecho" de la Fase Cero

1. `firmware/tools/build_sim.sh` compila sin errores en este Mac; `sim_shot.sh docs/screenshots/F0-rockbox-stock.png 300` produce una captura del **menú principal stock de Rockbox** (prueba de que la UI stock sigue intacta).
2. `firmware/tools/build_target.sh` produce `build-ipod6g/rockbox.ipod` y `build-ipod6g-boot/bootloader-ipod6g.ipod` (toolchain compilado por `build_toolchain.sh`).
3. `diff -rq` entre `Metro-Aura/firmware/rockbox/` y el upstream de scratch lista **exactamente** los 9 archivos de §0.2 (más nada).
4. `MODIFICATIONS.md` lista esos 9 archivos; `DECISIONS.md` tiene M-001…M-020; `git status` limpio tras el commit.

---

## §1 — Capa de diseño: arquitectura de la UI Metro

### 1.1 Principios de arquitectura

1. **Todo en C** en `apps/metro/` (M-006). Rockbox core provee: reproducción (`playlist_*`, `audio_*`), índice (`tagcache_search*`), plugins (`plugin_load`), decodificación de imágenes (`read_jpeg_file`, `read_bmp_file`), fuentes (`font_load_ex`, `lcd_putsxy`, `font_getstringsize`), dibujo (`lcd_*`, `viewport_set_buffer`), sistema (`default_event_handler`, `settings_save`, `backlight_*`, `rtc_*`).
2. **Separación en cuatro capas**, cada una en archivos propios, sin dependencias hacia arriba:
   - **infra** (sin conocimiento de pantallas ni estética): `metro_settings`, `metro_sync*`, `metro_media_categories`, `metro_device`, `metro_music`, `metro_video`, `metro_photos`, `metro_playlists`, `metro_albumart`, `metro_lang`, `metro_fsutil`.
   - **motor de UI**: `metro_input` (+`metro_keymap`), `metro_nav` (máquina de navegación pura), `metro_draw` (shell de dibujo), `metro_fonts`, `metro_palette`/`metro_theme`, `metro_widgets` (lista, encabezado de pivots, tiles, barra de progreso, overlay de volumen/letra), `metro_fb` + `metro_transitions` + `metro_motion` (F11+).
   - **pantallas**: `metro_screen_hub`, `metro_screen_list` (genérica: pivots + lista), `metro_screen_nowplaying`, `metro_screen_settings`, `metro_screen_about`, `metro_screen_usb`, `metro_screen_splash`.
   - **orquestación**: `metro_main` (bucle, mantenimiento, disk handoff).
3. **Modelo de datos declarativo para las listas**: toda pantalla de lista es un `struct metro_page` con N `struct metro_pivot`, cada pivot con un proveedor (`count()`, `get_row(i, &row)`, `on_select(i)`, `on_play(i)`) — la pantalla genérica dibuja cualquier proveedor. Los proveedores viven en infra (`metro_music.c` expone `artists`, `albums`, `songs`, `genres`, `playlists`, `albums_of_artist`, `songs_of_album`, …).
4. **Dibujo siempre relativo al viewport activo** (`lcd_set_viewport`), nunca a coordenadas absolutas fijas del LCD — así una pantalla puede renderizarse a un buffer offscreen sin cambios (base de las transiciones, B.3).
5. **Cero cromo de Rockbox visible**: superficies catalogadas en `Aura-Firmware/docs/superficies-rockbox.md` se cubren en F9 con los mismos mecanismos (settings + re-skin genérico de `splash_internal()` + pantalla USB propia).

### 1.2 Archivos nuevos (`firmware/rockbox/apps/metro/`)

| Archivo | Responsabilidad | Fase |
|---|---|---|
| `SOURCES` (bloque añadido al final de `apps/SOURCES`) | Lista de `.c` de Metro | F1 |
| `metro_main.c/.h` | `metro_main()`: higiene M-019, carga de fuentes/ajustes, bucle principal (`get_custom_action` → despacho a la pantalla activa → `default_event_handler` para lo no consumido; captura `SYS_POWEROFF` para dibujar apagado; `SYS_USB_CONNECTED` → `metro_screen_usb` → al volver, `metro_disk_handoff()`), `metro_disk_handoff()` (aplica `rtc_sync_*`, `metro_sync_check_pending()`, `metro_device_reload()`), temporizador de reloj/batería para redibujo de cabecera | F1, F6 |
| `metro_input.c/.h` | Envuelve `get_custom_action(ctx, timeout, metro_keymap_get)`; expone `metro_input_next(ctx, timeout)` devolviendo `enum metro_action` + `wheel_steps` (aplicando `button_apply_acceleration(get_action_data())`) + `wheel_velocity` cruda | F3 |
| `metro_keymap.c/.h` | Tablas `button_mapping[]` por contexto Metro: `MCTX_LIST`, `MCTX_HUB`, `MCTX_PLAYER`, `MCTX_OPTIONS`; función `metro_keymap_get(int ctx)`; acciones `MACT_*` (tabla en §2.3) | F3 |
| `metro_nav.c/.h` (+ `test/test_nav.c`) | Pila de páginas (`MAX_DEPTH=8`), índice de pivot y selección por pivot, operaciones `push/pop/pop_to_root/pivot_next/pivot_prev/select_move(delta)`; sin dependencias de Rockbox (host-testable) | F3 |
| `metro_page.h` | `struct metro_row {const char *title; const char *subtitle; enum row_kind kind; int icon;}`, `struct metro_pivot {const char *name; count/get_row/on_select/on_play; void *ctx;}`, `struct metro_page {title; pivots[]; npivots; flags;}` | F3 |
| `metro_fonts.c/.h` | Roles `MFONT_DISPLAY/TITLE/LIST/LIST_SEL/CAPTION`, rutas `/.rockbox/fonts/metro-<rol>.fnt` (nombres: `metro-display-48.fnt`, `metro-title-28.fnt`, `metro-list-20.fnt`, `metro-listsel-20.fnt`, `metro-caption-14.fnt`), carga con `font_load_ex(path, 0, 400)`, fallback `FONT_SYSFIXED`, `metro_font_id(rol)`, `metro_text_width(rol, str)` | F2 |
| `metro_palette.h` | Constantes RGB565 (`LCD_RGBPACK`) de M-012; tabla de 10 acentos; `metro_color(role)` resuelve según tema activo | F2 |
| `metro_theme.c/.h` | Tema (dark/light) + acento activos, lectura/escritura vía `metro_settings` | F2/F8 |
| `metro_draw.c/.h` | Primitivas de alto nivel sobre `lcd_*`: `metro_draw_text(rol, x, y, str, color, clip_w)`, `metro_draw_text_cut_right()` (recorte en borde derecho, A.6), `metro_draw_header(page_title)` (línea superior: título en caption + hora + batería), `metro_draw_pivots(page, active, x_offset)`, `metro_draw_rows(pivot, first, sel, x_offset)`, `metro_draw_tile(rect, label, art)`, `metro_draw_progress(rect, pct)`, `metro_draw_battery(x,y)`, `metro_draw_clear()` | F2, F3 |
| `metro_widgets.c/.h` | Overlay de volumen (barra inferior 1.5 s), letra índice flotante en scroll rápido, estado vacío ("no music" con tile de acento), diálogo de 2 opciones (sí/no) propio | F5, F10 |
| `metro_lang.c/.h` | `enum metro_str`, tablas ES/EN, `metro_str(id)`; append-only | F3 |
| `metro_settings.c/.h` | Lee/escribe `/.rockbox/aura/aura.cfg` (`settings_parseline()` + `read_line()`, buffer 64 bytes por línea, regenera el archivo entero en cada guardado): claves `firmware_family: metro`, `sync_marker_supported: 1`, `theme`, `accent`, `animations`, `graphics`, `language`, `tz_local_quarters`, `rtc_sync_*` (leídas y descartadas), `first_boot_done`; garantiza que el archivo **existe desde el primer arranque** (E.2) | F6 |
| `metro_sync.c/.h`, `metro_sync_marker.c/.h` (+ `test/test_sync_marker.c`) | Port de `aura_sync*.c`: marcador `/.aura/sync-pending.json`, `Q_UPDATE`/`Q_REBUILD`, `attempts`, pantalla de progreso "updating library…" (única pantalla completa de espera) | F6 |
| `metro_device.c/.h` (+ `test/test_device.c`) | Lee `/.rockbox/aura/device.cfg` (`device_name` para "about") | F6 |
| `metro_media_categories.c/.h` | Port de `aura_media_categories.c` (`video_categories.cfg`, `photo_categories.cfg`) | F7 |
| `metro_music.c/.h` | Port de la parte infra de `aura_music.c`: proveedores sobre `tagcache_search*` (`artists`, `albums`, `songs`, `genres`, `albums_of_artist`, `songs_of_artist`, `songs_of_album`, `albums_of_genre`), arranque de reproducción (`playlist_create(NULL,NULL)` + `playlist_insert_track/…` + `playlist_start(index, 0, 0)` (`void`, `apps/playlist.h:127`)), `shuffle_all`, playlists de `/Playlists/*.m3u8` (`playlist_create(dir,file)`), consulta de estado (`audio_status()`, `audio_current_track()`), `metro_music_db_ready()` (cede mientras `metro_sync_job_active()`) | F4 |
| `metro_video.c/.h` | Escaneo de `/Videos` (`.mpg/.mpeg`, `VIDEO_NAME_LEN=96`, tope 100, orden natural), categorías, lanzamiento `plugin_load(VIEWERS_DIR"/mpegplayer.rock", path)` con `plugin_set_silent_open_errors(true)` | F7 |
| `metro_photos.c/.h` | Escaneo de `/Photos` (`.jpg/.jpeg`, `PHOTO_NAME_LEN=96`, tope 500), categorías, lanzamiento `imageviewer.rock` | F7 |
| `metro_albumart.c/.h` | Carátula de la pista actual: `find_albumart()` (core) + `read_jpeg_file()`/`read_bmp_file()` a un buffer estático 136×136 (`FORMAT_NATIVE|FORMAT_RESIZE|FORMAT_KEEP_ASPECT`, flags en `firmware/export/lcd.h:552-555`; firma `read_jpeg_file(path, &bm, maxsize, format, cformat)` en `apps/recorder/jpeg_load.h:41`), caché por ruta (1 entrada); nunca en bucle de animación | F5 |
| `metro_fsutil.c/.h` | Helpers de directorio (orden natural, filtros por extensión) | F7 |
| `metro_screen_hub.c/.h` | Raíz twist: lista vertical en fuente `display` (`now playing` solo si `audio_status()&AUDIO_STATUS_PLAY`, `music`, `videos`, `photos`, `settings`), pitch 52 px, recorte en borde inferior | F3 |
| `metro_screen_list.c/.h` | Pantalla genérica: cabecera + encabezado de pivots + lista del pivot activo; ventaneo con selección; PLAY sobre fila reproducible | F3 |
| `metro_screen_nowplaying.c/.h` | Ahora suena (layout §1.4), controles, overlay de volumen, página de opciones (up next / shuffle / repeat) como `metro_page` normal | F5 |
| `metro_screen_settings.c/.h` | Páginas de Ajustes (lista raíz + sublistas de opciones) | F3 (esqueleto), F8 (real) |
| `metro_screen_about.c/.h` | "about": nombre de dispositivo (`device.cfg`), conteos (`sync_summary.cfg`), versión, "based on Rockbox", ms medidos de FX (debug) | F8 |
| `metro_screen_usb.c/.h` | Pantalla USB propia (wordmark + "connected"), llamada desde `metro_main` al recibir `SYS_USB_CONNECTED` antes de `default_event_handler` | F9 |
| `metro_screen_splash.c/.h` | Pantalla de arranque (tras el logo bitmap: wordmark + barra fina de progreso mientras carga tagcache/fuentes) y de apagado | F1/F9 |
| `metro_fb.c/.h` | Buffers estáticos `s_fb_from/s_fb_to`, `metro_fb_capture()`, `metro_fb_render(fb, draw_fn, ctx)` (vía `viewport_set_buffer`), `metro_fb_present_slide(from,to,dx)`, `metro_fb_present_fade(from,to,a)`, `metro_fb_present_turnstile(src,angle,dir)` | F11/F12 |
| `metro_motion.c/.h` (+ `test/test_motion.c`) | Easings en punto fijo 0..256: `ease_out_expo` (tabla 16 entradas), `ease_out_quad`, lineal | F11 |
| `metro_transitions.c/.h` | Catálogo (§3.3), niveles (M-015), `METRO_TRACE`, auto-degradación | F11/F12 |
| `test/Makefile`, `test/*.c` | Tests host-side (`make -C apps/metro/test test`) | F3+ |

Presupuesto estimado: ~9 000–11 000 líneas de C nuevas `[ESTIMADO]` (Aura tiene ~26 000 en `apps/aura/`; Metro es deliberadamente más chico).

### 1.3 Archivos de Rockbox modificados (fuera de `apps/metro/`)

| Archivo | Cambio | Fase | Marca |
|---|---|---|---|
| `apps/main.c` | Tras `init()`: `metro_main(); /* never returns */` en lugar de `root_menu()`; sin texto de versión en `show_logo_boot()`; splash de arranque centrado | F1 | `Metro (M-006)` |
| `apps/SOURCES` | Bloque `#ifndef BOOTLOADER` … lista de `metro/*.c` … `#endif` al final | F1 | `Metro (M-006)` |
| `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` | Wordmark "metro" (Selawik Light, blanco sobre negro), mismas dimensiones; generado por `firmware/tools/gen_logo.py` (Pillow) desde `firmware/assets/fonts-src/` | F1 | — (bitmap) |
| `apps/bitmaps/native/usblogo.176x48x16.bmp` | Wordmark "usb", mismas dimensiones | F9 | — |
| `apps/bitmaps/native/SOURCES` + nuevos `metro_battery_{full,mid,low}.bmp`, `metro_play.bmp`, `metro_pause.bmp`, `metro_shuffle.bmp`, `metro_repeat.bmp` (16 bpp, ≤ 24×24) | Íconos compilados (M-018) | F5 | — |
| `apps/gui/splash.c` | En `splash_internal()`, antes de dibujar: `metro_splash_restyle(&buf, &fg, &bg)` (traduce/normaliza el mensaje contra tabla y fuerza colores de `metro_palette`); mecanismo D-055 | F9 | `Metro (from Aura D-055)` |
| `apps/gui/usb_screen.c` | Fix de centrado del logo (D-223) + colores desde `metro_palette` (la pantalla completa la dibuja `metro_screen_usb`, pero `usb_screen.c` sigue siendo el fallback del core) | F9 | `Metro (from Aura D-223)` |
| `uisimulator/common/sim_tasks.c` | (F0) | F0 | — |
| Los 9 de §0.2 | (F0) | F0 | — |

Todo cambio se registra en `MODIFICATIONS.md` en la misma pasada.

### 1.4 Especificación visual (320×240, tema oscuro por defecto)

**Rejilla base**: margen izquierdo 12 px, margen derecho 0 (el texto se recorta en x=320), cabecera 0–24, contenido desde y=24.

**Cabecera (todas las pantallas salvo splash/USB)**: y=0..24. Izquierda: título de página en `caption` gris (`"music"`, `"settings"`); derecha: hora `HH:MM` en `caption` gris (si `rtc` válida) + ícono batería 16×8. Sin línea separadora.

**Hub (raíz)**: filas en `display` (48 px), pitch 52, primera fila y=32; seleccionada blanca, resto `#999`; la 5.ª fila se corta en el borde inferior (240) por diseño; ventaneo cuando hay > 4 ítems.

**Página de lista (pivots)**: encabezado de pivots en `display` a y=28 (baseline ≈ 76): pivot activo en blanco desde x=12; siguientes pivots a la derecha, separados por 24 px, en `#666`, recortados en x=320; el pivot anterior no se dibuja (WP7). Lista desde y=84: pitch 28 px, título en `list_sel` blanco (seleccionada) o `list` `#999` (resto), subtítulo opcional (`caption`, `#666`) en la misma fila a la derecha o debajo (pitch 36 si hay subtítulos). Visible: 5 filas (pitch 28) o 4 (pitch 36); la siguiente fila asoma cortada. Ventaneo: la selección se mantiene en las filas 0..3; al pasar de la 3, la lista se desplaza. Sin scrollbar. Scroll rápido (aceleración ≥ 3 ítems/evento): letra índice flotante en `display` acento, centrada, sobre fondo bg, 600 ms tras el último evento.

**Ajustes**: misma página de lista sin pivots (o con pivots `general | display | about`); filas con valor a la derecha en `caption` acento (`theme  dark`).

**Now playing**: cabecera con `now playing`; carátula 136×136 en (12, 40) (o tile acento con inicial del álbum si no hay carátula); columna de texto en x=160: título en `title` (28) blanco, 2 líneas máx. con recorte; artista `list` `#999`; álbum `caption` `#666`; y=200: tiempo transcurrido `caption` izquierda / restante derecha; y=214: barra de progreso 320×4 acento sobre `#333`; estados shuffle/repeat como íconos 16 px en la esquina inferior derecha en acento cuando activos. Overlay de volumen: barra 320×6 en y=232 durante 1.5 s tras mover la rueda + `caption` "volume 60%".

**Estados vacíos**: tile acento 96×96 centrado con texto `caption` ("no music yet — sync with Aura Studio").

**Splash de arranque**: logo bitmap (wordmark) + barra fina acento 120×2 bajo el wordmark mientras `!tagcache_is_usable()`/fuentes cargan.

**Tema claro**: mismos layouts con la paleta invertida (M-012).

### 1.5 Qué NO se hace en el skin engine

Nada. No se instalan `.wps/.sbs/.fms` (se conserva `rockbox_failsafe.*` compilado por defecto, nunca activo porque no se entra a `root_menu`). Justificación A.4/A.5.

---

## §2 — Navegación: esqueleto twist del Zune 30 sobre clickwheel

### 2.1 Modelo

- **Página** = título + 1..N **pivots** hermanos (misma profundidad). El twist horizontal (LEFT/RIGHT) cambia de pivot; la rueda mueve la selección dentro del pivot; SELECT profundiza (push de una página nueva); MENU vuelve (pop).
- **Pila** de hasta 8 páginas. Cada entrada guarda `pivot_index` y `sel[pivot]`, así "volver" restaura exactamente dónde estaba el usuario (Zune 30).
- **Now playing** es una página especial que se **empuja** sobre la pila (desde el hub, o automáticamente al iniciar reproducción desde una lista) y se **saca** con MENU; PLAY corto en cualquier página = play/pausa global.

### 2.2 Árbol de navegación v1

```
hub (raíz, fuente display)
├── now playing            (solo si hay audio activo)  → página NP
├── music                  → pivots: artists | albums | songs | genres | playlists
│   ├── artists → [artista] → pivots: albums | songs   → [álbum] → songs → [canción] ▶ NP
│   ├── albums  → [álbum: título / subtítulo artista] → songs → [canción] ▶ NP
│   ├── songs   → [canción] ▶ NP (cola = todo el pivot desde la selección)
│   ├── genres  → [género] → pivots: albums | songs → …
│   └── playlists → [lista .m3u8] → songs → [canción] ▶ NP
├── videos                 → pivots: all | movies | series | clips   (pivots de categoría solo si video_categories.cfg existe)
│   └── [video] ▶ mpegplayer (plugin) → al volver, misma posición
├── photos                 → pivots: all | photos | images | ai       (ídem photo_categories.cfg)
│   └── [foto] ▶ imageviewer (plugin)
└── settings               → pivots: general | display | about
    ├── general: language (es/en) · animations (all/minimal/off) · graphics (full/lite) · library (update now / rebuild) · reset settings
    ├── display: theme (dark/light) · accent (10) · brightness (1..N) · backlight timeout (5s…always)
    └── about: device name · tracks/videos/photos (sync_summary.cfg) · version · "based on Rockbox" · fx trace (debug)
NP (now playing) — SELECT → página "options": up next (lista de la cola) | shuffle on/off | repeat off/all/one
```

Reglas: al elegir una canción en un pivot `songs`, la cola es todo el pivot en orden y se empieza en la seleccionada; PLAY sobre un artista/álbum/género/playlist = reproducir todo en orden (SELECT sostenido = shuffle all); al terminar de reproducir todo, NP se queda con la última pista en pausa (comportamiento Rockbox).

### 2.3 Tabla completa gesto Zune 30 → evento clickwheel → acción Metro

Contextos Metro: `HUB` (raíz), `LIST` (páginas con pivots/lista, incluye Ajustes), `PLAYER` (now playing), `OPTIONS` (página de opciones de NP, es una `LIST` con teclas de transporte activas), `DIALOG` (sí/no propio), `PLUGIN` (mpegplayer/imageviewer: keymap del propio plugin, fuera de este mapa).

| Gesto Zune 30 | Contexto | Evento clickwheel (bit + modificador) | Acción `MACT_*` | Comportamiento |
|---|---|---|---|---|
| Pad arriba / abajo (recorrer lista) | HUB, LIST, OPTIONS | `BUTTON_SCROLL_BACK` / `BUTTON_SCROLL_FWD` (con o sin `BUTTON_REPEAT`) | `MACT_PREV` / `MACT_NEXT` | Mueve la selección `wheel_steps` posiciones (`button_apply_acceleration`); ≥ 3 pasos/evento activa la letra índice |
| Pad arriba / abajo | PLAYER | `BUTTON_SCROLL_BACK` / `BUTTON_SCROLL_FWD` | `MACT_VOL_DOWN` / `MACT_VOL_UP` | Volumen ±1 por evento (con aceleración), overlay 1.5 s |
| Pad izquierda / derecha (twist entre pivots) | LIST, OPTIONS | `BUTTON_LEFT\|BUTTON_REL` / `BUTTON_RIGHT\|BUTTON_REL` (prereq: el mismo botón sin REL) | `MACT_PIVOT_PREV` / `MACT_PIVOT_NEXT` | Cambia de pivot con transición SLIDE (F11); en el primer/último pivot no hace nada (sin wrap, como WP7 Pivot no-táctil) |
| Pad izquierda / derecha | HUB | `BUTTON_LEFT\|BUTTON_REL` / `BUTTON_RIGHT\|BUTTON_REL` | `MACT_NONE` | Sin efecto (la raíz no tiene pivots) |
| Pad izquierda / derecha (pista anterior/siguiente) | PLAYER | `BUTTON_LEFT\|BUTTON_REL` / `BUTTON_RIGHT\|BUTTON_REL` | `MACT_TRACK_PREV` / `MACT_TRACK_NEXT` | `audio_prev()` (si > 3 s reproducidos, reinicia la pista) / `audio_next()` |
| Mantener izquierda / derecha (rebobinar/adelantar) | PLAYER | `BUTTON_LEFT\|BUTTON_REPEAT` / `BUTTON_RIGHT\|BUTTON_REPEAT` | `MACT_SEEK_BACK` / `MACT_SEEK_FWD` | `audio_ff_rewind()` acelerado mientras se sostiene; al soltar (`REL`) reanuda |
| Centro / OK (entrar) | HUB, LIST | `BUTTON_SELECT\|BUTTON_REL` (prereq `BUTTON_SELECT`) | `MACT_SELECT` | Fila navegable → `push(page)` con transición PUSH; fila reproducible (canción) → reproducir + `push(NP)`; fila de ajuste → cicla valor o abre sublista |
| Centro / OK | PLAYER | `BUTTON_SELECT\|BUTTON_REL` | `MACT_OPTIONS` | Empuja página `options` (up next / shuffle / repeat) |
| Mantener centro (acción secundaria) | LIST (fila de artista/álbum/género/playlist) | `BUTTON_SELECT\|BUTTON_REPEAT` | `MACT_SHUFFLE_ALL` | Reproduce todo el contenido de la fila en aleatorio + `push(NP)` |
| Mantener centro | PLAYER | `BUTTON_SELECT\|BUTTON_REPEAT` | `MACT_TOGGLE_SHUFFLE` | Alterna shuffle (feedback: ícono acento) |
| Back (volver) | LIST, OPTIONS, PLAYER | `BUTTON_MENU\|BUTTON_REL` (prereq `BUTTON_MENU`) | `MACT_BACK` | `pop()` con transición POP; en NP vuelve a la página desde la que se entró |
| Back | HUB | `BUTTON_MENU\|BUTTON_REL` | `MACT_NONE` | Sin efecto (ya está en la raíz) |
| Mantener Back (ir al inicio) | LIST, OPTIONS, PLAYER | `BUTTON_MENU\|BUTTON_REPEAT` | `MACT_HOME` | `pop_to_root()` — vuelve al hub `[ESTIMADO en fidelidad al Zune]` |
| Play/Pause | PLAYER, HUB, LIST sobre fila NO reproducible | `BUTTON_PLAY\|BUTTON_REL` (prereq `BUTTON_PLAY`) | `MACT_PLAYPAUSE` | `audio_pause()`/`audio_resume()`; si no hay nada cargado, no hace nada |
| Play/Pause sobre un ítem (reproducir lo seleccionado) | LIST sobre fila reproducible (canción, álbum, artista, género, playlist) | `BUTTON_PLAY\|BUTTON_REL` | `MACT_PLAY_ITEM` | Reproduce la fila (canción: desde ella; contenedor: todo en orden) + `push(NP)` |
| Mantener Play (apagar) | todos | `BUTTON_PLAY\|BUTTON_REPEAT` sostenido ~3 s | (reservado) | El driver (`button.c`, `POWEROFF_BUTTON=BUTTON_PLAY`, `POWEROFF_COUNT=40`) emite `SYS_POWEROFF`; Metro **no** asigna nada a `PLAY\|REPEAT`, y al recibir `SYS_POWEROFF` dibuja la pantalla de apagado y deja pasar el evento a `default_event_handler()` |
| Interruptor Hold | todos | (hardware: el sensor de rueda se apaga) | — | Sin eventos; Metro no hace nada especial |
| Reset (Menú+Centro sostenidos) | todos | (hardware/PMU) | — | Fuera del firmware |
| — (sin equivalente Zune) | DIALOG (sí/no propio) | `SCROLL_*` mueve, `SELECT\|REL` confirma, `MENU\|REL` cancela | `MACT_PREV/NEXT/SELECT/BACK` | Único diálogo modal del sistema (p. ej. "rebuild library?") |
| — | todos | `SYS_USB_CONNECTED` | — | `metro_screen_usb` → `default_event_handler()` (bloquea hasta desconectar) → `metro_disk_handoff()` |
| — | todos | `SYS_CHARGER_CONNECTED/DISCONNECTED`, `SYS_BATTERY_UPDATE` | — | Redibujar batería de cabecera |

Prereq de `BUTTON_REL`: en todas las filas con `|BUTTON_REL` la tabla lleva como tercer campo el botón sin REL — así un `REL` huérfano (el de la pulsación que abrió la pantalla) se descarta (C.3). Combinaciones `SELECT+PLAY`, `MENU+SELECT` no se mapean.

### 2.4 Aceleración de rueda

`metro_input` aplica `button_apply_acceleration(get_action_data())` sobre `MACT_NEXT/PREV` (curva de fábrica calibrada en hardware, C.2). En listas ≥ 40 filas y pasos ≥ 3, `metro_widgets_index_letter()` muestra la inicial de la fila destino. En el simulador la aceleración no se puede probar (C.6): el criterio de hecho en sim es solo dirección/umbral; el "feel" se ajusta en hardware al final (F13).

---

## §3 — Transiciones y animaciones (capa FINAL: F11–F12)

### 3.1 Infraestructura (`metro_fb.c`)

- `static fb_data s_fb_from[LCD_HEIGHT*LCD_WIDTH]`, `s_fb_to[...]` (M-013), envueltos en `struct frame_buffer_t` para `viewport_set_buffer()`.
- `metro_fb_capture(dst)`: `memcpy` del framebuffer del LCD (`lcd_framebuffer`/`FBADDR(0,0)`) → `dst` (150 KB).
- `metro_fb_render(dst, draw_fn, ctx)`: `viewport_set_buffer(vp, dst)` → `draw_fn(ctx)` (la pantalla se dibuja completa una sola vez fuera de línea) → restaurar buffer.
- `metro_fb_present_slide(from, to, dx)`: para cada fila `y`, `memcpy` de `LCD_WIDTH-|dx|` píxeles de `from` desplazados y `|dx|` de `to` en la franja entrante → framebuffer del LCD; `lcd_update()`. Costo ≈ 2 `memcpy` de 150 KB por cuadro (B.3).
- `metro_fb_present_fade(from, to, alpha256)`: blend por píxel (multiplicación + shift, sin división: `((to-from)*a>>8)+from` por canal en RGB565 desempaquetado; ~77 k píxeles/cuadro) (B.4). Solo con `graphics=full`.
- `metro_fb_present_turnstile(src, angle256, dir)`: proyección por columna estilo PictureFlow: para cada columna de pantalla `x`, columna fuente `xs` y escala vertical `sy` desde una tabla precalculada de 32 ángulos × 320 columnas (punto fijo 16.16), copia vertical con paso fijo (B.5). Fondo `bg` donde no hay proyección.
- Todas presentan con `lcd_update()` completo (el `memcpy` a `lcd_dblbuf` + DMA es el costo fijo, B.1).

### 3.2 Bucle de animación canónico (`metro_transitions.c`)

```
if (!lcd_active() || level == ANIM_OFF) { draw_target_directly(); return; }
cpu_boost(true);
for (i = 1; i <= frames; i++) {
    p = ease(i, frames);            /* 0..256, tabla */
    present(p);                     /* una de las primitivas de 3.1 */
    drain_button_queue_if_full();   /* D-074 */
    sleep(frame_delay);             /* nunca omitirlo: cede al hilo de audio */
}
cpu_boost(false);
METRO_TRACE("slide %d frames in %ld ticks", frames, current_tick - t0);
```

`frames`/`frame_delay` por nivel: `all` = 8 cuadros × 3 ticks (≈ 240–270 ms); `minimal` = 4 cuadros × 3 ticks (≈ 120–150 ms); `off` = 0. Easing por defecto `ease_out_expo` (WP7 exponente 6, tabla de 16 entradas generada offline en `metro_motion.c`, F.3).

### 3.3 Catálogo de transiciones Metro (qué, dónde, con qué primitiva)

| Transición | Dónde | Primitiva | `all` | `minimal` | `off` | Fase |
|---|---|---|---|---|---|---|
| **SLIDE** (twist entre pivots) | LIST/OPTIONS al `MACT_PIVOT_*` | `present_slide` con `dx` de 320→0 (dirección según sentido) | 8×3 ticks, ease_out_expo | 4×3 | instantáneo | F11 |
| **PUSH / POP** (profundizar / volver) | `push(page)` / `pop()` | v1: `present_slide` (entra desde la derecha; vuelve hacia la derecha) | 8×3 | 4×3 | inst. | F11 |
| **PUSH / POP turnstile** | Igual, sustituye a SLIDE cuando `animations=all` y `graphics=full` | `present_turnstile` (−80°→0° entrada, 0°→50° salida; ida en Y hacia la izquierda, vuelta al revés — F.3) | 8×3 (out 4 + in 8 en 12 cuadros ≈ 360 ms) | usa SLIDE | inst. | F12 |
| **FADE** (a/desde Now Playing, a/desde plugins) | `push(NP)`, vuelta de plugin | `present_fade` | 6×3 | usa SLIDE | inst. | F11 |
| **FEATHER** (entrada escalonada de filas) | Primera pintura de una página de lista tras PUSH | Redibujo de la lista N veces con `y_offset` decreciente por fila (offset inicial 8 px/fila, escalonado 1 cuadro/fila) sobre `s_fb_to` y `present_fade`/copia | 6 cuadros | off | off | F12 |
| **VOLUME OVERLAY** | PLAYER | Redibujo de la franja inferior (`lcd_update_rect`) | siempre (no es transición) | ídem | ídem | F5 |
| **INDEX LETTER** | LIST scroll rápido | Redibujo parcial | siempre | ídem | ídem | F10 |
| **CONTINUUM** (el título de la fila "vuela" a la cabecera de la página nueva) | PUSH desde una fila | Captura del rectángulo de la fila + reubicación con `lcd_bitmap_part` en 6 cuadros sobre `present_slide` | backlog | — | — | Backlog |
| **Fondo de carátula atenuada en NP** | PLAYER (`graphics=full`) | Carátula escalada a 320×240 (`read_jpeg_file` con `FORMAT_RESIZE`, una vez por pista, a un buffer estático propio de 150 KB — 3.er buffer, solo si `graphics=full`) + `present_fade` a 30 % sobre bg | estático | — | — | F12 |
| **Blur real**, **Roll**, **parallax continuo** | — | — | no se hacen (F.4) | | | — |

### 3.4 Instrumentación y fallback

- `METRO_TRACE(...)` = `DEBUGF` con `current_tick` (cero costo en release), en cada transición: cuadros, ticks, nivel.
- `metro_transitions_stats` (últimas 8 mediciones) visible en `about` en builds de depuración.
- Auto-degradación por sesión (M-015): > 2× presupuesto (presupuesto = `frames × frame_delay` ticks) en 3 ejecuciones consecutivas → `animations` efectivo baja un nivel hasta reinicio; se registra en el trace. El ajuste persistido del usuario no cambia solo.
- Antes de F11 no existe ninguna animación: la UI completa funciona con cambios instantáneos (M-014, orden del encargo).

---

## §4 — Compatibilidad con Aura Studio: checklist verificable

Se mantiene como `docs/COMPAT_STUDIO.md` (checklist viva) desde F6; aquí la versión inicial. Cada ítem tiene método de verificación en simulador (`simdisk/`) salvo donde se indica hardware.

| # | Requisito | Origen | Verificación |
|---|---|---|---|
| C1 | `/.rockbox/aura/` existe tras el primer arranque | E.2 | `ls simdisk/.rockbox/aura` tras `sim_shot` de arranque |
| C2 | `/.rockbox/aura/aura.cfg` existe tras el primer arranque y se regenera entero en cada guardado | E.2/E.1 | timestamp antes/después de cambiar un ajuste; `cat` del archivo |
| C3 | `aura.cfg` contiene `sync_marker_supported: 1` y `firmware_family: metro`; **no** contiene `theme_format_supported` | M-004/M-005/E.10 | `grep` |
| C4 | Un `/.aura/sync-pending.json` `{version:1, changes:{music:true}}` colocado a mano se procesa al arrancar (tagcache `Q_UPDATE`/`Q_REBUILD`), sube `attempts` durante el trabajo y se borra al terminar bien | E.4 | colocar marcador, `sim_shot` con 600 ticks, verificar ausencia del archivo y base actualizada (nueva pista visible en `songs`) |
| C5 | Marcador con `version: 2` se deja intacto | E.4 | ídem, el archivo sigue |
| C6 | Al volver de la pantalla USB (sim: `SYS_USB_CONNECTED` inyectable vía sim) se ejecuta `metro_disk_handoff()` | E.9/D.8 | trace en log del sim |
| C7 | Claves `rtc_sync_*` completas en `aura.cfg` se aplican al RTC y se descartan en el siguiente guardado | E.1 v7 | escribir claves a mano, arrancar, ver hora en cabecera y `aura.cfg` sin las claves |
| C8 | Tagcache indexa `/Music/` en cualquiera de los 3 layouts (artista/álbum, álbum, artista) | E.1 | `gen_test_media.sh` genera los tres layouts; `artists/albums/songs` muestran todo |
| C9 | `cover.jpg` en la carpeta del álbum se muestra en NP; carátula embebida JPEG (APIC) también | E.5 | test-media con ambas variantes; captura de NP |
| C10 | `.lrc` junto al audio no rompe nada (v1 no lo muestra; backlog) | E.1 | sin efecto |
| C11 | `Playlists/*.m3u8` con rutas absolutas UNIX se listan en `music › playlists` y reproducen | E.8 | captura + reproducción en sim |
| C12 | `/Videos/*.mpg` con nombre de 95 bytes UTF-8 se lista y abre con mpegplayer; póster `.jpg` hermano no rompe el listado | E.1/E.7 | test-media con nombre límite |
| C13 | `/Photos/*.jpg` ≤ 640 px se listan (hasta 500) y abren con imageviewer | E.1 | captura |
| C14 | `video_categories.cfg`/`photo_categories.cfg` presentes → pivots de categoría; ausentes → solo `all` | E.1 D.2 | ambos casos capturados |
| C15 | `sync_summary.cfg` presente → conteos en `about`; ausente → guiones | E.1 | ambos casos |
| C16 | `device.cfg` presente → nombre en `about`; ausente → etiqueta genérica | E.1 | ambos casos |
| C17 | `artists/`+`artist_images.cfg`, `ratings.cfg`, `themes/`, `sync_manifest.json` presentes **no rompen nada** (se ignoran en v1) | E.1 | colocar archivos de muestra, navegar todo |
| C18 | Ninguna ruta/clave del contrato se escribe con otro nombre; ningún archivo del contrato que Studio escribe es borrado por Metro (salvo el marcador al terminar) | E.1 | revisión de código + `find simdisk` antes/después |
| C19 | Descriptor USB = stack Rockbox stock (sin cambios en `usb-s5l8702.c`/strings) → `runningFirmware == .rockboxFamily` | E.2 | revisión de diff (F0 no toca USB) |
| C20 | El binario `rockbox.ipod` arranca en el 6G, `AuraDeviceProbe` clasifica `.aura(hasBooted:true)` con Studio real | E.2 | **hardware** (F13, dueño) |
| C21 | Advertencia documentada: Aura Studio ofrecerá "actualizar" (a Aura) mientras no reconozca `firmware_family` | E.3/M-004 | `ESTADO_FINAL.md` |

---

## §5 — Orden de ejecución (fases de commits pequeños y compilables)

Reglas de todas las fases: compila sim **y** target al cierre; criterio de hecho verificado en el simulador con captura versionada en `docs/screenshots/F<n>-*.png` (vía `sim_shot.sh`); commit descriptivo en inglés (`F<n>: …`); parada intermedia con resumen + capturas + espera de aprobación; toda desviación factual a `docs/DESVIACIONES.md`; `MODIFICATIONS.md` y `docs/COMPAT_STUDIO.md` al día. Orden: navegación → estética → animaciones.

### F0 — Fase Cero de separación
- **Hace**: §0 completo (siembra, 9 archivos portados, tooling, toolchain, docs de repo).
- **Archivos**: los de §0.1–§0.3.
- **Hecho**: §0.4 (captura `F0-rockbox-stock.png` del menú stock; `rockbox.ipod` y bootloader compilados; `diff -rq` = 9 archivos).
- **Commits**: `F0: import Rockbox 0726ec93 (unmodified upstream)` · `F0: port ipod6g hardware/build/compat fixes from Aura-Firmware` · `F0: add build, simulator and test-media tooling` · `F0: repo docs (README, LICENSE, MODIFICATIONS, DECISIONS, CLAUDE)`.

### F1 — Gancho de arranque y esqueleto de Metro
- **Hace**: `apps/metro/metro_main.c` mínimo (higiene M-019, bucle que dibuja "metro" con `FONT_SYSFIXED` centrado y llama `default_event_handler`); `apps/main.c` llama `metro_main()`; bloque en `apps/SOURCES`; logo de arranque wordmark (`gen_logo.py`); `metro_screen_splash` mínima.
- **Archivos**: `apps/main.c` (mod), `apps/SOURCES` (mod), `apps/metro/{metro_main.c,.h,metro_screen_splash.c,.h}`, `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` (reemplazo), `firmware/tools/gen_logo.py`, `firmware/assets/fonts-src/Selawik-*.ttf` (OFL, con `LICENSE.txt`).
- **Hecho**: `F1-boot.png` muestra la pantalla Metro (nunca el menú de Rockbox); USB en sim (`SYS_USB_CONNECTED` no inyectable en sim: verificar solo que `default_event_handler` se llama — trace); apagado por `SYS_POWEROFF` limpio en sim (cerrar ventana → sin panic).
- **Commit**: `F1: boot into metro_main() instead of root_menu(); boot wordmark`.

### F2 — Fuentes, paleta y shell de dibujo
- **Hace**: `gen_fonts.sh` (convttf → `firmware/assets/fonts/metro-*.fnt`, 5 fuentes, rango 0x20–0x17F, `-x`), `build_sim.sh` copia las fuentes a `simdisk/.rockbox/fonts/`; `metro_fonts` (`font_load_ex(path,0,400)`, fallback), `metro_palette.h`, `metro_theme`, `metro_draw` (texto con recorte, cabecera con hora/batería, `draw_rows`, `draw_pivots`); pantalla de "especimen" temporal (escala tipográfica + paleta) accesible al arrancar.
- **Archivos**: `firmware/tools/gen_fonts.sh`, `firmware/assets/fonts/*.fnt`, `apps/metro/{metro_fonts,metro_palette.h,metro_theme,metro_draw}`, `firmware/tools/build_sim.sh` (mod).
- **Hecho**: `F2-type-specimen.png`: 5 roles con Selawik cargada (no sysfont), un titular en `display` recortado en x=320, tira de 10 acentos, hora/batería en cabecera. Log del sim: 5 `font_load_ex` OK.
- **Commit**: `F2: Selawik font pipeline, palette, drawing shell`.

### F3 — Núcleo de navegación twist (sin biblioteca real)
- **Hace**: `metro_nav` + `test/test_nav.c` (push/pop/pivot/sel, ventaneo), `metro_page.h`, `metro_keymap` (tablas §2.3, contextos `HUB/LIST/DIALOG`), `metro_input` (`get_custom_action`, aceleración), `metro_lang` (ES/EN base), `metro_screen_hub`, `metro_screen_list` genérica, `metro_screen_settings` esqueleto con proveedores estáticos (valores en RAM, sin persistir), diálogo sí/no; el hub muestra `music/videos/photos/settings` (music/videos/photos con pivots y listas de **datos ficticios** de 30 filas para probar ventaneo).
- **Archivos**: `apps/metro/{metro_nav,metro_page.h,metro_keymap,metro_input,metro_lang,metro_screen_hub,metro_screen_list,metro_screen_settings,metro_widgets(dialog),test/Makefile,test/test_nav.c}`.
- **Hecho**: `make -C apps/metro/test test` verde; capturas `F3-hub.png`, `F3-music-artists.png` (pivot activo blanco, siguientes en gris recortados), `F3-music-albums.png` tras `RIGHT` (twist), `F3-list-scrolled.png` tras 6×`SCROLL_FWD` (ventaneo), `F3-settings.png`, `F3-back.png` tras `MENU` (vuelve al hub con la selección restaurada). Todo con `sim_shot.sh … "SELECT,RIGHT,SCROLL_FWD,…"`.
- **Commit**: `F3: twist navigation core (nav stack, pivots, keymap, generic list screen)`.

### F4 — Biblioteca de música real (tagcache) y reproducción
- **Hace**: `metro_music` (proveedores sobre `tagcache_search*`, orden alfabético natural, `tag_albumartist` (existe en `apps/tagcache.h:37`) como agrupador de álbumes cuando la pista lo trae, `tag_artist` si no; las fotos de artista (backlog) comparan contra `tag_artist` (E.5)), reproducción (`playlist_create`/`insert`/`start`, `shuffle_all`), playlists `.m3u8`, `metro_music_db_ready()` con estado "updating library…" placeholder mientras `!tagcache_is_usable()`; el hub muestra `now playing` cuando hay audio; SELECT en canción → reproduce y empuja una NP placeholder (texto plano).
- **Archivos**: `apps/metro/{metro_music.c/.h,metro_playlists (dentro de metro_music o aparte)}`, `metro_screen_hub` (mod), `metro_screen_list` (mod: `on_play`), `firmware/tools/gen_test_media.sh` (mod: 3 layouts, 12 pistas, 3 álbumes, 2 artistas, géneros).
- **Hecho**: con `gen_test_media.sh` + base construida en el sim: capturas `F4-artists.png` (datos reales), `F4-artist-albums.png`, `F4-album-songs.png` (subtítulos), `F4-genres.png`, `F4-playlists.png`; log del sim muestra `audio_play` tras SELECT en una canción y `now playing` aparece en el hub (`F4-hub-playing.png`). C8, C11 verificados.
- **Commit**: `F4: music library providers over tagcache; playback start; playlists`.

### F5 — Now Playing
- **Hace**: `metro_screen_nowplaying` (layout §1.4), `metro_albumart` (carátula 136×136 con caché de 1), controles (§2.3 contexto PLAYER: volumen con overlay, prev/next, seek, play/pausa global), página `options` (up next = cola vía `playlist_get_track_info`, shuffle, repeat), íconos compilados (batería, play/pausa, shuffle, repeat), tile acento con inicial cuando no hay carátula, FADE inexistente aún (cambio instantáneo).
- **Archivos**: `apps/metro/{metro_screen_nowplaying,metro_albumart,metro_widgets(volume overlay)}`, `metro_keymap` (mod: `PLAYER`, `OPTIONS`), `apps/bitmaps/native/{metro_*.bmp,SOURCES}`.
- **Hecho**: `F5-nowplaying-art.png` (carátula real de test-media), `F5-nowplaying-noart.png` (tile), `F5-volume-overlay.png` (tras `SCROLL_FWD`), `F5-options.png`, `F5-upnext.png`; `LEFT/RIGHT` cambian de pista (log). C9 verificado.
- **Commit**: `F5: now playing screen, album art, transport controls, options page`.

### F6 — Compatibilidad Aura Studio (núcleo)
- **Hace**: `metro_settings` (`aura.cfg` con claves M-017, creado al primer arranque), `metro_sync` + `metro_sync_marker` (+ test) con pantalla "updating library…" (única pantalla completa de espera; posponible con MENU), `metro_device`, `metro_disk_handoff()` al arrancar y tras USB, `rtc_sync_*`, `docs/COMPAT_STUDIO.md` (checklist §4 con estado por ítem).
- **Archivos**: `apps/metro/{metro_settings,metro_sync,metro_sync_marker,metro_device,test/test_sync_marker.c,test/test_device.c}`, `metro_main` (mod), `metro_music` (mod: cede mientras `metro_sync_job_active()`).
- **Hecho**: C1–C7 verificados en sim (capturas `F6-updating-library.png`, `F6-aura-cfg.txt` copia del archivo generado); tests verdes.
- **Commit**: `F6: aura.cfg settings store, sync marker processing, disk handoff, device.cfg`.

### F7 — Videos y fotos
- **Hace**: `metro_video`, `metro_photos`, `metro_media_categories`, `metro_fsutil`; páginas con pivots de categoría condicionales; lanzamiento de `mpegplayer.rock`/`imageviewer.rock` con errores silenciados; al volver del plugin, redibujo completo (invalidar caches).
- **Archivos**: `apps/metro/{metro_video,metro_photos,metro_media_categories,metro_fsutil}`, `metro_screen_hub` (mod), `gen_test_media.sh` (mod: `.mpg`, JPEGs, cfgs).
- **Hecho**: `F7-videos-all.png`, `F7-videos-movies.png` (con cfg), `F7-videos-nocfg.png` (solo `all`), `F7-photos.png`; en sim, SELECT sobre un video abre mpegplayer (captura `F7-mpegplayer.png` a 200 ticks); MENU dentro del plugin vuelve a la lista en la misma posición. C12–C14 verificados.
- **Commit**: `F7: videos and photos browsers with category pivots; plugin launch`.

### F8 — Ajustes reales y About
- **Hace**: páginas de Ajustes conectadas a `global_settings` (brillo, retroiluminación, shuffle/repeat globales) y a `metro_settings` (tema, acento, idioma, animations/graphics — aún sin efecto visual de animación, solo persistencia), "library › update now/rebuild" (vía `metro_sync`, con diálogo), "reset settings", `metro_screen_about` (`device.cfg`, `sync_summary.cfg`, versión, "based on Rockbox"), tema claro completo, ajuste de idioma con recarga inmediata.
- **Archivos**: `metro_screen_settings` (real), `metro_screen_about`, `metro_theme` (mod), `metro_lang` (completa).
- **Hecho**: `F8-settings-general.png`, `F8-settings-display.png`, `F8-accent-teal.png` (acento cambiado), `F8-light-hub.png` + `F8-light-nowplaying.png` (tema claro), `F8-english-hub.png`, `F8-about.png`; los valores persisten tras reiniciar el sim (`aura.cfg`). C15, C16 verificados.
- **Commit**: `F8: settings pages wired to storage; about screen; light theme; language`.

### F9 — Superficies de Rockbox y arranque/apagado/USB
- **Hace**: gancho en `apps/gui/splash.c` (`metro_splash_restyle`: tabla ES/EN de mensajes de tagcache/playlist/batería, colores Metro), `metro_screen_usb` + `usb_screen.c` (fix de centrado + colores) + `usblogo` wordmark, pantalla de apagado, splash de arranque final con barra, `show_shutdown_message=false` ya en M-019; recorrer `Aura-Firmware/docs/superficies-rockbox.md` niveles 1–2 y anotar estado en `docs/SUPERFICIES.md`.
- **Archivos**: `apps/gui/splash.c` (mod), `apps/gui/usb_screen.c` (mod), `apps/bitmaps/native/usblogo.176x48x16.bmp`, `apps/metro/{metro_screen_usb,metro_screen_splash (final),metro_splash_lang.c/.h}`, `docs/SUPERFICIES.md`.
- **Hecho**: `F9-splash-committing.png` (forzar un splash de tagcache borrando `.tcd` en simdisk y capturando a pocos ticks — si no se atrapa, documentar como en Aura), `F9-usb.png` (dibujando la pantalla directamente por un modo de prueba `METRO_SIM_FORCE_USB=1` en sim_tasks — añadir a la lista de tooling), `F9-boot.png`, `F9-shutdown.png`.
- **Commit**: `F9: restyle Rockbox surfaces (splash, USB, boot, shutdown)`.

### F10 — Pulido estético (Metro completo, sin animación)
- **Hace**: letra índice flotante en scroll rápido, estados vacíos con tile, subtítulos y recorte en todas las listas, encabezado de pivots con offset exacto, iconos de estado en NP, ajustes de pitch/márgenes tras revisar capturas lado a lado con referencias Zune 30/HD, `sim_matrix.sh` completa (cada pantalla × 2 temas × 3 acentos), README con capturas.
- **Archivos**: `metro_draw`, `metro_widgets`, `metro_screen_*` (ajustes finos), `firmware/tools/sim_matrix.sh`.
- **Hecho**: `docs/screenshots/F10-matrix/*.png` completa; revisión visual del dueño (parada intermedia con la matriz).
- **Commit**: `F10: visual polish pass (index letter, empty states, spacing)`.

### F11 — Transiciones nivel 1 (SLIDE, PUSH/POP, FADE) + niveles + instrumentación
- **Hace**: `metro_fb`, `metro_motion` (+ test), `metro_transitions` con SLIDE (twist), PUSH/POP por slide, FADE (NP/plugins), matriz `animations × graphics` operativa desde Ajustes, `METRO_TRACE`, auto-degradación, `cpu_boost` por transición, guards.
- **Archivos**: `apps/metro/{metro_fb,metro_motion,metro_transitions,test/test_motion.c}`, `metro_screen_list/hub/nowplaying` (mod: llaman a transiciones), `metro_screen_settings` (mod).
- **Hecho**: capturas a mitad de transición con `sim_shot.sh` (tick preciso tras `RIGHT`) `F11-slide-mid.png`, `F11-push-mid.png`, `F11-fade-mid.png`; con `animations=off` la captura al mismo tick ya muestra el destino; log del sim con `METRO_TRACE` por transición; tests verdes; **compila para ipod6g**. Nota: los tiempos del sim no son representativos (B.11) — la medición real es F13.
- **Commit**: `F11: transition engine (slide, push/pop, fade), FX levels, tracing`.

### F12 — Transiciones nivel 2 (TURNSTILE, FEATHER, fondo de carátula)
- **Hace**: `present_turnstile` con tabla precalculada, PUSH/POP turnstile bajo `all`+`full`, FEATHER en la primera pintura de listas, fondo de carátula atenuada en NP (`graphics=full`, 3.er buffer estático), pruebas de degradación.
- **Archivos**: `metro_fb` (mod), `metro_transitions` (mod), `metro_screen_nowplaying` (mod), `metro_albumart` (mod: escalado 320×240 bajo demanda).
- **Hecho**: `F12-turnstile-mid.png`, `F12-feather-mid.png`, `F12-np-artbg.png`; con `graphics=lite` la NP no tiene fondo (`F12-np-lite.png`); compila para ipod6g.
- **Commit**: `F12: turnstile and feathered transitions; dimmed album-art background`.

### F13 — Release, flasheo y medición en hardware
- **Hace**: `package_dist.sh` (portado: `rockbox.ipod`, `rockbox.zip` con `.rockbox/fonts/metro-*.fnt`, bootloader, `mks5lboot`, `checksums.txt`, `MODIFICATIONS.md`), `firmware/dist/README.md`, `docs/GUIA_FLASHEO.md` (portada de `guia-flasheo-restauracion.md`, adaptada), `docs/ESTADO_FINAL.md` (qué funciona, pendiente, desviaciones, cómo flashear, advertencia M-004, procedimiento de medición de FX en hardware con `METRO_TRACE`/build DEBUG y cómo bajar de nivel), tag `v0.1.0`.
- **Hecho**: artefactos generados y verificados por checksum; el dueño flashea y reporta: arranque, navegación, reproducción, sync con Aura Studio (C20), fluidez de transiciones → ajuste de `frames/frame_delay` si hace falta (commit de ajuste).
- **Commit**: `F13: release packaging, flashing guide, final status`.

### Backlog (fuera de v1, en orden sugerido)
1. Modo "cubrir pantalla" y OSD Metro en mpegplayer (port de D-304/305/308 mecanismo).
2. Visor de fotos propio (port infra de `aura_photos.c`) con ajustar/cubrir.
3. Letras `.lrc` en NP (port `aura_lrc.c` + test).
4. Fotos de artista como tiles cuadrados (`artist_images.cfg`).
5. Import de `ratings.cfg` y calificación en `options`.
6. Quickplay (tiles de álbumes recientes vía runtime DB de tagcache).
7. CONTINUUM.
8. Temporizador de sueño, EQ presets, bloqueo por PIN (D-238).
9. Seguimiento en `Aura-Studio/`: reconocer `firmware_family: metro` (M-004).

---

## §6 — Reglas transversales para la Fase 4

1. **Nunca dejar el build roto**: cada fase cierra con `build_sim.sh` y `build_target.sh` en verde. Si el target no compila por una API host-only, se corrige antes de avanzar.
2. **Un `.c/.h` nuevo = cabecera GPL v2 + prefijo `metro_`**; un archivo de Rockbox tocado = comentario inline `Metro (M-NNN)` + fila en `MODIFICATIONS.md` en el mismo commit.
3. **Cero RGB fuera de `metro_palette.h`; cero `snprintf` de rutas del contrato fuera de `metro_settings.c`/`metro_sync.c`/`metro_device.c`/`metro_media_categories.c`** (un solo sitio por ruta).
4. **Prohibido desde `apps/metro/`**: `root_menu`, `do_menu`, `gui_synclist`, `rockbox_browse`, `kbd_input`, `gui_syncyesno`, `tree.c`, `tagtree.c`, skin engine, `splash()` para mensajes propios (usar `metro_widgets`).
5. **Animación**: solo en F11+; toda función de transición empieza con `if (!lcd_active() || level == OFF)`; nunca I/O de disco dentro de un bucle de cuadros; `sleep()` entre cuadros; `drain_button_queue_if_full()`; `cpu_boost` balanceado.
6. **Simulador primero**: criterio de hecho = capturas en `docs/screenshots/`; hardware solo en F13 (y cuando el dueño lo pida).
7. **Desviaciones**: si el plan cita un archivo/API/valor que no existe o difiere, se corrige y se anota en `docs/DESVIACIONES.md` (`F<n> · qué decía el plan · qué se encontró · qué se hizo`). Las decisiones M-NNN no se cambian por preferencia; una desviación de diseño necesaria se propone en la parada intermedia.
8. **Paradas**: al cerrar cada F<n>: resumen, capturas, estado de `COMPAT_STUDIO.md`, y espera de aprobación antes de F<n+1>.
9. **Modelo sugerido por fase** (orientativo): F0–F10 Sonnet; F11–F12 Sonnet, subir a Opus si el turnstile o el rendimiento del sim se atoran; F13 Sonnet.
