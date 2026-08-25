# Contrato entre `moonlit-aura` y Aura Studio

**Versión 1 — 2026-08-25 (hito H1, D-001/D-002/D-017/D-021/D-023).**

Este archivo **no copia** los contratos canónicos de `Aura-Firmware`; los
**referencia** y declara cómo moonlit.aura los cumple. moonlit.aura
consume esos contratos sin modificarlos (regla heredada de Metro-Aura,
`CLAUDE.md` Metro:66-71): el contrato de datos es inmutable desde este
lado.

Contratos referenciados (fuente canónica, se leen desde el repo hermano):

| Contrato | Versión referenciada | Ruta |
|---|---|---|
| Firmware ↔ Studio | **v13** (2026-08-23) | `Aura-Firmware/CONTRATO-firmware-studio.md` |
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
   `metro_settings.c:246`). El cambio de familia por renombre (M-090)
   se conserva: `/.rockbox` → `/.firmware-moonlit`, `/.firmware-aura` →
   `/.rockbox`, respaldo `/rockbox.ipod`, marcador con `music: true`,
   reinicio en seco. En Studio corresponde a
   `FirmwareFamily.dormantTreeName = ".firmware-moonlit"`.
4. **Caché privada `/.rockbox/aura/moonlitcache/<fuente>/`**
   (`metro_settings.c:216`, D-023). Árbol interno de moonlit, ajeno al
   contrato: Studio no lo lee ni lo escribe (C23) y la limpieza de
   convivencia entre familias lo borra completo, igual que
   `metrocache/`, `photocache/` y `cfcache/`. Nunca `metrocache/`.
5. **`install_manifest.cfg`** se ignora (C28). `version.txt` dentro de
   `rockbox.zip` solo se escribe con `--release-tag` (C22, M-056).
6. **Build reproducible**: nada que viaje en `rockbox.zip` usa
   `__DATE__`/`__TIME__`, marcas de tiempo ni aleatoriedad (C28).
7. **Release en GitHub `ricolinos/moonlit-aura`** con exactamente los
   assets de la tabla §A del contrato v13 que aplican a una familia sin
   sistema de temas: `rockbox.ipod`, `rockbox.zip`,
   `bootloader-ipod6g.ipod`, `mks5lboot`, `checksums.txt`,
   `MODIFICATIONS.md`, `THIRD-PARTY-NOTICES.txt`. Sin `AuraPalette.swift`,
   `theme-format-v1.json` ni `aura-theme-default.zip` (D-009).
   Productor: `firmware/tools/package_dist.sh`.
8. **Centinela de árbol instalado**: `/.rockbox/fonts/moonlit-body-18.fnt`
   — archivo que solo moonlit escribe (PA-7; el nombre queda fijado
   aquí y H2 lo materializa). En Studio:
   `FirmwareFamily.installedTreeSentinel`.

Todo lo demás (C1–C28 de `docs/COMPAT_STUDIO.md`) se hereda de Metro-Aura
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
cada release se anotan aquí en H7 (D-026).

Los cuatro binarios (`mks5lboot`, `bootloader-ipod6g.ipod`,
`rockbox.ipod`, `rockbox.zip`) son derivados de Rockbox, GPL v2 (§B del
contrato v13). `rockbox.zip` contiene además Libre Baskerville y
Montserrat (SIL OFL 1.1) y Material Symbols rasterizados (Apache 2.0);
sus avisos van en `THIRD-PARTY-NOTICES.txt` (D-002).

## §C — Lo que moonlit.aura requiere de Studio (no ejecutado en este plan)

- Reconocer `firmware_family: moonlit` como familia **instalable y
  actualizable**: `FirmwareFamily` con `configValue "moonlit"`,
  `displayName "moonlit.aura"`, `releaseRepository "ricolinos/moonlit-aura"`,
  `bundleSubdirectory "moonlit"`, `installedTreeSentinel
  "/.rockbox/fonts/moonlit-body-18.fnt"`, `dormantTreeName
  ".firmware-moonlit"`. Detalle línea a línea en
  `docs/plans/PROMPT-aura-studio.md`.
- Pantalla Extras › Licencias (§B del contrato v13): URL del repo, tag
  de `FIRMWARE_VERSION`, enlaces a `MODIFICATIONS.md` y
  `THIRD-PARTY-NOTICES.txt` del release de moonlit.
- **Hasta entonces** Studio degrada de forma segura
  (`SyncMarker.swift:68-99`; `02-investigacion.md` §4) pero **no ofrece
  actualizaciones** de moonlit. Riesgo M-004 (D-021): un Studio anterior
  a ST-045 compara hashes de `rockbox.ipod` y puede ofrecer "actualizar"
  a Aura; uno con ST-045 pero sin `moonlit` puede ofrecer volver a Aura o
  a Metro. Documentado para el usuario en `docs/GUIA_FLASHEO.md` y
  `README.md`; sin mitigación posible desde este repo.
