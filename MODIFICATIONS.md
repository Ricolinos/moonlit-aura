# MODIFICATIONS.md — Aviso de modificaciones (GPL v2 §2a)

Este archivo cumple el requisito de la GPL v2 §2(a) de hacer constar,
de forma destacada, que este software fue modificado y la fecha de la
modificación.

## Origen

moonlit.aura es a su vez un fork de Metro-Aura a partir del tag
`moonlit-fork-base` (commit `2f1bd28a693b52a3554ecd8b4524ae1afa8e975d`,
2026-08-25). Todo lo que sigue en este archivo aplica tal cual a
moonlit.aura; las modificaciones nuevas fuera de `apps/metro/` se
anotan más abajo marcadas `moonlit (D-NNN)`.

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

**R3-F8 (2026-08-20, M-069):** el sondeo del hilo del simulador pasa de
`HZ/10` a `HZ/50` mientras hay un volcado o botones pendientes. Con 10
ticks de resolución, una animación de 24 ticks (los 8 cuadros × 3 del
PUSH bajo `animations=all`) solo se puede muestrear dos veces y la
primera cae ya pasado el tercer cuadro — imposible capturar el arranque
de CONTINUUM, que es justo el criterio de "hecho" de esa fase. A 2
ticks hay ~12 muestras dentro de la misma animación. Mismo carácter que
la ampliación de `METRO_MAX_INJECT_BUTTONS` que este archivo ya traía:
herramienta de pruebas, solo compila en el simulador, sin ningún
impacto en el binario de hardware.

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

- `apps/gui/usb_screen.c` (M-088): tres bloques `#ifdef IPOD_6G`, marcados
  `Metro (M-088)` — `#include "metro/metro_screen_usb.h"`; en
  `usb_screens_draw()` la pantalla principal llama
  `metro_screen_usb_show()` en vez de pintar `bm_usblogo` (el logo
  genérico "USB"); y en el bucle de `handle_usb_events()` el sondeo pasa
  de `HZ/2` a `HZ/10` y llama `metro_screen_usb_tick()` en cada vuelta,
  para animar los puntos del indicador indeterminado. Sin cambio en la
  lógica USB/HID ni en el resto de targets. Todo lo que Metro dibuja ahí
  está embebido en el binario (`font_disable_all()` sigue corriendo
  antes, sin tocarse).
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

### R2-F4, continuación (2026-08-19, M-060)

Rediseño real "Zune HD" del OSD de video, pedido explícitamente por el
dueño tras verificar M-059 en el simulador interactivo (el port
mecánico ya quitaba el menú nativo de Rockbox, pero el OSD en sí no se
parecía al reproductor del Zune). Todo el cambio vive en
`apps/plugins/mpegplayer/mpegplayer.c`, comentario inline `Metro
(M-060)`/`R2-F4 Zune redesign (M-060)` en cada punto:

- `struct osd` pierde el campo `icons` (ya no hay ícono bitmap); las 3
  externs `mpegplayer_status_icons_8/12/16x8x1` se quitan (los `.bmp`
  fuente en `apps/plugins/bitmaps/mono/` se dejan intactos, fuera de
  alcance).
- `osd_text_init()`: reescritura completa, de layout de dos filas
  (ícono+tiempos arriba, barra abajo) a una sola fila (ícono, tiempo
  transcurrido, barra, duración), usando `vo_rect_set_ext()` en vez del
  truco original de ancho-como-`.r`-luego-offset.
- `osd_refresh_background()`: el bisel elevado de 4 líneas de
  brillo/sombra se quita, un solo relleno plano.
- `draw_status_icon()`/`draw_tri_stepped()` (nuevas): ícono geométrico
  por `draw_fillrect()`, reemplaza el bitmap+sombra de
  `osd_refresh_status()`.
- `draw_scrollbar_draw()`: línea de 2px con "thumb" cuadrado de 4px en
  el borde de lo reproducido, en vez del bloque de altura completa
  sin punta.
- `osd_init()`: `osd.prog_trackcolor` pasa de `s_metro_tertiary` sólido
  a `draw_blendcolor(osd.bgcolor, MYLCD_WHITE, 71)` (~28% blanco);
  carga `metro-caption-14.fnt` vía `rb->font_load()`
  (`draw_setfont_osd()`, con reserva a `FONT_UI` si falla).
- `draw_oriented_mono_bitmap_part()` (la variante no-portrait) y
  `draw_hline()` se eliminan por quedar sin llamadores tras lo
  anterior.

Ver `DECISIONS.md` M-060 para el detalle de cada uno de los 5 cambios
de la maqueta aprobada por el dueño, incluida la razón por la que el
panel no puede ser realmente transparente sobre el video en vivo
(`docs/DESVIACIONES.md` R2-4).

### R2-F4, cierre (2026-08-19, M-061)

Menús del plugin reconstruidos con la anatomía real de página Metro y
volumen del OSD como barra de nivel, tras verificación del dueño en el
simulador interactivo (que además destapó que M-060 nunca había
llegado al simdisk -- `make` no instala plugins, ver
`docs/DESVIACIONES.md` R2-5). Comentarios inline `Metro (M-060 cont.)`/
`M-061`:

- `apps/plugins/mpegplayer/mpeg_settings.h`: 5 IDs de string nuevos
  (títulos de página en minúsculas: `MSTR_TITLE_VIDEO`/`_SETTINGS`/
  `_DISPLAY`/`_AUDIO`/`_BRIGHTNESS`); declaraciones
  `metro_font_caption()`/`metro_font_list()`/`metro_font_list_sel()`/
  `metro_font_display()`/`metro_font_title()`.
- `apps/plugins/mpegplayer/mpeg_settings.c`: `metro_page_chrome()`
  (nueva -- ceja caption + reloj + batería 18x9 replicando
  `metro_draw_header()`/`metro_draw_battery()` de `apps/metro/`, más el
  título de página en display-48 a (12,28));
  `metro_menu_draw()` ahora dibuja esa anatomía completa con filas
  desde y=84; `display_options()`/`audio_options()`/`mpeg_settings()`
  reescritas al patrón ciclar-en-el-lugar con valores visibles
  (los selectores de dos filas por valor se eliminaron);
  `metro_adjust_draw()` (brillo) con la misma anatomía y el valor en
  title-28/acento; strings de ceja en minúsculas.
- `apps/plugins/mpegplayer/mpegplayer.c`: carga de
  `metro-display-48.fnt`/`metro-title-28.fnt` (deduplicadas por ruta
  contra las de la app, `firmware/font.c`); `osd_refresh_volume()`
  reescrita a barra de nivel de 28px (pista 28% blanco + relleno
  acento, rango real de SOUND_VOLUME normalizado) en vez del texto
  "-NdB"; `osd_text_init()` reserva ancho fijo para esa barra.

Ver `DECISIONS.md` M-061.

### moonlit H1 (2026-08-25, D-025)

- `apps/plugins/mpegplayer/mpegplayer.c`: el `#include "metro_palette.h"`
  de M-059 pasa a `#include "../../metro/metro_palette.h"`. Motivo:
  `tools/make.inc` genera `make.dep` con `-MG -MM` y solo los `CFLAGS`
  globales (sin el `-I$(APPSDIR)/metro` que añade `mpegplayer.make`),
  así que el header no encontrado se registraba como objetivo fantasma
  `$(BUILDDIR)/metro_palette.h` y un `configure` desde cero
  (`build_sim.sh --reconfigure`) abortaba con "No rule to make target".
  En Metro-Aura no se notó porque su `make.dep` es anterior a M-059.
  Comentario inline `moonlit (D-025)`.
- `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` (moonlit H1, D-026):
  regenerado con `firmware/tools/gen_logo.py` como wordmark provisional
  de una sola línea "moonlit.aura" (misma tipografía Selawik Light hasta
  H2, mismas dimensiones 320×98; `bitmaps.make` sin cambios). H5 lo
  sustituye por el logotipo Waning Crescent.

### moonlit M2 (2026-08-25, D-032)

- `apps/SOURCES`: la entrada `metro/metro_fonts.c` pasa a
  `metro/moonlit_fonts.c` (7 roles MD3, Libre Baskerville + Montserrat,
  reemplaza los 5 roles Selawik). Comentario inline `moonlit (D-032)`.
- `apps/plugins/mpegplayer/mpegplayer.c:1400,1413-1420` (M-060, R2-F4):
  los 5 `#define METRO_*_FONT_PATH` apuntaban a los `.fnt` de Selawik
  que este hito elimina (regresión hallada en revisión adversarial, no
  prevista por el plan). Pasan a los equivalentes por rol en
  `moonlit-{body-18,list-20,listsel-20,display-40,title-28}.fnt`
  (`caption`→`body` porque `MFONT_CAPTION` es ahora `#define MFONT_BODY`
  en `apps/metro/moonlit_fonts.h`; `display` 48px→40px, tamaño MD3
  real). Sin `rb->font_load()` devolviendo un id válido, el OSD y
  `mpeg_settings.c` caían silenciosamente a `FONT_UI` (`mpegplayer.c:1428-1453`
  ya tenían ese *fallback*, así que nunca crasheaba -- solo se veía
  distinto). Comentario inline `moonlit (D-032)`. Ver `DECISIONS.md`
  D-032 punto 3.

### moonlit M3 (2026-08-25, D-033)

- `apps/SOURCES`: la entrada `metro/metro_icons_table.c` pasa a
  `metro/moonlit_icons.c` + `metro/moonlit_icons_table.c` (mascaras de
  cobertura de 8 bits, Material Symbols Rounded, reemplaza las
  mascaras monocromas de 1 bit de Fluent System Icons). Comentario
  inline `moonlit (D-033)`. `metro/metro_glyphs_table.c` no se toca
  (glifo USB de M-089, se conserva hasta M5).

Nota (fuera de `apps/metro/`, sin comentario inline por no ser Rockbox
sino una herramienta propia del repo): `firmware/tools/gen_logo.py`
apuntaba a `firmware/assets/fonts-src/Selawik-Light.ttf`, que este
hito elimina (M-020). Se actualizó su `FONT_PATH` a
`design-system/vendor/libre-baskerville/LibreBaskerville-Regular.ttf`
para que el generador del wordmark provisional (D-026) siga
funcionando -- `gen_logo.py` no se invoca desde ningún script de build
(`build_sim.sh`/`build_target.sh`/`package_dist.sh`), solo a mano, y
de todos modos se elimina por completo en M9 (plan
`05-plan-correctivo.md` §M9) junto con el wordmark provisional.

### moonlit M4 (2026-08-25, D-035)

- `apps/plugins/mpegplayer/mpegplayer.c`: el `#include "../../metro/metro_palette.h"`
  de D-025 pasa a `#include "../../metro/moonlit_tokens.h"` -- M4 retira
  `metro_palette.h` (sustituido por `moonlit_palette.c/.h`, que tiene
  un `.c` y por lo tanto no se puede enlazar desde un plugin, ver
  `DECISIONS.md` D-035). `metro_load_personalization()` reescrita: los
  10 acentos WP7 planos (`metro_accent_colors[10]`) pasan a los 4
  presets MD3 por esquema (`metro_accent_colors_night/dawn[4]`,
  D-028) y las 4 tinturas WP7 (`METRO_DARK_BG` etc.) a los roles de
  superficie `MOONLIT_NIGHT_*`/`MOONLIT_DAWN_*`. Comentario inline
  `moonlit (D-025, D-035)`.
- `apps/plugins/mpegplayer/mpeg_settings.c`: un comentario (línea ~23)
  actualizado para no citar `metro_palette.h` (eliminado) ni "10
  acentos" (ahora 4) -- sin cambio de código.
- `apps/plugins/mpegplayer/mpegplayer.make`: comentario junto a
  `-I$(APPSDIR)/metro` actualizado (`metro_palette.h` → `moonlit_tokens.h`,
  mismo razonamiento de header puro). El flag en sí no cambia.

Ver `DECISIONS.md` D-034…D-038 para el resto de M4 (íntegro dentro de
`apps/metro/`, sin más archivos fuera de ese árbol).

### moonlit M5 (2026-08-25, D-040)

- `apps/SOURCES`: se retira la entrada `metro/metro_glyphs_table.c` —
  M5 elimina ese archivo y `metro/metro_glyphs.h` por completo (el
  glifo USB Fluent de M-089 pasa a `moonlit_icon_draw(MOONLIT_ICON_USB,
  ...)`, D-040). Sin comentario inline por ser una eliminación, no una
  sustitución de ruta.

Ver `DECISIONS.md` D-039…D-040 para el resto de M5 (íntegro dentro de
`apps/metro/`, sin más archivos fuera de ese árbol).

### moonlit M6 (2026-08-25, D-041)

- `apps/SOURCES`: se agregan las entradas `metro/moonlit_flow.c` y
  `metro/moonlit_wheel.c` — motor de proyección y dinámica de rueda de
  Marea, copiados de `aura-upstream/main` (`aura_flow.c`,
  `aura_wheel.c`) con prefijo renombrado y, en `moonlit_flow`, el eje
  de barrido girado de columnas a filas (D-041). Comentario inline
  `moonlit (D-014, D-041, M6)` / `moonlit (D-019, D-041, M6)`.

Ver `DECISIONS.md` D-041 para el resto de M6 (`apps/metro/moonlit_flow.{c,h}`,
`apps/metro/moonlit_wheel.{c,h}`, `apps/metro/metro_input.{c,h}`,
`apps/metro/test/test_flow.c`, `apps/metro/test/test_wheel.c`,
`apps/metro/test/Makefile`, todo dentro de `apps/metro/`).

### moonlit M7 (2026-08-25, D-042)

- `apps/SOURCES`: se agregan las entradas `metro/moonlit_art.c` y
  `metro/moonlit_art_cache.c` — formato de caché `.pfraw` fila-contigua
  de portadas (de `aura_art.c`) y resolución álbum→píxeles + precarga
  D-224 (D-042). Comentarios inline `moonlit (D-020, D-042, M7)` /
  `moonlit (D-042, M7)`.

Ver `DECISIONS.md` D-042 para el resto de M7 (`apps/metro/moonlit_art.{c,h}`,
`apps/metro/moonlit_art_cache.{c,h}`, `apps/metro/metro_albumart.{c,h}`,
`apps/metro/metro_music.c`, `apps/metro/test/test_art.c`,
`apps/metro/test/file.h`, `apps/metro/test/lcd.h`,
`apps/metro/test/Makefile`, todo dentro de `apps/metro/`).

### moonlit M8 (2026-08-25, D-043)

- `apps/SOURCES`: se agrega la entrada `metro/moonlit_screen_marea.c`
  — pantalla Marea (Cover Flow vertical, D-029/D-030), portadas de
  `moonlit_art`/`moonlit_art_cache` (M7) y motor de proyección de
  `moonlit_flow` (M6). Comentario inline
  `moonlit (D-029, D-030, M8): pantalla Marea, portadas izquierda/info derecha`.

Ver `DECISIONS.md` D-043 para el resto de M8 (`apps/metro/moonlit_screen_marea.{c,h}`,
`apps/metro/metro_screen_hub.{c,h}`, `apps/metro/metro_music.{c,h}`,
`apps/metro/metro_lang.{c,h}`, `apps/metro/metro_main.c`, todo dentro
de `apps/metro/`).

### moonlit M9 (2026-08-26, D-044)

- `apps/SOURCES`: se agregan las entradas `metro/moonlit_logo.c` y
  `metro/moonlit_logo_table.c` — logotipo Waning Crescent (creciente +
  wordmark, D-016/D-044). Comentario inline
  `moonlit (D-016, D-044, M9): logotipo Waning Crescent, mascaras 8-bit`.
- `apps/bitmaps/native/rockboxlogo.320x98x16.bmp`: restaurado al
  bitmap oficial de Rockbox (`git show
  5c6da72d:firmware/rockbox/apps/bitmaps/native/rockboxlogo.320x98x16.bmp`,
  el import sin modificar de F0). `firmware/tools/gen_logo.py` (que lo
  sobrescribía con el wordmark de texto provisional de D-026) se
  elimina en este mismo hito. Este bitmap solo lo dibuja
  `apps/main.c:271` (`show_logo_boot()`, código stock de Rockbox) antes
  de que `metro_main()` tenga el control de la pantalla — ajeno al
  sistema de diseño de moonlit, ver D-044 punto 3 en `DECISIONS.md`.

Ver `DECISIONS.md` D-044 para el resto de M9 (`apps/metro/moonlit_logo.{c,h}`,
`apps/metro/metro_screen_splash.c`, `apps/metro/metro_screen_list.c`,
`apps/metro/metro_screen_hub.c`, `apps/metro/metro_draw.{c,h}`,
`apps/metro/metro_screen_usb.c`, `apps/metro/metro_lang.{c,h}`, todo
dentro de `apps/metro/`).

### moonlit release v0.1.0 (2026-08-26, D-047)

- `apps/SOURCES`: se agrega la entrada `metro/metro_firmware_families.c`
  — tabla pura de familias hermanas para el submenú "cambiar sistema"
  (D-047). Comentario inline `moonlit (D-047): sibling family table for
  "cambiar sistema"`.

Ver `DECISIONS.md` D-047 para el resto (`apps/metro/metro_firmware_families.{c,h}`,
`apps/metro/metro_settings.{c,h}`, `apps/metro/metro_screen_settings.c`,
`apps/metro/metro_lang.{c,h}`, `apps/metro/test/`, todo dentro de `apps/metro/`).
