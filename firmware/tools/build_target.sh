#!/usr/bin/env bash
# Compila el firmware y el bootloader para el target real ipod6g.
# Requiere el toolchain ARM ya compilado (firmware/tools/build_toolchain.sh).
#
# Uso:
#   firmware/tools/build_target.sh              # firmware + bootloader
#   firmware/tools/build_target.sh --firmware    # solo firmware
#   firmware/tools/build_target.sh --bootloader  # solo bootloader
#
# RBDEV_TOOLCHAIN=<ruta a bin/> usa un toolchain externo en vez del de
# este repo (atajo de desarrollo, nunca por defecto — DECISIONS.md M-002).
#
# VERSION=<string> fuerza la cadena de versión (rockbox-info.txt/
# rbversion.h) en vez de que Rockbox la calcule con tools/version.sh --
# ese script asume que la ruta de código fuente ES la raíz del repo
# git (compara con GIT_WORK_TREE apuntando ahí); en Metro-Aura el repo
# real es la carpeta padre de firmware/rockbox/, así que esa
# comparación falla estructuralmente y siempre marca "M" (árbol
# modificado), incluso con todo comiteado -- DECISIONS.md M-050.
# package_dist.sh calcula esta variable correctamente antes de
# invocar este script para un release; en desarrollo normal, se deja
# sin usar (la "M" de más no afecta nada funcional).

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/firmware/rockbox"
TC_BIN="${RBDEV_TOOLCHAIN:-$ROOT_DIR/firmware/toolchain/bin}"

if [[ ! -d "$TC_BIN" ]]; then
  echo "ERROR: no se encontro el toolchain en $TC_BIN" >&2
  echo "       corre firmware/tools/build_toolchain.sh primero," >&2
  echo "       o exporta RBDEV_TOOLCHAIN=<ruta a un toolchain existente>." >&2
  exit 1
fi

WHAT="${1:---all}"

build_one() {
  local dir="$1" type="$2"
  mkdir -p "$ROOT_DIR/firmware/$dir"
  cd "$ROOT_DIR/firmware/$dir"
  if [[ ! -f Makefile ]]; then
    echo "==> Configurando $dir (type=$type)"
    PATH="$TC_BIN:$PATH" "$SRC_DIR/tools/configure" --target=ipod6g --type="$type"
  fi
  echo "==> Compilando $dir"
  PATH="$TC_BIN:$PATH" make -j"$(sysctl -n hw.ncpu)" ${VERSION:+VERSION="$VERSION"}
}

if [[ "$WHAT" == "--all" || "$WHAT" == "--firmware" ]]; then
  build_one build-ipod6g N
  echo "==> Listo: firmware/build-ipod6g/rockbox.ipod"
fi

if [[ "$WHAT" == "--all" || "$WHAT" == "--bootloader" ]]; then
  build_one build-ipod6g-boot B
  echo "==> Listo: firmware/build-ipod6g-boot/bootloader-ipod6g.ipod"
fi
