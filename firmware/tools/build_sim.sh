#!/usr/bin/env bash
# Compila el simulador SDL de Metro-Aura (target ipod6g) en macOS.
#
# Requisitos (Homebrew): sdl2, un gcc real (no el clang de Xcode) —
# `brew install sdl2 gcc`. `tools/configure` detecta automáticamente
# el gcc de Homebrew más nuevo disponible (16, 15, 14 o 13); ver
# DECISIONS.md M-021/M-022 y docs/DESVIACIONES.md F0-1 sobre este fix,
# heredado de Aura-Firmware (D-007 en su bitácora).
#
# Uso:
#   firmware/tools/build_sim.sh              # configura (primera vez) y compila
#   firmware/tools/build_sim.sh --reconfigure   # fuerza reconfigurar desde cero
#   firmware/tools/build_sim.sh --run           # compila y lo abre
#
# RBDEV_TOOLCHAIN=<ruta a bin/> antepone un toolchain externo al PATH
# (atajo de desarrollo, nunca usado por defecto — ver DECISIONS.md M-002).
# El simulador no lo necesita (usa el gcc del host), pero se respeta
# por consistencia con build_target.sh.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/firmware/rockbox"
BUILD_DIR="$ROOT_DIR/firmware/build-sim"

if [[ -n "${RBDEV_TOOLCHAIN:-}" ]]; then
  export PATH="$RBDEV_TOOLCHAIN:$PATH"
fi

if [[ "${1:-}" == "--reconfigure" ]]; then
  rm -rf "$BUILD_DIR"
  shift
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [[ ! -f Makefile ]]; then
  echo "==> Configurando simulador (target ipod6g)"
  "$SRC_DIR/tools/configure" --target=ipod6g --type=s
fi

echo "==> Compilando (make -j)"
make -j"$(sysctl -n hw.ncpu)"

echo "==> Instalando assets de Rockbox en el disco simulado"
make install

if [[ -d "$ROOT_DIR/firmware/assets/fonts" ]] && ls "$ROOT_DIR/firmware/assets/fonts"/*.fnt >/dev/null 2>&1; then
  echo "==> Instalando fuentes Metro en el disco simulado"
  mkdir -p simdisk/.rockbox/fonts
  cp "$ROOT_DIR"/firmware/assets/fonts/*.fnt simdisk/.rockbox/fonts/
fi

echo "==> Listo: $BUILD_DIR/rockboxui"

if [[ "${1:-}" == "--run" ]]; then
  exec ./rockboxui
fi
