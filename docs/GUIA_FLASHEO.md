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

> **Salvedad confirmada en hardware (2026-08-23, ST-050 de Aura
> Studio):** el dual boot **solo funciona en un iPod en formato
> "winpod"** (restaurado con iTunes en Windows). Un iPod restaurado
> desde Mac usa un esquema de particiones que el bootloader no lee, y
> la combinación MENU **no arranca Apple** aunque el flasheo haya sido
> dual. En ese caso la única diferencia real entre `--bl-inst` y
> `--bl-inst --single` es que el segundo lo dice de frente. Aura Studio
> ya instala siempre en modo single por esto.

## Advertencia: riesgo con Aura Studio (`DECISIONS.md` D-021, heredado de Metro M-004)

Si tienes **Aura Studio** instalado (la app hermana de este proyecto)
con el firmware de Aura embebido, y conectas un iPod con moonlit.aura:

- Un Studio **anterior a ST-045** compara hashes de `rockbox.ipod`
  (`AuraUpdateChecker`) — no van a coincidir, y puede ofrecerte
  "actualizar", lo que **sobrescribiría moonlit.aura con Aura** si
  aceptas.
- Un Studio **con ST-045 o posterior** distingue familias por la clave
  `firmware_family` de `aura.cfg`, pero **no conoce la familia
  `moonlit`** hasta que se ejecute el cambio descrito en
  `docs/plans/PROMPT-aura-studio.md` (D-017). Mientras tanto puede
  ofrecerte volver a Aura **o a Metro**, y no ofrece actualizaciones de
  moonlit.

moonlit.aura escribe `firmware_family: moonlit` en `aura.cfg`
(`CONTRATO-moonlit-studio.md` §A.1) precisamente para que ese Studio
futuro lo reconozca. Hasta que exista: **no aceptes una actualización
ni un cambio de familia desde Aura Studio sobre un iPod con moonlit.aura
instalado**. No hay mitigación posible desde este repositorio.

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

   **Hazlo desde Terminal, no arrastrando en Finder:**
   ```
   unzip -o rockbox.zip -d /Volumes/NOMBRE_DEL_IPOD/
   ```

   Eso es todo: el zip ya trae `rockbox.ipod` dentro de `.rockbox/`,
   que es donde el bootloader lo busca primero. **No** hace falta
   copiarlo aparte a la raíz.

   > **Por qué Terminal y no Finder.** `.rockbox` empieza con punto,
   > así que **Finder la esconde**. Al descomprimir y arrastrar es muy
   > fácil terminar con la carpeta creada pero vacía. Le pasó al dueño
   > en el primer flasheo real: el iPod arrancó igual —el bootloader
   > cae a `/rockbox.ipod` en la raíz si no encuentra el de
   > `.rockbox/`— pero sin fuentes, sin códecs y sin plugins. El
   > síntoma visible fue solo "las tipografías se ven mal"; la música
   > directamente no podía sonar.

6. **Verifica antes de expulsar** — dos comandos que atrapan justo ese
   fallo:
   ```
   ls /Volumes/NOMBRE_DEL_IPOD/.rockbox/fonts/metro-*.fnt | wc -l   # 5
   ls /Volumes/NOMBRE_DEL_IPOD/.rockbox/codecs/*.codec | wc -l      # 43
   ```
   Si dan 0, la extracción no llegó: repite el paso 5.

7. **Expulsa el disco con el botón de expulsar** —no desconectes el
   cable a secas— y reinicia el iPod. Sin ningún botón presionado,
   debería arrancar directo a Metro-Aura.

   > Arrancar el cable en caliente sobre un FAT montado puede dejar el
   > sistema de archivos inconsistente. En el primer flasheo real eso
   > terminó con un volumen que ya no montaba (`fsck_msdos`: *could not
   > read boot block*).

**Modo single-boot** (destruye el arranque original de Apple —
**no recomendado** salvo estar seguro de que no lo necesitas): agrega
`--single` al comando de instalación del paso 4
(`./mks5lboot --bl-inst --single bootloader-ipod6g.ipod`).

## Actualizar una instalación existente

Repite los pasos 3-7 con los archivos nuevos — instalar el bootloader
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
- **"El volumen ya no monta"** (`fsck_msdos: could not read boot
  block`, o Finder que no lo ve pero `diskutil list` sí muestra la
  partición): casi siempre es un FAT dejado inconsistente por
  desconectar el cable sin expulsar. Primero **desconecta y reconecta
  con otro cable y otro puerto** — el conector de 30 pines y los cables
  de iPod fallan mucho, y un boot block "ilegible" suele volver a
  leerse al reestablecer el enlace. Si monta, expúlsalo bien y corre
  `diskutil verifyVolume` antes de nada. Si no monta ni con cable
  nuevo, el disco se puede reformatear a FAT32 y reinstalar desde cero
  (la biblioteca vive en Aura Studio, el iPod es una réplica). Y si
  vuelve a pasar tras un ciclo limpio, sospecha del disco antes que del
  software: en un 6G original es un disco de 1.8" de ~2007.
- **"Olvidé la clave del candado de pantalla"**: el aparato **no**
  queda inservible y no hace falta reflashear nada. Conecta el cable
  (el USB sigue funcionando con el candado puesto, a propósito), abre
  `.rockbox/aura/aura.cfg` en el disco del iPod, borra las dos líneas
  `screen_lock: 1` y `screen_lock_pin: ####`, guarda y expulsa. El
  candado desaparece al terminar la sesión USB, sin reiniciar
  siquiera. Explicación completa y sus límites (es un candado de
  interfaz, no de datos) en `docs/ESTADO_FINAL.md`.
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
