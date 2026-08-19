# COMPAT_STUDIO.md — Checklist vivo de compatibilidad con Aura Studio

Estado real de cada ítem del checklist C1–C21 de
`docs/plans/PLAN_MAESTRO.md` §4. Se actualiza al cierre de cada fase
que lo toque. Fuente del contrato: `Aura-Firmware/CONTRATO-firmware-studio.md`
y `Aura-Firmware/docs/contracts/library-layout-v1.md` (no copiados a
este repo — se leen desde el repo hermano; el contrato es inmutable
desde este lado).

Leyenda: ⬜ pendiente · 🟡 parcial/no verificado · ✅ verificado en simulador · 🔒 verificado en hardware

| # | Requisito | Estado | Fase | Nota |
|---|---|---|---|---|
| C1 | `/.rockbox/aura/` existe tras el primer arranque | ⬜ | F6 | |
| C2 | `aura.cfg` existe y se regenera entero en cada guardado | ⬜ | F6 | |
| C3 | `aura.cfg` con `sync_marker_supported: 1` y `firmware_family: metro`, sin `theme_format_supported` | ⬜ | F6 | |
| C4 | Marcador `/.aura/sync-pending.json` procesado y borrado al terminar bien | ⬜ | F6 | |
| C5 | Marcador con `version` no soportada se deja intacto | ⬜ | F6 | |
| C6 | `metro_disk_handoff()` corre al volver de USB | ⬜ | F9 | |
| C7 | Claves `rtc_sync_*` se aplican y se descartan | ⬜ | F6 | |
| C8 | Tagcache indexa los 3 layouts de `/Music/` | ✅ | F4 | `firmware/tools/gen_test_media.sh` genera pistas en los 3 layouts (`Artista/Álbum/archivo`, `Álbum/archivo`, `Artista/archivo`); los 3 aparecen correctamente agrupados en `docs/screenshots/F4-artists.png`/`F4-artist-albums.png` ("Flat Album Test"/"Flat Artist Test" son los layouts sin carpeta de artista/álbum). Por diseño el firmware no distingue layouts (`tagcache_scan_paths = "/"`, ver `docs/contracts/library-layout-v1.md` de Aura-Firmware) — Metro no necesitó código propio para esto |
| C9 | `cover.jpg` de carpeta y carátula embebida (APIC) se muestran | 🟡 | F5 | `docs/screenshots/F5-nowplaying-art.png` verifica carátula de carpeta (`find_albumart()`); `F5-nowplaying-noart.png` verifica el tile de respaldo cuando no hay arte. Carátula embebida (APIC) NO se ejercitó con ningún fixture — mismo camino de código (`metro_albumart.c: decode_embedded()`), solo falta un `.mp3` de prueba con arte embebido en `gen_test_media.sh` |
| C10 | `.lrc` no rompe nada (sin mostrarse en v1) | ✅ | F4 | `metro-test.lrc` convive en `Music/` con las pistas de prueba en todas las corridas de F4; tagcache lo ignora (extensión no reconocida como audio), Metro no lo lee en ningún lado. Backlog: mostrarlo |
| C11 | `Playlists/*.m3u8` se listan y reproducen | ✅ | F4 | `docs/screenshots/F4-playlists.png` (listado) + verificado que seleccionar "QA Favorites" arranca reproducción real (pantalla "reproduciendo" con la primera pista de la lista) |
| C12 | `/Videos/*.mpg` con nombre límite (95 bytes) se lista y abre | ⬜ | F7 | |
| C13 | `/Photos/*.jpg` ≤640px se listan (hasta 500) y abren | ⬜ | F7 | |
| C14 | Categorías de video/foto: presentes → pivots; ausentes → solo "all" | ⬜ | F7 | |
| C15 | `sync_summary.cfg`: presente → conteos; ausente → guiones | ⬜ | F8 | |
| C16 | `device.cfg`: presente → nombre; ausente → genérico | ⬜ | F8 | |
| C17 | Archivos no usados en v1 (`artists/`, `ratings.cfg`, `themes/`, `sync_manifest.json`) no rompen nada | ⬜ | F7-F8 | |
| C18 | Ninguna ruta/clave del contrato se escribe con otro nombre; nada se borra salvo el marcador al terminar | ⬜ | continuo | Revisar en cada fase que toque `metro_settings`/`metro_sync` |
| C19 | Descriptor USB sin cambios (stack Rockbox stock) | ✅ | F0 | F0 no toca `usb-s5l8702.c` ni strings de identidad USB — confirmado por el diff de F0 (ningún archivo USB en la lista de 10 portados) |
| C20 | `AuraDeviceProbe` clasifica `.aura(hasBooted:true)` con Studio real | ⬜ | F13 | Requiere hardware |
| C21 | Advertencia documentada sobre `AuraUpdateChecker` | ⬜ | F13 | `docs/ESTADO_FINAL.md` |
