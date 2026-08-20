# Estado final — Metro-Aura v0.1.0

Cierre de la Fase 4 de ejecución (F0-F13, `docs/plans/PLAN_MAESTRO.md`).
Qué funciona, qué queda pendiente, cómo instalar, y cómo medir el
rendimiento real de las transiciones en hardware. La fuente de verdad
de las decisiones sigue siendo `DECISIONS.md`; este documento resume,
no reemplaza.

## Qué funciona (verificado en el simulador SDL, F0-F12)

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

Suite de tests de host: 251 checks, 0 fallos
(`firmware/rockbox/apps/metro/test/`, `make -C apps/metro/test`).
Ambos builds (`firmware/tools/build_sim.sh`, `firmware/tools/build_target.sh`)
compilan limpio, sin advertencias, en cada fase desde F0.

## Qué NO está verificado (pendiente de hardware real)

Todo lo de arriba se verificó **exclusivamente en el simulador SDL**
(`PLAN_MAESTRO.md` regla transversal #6: "simulador primero"). Nunca
se conectó un iPod Classic 6G real durante F0-F13. En particular:

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

## Qué reportar tras flashear (primera sesión con hardware real)

1. **Arranque**: ¿llega a la pantalla de música/videos/fotos/ajustes
   sin errores? ¿Se ve el splash con el wordmark?
2. **Navegación**: twist entre pivots, profundizar/volver, todos los
   gestos de la tabla de `PLAN_MAESTRO.md` §2.3.
3. **Reproducción**: música real desde el propio dispositivo,
   carátulas, video, fotos.
4. **Sincronización con Aura Studio** (si aplica, checklist C20 de
   `docs/COMPAT_STUDIO.md`): un marcador real, escrito por Aura Studio
   de verdad, procesado correctamente al arrancar.
5. **Fluidez de las transiciones**: ¿se sienten bien a ojo? Si no,
   correr el procedimiento de medición de arriba antes de ajustar nada
   a ciegas.

Cualquier hallazgo de esta primera sesión con hardware real es
material nuevo para `docs/DESVIACIONES.md`/`DECISIONS.md`, siguiendo
el mismo formato que el resto del proyecto.
