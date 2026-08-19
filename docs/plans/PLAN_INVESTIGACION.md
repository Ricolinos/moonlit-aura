# PLAN DE INVESTIGACIÓN — Metro-Aura

**Fase 1 del proyecto Metro-Aura.** Este documento es un plan: no contiene
hallazgos, solo QUÉ leer y QUÉ preguntas responder. Lo ejecuta la Fase 2 y
su único producto es `Metro-Aura/docs/INVESTIGACION.md`.

Fecha: 2026-08-19. Repos hermanos (carpeta padre `Aura/`):
`Aura-Firmware/`, `Aura-Studio/`, `Metro-Aura/` (este). Todos son
repositorios Git independientes; ninguna instrucción de este plan implica
modificar archivos en `Aura-Firmware/` ni en `Aura-Studio/`.

---

## 0. Prerrequisitos, fuentes de lectura y reglas de ejecución

### 0.1 Estado real de los tres repos (verificado el 2026-08-19)

| Repo | Contenido relevante |
|---|---|
| `Metro-Aura/` | **Vacío**: solo `.gitattributes` y el commit `c332fdd Initial commit`. No hay árbol de Rockbox todavía. |
| `Aura-Firmware/` | Rockbox vive en `firmware/rockbox/` (importado in-repo, sin `.git` interno) desde el commit upstream **`0726ec93517a61f602679ab052b083217ec9c96d`** (2026-08-09, espejo `github.com/Rockbox/rockbox`). Sobre ese árbol hay exactamente **27 archivos de Rockbox modificados** (lista canónica en `Aura-Firmware/MODIFICATIONS.md`) y **107 archivos nuevos en `apps/aura/`** (la UI Apple/Aura completa). Builds en `firmware/build-sim`, `firmware/build-ipod6g`, `firmware/build-ipod6g-boot`; toolchain ARM ya compilado en `firmware/toolchain/`; scripts en `firmware/tools/` (`build_sim.sh`, `gen_test_media.sh`, `apple2026_sim_shot.sh`, `apple2026_sim_matrix.sh`, `package_dist.sh`). Documentación: `docs/guia-desarrollo.md`, `docs/guia-flasheo-restauracion.md`, `docs/superficies-rockbox.md`, `docs/aura-design-system/`, `DECISIONS.md` (D-286+) y `DECISIONS-ARCHIVE.md` (D-001…D-285). |
| `Aura-Studio/` | App macOS Swift en `studio/AuraStudio/Sources/AuraStudio/` (`Services/`, `Models/`, `ViewModels/`, `Views/`). Contratos: `CONTRATO-firmware-studio.md` (v7), `CONTRATO-dispositivo.md`, `CONTRATO-formato-tema.md`, `docs/contracts/library-layout-v1.md`, `FIRMWARE_VERSION`, `scripts/fetch-firmware.sh`. Decisiones `ST-NNN` en su `DECISIONS.md`. |

### 0.2 De dónde se lee "Rockbox limpio" durante la investigación

Como `Metro-Aura/` no tiene fuente, la Fase 2 lee Rockbox desde
`Aura-Firmware/firmware/rockbox/`, con esta regla:

- Todo archivo que **no** esté en la lista de 27 de `MODIFICATIONS.md` es
  byte-idéntico al upstream `0726ec9` → se lee directamente como referencia
  de Rockbox.
- Los 27 archivos modificados llevan sus cambios marcados inline con
  comentarios `Aura` / `D-NNN`. Para separar upstream de modificación, la
  Fase 2 debe **obtener una copia de referencia del upstream** en un
  directorio de scratch fuera de los tres repos (p. ej.
  `$CLAUDE_JOB_DIR/tmp/rockbox-upstream/`), preferentemente vía
  `curl -L https://github.com/Rockbox/rockbox/archive/0726ec93517a61f602679ab052b083217ec9c96d.tar.gz`
  (o `git clone --filter=blob:none` + `git checkout 0726ec93`), y hacer
  `diff -u` de esos 27 archivos. Si no hay red, se usan los marcadores
  inline como delimitación y se anota `[ESTIMADO]` en cada clasificación
  afectada.
- Descargar una referencia en scratch **no** viola la regla "sin modificar
  código fuente" de la Fase 2. Nada se escribe dentro de los repos salvo
  `Metro-Aura/docs/INVESTIGACION.md`.

### 0.3 Decisión propuesta para ratificar en la Fase 3 (no ejecutar en Fase 2)

Sembrar `Metro-Aura/` desde **el mismo commit upstream `0726ec9`** que usa
Aura-Firmware. Justificación: los fixes de hardware/build del 6G (sección D)
portan como parches limpios sin conflictos; el toolchain, `tools/configure`
y el simulador ya están probados contra ese commit en este Mac. La Fase 2
solo debe **verificar** (pregunta D.7) si el upstream posterior a
`0726ec9` trae correcciones relevantes para `ipod6g` (LCD, botón, ATA/CE-ATA,
PMU) que hicieran preferible un rebase; si no las hay, la decisión queda
como está.

### 0.4 Reglas de ejecución de la Fase 2

1. **Solo se crea/edita** `Metro-Aura/docs/INVESTIGACION.md`. Cero cambios en
   `Aura-Firmware/`, `Aura-Studio/`, ni en el árbol Rockbox.
2. Lectura masiva con **subagentes** (uno por sección o sub-bloque); el
   contexto principal recibe resúmenes con rutas y números de línea, nunca
   volcados de archivos. `pictureflow.c` (5 014 líneas), `tagcache.c`,
   `list.c`, `skin_render.c` se leen únicamente vía subagente.
3. Cada hallazgo se escribe con este formato fijo:
   - **Pregunta** (copiada del plan, con su identificador, p. ej. `A.3`)
   - **Respuesta concreta** (una afirmación verificable, no un párrafo vago)
   - **Evidencia**: `ruta/archivo.c:línea` (rutas relativas al repo que
     corresponda: `Aura-Firmware/firmware/rockbox/…`, `Aura-Studio/studio/…`)
   - **Implicación para Metro-Aura** (qué se puede/no se puede hacer, qué
     costará, qué archivo habrá que tocar)
   - Marca `[VERIFICADO]` (leído en código o medido) o `[ESTIMADO]`
     (inferencia, documentación externa, sin poder medir).
4. Se puede compilar y ejecutar el **simulador SDL ya existente** de
   Aura-Firmware (`firmware/tools/build_sim.sh`) y sus scripts de captura
   para responder preguntas empíricas (p. ej. F.6 tipografía gigante), y se
   pueden correr herramientas de Rockbox (`tools/convttf`) escribiendo su
   salida en scratch. Nunca dentro de los repos.
5. Toda pregunta del plan termina en INVESTIGACION.md con respuesta, o con
   `NO RESUELTO: <razón>`. No se omiten preguntas en silencio.
6. Documento en español (México, neutro, sin voseo); identificadores de
   código, rutas y nombres de símbolos tal cual están en inglés.
7. Cierre obligatorio: **sección G "Riesgos y decisiones abiertas"**
   ordenada por impacto (ver §7 de este plan).

### 0.5 Orden de ejecución y prioridad

| Prioridad | Sección | Por qué en ese lugar | Ejecución sugerida |
|---|---|---|---|
| 1 | **E** Compatibilidad Aura Studio | Restricción dura: define qué NO puede cambiar. Es acotada (contratos ya escritos). | Subagente dedicado desde el inicio |
| 2 | **D** Herencia de Aura | Insumo directo de la "Fase Cero de separación" del Plan Maestro. Acotada (27 + 107 archivos). | Subagente dedicado desde el inicio |
| 3 | **A** Arquitectura UI | Decide tema vs. C, la decisión arquitectónica central. | Hilo principal + subagentes por bloque |
| 4 | **C** Input | Pequeña, y sin ella no hay esqueleto twist. | Después de A (comparte `list.c`, `action.c`) |
| 5 | **B** Animaciones | Capa final del proyecto, pero su factibilidad condiciona el diseño (buffers, tiempos). | Subagente para PictureFlow + LCD driver |
| 6 | **F** Referencia Metro/Zune | No depende del código; corre en paralelo con web. | Subagente con WebSearch/WebFetch desde el inicio |

E, D y F son independientes entre sí y de A–C: lanzarlas en paralelo al
comenzar.

---

## A. Arquitectura UI de Rockbox (menús, listas, skin engine, viewports)

**Objetivo**: saber con precisión qué partes de la UI Metro se pueden lograr
solo con temas (WPS/SBS/skin engine + settings) y qué partes exigen C, y
cómo se inserta una UI propia en el arranque sin romper el resto del sistema.

### A.1 Archivos a leer

Rockbox (`Aura-Firmware/firmware/rockbox/`):

- Arranque y control: `apps/main.c` (secuencia de `init()`, `show_logo`,
  entrada a `root_menu()`), `apps/root_menu.c`, `apps/root_menu.h`,
  `apps/menu.c`, `apps/menu.h`, `apps/menus/` (todos, solo estructura),
  `apps/screens.c`, `apps/misc.c` (`default_event_handler()`), `apps/misc.h`.
- Listas: `apps/gui/list.c`, `apps/gui/list.h`, `apps/gui/bitmap/list.c`,
  `apps/gui/bitmap/list-skinned.c`, `apps/gui/line.c`, `apps/gui/line.h`,
  `apps/gui/icon.c`, `apps/gui/scrollbar.c`, `apps/tree.c`, `apps/tree.h`,
  `apps/tagtree.c`, `apps/tagnavi.config`, `apps/onplay.c`.
- Viewports y pantallas: `apps/gui/viewport.c`, `apps/gui/viewport.h`,
  `apps/screen_access.c`, `apps/screen_access.h`, `firmware/export/lcd.h`,
  `firmware/drivers/lcd-bitmap-common.c`, `firmware/drivers/lcd-16bit-common.c`,
  `firmware/drivers/lcd-16bit.c`, `firmware/drivers/lcd-scroll.c`,
  `firmware/scroll_engine.c`, `firmware/export/scroll_engine.h`.
- Skin engine: `apps/gui/skin_engine/skin_engine.c`, `skin_engine.h`,
  `skin_parser.c`, `skin_render.c`, `skin_display.c`, `skin_tokens.c`,
  `skin_backdrops.c`, `wps_internals.h`, `apps/gui/wps.c`,
  `apps/gui/statusbar-skinned.c`, `apps/gui/backdrop.c`,
  `lib/skin_parser/` (gramática: `tag_table.c`, `skin_parser.c`),
  `manual/appendix/wps_tags.tex` (referencia completa de tags),
  `wps/cabbiev2.320x240x16.wps`, `wps/rockbox_failsafe.sbs`,
  `wps/rockbox_failsafe.fms`, `wps/WPSLIST`, `wps/cabbiev2.320x480x16.WIP.sbs`
  (único `.sbs` de color grande, útil para ver la sintaxis de listas skinned).
- Fuentes y texto: `firmware/font.c`, `firmware/export/font.h`
  (¡uno de los 27 modificados por Aura: `MAXFONTS`/`MAX_FONT_SIZE`!),
  `firmware/drivers/lcd-16bit-common.c` (`lcd_putsxyofs`, `lcd_alpha_bitmap_part`,
  `lcd_mono_bitmap_part`), `apps/gui/skin_engine/skin_fonts` no existe:
  la carga de fuentes de skin está en `skin_parser.c`/`skin_engine.c` — localizarla.
- Ajustes: `apps/settings.h`, `apps/settings.c`, `apps/settings_list.c`,
  `apps/menus/settings_menu.c`, `apps/lang/english.lang`, `apps/lang/`
  (cómo se agregan cadenas; ver cómo lo hizo Aura: `apps/aura/aura_lang.c`).
- Memoria: `firmware/buflib_mempool.c`, `firmware/core_alloc.c`, `firmware/include/buflib.h`, `firmware/export/config.h`
  y `firmware/export/config/ipod6g.h` (`PLUGIN_BUFFER_SIZE`, `MEMORYSIZE`),
  `apps/plugin.c` (cómo se reserva el buffer de plugins).
- Precedente Aura (para entender cómo se "secuestra" la UI, no para copiar
  diseño): `apps/aura/aura_main.c` (731 líneas: bucle principal, `next_button()`),
  `apps/aura/aura_nav.c` + `apps/aura/test/test_nav.c` (máquina de navegación
  pura con tests host-side), `apps/aura/aura_menu_list.c`, `apps/aura/aura_widgets.c`,
  `apps/aura/apple2026_shell.c` (679 líneas: helpers de dibujo sobre `lcd_*`),
  y `Aura-Firmware/docs/superficies-rockbox.md` (checklist de todas las
  superficies de Rockbox que "se cuelan" y cómo se taparon: splash, USB,
  apagado, bootloader, mpegplayer).

### A.2 Preguntas a responder

- **A.1** ¿Dónde exactamente toma el control la UI de Rockbox tras `init()` en
  `apps/main.c` y cómo insertó Aura su bucle (`D-001`/`D-014`)? ¿Qué se pierde
  si no se entra nunca a `root_menu()` (p. ej. `default_event_handler`,
  manejo USB, apagado por batería, `SYS_*` events)? Listar qué llamadas de
  mantenimiento debe hacer un bucle propio en cada iteración.
- **A.2** `gui_synclist`: ¿qué callbacks expone (`get_name`, `get_icon`,
  `get_talk`, `get_color`, `draw_item`?), qué controla la geometría de fila
  (altura por fuente, padding, `list_line_height`?), y hasta dónde se puede
  parametrizar SIN modificar `list.c` (filas altas con texto gigante,
  fila seleccionada con fuente distinta a las demás, márgenes asimétricos,
  ítems parcialmente fuera de pantalla)? ¿Qué obligaría a duplicar la lista
  en C propio (como hizo Aura con `aura_menu_list.c`)?
- **A.3** Listas skinned (`list-skinned.c`, tags `%LT`, `%LI`, `%Lb`, `%Vi`…):
  ¿qué permiten (fuentes por estado seleccionado/no seleccionado, posiciones,
  íconos), y qué no (movimiento horizontal, animación, más de una lista
  visible, encabezado de pivots)? Enumerar límites concretos con la línea del
  parser que los impone.
- **A.4** Skin engine en general: catálogo de capacidades relevantes para
  Metro — múltiples fuentes simultáneas (`%Fl`, ¿cuántas?), viewports
  condicionales (`%Vd`, `%?`), texto con recorte (¿recorta o hace scroll
  automático `%s`? ¿se puede desactivar el scroll?), alineación, imágenes,
  barra de progreso, carátula (`%Cd`), colores por viewport, backdrop.
  ¿Existe algún mecanismo de redibujo por cuadro/tiempo controlable por el
  tema (timers, `%t`)? ¿Cuál es el costo de RAM del buffer de skin
  (`skin_buffer`) y de CPU de un redibujo completo del WPS?
- **A.5** Matriz "se logra con tema" vs "exige C" para los patrones Metro
  (llenar con las columnas: patrón / tema sí-parcial-no / archivo C
  involucrado si no): tipografía gigante recortada en el borde derecho,
  encabezado de pivots con el activo en blanco y los demás en gris
  desplazados, lista vertical con selección por posición fija, tiles en
  cuadrícula, fondo con carátula desenfocada/atenuada, barra de estado mínima,
  transición slide, transición turnstile, contador/progreso en Now Playing.
- **A.6** Viewports y recorte: ¿`lcd_putsxy`/`lcd_putsxyofs` recortan glifos
  parcialmente fuera del viewport (necesario para el "texto que se corta en
  el borde")? ¿`lcd_set_viewport` acepta viewports con `x<0` o que
  sobresalen del LCD, o hay validación/`panicf` (`viewport.c`,
  `lcd-bitmap-common.c`)? ¿Cómo se dibuja texto con `x` negativo?
- **A.7** Fuentes: ¿cuántas fuentes pueden estar cargadas a la vez
  (`MAXFONTS`), tamaño máximo de glifo/fuente (`MAX_FONT_SIZE`, tamaño del
  caché de glifos), soporte de fuentes anti-aliased (`font->depth`,
  `lcd_alpha_bitmap_part`), API para medir texto (`font_getstringsize`,
  `font_get_width`), y qué modificó exactamente Aura en `font.h` y por qué?
  ¿Qué implica cargar una fuente de 60–72 px AA en RAM (estimar bytes por
  glifo y por caché)?
- **A.8** Barra de estado y backdrop: cómo se apagan/sustituyen
  (`STATUSBAR_OFF`, `backdrop_file="-"`, ver D-051 en Aura), y qué queda
  dibujando Rockbox por su cuenta aunque la UI sea propia (splash, yesno,
  `usb_screen`, `shutdown`, "Committing database", plugins).
- **A.9** Ajustes: ¿es viable reutilizar `settings_list.c` + `do_menu()` con
  una lista re-estilizada, o Metro debe redibujar cada pantalla de ajustes
  (Aura eligió reescribir; ¿por qué? — buscar en DECISIONS)? ¿Cómo se
  agregan cadenas al `.lang` y en qué idioma(s) va la UI (D-013 de Aura:
  español)? Para Metro decidir en Fase 3; aquí solo listar mecanismo y costo.
- **A.10** Memoria disponible para UI: tamaño del buffer de plugins en 6G,
  cómo se pide RAM al `core_alloc`/`buflib` desde el core sin robársela al
  audio en tiempo de reproducción, y si es viable reservar de forma
  permanente 1–2 framebuffers extra (150 KB c/u) para transiciones.
  ¿Qué reservó Aura para sus transiciones (`aura_transitions.c`) y de dónde?
- **A.11** Íconos/bitmaps: pipeline `apps/bitmaps/` + `bmp2rb`, carga en
  runtime de BMP (`read_bmp_file`, `apps/recorder/bmp.c`), formatos
  soportados en 16 bpp, y el costo que documentó Aura por leer íconos de
  disco por cuadro (D-3xx sobre el morph, `DECISIONS.md` ~línea 461).
- **A.12** Plugins vs core: ¿qué ventajas concretas tiene ejecutar pantallas
  ricas como plugin (PictureFlow) frente al core (acceso a `PLUGIN_BUFFER`,
  API `rb->`)? ¿Cómo lanza Aura `mpegplayer` y `imageviewer` desde su UI
  (`aura_video.c`, `aura_photos.c`, `plugin_set_silent_open_errors()`)?

---

## B. Animaciones y transiciones (LCD del ipod6g, primitivas, PictureFlow, costo)

**Objetivo**: cuantificar qué se puede animar a mano en 320×240×16 con la
CPU S5L8702 a 216 MHz, con qué primitivas, y cuánto cuesta en CPU/batería.

### B.1 Archivos a leer

- Driver LCD del target: `firmware/target/arm/s5l8702/lcd-s5l8702.c`,
  `firmware/target/arm/s5l8702/lcd-asm-s5l8702.S`,
  `firmware/target/arm/s5l8702/ipod6g/lcd-6g.c`,
  `firmware/target/arm/s5l8702/ipod6g/lcd-target.h`,
  `firmware/target/arm/s5l8702/dma-s5l8702.c`, `pl080.c`,
  `firmware/target/arm/s5l8702/system-s5l8702.c`, `clocking-s5l8702.c`
  (frecuencias, `cpu_boost`), `firmware/target/arm/s5l8702/timer-s5l8702.c`,
  `system-target.h` (¿`USEC_TIMER`?), `firmware/kernel/tick.c`,
  `firmware/kernel/timeout.c`, `firmware/timer.c`, `firmware/export/kernel.h` (`HZ`),
  `firmware/backlight.c` (`lcd_active()`, apagado de LCD/backlight),
  `firmware/export/config/ipod6g.h` (`HAVE_LCD_ENABLE`, `HAVE_LCD_SLEEP`,
  `CPU_FREQ`, `HAVE_ADJUSTABLE_CPU_FREQ`).
- Primitivas genéricas: `firmware/drivers/lcd-16bit.c`, `lcd-16bit-common.c`,
  `lcd-bitmap-common.c`, `firmware/export/lcd.h`, `firmware/asm/lcd-as-memframe.c`,
  `firmware/asm/arm/` (memcpy/memset en ensamblador), `firmware/asm/memcpy.c`,
  `lib/fixedpoint/fixedpoint.h`, `apps/recorder/resize.c`, `apps/recorder/resize.h`
  (`HAVE_UPSCALER`), `apps/recorder/jpeg_load.c`, `apps/recorder/albumart.c`.
- Referencia obligatoria de optimización: `apps/plugins/pictureflow/pictureflow.c`
  (5 014 líneas — leer vía subagente con estas consignas: estructura del bucle
  de render, formato de las tablas de punto fijo, cómo dibuja cada slide
  (por columnas, `rb->lcd_bitmap_part`? escritura directa al framebuffer?),
  caché de carátulas escaladas, uso de `rb->cpu_boost`, `rb->yield`,
  `rb->lcd_update` vs `lcd_update_rect`, cómo mide/limita FPS, buffers y de
  dónde salen), `apps/plugins/pictureflow/pictureflow.make`,
  `apps/plugins/lib/xlcd_core.c`, `xlcd_draw.c`, `xlcd_scroll.c`,
  `apps/plugins/lib/bmp_smooth_scale.c`, `apps/plugins/lib/pluginlib_bmp.c`,
  `apps/plugins/lib/pluginlib_jpeg_load.c`, `apps/plugins/test_fps.c`,
  `apps/plugins/test_gfx.c`, `apps/plugins/test_scanrate.c`,
  `apps/plugins/mpegplayer/video_out_rockbox.c` (blit por cuadro real).
- Precedente Aura (motor propio de transiciones): `apps/aura/aura_transitions.c`
  (1 462 líneas), `apps/aura/aura_motion.c` (94, easings) + `test/test_motion.c`,
  `apps/aura/aura_fx.h`, `apps/aura/aura_coverdrift.c`, `apps/aura/aura_flow.c`,
  `apps/aura/aura_musicflow.c`, `apps/aura/aura_nowplaying.c` (`mode4_morph()`,
  instrumentación `DEBUGF`/`TRANSITION_LOG`), y los documentos
  `Aura-Firmware/docs/aura-design-system/transiciones/00-vocabulario.md`,
  `docs/aura-design-system/sistema/06-niveles-de-fx.md`,
  `docs/plans/PLAN-niveles-fx.md` (sistema de niveles de FX con fallback —
  precedente directo del "toggle + fallback" que pide Metro), y las entradas
  de `DECISIONS.md` que hablan de rendimiento en hardware real (buscar
  "morph", "hardware real", "TRANSITION_LOG").
- Fuentes externas (WebSearch/WebFetch, marcar `[ESTIMADO]`): wiki de Rockbox
  páginas "TestFPS"/"LcdFrameRate" (resultados de `test_fps` para
  `ipod6g`/"iPod Classic"), "IpodClassicPort", "S5L8702", "PictureFlow".

### B.2 Preguntas a responder

- **B.1** Ruta completa de `lcd_update()` en 6G: ¿copia el framebuffer al
  controlador por DMA (`pl080`) o por bucle de CPU (`lcd-asm-s5l8702.S`)?
  ¿Es bloqueante? ¿Espera al final del DMA anterior (Aura menciona "espera de
  DMA")? ¿Cuánto tarda una actualización completa y una `lcd_update_rect`
  parcial (evidencia: código + cualquier medición publicada)? ¿Hay tearing/
  vsync (`lcd_wait_frame`?)?
- **B.2** FPS realista: ¿qué reporta `test_fps` en ipod6g según la wiki
  (full-screen 1/1 y 1/4)? ¿Qué FPS declara/logra PictureFlow en este
  hardware? Derivar un **presupuesto por cuadro** (ms para dibujar + ms para
  `lcd_update`) para 30 fps y para 20 fps.
- **B.3** Framebuffer: layout (`lcd_framebuffer`, `FBADDR`, stride,
  RGB565 nativo u orientación transpuesta en `lcd-16bit-vert`?), si el
  viewport soporta un buffer propio (`struct viewport::buffer` /
  `lcd_set_framebuffer`, `viewport_set_buffer`) para **dibujar una pantalla
  completa fuera de línea** y luego blitearla; costo de un `memcpy` de 150 KB
  y de un blit de columnas desplazadas (`lcd_bitmap_part` con `stride`) —
  fundamento del slide horizontal barato.
- **B.4** Primitivas disponibles y sus costos relativos: `lcd_fillrect`,
  `lcd_hline/vline`, `lcd_bitmap_part`, `lcd_bitmap_transparent_part`,
  `lcd_alpha_bitmap_part` (¿solo para fuentes AA o cualquier máscara?),
  `lcd_gradient_fillrect`?, `lcd_set_drawmode` (`DRMODE_*`), ¿existe blend
  alfa RGB565 en el core (`lcd_blit_*`, `blend`)? ¿Qué falta para: fundido
  (fade) a negro, atenuar una carátula, difuminar? Estimar costo por píxel
  en ciclos y por cuadro completo.
- **B.5** PictureFlow, técnicas concretas a replicar (con línea): punto fijo
  y tablas precalculadas, dibujo por columnas, evitar división por píxel,
  cachés, `cpu_boost(true)` durante animación, `yield()`/`sleep(0)` para no
  matar el hilo de audio, lectura de botón no bloqueante durante la
  animación (interrumpibilidad), y cómo alterna buffers.
- **B.6** Reloj de animación: `current_tick` es `HZ=100` (10 ms) — ¿existe un
  temporizador de microsegundos en s5l8702 (`USEC_TIMER`, `timer_register`)
  utilizable desde el core para easings basados en tiempo real y no en
  número de cuadros? ¿Cómo lo hace Aura (`aura_motion.c`)?
- **B.7** Hilos y audio: ¿en qué hilo corre la UI, qué pasa con la
  reproducción si un bucle de animación no cede durante 300 ms, y cuál es la
  disciplina mínima (`yield`, `sleep`, `button_get_w_tmo(0)`)?
- **B.8** Batería/CPU: costo de `cpu_boost` (216 vs frecuencia normal —
  ¿cuál es la normal en 6G?), y política recomendable (boost solo durante la
  transición). ¿Rockbox tiene "boost counter" visible en debug para medir?
- **B.9** `lcd_active()`/backlight: qué garantías necesita una animación
  para no dibujar con el LCD dormido (Aura: "toda animación respeta la puerta
  `lcd_active()`").
- **B.10** Lecciones de Aura: qué transiciones implementó (`aura_transitions.c`
  — enumerar patrones), cuál fue lenta en hardware real y por qué (D-3xx morph:
  íconos desde disco por cuadro, divisiones, `lcd_bitmap` de 1 px), y qué
  mecanismo de fallback/niveles ya diseñó (`06-niveles-de-fx.md`,
  `PLAN-niveles-fx.md`). Extraer reglas de oro para Metro.
- **B.11** Simulador vs hardware: confirmar que el simulador SDL no sirve para
  medir tiempos (Aura ya lo constató) y proponer el método de medición en
  target: overlay FPS de depuración, `DEBUGF` con ticks, `test_fps`.
  Definir cómo la Fase 4 va a decidir "el rendimiento no da → fallback".

---

## C. Input: clickwheel → eventos → UI, y mapeo del twist

**Objetivo**: entender el camino ISR → cola de botones → `action.c` → lista,
qué datos trae la rueda (dirección, repetición, aceleración) y qué tocar para
mapear el twist del Zune 30 y un scroll con aceleración de calidad.

### C.1 Archivos a leer

- `firmware/target/arm/ipod/button-clickwheel.c` (**es el driver que usa
  el 6G**, ver `firmware/SOURCES:801`), `firmware/target/arm/s5l8702/ipod6g/button-target.h`
  (bits `BUTTON_SELECT/MENU/LEFT/RIGHT/SCROLL_FWD/SCROLL_BACK/PLAY`),
  `firmware/drivers/button.c`, `firmware/export/button.h`
  (`BUTTON_REPEAT`, `BUTTON_REL`, `button_get`, `button_get_w_tmo`,
  `button_get_data`, `button_clear_queue`, `HAVE_WHEEL_ACCELERATION`),
  `firmware/export/config/ipod6g.h` (`HAVE_SCROLLWHEEL`,
  `HAVE_WHEEL_ACCELERATION`, `WHEEL_ACCEL_START 270`, `WHEEL_ACCELERATION 3`),
  `firmware/target/arm/s5l8702/ipod6g/piezo-6g.c` (clic sonoro),
  `firmware/powermgmt.c` (apagado por PLAY largo — dónde vive),
  `apps/action.c`, `apps/action.h`, `apps/keymaps/keymap-ipod.c` (503 líneas:
  contextos `CONTEXT_STD`, `CONTEXT_LIST`, `CONTEXT_WPS`, `CONTEXT_TREE`…),
  `apps/gui/list.c` (`gui_synclist_do_button`: manejo de aceleración de
  rueda, `list_do_action_timeout`, salto por páginas), `apps/plugins/lib/pluginlib_actions.c`
  (mapeo que usa PictureFlow), `apps/misc.c` (`default_event_handler`:
  `SYS_USB_CONNECTED`, `SYS_POWEROFF`, `SYS_CHARGER_*`).
- Simulador: `firmware/target/hosted/sdl/button-sdl.c`,
  `uisimulator/buttonmap/ipod.c` (mapa de teclas del ipod en el sim),
  `uisimulator/common/sim_tasks.c` (modificado por Aura: capturas), y los
  scripts `Aura-Firmware/firmware/tools/apple2026_sim_shot.sh` /
  `apple2026_sim_matrix.sh` (cómo inyectan `"SELECT,SCROLL_FWD,…"` sin
  Accesibilidad de macOS: D-008/D-017).
- Precedente Aura: `apps/aura/aura_wheel.c` (51 líneas), `apps/aura/aura_main.c`
  (`next_button()`: filtrado de `BUTTON_REL`; ver la entrada de DECISIONS
  sobre el `REL` pendiente que cerraba un mensaje), `apps/aura/aura_nav.c`
  + `test/test_nav.c`.

### C.2 Preguntas a responder

- **C.1** Flujo completo de un movimiento de rueda en 6G: ISR/poll en
  `button-clickwheel.c` → ¿qué se encola (`BUTTON_SCROLL_FWD` con
  `BUTTON_REPEAT`? ¿un contador de "clics" o velocidad en los bits altos /
  `button_get_data()`?) → `button.c` (¿aplica `WHEEL_ACCELERATION` ahí o en
  `list.c`?) → `action.c` (`ACTION_STD_NEXT/PREV`, `ACTION_STD_NEXTREPEAT`)
  → `list.c`. Citar líneas en cada salto.
- **C.2** Aceleración: qué significan `WHEEL_ACCEL_START` y
  `WHEEL_ACCELERATION`, cómo se traduce velocidad a "ítems por evento" en
  `gui_synclist_do_button`, y qué habría que hacer para una aceleración
  propia (curva Zune: lenta al inicio, salto de página tras N clics, con
  "letra índice" flotante).
- **C.3** Botones: `LEFT`, `RIGHT`, `MENU`, `SELECT`, `PLAY` — pulsación
  corta vs larga (`BUTTON_REPEAT` tras cuánto tiempo, `button_hold`?),
  combinaciones que el firmware/hardware **reserva** y no se pueden remapear
  (`SELECT+MENU` reinicio, `SELECT+PLAY`?, `PLAY` largo apaga: ¿en driver,
  `powermgmt`, o keymap?), y `BUTTON_REL` (Aura lo filtra: por qué).
- **C.4** Dos estrategias para una UI propia: (a) `get_action()` con
  contextos de `keymap-ipod.c` (¿se pueden agregar contextos nuevos sin
  tocar `action.h`?), (b) `button_get*` crudo como Aura. Ventajas, riesgos
  (perder `SYS_*`, USB, apagado), y qué eligió Aura y por qué (`aura_main.c`,
  DECISIONS).
- **C.5** Insumo para la tabla "gesto Zune → evento clickwheel" de la Fase 3:
  inventario exhaustivo de eventos distintos disponibles (con y sin repeat,
  con y sin datos de velocidad, hold), para que la Fase 3 asigne:
  vertical = rueda, profundizar = SELECT, retroceder = MENU, twist entre
  pivots = LEFT/RIGHT (corto), acciones de reproducción = PLAY / LEFT/RIGHT
  largos, etc. **No decidir el mapeo aquí**; solo enumerar la materia prima
  y sus restricciones.
- **C.6** Simulador: cómo se envían `SCROLL_FWD/BACK` con teclado/rueda del
  sim, cómo se emula aceleración, y cómo capturar pantalla headless (reusar
  la técnica de Aura para las capturas de "hecho" de la Fase 4).
- **C.7** Piezo: ¿el clic de la rueda lo dispara el driver o la UI
  (`piezo_button_beep`)? ¿Configurable? (Zune 30 no tenía clic; decidir en
  Fase 3.)

---

## D. Herencia de Aura-Firmware: hardware/build (portar) vs design system (reemplazar)

**Objetivo**: lista exacta, archivo por archivo, de qué se porta intacto a
Metro-Aura en la Fase Cero y qué se descarta, con la razón.

### D.1 Archivos a leer

- `Aura-Firmware/MODIFICATIONS.md` (lista canónica de los 27 archivos de
  Rockbox modificados, con su `D-NNN`), `Aura-Firmware/DECISIONS.md` y
  `DECISIONS-ARCHIVE.md` (buscar cada `D-NNN` citado en MODIFICATIONS.md:
  como mínimo D-002, D-007, D-008, D-017, D-021, D-032, D-050, D-051, D-052,
  D-055, D-061, D-062, D-064, D-244, D-284/285, D-286, D-293, D-298,
  D-304…D-309, D-321), `Aura-Firmware/README.md`, `docs/guia-desarrollo.md`,
  `docs/guia-flasheo-restauracion.md`, `firmware/dist/README.md`,
  `firmware/tools/*.sh`, `.gitignore` de Aura-Firmware.
- Los 27 archivos (rutas relativas a `Aura-Firmware/firmware/rockbox/`),
  cada uno diffeado contra el upstream `0726ec9` (§0.2):
  `apps/SOURCES`, `apps/bitmaps/native/rockboxlogo.320x98x16.bmp`,
  `apps/bitmaps/native/usblogo.176x48x16.bmp`, `apps/gui/splash.c`,
  `apps/gui/usb_screen.c`, `apps/main.c`, `apps/misc.c`, `apps/misc.h`,
  `apps/plugin.c`, `apps/plugin.h`, `apps/plugins/mpegplayer/mpeg_settings.c`,
  `apps/plugins/mpegplayer/mpeg_settings.h`, `apps/plugins/mpegplayer/mpegplayer.c`,
  `apps/plugins/mpegplayer/stream_mgr.c`, `apps/plugins/mpegplayer/video_out.h`,
  `apps/plugins/mpegplayer/video_out_rockbox.c`, `apps/plugins/solitaire.c`,
  `apps/settings.h`, `apps/tagcache.c`, `apps/tagcache.h`,
  `bootloader/ipod-s5l87xx.c`, `firmware/export/config/ipod6g.h`,
  `firmware/export/font.h`, `firmware/target/hosted/filesystem-unix.c`
  (documentado como byte-idéntico: confirmar), `lib/rbcodec/codecs/aiff.c`,
  `uisimulator/common/sim_tasks.c`, `utils/mks5lboot/Makefile`.
- `apps/aura/` completo (107 archivos): solo cabecera + resumen de
  responsabilidad por archivo (subagente), para clasificarlos.
- `Aura-Firmware/design-system/` (`generate.py`, `tokens.json`, `scripts/`,
  `vendor/inter-ttf`, `vendor/lucide-svg`, `vendor/phosphor-svg`, `assets/`):
  qué genera (fuentes `.fnt` vía `convttf`, íconos BMP, logo de arranque,
  `AuraPalette.swift`) y qué partes del pipeline son reutilizables sin el
  diseño Apple.
- `Aura-Firmware/firmware/toolchain/` (solo verificar reubicabilidad, no
  leer), `firmware/build-sim/` y `firmware/build-ipod6g/` (solo para saber
  cómo se configuraron: `Makefile`, `autoconf.h`).

### D.2 Preguntas a responder

- **D.1** Para cada uno de los 27 archivos: tabla con columnas *archivo /
  qué cambia (1 línea) / D-NNN / categoría / veredicto para Metro*, donde
  categoría ∈ {**HW-BUILD** (fix de hardware, build, simulador, codec —
  portar intacto), **COMPAT-STUDIO** (tagcache/sync/settings que Studio
  necesita — portar intacto o casi), **DISEÑO** (Apple/Aura — no portar),
  **MIXTO** (separar el diff en dos)}. Casos que ya se anticipan como
  MIXTOS y hay que desmenuzar: `ipod6g.h` (backlight/USB = HW; ¿algo de UI?),
  `mpegplayer/*` (modo "cubrir" = feature neutra; paleta/OSD Aura = diseño;
  traducción al español = decidir), `apps/main.c` (el gancho de arranque es
  el mecanismo a replicar; el logo/splash centrado es diseño), `font.h`
  (límite de fuentes = infra, pero re-dimensionar para Metro), `misc.c`,
  `splash.c`/`usb_screen.c` (mecanismo de re-skin = reutilizable; look =
  diseño), `tagcache.c` (D-021/D-244/D-293: ¿todo es compat/robustez?),
  `settings.h`.
- **D.2** Para `apps/aura/` (107 archivos): clasificar cada archivo en
  {**INFRA REUTILIZABLE tal cual o casi** (p. ej. `aura_sync.c`,
  `aura_sync_marker.c`, `aura_media_categories.c`, `aura_artist_images*.c`,
  `aura_fsutil.c`, `aura_lrc.c`, `aura_albumart.c`/`aura_art.c`,
  `aura_music.c` consultas a tagcache, `aura_settings.c` formato `aura.cfg`,
  `aura_device_name.c`, `aura_wheel.c`, `aura_motion.c`, `aura_lang.c`,
  `aura_video.c`, `aura_photos.c`, `test/`), **PATRÓN A COPIAR pero código
  nuevo** (bucle principal, máquina de navegación, motor de transiciones,
  niveles de FX), **DISEÑO APPLE — no portar** (`apple2026_*`, `aura_style*`
  sistema de temas, `aura_widgets`, `aura_status_bar*`, `aura_coverdrift`,
  `aura_musicflow`/`movieflow`, `aura_screenlock`, `aura_patterns`,
  `aura_selection_summary`, etc.)}. Anotar dependencias cruzadas: ¿se puede
  extraer `aura_sync.c` sin arrastrar `apple2026_shell`? Listar los `#include`
  de cada archivo INFRA.
- **D.3** Licencias/branding: qué assets de Aura son libres (Inter — OFL,
  Lucide — ISC, Phosphor — MIT) y cuáles son marca Aura (logo, wordmark,
  nombre "aura" en rutas `.rockbox/aura/`, `a26-*`). Para Metro: ¿conviene
  **conservar los nombres de ruta `.rockbox/aura/…`** por compatibilidad con
  Studio aunque el firmware se llame Metro? (Responder con la evidencia de E;
  aquí solo señalar el conflicto.)
- **D.4** Build: ¿el toolchain `Aura-Firmware/firmware/toolchain/` es
  reutilizable desde otro checkout con solo `PATH` (comprobar con
  `arm-elf-eabi-gcc --version` y `-print-search-dirs` que no dependa del
  `cwd`; verificar si `--prefix` quedó absoluto y si eso es un problema), o
  Metro debe compilar el suyo (~1 h)? ¿`tools/configure` de Rockbox necesita
  el parche de D-007 (gcc de Homebrew) — está dentro de los 27? Documentar
  el comando exacto de configuración de sim y target que usó Aura
  (`build-sim/Makefile`, `autoconf.h`).
- **D.5** Simulador: qué cambió Aura para capturas headless
  (`sim_tasks.c`, D-008/D-017) y si ese cambio es HW-BUILD (portar) — sí lo
  parece; confirmar y describir la interfaz (variables de entorno, ticks,
  secuencia de teclas) para reusar la técnica de captura como criterio de
  "hecho" en Fase 4.
- **D.6** Bootloader y flasheo: qué cambió en `bootloader/ipod-s5l87xx.c`
  (D-064: arranque silencioso — ¿HW o diseño? Es UX pero de bajo nivel:
  clasificar), cómo se construye `bootloader-ipod6g.ipod`, qué hace
  `mks5lboot` con el parche de libusb (D-050), y el procedimiento de flasheo
  y restauración (`guia-flasheo-restauracion.md`) — insumo para
  `ESTADO_FINAL.md`.
- **D.7** Upstream: ¿hay commits en `github.com/Rockbox/rockbox` posteriores
  a `0726ec9` que toquen `firmware/target/arm/s5l8702/`, `ipod6g`,
  `button-clickwheel.c`, `tagcache.c` o el simulador SDL en macOS? (WebFetch
  al historial de esas rutas.) Recomendación: sembrar Metro en `0726ec9`
  salvo evidencia fuerte en contra.
- **D.8** Interfaz mínima que Metro debe conservar del arranque de Aura para
  no romper Studio ni el disco (`aura_main_sync_after_disk_handoff()`,
  `aura_settings_apply_pending_clock()`, USB screen → reanudación): listar
  las funciones/puntos de enganche con su archivo.

---

## E. Compatibilidad con Aura Studio (restricción dura)

**Objetivo**: tabla exhaustiva de todo lo que Studio escribe/lee/espera en el
disco del iPod, y en qué puntos del firmware se toca; clasificar cada fila en
OBLIGATORIO / RECOMENDADO / FUERA DE ALCANCE para Metro, con la consecuencia
concreta de no cumplirla.

### E.1 Archivos a leer

- Contratos (leer las copias de **ambos** repos y diffearlas para confirmar
  que son idénticas): `Aura-Firmware/CONTRATO-firmware-studio.md` (v7, §A–§E,
  tabla §D y D.1–D.4), `Aura-Firmware/docs/contracts/library-layout-v1.md`
  (§1–§4: layouts de `Music/`, `Videos/`, `Photos/`, carátulas, `.lrc`,
  póster, marcador `/.aura/sync-pending.json`), `CONTRATO-dispositivo.md`,
  `CONTRATO-formato-tema.md`; `Aura-Studio/CONTRATO-*.md`,
  `Aura-Studio/docs/contracts/library-layout-v1.md`, `Aura-Studio/FIRMWARE_VERSION`,
  `Aura-Studio/scripts/fetch-firmware.sh`, `Aura-Studio/DECISIONS.md`
  (ST-011, ST-012, ST-013, ST-019, ST-026, ST-035, ST-039, ST-041 y las que
  toquen sync/instalación), `Aura-Studio/CLAUDE.md`, `Aura-Studio/README.md`.
- Studio (Swift, `Aura-Studio/studio/AuraStudio/Sources/AuraStudio/`):
  `Services/AuraDeviceProbe.swift` (**cómo decide que el iPod tiene Aura /
  "ya arrancó"**), `Services/AuraUpdateChecker.swift` (hash de
  `rockbox.ipod`), `Services/InstallPlanner.swift`, `Services/BundledArtifacts.swift`,
  `Services/MKS5LBootRunner.swift`, `Services/GitHubReleaseChecker.swift`,
  `Services/LibrarySync.swift`, `Services/DeviceSyncIndex.swift`,
  `Services/SyncMarker.swift`, `Services/PathSanitizer.swift`,
  `Services/PlaylistExporter.swift`, `Services/PlaylistImporter.swift`,
  `Services/AudioTranscoder.swift`, `Services/FFmpegTranscoder.swift`,
  `Services/ImageResizer.swift`, `Services/ID3Writer.swift`,
  `Services/ArtistImageStore.swift`, `Services/ClockSyncWriter.swift`,
  `Services/DeviceNameStore.swift`, `Services/ThemeInstaller.swift`,
  `Services/ThemeValidator.swift`, `Services/MediaCategoryClassifier.swift`,
  `Services/LocalTagReader.swift`, `Models/LibraryGrouping.swift`,
  `Models/MediaCategory.swift`, `Models/TrackMetadata.swift`,
  `Models/Playlist.swift`, `Models/LibraryPersistence.swift`,
  `Models/InstallerStep.swift`, `ViewModels/InstallerViewModel.swift`
  (sentinela `.rockbox/fonts/a26-title-20.fnt`).
- Firmware, lado que consume el contrato (`Aura-Firmware/firmware/rockbox/`):
  `apps/aura/aura_sync.c`, `aura_sync_marker.c`, `aura_settings.c` (todas
  las claves que escribe en `aura.cfg` y cómo regenera el archivo entero),
  `aura_device_name.c`, `aura_media_categories.c`, `aura_artist_images.c`,
  `aura_artist_images_parse.c`, `aura_photos.c`, `aura_music.c`,
  `aura_video.c`, `aura_lrc.c`, `aura_albumart.c`, `aura_main.c`
  (`aura_main_sync_after_disk_handoff()`), `apps/tagcache.c` (+ D-293),
  `apps/tagcache.h`, `apps/tagtree.c`, `apps/tagnavi.config`,
  `lib/rbcodec/metadata/` (formatos: `mp3.c`, `mp4.c`, `flac.c`, `aiff.c`,
  `wave.c`, `id3tags.c`), `apps/recorder/albumart.c` (orden de búsqueda de
  `cover.jpg`), `apps/playlist.c` (`.m3u8`, rutas), `apps/plugins/mpegplayer/`
  (contenedor/códecs de video aceptados), `firmware/export/config/ipod6g.h`.
- Datos de prueba: `Aura-Firmware/firmware/test-media/`, `Aura-Studio/test-media/`,
  `Aura-Firmware/firmware/tools/gen_test_media.sh`.

### E.2 Preguntas a responder

- **E.1** Tabla completa derivada de §D del contrato (una fila por ruta o
  clave, ~25 filas): *ruta / quién escribe / quién lee / formato / punto del
  firmware que la toca (archivo:función) / clasificación para Metro
  (OBLIGATORIO · RECOMENDADO · FUERA DE ALCANCE) / consecuencia de no
  cumplir*. Como mínimo: `Music/` (3 layouts), `Videos/` + póster `.jpg`,
  `Photos/` (D.1), `Playlists/`, `cover.jpg`/carátula embebida, `.lrc`,
  `.rockbox/database_*.tcd`, `/.aura/sync-pending.json`,
  `.rockbox/aura/aura.cfg` y cada clave que Studio lee o escribe
  (`theme_id`, `theme_format_supported`, `sync_marker_supported`,
  `rtc_sync_*`, `tz_local_quarters`), `device.cfg`, `sync_summary.cfg`,
  `sync_manifest.json`, `ratings.cfg`, `video_categories.cfg`,
  `photo_categories.cfg`, `artists/` + `artist_images.cfg`, `themes/<id>/`,
  `.rockbox/icons/aura/`, `.rockbox/fonts/a26-title-20.fnt`, `.rockbox/rockbox.ipod`.
- **E.2** **Detección de Aura por Studio**: ¿qué comprueba exactamente
  `AuraDeviceProbe.swift` (existencia de `aura.cfg`? claves? `rockbox.ipod`?
  fuente sentinela?) y qué cambia en el comportamiento de Studio si el
  dispositivo "no es Aura" (¿se niega a sincronizar? ¿ofrece instalar Aura
  encima de Metro? ¿borra `database_*.tcd`?). Esta es la pregunta más
  crítica de la sección: determina si Metro debe **imitar la identidad de
  Aura en disco** (escribir `aura.cfg` con las mismas claves) para que Studio
  lo trate como compatible.
- **E.3** `AuraUpdateChecker`/instalador: al ver un `rockbox.ipod` cuyo hash
  no coincide con el embebido, ¿solo avisa o puede reinstalar Aura sin
  confirmación? Riesgo de que Studio "actualice" Metro de vuelta a Aura.
  ¿Qué mecanismo tendría Metro para su propia distribución (release propio,
  `--from-dir`, instalación manual)? Solo documentar; la decisión es de
  Fase 3.
- **E.4** Marcador de sincronización: flujo completo Studio→firmware
  (`SyncMarker.swift` ↔ `aura_sync_marker.c`/`aura_sync.c`: esquema JSON,
  `attempts`, cuándo se borra, `Q_UPDATE` vs `Q_REBUILD`), y qué pasa si el
  firmware **no** anuncia `sync_marker_supported` (Studio borra
  `database_*.tcd` y el firmware reconstruye al arrancar — ¿es aceptable
  como mínimo viable para Metro o es demasiado lento con bibliotecas
  grandes?). Cuantificar con lo que digan las decisiones de Aura.
- **E.5** Tagcache: qué tags usa Studio al escribir metadatos
  (`ID3Writer.swift`, `TrackMetadata.swift`: álbum, artista de álbum,
  agrupación, número de disco, año, género, compositor, rating?) y cómo los
  indexa Rockbox (`tagcache.c` `TAG_*`; ¿`tagnavi.config` o consultas
  directas como `aura_music.c`?). ¿Qué campos usa la UI de Aura que Metro
  también necesitará (p. ej. `albumartist` para agrupar)? ¿Cómo se buscan
  las carátulas (`albumart.c`: `cover.jpg` en carpeta del álbum vs embebida)
  y las letras (`aura_lrc.c`: mismo nombre base)?
- **E.6** Formatos que Studio produce y el firmware debe reproducir/mostrar:
  audio (`AudioTranscoder`/`FFmpegTranscoder`: ¿AAC/ALAC en `.m4a`, MP3,
  FLAC, AIFF, WAV? bitrates), video (`.mpg` MPEG-1/2 + MP2 audio, resolución
  320×240 o mayor, ¿lo que acepta `mpegplayer`?), fotos (JPEG baseline
  ≤640 px, D.1), fotos de artista (JPEG ≤128 px cuadrado, D.3), pósters.
  Verificar contra `lib/rbcodec/codecs/`, `mpegplayer`, `jpeg_load.c`.
- **E.7** Reglas de nombres/rutas (`PathSanitizer.swift`, `library-layout-v1.md`):
  charset, longitud máxima (≤ 95 bytes en `Photos/`, `read_line()` de 63
  bytes en `.cfg`), Unicode NFC/NFD, límites que el firmware asume
  (`MAX_PATH`, `tagcache` tamaño de campo).
- **E.8** Playlists: formato exacto que exporta `PlaylistExporter.swift`
  (`.m3u8`, rutas absolutas/relativas, BOM, orden), y qué soporta
  `apps/playlist.c`. ¿Aura muestra playlists desde `Playlists/`? ¿Cómo?
- **E.9** Puntos de arranque/USB del firmware que el contrato exige
  (`aura_main_sync_after_disk_handoff()`: aplicar `rtc_sync_*`, leer
  marcador; volver de la pantalla USB): describir el orden exacto y qué
  ocurre si Metro no lo hace (reloj desincronizado, base vieja).
- **E.10** Temas (`CONTRATO-formato-tema.md`, `ThemeInstaller.swift`): ¿qué
  pasa si Metro no soporta el formato de tema (`theme_format_supported`
  ausente)? Confirmar que Studio degrada bien (no instala) — candidato a
  FUERA DE ALCANCE.
- **E.11** Checklist preliminar de compatibilidad (insumo para el §4 del
  Plan Maestro): lista de comprobaciones binarias, cada una con su método de
  verificación en simulador (disco simulado con `gen_test_media.sh` +
  archivos del contrato) o en hardware.

---

## F. Referencia Metro / Zune: catálogo de patrones, paleta, tipografía, transiciones

**Objetivo**: catálogo priorizado (impacto visual vs. costo en este hardware)
de lo que se replica del Zune 30 (estructura twist) y del Zune HD (lenguaje
Metro), con valores concretos, y la estrategia para la tipografía gigante sin
Zegoe/Segoe.

### F.1 Fuentes a consultar

No hay código que leer; es investigación documental (WebSearch/WebFetch)
más experimentos con herramientas ya existentes. Todo lo no verificable se
marca `[ESTIMADO]`.

- Zune 30 / firmware 2.x "twist interface": reseñas de 2007 (Engadget,
  Gizmodo, Ars Technica "Zune 2.0 review"), videos de la UI (YouTube:
  "Zune 30 twist interface", "Zune 2.0 UI walkthrough"), Wikipedia "Zune"
  (secciones de software/firmware), manual de usuario del Zune 30 (PDF de
  Microsoft, "Zune Product Guide"), artículos de diseño sobre el equipo de
  Zune (Jeff Fong / Zune design team, "Metro" origen).
- Zune HD: reseñas (Engadget, Ars, Anandtech 2009), videos de transiciones
  ("Zune HD UI transitions", "Zune HD quickplay"), documentos de Windows
  Phone 7 que formalizaron el mismo lenguaje: "Windows Phone 7 UI Design and
  Interaction Guide" (PDF, v2.0, 2010) — escala tipográfica (Segoe WP:
  tamaños 15/17/19/20/24/32/42/54/72 px), colores de acento con hex,
  Panorama/Pivot, "turnstile"/"continuum"/"swivel"/"slide"/"readerboard"
  (nombres de transiciones de WP7/Zune HD), Silverlight Toolkit for WP7
  (`TurnstileTransition`, `SlideTransition`, `SwivelTransition`,
  `RotateTransition` — duraciones y easings exactos están en el código
  fuente del toolkit en GitHub/CodePlex; buscar los valores).
- Paleta: colores de acento de Windows Phone 7 (magenta `#FF0097`, púrpura
  `#A200FF`, teal `#00ABA9`, lima `#8CBF26`, café `#A05000`, rosa `#E671B8`,
  naranja `#F09609`, azul `#1BA1E2`, rojo `#E51400`, verde `#339933` —
  verificar contra la guía oficial), fondo negro/blanco de Zune HD, grises de
  texto secundario (WP7 `PhoneSubtleBrush` ~66% opacidad, `PhoneDisabledBrush`)
  y la paleta del software Zune de escritorio (rosa Zune `#F10DA2`? — verificar).
- Tipografía: Zegoe UI (Zune) / Segoe WP (WP7) — propietarias. Alternativas
  a evaluar: **Selawik** (Microsoft, SIL OFL 1.1, métricamente compatible
  con Segoe UI; pesos Light/Semilight/Regular/Semibold/Bold — verificar
  licencia, repo `github.com/microsoft/Selawik` y que incluya Light),
  **Open Sans** (Steve Matteson, mismo diseñador de Segoe UI; OFL),
  **Noto Sans**/**Droid Sans**, y como referencia de "ancho de línea" el
  Inter que Aura ya convierte. Herramientas: `tools/convttf.c` (opciones:
  tamaño en px, `-a`/anti-alias, rangos de caracteres, `-t` trimming), 
  `tools/convbdf.c`, `Aura-Firmware/design-system/generate.py` (invocación
  real de `convttf` con Inter y cómo nombra/instala los `.fnt`),
  `Aura-Firmware/docs/aura-design-system/fundamentos/02-tipografia.md`
  (escala que ya funcionó en 320×240 — como comparativa de legibilidad).
- Interacción no táctil: cómo mostraba el Zune 30 la selección (ítem
  seleccionado blanco/grande, resto gris; encabezado de categorías con la
  activa resaltada), la "letra flotante" al hacer scroll rápido, y el
  comportamiento del botón Back vs Left en el pad.

### F.2 Preguntas a responder

- **F.1** Estructura del Zune 30 (twist): describir con precisión la
  jerarquía de pantallas (raíz: music/videos/pictures/radio/podcasts/
  marketplace/social/settings; dentro de Music: pivots artists/albums/songs/
  genres/playlists…), qué hacía cada botón del pad (arriba/abajo, izq/der,
  centro, Back, Play/Pause), qué mostraba la cabecera al girar entre pivots,
  y cómo se veía la selección. Producir un diagrama textual del árbol
  (insumo directo del esqueleto de navegación de la Fase 3).
- **F.2** Zune HD, catálogo de patrones visuales con especificación
  operativa (qué se ve, tamaños relativos, colores): tipografía gigante
  recortada, encabezado Panorama/Pivot, tiles (Quickplay/pins), lista con
  índice alfabético, Now Playing (carátula grande + texto), fondo con
  carátula atenuada, iconografía mínima, barra de estado (reloj/batería).
- **F.3** Catálogo de transiciones (Zune HD y WP7, más las del Zune 30):
  nombre / qué mueve / duración / easing / cuándo se usa — con los valores
  del Silverlight Toolkit cuando existan (`[VERIFICADO]` si se leyó el código,
  `[ESTIMADO]` si es de video). Incluir: slide (in/out, left/right),
  turnstile (rotación en Y con perspectiva), continuum (el ítem "vuela" al
  título de la siguiente pantalla), swivel, fade, readerboard, "peek" del
  Panorama, movimiento de fondo (parallax) del Panorama.
- **F.4** Matriz de priorización para el hardware del 6G: filas = patrones
  de F.2 + F.3; columnas = impacto visual (alto/medio/bajo), costo de
  implementación (usando los hallazgos de A y B: p. ej. slide = memcpy de
  columnas → barato; turnstile = proyección por columna tipo PictureFlow →
  medio; continuum = texto que se mueve y escala → medio; blur de fondo →
  caro, sustituir por atenuación/dither; parallax → medio), riesgo,
  recomendación (hacer / hacer degradado / no hacer). Ordenar por
  impacto/costo.
- **F.5** Paleta final propuesta en RGB565-friendly: negro/blanco base,
  2 grises de texto secundario, ≥8 acentos con hex, y verificación de que
  cada color sobrevive a 5-6-5 bits sin virar (calcular el RGB565 más
  cercano y su error). Modo claro y oscuro (Zune HD ofrecía ambos).
- **F.6** Tipografía gigante — experimento obligatorio (en scratch, sin
  tocar repos): tomar la alternativa candidata (Selawik Light/Regular, u
  Open Sans si Selawik falla), convertirla con `convttf` a 3 tamaños
  (~24, ~42, ~64 px) con anti-alias, medir el tamaño del `.fnt`, verificar
  que carga (límite `MAX_FONT_SIZE`/`font.h`) y **capturar en el simulador
  de Aura** (o vía un plugin de prueba existente como `test_gfx`, sin
  modificar código) cómo se ve un titular recortado en el borde derecho.
  Si no es posible sin tocar código, documentar el bloqueo y dejar el
  experimento como primer paso de la Fase 4. Reportar: legibilidad,
  peso en RAM del caché de glifos, tiempo de carga.
- **F.7** Estrategia de tipografía definitiva: (a) fuente libre
  métricamente cercana empaquetada en el build (¿cuál, qué pesos, qué
  tamaños, qué rangos Unicode: latín básico + latín-1 + símbolos), y (b)
  flujo opcional "el usuario aporta Zegoe/Segoe": qué haría el usuario
  (`convttf` local con parámetros documentados → copiar `.fnt` a
  `.rockbox/fonts/` con un nombre convenido → el firmware lo prefiere si
  existe), y si conviene apoyarse en el mecanismo de "Estilo"/tema de Aura o
  en algo más simple. Documentar riesgo legal: cero fuentes de Microsoft en
  el repo ni en el `.zip` distribuido.
- **F.8** Sonido/háptica: ¿el Zune tenía clics? (No táctil: decidir en
  Fase 3 si el piezo del iPod se usa.) Solo documentar.
- **F.9** Naming y branding: nombre "Metro-Aura" en la UI, logo de arranque
  (bitmap 320×98 que reemplaza a `rockboxlogo`), sin material de Microsoft
  (logotipos Zune/Windows) — listar qué es homenaje permitido (formas,
  colores, tipografía libre) vs. qué evitar (logos, nombres registrados en
  pantalla).

---

## 7. Cierre de INVESTIGACION.md — sección G "Riesgos y decisiones abiertas"

Al terminar A–F, escribir la sección G con una tabla ordenada por impacto
(alto → bajo): *ID / riesgo o decisión / evidencia (sección.pregunta) /
opciones / recomendación del investigador / a quién toca decidir (Fase 3)*.
Deben aparecer, como mínimo, estas decisiones si la investigación no las
cierra por sí sola:

1. Identidad en disco: ¿Metro escribe `.rockbox/aura/aura.cfg` y demás rutas
   `aura/` para que Studio lo reconozca? (E.2, D.3)
2. Marcador de sync: implementar `sync_marker_supported` desde el inicio o
   aceptar el fallback de borrado de `.tcd`. (E.4)
3. Distribución/actualización: cómo convive con `AuraUpdateChecker`. (E.3)
4. Tema vs C: proporción recomendada y qué se gana/pierde. (A.5)
5. Bucle principal propio (estilo Aura) vs `root_menu` + listas de Rockbox
   re-estiladas. (A.1, A.2, C.4)
6. Buffers de transición: cuánta RAM reservar y de dónde. (A.10, B.3)
7. FPS objetivo y política de fallback/niveles. (B.2, B.10, B.11)
8. Fuente base y flujo de fuente aportada por el usuario. (F.6, F.7)
9. Idioma de la UI (español como Aura, inglés como Zune original, o ambos
   vía `.lang`). (A.9)
10. Commit base de Metro-Aura (`0726ec9` vs upstream más nuevo). (D.7)
11. Piezo/clic. (C.7, F.8)

Cada punto lleva la recomendación del investigador con su justificación,
para que la Fase 3 solo tenga que ratificar o rebatir.

---

## 8. Definición de "hecho" de la Fase 2

- `Metro-Aura/docs/INVESTIGACION.md` existe, con secciones A–G en este orden,
  y **cada pregunta identificada** (A.1…A.12, B.1…B.11, C.1…C.7, D.1…D.8,
  E.1…E.11, F.1…F.9) tiene respuesta con evidencia e implicación, o
  `NO RESUELTO: <razón>`.
- Toda afirmación cuantitativa (FPS, ms, KB, hex de color) lleva
  `[VERIFICADO]` o `[ESTIMADO]`.
- Las tablas D.1 (27 archivos), D.2 (107 archivos) y E.1 (~25 filas) están
  completas, sin "etc.".
- Ningún archivo fuera de `Metro-Aura/docs/INVESTIGACION.md` fue creado o
  modificado dentro de los tres repos (verificable con `git status` en cada
  uno).
- El investigador cierra con la parada obligatoria: resumen, qué revisar,
  fase siguiente (3) y modelo recomendado (Fable).
