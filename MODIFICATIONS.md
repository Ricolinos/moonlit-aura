# MODIFICATIONS.md — Aviso de modificaciones (GPL v2 §2a)

Este archivo cumple el requisito de la GPL v2 §2(a) de hacer constar,
de forma destacada, que este software fue modificado y la fecha de la
modificación.

## Origen

Este repositorio es un fork de [Rockbox](https://www.rockbox.org/)
(espejo [github.com/Rockbox/rockbox](https://github.com/Rockbox/rockbox)),
software libre bajo GPL v2. Rockbox se importó a `firmware/rockbox/`
en el commit base **`0726ec93517a61f602679ab052b083217ec9c96d`**
(2026-08-09), clonado sin el `.git` interno de Rockbox (se versiona
dentro del `.git` de este repositorio; ver `DECISIONS.md`, M-001).

Es el **mismo commit base** que usa el proyecto hermano
[Aura-Firmware](https://github.com/Ricolinos/Aura-Firmware) — ver
`DECISIONS.md` M-001 para la justificación.

## Los 10 archivos de Rockbox modificados fuera de `apps/metro/`

Todos son **fixes de hardware/build/compatibilidad heredados de
Aura-Firmware**, portados byte-idénticos (ver `DECISIONS.md` M-005,
`docs/DESVIACIONES.md` F0-2 sobre por qué no se reescribió su
atribución inline). Cada uno conserva su propio comentario de
modificación GPL original, autoría de Aura-Firmware, con referencia a
sus decisiones `D-NNN` (bitácora en el `DECISIONS.md`/`DECISIONS-ARCHIVE.md`
de ese repositorio, no en este). Este archivo documenta que además
fueron **portados a Metro-Aura** el 2026-08-20, sin más cambios que la
copia:

- `apps/plugin.c` (D-298: `plugin_set_silent_open_errors()`)
- `apps/plugin.h` (D-298: declaración)
- `apps/tagcache.c` (D-021/D-244/D-293: contador de trabajos de
  reconstrucción, descarte de temporal huérfano, fix de fuga de
  buffer en simulador, fix de `load_ramcache()` con base creciente,
  `commit()` con buffer temporal general)
- `apps/tagcache.h` (D-293: `tagcache_get_build_jobs_done()`,
  `tagcache_has_pending_temp()`, `tagcache_discard_pending_temp()`)
- `bootloader/ipod-s5l87xx.c` (D-064: arranque silencioso bajo
  `#ifdef IPOD_6G`, sin afectar `IPOD_NANO3G`)
- `firmware/export/config/ipod6g.h` (D-06x: `CONFIG_BACKLIGHT_FADING
  BACKLIGHT_FADING_SW_HW_REG`; D-061: fix de `USBPOWER_BTN_IGNORE`
  fuera de `#ifdef BOOTLOADER`)
- `lib/rbcodec/codecs/aiff.c` (tolerancia a variaciones de AIFF
  exportado por Music/iTunes)
- `tools/configure` (D-007: detección del GCC de Homebrew más
  reciente disponible en macOS, con fallback 16→15→14→13 — **no
  estaba documentado en la lista de 27 de Aura-Firmware**, hallado y
  añadido en la Fase Cero de este repositorio; ver
  `docs/DESVIACIONES.md` F0-1)
- `uisimulator/common/sim_tasks.c` (automatización de entrada +
  captura de pantalla headless para el simulador SDL; variables de
  entorno renombradas `METRO_SIM_*` en vez de `AURA_SIM_*`, ver más
  abajo)
- `utils/mks5lboot/Makefile` (backend libusb opcional en macOS)

(Rutas relativas a `firmware/rockbox/`.)

### Excepción: `uisimulator/common/sim_tasks.c` sí se modificó al portar

A diferencia de los otros 9, este archivo **no** se portó byte-idéntico:
las variables de entorno y funciones que Aura-Firmware nombró con el
prefijo `AURA_SIM_*` se renombraron a `METRO_SIM_*`
(`METRO_SIM_AUTODUMP_TICKS`, `METRO_SIM_AUTODUMP_QUIT`,
`METRO_SIM_BUTTONS`), porque son identificadores de proyecto, no un
aviso de modificación GPL. El mecanismo (inyección de botones +
autodump headless) es idéntico.

**F9 (2026-08-20, M-039):** agrega el token `USB_INSERT` a
`METRO_SIM_BUTTONS` — llama `sim_trigger_usb(true)` (la misma función
que ya dispara el menú interactivo del simulador) en vez de postear un
botón, para poder capturar `metro_screen_usb.c` de forma headless. No
existía en el mecanismo original de Aura-Firmware.

## `apps/metro/` — código nuevo, no una modificación

Todo el árbol `firmware/rockbox/apps/metro/` es código **nuevo**,
escrito para este proyecto — no es una modificación de un archivo
preexistente de Rockbox. Cada archivo lleva su propia cabecera de
copyright GPL v2. Es la UI "Metro" (menús, navegación twist,
reproducción) que reemplaza `root_menu()`/la UI de menús estándar de
Rockbox — ver `DECISIONS.md` M-006.

## Otros archivos de Rockbox modificados

Esta sección se actualiza en cada fase de `PLAN_MAESTRO.md` §5 que
toque un archivo de Rockbox fuera de `apps/metro/` — ver el registro
vivo en `DECISIONS.md`.

### F1 (2026-08-20)

- `apps/main.c`: `root_menu()` → `metro_main()` como único punto de
  entrada de la UI (M-006, no retorna). `metro_apply_hygiene()`
  (M-019) se llama en los dos cuerpos de `init()` (simulador y target
  real), justo después de `settings_load()` y antes de
  `settings_apply(true)`/`settings_apply_skins()` — es el único punto
  donde puede correr sin que el backdrop Cabbie v2 stock ya se haya
  pintado sobre el LCD (ver `docs/DESVIACIONES.md` F1-1).
- `apps/SOURCES`: bloque agregado al final listando
  `metro/metro_main.c` y `metro/metro_screen_splash.c`.
- `apps/bitmaps/native/rockboxlogo.320x98x16.bmp`: reemplazado por el
  wordmark "metro" (Selawik Light, blanco sobre negro), mismo nombre
  de archivo y dimensiones exactas (320×98) que el original — la regla
  de `apps/bitmaps/bitmaps.make` que lo compila a `bm_rockboxlogo` vía
  `bmp2rb` no necesitó ningún cambio. Generado por
  `firmware/tools/gen_logo.py` desde
  `firmware/assets/fonts-src/Selawik-Light.ttf` (M-020).

### F9 (2026-08-20)

- `apps/gui/splash.c` (M-037): un solo gancho de una línea —
  `metro_splash_translate(splash_buf, sizeof(splash_buf))`, corre
  justo después de que `vsnprintf()` resuelve el mensaje (con
  cualquier argumento dinámico ya sustituido) y antes del ajuste de
  línea, que no se toca. Reescribe al wording de Metro (ES/EN) los
  mensajes conocidos que vienen del árbol de Rockbox que Metro no
  controla (tagcache, carga de playlist/plugin, apagado por batería
  baja) — mismo mecanismo que `aura_splash_translate()` de
  Aura-Firmware (D-055 en ese repo), sin copiar su código: la tabla de
  mensajes vive en `apps/metro/metro_splash_lang.c`, código nuevo de
  Metro. Un mensaje sin regla conocida se muestra tal cual, nunca se
  reemplaza por un genérico que esconda información de diagnóstico.

### R2-F4 (2026-08-19, M-059)

Puerto + restilado del plugin `mpegplayer` (video) al estilo Metro,
siguiendo el mismo mecanismo que Aura-Firmware ya probó primero
(D-304..D-309 de ese repositorio, consultado en solo lectura como guía
de mecanismo — nunca copiado archivo por archivo; el diseño propio de
Aura, como su barra "píldora" redondeada, no se portó, ver
`docs/DESVIACIONES.md` R2-3). Los 7 archivos, cada uno con su propio
comentario inline `Metro (M-059)` en el punto exacto del cambio:

- `apps/plugins/mpegplayer/mpeg_settings.h`: `SETTINGS_VERSION` 5→6;
  `enum mpeg_scale_mode_id` (ajustar/cubrir) y el campo
  `settings.scale_mode` nuevos; `MPEG_SETTING_ENABLE_START_MENU`/
  `MPEG_MENU_RESUME` eliminados (el menú de inicio interactivo ya no
  existe, ver `mpeg_settings.c`).
- `apps/plugins/mpegplayer/mpeg_settings.c`: reescritura completa del
  menú de ajustes — el bloque de `#define MPEG_START_TIME_*` por
  target (~400 líneas) y `get_start_time()`/`show_start_menu()`/
  `draw_slider()`/`display_thumb_image()`/`show_loading()`/
  `increment_time()`/`resume_options()` se eliminan (código muerto una
  vez removido el menú de inicio); `rb->do_menu()`/`rb->set_option()`/
  `rb->set_int_ex()` (widgets 100% nativos de Rockbox) reemplazados por
  `metro_menu_draw()`/`metro_menu_pick()`/`metro_menu_adjust_int()`
  (widget propio, geometría de `metro_draw_rows()`: pitch 28px, x=12,
  seleccionado en fg, resto en secundario, sin píldora); tabla
  bilingüe ES/EN propia del plugin (`metro_str()`, no puede incluir
  `metro_lang.c`); `mpeg_start_menu()` ahora resuelve directo
  (`MPEG_START_SEEK`), sin mostrar nunca el menú "Play from
  beginning/Resume/Set time/Settings/Quit".
- `apps/plugins/mpegplayer/mpegplayer.c`: `MPEG_TOGGLE_SCALE` (SELECT)
  nuevo en el keypad del iPod; `struct osd` gana
  `prog_trackcolor`/`accent`; `metro_load_personalization()` (nuevo)
  lee `/.rockbox/aura/aura.cfg` una vez en `osd_init()` (esquema de
  Metro: `theme`/`accent`/`language`, colores solo vía
  `metro_palette.h`); `draw_scrollbar_draw()` reescrita a barra plana
  de dos colores (pista terciaria + relleno acento), sin borde;
  `osd_refresh_status()` recolorea el ícono de estado al acento; 4
  strings de splash traducidos vía `metro_str()`.
- `apps/plugins/mpegplayer/stream_mgr.c`: 8 strings de splash de error
  (fallos de inicialización de hilos/memoria) traducidos vía
  `metro_str()` en vez de texto en inglés fijo.
- `apps/plugins/mpegplayer/video_out.h`: 2 declaraciones nuevas,
  `vo_update_scale_mode()`/`vo_toggle_scale_mode()`.
- `apps/plugins/mpegplayer/video_out_rockbox.c`: modo "cubrir" —
  `vo_draw_frame_cover()` (recorta+escala por muestreo nearest-neighbor
  sobre la memoria sobrante del arena de libmpeg2, reutilizando
  `stretch_image_plane()` ya existente) y `vo_recalc_rect()`; guarda
  `scale_mode_locked` en `vo_setup()` contra codificadores MPEG-2 de
  GOP corto que repiten la cabecera de secuencia durante la
  reproducción normal (bug real encontrado y documentado primero por
  Aura-Firmware, D-308).
- `apps/plugins/mpegplayer/mpegplayer.make`: agrega
  `-I$(APPSDIR)/metro` a `MPEGCFLAGS` para incluir `metro_palette.h`
  directo (header puro, sin `.c`, nada que enlazar).

Ver `DECISIONS.md` M-059 para el detalle completo de cada decisión de
diseño y el bug de memoria encontrado y corregido durante la
verificación (no presente en el port mecánico inicial de Aura).
