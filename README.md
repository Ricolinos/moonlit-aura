# moonlit.aura

Firmware para iPod Classic 6G (S5L8702, pantalla 320×240, 64 MB de RAM,
ARM sin GPU ni FPU, clickwheel), fork de
[Rockbox](https://www.rockbox.org/) a través de
[Metro-Aura](https://github.com/Ricolinos/Metro-Aura).

Lenguaje visual **Waning Crescent**: calma nocturna, luz desde la
izquierda, tipografía con serifas (Libre Baskerville) para títulos y
Montserrat para texto, elevación por tono en vez de sombras difusas,
iconos Material Symbols compilados en el binario. Ninguna animación
fuera de `lcd_active()`; ningún color fuera de `design-system/tokens.json`.

## Estado

**En construcción, con el lenguaje visual Waning Crescent ya en pie.**
Este repositorio se forkeó de Metro-Aura en el tag `moonlit-fork-base`
(`2f1bd28a`). Hitos M1–M9 de `docs/plan/05-plan-correctivo.md`
ejecutados: tokens MD3 de dos esquemas (`night`/`dawn`), tipografía
Libre Baskerville + Montserrat, iconos Material Symbols compilados,
paleta y elevación tonal en hub/lista/ajustes/Ahora suena/candado/USB,
pantalla "Marea" (Cover Flow vertical) y el logotipo Waning Crescent.
Pendientes: M10 (esta sincronización de documentación), M11 (revisión
adversarial) y M12 (medición en hardware real + empaquetado). Las
decisiones cerradas viven en `DECISIONS.md` (D-001 en adelante); la
bitácora heredada de Metro se conserva de solo lectura en
`DECISIONS-METRO-ARCHIVE.md`.

## Hardware

Solo iPod Classic 6G / 6.5G / 7G (S5L8702). Mismo bootloader y
`mks5lboot` que Metro-Aura y Aura-Firmware (frontera GPL versionada en
`CONTRATO-moonlit-studio.md` §B).

## Compilar

Ver `docs/guia-desarrollo.md`. Resumen:

```bash
firmware/tools/build_toolchain.sh   # una sola vez (o RBDEV_TOOLCHAIN=<bin/> de un toolchain existente)
firmware/tools/build_sim.sh --run   # simulador SDL, día a día
firmware/tools/build_target.sh      # target real ipod6g + bootloader
```

## Instalar

Ver `docs/GUIA_FLASHEO.md` — procedimiento completo con `mks5lboot`,
requisitos y qué hacer si algo sale mal.

## Compatibilidad con Aura Studio

moonlit.aura hereda de Metro-Aura la compatibilidad con
[Aura Studio](https://github.com/Ricolinos/Aura-Studio): misma
estructura de directorios, tagcache, formatos y marcador de
sincronización (`docs/COMPAT_STUDIO.md`). Se identifica ante Studio con
`firmware_family: moonlit`, caché propia `/.rockbox/aura/moonlitcache/`
y árbol dormido `/.firmware-moonlit/` (D-001). Los cambios necesarios
en Studio para reconocer esta familia son trabajo del repo hermano
(D-017), no de este.

Lo que moonlit.aura garantiza y requiere de Studio está cerrado en
`CONTRATO-moonlit-studio.md` (v1), que referencia —no copia— los
contratos canónicos de `Aura-Firmware` (v13, v2, v1.3).

**Advertencia**: una versión de Aura Studio que no conozca la familia
`moonlit` puede ofrecer "actualizar" el iPod de vuelta a Aura o Metro
— ver `DECISIONS.md` D-021 y `docs/GUIA_FLASHEO.md`.

## Licencia

GPL v2 (heredada de Rockbox) — ver `LICENSE`. Avisos de modificación en
`MODIFICATIONS.md`. Fuentes e iconos vendoreados llevan su licencia en
`design-system/vendor/<asset>/`. Este proyecto no está afiliado a Apple.
