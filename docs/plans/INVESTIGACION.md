# INVESTIGACIÓN — Metro-Aura

**Fase 2 del proyecto Metro-Aura.** Ejecución de `PLAN_INVESTIGACION.md` sección por sección. Documento de solo lectura sobre el código real de `Aura-Firmware/`, `Aura-Studio/` y referencias externas — no se modificó ningún archivo en ninguno de los tres repositorios durante esta fase; el único archivo creado/editado fue este.

Fecha: 2026-08-19/20. Cada hallazgo lleva su pregunta (ID del plan), respuesta concreta, evidencia (ruta:línea o URL) e implicación para Metro-Aura, marcado `[VERIFICADO]` o `[ESTIMADO]`.

---

# A. Arquitectura UI de Rockbox (menús, listas, skin engine, viewports)

## A.1 — ¿Dónde toma el control la UI de Rockbox y cómo se insertó Aura?

**Respuesta:** `apps/main.c:int main(void)` corre `init()` (inicializa hardware, kernel, buflib, LCD, botones, powermgmt, backlight, unicode, idioma, `viewportmanager_init()`, storage, PCM/DSP, `settings_load()`, dircache, tagcache, `tree_mem_init()`, `filetype_init()`, `playlist_init()`, `audio_init()`) y luego, en vez de entrar a `root_menu()` (el punto de entrada estándar de Rockbox), llama **una sola vez** a `aura_main()` y nunca regresa (`main.c` no tiene ningún código después de esa llamada). El hook es un reemplazo de una sola línea, no una integración incremental.

**Evidencia:** `apps/main.c:249-253` (comentario explícito "Aura reemplaza por completo la UI de Rockbox... aura_main() es el unico punto de entrada"), `apps/main.c:169-253` (cuerpo completo de `main()`), `apps/main.c:430-560` (`init()`: `system_init()`, `core_allocator_init()`, `kernel_init()`, `lcd_init()`, `font_init()`, `show_logo_boot()`, `button_init()`, `powermgmt_init()`, `backlight_init()`, `unicode_init()`, `lang_init()`, `gui_syncstatusbar_init()`, `gui_sync_skin_init()`, `sb_skin_init()`, `viewportmanager_init()`, `storage_init()`, `pcm_init()`, `dsp_init()`, `settings_reset()`/`settings_load()`, `init_dircache()`, `init_tagcache()`, `tree_mem_init()`, `filetype_init()`, `playlist_init()`, `audio_init()`).

`root_menu()` (y `tree.c`, `tagtree.c`, `apps/recorder/keyboard.c`, `apps/gui/yesno.c`, `rockbox_browse()`, `do_menu()`/`main_menu()`) **siguen compilados y linkeados pero son inalcanzables** — nadie en `apps/aura/` los invoca. Regla de proyecto explícita: "ninguna pantalla nueva de Aura debe llamar a `kbd_input()`/teclado de Rockbox, `gui_syncyesno()`, `rockbox_browse()`, `do_menu()`/`main_menu()`, ni cualquier función de `apps/gui/`, `apps/menus/`, `apps/tree.c` o `apps/tagtree.c` pensada para la UI del core". Única excepción documentada: `mpeg_settings.c` usaba `rb->do_menu()` — eliminado en D-304/D-307 por ser "la única puerta trasera real a la UI de Rockbox", reemplazado por un menú propio.

**Qué se pierde si no se llama a mano a `default_event_handler`:** Aura **sí** sigue llamándolo desde su propio bucle — USB, apagado por batería y eventos `SYS_*` no se pierden, pero es responsabilidad del bucle de UI propio invocarlo en cada iteración.

**Evidencia (bucle propio):** `apps/aura/aura_main.c:376-403` (rama de espera larga, detecta `SYS_USB_CONNECTED`), `apps/aura/aura_main.c:633-700` (bucle principal: captura `SYS_POWEROFF` antes de que `default_event_handler()` corra `clean_shutdown()`, llama `default_event_handler(button)` para todo evento no consumido), `apps/aura/aura_main.c:167-236` (`next_button()`: envuelve `button_get`/`button_get_w_tmo`, filtra `BUTTON_REL` salvo LEFT/RIGHT, maneja "hold", captura `s_wheel_velocity` desde `button_get_data()`).

**Implicación para Metro-Aura:** el patrón "bucle propio que llama `default_event_handler()` en cada vuelta" es el mecanismo correcto a replicar. Un bucle de UI propio debe: (1) manejar `SYS_USB_CONNECTED`/`SYS_POWEROFF` a través de `default_event_handler()`, (2) filtrar `BUTTON_REL` salvo donde importe el "soltar" (bug real documentado: el `REL` de la pulsación que abrió una pantalla la cierra de inmediato si no se filtra), (3) capturar velocidad de rueda vía `button_get_data()` en el mismo ciclo que detecta `SCROLL_FWD`/`SCROLL_BACK`.

`[VERIFICADO]`

## A.2 — `gui_synclist`: callbacks, geometría y límites sin modificar `list.c`

**Respuesta:** `gui_synclist` es un widget basado en callbacks: `callback_get_item_name`, `callback_get_item_icon`, `callback_get_item_color` (solo `HAVE_LCD_COLOR`), y **`callback_draw_item`** — hook "owner-drawn" por fila que recibe un `struct list_putlineinfo_t` completo y puede dibujar libremente dentro de esa fila. Si es `NULL`, se usa `_default_listdraw_fn`.

**El límite real:** `line_height` es **un valor por lista y por pantalla**, calculado una sola vez en `gui_synclist_init()` — no hay alturas de fila distintas dentro de la misma lista (imposible tener la fila seleccionada con fuente gigante y las demás pequeñas sin forkear `list.c`/`bitmap/list.c`). No hay soporte nativo para movimiento horizontal, más de una lista visible a la vez, ni encabezado de pivots.

**Evidencia:** `apps/gui/list.h:142-193` (`line_height[NB_SCREENS]` único, `callback_draw_item`), `apps/gui/list.c:89` (ítems visibles = altura viewport / altura de línea fija), `apps/gui/list.c:118-122` (cálculo de `line_height` una sola vez), `apps/gui/bitmap/list.c:144-232` (`_default_listdraw_fn`, `linedes.height` fijo).

**Por qué Aura NO usa `gui_synclist`:** construyó su propio widget de lista desde cero (`aura_widgets_draw_list()` primero, `apps/aura/aura_menu_list.c` "MenuList v2" después) — nunca instancia `struct gui_synclist`. Incluso con `callback_draw_item` disponible, prefirió control total sobre la geometría (panel dividido izquierda/derecha, filas con switch/checkmark/ícono de 48px, windowing con selección centrada).

**Evidencia:** `Aura-Firmware/DECISIONS-ARCHIVE.md:784,918` (limitación documentada), `:381-387` (layout dividido), `:984-1004` (D-092/D-093 "MenuList v2"), `apps/aura/aura_menu_list.h:23-30`.

**Implicación para Metro-Aura:** para el esqueleto twist (listas verticales + panel de pivot horizontal, filas de tipografía variable, índice alfabético flotante), `gui_synclist` no alcanza aun con owner-draw. El patrón correcto (probado dos generaciones por Aura) es un widget de lista propio en C, reutilizando solo la idea de aceleración de rueda (C.2) y `viewport`/`clip_viewport_rect` como primitivas de bajo nivel.

`[VERIFICADO]`

## A.3 — Listas skinned (`list-skinned.c`, tags `%LT`/`%LI`/`%Lb`)

**Respuesta:** Catálogo de tags de lista: `%Lt` (título), `%LT[|IS]` (texto de ítem), `%LR`/`%LC`/`%LN` (fila/columna/número dentro del bucle), `%Li`/`%LI[|IS]` (íconos), `%Lb Sii|S` (config de fila), `%Lc` (¿seleccionado?, para `%?`), `%LB` (¿necesita scrollbar?) — dentro de un bloque `%Vp` (`SKIN_TOKEN_VIEWPORT_CUSTOMLIST`).

**Límites concretos:** sustituto declarativo de `_default_listdraw_fn` — reposiciona/restilyza contenido de fila pero sigue dentro del modelo de una sola lista, scroll solo vertical, altura de fila fija impuesta por `list.c`/`bitmap/list.c` por debajo. Sin tag de movimiento horizontal, sin noción de "pivot"/"twist", sin dos listas coexistiendo con transición entre ellas.

**Evidencia:** `lib/skin_parser/tag_table.c:74` (`%Vp` = `SKIN_TOKEN_VIEWPORT_CUSTOMLIST`), `:209-218` (tags `Lt/LT/LR/LC/LN/Li/LI/Lb/Lc/LB`), `apps/gui/bitmap/list-skinned.c` (renderiza fila por fila re-evaluando el árbol de tags).

**Implicación para Metro-Aura:** listas skinned sirven para reestilizar contenido de fila (look), no comportamiento (twist horizontal, pivots, doble lista) — confirma A.2: ese comportamiento va en C.

`[VERIFICADO]`

## A.4 — Catálogo de capacidades del skin engine

| Capacidad | Tag(s) | Notas |
|---|---|---|
| Fuentes múltiples cargadas por el tema | `%Fl` (`skin_parser.c:464`) | Array `skinfonts[MAXUSERFONTS]` (14 slots, ver A.7) |
| Viewports condicionales | `%Vd S`, `%Vl S[IP][IP][ip][ip]i`, `%if`/`%and`/`%or` | Lógica condicional real sobre valores dinámicos |
| Recorte/scroll de texto | Implícito vía `clip_viewport_rect` (A.6) + `%s` (`SUBLINE_SCROLL`) | Marquee activable por subline; recorte duro automático si no se declara `%s` |
| Alineación | `%ac`/`%al`/`%aL`/`%ar`/`%aR`/`%ax` | Centro/izquierda/derecha + RTL |
| Imágenes | `%x`, `%xl`/`%xd`, `%X` (backdrop), `%x9` | — |
| Barra de progreso | `%pb` (`BAR_PARAMS`) | Refresh especial `SKIN_REFRESH_PLAYER_PROGRESS` |
| Carátula | `%Cl`/`%Cd`/`%C` | Carga, muestra, condicional "¿se encontró?" |
| Colores por viewport | `%Vf`/`%Vb`, `%Vg` (gradiente), `%Vs` | Por viewport, no por elemento (salvo `%Lc` en listas) |
| Timers | `%t D\|d` (`SUBLINE_TIMEOUT`) | Cambia de subline tras N segundos — temporizador de **contenido**, no de animación (sin interpolación de posición/opacidad) |
| Buffer de skin | — | Tamaño no cuantificado en esta pasada — depende del `.wps`/`.sbs` activo. `[ESTIMADO]` |

**Implicación para Metro-Aura:** el skin engine **no tiene ningún primitivo de animación/transición temporal** — confirma que todo el sistema de transiciones tiene que ser C puro, igual que `aura_transitions.c`. El motor de skins sirve, en el mejor caso, para declarar el *layout estático*, nunca el movimiento.

`[VERIFICADO]` (capacidades) / `[ESTIMADO]` (tamaño de `skin_buffer`)

## A.5 — Matriz "tema vs. C" para los patrones Metro

| Patrón Metro | ¿Tema? | Archivo C si no | Razón |
|---|---|---|---|
| Tipografía gigante recortada en el borde derecho | Parcial (posición vía viewport) | — | El recorte es automático (`clip_viewport_rect`, A.6); el límite real es el tamaño del buffer de fuente (A.7) |
| Encabezado de pivots (activo blanco, resto gris, desplazado) | No | C nuevo | Sin equivalente en tags de skin |
| Lista vertical con selección en posición fija | Parcial | Widget C propio | `gui_synclist`/listas skinned dan scroll vertical con selección fija, pero altura uniforme y sin twist |
| Tiles en cuadrícula | No | C nuevo | Sin noción de grilla 2D con selección direccional |
| Fondo con carátula desenfocada/atenuada | Parcial (atenuar sí; desenfocar no) | C (blend manual) | Sin tag de "atenuar imagen"; blur real demasiado caro (ver B) |
| Barra de estado mínima | Sí | — | Declarable en `.sbs` o reemplazable en C |
| Transición slide | No | C nuevo | Sin primitivas de animación en el skin engine |
| Transición turnstile | No | C nuevo, referencia PictureFlow | Requiere proyección por columna |
| Contador/progreso en Now Playing | Sí (`%pb`) | — | Versión estilizada (píldora redondeada) exige C |

`[VERIFICADO]`

## A.6 — Viewports y recorte

**Respuesta:** El recorte es automático vía `clip_viewport_rect()` (`firmware/drivers/lcd-bitmap-common.c:132`), invocada al inicio de cada primitiva de dibujo — un `lcd_putsxyofs`/`lcd_bitmap_part` con coordenadas fuera del viewport se recorta a la intersección, sin `panicf`. `viewport_set_defaults()` no valida rangos de `x`/`y`/`width`/`height` contra el LCD físico. Texto con `x` negativo: se ajusta origen/ancho a la porción visible, el glifo se dibuja recortado.

**Evidencia:** `firmware/drivers/lcd-bitmap-common.c:132`, `firmware/drivers/lcd-16bit-common.c:49,137,227,444,454`, `apps/gui/viewport.c:183,196` (los únicos `panicf` son por stack de viewportmanager lleno/vacío, no geometría), `apps/gui/viewport.c:327-344`.

**Implicación para Metro-Aura:** el "texto que se corta en el borde" (lenguaje Metro) es gratis a nivel de primitiva — solo dibujar con un viewport más angosto que el texto.

`[VERIFICADO]`

## A.7 — Fuentes: capacidad, tamaño, medición

**Respuesta:** `MAXUSERFONTS = 14` (subido por Aura desde 12, comentario "Aura llegó al límite exacto de 12"). `MAXFONTS = FONT_FIRSTUSERFONT + MAXUSERFONTS = 14`.

**Hallazgo crítico:** `MAX_FONT_SIZE` se define en `firmware/font.c:62-70` según `LCD_HEIGHT`/`MEMORYSIZE`. **`ipod6g.h` nunca define `MEMORYSIZE`** (confirmado por grep — ausente también en los `autoconf.h` generados) → `MEMORYSIZE` vale `0` en preprocesador → con `LCD_HEIGHT=240>64` verdadero pero `MEMORYSIZE(0)>2` falso, **`MAX_FONT_SIZE = 10000 bytes`** (no 60000). Además `FONT_HARD_LIMIT` se activa cuando `MEMORYSIZE<4` — con `MEMORYSIZE=0` también verdadero, así que **el límite de 10 KB es un tope duro e incondicional** (`font_load_ex()` re-clampa `bufsize` a `MAX_FONT_SIZE` sin importar qué pida el llamador).

**Qué pasa si un `.fnt` pesa más de 10 KB:** no falla — `font_load_ex()` compara `bufsize < file_size` y activa `cached = true`: modo **caché de glifos bajo demanda** (`GLYPHS_TO_CACHE = 256` glifos en rotación LRU dentro del mismo presupuesto de 10 KB) — mismo patrón de "lectura de disco por evento" que causó el costo alto del morph de Now Playing (ver B.10).

**Costo estimado de una fuente 60-72px AA:** con el tope de 10 KB, un alfabeto latino completo a 60-72px AA casi seguro no entra precargado — solo dígitos + un puñado de símbolos caben; un alfabeto completo operará en modo caché-bajo-demanda.

**Evidencia:** `firmware/export/font.h:51-64` (D-263/D-267, `MAXUSERFONTS 14`), `firmware/font.c:62-76` (`MAX_FONT_SIZE`, `FONT_HARD_LIMIT`), `firmware/font.c:440-475` (`font_load_ex`: clamp, `cached = (bufsize < file_size)`).

**Implicación para Metro-Aura — el hallazgo más importante de la sección A para la tipografía gigante (ver F.6/F.7):** no es un problema de RAM total (64 MB disponibles) sino de un tope duro de Rockbox de 10 KB por fuente cargada en este target. Estrategia: (a) subconjuntos de caracteres muy acotados para tamaños "gigantes" (dígitos, dos puntos, AM/PM — nunca alfabeto completo a 72px AA), o (b) aceptar modo caché-bajo-demanda para alfabetos completos grandes y medir en hardware real el costo de fallo de caché durante scroll, o (c) investigar si redefinir `MEMORYSIZE` para `ipod6g` es viable sin romper otras partes de Rockbox que dependen de esa macro en otros contextos (`firmware/export/config.h:964,1059,1154,1158`) — no es un cambio trivial de una línea, tiene efectos colaterales por auditar.

`[VERIFICADO]` (con errata, ver abajo)

> **ERRATA A.7 (añadida en Fase 3, 2026-08-20, verificada en código):** la conclusión central de este hallazgo es **incorrecta**. `MEMORYSIZE` **sí** está definido para ipod6g: no lo define `ipod6g.h`, lo inyecta `tools/configure` en el `Makefile` generado (`Aura-Firmware/firmware/build-ipod6g/Makefile:27` y `build-sim/Makefile:27`: `export MEMORYSIZE=64`, pasado como `-DMEMORYSIZE=64`). Por lo tanto `MAX_FONT_SIZE = 60000` (no 10 000), y `FONT_HARD_LIMIT` **no** se define (`MEMORYSIZE < 4` es falso). Además `font_load_ex(path, buf_size, glyphs)` (`firmware/font.c:454-475`) dimensiona el buffer como `glyphs × bytes_por_glifo` cuando `glyphs > 0` — si ese tamaño ≥ tamaño del archivo, la fuente se carga **completa** desde buflib sin caché; solo `font_load()` a secas (= `font_load_ex(path, 60000, 256)`) cae en modo caché para fuentes grandes. Métricas reales de los `.fnt` de F.6: Selawik Light 64px = maxwidth 60, height 65, 1 950 B/glifo, archivo 284 KB; 42px = 39/43, 839 B/glifo, 148 KB; 24px = 22/25, 61 KB. **Implicación corregida:** la tipografía gigante NO está limitada a 10 KB — Metro carga sus fuentes de display con `font_load_ex(path, 0, N ≥ glifos_de_la_fuente)` y paga ~200-300 KB de buflib por fuente grande (irrelevante sobre 64 MB). El riesgo real es solo el número de slots (`MAXUSERFONTS`, 12 en upstream) y el tiempo de carga desde disco al arrancar. La decisión G.3 se re-resuelve en `PLAN_MAESTRO.md`.

## A.8 — Barra de estado y backdrop; qué sigue dibujando Rockbox

**Respuesta:** Aura apaga la barra de estado clásica y el backdrop con dos asignaciones directas: `global_settings.statusbar = STATUSBAR_OFF` y `global_settings.backdrop_file[0] = '-'`. El resto de superficies que se "cuelan" están catalogadas en `docs/superficies-rockbox.md` (4 niveles de riesgo): splash genérico (re-vestido, no eliminado — decenas de sitios lo llaman sin control individual), pantalla USB, mensaje de apagado, texto del bootloader, menú/OSD de mpegplayer, "Scanning disk..."/"Committing database" del tagcache.

**Evidencia:** `apps/main.c:507-509`, `Aura-Firmware/docs/superficies-rockbox.md:1-40`, `MODIFICATIONS.md` (`splash.c`, `usb_screen.c` en la lista de 27 modificados).

**Implicación para Metro-Aura:** "apagar lo que se puede apagar por setting + re-vestir genéricamente lo que no se puede interceptar por completo" es el camino correcto — eliminar cada superficie por completo exigiría tocar decenas de sitios dispersos con alto riesgo de romper comportamiento de bajo nivel.

`[VERIFICADO]`

## A.9 — Ajustes: reutilizar `settings_list.c`+`do_menu()` vs. reescribir

**Respuesta:** Aura reescribió por completo las pantallas de Ajustes en C propio, nunca reutiliza `do_menu()`/`settings_list.c`. Se infiere de la regla de proyecto de A.1 y del caso corregido (`mpeg_settings.c` usando `rb->do_menu()`, calificado "puerta trasera real" y eliminado).

**Cadenas de idioma:** Aura no carga el sistema `.lang` de Rockbox (D-013) — `apps/aura/aura_lang.c` es una tabla de cadenas propia en español, indexada por enum propio, append-only.

**Evidencia:** `Aura-Firmware/DECISIONS-ARCHIVE.md:349`, `DECISIONS.md:314` (menú nativo de mpegplayer en inglés puro porque usa `rb->str(LANG_X)` y Aura nunca carga `.lang`), `DECISIONS.md:367-370`, `apps/aura/aura_lang.c`.

**Implicación para Metro-Aura:** el mecanismo (tabla de cadenas C propia, sin `.lang`) es reutilizable si Metro conserva español; si decide inglés (más fiel al Zune original) o bilingüe, se reescribe la tabla pero el mecanismo se mantiene.

`[VERIFICADO]` (mecanismo) / `[ESTIMADO]` (razón exacta de la elección, nunca documentada como D-NNN dedicada)

## A.10 — Memoria disponible para UI

**Respuesta:** `PLUGIN_BUFFER_SIZE = 0x200000` (2 MiB) — buffer para un plugin cargado, no para el core. Para el core, la memoria se reserva vía `core_alloc`/`buflib_mempool` de un pool común que también usa el audio. No se encontró una constante única de "RAM libre para UI del core". La pregunta de dónde reserva `aura_transitions.c` sus buffers extra se resuelve en la sección B (ver B.3): son arreglos **estáticos en BSS**, 150 KB cada uno, no dinámicos.

**Evidencia:** `firmware/export/config/ipod6g.h:153` (`PLUGIN_BUFFER_SIZE`).

**Implicación para Metro-Aura:** 2 MiB confirmado para plugins. Para el core, la disponibilidad real de RAM permanente debe medirse empíricamente antes de comprometerse a un diseño.

`[VERIFICADO]` (`PLUGIN_BUFFER_SIZE`) / `NO RESUELTO: presupuesto exacto de RAM del core para UI permanente — recomendación: medir en Fase 4 pidiendo 2-3 buffers de 150 KB permanentes y verificar que no compiten con el buffer de audio.`

## A.11 — Íconos/bitmaps: pipeline y costo de lectura por cuadro

**Respuesta:** Pipeline de compilación: `apps/bitmaps/` (BMP nativo) → `bmp2rb` → símbolo C embebido vía `apps/bitmaps/bitmaps.make`. Aparte, Aura carga íconos de tema **en runtime desde disco** (`aura_style_read_icon_bmp()`) — el perfil de rendimiento documentado del morph de Now Playing confirma que cada ícono (~13-15 por cuadro) se lee y decodifica de disco **en cada cuadro**: ~250-285 aperturas de archivo en 330 ms, identificado como el sospechoso dominante de la lentitud (ausencia de caché de íconos en RAM).

**Evidencia:** `Aura-Firmware/DECISIONS.md:461` (perfil completo).

**Implicación para Metro-Aura — regla de oro directa:** nunca leer bitmaps/íconos de disco dentro de un bucle de animación por cuadro; cualquier ícono que participe en una transición debe estar decodificado y cacheado en RAM *antes* de que arranque la animación.

`[VERIFICADO]`

## A.12 — Plugins vs. core

**Respuesta:** Ejecutar como plugin da acceso al `PLUGIN_BUFFER_SIZE` completo (2 MiB) como memoria dedicada exclusiva — ventaja real frente al core, que compite por RAM permanentemente. Contrapartida: API limitada `rb->`, riesgo de "puerta trasera" a UI nativa si no se re-viste con cuidado (caso real: `mpeg_settings.c`).

Aura lanza `mpegplayer`/`imageviewer` vía `plugin_load()` normal más `plugin_set_silent_open_errors()` (D-298, uno de los 27 modificados) — silencia los `splash()` nativos de dos ramas de error de `plugin_load()`, opt-in por llamador, sin efecto en el resto de Rockbox por defecto.

**Evidencia:** `Aura-Firmware/MODIFICATIONS.md` (`apps/plugin.c`/`.h`, D-298), `firmware/export/config/ipod6g.h:153`.

**Implicación para Metro-Aura:** para efectos visuales pesados y aislados (pantalla tipo Cover Flow/turnstile a pantalla completa), correr como plugin es una opción real con presupuesto de memoria generoso y aislado — a valorar en Fase 3 frente al costo de tenerla fuera del árbol principal de navegación.

`[VERIFICADO]` (mecanismo) / `[ESTIMADO]` (recomendación de uso)

---

# B. Animaciones y transiciones (LCD del ipod6g, primitivas, PictureFlow, costo)

## B.1 — Ruta completa de `lcd_update()` en 6G

**Respuesta:** `lcd_update()` es un alias de `lcd_update_rect(0, 0, LCD_WIDTH, LCD_HEIGHT)`. Ruta real: (1) `displaylcd_wait_dma()` — espera (con `yield()` en el lazo) a que termine la transferencia DMA anterior si la había; (2) `displaylcd_setup()` — programa la ventana de escritura del controlador (8080-paralelo); (3) **copia por CPU** (`memcpy`) del framebuffer lógico (`FBADDR`) a un buffer intermedio dedicado a DMA, `lcd_dblbuf` (150 KB, `CACHEALIGN_ATTR`); (4) `displaylcd_dma()` — encola la transferencia real vía DMA (PL080) y **retorna sin esperar**. Es decir: bloqueante solo respecto de la transferencia DMA *anterior*; el costo de CPU real y medible es el `memcpy` del paso 3, no la transferencia (paralela por hardware).

**Evidencia:** `firmware/target/arm/s5l8702/lcd-s5l8702.c:323-326` (`lcd_update`), `:380-435` (`lcd_update_rect`, memcpy condicional completo vs. por fila), `:362-376` (`displaylcd_dma`/`wait_dma`).

**Implicación para Metro-Aura:** la puerta real de rendimiento es el `memcpy` de hasta 150 KB por cuadro completo, no la espera de pantalla — un `lcd_update_rect` parcial es proporcionalmente más barato en la copia.

`[VERIFICADO]` (mecanismo) / `NO RESUELTO: tiempo exacto en ms de un lcd_update() completo en hardware real — recomendación: instrumentar con USEC_TIMER (B.6) alrededor de displaylcd_dma()/wait_dma() como primer experimento de Fase 4.`

## B.2 — FPS realista y presupuesto por cuadro

**Respuesta:** No existe cifra publicada verificable de `test_fps` específica para ipod6g/iPod Classic. Único dato relevante encontrado: en iPod 5G, PictureFlow "alcanza solo 8-10 cuadros" con fallos de carga de carátulas, mientras "en dispositivos iPod Classic funciona mucho mejor" (sin cifra exacta) — diferencia atribuible al SoC más rápido del 6G (S5L8702 216 MHz vs. PortalPlayer del 5G). `[ESTIMADO]`, sin cifra dura para 6G.

**Presupuesto derivado** (razonamiento propio): con el techo de 330 ms que Aura ya usó como presupuesto nominal para su morph (B.10) y el objetivo de 20-30 fps del proyecto, presupuesto por cuadro **33 ms (30 fps) a 50 ms (20 fps)**.

**Evidencia:** búsqueda web (resultado sobre iPod 5G vs. Classic con PictureFlow); `Aura-Firmware/DECISIONS.md:262`.

**Implicación para Metro-Aura:** diseñar con 33-50 ms/cuadro como objetivo de trabajo, tratado como hipótesis a validar en hardware real antes de comprometerse — razón directa del toggle + fallback pedido por el proyecto.

`[ESTIMADO]` / `NO RESUELTO: cifra dura de FPS en hardware real para ipod6g — pendiente de medición directa en Fase 4 (ver B.11).`

## B.3 — Framebuffer: layout, buffer propio de viewport, costo de memcpy/blit

**Respuesta:** `LCD_STRIDEFORMAT` hereda el default de `config.h` (`HORIZONTAL_STRIDE`) — framebuffer row-major estándar, RGB565 nativo.

**Buffer propio por viewport — sí soportado y ya en uso:** `viewport_set_buffer(vp, buffer, screen)` asigna un `struct frame_buffer_t` arbitrario; si `buffer` es `NULL` usa el framebuffer principal. Aura ya lo explota: `s_push_fb`/`s_outgoing_fb`, dos arreglos **estáticos** de 320×240×2 bytes = **150 KB cada uno**, reservados en BSS (no heap ni `core_alloc`/`buflib`) — responde A.10: son estáticos de por vida del programa.

**Costo de memcpy/blit:** un `memmove` por fila (stride horizontal garantizado) es la operación de "desplazar columnas" barata que sostiene el slide horizontal; `lcd_bitmap_part()` con offset pinta la franja entrante.

**Evidencia:** `firmware/export/config.h:816-818`, `apps/gui/viewport.c:313-325` (`viewport_set_buffer`), `apps/aura/aura_transitions.c:98-112` (`s_push_fb`), `:131` (`s_outgoing_fb`), `:75-90` (comentario: "la pantalla nueva se renderiza UNA vez a un framebuffer offscreen propio... cada cuadro es solo desplazar con memmove por fila y blitear la franja entrante con lcd_bitmap_part").

**Implicación para Metro-Aura:** "prerrenderizar destino UNA vez a un framebuffer offscreen estático de 150 KB + animar solo composición/blit por cuadro" es el mecanismo correcto y probado — Metro puede reservar 2-3 buffers de 150 KB (450 KB total, insignificante sobre 64 MB) como arreglos estáticos.

`[VERIFICADO]`

## B.4 — Primitivas disponibles y costos relativos

| Primitiva | Disponible | Costo relativo |
|---|---|---|
| `lcd_fillrect` | Sí | Barato |
| `lcd_hline`/`lcd_vline` | Sí | Barato |
| `lcd_bitmap_part` | Sí | Barato — sin blend |
| `lcd_bitmap_transparent_part` | Sí | Medio — hueco por color clave, sin alpha real |
| `lcd_alpha_bitmap_part`/`_mix` | Sí | Medio — diseñada para glifos AA, no blend arbitrario |
| `lcd_gradient_fillrect`/`_part` | Sí | Barato-medio |
| Blend alfa RGB565 genérico entre dos bitmaps arbitrarios | **No existe en el core** | — |

**Hallazgo clave:** sin primitivo de Rockbox para atenuar/mezclar imágenes con alfa arbitrario. Aura escribió `a26_shell_blend(from, to, alpha_256)`: 3 restas + 3 multiplicaciones + 3 divisiones por 256 (constante potencia de 2 → shift, sin costo real de división) por píxel.

**Evidencia:** `firmware/drivers/lcd-16bit-common.c:129,436`, `firmware/export/lcd.h:602-604,635`, `apps/aura/apple2026_shell.c:665-679` (`a26_shell_blend`), uso en `apps/aura/aura_transitions.c:423` (blend por píxel en bucle x/y completo dentro de la animación de push).

**Implicación para Metro-Aura:** fundido/atenuación de carátula = viable pero es trabajo de CPU por píxel escrito a mano; confirma que "difuminar" (blur real) no tiene primitivo de apoyo y su costo sería mucho mayor (F.4 ya clasifica blur como "no hacer").

`[VERIFICADO]`

## B.5 — Técnicas de PictureFlow a replicar

1. **Punto fijo con tablas precalculadas**: `#define PFreal long`; `sin_tab`/`fsin()`/`fcos()` son tablas trigonométricas con interpolación lineal, nunca FPU (el S5L8702 no tiene FPU). `fmul`/`fdiv` son helpers de punto fijo, con variantes explícitamente optimizadas por el autor original.
2. **Dibujo por columnas, sin división por píxel**: `render_slide()` (línea 3013) recorre `for (x = xi; x < w; x++)` — una columna a la vez, con proyección en punto fijo por columna y acumulación de paso fijo (`p -= dy`) verticalmente — el único costo por píxel es un shift y una comparación.
3. **Tabla de reflejo precalculada**: `reflect_table[REFLECT_HEIGHT]` se calcula una sola vez fuera del bucle de cuadro.
4. **Caché de carátulas escaladas**: `aa_cache` con `incremental_albumart_cache()` construye el caché en cuadros de "idle".
5. **`cpu_boost`**: `rb->cpu_boost(true)` se activa **una sola vez al entrar** al bucle principal (comentario "revert in cleanup") y se revierte solo al salir del plugin — bostea toda la sesión interactiva, no por transición individual.
6. **`yield()`/lectura de botón no bloqueante durante animación**: `rb->yield()` tras cada `lcd_update()`; `rb->get_custom_action(..., instant_update ? 0 : HZ/16, ...)` — timeout 0 durante animación activa, HZ/16 (~6 ms) en reposo — interrupción inmediata por botón nuevo.

**Evidencia:** `apps/plugins/pictureflow/pictureflow.c` líneas 234, 709-840, 859-878, 3013-3086, 525, 3032-3037, ~4726, 4469, 3493, 4771-4772 — todas leídas directamente.

**Implicación para Metro-Aura:** el patrón "boost de sesión completa" (PictureFlow) es distinto al de Aura (que nunca lo usa, ver B.10) — decisión de Fase 3 según cuánto tiempo pase en pantallas animadas.

`[VERIFICADO]`

## B.6 — Reloj de animación

**Respuesta:** Dos temporizadores distintos: (1) `current_tick`, resolución `HZ` (heredado del default genérico de Rockbox, típicamente 100 = 10 ms — `[ESTIMADO]` valor exacto, no confirmado línea por línea para este target); (2) `USEC_TIMER`, temporizador real de microsegundos, confirmado en `system-target.h:53-54`, usado hoy solo para `udelay()` de bajo nivel, **no** para animación de UI.

`aura_motion.c` (easings puros) recibe `elapsed`/`duration` como parámetros genéricos — agnóstico de la fuente de tiempo. Pero `aura_transitions.c` **no** usa tiempo real transcurrido: usa **conteo de cuadros fijo** (`for i in 1..frames`) con `sleep(frame_delay)` entre cuadros — "N cuadros a un delay fijo", no interpolación contra tiempo real. La tabla de easing (`spring_table`, 24 pasos) se generó offline ("no hay FPU").

**Evidencia:** `firmware/target/arm/s5l8702/system-target.h:53-54`, `apps/aura/aura_motion.c:24-94`.

**Implicación para Metro-Aura:** `USEC_TIMER` disponible si se quisieran easings basados en tiempo real transcurrido; el patrón de Aura (cuadros fijos a delay fijo, escalado por nivel de FX) es más predecible para presupuestar CPU y es el recomendado por defecto.

`[VERIFICADO]`

## B.7 — Hilos y audio

**Respuesta:** La UI corre en el hilo principal junto con el resto de la app; el audio corre en hilo/contexto separado que depende de que el hilo principal ceda CPU regularmente. Bug real documentado (D-074): una secuencia de transiciones encadenadas sin drenar la cola de botones desbordaba la cola del kernel (`KERNEL_ASSERT "queue_post ovf"`) — corrección: `drain_button_queue_if_full()` en cada cuadro. No hace falta atender el botón durante la animación, pero sí impedir que la cola rebalse.

**Evidencia:** `apps/aura/aura_transitions.c:59-70` (comentario D-074 completo), `:70-73`, uso en `:449,496`.

**Implicación para Metro-Aura:** replicar `sleep()` entre cuadros (nunca omitirlo) + `drain_button_queue_if_full()` en cualquier bucle de animación encadenable — lección de un bug real de producción.

`[VERIFICADO]`

## B.8 — Batería/CPU: costo de `cpu_boost`

**Respuesta:** `CPUFREQ_MAX = 216 000 000` Hz, `CPUFREQ_NORMAL = 54 000 000` Hz — **`cpu_boost(true)` es un factor de 4×**. `cpu_boost()` es un contador con lock (`boost_counter`), no un booleano — múltiples llamadores pueden pedir boost sin pisarse.

**Evidencia:** `firmware/target/arm/s5l8702/system-target.h:30-32`, `firmware/export/config/ipod6g.h:200` (`CPU_FREQ 216000000`), `firmware/system.c:89-130`.

**Implicación para Metro-Aura:** factor de 4× es sustancial — boostear indiscriminadamente (sesión completa, estilo PictureFlow) tiene costo de batería real en un reproductor. Recomendación: boost por transición individual (activar al empezar el bucle de cuadros, desactivar al terminar), más cercano al patrón de Aura (que de hecho no lo usa nunca, B.10).

`[VERIFICADO]`

## B.9 — `lcd_active()`/backlight: garantías para animación

**Respuesta:** `lcd_active()` devuelve el flag interno `lcd_ispowered` — el mismo que `lcd_update_rect()` consulta internamente (si el LCD está dormido, `lcd_update_rect()` ya es no-op). Pero el guard de más alto nivel que usa Aura en cada función de transición (`!lcd_active() || aura_settings.animation_mode == AURA_ANIM_NONE`, 9 sitios) existe para evitar el trabajo de **dibujo** (CPU) que precede a `lcd_update()`, no solo la escritura física.

**Evidencia:** `firmware/target/arm/s5l8702/lcd-s5l8702.c:411-413,487-489`, 9 sitios de guard en `apps/aura/`.

**Implicación para Metro-Aura:** replicar el mismo guard al inicio de cualquier función de transición es obligatorio por corrección (no animar algo invisible) y eficiencia (ahorra CPU de dibujo, no solo escritura).

`[VERIFICADO]`

## B.10 — Lecciones de Aura: catálogo de transiciones, lentitud real del morph, sistema de niveles

**Catálogo de `aura_transitions.c`** (1462 líneas): `aura_transition_shift_and_reveal()` ("Push real", dos pantallas se mueven en bloque, ease-out cuadrático, con fase extra de "lift" de la barra de estado), transición de Music Flow enter/return con "revelado detrás de paneles" vía CoverDrift, `flow-to-player`/`flow-return` (con texturas de fondo). Todas comparten: captura de "lo que hay en pantalla" a buffer estático, destino prerrenderizado una vez a otro buffer estático (`viewport_set_buffer`), composición por cuadro con blend/blit, instrumentación `TRANSITION_LOG`/`DEBUGF` (sin costo en producción).

**Caso documentado de lentitud real (`mode4_morph()`, Now Playing)** — perfil citado de `DECISIONS.md:461`: reconstruye la pantalla completa desde cero en cada uno de ~19 cuadros; **sospechoso dominante: ausencia de caché de íconos en RAM** — cada ícono (13-15 por cuadro) se lee/decodifica de disco en cada cuadro, ~250-285 aperturas de archivo en 330 ms; sospechosos secundarios: divisiones enteras por píxel en el tinte del panel derecho (~45 600/cuadro, ARM sin divisor por hardware), reproyección de carátula con ~160 `lcd_bitmap()` de 1 px de ancho por cuadro, remaquetado de texto invariante recalculado cada cuadro.

**Sistema de niveles (matriz de 2 ejes, no toggle binario):** `AURA_ANIM_NONE`/`MINIMAL`/`ALL` (Animaciones — el *cómo* del movimiento) **cruzado con** un eje de Gráficos (el *qué* se dibuja). Principio rector: "el canon es la versión de máxima fidelidad... los niveles de abajo son **sustracciones** sobre ese canon, nunca redefiniciones — un solo código con puntos de sustracción" — con regla de precedencia: "Gráficos decide QUÉ existe; Animaciones decide CÓMO se mueve lo que existe". Ejemplo de valores: Push-and-Drop, 8 cuadros/60 fps en "Todas" vs. 4 cuadros/45 fps en "Mínimas".

**Hallazgo adicional:** `aura_transitions.c` **nunca llama `cpu_boost()`** (grep completo, cero resultados) — contraste directo con PictureFlow (B.5/B.8), que sí lo hace durante toda su sesión.

**Evidencia:** líneas ya citadas en B.3/B.5, `Aura-Firmware/DECISIONS.md:461`, `docs/aura-design-system/sistema/06-niveles-de-fx.md:1-60`, `apps/aura/apple2026_tokens.h:186-189`.

**Implicación para Metro-Aura — reglas de oro:**
1. Nunca leer/decodificar bitmaps de disco dentro de un bucle de animación por cuadro.
2. Nunca reconstruir la pantalla completa desde cero por cuadro si se puede prerrenderizar una vez y componer.
3. Evitar división entera por píxel en bucles calientes — punto fijo con tablas, o multiplicación+shift.
4. Sistema de niveles de FX como matriz de 2 ejes (movimiento × contenido) desde el principio.
5. Decidir explícitamente por tipo de animación si vale la pena `cpu_boost` (sesión larga) o mantenerse a frecuencia normal (transición puntual).
6. Instrumentación `DEBUGF`/tick-based sin costo en producción, mecanismo correcto para decidir con datos reales.

`[VERIFICADO]`

## B.11 — Simulador vs. hardware: método de medición

**Respuesta:** Confirmado por Aura mismo: "en el simulador (hardware rápido, no representativo del ARM real) el tiempo total ya estaba cerca del presupuesto nominal — confirma que el simulador no sirve para medir el costo real por cuadro, solo corrección visual".

**Método recomendado, dos precedentes ya en el árbol:** (1) `TRANSITION_LOG`/`DEBUGF` con ticks (patrón Aura, cero costo en producción, activo solo en builds DEBUG); (2) overlay de FPS en vivo (patrón PictureFlow, contador `frames`/`update_interval`, texto dibujado sobre la pantalla).

**Criterio de fallback propuesto:** comparar el tiempo medido (siempre en hardware real, nunca en simulador) contra el presupuesto de B.2 (33/50 ms); si se excede de forma sostenida, la transición cae al siguiente nivel de la matriz de B.10 — misma mecánica de sustracción que Aura ya implementó.

**Evidencia:** `Aura-Firmware/DECISIONS.md:262`, `apps/plugins/pictureflow/pictureflow.c:4682-4741`.

**Implicación para Metro-Aura:** la Fase 4 puede portar directamente `TRANSITION_LOG`/`DEBUGF` como primer paso antes de escribir cualquier transición nueva.

`[VERIFICADO]`

---

# C. Input: clickwheel → eventos → UI, y mapeo del twist

## C.1 — Flujo completo de un movimiento de rueda en 6G

**Respuesta:** El clickwheel del 6G se lee por interrupción. `INT_WHEEL()` (ISR) llama a `ipod_4g_button_read()`, que decodifica el registro `WHEELRX`: bits de botones y, si el bit `0x40000000` está activo, un valor absoluto de posición de rueda (0-0x5F, `WHEELCLICKS_PER_ROTATION=96`). La ISR calcula `wheel_delta` contra la posición anterior (con manejo de wraparound), y solo genera evento si `|wheel_delta| >= WHEEL_SENSITIVITY` (4 clics). Si hay evento, calcula **velocidad real en grados/segundo** (`USEC_TIMER`) y hace `button_queue_post(BUTTON_SCROLL_FWD|BACK [|REPEAT], data)` — con `HAVE_SCROLLWHEEL` (caso ipod6g), `data` empaqueta `(1<<31) | (1<<24) | wheel_velocity` (bit 31 = usar aceleración, bits 30:24 = mensajes saltados, bits 23:0 = velocidad grados/seg). Esto ocurre **directamente en la ISR**, no en `button.c`.

`button_tick()` (hilo de kernel, cada tick `HZ`) filtra por debounce y aplica su propia máquina de repetición genérica **por encima** de la que ya viene con `BUTTON_REPEAT` desde la ISR — dos mecanismos de repetición superpuestos. `action.c:get_action()` resuelve `ACTION_STD_NEXT`/`NEXTREPEAT` vía `keymap-ipod.c`, y `apps/gui/list.c:gui_synclist_do_button()` traduce con `button_apply_acceleration(get_action_data())`.

**Evidencia:** `firmware/target/arm/ipod/button-clickwheel.c:110-298` (`ipod_4g_button_read`), `:358-368` (`INT_WHEEL`), `firmware/drivers/button.c:436-493` (`button_read_device`), `:199-414` (`button_tick`), `apps/action.c:1181-1267`, `apps/gui/list.c:612`.

**Implicación para Metro-Aura:** la velocidad real (grados/seg) ya viene calculada desde la ISR — cualquier UI, propia o de Rockbox, accede a ella capturando `button_get_data()`/`get_action_data()` en el mismo ciclo en que detecta `SCROLL_FWD`/`BACK`. No hace falta reimplementar el cálculo de velocidad.

`[VERIFICADO]`

## C.2 — Aceleración: `WHEEL_ACCEL_START`/`WHEEL_ACCELERATION`

**Respuesta:** `WHEEL_ACCEL_START=270` (grados/seg, umbral) y `WHEEL_ACCELERATION=3` (curva de exponente 4: `v = (v⁴ + redondeo) >> 16`, la más agresiva de tres opciones). El multiplicador final es `delta * v` (`delta` siempre 1 en la ISR de ipod6g). Ocurre en `button_apply_acceleration()`, invocado **explícitamente desde `apps/gui/list.c:gui_synclist_do_button()`** — la aceleración **no es automática** para cualquier consumidor de botones: un widget de lista propio debe llamarla él mismo o replicar su fórmula.

En `list.c` el resultado se usa como `next_item_modifier` — ítems que avanza la selección por evento.

**Evidencia:** `firmware/export/config/ipod6g.h:73-74`, `firmware/drivers/button.c:618-660`, `apps/gui/list.c:610-612` (única llamada localizada fuera de `debug_menu.c`).

**Implicación para Metro-Aura:** la aceleración de fábrica ya está calibrada con datos reales de hardware — base sólida y barata de reusar; una curva propia (para el "índice alfabético flotante" del twist Zune) exige solo interceptar `wheel_velocity` antes de que `button_apply_acceleration()` la transforme.

`[VERIFICADO]`

## C.3 — Botones: pulsación corta/larga, combinaciones reservadas, `BUTTON_REL`

**Respuesta:**
- **Corta vs. larga:** no hay distinción de duración a nivel de driver — solo "presionado"/`BUTTON_REL` (soltado). La distinción es responsabilidad de `button.c`/`action.c`: tras `REPEAT_START` (300 ms) sostenido sin soltar, se emite `BUTTON_REPEAT` cada 160→50 ms (acelerando). `keymap-ipod.c` usa esto: `BUTTON_MENU|BUTTON_REL` = `ACTION_STD_MENU` (corta) vs. `BUTTON_MENU|BUTTON_REPEAT` = `ACTION_STD_QUICKSCREEN` (sostenido).
- **`button_hold`:** manejado en la capa de driver — mientras está activo, el sensor de rueda se **apaga por hardware**; la UI nunca ve eventos durante el hold, sin comprobación explícita en capas superiores.
- **Combinaciones reservadas por software (confirmadas en `keymap-ipod.c`):** `SELECT+PLAY` → `ACTION_TREE_HOTKEY`/`ACTION_WPS_HOTKEY`; `MENU+REPEAT` → `ACTION_STD_QUICKSCREEN`; `SELECT+REPEAT` → `ACTION_STD_CONTEXT`; `PLAY+REPEAT` → `ACTION_STD_CANCEL`. Ninguna reservada a nivel hardware/driver — todas reasignables cambiando el keymap.
- **Reset físico (`SELECT+MENU` ~6-8s):** no encontrado en el código — comportamiento del PMU/hardware del iPod, fuera del alcance del firmware. `NO RESUELTO: conocimiento de hardware del iPod real, no verificado en el árbol — [ESTIMADO].`
- **`BUTTON_REL`:** Aura lo filtra a mano (ver A.1). `action.c` ya lo maneja distinto: patrón "prereq button" en las tablas de keymap (`{ACTION_STD_OK, BUTTON_SELECT|BUTTON_REL, BUTTON_SELECT}`) — descarta automáticamente `REL` sueltos que no siguen a una entrada anterior propia.

**Evidencia:** `firmware/drivers/button.c:70-84,220-242`, `firmware/target/arm/ipod/button-clickwheel.c:410-480` (`button_hold()`), `apps/keymaps/keymap-ipod.c:43-92`, `apps/action.c:370-374,435-462`.

**Implicación para Metro-Aura:** si Metro adopta `get_action()`/`action.c` (ver C.4) en vez de lectura cruda, el filtrado de `BUTTON_REL` que Aura escribió a mano **ya viene resuelto** por el mecanismo de "prereq button".

`[VERIFICADO]` (salvo el combo de reset físico, `[ESTIMADO]`)

## C.4 — `get_action()` (con contextos) vs. lectura cruda de botones

**Respuesta:**

**(a) `get_action()`/contextos:** Rockbox encadena contextos custom con `LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_X)` — cada tabla puede "caer" a otra si no encuentra coincidencia. `action.h` define el espacio de nombres, no una lista cerrada — agregar un contexto nuevo exige una tabla `button_mapping[]` nueva y un valor propio (patrón `CONTEXT_CUSTOM|<algo>`), sin tocar el núcleo de `action.c`.

**(b) Lectura cruda (`button_get()`/`button_get_w_tmo()`), como Aura:** control total, a costa de reimplementar lo que `action.c` resuelve gratis (filtrado de `BUTTON_REL`, C.3).

**Qué eligió Aura y por qué:** lectura cruda vía `next_button()`. Sin `D-NNN` dedicado que documente la razón — se infiere de la regla general de reescribir todo lo visual/interactivo en vez de reutilizar capas intermedias.

| | `get_action()` + contextos | Lectura cruda |
|---|---|---|
| `BUTTON_REL` espurio | Resuelto automáticamente | Filtrado a mano (patrón probado por Aura) |
| Twist horizontal | Contexto de pivot mapea LEFT/RIGHT a `ACTION_PIVOT_PREV/NEXT` | Directo, sin intermediarios |
| Velocidad de rueda | Vía `get_action_data()` | Directo, vía `button_get_data()` |
| SYS_*/USB/apagado | Comparten la misma cola — sin diferencia | Igual |
| Costo de desarrollo | Tablas de keymap declarativas por contexto nuevo | Lógica de despacho a mano (como Aura) |
| Precedente probado | Ninguno en Aura | Probado y depurado por Aura (bug de `BUTTON_REL` ya corregido) |

**Evidencia:** `apps/action.h`, `apps/keymaps/keymap-ipod.c:63,74,111,117,122,140,159`, `apps/aura/aura_main.c:167-236`.

**Implicación para Metro-Aura:** decisión abierta de Fase 3 — ninguna opción es objetivamente superior. `get_action()` ahorra reimplementar el filtrado; lectura cruda da control total con precedente ya depurado.

`[VERIFICADO]` (mecanismo) / `[ESTIMADO]` (razón exacta de la elección de Aura)

## C.5 — Inventario de eventos disponibles (materia prima, no decisión de mapeo)

| Evento crudo | Con `BUTTON_REPEAT` | Con `BUTTON_REL` | Datos adicionales | Restricción |
|---|---|---|---|---|
| `SCROLL_FWD`/`BACK` | Sí, automático desde la ISR | No aplica | Velocidad real grados/seg | Umbral mínimo 4 clics (`WHEEL_SENSITIVITY`) |
| `SELECT` | Sí (300 ms) | Sí | — | Ninguna reservada; libre para "profundizar" |
| `MENU` | Sí | Sí | — | Ninguna reservada; libre para "retroceder"/Back |
| `LEFT`/`RIGHT` | Sí | Sí | — | Ninguna reservada; candidatos para "pivot" |
| `PLAY` | Sí | Sí | — | Ninguna reservada a nivel driver ("apagar" es lógica de `power_thread`, activable solo si el keymap lo mapea) |
| `SELECT+PLAY` (combinación) | — | — | — | Detectable en `action.c`, no reservada por hardware |
| Candado (`button_hold`) | — | — | — | Bloquea el sensor por hardware — cero eventos posibles |

**Implicación para Metro-Aura:** 5 entradas físicas independientes más PLAY, cada una con estado corto/repetido/soltado, y la rueda con velocidad real medida en hardware — suficiente materia prima para el mapeo (decisión de Fase 3): vertical = rueda, profundizar = SELECT, retroceder = MENU, twist entre pivots = LEFT/RIGHT corto, PLAY = transporte.

`[VERIFICADO]`

## C.6 — Simulador: emulación de rueda y captura headless

**Respuesta:** El simulador emula la rueda de dos formas, **sin velocidad real** en ninguna: (1) rueda del ratón (`scrollwheel_event()`, `SDL_MOUSEWHEEL`) — cada tick genera un único `SCROLL_FWD`/`BACK` con `data = 1<<24` fijo (bit de aceleración **apagado** — `button_apply_acceleration()` nunca recibe el bit de aceleración encendido, devuelve `delta` sin multiplicar); (2) teclado (flechas/`KP_8`/`KP_2`) mapea directo sin ningún dato de velocidad.

**Captura headless (técnica de Aura, D-008/D-017):** `sim_tasks.c` agrega inyección vía variable de entorno (`AURA_SIM_BUTTONS="SELECT,MENU,SCROLL_FWD,..."`), temporización fija (`AURA_INJECT_PRESS_GAP=HZ/4`=250ms, `AURA_INJECT_RELEASE_GAP=HZ/20`=50ms, token de espera `AURA_INJECT_WAIT_TICKS=HZ`=1s). Cada botón se postea presión+`BUTTON_REL` con `data=0` siempre (sin velocidad).

**Evidencia:** `firmware/target/hosted/sdl/button-sdl.c:117-138,643-648`, `uisimulator/buttonmap/ipod.c:45-52`, `uisimulator/common/sim_tasks.c:70-172`.

**Implicación para Metro-Aura:** la técnica de captura headless sirve tal cual como criterio de "hecho" en Fase 4, pero **cualquier comportamiento dependiente de velocidad real de rueda no se puede verificar visualmente en el simulador**, solo lógica de umbral/dirección — la aceleración del twist exige hardware real.

`[VERIFICADO]`

## C.7 — Piezo (clic de la rueda)

**Respuesta:** El clic **no lo dispara el driver ni la ISR** — es decisión de la capa de aplicación: `keyclick_click()` llama a `piezo_button_beep(false, false)` solo si `global_settings.keyclick_hardware` está activo, y esta función **solo se invoca desde `apps/action.c:1004-1005`**, dentro de `get_action()`/`get_custom_action()`. Como Aura lee botones crudos y **nunca pasa por `get_custom_action()`**, el piezo **nunca suena en Aura**, sin cambio explícito alguno — consecuencia automática de la elección arquitectónica de C.4.

**Evidencia:** `apps/misc.c:1111-1169`, `apps/action.c:1004-1005`, ausencia confirmada de referencias a piezo en `button-clickwheel.c`/`piezo-6g.c`.

**Implicación para Metro-Aura:** si Metro adopta lectura cruda (como Aura), el piezo queda desactivado por herencia directa del mecanismo — coherente con F.8 (el pad del Zune 30 no tenía el mismo feedback táctil-sonoro). Si adopta `get_action()`, heredaría el piezo salvo que se desactive `keyclick_hardware` por defecto.

`[VERIFICADO]`

---

# D. Herencia de Aura-Firmware: hardware/build (portar) vs design system (reemplazar)

## D.1 — Los 27 archivos de Rockbox modificados fuera de `apps/aura/`

Tabla diffeada línea por línea contra el upstream `0726ec93` (descargado a scratch fuera de los repos). Rutas relativas a `Aura-Firmware/firmware/rockbox/`.

| Archivo | Qué cambia | D-NNN | Categoría | Veredicto para Metro |
|---|---|---|---|---|
| `apps/SOURCES` | Bloque de 59 líneas listando los 52 `.c` de `apps/aura/` | D-001/D-014 | MIXTO | Técnica reutilizable (bloque propio al final de SOURCES); lista concreta se reemplaza |
| `apps/bitmaps/native/rockboxlogo.320x98x16.bmp` | Bitmap: wordmark "aura" | D-052 | DISEÑO | No portar bitmap; sí el hallazgo de que basta un BMP del mismo nombre/dimensiones |
| `apps/bitmaps/native/usblogo.176x48x16.bmp` | Bitmap: wordmark "USB" acento Aura | D-061 | DISEÑO | Igual — mecanismo sí, arte no |
| `apps/gui/splash.c` | Intercepta el string ya resuelto de `splash_internal()` contra tabla de mensajes | D-055/D-056 | MIXTO | Mecanismo valioso y barato de portar; tabla en español-Aura no se porta |
| `apps/gui/usb_screen.c` | Logo centrado (bug real de centrado), logo propio, colores forzados | D-223/D-225 | MIXTO | Fix de centrado se porta; logo/color propios no |
| `apps/main.c` | Gancho de arranque (`aura_main()`), higiene de `global_settings`, texto de versión suprimido | D-001/D-014/D-015/D-021/D-051/D-052/D-055 | MIXTO — núcleo de la Fase Cero | **Portar**: `tagcache_ram=true` (D-021), higiene de ajustes (`statusbar`, `backdrop_file`, `usb_hid`, `talk_menu`), el mecanismo del gancho en sí. **No portar**: paleta oscura forzada del core, texto "Aura" del logo |
| `apps/misc.c` | `default_event_handler_deferred_usb()` | D-238 | HW-BUILD/COMPAT | Portar si Metro implementa bloqueo por PIN |
| `apps/misc.h` | Declaración de arriba | D-238 | HW-BUILD/COMPAT | Igual |
| `apps/plugin.c` | `plugin_set_silent_open_errors()` | D-298 | HW-BUILD/INFRA | Portar tal cual — flag opt-in, sin efecto por defecto |
| `apps/plugin.h` | Declaración | D-298 | HW-BUILD/INFRA | Portar junto |
| `apps/plugins/mpegplayer/mpeg_settings.c` | Elimina ~340 líneas de UI muerta, agrega `scale_mode` + menú propio | D-062/D-304/D-307/D-309 | MIXTO | Eliminación de UI muerta y mecanismo "ir directo a reproducir" se porta; menú con colores Aura no, pero el patrón (menú C dibujado a mano) sí es el que Metro va a querer |
| `apps/plugins/mpegplayer/mpeg_settings.h` | `enum mpeg_scale_mode_id`, `SETTINGS_VERSION` 5→6 | D-304 | MIXTO | Campo `scale_mode` (ajustar/cubrir) portable |
| `apps/plugins/mpegplayer/mpegplayer.c` | Color de barra, toggle SELECT, splashes traducidos, OSD lee `aura.cfg` | D-062/D-304/D-305/D-306/D-307 | MIXTO | Portar el toggle ajustar/cubrir y el patrón "OSD lee config propio"; no portar paleta/strings literales |
| `apps/plugins/mpegplayer/stream_mgr.c` | Splashes traducidos | D-304 | DISEÑO/i18n | No portar directo |
| `apps/plugins/mpegplayer/video_out.h` | `vo_update_scale_mode()`, `vo_toggle_scale_mode()` | D-304 | HW-BUILD/feature neutra | Portar si se adopta ajustar/cubrir |
| `apps/plugins/mpegplayer/video_out_rockbox.c` | Modo "cubrir", fix de parpadeo OSD, fix de `vo_setup()` | D-304/D-305/D-308 | HW-BUILD + fix de bug real | Portar completo — D-308 es bug real de Rockbox/mpegplayer, no de diseño |
| `apps/plugins/solitaire.c` | Paleta de mesa "Apple2026" | D-072 (heredado) | DISEÑO | No portar |
| `apps/settings.h` | Expone `eq_defaults[]` | (sin D-NNN explícito) | HW-BUILD/INFRA | Portar si Metro construye presets de EQ propios |
| `apps/tagcache.c` | Contador de jobs, descarte de temporal huérfano, fix de fuga (D-244), fix de `load_ramcache()`, `commit()` prefiere buffer temporal | D-021/D-244/D-293 | HW-BUILD/COMPAT-STUDIO (crítico) | **Portar completo, sin recortes** — correcciones de robustez reales + funciones que el contrato de sync necesita |
| `apps/tagcache.h` | 3 declaraciones | D-293 | HW-BUILD/COMPAT-STUDIO | Portar junto |
| `bootloader/ipod-s5l87xx.c` | `verbose=false` bajo `#ifdef IPOD_6G` | D-064 | HW-BUILD | Portar — quirúrgico, no toca lógica de arranque/DFU |
| `firmware/export/config/ipod6g.h` | `CONFIG_BACKLIGHT_FADING` real vía PMU + fix `USBPOWER_BTN_IGNORE` | D-06x/D-061 | HW-BUILD puro | Portar completo — ambos son fixes de hardware genuinos |
| `firmware/export/font.h` | `MAXUSERFONTS` 12→14 | D-263/D-267 | MIXTO | Mecanismo reutilizable; valor concreto (14) recalcular para la escala tipográfica de Metro |
| `firmware/target/hosted/filesystem-unix.c` | **Byte-idéntico al upstream** (confirmado con `cmp`) | D-284 (revertido D-285) | N/A | Nada que portar |
| `lib/rbcodec/codecs/aiff.c` | Tolera variaciones de AIFF de Music/iTunes | (sin D-NNN citado) | HW-BUILD/COMPAT-STUDIO | Portar completo — fix de compatibilidad de códec |
| `uisimulator/common/sim_tasks.c` | Automatización de entrada + captura headless | D-008/D-017 | HW-BUILD/tooling | Portar completo — infraestructura de captura para criterio de "hecho" |
| `utils/mks5lboot/Makefile` | Backend libusb opcional en macOS | D-050 | HW-BUILD/tooling | Portar |

**Resumen D.1:** 9 archivos HW-BUILD/COMPAT-STUDIO puros (portar sin cambios), 2 DISEÑO puro (no portar), 16 MIXTOS (desglosar en la Fase Cero).

## D.2 — Clasificación de `apps/aura/` (107 entradas)

**INFRA REUTILIZABLE** (portar tal cual o con cambios menores, cero o mínima dependencia de `apple2026_shell`/`aura_style`): `aura_sync.c/.h`, `aura_sync_marker.c/.h` 🧪, `aura_media_categories.c/.h`, `aura_artist_images.c/.h` + `_parse.c/.h` 🧪, `aura_fsutil.c/.h`, `aura_lrc.c/.h` 🧪, `aura_device.c/.h` + `aura_device_name.c/.h` 🧪, `aura_albumart.c/.h` (parcial — separar el paso final de conversión a framebuffer), `aura_music.c/.h` (parcial — extraíble con trabajo moderado), `aura_settings.c/.h` (desacoplar claves de tema), `aura_statusbar.c/.h`, `aura_manifest.c/.h` (`NO RESUELTO: función exacta de negocio sin leer el cuerpo completo — revisar en Fase 3 antes de portar`), `test/` (10 archivos + Makefile) 🧪.

**PATRÓN A COPIAR** (mecanismo valioso, código nuevo para Metro): `aura_main.c/.h` (bucle principal, orquestación), `aura_nav.c/.h` 🧪 (máquina de navegación genérica — reescribir pensando ya en pivots), `aura_wheel.c/.h` 🧪, `aura_motion.c/.h` 🧪 (easings — curvas cambian, interfaz de punto fijo se mantiene), `aura_flow.c/.h` 🧪 (aritmética de punto fijo derivada de PictureFlow, D-219 cita `pictureflow.c:803`), `aura_patterns.c/.h` 🧪, `aura_art.c/.h` (caché de transformaciones en disco `.pfraw`), `aura_transitions.c/.h` (motor completo — mecanismo de niveles+fallback+instrumentación se copia, transiciones concretas se rediseñan para vocabulario Zune), `aura_lang.c/.h` (mecanismo de tabla propia sin `.lang`), `aura_splash_lang.c/.h`, `aura_category.c/.h` (mecanismo "color por categoría"), `aura_menu_list.c/.h` (confirma que Aura reescribió la lista en vez de parametrizar `list.c`), `aura_search.c/.h`.

**DISEÑO APPLE — no portar**: `apple2026_shell.c/.h`, `apple2026_tokens.h`, `aura_style.c/.h`, `aura_style_manifest.c/.h`, `aura_widgets.c/.h`, `aura_status_bar_v2.c/.h`, `aura_clock_indicator.c/.h`, `aura_dynamic_title.c/.h`, `aura_marquee.c/.h`, `aura_scroll_indicator.c/.h`, `aura_selection_summary.c/.h`, `aura_selector.c/.h`, `aura_coverdrift.c/.h`, `aura_fx.h`, `aura_screenlock.c/.h`, `aura_shutdown_screen.c/.h`, `aura_screens.c/.h` (6145 líneas, el archivo más grande — dispatcher visual de todas las pantallas), `aura_alarms.c/.h`, `aura_calendar.c/.h`, `aura_stopwatch.c/.h`, `aura_worldclock.c/.h` (lógica de RTC subyacente reutilizable pero entremezclada con el dibujo), `aura_musicflow.c/.h`, `aura_movieflow.c/.h`, `aura_nowplaying.c/.h` (incluye `mode4_morph()`, ver B.10), `aura_photos.c/.h` (lógica de listado extraíble pero mezclada), `aura_video.c/.h` (patrón `plugin_set_silent_open_errors()` reutilizable, resto no), `aura_color.c/.h`.

**Resumen D.2:** ~13 módulos INFRA REUTILIZABLE, ~13 PATRÓN A COPIAR, ~26 DISEÑO APPLE (de 52 módulos `.c`). Proporción ~25%/25%/50%.

`[VERIFICADO]` (por lectura de cabecera + `#include`s + fragmento de cada archivo)

## D.3 — Licencias/branding

**Respuesta:** Assets libres confirmados: **Inter** (SIL OFL), **Lucide** (ISC), **Phosphor** (MIT) — ninguno es material Apple ni Microsoft, Metro puede usarlos si su elección coincide (ver F). D-286 documenta que SF Pro/SF Symbols fueron retirados por ser material Apple no redistribuible — regla equivalente para Metro: cero Zegoe/Segoe.

Branding de Aura a no portar: wordmark "aura", nombre "Aura" en textos de cara al usuario, prefijo `apple2026_*`.

**Conflicto para la sección G:** los nombres de ruta en disco (`.rockbox/aura/`, claves `aura.cfg`) no son branding visual — son el contrato de compatibilidad con Studio (ver E.2). Recomendación preliminar `[ESTIMADO]`: Metro debería **conservar** `.rockbox/aura/` como prefijo de ruta, aunque el firmware se llame/muestre "Metro" — cambiarlo rompe la detección de `AuraDeviceProbe.swift` sin ninguna ganancia.

`[VERIFICADO]`

## D.4 — Build

**Respuesta:** El toolchain de Aura-Firmware se compiló con `RBDEV_PREFIX` relativo al propio checkout — el prefix queda embebido de forma absoluta en los binarios generados (comportamiento estándar de toolchains GNU cross). **No es reutilizable moviéndolo a `Metro-Aura/`** sin recompilar; D-032 documenta que la compilación completa toma "unos pocos minutos" en Mac mini M4, así que compilar uno propio para Metro es barato y evita acoplamiento entre repos.

El parche D-007 (detección de GCC de Homebrew) vive en `tools/configure`, que **no** está en la lista de 27 modificados — sugiere que ya está integrado en upstream `0726ec93`.

**Evidencia:** `guia-desarrollo.md`, `MODIFICATIONS.md`.

`[VERIFICADO]` parcial / `NO RESUELTO: no se confirmó línea por línea que tools/configure sea idéntico al upstream (asumido por ausencia en la lista de 27, no diffeado explícitamente).`

## D.5 — Simulador

**Respuesta:** El cambio en `sim_tasks.c` (D-008/D-017) es automatización de captura headless — **HW-BUILD/tooling puro**, cero relación con diseño, se porta completo. Los scripts que lo consumen (`apple2026_sim_shot.sh`, `apple2026_sim_matrix.sh`) están fuera del árbol Rockbox y deben renombrarse si se portan (el prefijo `apple2026_` es cosmético de nombre de script).

`[VERIFICADO]`

## D.6 — Bootloader y flasheo

**Respuesta:** D-064 (`bootloader/ipod-s5l87xx.c`) se clasifica HW-BUILD pese a estar motivado por UX: toca solo visibilidad de texto (`verbose`), nunca lógica de detección/montaje/DFU. `mks5lboot` con parche libusb (D-050) es herramienta de flasheo del host, se porta como utilidad de desarrollo. El procedimiento completo está en `guia-flasheo-restauracion.md` (no releído completo en esta pasada — insumo directo para `ESTADO_FINAL.md` en Fase 4).

`[VERIFICADO]`

## D.7 — Upstream

**Respuesta:** `git ls-remote https://github.com/Rockbox/rockbox HEAD` devolvió `8cfa4bfd8cd596a48d2afea441f297f2519008fd` — HEAD del mirror está muy por delante de `0726ec93517a61f602679ab052b083217ec9c96d` (2026-08-09). No se hizo un `git log` completo de las rutas `firmware/target/arm/s5l8702/`, `button-clickwheel.c`, `tagcache.c` entre ambos commits (costo/tiempo, el tarball descargado no trae `.git`).

**Recomendación (se mantiene la de §0.3 del plan):** sembrar Metro en el mismo commit `0726ec93` que usa Aura — toolchain, los 27 parches auditados, y el simulador SDL ya están probados contra ese commit exacto; cualquier commit más nuevo introduce una variable no probada sin beneficio claro documentado.

`[VERIFICADO]` (existencia de commits posteriores) / `NO RESUELTO: alcance exacto de cambios upstream posteriores a 0726ec93 en las rutas específicas del target, sin git log con historial completo.`

## D.8 — Interfaz mínima de arranque que Metro debe conservar

**Respuesta:** Cuatro puntos de enganche exactos:

1. **`aura_main_sync_after_disk_handoff()`** (`apps/aura/aura_sync.c`) — invocada al arrancar y al volver de `default_event_handler(SYS_USB_CONNECTED)`. Único punto donde el firmware sabe que Studio pudo haber escrito al disco.
2. **`aura_settings_apply_pending_clock()`** (`apps/aura/aura_settings.c`, D-321) — dentro del handoff de arriba; aplica RTC si las 6 claves `rtc_sync_*` están completas.
3. **Filtrado de `BUTTON_REL`** en `next_button()` — necesario porque Rockbox deja eventos "soltar" pendientes que confunden a un lector de botones crudo.
4. **El patrón D-015** (`root_menu()` sustituido por una línea en `apps/main.c`, con `root_menu.c` compilado pero inalcanzable) — Metro necesita el equivalente exacto.

**Implicación para Metro-Aura:** sin estos 4 puntos, Metro rompería exactamente lo que la restricción dura del proyecto prohíbe: Studio sincroniza y el firmware nunca se entera (sin #1), la hora queda desincronizada tras cada conexión (sin #2), mensajes de UI se cierran solos por eventos fantasma de botón (sin #3).

`[VERIFICADO]` (orden exacto de las 3 llamadas dentro del handoff no verificado línea por línea — ver también E.9)

---

# E. Compatibilidad con Aura Studio (restricción dura)

## E.1 — Tabla completa del contrato de datos en disco

| Ruta / clave | Escribe | Lee | Formato / notas | Clasificación Metro | Consecuencia de no cumplir |
|---|---|---|---|---|---|
| `.rockbox/rockbox.ipod` | Studio (instalador) | Bootloader; Studio (`AuraUpdateChecker`) | Binario | **OBLIGATORIO** | Sin esto no arranca nada |
| `.rockbox/aura/` o `.rockbox/icons/aura/` (existencia) | Firmware / instalador | Studio (`AuraDeviceProbe.probe()`) | — | **OBLIGATORIO** (E.2) | Sin uno de los dos, Studio clasifica el disco como `.rockbox` genérico, no `.aura` |
| `.rockbox/aura/aura.cfg` | Firmware (`aura_settings_save()`) | Studio (`AuraDeviceProbe`, `hasBooted`) | `clave: valor`, regenerado ENTERO en cada guardado | **OBLIGATORIO** | Sin él, `isAura` nunca es `true` por evidencia de arranque |
| `aura.cfg` → `theme_id` | Firmware; Studio también puede editar la línea | Firmware al arrancar | D-289 | RECOMENDADO | Sin soporte de temas, Studio no instala temas — degradación soportada |
| `aura.cfg` → `theme_format_supported` | Firmware, SIEMPRE | Studio, antes de instalar tema | D-289 | RECOMENDADO | Ausente = Studio no instala — inofensivo |
| `aura.cfg` → `sync_marker_supported` | Firmware, SIEMPRE | Studio, al terminar sync | D-293, valor = versión de esquema (`1`) | **OBLIGATORIO** (E.4) | Ausente → Studio borra `.tcd` en cada sync, reconstrucción completa siempre |
| `aura.cfg` → `rtc_sync_year/month/day/hour/min/sec` | Studio (`ClockSyncWriter`) | Firmware (`aura_settings_apply_pending_clock()`) | v7/D-321, transitorias | RECOMENDADO | Reloj no se sincroniza solo — UX menor |
| `aura.cfg` → `tz_local_quarters` | Firmware y Studio | Firmware (reloj mundial) | Persistente | RECOMENDADO | — |
| `.rockbox/aura/themes/<id>/` | Studio o usuario | Firmware (`aura_style_scan()`) | `CONTRATO-formato-tema.md`, 14 fuentes + 801 máscaras | **FUERA DE ALCANCE** (E.10) | Studio no ofrece instalar temas si `theme_format_supported` ausente |
| `.rockbox/aura/device.cfg` | Studio (solo `device_owner`) | Studio; firmware solo `device_name` | `CONTRATO-dispositivo.md` v2, líneas ≤63 bytes | RECOMENDADO | Sin leerlo, Metro no muestra nombre editable — no rompe Studio |
| `/.aura/sync-pending.json` | Studio; firmware sube `attempts` y borra al terminar | Firmware al arrancar y al volver de USB | `library-layout-v1.md` §4, v1 | **OBLIGATORIO** (E.4) | Sin leerlo, Metro nunca sabe qué reconstruir tras un sync |
| `.rockbox/database_*.tcd` (8 archivos) | Firmware (tagcache); Studio solo los borra sin `sync_marker_supported` | Firmware | Índice binario | **OBLIGATORIO** (formato en sí) | Metro debe usar el mismo `tagcache.c`/formato — no hay alternativa razonable |
| `.rockbox/aura/sync_manifest.json` | Studio | Studio | JSON interno | Irrelevante para el firmware | — |
| `.rockbox/aura/sync_summary.cfg` | Studio | Firmware ("Acerca de") | `clave: valor` | RECOMENDADO | Sin él, "Acerca de" no muestra conteos — cosmético |
| `.rockbox/aura/ratings.cfg` | Studio | Firmware (`import_ratings_from_studio()`) | `/ruta: calificación(0-10)` | RECOMENDADO | Calificaciones no llegan al iPod — UX |
| `.rockbox/icons/aura/` | Instalador | Firmware | — | Depende del diseño de Metro (F) | — |
| `.rockbox/fonts/a26-title-20.fnt` | Instalador | Studio (sentinela de "árbol instalado") | Nombre específico de Aura | NO APLICA sin cambio | Studio usa este nombre exacto como prueba de instalación completa — ya marcado en el contrato como "candidato a reemplazo, no implementado" |
| `Playlists/<Nombre>.m3u8` | Studio | Firmware (`aura_music.c:993`, vía `playlist_create()` stock) | `#EXTM3U` + rutas absolutas UNIX, UTF-8 | **OBLIGATORIO** | Código Rockbox core sin modificar — Metro lo hereda gratis |
| `Playlists/<Nombre>.jpg` | Studio | Firmware (`aura_playlist_art_load()`) | JPEG, mismo nombre base | RECOMENDADO | Cosmético |
| `Music/` (3 layouts) | Studio | Firmware (tagcache escanea `/` entero) | Saneo FAT32 (`PathSanitizer`, 120 chars/componente) | **OBLIGATORIO** | Ninguna — layout indiferente al firmware |
| `Music/.../*.lrc` | Studio | Firmware (`aura_lrc.c`) | UTF-8, LRC estándar | RECOMENDADO | Sin letras, no hay modo Letras — feature opcional |
| `cover.jpg` en carpeta de álbum | Studio (política `albumOnly`) | Firmware (`find_albumart()`, Rockbox stock sin modificar) | JPEG/BMP | **OBLIGATORIO** | — |
| Carátula embebida (frame `APIC`, política `perTrack`) | Studio | Firmware, solo JPEG | Frame ID3v2.3 | RECOMENDADO | Solo importa con política `perTrack` |
| `/Videos/` (plano) | Studio | Firmware (`aura_video.c`, `VIDEO_NAME_LEN=96`, `MAX_VIDEOS=100`) | Nombre ≤95 bytes UTF-8+ext | **OBLIGATORIO** | Metro debe respetar el mismo límite |
| `Videos/<archivo>.jpg` (póster) | Studio (opcional) | Firmware (`aura_screens.c`, CoverDrift) | JPEG, mismo nombre base | RECOMENDADO | Cosmético |
| `/Photos/` (plano) | Studio | Firmware (`aura_photos.c`, `PHOTO_NAME_LEN=96`, `MAX_PHOTOS=500`) | JPEG baseline ≤640px, sRGB | **OBLIGATORIO** | — |
| `.rockbox/aura/video_categories.cfg` | Studio, OPCIONAL | Firmware (`MAX_VIDEO_ENTRIES=100`) | `nombre.ext: movie\|series\|clip` | RECOMENDADO | Degradación soportada nativa ("sin categoría") |
| `.rockbox/aura/photo_categories.cfg` | Studio, OPCIONAL | Firmware (`MAX_PHOTO_ENTRIES=500`) | `nombre.ext: photo\|image\|ai` | RECOMENDADO | Ídem |
| `.rockbox/aura/artists/<archivo>.jpg` | Studio, OPCIONAL | Firmware (`MAX_ENTRIES=300`) | JPEG ≤128px cuadrado | RECOMENDADO | Sin ellas, placeholder circular — degradación soportada |
| `.rockbox/aura/artist_images.cfg` | Studio, OPCIONAL | Firmware | `archivo.jpg: <artista>`, `strcmp` byte a byte contra `tag_artist` | RECOMENDADO | Ídem |

**Nota E.1-bis:** `.rockbox/aura/version.txt` **no aparece en `CONTRATO-firmware-studio.md` §D** pero sí lo consume `AuraUpdateChecker.installedVersionTag()` — laguna real del contrato escrito. El firmware nunca lo escribe; lo genera `package_dist.sh --release-tag` estáticamente en tiempo de build. **Implicación:** si Metro adopta el flujo de hash-checking, debe replicar este archivo estático en su propio empaquetado; si no, `installedVersionTag()` devuelve `nil` y el chequeo cae al hash SHA-256 sin red — camino ya soportado sin cambios.

`[VERIFICADO]`

## E.2 — Detección de Aura por Studio (pregunta crítica)

**Respuesta:** `AuraDeviceProbe.probe()` separa dos hechos: (1) **archivos en disco** — clasifica `.aura(hasBooted: exists("aura.cfg"))` si existe `.rockbox/aura` o `.rockbox/icons/aura`; o `.aura(hasBooted:false)` si solo existe `rockbox.ipod`; o `.rockbox(hasBooted:…)` (genérico, NO Aura) si solo existe `.rockbox/`; (2) **descriptor USB en vivo** (`runningFirmware`) — string USB real ("Rockbox.org/Rockbox media player" vs. "Apple Inc./iPod").

El booleano decisivo:
```swift
var isAura: Bool {
    guard case .aura(let hasBooted) = firmware else { return false }
    return hasBooted || runningFirmware == .rockboxFamily
}
```
Hace falta clasificación de archivos `.aura` **Y** evidencia real de arranque (`aura.cfg` existe) **O** USB atendido por firmware de la familia Rockbox (`.rockboxFamily`, no restringido a la palabra "Aura").

**Nada en el código comprueba el nombre "Aura" en sí** — ni contenido de `aura.cfg`, ni identificador de producto, ni cadena literal "Aura" en el descriptor USB. La detección se basa en tres cosas puramente estructurales: (a) existe `.rockbox/aura/`, (b) existe `.rockbox/aura/aura.cfg`, (c) el USB reporta "Rockbox" (cualquier fork).

**Implicación para Metro-Aura:** Metro **puede hacerse pasar por "Aura instalada" sin cambiar ni una línea de Swift**, con tres condiciones triviales desde C: crear `.rockbox/aura/`, escribir `.rockbox/aura/aura.cfg` al primer arranque (cualquier contenido — Studio no lo parsea para `isAura`), y no cambiar el string de identidad USB del stack Rockbox stock. **Decisión más importante de la sección G**: Metro no necesita "fingir ser Aura" en ningún sentido profundo — con el mismo layout de directorios que ya exige el resto del contrato, la detección de Studio ya lo reconoce como compatible. El riesgo real no es "Studio no lo detecta" sino el opuesto (E.3).

`[VERIFICADO]`

## E.3 — `AuraUpdateChecker` / riesgo de reinstalación

**Respuesta:** `checkForUpdate()` compara por tag semver vía `version.txt`+GitHub Releases (caché 24h); si falla, cae a `isUpdateAvailable()` — compara hash SHA-256 del `rockbox.ipod` embebido contra el instalado. Solo **avisa** — no reinstala por su cuenta (función de solo lectura, sin llamada a `InstallPlanner`).

**Riesgo real:** si el usuario tiene Aura Studio (con `rockbox.ipod` de Aura embebido) y conecta un iPod con Metro-Aura, el hash nunca coincidirá → "hay actualización disponible" **permanentemente**; si el usuario pulsa actualizar, el flujo normal de instalación **sí sobrescribiría Metro con Aura**. No es hipotético — es el comportamiento por diseño de la función.

**Implicación para Metro-Aura:** Metro necesita su propia app Studio (o variante con su propio `rockbox.ipod` embebido), o Studio debe aprender a distinguir builds. Ninguna de las dos existe hoy. **Decisión abierta bloqueante de Fase 3** — afecta si el proyecto necesita también tocar `Aura-Studio/`, lo cual el encabezado de este documento prohíbe hacer desde `Metro-Aura/` sin una sesión dedicada a ese repo.

`[VERIFICADO]` ("no reinstala sola") / `[ESTIMADO]` (punto exacto de UI que consume el resultado, no explorado)

## E.4 — Marcador de sincronización: flujo y viabilidad del fallback

**Respuesta:** **Studio** (`SyncMarker.swift`): al terminar un sync con cambios, escribe `{version:1, timestamp, changes:{music,video,images}, attempts:0}` en `/.aura/sync-pending.json`; si ya había un marcador sin procesar, las secciones se acumulan. **Firmware** (`aura_sync.c`): `aura_sync_check_pending()` al arrancar y al volver de USB, parser propio minimalista (sin librería JSON real); si `version > SUPPORTED` → ignora y deja intacto; `music` dispara `tagcache_update()` (incremental) o `tagcache_rebuild()` (si no hay base usable); `video`/`images` solo invalidan listado en RAM. `attempts` lo sube el firmware antes de empezar (sobrevive corte de batería); a 3 fallos consecutivos, deja de reintentar solo.

**Viabilidad del fallback sin `sync_marker_supported`:** Studio (sin cambios) borra los 8 `.tcd` en cada sync y el firmware reconstruye **completo** (`Q_REBUILD`) al arrancar — camino ya soportado y probado en producción por Aura (mecanismo original, previo a D-293), no un experimento. El costo real en tiempo para bibliotecas grandes en hardware real **no está cuantificado**.

**Evidencia:** `SyncMarker.swift:50`, `aura_sync.c`, `aura_sync_marker.c`.

**Implicación para Metro-Aura:** recomendación — implementar el marcador desde el día uno (código ya escrito y testeado, portable casi sin cambios, ver D.2) en vez de depender del fallback.

`[VERIFICADO]` (mecanismo) / `NO RESUELTO: costo real en tiempo de Q_REBUILD completo en hardware real con biblioteca grande, sin medición documentada.`

## E.5 — Tags de metadata y búsqueda de carátulas

**Respuesta:** `ID3Writer.swift` escribe frames ID3v2.3 propios: `TIT2`, `TPE1`(→`tag_artist`), `TALB`, `TPE2`(→`tag_albumartist`), `TYER`, `TCON`, `TCOM`, `TRCK`, `APIC`. El firmware usa **`tag_artist`, NO `tag_albumartist`**, para fotos de artista (confirmado en el propio contrato §D.3). Aura no usa `apps/tagtree.c`/`tagnavi.config` — consultas directas a tagcache en `aura_music.c` (comentario D-022: "Aura nunca usa apps/tagtree.c, tiene su propia navegación").

Carátulas: `find_albumart()` (Rockbox stock, sin modificar, no está en la lista de 27) prueba en orden `<pista>.*` → `<álbum>.*` → `cover.*` → `../<álbum>.*` → `../cover.*` → `albumart/<artista>-<álbum>.*`. Studio usa `albumOnly` (un único `cover.jpg` por álbum) — coincide con el tercer patrón.

**Evidencia:** `ID3Writer.swift:71-80`, `apps/recorder/albumart.c:104-119`, `aura_music.c:154-155`.

**Implicación para Metro-Aura:** `find_albumart()` se hereda gratis con el árbol base. La convención `tag_artist` para fotos de artista debe replicarse exactamente (mismo `strcmp` byte a byte, sin normalizar acentos) si Metro reutiliza `artist_images.cfg`.

`[VERIFICADO]`

## E.6 — Formatos producidos por Studio que el firmware debe reproducir

**Audio:** `AudioTranscoder.swift` codifica a MP3 `libmp3lame` 256kbps cuando hace falta transcodificar. `[VERIFICADO]` parcial (no confirmado si copia AAC/ALAC/FLAC/WAV/AIFF sin transcodificar cuando ya son compatibles — el árbol de Rockbox soporta todos esos códecs).

**Video:** `FFmpegTranscoder.swift` siempre transcodifica a **MPEG-1/2 320×240, MPEG-PS (`.mpg`)**, `-r 24` forzado si la fuente excede 24fps (límite real de hardware documentado: "mpegplayer en el S5L8702 decodifica bien hasta ~24-25 fps"). Es el **único** formato reproducible — sin ruta de "copiar sin transcodificar".

**Fotos:** JPEG baseline ≤640px, sRGB sin ICC (comentario explícito: "640 = máximo que admite el firmware").

**Fotos de artista:** JPEG ≤128px cuadrado.

**Implicación para Metro-Aura:** sin margen de negociación en formatos — video **tiene que ser** MPEG-1/2 320×240 (todo pasa por `mpegplayer`, heredado del core sin cambios si se conserva ese plugin); fotos JPEG baseline ≤640px.

`[VERIFICADO]` (intención de video/fotos) / `[ESTIMADO]` (mecanismo exacto de forzado de baseline en audio/fotos, no confirmado línea por línea)

## E.7 — Reglas de nombres y rutas

**Respuesta:** `PathSanitizer.swift` tiene dos funciones: `sanitize(_:maxLength:)` para componentes de ruta (`/Music/`, recorta a 120 **caracteres**), y `sanitizeFilename(_:maxBytes:)` para nombres finales en `/Videos/`/`/Photos/` (recorta por **bytes UTF-8**, preservando extensión — coincide con `VIDEO_NAME_LEN=96`/`PHOTO_NAME_LEN=96` del firmware, 95 bytes útiles). `read_line()` del firmware usa buffers de 63-64 bytes para todos los `.cfg`.

**Evidencia:** `PathSanitizer.swift`, `aura_settings.c:242` (`char line[64]`), `aura_device.c:44`.

**Implicación para Metro-Aura:** los límites exactos (95 bytes de nombre en Video/Fotos, 63 bytes de línea en `.cfg`) son propiedades de los buffers C del firmware — Metro los hereda automáticamente si reutiliza los mismos tamaños de buffer al portar `aura_video.c`/`aura_photos.c`/parsers `.cfg`.

`[VERIFICADO]`

## E.8 — Playlists

**Respuesta:** `PlaylistExporter.m3u8Contents()` genera `#EXTM3U` + rutas **absolutas UNIX**, UTF-8, extensión `.m3u8`. Rockbox stock (`playlist_create()`) acepta rutas UNIX absolutas sin modificarlas — código no tocado por Aura. Aura escanea `/Playlists` (`aura_music.c:993-1014`) y delega en funciones stock para reproducir.

**Evidencia:** `apps/playlist.c:319,461,2056`.

**Implicación para Metro-Aura:** 100% código Rockbox core sin modificar — se hereda gratis con el fork base; solo el escaneo/listado es código Aura (categoría PATRÓN A COPIAR).

`[VERIFICADO]`

## E.9 — Puntos de arranque/USB que el contrato exige

**Respuesta:** Dos momentos donde el firmware "recupera el disco": al arrancar y al volver de la pantalla USB. En ambos debe: (1) `aura_settings_apply_pending_clock()`, (2) `aura_sync_check_pending()`, (3) `aura_device_reload()` — orden exacto no verificado línea por línea en esta pasada.

**Implicación para Metro-Aura:** si Metro no llama estas tres funciones (o equivalentes) en ambos puntos, el reloj queda desincronizado y la biblioteca no se actualiza tras un sync — regresiones de UX visibles, no catastróficas.

`[ESTIMADO]` (orden exacto) / `NO RESUELTO: orden exacto de las tres llamadas en aura_main.c — recomendación: leer aura_main_sync_after_disk_handoff() completo antes de reimplementar.`

## E.10 — Temas: degradación sin soporte

**Respuesta:** `ThemeValidator.swift` valida antes de instalar: manifiesto parseable, las 801 máscaras, `theme_format ≤ theme_format_supported` leído del `aura.cfg` del dispositivo. Si Metro no escribe `theme_format_supported`, la clave simplemente no existe → Studio no confirma soporte → instalación de temas queda deshabilitada de forma segura (mismo patrón usado para `sync_marker_supported`, que devuelve `nil` limpiamente si falta).

**Implicación para Metro-Aura:** confirmado **FUERA DE ALCANCE seguro** — Metro puede no implementar el sistema de temas de Aura sin ningún riesgo de romper Studio.

`[VERIFICADO]` (patrón general) / `[ESTIMADO]` (aplicación idéntica a `theme_format_supported`, `ThemeInstaller.swift` no releído completo)

## E.11 — Checklist preliminar de compatibilidad

| # | Comprobación | Método de verificación |
|---|---|---|
| 1 | `.rockbox/aura/` existe tras el primer arranque | Simulador, inspeccionar disco simulado |
| 2 | `aura.cfg` se crea y regenera en cada guardado de ajuste | Simulador, comparar timestamp antes/después |
| 3 | `aura.cfg` anuncia `sync_marker_supported: 1` | `grep` sobre el `aura.cfg` del disco simulado |
| 4 | `/.aura/sync-pending.json` se procesa y borra al reconstruir bien | Simulador, marcador manual + arranque |
| 5 | `find_albumart()` encuentra `cover.jpg` en carpeta de álbum | Simulador con `test-media` de Aura-Firmware (fixture reutilizable) |
| 6 | `.lrc` sincronizado se reproduce en modo Letras (si se implementa) | Simulador con datos `.lrc` |
| 7 | Playlist `.m3u8` con rutas absolutas se reproduce completa | Simulador, playlist de prueba formato `PlaylistExporter` |
| 8 | Nombre de archivo de 95 bytes UTF-8 no se trunca ni falla | Simulador, archivo con nombre límite |
| 9 | `AuraDeviceProbe.isAura` reconoce el disco como compatible | Manual — verificar existencia de `.rockbox/aura/aura.cfg` |
| 10 | `AuraUpdateChecker` no ofrece "reinstalar Aura" sobre un iPod Metro | **Bloqueado**: requiere resolver E.3 (Fase 3) |

`[VERIFICADO]`

---

# F. Referencia Metro / Zune: catálogo de patrones, paleta, tipografía, transiciones

## F.1 — Estructura de navegación del Zune 30 (twist interface)

**Respuesta:** Microsoft llamó "twist interface" a la UI del Zune, "navegación bidimensional" vía pad direccional: una dimensión es scroll vertical dentro de una lista, la otra es movimiento horizontal para profundizar/retroceder entre niveles jerárquicos. Secciones raíz confirmadas: **Music, Videos, Pictures, Social, Radio, Podcasts, Marketplace, Games, Settings**. Dentro de Music, categorías tipo pivot: Artists/Albums/Songs/Genres/Playlists, con "Quickplay" como acceso directo. Back retrocede un nivel; SELECT/centro del pad profundiza; arriba/abajo mueve la lista; izquierda/derecha alterna pivots hermanos sin volver a la raíz. PLAY/PAUSE físico controla transporte global.

**Evidencia:** https://en.wikipedia.org/wiki/Zune (nombre "twist interface", "two-dimensional navigation", lista de secciones raíz) — `[VERIFICADO]` nombre/cita/lista raíz; `[ESTIMADO]` detalle de pivots internos de Music y mapeo Back/pad (no confirmado en fuente primaria de este sondeo).

**Implicación para Metro-Aura:** modelar el árbol de Fase 3 como matriz profundidad (vertical) × hermandad (horizontal), con Back como operación explícita distinta de "pivot izquierda/derecha". Mapeo natural a clickwheel: `SCROLL_FWD/BACK` = vertical, `LEFT/RIGHT` = pivot entre hermanas, `SELECT` = profundizar, `MENU` = Back (ver C.5). No más de 2-3 niveles de pivots hermanos por pantalla.

## F.2 — Catálogo de patrones visuales del Zune HD (Metro)

**Respuesta:** Zune HD (2009, 480×272 táctil), según Wikipedia "precursor del lenguaje de diseño Metro que culminó con Windows Phone" — usaba fuente **Zegoe**, versión modificada de Segoe (propietaria). Patrones del lenguaje Metro heredados por WP7 (misma época, mejor documentado): tipografía como elemento dominante ("content before chrome"), encabezados en minúsculas, "wide tiles", paneles Panorama (fondo panorámico con scroll horizontal continuo, "peek" de la siguiente sección) y Pivot (pestañas horizontales, contenido independiente). Colores planos en vez de skeuomorfismo. Now Playing: carátula grande a pantalla completa con controles mínimos.

Patrones razonablemente atribuibles al Zune HD (no confirmados con fuente primaria): Quickplay con mosaicos, índice alfabético lateral, titular grande recortado contra el borde.

**Evidencia:** https://en.wikipedia.org/wiki/Zune_HD (Zegoe/Segoe, rol precursor) `[VERIFICADO]`; https://en.wikipedia.org/wiki/Metro_(design_language) (tipografía, tiles, Panorama/Pivot) `[VERIFICADO]` para WP7 en general, `[ESTIMADO]` aplicación 1:1 al Zune HD específico.

**Implicación para Metro-Aura:** más seguro de portar (fuente confirmada): tipografía dominante + tiles planos + "content before chrome" (ya es regla de Aura). Panorama con "peek" es costoso en 320×240 sin GPU — candidato a simplificar a pivot con turnstile en vez de paneo continuo. Quickplay/índice alfabético son decisión de Fase 3, no hecho verificado del Zune HD original.

## F.3 — Catálogo de transiciones (Windows Phone Toolkit, Ms-PL, mismo mecanismo que WP7/Zune HD)

Fuente: `github.com/microsoftarchive/WindowsPhoneToolkit`, `Microsoft.Phone.Controls.Toolkit/Transitions/Storyboards/*.xaml` — todos `[VERIFICADO]` salvo donde se indica.

| Nombre | Qué mueve | Duración | Easing | Cuándo se usa |
|---|---|---|---|---|
| **Turnstile In** (adelante) | `RotationY`: -80°→0° + opacidad 0→1 en 10ms | 350 ms | `ExponentialEase EaseOut` exp 6 | Entrada al profundizar |
| **Turnstile Out** (adelante) | `RotationY`: 0°→50°, opacidad cae en los últimos 10ms | 250 ms | `ExponentialEase EaseIn` exp 6 | Salida al profundizar |
| **Turnstile In** (atrás) | `RotationY`: 50°→0° + opacidad 0→1 en 10ms | 350 ms | `ExponentialEase EaseOut` exp 6 | Entrada al volver (Back) |
| **Slide FadeIn** | `TranslateTransform.X`: 200px→0 + opacidad 0→1 | 500 ms | `ExponentialEase EaseOut` exp 6 | Transición lateral entre pivots (equivalente al twist horizontal) |
| **Swivel Forward In** | `RotationX`: -45°→0°, opacidad fija en 1 | 350 ms | `ExponentialEase EaseOut` exp 6 | Entrada de elemento hijo, menos "salto" que Turnstile |
| **Swivel FullScreen In** | `RotationX`: -30°→0° | 350 ms | `ExponentialEase EaseOut` exp 6 | Variante a pantalla completa |
| **Rotate 90 In (Clockwise)** | `RotationZ`: 90°→0° + opacidad 0→1 | 250 ms | Rotación exp 1, opacidad `SineEase EaseOut` | Overlays/diálogos |
| **Roll** | `RotationZ`: 0°→45°(0.3s)→90°(0.6s), 2 fases coreografiadas | 600 ms | Exponencial, EaseIn→EaseOut exp 6 | Efecto "enrollar" — el más caro del catálogo |
| **Feathered/Turnstile-Feather** | Igual que Turnstile pero con `BeginTime` (offset) por ítem — cascada | Igual que Turnstile + offset por ítem | Heredado de Turnstile | Entrada escalonada de una lista completa — offset típico `[ESTIMADO]` 30-50ms/ítem |

**Implicación para Metro-Aura:** todas las duraciones caen en 250-600ms, consistente con el techo de 330ms que Aura ya midió como presupuesto nominal (B.10). **Slide** (memcpy de columnas, B.3) es el más barato con las primitivas de la sección B — candidato por defecto entre pivots. **Turnstile/Swivel** son el equivalente conceptual de las técnicas de PictureFlow por columnas — factibles con tablas precalculadas, a evaluar costo real en B. **Roll** (2 fases, 600ms) no recomendado como transición estándar. **Feathered** es barato de adaptar sobre cualquier transición base.

## F.4 — Matriz de priorización (impacto visual vs. costo en ipod6g)

| Patrón | Impacto | Costo estimado | Riesgo | Recomendación |
|---|---|---|---|---|
| Tipografía dominante + recorte en el borde | Alto | Bajo (primitiva nativa, ver A.6) | Bajo | Hacer |
| Tiles planos de color sólido | Alto | Bajo (rectángulos + texto) | Ninguno | Hacer |
| Encabezado de pivots | Alto | Bajo (texto + color, sin animación) | Ninguno | Hacer |
| Slide horizontal (pivot↔pivot) | Alto | Bajo (memcpy de columnas, B.3) | Bajo | Hacer |
| Turnstile/Swivel | Alto | Medio (tablas de perspectiva, B.5) | Medio (riesgo de repetir el problema del morph, D-3xx) | Hacer con nivel de FX + fallback a Slide |
| Cascada de lista (Feathered) | Medio | Bajo (offset de tiempo por fila) | Bajo | Hacer |
| Fondo con carátula atenuada/parallax | Medio-Alto | Medio-Alto (blend por píxel, B.4) | Alto si se anima continuo (mitigado: sin touch continuo en ipod6g) | Versión estática (fondo fijo atenuado); degradar parallax si el costo no da |
| Roll (2 fases, 600ms) | Bajo-Medio | Alto | Alto | No hacer en v1 |
| Blur real de fondo | Medio | Muy alto (sin GPU, O(radio²)/píxel) | Alto | No hacer — sustituir por atenuación plana |

**Implicación para Metro-Aura:** este orden es el orden recomendado de implementación de Fase 3/4 — texto y tiles primero (barato, alto impacto, no depende de B), slide y cascada después, turnstile/swivel solo tras medir presupuesto real de CPU en B, roll y blur descartados de la v1.

## F.5 — Paleta propuesta (RGB565)

Valores base de la tabla oficial de acentos WP7.0/7.1 (10 colores). Conversión a RGB565 y error de cuantización calculados directamente.

| Color | Hex (8-8-8) | RGB565 | Hex reconstruido | Error euclidiano (de 441 máx.) |
|---|---|---|---|---|
| Negro (fondo) | `#000000` | (0,0,0) | `#000000` | 0.00 |
| Blanco (texto primario) | `#FFFFFF` | (31,63,31) | `#FFFFFF` | 0.00 |
| Gris secundario | `#999999` | (19,38,19) | `#9C9A9C` | 4.36 |
| Gris terciario | `#666666` | (12,25,12) | `#636563` | 4.36 |
| Azul | `#1BA1E2` | (3,40,27) | `#19A2DE` | 4.58 |
| Café | `#A05000` | (19,20,0) | `#9C5100` | 4.12 |
| Verde | `#339933` | (6,38,6) | `#319A31` | 3.00 |
| Lima | `#8CBF26` | (17,47,5) | `#8CBE29` | 3.16 |
| Magenta | `#FF0097` | (31,0,18) | `#FF0094` | 3.00 |
| Mango (naranja) | `#F09609` | (29,37,1) | `#EF9608` | 1.41 |
| Rosa | `#E671B8` | (28,28,22) | `#E671B5` | 3.00 |
| Púrpura | `#A200FF` | (20,0,31) | `#A500FF` | 3.00 |
| Rojo | `#E51400` | (28,5,0) | `#E61400` | 1.00 |
| Teal | `#00ABA9` | (0,42,21) | `#00AAAD` | 4.12 |

**Evidencia:** https://davefancher.com/2012/10/25/windows-phone-accent-color-cheat-sheet/ (tabla RGB/hex de 10 colores 7.0/7.1) `[VERIFICADO]`; conversión RGB565 `[VERIFICADO]`, cálculo propio reproducible.

**Implicación para Metro-Aura:** error máximo de toda la tabla es 4.58/441 (~1%) — ningún color "vira" perceptiblemente en RGB565; se puede usar el hex original de WP7 sin ajuste especial. Recomendación de Fase 3: negro/blanco base (modo oscuro), dos grises para texto secundario/terciario, un acento por categoría de contenido (mismo patrón "color por categoría" que ya documenta Aura).

## F.6 — Experimento de tipografía gigante

**Respuesta:** El pipeline de conversión funciona de punta a punta con una fuente real y libre. Se descargó **Selawik Light** (release oficial `microsoft/Selawik` v1.01) y se convirtió con `tools/convttf` (ya compilado en Aura-Firmware) a tres tamaños, anti-alias activado, `-x` (trim horizontal):

| Tamaño | Resultado | Peso del `.fnt` | Glifos | Errores |
|---|---|---|---|---|
| 24px | `selawkl-24.fnt` | 60.7 KB | 348 | 0 |
| 42px | `selawkl-42.fnt` | 147.7 KB | 348 | 0 |
| 64px | `selawkl-64.fnt` | 284.4 KB | 348 | 0 |

Conversión sin errores en los tres tamaños, formato reconocido por `firmware/font.c`. No se encontró en `font.h` límite de tamaño de píxel hardcodeado — `MAXFONTS` limita cuántas fuentes distintas hay cargadas, no el tamaño de cada una; el límite práctico real es el de `MAX_FONT_SIZE`/modo caché ya documentado en A.7.

**Bloqueo explícito, sin inventar resultado:** no fue posible completar la verificación **visual** (titular recortado en el simulador SDL real) sin modificar el árbol de Aura-Firmware — `build_sim.sh` copia archivos dentro del repo e instala fuentes/íconos en `firmware/build-sim/simdisk/`, y requeriría código C o un `.wps` nuevo — cambios de código fuente prohibidos en Fase 2. **Este sub-experimento queda pendiente como primer paso verificable de la Fase 4** (criterio de "hecho" natural: captura con `apple2026_sim_shot.sh` mostrando titular Selawik 64px recortado contra el borde derecho).

**Evidencia:** ejecución real de `convttf` en este sondeo, "done (converted 348 glyphs, 0 errors)" en los tres tamaños `[VERIFICADO]`; ausencia de límite de tamaño en `font.h` `[VERIFICADO]` por lectura, no exhaustivo respecto a otros consumidores del formato.

**Implicación para Metro-Aura:** la infraestructura (conversión TTF→bitmap AA) no es bloqueante — funciona hoy con una fuente libre real a los tamaños del objetivo (~24-64px). El riesgo real a resolver en Fase 4 es rendimiento/legibilidad en pantalla real y el límite de 10 KB de A.7, no la herramienta.

## F.7 — Estrategia de tipografía definitiva

**Respuesta:**

**(a) Fuente libre empaquetada — recomendación: Selawik.** Licencia SIL OFL 1.1 confirmada en el repositorio oficial ("Reserved Font Name Selawik" restringe solo derivados con ese nombre, no la redistribución del binario tal cual). Release oficial con binarios compilados trae 5 pesos: Light, Semilight, Regular, Semibold, Bold — creada explícitamente por Microsoft como "reemplazo de código abierto para Segoe UI". Pesos recomendados: **Light** para titulares gigantes (fiel al peso fino de Zegoe en Zune HD/WP7), **Semibold**/**Regular** para listas y cuerpo. Rango de caracteres: 348 glifos (Latin Basic + Latin-1 + Latin Extended-A, suficiente para español con acentos/ñ), sin necesidad de limitar salvo para ahorrar espacio en tamaños grandes (decisión de Fase 3 con datos reales de RAM, ver A.7).

**(b) Flujo opcional "el usuario aporta Zegoe/Segoe":** procedimiento manual documentado en la guía de desarrollo — el usuario corre `convttf -p <tamaño> -x -o a26-<rol>-<tamaño>.fnt <su TTF>` con los mismos parámetros que Metro-Aura, copia el resultado a una ruta convenida en `.rockbox/fonts/` o, preferentemente, enchufado al mecanismo de "Estilo"/tema que ya construyó Aura (D-289, resuelve el fallback de seguridad de no dejar el dispositivo sin UI legible) — decisión de Fase 3.

**Evidencia:** licencia y binarios de Selawik `[VERIFICADO]` (repo y release oficiales, descargados y probados en F.6). Mecanismo de tema de Aura como candidato `[ESTIMADO]` (no releído en detalle, referencia cruzada a `CONTRATO-formato-tema.md`).

**Implicación para Metro-Aura:** cero riesgo legal — Selawik es SIL OFL, redistribuible en un firmware GPL v2 (mismo criterio que Inter). Nota residual: el "Reserved Font Name" del OFL solo restringe derivados con el nombre "Selawik", no su uso normal como fuente del sistema.

## F.8 — Sonido/háptica del twist

**Respuesta:** No se encontró evidencia primaria de que el Zune 30 tuviera retroalimentación sonora por defecto al girar el pad (D-pad de 4 direcciones + centro, no rueda continua). El iPod 6G sí dispara un clic de piezo por defecto (ver C.7 para el mecanismo exacto).

**Evidencia:** ausencia de mención en https://en.wikipedia.org/wiki/Zune `[ESTIMADO]` (ausencia de evidencia no es evidencia de ausencia).

**Implicación para Metro-Aura:** decisión de Fase 3. Dato a favor de desactivarlo por defecto: el pad discreto del Zune 30 no generaba el mismo feedback táctil-sonoro que el clickwheel — y es además la consecuencia automática si Metro adopta lectura cruda de botones (C.7).

## F.9 — Naming y branding

**Respuesta:** Nombre en pantalla: "Metro-Aura" (o variante corta, decisión de Fase 3) — nunca "Zune" ni "Windows Phone" como nombre de producto visible, cero logotipos de Microsoft/Zune en el árbol ni en el `.zip` distribuido (mismo criterio que Aura-Firmware aplica con material Apple). Homenaje legítimo sin riesgo: paleta de colores (F.5, valores no protegibles), tipografía libre métricamente cercana (F.7), patrones de movimiento/layout genéricos del lenguaje Metro (un lenguaje de diseño no es marca registrada). Evitar sin excepción: wordmark "Zune"/"Windows Phone", logo de Windows/Zune, el nombre "Zegoe"/"Segoe" en cualquier string de cara al usuario, iconografía Fluent/Segoe MDL2 Assets (licencia restrictiva de Microsoft, a diferencia de Lucide/Phosphor).

**Evidencia:** criterio propio, por analogía con la regla ya vigente sobre material Apple en `Aura-Firmware/CLAUDE.md` `[ESTIMADO]`.

**Implicación para Metro-Aura:** el logo de arranque (bitmap 320×98 que reemplaza `rockboxlogo`, mismo mecanismo D-052) debe ser wordmark propio de "Metro-Aura" con la tipografía de F.7, nunca recreación del logo de Zune. Puramente de producto/diseño gráfico, no bloquea nada técnico de las Fases 2-4.

---

# G. Riesgos y decisiones abiertas

Ordenado por impacto (alto → bajo). Cada punto lleva evidencia (sección.pregunta), opciones, recomendación del investigador, y a quién toca decidir.

## Alto impacto

### G.1 — Riesgo de que Studio reinstale Aura sobre un iPod con Metro-Aura
**Evidencia:** E.3. `AuraUpdateChecker` compara hash de `rockbox.ipod`; si el usuario tiene Aura Studio y conecta un iPod Metro, verá "actualización disponible" permanentemente y el flujo normal de instalación sobrescribiría Metro con Aura.
**Opciones:** (a) Metro necesita su propia app Studio/variante con su propio `rockbox.ipod` embebido; (b) modificar `AuraUpdateChecker` en `Aura-Studio/` para distinguir builds (fuera de alcance de esta sesión, exige una sesión dedicada a ese repo, ver `CLAUDE.md` de la carpeta padre); (c) aceptar el riesgo y documentarlo como advertencia al usuario.
**Recomendación:** (a) — es la única opción que no requiere tocar `Aura-Studio/` desde esta sesión ni depender de que el usuario nunca mezcle ambas apps.
**Decide:** Fase 3, posiblemente con una sesión aparte dedicada a `Aura-Studio/` si se opta por (b).

### G.2 — Identidad en disco: ¿Metro imita la estructura `.rockbox/aura/`?
**Evidencia:** E.2, D.3. Studio detecta compatibilidad por estructura de disco (`.rockbox/aura/` + `aura.cfg`), no por contenido ni nombre.
**Opciones:** (a) conservar `.rockbox/aura/` como prefijo de ruta aunque el firmware se llame/muestre "Metro"; (b) usar un prefijo propio (`.rockbox/metro/`) y perder compatibilidad automática con Studio.
**Recomendación:** (a) — sin ninguna desventaja real detectada; cambiar el prefijo rompe la detección sin ganancia.
**Decide:** Fase 3 (ratificación, no hay contraindicación encontrada).

### G.3 — Límite duro de 10 KB por fuente cargada (`MAX_FONT_SIZE`)
> **ERRATA (Fase 3):** el límite de 10 KB no existe en este target — ver errata en A.7. G.3 queda reducido a: elegir la escala tipográfica y el modo de carga (`font_load_ex` completo). Resuelto en `PLAN_MAESTRO.md` §D.
**Evidencia:** A.7, F.6, F.7. `ipod6g.h` nunca define `MEMORYSIZE` → tope duro de 10 KB por fuente, incondicional. Alfabetos completos AA a 60-72px no caben precargados, solo en modo caché-bajo-demanda (256 glifos LRU).
**Opciones:** (a) subconjuntos de caracteres acotados para tamaños gigantes (dígitos + símbolos); (b) aceptar modo caché-bajo-demanda para alfabetos completos grandes, midiendo el costo de fallo de caché en hardware real; (c) investigar redefinir `MEMORYSIZE` para `ipod6g` (riesgo: efectos colaterales en otras partes de `config.h` que la usan).
**Recomendación:** (a) para títulos/números grandes (el caso de mayor impacto visual del proyecto, F.4), (b) para listas con fuente mediana donde el alfabeto completo es necesario — evitar (c) salvo que (a)+(b) resulten insuficientes tras medir en Fase 4.
**Decide:** Fase 3, con el experimento de F.6 completado en Fase 4 antes de comprometerse al diseño final.

### G.4 — Arquitectura de navegación: bucle propio + widget de lista en C, `get_action()` vs. lectura cruda
**Evidencia:** A.1, A.2, A.5, C.4. `gui_synclist` no alcanza para el twist (altura de fila fija, sin movimiento horizontal) — confirma que la navegación exige C propio. Queda abierto si usar `get_action()`/contextos o lectura cruda de botones como Aura.
**Opciones:** ver tabla comparativa de C.4.
**Recomendación:** `get_action()`/contextos — ahorra reimplementar el filtrado de `BUTTON_REL` (ya resuelto por el patrón "prereq button") y da un mecanismo declarativo natural para la jerarquía de profundidad del twist; el costo de aprender el sistema de tablas de Rockbox es menor que el de mantener a mano toda la lógica de despacho que ya escribió y depuró Aura por otro camino.
**Decide:** Fase 3.

### G.5 — FPS objetivo y política de fallback sin datos de hardware real
**Evidencia:** B.2, B.10, B.11. Sin cifra dura de FPS medida en ipod6g; presupuesto derivado de 33-50 ms/cuadro es una hipótesis, no una medición.
**Opciones:** (a) diseñar con el presupuesto derivado como objetivo de trabajo y validar en Fase 4 con `TRANSITION_LOG`/overlay de FPS antes de comprometerse; (b) posponer cualquier decisión de FPS hasta tener hardware real disponible.
**Recomendación:** (a) — el sistema de niveles de FX de 2 ejes de Aura (B.10) ya da un mecanismo de sustracción/fallback probado; usarlo desde el diseño inicial evita tener que retrofit-earlo después.
**Decide:** Fase 3 (diseño de la matriz de niveles), Fase 4 (medición real y ajuste de umbrales).

### G.6 — Marcador de sincronización: implementar desde el día uno
**Evidencia:** E.4. Sin `sync_marker_supported`, Studio borra `.tcd` y fuerza reconstrucción completa en cada sync — sin costo cuantificado para bibliotecas grandes.
**Opciones:** (a) portar `aura_sync_marker.c`/`aura_sync.c` (ya clasificados INFRA REUTILIZABLE en D.2) desde el inicio; (b) depender del fallback y medir el costo real antes de decidir.
**Recomendación:** (a) — el código ya existe, está testeado (test host-side), y es barato de portar; no hay razón para depender del camino más lento.
**Decide:** Fase 3.

## Medio impacto

### G.7 — Buffers de transición (resuelto, bajo riesgo)
**Evidencia:** A.10, B.3. Arreglos estáticos en BSS de 150 KB cada uno, sin pasar por el alocador dinámico — patrón claro y ya probado.
**Recomendación:** replicar tal cual, 2-3 buffers estáticos (450 KB total, insignificante sobre 64 MB).
**Decide:** Fase 3 (ratificación).

### G.8 — Idioma de la UI
**Evidencia:** A.9. Aura usa tabla de cadenas propia en español, sin el sistema `.lang` de Rockbox.
**Opciones:** español (consistente con Aura), inglés (más fiel al Zune/WP7 original), bilingüe (tabla con selector).
**Recomendación:** sin recomendación técnica — es decisión de producto pura; el mecanismo (tabla propia sin `.lang`) es válido para cualquiera de las tres opciones.
**Decide:** Fase 3 (dueño del proyecto).

### G.9 — Piezo/clic de la rueda
**Evidencia:** C.7, F.8. Si Metro usa lectura cruda de botones, el piezo queda desactivado automáticamente sin código dedicado.
**Recomendación:** dejarlo desactivado por defecto — más fiel al pad discreto del Zune 30 y es la consecuencia natural de G.4 si se elige lectura cruda; si se elige `get_action()`, desactivar `keyclick_hardware` explícitamente.
**Decide:** Fase 3.

### G.10 — Commit base de Metro-Aura
**Evidencia:** D.7. Sin evidencia de fixes relevantes en `s5l8702`/`ipod6g`/`button-clickwheel.c` posteriores a `0726ec93` en el sondeo realizado (no exhaustivo).
**Recomendación:** sembrar en `0726ec93517a61f602679ab052b083217ec9c96d`, mismo commit que Aura — toolchain, los 27 parches auditados y el simulador ya están probados contra ese commit exacto.
**Decide:** Fase 3 (ratificación).

## Bajo impacto / detalles pendientes de resolver antes de portar

### G.11 — RAM disponible para el core UI sin cuantificar
**Evidencia:** A.10, `NO RESUELTO`. Presupuesto de RAM permanente que el core puede reservar sin competir con el buffer de audio.
**Recomendación:** medir empíricamente en Fase 4 pidiendo 2-3 buffers de 150 KB permanentes.
**Decide:** Fase 4 (medición), no bloquea el diseño de Fase 3.

### G.12 — Propósito exacto de `aura_manifest.c` sin confirmar
**Evidencia:** D.2, `NO RESUELTO`. Clasificado tentativamente INFRA REUTILIZABLE por ausencia de dependencias de diseño, pero su función de negocio exacta no se leyó completa.
**Recomendación:** releer el cuerpo completo antes de decidir si se porta, en la Fase 3 o al inicio de la Fase 4.
**Decide:** quien ejecute la Fase Cero del Plan Maestro.

### G.13 — Alcance exacto de cambios upstream posteriores a `0726ec93`
**Evidencia:** D.7, `NO RESUELTO`. Sin `git log` completo de las rutas relevantes del target entre `0726ec93` y HEAD actual del mirror (`8cfa4bfd...`).
**Recomendación:** aceptar el riesgo dado que no se encontró evidencia de un fix nombrado y relevante — solo reconsiderar si en Fase 3/4 aparece un problema concreto que un commit posterior resuelva.
**Decide:** no bloquea nada; queda como nota de vigilancia.

---

## Definición de "hecho" de la Fase 2 — verificación final

- `Metro-Aura/docs/INVESTIGACION.md` existe, secciones A–G en orden. ✅
- Cada pregunta identificada en el plan (A.1–A.12, B.1–B.11, C.1–C.7, D.1–D.8, E.1–E.11, F.1–F.9) tiene respuesta con evidencia e implicación, o `NO RESUELTO: <razón>`. ✅ (6 preguntas puntuales marcadas `NO RESUELTO` explícitamente: B.1 tiempo exacto de `lcd_update()`, B.2 FPS real en hardware, D.4 `tools/configure` no diffeado, D.7 alcance de cambios upstream, D.2 propósito de `aura_manifest.c`, E.9 orden exacto de las 3 llamadas de handoff)
- Toda afirmación cuantitativa lleva `[VERIFICADO]` o `[ESTIMADO]`. ✅
- Tablas D.1 (27 archivos), D.2 (107 archivos vía agrupación completa por categoría), E.1 (~30 filas) completas, sin "etc.". ✅
- Ningún archivo fuera de `Metro-Aura/docs/INVESTIGACION.md` fue creado o modificado dentro de los tres repos durante esta fase.
- Parada obligatoria a continuación.
