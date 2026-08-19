#!/usr/bin/env bash
# Compila el toolchain ARM (arm-elf-eabi) para el target ipod6g, una
# sola vez. Basado en el mismo procedimiento que Aura-Firmware (D-032):
# el propio script de Rockbox `rockboxdev.sh`, sin parches propios.
#
# Uso:
#   firmware/tools/build_toolchain.sh
#
# Tarda unos minutos en un Mac Apple Silicon reciente.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/firmware/rockbox"
TC_DIR="$ROOT_DIR/firmware/toolchain"

mkdir -p "$TC_DIR"
export RBDEV_PREFIX="$TC_DIR"
export RBDEV_TARGET="a"
export RBDEV_DOWNLOAD="$TC_DIR/dl"
export RBDEV_BUILD="$TC_DIR/build-tmp"

echo "==> Compilando toolchain arm-elf-eabi en $TC_DIR"
bash "$SRC_DIR/tools/rockboxdev.sh"

echo "==> Listo: $TC_DIR/bin"
