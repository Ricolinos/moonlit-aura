# Guía de flasheo y restauración — iPod Classic 6G

Metro-Aura **no tiene un instalador con interfaz gráfica** (a diferencia
de Aura, que se instala desde Aura Studio) — la instalación es manual,
por línea de comandos, con la herramienta `mks5lboot` que trae el
propio fork de Rockbox. Esta guía cubre el procedimiento completo:
qué instala, cómo entrar a modo DFU, los comandos exactos, y cómo
restaurar el iPod a su estado original.

Portado de `Aura-Firmware/docs/guia-flasheo-restauracion.md`
(`DECISIONS.md` M-046 y ss.) y adaptado: esa guía delega el paso a
paso a Aura Studio (un instalador aparte); esta lo explica completo,
porque Metro-Aura no tiene ese instalador.

## Qué instala Metro-Aura, exactamente

Metro-Aura usa un **bootloader dual-boot** (heredado de Rockbox,
`bootloader/ipod-s5l87xx.c`, compilado en `firmware/dist/bootloader-ipod6g.ipod`)
que reemplaza el arranque de NOR del iPod, pero **no borra ni toca el
firmware original de Apple** — decide a cuál arrancar según qué
botones mantengas presionados en el momento del arranque:

| Combinación | Resultado |
|---|---|
| (nada) | Arranca Metro-Aura |
| MENU (mantenido ~800ms) | Arranca el firmware original de Apple |
| SELECT + MENU (~5s, reset; +8s más → modo DFU) | Reinicia / entra a modo DFU |
| SELECT + LEFT (~7s) | Diagnósticos de Apple |
| SELECT + PLAY (~7s) | Modo disco de Apple |
| SELECT + RIGHT | Modo Bootloader USB (para copiar archivos) |
| Interruptor Hold activado al encender | Firmware original de Apple |

Este es el comportamiento estándar de los bootloaders duales de
Rockbox para iPod (`firmware/rockbox/utils/mks5lboot/README`,
sección "Dual-Boot") — Metro-Aura lo hereda sin modificarlo.

## Advertencia: riesgo con Aura Studio (`DECISIONS.md` M-004)

Si tienes **Aura Studio** instalado (la app hermana de este proyecto)
con el firmware de Aura embebido, y conectas un iPod con Metro-Aura,
`AuraUpdateChecker` de esa app compara hashes — no van a coincidir, y
puede ofrecerte "actualizar", lo que **sobrescribiría Metro-Aura con
Aura** si aceptas. Metro-Aura escribe una clave propia
(`firmware_family: metro` en `aura.cfg`) para que un futuro Aura
Studio pueda reconocerlo y no ofrecer esa acción, pero esa lectura del
lado de Aura Studio no existe todavía (trabajo en ese otro
repositorio, fuera de alcance aquí) — **no aceptes una actualización
de Aura Studio sobre un iPod con Metro-Aura instalado** hasta que ese
soporte exista.

## Requisitos previos

- iPod Classic 6G, con el firmware original de Apple instalado y
  arrancando (versión 2.0.4 o 2.0.5 según el modelo — ver
  `firmware/rockbox/utils/mks5lboot/README`).
- Disco formateado en FAT32 (no APFS/HFS+ ni el formato de partición
  propietario de Apple) — si no lo está, convertirlo primero
  (`http://www.rockbox.org/wiki/IpodConversionToFAT32`).
- macOS/Linux/Windows con `libusb` (en macOS, `brew install libusb` si
  hace falta) — `mks5lboot` ya viene compilado en el Release.
- Si tienes iTunes instalado y corriendo: ciérralo, y si el proceso
  `iTunesHelper` sigue activo, suspéndelo (`ps x | grep iTunesHelper`,
  luego `kill -STOP <PID>`) antes de entrar a modo DFU — reanúdalo
  después con `kill -CONT <PID>` una vez terminado el flasheo.

## Instalación paso a paso

1. **Descarga el Release** (o corre `firmware/tools/package_dist.sh`
   para generarlo desde el código fuente) — necesitas
   `bootloader-ipod6g.ipod`, `rockbox.ipod`, `rockbox.zip`,
   `mks5lboot` y `checksums.txt`, todos en `firmware/dist/`.

2. **Verifica los checksums** antes de continuar:
   ```
   cd firmware/dist
   shasum -a 256 -c checksums.txt
   ```
   Si alguno no coincide, no sigas — vuelve a descargar o recompilar
   desde cero (ver `docs/guia-desarrollo.md`).

3. **Entra a modo DFU**: mantén presionados SELECT + MENU por unos
   ~12 segundos seguidos (el dispositivo resetea a los ~5s, sigue
   sosteniendo hasta que la pantalla se ponga negra). Puedes confirmar
   que entró correctamente corriendo, en otra terminal:
   ```
   ./mks5lboot --dfuscan --loop
   ```
   (Ctrl-C para salir del loop una vez que veas `[INFO] DFU device
   state: 2`.)

4. **Instala el bootloader dual-boot**:
   ```
   ./mks5lboot --bl-inst bootloader-ipod6g.ipod
   ```
   El dispositivo emite un tono de "vivo" (2000Hz/100ms) al recibir la
   imagen. Si la instalación sale bien, suena un tono doble
   (1000Hz/100ms + 2000Hz/150ms) y el dispositivo reinicia solo. Si
   algo sale mal, suena un tono grave (330Hz/500ms) — ver "Problemas
   comunes" abajo.

5. **Copia el firmware al disco**: tras el reinicio, vuelve a
   encender el iPod manteniendo SELECT + RIGHT para entrar al modo
   Bootloader USB — el dispositivo se monta como un disco USB normal.
   Copia:
   - `rockbox.ipod` a la raíz del disco.
   - El contenido de `rockbox.zip` (la carpeta `.rockbox/`) también a
     la raíz, reemplazando si ya existe.

6. **Expulsa el disco de forma segura** y reinicia el iPod. Sin
   ningún botón presionado, debería arrancar directo a Metro-Aura.

**Modo single-boot** (destruye el arranque original de Apple —
**no recomendado** salvo estar seguro de que no lo necesitas): agrega
`--single` al comando de instalación del paso 4
(`./mks5lboot --bl-inst --single bootloader-ipod6g.ipod`).

## Actualizar una instalación existente

Repite los pasos 3-5 con los archivos nuevos — instalar el bootloader
de nuevo sobre uno ya instalado simplemente lo actualiza (mismo
mecanismo, sin necesidad de desinstalar primero).

## Restaurar el iPod original

Para quitar el bootloader dual-boot y volver a arrancar directo al
firmware original de Apple (sin rastro de Metro-Aura en el arranque):

1. Entra a modo DFU (paso 3 de arriba).
2. Corre:
   ```
   ./mks5lboot --bl-uninst ipod6g
   ```

La música/fotos/videos que hayas sincronizado **no se borran solos**
del disco (quedan en `Music/`, `Videos/`, `Photos/` del volumen) —
bórralos a mano si quieres limpiar el disco por completo.

## Problemas comunes (nivel dispositivo/bootloader)

- **"No se detecta el iPod en modo DFU"**: la combinación de botones
  es sensible al timing — soltar antes de tiempo (antes de los ~12
  segundos y de que la pantalla se ponga negra) no llega a activar el
  DFU. Reintenta desde cero (desconectar, reconectar, repetir la
  combinación completa).
- **"El iPod no está formateado en FAT32"**: Metro-Aura (como todo
  Rockbox) necesita FAT32, no HFS+/APFS ni el formato de partición
  propietario de Apple viejo.
- **Un checksum no coincide**: no continúes — probablemente los
  archivos se corrompieron al descargar. Vuelve a descargarlos, o
  recompila desde cero (`docs/guia-desarrollo.md`).
- **No hay driver DFU válido (Windows)**: usa Zadig
  (`http://zadig.akeo.ie/`) para instalar un driver WinUSB o libusbK
  — los drivers libusb-win32 (libusb0) no funcionan con `mks5lboot`.
- **Acceso USB denegado**: corre `mks5lboot` con privilegios elevados
  (`sudo` en macOS/Linux, Administrador en Windows).
- **Si algo sale mal a mitad del flasheo del bootloader**: un tono
  grave repetido (330Hz) indica que el NOR se corrompió y hace falta
  restaurar vía iTunes/Finder (modo de recuperación de Apple). Esto es
  infraestructura de Rockbox, no específica de Metro-Aura, y es la
  misma que usan miles de instalaciones de Rockbox en iPods S5L desde
  hace años.

## Qué está verificado y qué no

Todo lo de este documento (mecanismo de dual-boot, comandos de
`mks5lboot`, procedimiento de copia) es infraestructura heredada de
Rockbox/Aura-Firmware, con años de uso real en hardware — no algo que
esta sesión de desarrollo haya podido probar contra un iPod físico
(F0-F12 corrieron enteramente en el simulador SDL, ver
`docs/ESTADO_FINAL.md` para el detalle completo de qué se verificó
dónde). El firmware y el bootloader compilan limpio para hardware real
(`firmware/tools/build_target.sh`) y sus checksums quedan verificados
en cada empaquetado, pero el flasheo real contra un dispositivo físico
— y todo lo que solo se puede confirmar ahí (arranque real, fluidez de
las transiciones de F11/F12, sincronización con Aura Studio) — queda
pendiente de que el dueño del proyecto lo pruebe y reporte.
