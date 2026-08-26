#!/usr/bin/env bash
# F10 / moonlit (D-027, D-028, M4): captura la matriz de revision visual
# (05-plan-correctivo.md M4: "3 pantallas x 2 temas x 4 presets") --
# escribe un aura.cfg preconfigurado antes de cada captura
# (metro_settings_load() lo lee en el arranque, mismo mecanismo que un
# usuario real cambiando Ajustes > Pantalla) en vez de navegar el menu
# de Ajustes por cada combinacion, mucho mas rapido para 24 capturas.
# Reutiliza sim_shot.sh para el dump en si.
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

# theme: 0=night 1=dawn (metro_theme.h, D-027). accent: 0=moonstone
# (default) 1=tide 2=ember 3=moss -- los 4 presets MD3 completos
# (D-028; ya no son 10, cabe la matriz entera). macOS trae bash 3.2
# (sin arrays asociativos) -- nombres por funcion, no por `declare -A`.
THEMES="0 1"
ACCENTS="0 1 2 3"

theme_name() { case "$1" in 0) echo night ;; 1) echo dawn ;; esac; }
accent_name() { case "$1" in 0) echo moonstone ;; 1) echo tide ;; 2) echo ember ;; 3) echo moss ;; esac; }

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

# moonlit (M4): orden de archivo pantalla-tema-preset (p.ej.
# list-night-moonstone.png), el que usa la definicion de hecho de M4.
for theme in $THEMES; do
  for accent in $ACCENTS; do
    tname="$(theme_name "$theme")"
    aname="$(accent_name "$accent")"
    write_cfg "$theme" "$accent"

    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/hub-${tname}-${aname}.png" 400 ""
    # moonlit (M4): tres RIGHT, no dos -- quickplay(0)->artistas(1)->
    # albumes(2)->canciones(3). El comentario de este script siempre
    # dijo "musica > canciones" pero solo llegaba a "albumes" (una
    # cuadricula, no una lista de filas); hallado al verificar el
    # borde de elevacion (D-012) de la fila seleccionada, que una
    # cuadricula no tiene.
    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/list-${tname}-${aname}.png" 400 "SELECT,RIGHT,RIGHT,RIGHT"
    "$ROOT_DIR/firmware/tools/sim_shot.sh" \
      "$OUT_DIR/nowplaying-${tname}-${aname}.png" 400 "SELECT,RIGHT,RIGHT,SELECT"
  done
done

echo "==> Matriz completa en $OUT_DIR"
