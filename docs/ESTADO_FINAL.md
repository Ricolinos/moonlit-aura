# Estado final — Metro-Aura v0.3.0 (+ ronda 4 sin publicar)

**Lo publicado es `v0.3.0`.** El árbol trae además la **ronda 4**
completa salvo dos frentes bloqueados — todo lo de la sección "Lo que
agregó la ronda 4" está en `main` pero **todavía no tiene release**.

Cierre de la **ronda 3**. Acumula las tres rondas de ejecución: la
ronda 1 construyó el firmware (F0-F13, `docs/plans/PLAN_MAESTRO.md`),
la ronda 2 lo pulió y le agregó fotos/video de verdad
(`PLAN-metro-r2-maestro.md`), y la ronda 3 cerró el backlog
(`PLAN-metro-r3-maestro.md`). Qué funciona, qué queda pendiente, cómo
instalar, y cómo medir el rendimiento real en hardware. La fuente de
verdad de las decisiones sigue siendo `DECISIONS.md`; este documento
resume, no reemplaza.

## Qué funciona (verificado en el simulador SDL)

- **Navegación twist completa** (F3): pila de navegación por
  profundidad + pivot horizontal, mapeo de gestos del clickwheel
  fiel al Zune 30 (`docs/plans/PLAN_MAESTRO.md` §2.3), aceleración de
  rueda (`button_apply_acceleration`).
- **Música** (F4-F5): artistas/álbumes/canciones/géneros/playlists
  reales sobre tagcache, en los tres layouts de biblioteca del
  contrato con Aura Studio; reproducción, Now Playing con carátula
  real (folder art + JPEG embebido), transporte, opciones (aleatorio/
  repetir/up next).
- **Compatibilidad con Aura Studio** (F6): `aura.cfg` desde el primer
  arranque, procesamiento del marcador de sincronización
  (`sync-pending.json`), `device.cfg`, aplicación de `rtc_sync_*`.
  Checklist completo en `docs/COMPAT_STUDIO.md`.
- **Videos y fotos** (F7): listado real de `/Videos`/`/Photos`, pivots
  de categoría condicionales según `video_categories.cfg`/
  `photo_categories.cfg`, reproducción vía `mpegplayer`/`imageviewer`.
- **Ajustes reales** (F8): idioma (ES/EN, recarga inmediata), tema
  claro/oscuro, acento (10 colores WP7), brillo, retroiluminación,
  actualizar/reconstruir biblioteca, restablecer ajustes, About
  (`device.cfg`, `sync_summary.cfg`, versión).
- **Superficies de Rockbox restiladas** (F9): splash de arranque con
  traducción ES/EN, pantalla de apagado, pantalla de conexión USB
  propia (con la limitación real documentada en `docs/DESVIACIONES.md`
  F9-1: no llega a verse en el simulador, sin confirmar en hardware).
- **Pulido visual** (F10): letra de índice flotante en scroll rápido,
  estados vacíos, iconos geométricos reales (batería/aleatorio/
  repetir), recorte de títulos largos.
- **Motor de transiciones completo** (F11-F12): SLIDE (twist entre
  pivots), PUSH/POP (slide o turnstile según nivel), FADE (Now
  Playing/plugins, slide o blend real según nivel), FEATHER (cascada
  de filas), fondo de carátula atenuada en Now Playing — los cuatro
  bajo la matriz `animations` (off/minimal/all) × `graphics`
  (lite/full), ajustable desde Ajustes → General, con auto-degradación
  de sesión (M-015) y `METRO_TRACE` instrumentando cada transición.

### Lo que agregó la ronda 2

- **Miniaturas reales de fotos** (R2-F2): `/Photos` en cuadrícula de
  tiles con miniaturas decodificadas y cacheadas en disco.
- **Visor de fotos** (R2-F3): pantalla completa, ajustar/rellenar,
  navegación entre fotos de la misma categoría.
- **OSD de video estilo Zune HD** (R2-F4).
- **`version.txt` en los releases** (R2-F1), para que Aura Studio pueda
  distinguir versiones.

### Lo que agregó la ronda 3

- **Letras `.lrc` sincronizadas** (R3-F2): modo de pantalla completa en
  Now Playing, con parser propio de bajo consumo (11.6 KB vs los ~80 KB
  del enfoque de Aura-Firmware) y la línea activa avanzando sola.
- **Fotos de artista** (R3-F3): el pivot Artistas pasa a cuadrícula,
  consumiendo el `artist_images.cfg` que escribe Aura Studio.
- **Quickplay** (R3-F4): primer pivot de Música, los álbumes
  reproducidos más recientemente con su carátula real. Requirió
  encender el runtime DB de Rockbox, que estaba apagado y sin ninguna
  forma de activarlo desde Metro.
- **Calificaciones** (R3-F5): import de una vía desde `ratings.cfg` de
  Studio, más una fila de 5 estrellas en las opciones de Now Playing.
  La asimetría (lo que califiques en el iPod se pierde en el siguiente
  sync) está documentada en `docs/COMPAT_STUDIO.md` C26, no escondida.
- **Temporizador de sueño y presets de EQ** (R3-F6): dos filas nuevas
  en Ajustes, sin pantallas propias ni cambios al core.
- **Candado de pantalla** (R3-F7): clave de 4 dígitos con la rueda —
  ver su propia sección más abajo, incluida la salida de emergencia.
- **CONTINUUM** (R3-F8): al entrar a un álbum o artista, su nombre
  vuela desde la fila hasta la ceja de la página nueva mientras el
  resto gira.
- **Un motor de miniaturas compartido** (R3-F1) en vez de tres copias
  del mismo módulo, y el volcado de la cola de tagcache al apagar
  (R3-F4), que faltaba y perdía historial de reproducción en silencio.

### Lo que agregó la ronda 4

Ronda de correcciones sobre hardware real: el dueño flasheó por primera
vez y reportó nueve frentes. Siete se cerraron; dos siguen bloqueados
(ver abajo).

- **Iconografía Fluent** (FA-1): Metro no tenía *ningún* pipeline de
  iconos — sus cuatro iconos eran trazos geométricos a mano. Se
  construyó uno (`gen_icons.py` → tabla C commiteada, mismo patrón que
  la tabla del turnstile) y se adoptó Fluent System Icons (MIT).
  Reproducible y sin red: los SVG se versionan.
- **Los residuales de macOS dejan de contaminar las listas** (FA-2):
  `._IMG_1234.jpg` conserva la extensión y pasaba el filtro por sufijo.
  También afectaba a las playlists, cosa que no se había reportado.
- **Español impecable de verdad** (FA-5a): **ninguna** cadena en español
  llevaba acentos. 30 corregidas, incluida `"temporizador de sueno"`,
  que no era un typo sino otra palabra.
- **Cuadrícula de álbumes con carátula real** (FA-5b), reutilizando el
  motor de Quickplay en vez de duplicarlo.
- **Indicador de reproducción/pausa** (FA-6): antes la pantalla quedaba
  idéntica al pausar, salvo que el tiempo dejaba de avanzar.
- **El fondo del reproductor se separa del tile** (FA-7): foto del
  artista si la hay, si no la carátula, si no fondo plano.
- **PLAY funciona desde cualquier pantalla** (FA-8), no solo dentro de
  Now Playing.
- **Búsqueda con rampa** (FA-9): el salto era fijo de 5 s por evento.
- **Ordenamiento con acentos** y **rótulo del tile seleccionado**, dos
  consecuencias detectadas durante la ronda y resueltas en ella.

Bugs preexistentes que la ronda destapó y corrigió, ninguno reportado:
la inicial de una etiqueta se cortaba por **byte** (cualquier "Ángela"
o "Éxitos" renderizaba basura), y el ordenamiento mandaba las iniciales
acentuadas **detrás de la Z**.

Suite de tests de host: 2250 checks, 0 fallos
(`firmware/rockbox/apps/metro/test/`, `make -C apps/metro/test`) en 8
suites. Ambos builds (`firmware/tools/build_sim.sh`,
`firmware/tools/build_target.sh`) compilan limpio en cada fase desde F0.

## Qué NO está verificado (pendiente de hardware real)

Todo lo de arriba se verificó **exclusivamente en el simulador SDL**
(`PLAN_MAESTRO.md` regla transversal #6: "simulador primero").

**El aparato real sí se flasheó por primera vez en la ronda 4** — y de
esa sesión salieron los nueve frentes que la ronda atendió. Pero la
lista de verificación del final de este documento **sigue sin
responder**, por dos cosas que pasaron en esa misma sesión:

1. La instalación estaba **incompleta**: `.rockbox/` quedó vacía porque
   Finder no copia carpetas que empiezan con punto. El firmware
   arrancaba solo por el respaldo `/rockbox.ipod` de la raíz, sin
   fuentes, sin códecs y sin plugins. Se corrigió extrayendo el zip
   desde Terminal (ver "Cómo instalar").
2. El volumen se desconectó sin expulsión limpia y **dejó de montar**:
   `fsck_msdos` no puede leer siquiera el sector de arranque. Hasta
   resolver eso, nada del aparato es verificable.

Sigue sin verificarse en hardware, en particular:

- **Arranque real**: el bootloader dual-boot y el arranque de
  Metro-Aura en NOR/disco real nunca se probaron.
- **Fluidez de las transiciones (F11-F12)**: `INVESTIGACION.md` B.11
  ya advertía que los tiempos del simulador no son representativos del
  hardware real (SDL en una Mac moderna vs. un ARM11 sin GPU de 2007).
  `METRO_TRACE`/`logf()` (M-049) existen exactamente para esta
  medición — ver la sección de abajo.
- **Sincronización real con Aura Studio** (checklist C20 de
  `docs/COMPAT_STUDIO.md`): el flujo completo — Aura Studio escribiendo
  un marcador real en un dispositivo real y Metro-Aura procesándolo al
  arrancar — solo se probó colocando el marcador a mano en el disco
  simulado.
- **Pantalla USB** (F9-1): no se pudo confirmar si `metro_screen_usb_show()`
  llega a verse un cuadro en hardware real (en el simulador, no).
- **Batería/CPU real de cada nivel de FX**: `cpu_boost()` se llama
  correctamente (verificado por lectura de código, no por medición de
  consumo real).

## Desviaciones respecto al plan

Registro completo, fase por fase, con el razonamiento de cada una, en
`docs/DESVIACIONES.md`. Las de mayor impacto práctico:

- **F9-1 / M-038**: la pantalla USB de Metro no llega a verse ni un
  cuadro en el simulador — `gui_usb_screen_run()` toma control de
  inmediato. Sin confirmar en hardware.
- **F10-1 / M-040**: la letra de índice flotante omite la puerta
  "≥40 filas" del plan (ninguna lista real con los fixtures de esta
  sesión llega a ese tamaño) y su disparo real por scroll rápido no se
  pudo verificar — el simulador headless nunca produce aceleración de
  rueda real.
- **F11-1**: `MACT_OPTIONS` desde Now Playing usa PUSH en vez de FADE
  (es estructuralmente una `LIST` más); `MACT_HOME` con salto de
  varios niveles siempre anima un solo POP en vez de uno por nivel.
- **F12-1 / M-046**: la geometría de proyección del turnstile (eje de
  rotación, distancia focal de la cámara) es una elección de diseño
  propia, **no** confirmada contra el Turnstile real de WP7 — fácil de
  ajustar (`firmware/tools/gen_turnstile_table.py`) si en hardware real
  no se siente como el Zune HD.

## Advertencia: riesgo con Aura Studio (M-004)

**Metro-Aura no debe instalarse en un iPod ya gestionado por Aura
Studio sin revisar esto primero.** Si Aura Studio tiene el firmware de
Aura embebido y detecta un iPod con Metro-Aura, comparará hashes (no
van a coincidir) y puede ofrecer "actualizar" — aceptar esa acción
**sobrescribiría Metro-Aura con Aura**. Metro-Aura ya escribe
`firmware_family: metro` en `aura.cfg` para que un futuro Aura Studio
pueda reconocerlo y abstenerse, pero ese lado de la corrección vive en
el repositorio de Aura Studio, fuera de alcance de este proyecto — no
resuelto todavía. Ver `DECISIONS.md` M-004 y `docs/GUIA_FLASHEO.md`.

## Candado de pantalla: qué protege y cómo quitarlo si olvidas la clave

Ajustes → General → **candado** pide una clave de 4 dígitos (marcada
con la rueda) y la vuelve a pedir al arrancar el aparato.

**Es un candado de interfaz, no de datos.** Protege que alguien tome tu
iPod y curiosee tu biblioteca; **no** protege el contenido del disco.
El volumen es FAT sin cifrar, así que cualquiera que conecte el cable
lee todos tus archivos con o sin candado — Metro no difiere ni bloquea
la conexión USB mientras el candado está puesto, a propósito (ver
`docs/DESVIACIONES.md` R3-6). La clave misma se guarda **en texto
plano** en `.rockbox/aura/aura.cfg`, coherente con lo anterior: no
tendría sentido cifrarla en un disco que se lee entero de todas formas.
Tampoco hay límite de intentos ni retardo entre ellos.

### Salida de emergencia (si olvidaste la clave)

**El aparato nunca queda inservible por un PIN olvidado.** Como la
conexión USB sigue funcionando con el candado puesto:

1. Conecta el iPod a la computadora con el cable (funciona aunque la
   pantalla esté pidiendo la clave).
2. Abre `.rockbox/aura/aura.cfg` en el disco del iPod con cualquier
   editor de texto.
3. Borra estas dos líneas:
   ```
   screen_lock: 1
   screen_lock_pin: ####
   ```
4. Guarda, expulsa el disco y desconecta el cable.

El candado desaparece **en ese momento** — Metro relee el archivo al
terminar la sesión USB, así que ni siquiera hace falta reiniciar. Si
prefieres reiniciar, también funciona.

Metro además **falla abierto** si el archivo queda dañado: un
`screen_lock: 1` sin su línea de clave, o una clave que no sean
exactamente 4 dígitos, se tratan como "sin candado". Un `aura.cfg`
corrupto no puede dejarte fuera del aparato.

## Cómo instalar

Ver `docs/GUIA_FLASHEO.md` — procedimiento completo con `mks5lboot`
(Metro-Aura no tiene un instalador con interfaz gráfica). Los
artefactos se generan con:

```
firmware/tools/package_dist.sh
```

que produce `firmware/dist/{rockbox.ipod,rockbox.zip,bootloader-ipod6g.ipod,mks5lboot,checksums.txt,MODIFICATIONS.md,THIRD-PARTY-NOTICES.txt}`.

## Cómo medir el rendimiento real de las transiciones en hardware

`metro_transitions.c` instrumenta cada transición con `METRO_TRACE`
(M-049): cuántos cuadros corrió y en cuántos ticks reales, comparado
contra su presupuesto (`frames × frame_delay`). En el simulador esto
va a `stderr` (`DEBUGF`); en un iPod real, al buffer circular de
`logf()` que trae Rockbox de fábrica — visible desde el propio
dispositivo, sin cable ni depurador:

1. Con Metro-Aura arrancado y navegando con `animations=all` (el
   default), provoca varias transiciones (entrar a un pivot, twist
   entre pivots, entrar/salir de Now Playing).
2. Desde cualquier pantalla, entra al menú de depuración de Rockbox
   (`?`/combinación de fábrica del target, fuera del alcance de la UI
   propia de Metro — es infraestructura heredada) y elige
   **"Show Log File"**.
3. Cada línea `metro_transitions: <nombre> N cuadros en T ticks
   (budget B)` es una transición real. `T` muy por encima de `B`
   (más del doble, tres veces seguidas) dispara la auto-degradación de
   sesión de M-015 por sí sola — verás
   `metro_transitions: auto-degrade: effective animations level now N`
   en el log cuando eso pase.

**Si el hardware real resulta consistentemente más lento que el
presupuesto** (`T` muy por encima de `B` de forma sostenida, no solo
ocasional), el ajuste vive en `firmware/rockbox/apps/metro/metro_transitions.c`,
función `anim_level_spec()` — bajar `frames` y/o subir `frame_delay`
para el nivel afectado. Es un cambio de una tabla de dos números por
nivel, no una reestructuración; commitéalo aparte (`F13: adjust
transition frame budget from hardware measurement` o similar) una vez
confirmado.

## Lista de verificación en hardware real (sin responder todavía)

**Esta sección es el pendiente más viejo del proyecto.** Tres rondas
completas se verificaron solo en el simulador. Lo de abajo es para
llenarse **en el iPod real**, con una respuesta escrita por punto —
"sí", "no", o qué pasó exactamente. Cualquier hallazgo es material
nuevo para `docs/DESVIACIONES.md`/`DECISIONS.md`, con el mismo formato
que el resto del proyecto.

### Base (rondas 1-2)

| # | Qué probar | Resultado |
|---|---|---|
| H1 | **Arranque**: bootloader dual-boot, splash con wordmark, llega al hub | _(sin responder)_ |
| H2 | **Navegación**: twist entre pivots, profundizar/volver, todos los gestos de `PLAN_MAESTRO.md` §2.3 | _(sin responder)_ |
| H3 | **Reproducción**: música del propio dispositivo, carátulas reales, transporte | _(sin responder)_ |
| H4 | **Video y fotos**: `mpegplayer`/visor propio, miniaturas de la cuadrícula | _(sin responder)_ |
| H5 | **Pantalla USB** (F9-1): ¿se ve un cuadro de la pantalla propia de Metro, o `gui_usb_screen_run()` la tapa como en el simulador? | _(sin responder)_ |
| H6 | **Fluidez de transiciones**: ¿se sienten bien a ojo? Si no, correr la medición de la sección anterior **antes** de ajustar nada a ciegas | _(sin responder)_ |
| H7 | **Turnstile** (F12-1): ¿la geometría de proyección se siente como el Zune HD? Es una elección propia, sin confirmar | _(sin responder)_ |
| H8 | **Sync real con Aura Studio** (C20): marcador escrito por Studio de verdad, procesado al arrancar | _(sin responder)_ |
| H9 | **Batería/CPU por nivel de FX**: consumo real con `animations=all` vs `off` | _(sin responder)_ |

### Nuevo en la ronda 3

| # | Qué probar | Resultado |
|---|---|---|
| H10 | **Letras `.lrc`**: una pista con su `.lrc` al lado; ¿la línea activa avanza sola y en tiempo? | _(sin responder)_ |
| H11 | **Fotos de artista**: con un `artist_images.cfg` real de Studio, ¿la cuadrícula muestra las fotos? | _(sin responder)_ |
| H12 | **Quickplay + escritura de historial**: reproducir tres álbumes, **apagar bien** el aparato, encender, y ver si el orden sobrevivió. Es la prueba real del volcado de la cola de tagcache al apagar (R3-4 en `DESVIACIONES.md`), que en el simulador solo se pudo verificar por la vía indirecta | _(sin responder)_ |
| H13 | **Calificaciones**: `ratings.cfg` de Studio importado tras un sync real; calificar en el aparato y confirmar que sobrevive un apagado | _(sin responder)_ |
| H14 | **Temporizador de sueño**: armar 15 min y confirmar que el aparato **se apaga solo** — en el simulador solo se verificó que el temporizador queda armado, no que dispare | _(sin responder)_ |
| H15 | **Presets de EQ**: ¿se **oyen** distintos? En el simulador solo se verificó por inspección de estado, nunca por oído | _(sin responder)_ |
| H16 | **Candado**: configurar clave, apagar, encender, desbloquear. Y probar la salida de emergencia de verdad (borrar las dos líneas por USB) | _(sin responder)_ |
| H17 | **CONTINUUM**: ¿el título volador se lee bien a 30 fps reales, o se ve a saltos? La curva (`OUT_QUAD`) y el punto de cambio de fuente son fáciles de ajustar si no | _(sin responder)_ |

### Nuevo en la ronda 4

| # | Qué probar | Resultado |
|---|---|---|
| H18 | **Iconos Fluent a 16 px en el panel real**: ¿se leen play/pausa/aleatorio/repetir/altavoz, o se empastan? El simulador escala distinto que el LCD | _(sin responder)_ |
| H19 | **PLAY global**: pausar y reanudar desde el hub y desde una lista, con el botón físico | _(sin responder)_ |
| H20 | **Rampa de búsqueda**: sostener LEFT/RIGHT. ¿Se siente fluido? Es lo único de la ronda que el simulador **no puede** probar (su inyector nunca llega a `BUTTON_REPEAT`) | _(sin responder)_ |
| H21 | **`audio_ff_rewind()` en disco real**: ¿el re-seek del búfer añade su propio tirón, aparte del tamaño del paso? Hipótesis no confirmada | _(sin responder)_ |
| H22 | **Acentos en el panel real**: `música`, `álbumes`, `sueño`, `¿…?` — ¿se ven bien a 48 px y a 14 px? | _(sin responder)_ |
| H23 | **Fondo del reproductor**: las cuatro filas de la tabla, con biblioteca real | _(sin responder)_ |
| H24 | **Residuales**: conectar por USB, dejar que macOS escriba sus `._*`, y confirmar que no aparecen en Fotos/Videos/listas | _(sin responder)_ |
| H25 | **Rótulo del tile**: ¿se lee sobre carátulas claras y oscuras? | _(sin responder)_ |

### Lo que solo el hardware puede decir

Tres cosas de la ronda 3 están verificadas **por vía indirecta** en el
simulador y merecen atención especial en la primera sesión real:

1. **El volcado de tagcache al apagar** (H12) — en el simulador se
   comprobó gracias a que la cola se desbordó sola por volumen, no por
   un apagado limpio. El apagado real es el caso que importa.
2. **El disparo del temporizador de sueño** (H14) — nunca se dejó
   correr 15 minutos.
3. **El EQ por oído** (H15) — el simulador no tiene salida de audio
   representativa.
4. **La rampa de búsqueda** (H20) — el inyector de botones del
   simulador hace press-release corto y **nunca llega a
   `BUTTON_REPEAT`**, así que la rampa está cubierta por tests de host
   pero jamás se ha ejercitado sosteniendo un botón de verdad.
