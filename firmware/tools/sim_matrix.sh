#!/usr/bin/env bash
# F10: captura la matriz de revision visual (PLAN_MAESTRO.md F10: "cada
# pantalla x 2 temas x 3 acentos") -- escribe un aura.cfg preconfigurado
# antes de cada captura (metro_settings_load() lo lee en el arranque,
# mismo mecanismo que un usuario real cambiando Ajustes > Pantalla) en
# vez de navegar el menu de Ajustes por cada combinacion, mucho mas
# rapido para 18 capturas. Reutiliza sim_shot.sh para el dump en si.
#
# Pantallas: hub, una lista (musica > canciones), Now Playing -- el
# subconjunto donde tema/acento realmente se ven (fondo+texto, fila
# seleccionada, tile de caratula/progreso), no las 20+ pantallas de
# Metro completas.
#
# Uso: firmware/tools/sim_matrix.sh [dir_salida]
# Por defecto: docs/screenshots/F10-matrix

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/firmware/build-sim"
OUT_DIR="${1:-docs/screenshots/F10-matrix}"

if [[ "$OUT_DIR" != /* ]]; then
  OUT_DIR="$(pwd)/$OUT_DIR"
fi
mkdir -p "$OUT_DIR"

AURA_CFG="$BUILD_DIR/simdisk/.rockbox/aura/aura.cfg"

# theme: 0=oscuro 1=claro (metro_theme.h). accent: 0=azul 4=magenta
# (default, M-020) 3=lima -- espectro representativo (frio / vivido
# default / brillante), no los 10 acentos completos.
# macOS trae bash 3.2 (sin arrays asociativos) -- nombres por funcion,
# no por `declare -A`.
THEMES="0 1"
ACCENTS="0 4 3"

theme_name() { case "$1" in 0) echo dark ;; 1) echo light ;; esac; }
accent_name() { case "$1" in 0) echo blue ;; 4) echo magenta ;; 3) echo lime ;; esac; }

write_cfg() {
  local theme="$1" accent="$2"
  cat > "$AURA_CFG" <<EOF
firmware_family: moonlit
sync_marker_supported: 1
theme: $theme
accent: $accent
language: 0
animations: 1
graphics: 1
tz_local_quarters: 0
first_boot_done: 1
EOF
}

for theme in $THEMES; do
  for accent in $ACCENTS; do
    tname="$(theme_name "$theme")"
    aname="$(accent_name "$accent")"
    write_cfg "$theme" "$accent"

    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/${tname}-${aname}-hub.png" 200 ""
    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/${tname}-${aname}-list.png" 200 "SELECT,RIGHT,RIGHT"
    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/${tname}-${aname}-nowplaying.png" 200 "SELECT,RIGHT,RIGHT,SELECT"
  done
done

echo "==> Matriz completa en $OUT_DIR"
