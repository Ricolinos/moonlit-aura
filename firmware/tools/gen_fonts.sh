#!/usr/bin/env bash
# Convierte las fuentes Selawik vendoreadas a .fnt de Rockbox (formato
# RB12) con tools/convttf, un rol/tamaño a la vez -- ver DECISIONS.md
# M-010.
#
# Requiere tools/convttf compilado (requiere freetype2 vía pkg-config,
# `brew install freetype pkg-config`); se compila aquí mismo si falta.
#
# Uso:
#   firmware/tools/gen_fonts.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/firmware/rockbox"
FONTS_SRC="$ROOT_DIR/firmware/assets/fonts-src"
FONTS_OUT="$ROOT_DIR/firmware/assets/fonts"
CONVTTF="$SRC_DIR/tools/convttf"

if [[ ! -x "$CONVTTF" ]]; then
  echo "==> Compilando tools/convttf"
  ( cd "$SRC_DIR/tools" && \
    cc -lm -std=c99 -O2 -Wall -g convttf.c -o convttf \
      $(pkg-config --cflags --libs freetype2) )
fi

mkdir -p "$FONTS_OUT"

# rol:archivo-fuente:tamaño-px -- ver DECISIONS.md M-010
ROLES=(
  "display:Selawik-Light.ttf:48"
  "title:Selawik-Light.ttf:28"
  "list:Selawik-Regular.ttf:20"
  "listsel:Selawik-Semibold.ttf:20"
  "caption:Selawik-Regular.ttf:14"
)

# Rango de caracteres: latin basico + Latin-1 Supplement + Latin
# Extended-A (0x20-0x17F) -- suficiente para espanol con acentos/enie,
# sin cargar el conjunto completo de simbolos de Selawik.
#
# Sin "-x" (trim horizontal): convttf recorta hasta 2px por lado de
# TODO glifo "casi vacio", incluido el espacio (0x20), que a 20px pasa
# de ~5px a ~1px de ancho -- el texto se ve sin espacios. Aura-Firmware
# tampoco lo usa (design-system/generate.py: solo "-p <size>"). Ver
# DECISIONS.md M-028.
START=0x20
LIMIT=0x17F
DEFAULT=0x3F # '?'

for entry in "${ROLES[@]}"; do
  IFS=':' read -r role srcfont size <<< "$entry"
  in="$FONTS_SRC/$srcfont"
  out="$FONTS_OUT/metro-$role-$size.fnt"
  echo "==> $role: $srcfont @ ${size}px -> $(basename "$out")"
  "$CONVTTF" -p "$size" -s "$START" -l "$LIMIT" -D "$DEFAULT" \
    -o "$out" "$in"
done

echo "==> Listo: $FONTS_OUT"
ls -la "$FONTS_OUT"
