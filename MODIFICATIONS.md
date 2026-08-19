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

## `apps/metro/` — código nuevo, no una modificación

Todo el árbol `firmware/rockbox/apps/metro/` es código **nuevo**,
escrito para este proyecto — no es una modificación de un archivo
preexistente de Rockbox. Cada archivo lleva su propia cabecera de
copyright GPL v2. Es la UI "Metro" (menús, navegación twist,
reproducción) que reemplaza `root_menu()`/la UI de menús estándar de
Rockbox — ver `DECISIONS.md` M-006.

## Otros archivos de Rockbox modificados

Esta sección se actualiza en cada fase de `PLAN_MAESTRO.md` §5 que
toque un archivo de Rockbox fuera de `apps/metro/` (F1: `apps/main.c`,
`apps/SOURCES`, bitmaps de logo; F9: `apps/gui/splash.c`,
`apps/gui/usb_screen.c`, etc.) — ver el registro vivo en
`DECISIONS.md`.

*(Sin entradas todavía — se completa en F1.)*
