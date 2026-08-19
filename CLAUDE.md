# Metro-Aura

Firmware "Metro" para iPod Classic 6G, fork de Rockbox. Ver `README.md`
para qué es el proyecto y `docs/plans/PLAN_MAESTRO.md` para el plan
completo de ejecución.

## Idiomas

- Documentos `.md`: español (México, neutro, sin voseo).
- Código, comentarios de código nuevo, y mensajes de commit: inglés.
- Cadenas de cara al usuario en la UI: ver `apps/metro/metro_lang.c`
  (bilingüe ES/EN, español por defecto — `DECISIONS.md` M-009).

## Reglas de código (`apps/metro/`)

- Todo `.c`/`.h` nuevo lleva cabecera de copyright GPL v2.
- Cero color RGB hardcodeado fuera de `metro_palette.h`.
- Ninguna ruta del contrato de compatibilidad con Aura Studio
  (`.rockbox/aura/…`, `/.aura/…`) se construye con un `snprintf` propio
  fuera de `metro_settings.c`/`metro_sync.c`/`metro_device.c`/
  `metro_media_categories.c` — un sitio nuevo que necesite una de esas
  rutas pasa por esos módulos, nunca por una ruta armada a mano.
- Prohibido desde `apps/metro/`: `root_menu()`, `do_menu()`,
  `gui_synclist`, `rockbox_browse()`, `kbd_input()`, `gui_syncyesno()`,
  `apps/tree.c`, `apps/tagtree.c`, el skin engine (`.wps`/`.sbs`) para
  cualquier pantalla propia.
- Ninguna lectura de disco (bitmap, ícono, fuente) dentro de un bucle
  de animación por cuadro — decodificar y cachear en RAM antes de
  empezar la animación.
- Toda animación respeta la puerta `lcd_active()` y el nivel de FX
  activo (`aura.cfg` → `animations`/`graphics`, ver `DECISIONS.md`
  M-015).

## Cambios a archivos de Rockbox fuera de `apps/metro/`

Todo cambio a un archivo de Rockbox fuera de `apps/metro/` se registra
en `MODIFICATIONS.md` **en la misma pasada** (GPL v2 §2a) y se marca
inline en el código con un comentario `Metro (M-NNN)`. Excepción: los
10 archivos heredados de Aura-Firmware listados en `MODIFICATIONS.md`
se portan byte-idénticos, sin reescribir su atribución original — ver
`docs/DESVIACIONES.md` F0-2.

## Compatibilidad con Aura Studio (restricción dura)

Antes de tocar cualquier ruta bajo `.rockbox/aura/` o `/.aura/`, lee
`Aura-Firmware/CONTRATO-firmware-studio.md` y
`Aura-Firmware/docs/contracts/library-layout-v1.md` (fuente canónica
del contrato — no se copian a este repo, se leen desde el repo hermano)
y `docs/COMPAT_STUDIO.md` (checklist vivo de este repo). El contrato es
inmutable desde este lado: Metro-Aura consume el formato tal como está
documentado, nunca lo redefine unilateralmente.

## Documentos de trabajo

- `docs/plans/PLAN_INVESTIGACION.md`, `docs/plans/INVESTIGACION.md`,
  `docs/plans/PLAN_MAESTRO.md`: histórico de las Fases 1–3. No
  re-explorar lo que ya está documentado ahí.
- `docs/DESVIACIONES.md`: correcciones factuales al plan encontradas
  durante la ejecución.
- `docs/screenshots/`: capturas de criterio de "hecho" por fase,
  versionadas.
- La fuente de verdad de las decisiones es `DECISIONS.md` de este
  repositorio, no los planes.
