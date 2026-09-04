# Contrato entre `moonlit-aura` y Aura Studio

**Versión 7 — 2026-09-04 (ronda "ajustes 2", D-079).** Cambio respecto
a v6: referencia al contrato canónico **v19** (redactado por
Aura-Firmware en paralelo); §A.12 nueva, ajustes compartidos entre
familias `/.aura/settings.cfg` (bloqueo, brillo, apagado, idioma,
apariencia…) — mismo patrón de propiedad que `/.aura/art/format.txt`
(§A.11): fuera de `.rockbox/`, de los firmwares, Studio nunca lo toca
ni lo borra. La corrección de alcance de v19 sobre `rtc_sync_*` (§D.4
del contrato canónico: ahora en cada sincronización, no solo "Aura
corriendo") no cambia nada de este lado — moonlit ya leía esas siete
claves en el mismo punto de siempre (arranque y vuelta de USB), solo
que ahora Studio las escribe con más frecuencia.

**Versión 6 — 2026-09-04 (D-072).** Cambio respecto a v5: moonlit deja
de escribir `/.aura/thumbs/photos` — la maestra de una foto ya mide
80 px, exactamente el lado del tile, así que el `.mth` era una copia
byte a byte y la rejilla ahora lee la maestra directo. **Compatible en
las dos direcciones**: Metro puede seguir escribiendo ese directorio y
moonlit lo ignora; un `/.aura/thumbs/photos` heredado no estorba (y la
purga de `format.txt`, §A.11, se lo lleva igual). Álbumes y artistas
**sí** siguen usando `.mth`: su maestra es de 130 px y hay que reducirla
a 80, así que ahí la caché L2 evita trabajo real.

**Versión 5 — 2026-09-03 (D-062/D-063).** Cambios respecto a v4:
referencia al contrato canónico **v18** (redactado por Aura-Firmware en
paralelo); §A.11 gana `format.txt` (versión de formato del árbol de
caché derivada, con purga única al arrancar) y la clave de álbum pasa a
llevar el `mtime` de la `cover.jpg` hermana además del de la pista
representativa; §A.10 hereda esa misma clave. Studio no toca ni borra
`format.txt` por separado — sigue valiendo la regla de "borra el
directorio entero al forzar un rebuild".

**Versión 4 — 2026-08-26 (v0.1.5, D-059).** Cambios respecto a v3:
referencia al contrato canónico **v16** (caché maestra de imagen
compartida entre las tres familias, redactado por Aura-Firmware en
paralelo); §A.11 nueva, caché maestra compartida `/.aura/art/`; §A.4
aclara que `moonlitcache/art/` ya solo contiene la bandera
`.gc-pending` (el `.pfraw`/`.none` privado que documentaba se retiró
con la maestra); §A.10 aclara que las miniaturas ahora se DERIVAN de la
maestra en vez de decodificarse por separado.

**Versión 3 — 2026-08-26 (v0.1.2, D-054/D-055).** Cambios respecto a v2:
referencia al contrato canónico **v15** (base tagcache y miniaturas
compartidas entre familias, redactado por Aura-Firmware en paralelo);
§A.9 base compartida `/.aura/tagcache/` con sello `db_stamp.txt`; §A.10
miniaturas compartidas `/.aura/thumbs/`; §A.4 aclara que `moonlitcache/`
ya solo contiene `art/`.

**Versión 2 — 2026-08-26 (release v0.1.0, D-046/D-047/D-048).**
Cambios respecto a v1: referencia al contrato canónico **v14** (tres
familias, §A bis "registro de familias"); §A.3 "una fila por hermana";
§A.7 fija el primer tag `v0.1.0`; §B deja el marcador del SHA-256 del
release; §C declara el prefijo `moonlit.` de `FIRMWARE_VERSION`, unifica
el owner como `Ricolinos/moonlit-aura` y fija el centinela sin barra
inicial (tal como lo consume Studio).

**Versión 1 — 2026-08-25 (hito H1, D-001/D-002/D-017/D-021/D-023).**

Este archivo **no copia** los contratos canónicos de `Aura-Firmware`; los
**referencia** y declara cómo moonlit.aura los cumple. moonlit.aura
consume esos contratos sin modificarlos (regla heredada de Metro-Aura,
`CLAUDE.md` Metro:66-71): el contrato de datos es inmutable desde este
lado.

Contratos referenciados (fuente canónica, se leen desde el repo hermano):

| Contrato | Versión referenciada | Ruta |
|---|---|---|
| Firmware ↔ Studio | **v19** (2026-09-04; v16 + formato/purga de `/.aura/art/` (v18, D-063) + ajustes compartidos `/.aura/settings.cfg` y alcance de `rtc_sync_*` corregido (v19, D-079)) | `Aura-Firmware/CONTRATO-firmware-studio.md` |
| Nombre del dispositivo | **v2** (2026-08-17) | `Aura-Firmware/CONTRATO-dispositivo.md` |
| Estructura de biblioteca | **v1.3** (2026-08-18) | `Aura-Firmware/docs/contracts/library-layout-v1.md` |

Cualquier cambio a este archivo sube su número de versión. Los cambios
en Studio que aquí se requieren (§C) **no** se ejecutan desde este repo
(D-017): viven en `docs/plans/PROMPT-aura-studio.md`.

---

## §A — Lo que moonlit.aura garantiza

1. **`/.rockbox/aura/aura.cfg`** se regenera entero en cada guardado
   (`apps/metro/metro_settings.c`, `metro_settings_save()`, C2) y
   contiene:
   - `firmware_family: moonlit` (`metro_settings.c:131`, D-001),
   - `sync_marker_supported: 1` (`metro_settings.c:132`),
   - **sin** `theme_format_supported` (D-009): moonlit no tiene formato
     de tema instalable; Studio deshabilita esa función limpiamente.
2. **Marcador `/.aura/sync-pending.json`** (C4–C5): se procesa y se borra
   al terminar bien; una `version` no soportada se deja intacta
   (`metro_sync.c`, `metro_sync_marker.c`, tests host).
3. **Árbol dormido `/.firmware-moonlit/`** (contrato v10, D-326;
   `METRO_FW_OWN_DORMANT` en `metro_firmware_families.h:40`). El cambio
   de familia por renombre (M-090) se conserva y se generaliza a las
   **tres familias** del contrato v14 §A bis (D-047): Ajustes › General
   › "cambiar sistema" muestra **una fila por hermana** (Aura
   `/.firmware-aura`, Metro `/.firmware-metro`; tabla en
   `metro_firmware_families.c`), inerte con "no instalado" si falta el
   dormido; al confirmar: `/.rockbox` → `/.firmware-moonlit`,
   `/.firmware-<hermana>` → `/.rockbox`, respaldo `/rockbox.ipod`,
   marcador con `music: true` solo si la biblioteca cambió (M-091),
   reinicio en seco (`metro_settings.c`, `metro_firmware_switch_to()`).
   En Studio corresponde a `FirmwareFamily.dormantTreeName =
   ".firmware-moonlit"`.
4. **Caché privada `/.rockbox/aura/moonlitcache/art/`**
   (`metro_settings.c`, D-023; desde D-055 solo `art/`, los `.mth` viven
   en §A.10; desde D-059 `art/` en sí solo contiene la bandera interna
   `.gc-pending` — el `.pfraw`/`.none` privado que vivía aquí se retiró
   junto con la caché maestra compartida de §A.11). Árbol interno de
   moonlit, ajeno al contrato: Studio no lo lee ni lo escribe (C23) y la
   limpieza de convivencia entre familias lo borra completo, igual que
   `metrocache/`, `photocache/` y `cfcache/`. Nunca `metrocache/`.
5. **`install_manifest.cfg`** se ignora (C28). `version.txt` dentro de
   `rockbox.zip` solo se escribe con `--release-tag` (C22, M-056).
6. **Build reproducible**: nada que viaje en `rockbox.zip` usa
   `__DATE__`/`__TIME__`, marcas de tiempo ni aleatoriedad (C28; los
   plugins SDL Quake/Duke3D dejaron de embeber la hora de build en
   D-048).
7. **Release en GitHub `Ricolinos/moonlit-aura`**, primer tag
   **`v0.1.0`** (D-046: los tags heredados de Metro no se publican), con
   exactamente los assets de la tabla §A del contrato v14 que aplican a
   una familia sin sistema de temas: `rockbox.ipod`, `rockbox.zip`,
   `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`,
   `MODIFICATIONS.md`, `THIRD-PARTY-NOTICES.txt`. Sin `AuraPalette.swift`,
   `theme-format-v1.json` ni `aura-theme-default.zip` (D-009).
   Productor: `firmware/tools/package_dist.sh`.
8. **Centinela de árbol instalado**: `.rockbox/fonts/moonlit-body-18.fnt`
   (ruta relativa a la raíz del volumen, **sin barra inicial**, que es
   como la consume Studio)
   — archivo que solo moonlit escribe (PA-7; el nombre quedó fijado
   aquí y M2 lo materializó: `design-system/generate.py --fonts` genera
   `firmware/assets/fonts/moonlit-body-18.fnt`, que
   `firmware/tools/package_dist.sh` copia a `.rockbox/fonts/` en cada
   build — ya no es aspiracional, es un archivo real del árbol
   instalado). En Studio: `FirmwareFamily.installedTreeSentinel`.
9. **Base tagcache compartida `/.aura/tagcache/`** (v15, D-054, C29):
   `database_*.tcd` + `db_stamp.txt`, una sola para las tres familias
   (`apps/tagcache.c` byte-idéntico; ruta fijada en
   `global_settings.tagcache_db_path` antes de `tagcache_init()`).
   moonlit migra por `rename()` la base de su árbol al primer arranque,
   sella tras cada (re)construcción exitosa — sync de Studio o rebuild de
   bootstrap — comparando siempre contra `/.aura/library-stamp`, y al
   cambiar de familia deja el marcador `music: true` solo si ese sello
   difiere. Studio **no** lee ni escribe el directorio; lo **borra**
   completo (junto con el sello) cuando fuerza una reconstrucción.
10. **Miniaturas compartidas `/.aura/thumbs/{albums,artists,photos}/`**
   (v15, D-055, C30): `.mth` de 80×80 `fb_data` crudo, formato idéntico
   en Metro y moonlit. **v18 (D-072): moonlit ya no escribe el subdirectorio
   `photos`** — ver la nota de la versión 6 arriba; `albums` y `artists`
   siguen igual. Clave de álbum `a-<crc32 de la ruta de la pista
   representativa>.<tag_mtime>` — estable a través de rebuilds; fotos y
   artistas conservan `<archivo>.<mtime>`. **v18 (D-063):** ese
   `<tag_mtime>` es ahora `max(mtime de la pista representativa, mtime
   de la `cover.jpg` hermana si existe)`. Studio ignora el directorio
   salvo para borrarlo al forzar rebuild. Desde D-059 esta miniatura se
   DERIVA de la caché maestra de §A.11 (reducción entera 130→80) en vez
   de decodificar el JPEG por separado — mismo formato en disco, distinto
   origen del píxel. Huérfanos: moonlit los limpia tras un sync con
   música (bandera `moonlitcache/art/.gc-pending`, barrido por el
   constructor en segundo plano de §A.11, ya no bajo "preparando
   biblioteca").
11. **Caché maestra compartida `/.aura/art/{albums,artists,photos}/`**
   (v16, D-059, C31): un JPEG se decodifica UNA vez, sin importar cuál
   de las tres familias lo hizo primero. `<clave>.art` — cabecera LE de
   16 B (`magic 'MAST'`, `width`, `height`, `flags=0`, `reserved=0`) +
   RGB565 LE fila-contigua, cuadrado, SIN esquinas ni tema horneados
   (recorte fill-and-center-crop puro de la fuente): álbumes/artistas
   130×130, fotos 80×80. `<clave>.none` (0 B) es el marcador negativo
   compartido — reemplaza el `.none` privado de `moonlitcache/art/`
   (D-056). Misma clave de álbum que §A.10 (`a-<crc32 ruta pista
   representativa>.<tag_mtime>`); artistas/fotos análogas con prefijo
   `r-`/`p-` sobre la ruta del archivo de imagen. Escritura atómica
   (`<ruta>.tmp` + `rename()`): una familia hermana nunca lee una
   maestra a medio escribir. Cada familia deriva su propio tamaño de
   trabajo al cargar (moonlit: 130→120 para Marea, 130→80 para la
   rejilla de §A.10) — nunca al revés. Studio ignora el directorio
   salvo para borrarlo al forzar un rebuild (junto con §A.9/§A.10).

   **v18 (D-063), versión de formato y purga.** `/.aura/art/format.txt`
   contiene un entero decimal (v18: `2`). Al arrancar, cada familia lo
   lee; si falta o el valor es menor que el suyo, borra las cachés
   DERIVADAS —`/.aura/art/{albums,artists,photos}`, las miniaturas
   compartidas de §A.10 y la caché privada `moonlitcache/`— y escribe su
   versión. moonlit borra por sufijo conocido (`.art`, `.none`, `.mth`,
   `.pfraw`), nunca el directorio entero: son directorios compartidos y
   un barrido ciego podría llevarse algo de otra familia. Studio nunca
   lo toca ni lo borra por separado (igual que el resto de `/.aura/art`).
   Cierra lo que las claves no pueden: un tile mal derivado por una
   versión anterior del código sobrevive para siempre porque su clave no
   cambió.

   **v18 (D-063), clave de álbum.** El `<mtime>` de `a-<crc32>.<mtime>`
   es `max(mtime de la pista representativa, mtime de la `cover.jpg`
   hermana si existe)`: una carátula reescrita sin tocar la pista
   invalida la maestra (hipótesis (a) de D-055/D-056).

12. **Ajustes compartidos `/.aura/settings.cfg`** (v19, D-079, D.6 del
   contrato canónico): texto plano, una clave por línea (`clave:
   valor`), cabecera obligatoria `# aura-shared-settings v1`, escritura
   atómica (`.tmp` + `rename()`). moonlit lo consume con un módulo puro
   propio (`moonlit_shared_settings.c`/`.h`, host-testable, mismo criterio
   que `metro_sync_marker.c`) para no acoplar el parseo/serializado al
   resto del firmware. Trece claves conocidas: `rev`, `updated_by`,
   `screen_lock_enabled`, `screen_lock_pin`, `screen_lock_require`,
   `brightness`, `backlight_timeout`, `idle_poweroff`, `keyclick`,
   `volume_limit`, `replaygain`, `language`, `appearance`. Una clave
   desconocida se preserva textual al reescribir (`unknown_lines`); un
   valor fuera de rango se ignora clave por clave, nunca aborta el
   archivo entero (mismo criterio de tolerancia que el resto de `/.aura`).

   **Cuándo se aplica.** `metro_settings_apply_pending_shared()` corre
   en el mismo punto donde ya se aplicaba la hora (arranque y vuelta de
   USB, `metro_disk_handoff()`) y una segunda vez justo antes de
   `metro_screen_lock_init()` — el candado tiene que ver un `rev` nuevo
   ANTES de decidir si bloquea, o un `screen_lock_enabled: 0` escrito
   por Studio con el aparato apagado quedaría inalcanzable la próxima
   vez que encienda (ruta de emergencia por USB, D.6 del contrato
   canónico). `shared_rev_applied` vive en `aura.cfg` de moonlit, igual
   que las demás familias.

   **Cuándo se escribe.** `metro_settings_write_shared()` reescribe el
   archivo completo con `rev+1`, `updated_by: moonlit` y las 13 claves
   conocidas al valor VIGENTE — nunca el valor que traía el archivo
   viejo — cada vez que el usuario cambia una de ellas desde Ajustes o
   el candado, y al Restablecer ajustes. Studio nunca escribe ni borra
   este archivo (regla D.6): es propiedad exclusiva de los firmwares,
   igual que `/.aura/art`.

Todo lo demás (C1–C31 de `docs/COMPAT_STUDIO.md`) se hereda de Metro-Aura
sin cambios de formato.

## §B — Frontera GPL: bootloader y `mks5lboot`

Ningún repo hermano da número de versión propio a estos dos binarios
(plan §A.4). moonlit.aura los versiona explícitamente:

```
Versión de frontera GPL: BOOT-1

bootloader-ipod6g.ipod
  fuente:    firmware/rockbox/bootloader/ipod-s5l87xx.c @ moonlit-fork-base (2f1bd28a)
  base:      Rockbox 0726ec93517a61f602679ab052b083217ec9c96d (M-001)
  cambios:   MODIFICATIONS.md ("bootloader/ipod-s5l87xx.c", arranque silencioso, D-064 de AF)
  identidad: RBVERSION "<hash>-<fecha>" del build de release + SHA-256 en checksums.txt

mks5lboot
  fuente:    firmware/rockbox/utils/mks5lboot/ @ moonlit-fork-base (2f1bd28a)
  IM3_VERSION 1.0 (mks5lboot.h:41 — formato DFU, no versión de herramienta)
  cambios:   MODIFICATIONS.md ("utils/mks5lboot/Makefile", backend libusb opcional en macOS)
  identidad: SHA-256 en checksums.txt
```

`BOOT-1` sube a `BOOT-2` **solo** si cambia cualquiera de los dos
fuentes. El SHA-256 cambia con cada recompilación (RBVERSION embebido),
por eso no sirve como versión de fuente (PA-4). Los SHA-256 reales de
cada release se anotan aquí en M12 de `docs/plan/05-plan-correctivo.md`
(= H7 del plan 03; ningún binario de este par cambió en M1…M11,
`BOOT-1` sigue vigente):

```
checksums.txt del release v0.1.0:
  a9145ba8f3f030589593b40a64977b0be064b15272846d78417573169755e1df  bootloader-ipod6g.ipod
  035e14db2be7ce093a434a2dac8fff419c64a8be84644122710e2a1850fc3a91  mks5lboot
  246d7035ae4f5d208417a21b522f6786da9f181017718e892c690b5ac196cdae  rockbox.ipod
  (publicado en https://github.com/Ricolinos/moonlit-aura/releases/tag/v0.1.0)

checksums.txt del release v0.1.1 (BOOT-1 sigue vigente: fuentes de bootloader y
mks5lboot sin cambios; el SHA del bootloader cambia solo por el RBVERSION embebido):
  3ba5650be7bb4a7038e20843d14af119de3c4544d96623e9d0b60cc93b823e13  bootloader-ipod6g.ipod
  035e14db2be7ce093a434a2dac8fff419c64a8be84644122710e2a1850fc3a91  mks5lboot
  78161219a816b1d11bd6bef556c4cda2cfd582e294b88cde099f63dd925bfd30  rockbox.ipod
  (publicado en https://github.com/Ricolinos/moonlit-aura/releases/tag/v0.1.1)

checksums.txt del release v0.1.2 (BOOT-1 sigue vigente):
  e79e487746657bc862e189415cb375c4b8e19b768d46638e4c61ba518fbcaa7a  bootloader-ipod6g.ipod
  035e14db2be7ce093a434a2dac8fff419c64a8be84644122710e2a1850fc3a91  mks5lboot
  ca9d8bcd3b834b3faf35e7892ea4de934d52173fa219628569ccca2185c83607  rockbox.ipod
  (publicado en https://github.com/Ricolinos/moonlit-aura/releases/tag/v0.1.2)
```

Los cuatro binarios (`mks5lboot`, `bootloader-ipod6g.ipod`,
`rockbox.ipod`, `rockbox.zip`) son derivados de Rockbox, GPL v2 (§B del
contrato v14). `rockbox.zip` contiene además Libre Baskerville y
Montserrat (SIL OFL 1.1) y Material Symbols rasterizados (Apache 2.0);
sus avisos van en `THIRD-PARTY-NOTICES.txt` (D-002).

## §C — Lo que moonlit.aura requiere de Studio (no ejecutado en este plan)

- Reconocer `firmware_family: moonlit` como familia **instalable y
  actualizable**: `FirmwareFamily` con `configValue "moonlit"`,
  `displayName "moonlit.aura"`, `releaseRepository "Ricolinos/moonlit-aura"`,
  `bundleSubdirectory "moonlit"`, `installedTreeSentinel
  ".rockbox/fonts/moonlit-body-18.fnt"` (sin barra inicial),
  `dormantTreeName ".firmware-moonlit"`. Detalle línea a línea en
  `docs/plans/PROMPT-aura-studio.md`.
- Pin de versión en `FIRMWARE_VERSION` con el prefijo **`moonlit.`**
  (`moonlit.tag=v0.1.0` + cuatro hashes `moonlit.*` de `rockbox.ipod`,
  `rockbox.zip`, `bootloader-ipod6g.ipod`, `mks5lboot`), mismo formato
  que `metro.*`; el bundle vive en `bundleSubdirectory "moonlit"`. Lo
  actualiza Aura-Studio, nunca este repo.
- Pantalla Extras › Licencias (§B del contrato v14): URL del repo, tag
  de `FIRMWARE_VERSION`, enlaces a `MODIFICATIONS.md` y
  `THIRD-PARTY-NOTICES.txt` del release de moonlit.
- **Hasta entonces** Studio degrada de forma segura
  (`SyncMarker.swift:68-99`; `02-investigacion.md` §4) pero **no ofrece
  actualizaciones** de moonlit. Riesgo M-004 (D-021): un Studio anterior
  a ST-045 compara hashes de `rockbox.ipod` y puede ofrecer "actualizar"
  a Aura; uno con ST-045 pero sin `moonlit` puede ofrecer volver a Aura o
  a Metro. Documentado para el usuario en `docs/GUIA_FLASHEO.md` y
  `README.md`; sin mitigación posible desde este repo.
