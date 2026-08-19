# Guía de desarrollo — Metro-Aura

Cómo compilar y trabajar en el proyecto. Todos los comandos asumen que
estás parado en la raíz de este repo en un Mac con Apple Silicon.

## Requisitos (una sola vez)

```bash
brew install sdl2 gcc
```

`tools/configure` de Rockbox detecta el gcc real de Homebrew más nuevo
disponible (no el `clang` que Xcode expone como `gcc`) — fix heredado
de Aura-Firmware, ver `DECISIONS.md` M-021.

## Firmware — simulador SDL (día a día)

El simulador es el banco de pruebas principal — no hace falta el iPod
físico para desarrollar ni probar.

```bash
firmware/tools/build_sim.sh              # configura (primera vez) y compila
firmware/tools/build_sim.sh --run        # compila y lo abre
firmware/tools/gen_test_media.sh         # genera música de prueba en el disco simulado
```

Capturas headless (sin permisos de Accesibilidad de macOS):

```bash
firmware/tools/sim_shot.sh salida.png [ticks] ["SELECT,SCROLL_FWD,..."]
```

Tests de la lógica pura (una vez existan módulos en `apps/metro/test/`):

```bash
make -C firmware/rockbox/apps/metro/test test
```

## Firmware — target real `ipod6g`

Requiere el toolchain ARM (una sola vez, ~unos minutos):

```bash
firmware/tools/build_toolchain.sh
```

Con el toolchain listo, compilar firmware y bootloader:

```bash
firmware/tools/build_target.sh              # firmware + bootloader
firmware/tools/build_target.sh --firmware    # solo rockbox.ipod
firmware/tools/build_target.sh --bootloader  # solo bootloader-ipod6g.ipod
```

## Fuente de un fork de Aura-Firmware, no de un checkout hermano

Ningún script de este repo asume la existencia de un checkout de
`Aura-Firmware` ni de `Aura-Studio` al lado — la carpeta padre `Aura/`
los trata como repositorios independientes (ver `Aura/CLAUDE.md`). El
único acoplamiento permitido es de **contrato de datos en disco**
(`docs/COMPAT_STUDIO.md`), nunca de ruta de archivo compartida.
