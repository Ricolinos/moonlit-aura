#!/usr/bin/env bash
# Empaqueta los artefactos distribuibles del firmware en firmware/dist/.
# No se versionan (ver firmware/dist/README.md) -- este script los
# reproduce a partir del código fuente, para publicarlos como GitHub
# Release.
#
# Uso:
#   firmware/tools/package_dist.sh
#
# Requiere: el toolchain ARM instalado en firmware/toolchain/ (ver
# firmware/tools/build_toolchain.sh) -- o RBDEV_TOOLCHAIN apuntando a
# uno externo, igual que build_target.sh (DECISIONS.md M-002).
#
# A diferencia de Aura-Firmware's package_dist.sh (que sirve de
# referencia para este script -- ver DECISIONS.md), Metro-Aura no
# tiene sistema de temas/iconos instalables ni pipeline design-system/:
# M-018 fija toda la iconografía compilada en el binario, nunca leída
# de disco. Nada de AuraPalette.swift/theme-format-v1.json/
# aura-theme-default.zip aquí -- no hay a qué apuntarían.
#
# Produce:
#   firmware/dist/rockbox.ipod             -- binario del firmware
#   firmware/dist/rockbox.zip              -- árbol .rockbox/ completo
#                                              ("make zip": códecs,
#                                              plugins/rocks,
#                                              viewers.config, codepages,
#                                              langs -- con las fuentes
#                                              de Metro encima)
#   firmware/dist/bootloader-ipod6g.ipod   -- bootloader dual-boot (si
#                                              ya está compilado --
#                                              build_target.sh lo hace
#                                              por defecto, a diferencia
#                                              de Aura-Firmware esto no
#                                              necesita un paso manual)
#   firmware/dist/mks5lboot                -- herramienta de flasheo DFU
#   firmware/dist/MODIFICATIONS.md         -- listado GPL §2a, para el Release
#   firmware/dist/THIRD-PARTY-NOTICES.txt  -- licencia de Selawik (SIL OFL 1.1)
#   firmware/dist/checksums.txt            -- SHA-256 de todo lo de arriba

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SRC_DIR="$ROOT_DIR/firmware/rockbox"
BUILD_DIR="$ROOT_DIR/firmware/build-ipod6g"
BOOT_BUILD_DIR="$ROOT_DIR/firmware/build-ipod6g-boot"
DIST_DIR="$ROOT_DIR/firmware/dist"
TC_BIN="${RBDEV_TOOLCHAIN:-$ROOT_DIR/firmware/toolchain/bin}"

if [[ ! -d "$TC_BIN" ]]; then
  echo "ERROR: no se encontró el toolchain en $TC_BIN" >&2
  echo "       corre firmware/tools/build_toolchain.sh primero," >&2
  echo "       o exporta RBDEV_TOOLCHAIN=<ruta a un toolchain existente>." >&2
  exit 1
fi

echo "==> Compilando firmware + bootloader (build_target.sh)"
"$ROOT_DIR/firmware/tools/build_target.sh"

echo "==> Empaquetando el árbol .rockbox/ real (make zip: códecs, rocks, viewers.config, codepages, langs)"
(cd "$BUILD_DIR" && PATH="$TC_BIN:$PATH" make zip)

echo "==> Compilando mks5lboot"
MKS5LBOOT_DIR="$SRC_DIR/utils/mks5lboot"
(cd "$MKS5LBOOT_DIR" && make)

mkdir -p "$DIST_DIR"

echo "==> Copiando rockbox.ipod"
cp "$BUILD_DIR/rockbox.ipod" "$DIST_DIR/rockbox.ipod"

echo "==> Copiando mks5lboot"
cp "$MKS5LBOOT_DIR/mks5lboot" "$DIST_DIR/mks5lboot"

if [[ -f "$BOOT_BUILD_DIR/bootloader-ipod6g.ipod" ]]; then
  echo "==> Copiando bootloader-ipod6g.ipod"
  cp "$BOOT_BUILD_DIR/bootloader-ipod6g.ipod" "$DIST_DIR/bootloader-ipod6g.ipod"
else
  echo "ADVERTENCIA: $BOOT_BUILD_DIR/bootloader-ipod6g.ipod no existe -- build_target.sh debería haberlo generado. checksums.txt saldrá sin él." >&2
fi

echo "==> Armando rockbox.zip (.rockbox/ completo)"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/.rockbox"
# Base real -- todo lo que "make zip" acaba de producir (codecs/,
# rocks/, viewers.config, codepages/, langs/, themes/, wps/, eqs/,
# recpresets/, debug/, backdrops/, rockbox-info.txt, rockbox.ipod) --
# y ENCIMA las fuentes de Metro (metro_fonts.c las carga por nombre
# fijo, así que tienen que existir en el disco exactamente con esos
# nombres, igual que hace build_sim.sh con el simdisk).
unzip -q "$BUILD_DIR/rockbox.zip" -d "$STAGE"
mkdir -p "$STAGE/.rockbox/fonts"
cp "$ROOT_DIR"/firmware/assets/fonts/*.fnt "$STAGE/.rockbox/fonts/"

# Centinelas -- si "make zip" alguna vez deja de producir alguno de
# estos (cambio de layout de Rockbox upstream, target mal configurado,
# etc.), mejor abortar aquí con un mensaje claro que publicar un
# rockbox.zip mudo (sin video, sin audio en una instalación desde
# cero) -- misma lección que Aura-Firmware documentó de la manera
# difícil (D-297 en ese repositorio) y que este script hereda a
# propósito en vez de repetir el mismo descuido.
SENTINELS=(
  ".rockbox/rocks/viewers/mpegplayer.rock"
  ".rockbox/codecs/mpa.codec"
  ".rockbox/codecs/flac.codec"
  ".rockbox/codecs/aac.codec"
  ".rockbox/codecs/alac.codec"
  ".rockbox/viewers.config"
  ".rockbox/fonts/metro-display-48.fnt"
  ".rockbox/fonts/metro-list-20.fnt"
  ".rockbox/rockbox.ipod"
)
for sentinel in "${SENTINELS[@]}"; do
  if [[ ! -e "$STAGE/$sentinel" ]]; then
    echo "ERROR: falta $sentinel en el árbol armado -- rockbox.zip saldría incompleto. Revisa 'make zip' en $BUILD_DIR." >&2
    exit 1
  fi
done
echo "==> Centinelas verificados: $(find "$STAGE/.rockbox" -type f | wc -l | tr -d ' ') archivos en el árbol"

(cd "$STAGE" && zip -qr "$DIST_DIR/rockbox.zip" .rockbox)

echo "==> Copiando MODIFICATIONS.md (asset del Release, cumplimiento GPL §2a)"
cp "$ROOT_DIR/MODIFICATIONS.md" "$DIST_DIR/MODIFICATIONS.md"

echo "==> Generando THIRD-PARTY-NOTICES.txt (licencia de Selawik, asset del Release)"
{
  echo "Metro-Aura -- avisos de terceros"
  echo "================================="
  echo
  echo "El wordmark \"metro\" y la tipografía de la interfaz usan Selawik"
  echo "(Microsoft, SIL Open Font License 1.1) -- ver DECISIONS.md M-020:"
  echo "fuente de sistema publicada libremente, no un logotipo ni una"
  echo "marca de Zune/Windows Phone. El texto completo de la licencia"
  echo "sigue abajo."
  echo
  echo "-- Selawik (tipografía) ---------------------------------------------"
  echo
  cat "$ROOT_DIR/firmware/assets/fonts-src/LICENSE.txt"
} > "$DIST_DIR/THIRD-PARTY-NOTICES.txt"

echo "==> Generando checksums.txt"
(
  cd "$DIST_DIR"
  files=(rockbox.zip rockbox.ipod mks5lboot)
  [[ -f bootloader-ipod6g.ipod ]] && files+=(bootloader-ipod6g.ipod)
  shasum -a 256 "${files[@]}" > checksums.txt
)

echo "==> Listo: $DIST_DIR"
ls -la "$DIST_DIR"
