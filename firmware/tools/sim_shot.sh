#!/usr/bin/env bash
# Lanza el simulador Metro headless, opcionalmente inyecta una
# secuencia de botones, toma un screendump automatico y lo convierte a
# PNG. No requiere permisos de Grabacion de Pantalla ni de
# Accesibilidad de macOS: usa el mecanismo screen_dump() de Rockbox mas
# el parche de automatizacion en uisimulator/common/sim_tasks.c
# (heredado de Aura-Firmware D-008/D-017, ver MODIFICATIONS.md).
#
# Uso:
#   firmware/tools/sim_shot.sh <salida.png> [ticks] [botones]
#
# `ticks`   ticks del kernel de Rockbox (HZ ticks = 1s aprox.) a esperar
#           antes del dump -- desde el arranque si no hay `botones`, o
#           desde que termina de inyectarlos si los hay. Por defecto 150.
# `botones` lista separada por comas de: SELECT, MENU, SCROLL_FWD,
#           SCROLL_BACK, PLAY, LEFT, RIGHT, mas HOLD (conmuta el
#           interruptor Hold, moonlit D-069) -- se inyectan en orden antes
#           de tomar el dump. Ejemplo: "SCROLL_FWD,SCROLL_FWD,SELECT"

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/firmware/build-sim"
OUT_PNG="${1:?Uso: sim_shot.sh <salida.png> [ticks] [botones]}"
TICKS="${2:-150}"
BUTTONS="${3:-}"

# Resolver la ruta de salida ANTES de cambiar de directorio: un
# argumento relativo (el uso normal, p.ej. "docs/screenshots/x.png")
# debe interpretarse relativo al cwd del llamador, no a $BUILD_DIR.
if [[ "$OUT_PNG" != /* ]]; then
  OUT_PNG="$(pwd)/$OUT_PNG"
fi

cd "$BUILD_DIR"
rm -f simdisk/dump*.bmp

export METRO_SIM_AUTODUMP_TICKS="$TICKS"
export METRO_SIM_AUTODUMP_QUIT=1
if [[ -n "$BUTTONS" ]]; then
  export METRO_SIM_BUTTONS="$BUTTONS"
else
  unset METRO_SIM_BUTTONS 2>/dev/null || true
fi

RUN_LOG="$(mktemp -t moonlit_sim_shot)"
trap 'rm -f "$RUN_LOG"' EXIT
./rockboxui > "$RUN_LOG" 2>&1 || true

# moonlit (D-081): una fuente que no cupo en MAXUSERFONTS (font.h) falla
# en silencio para el usuario -- moonlit_fonts.c cae al font id
# primario en vez de estrellarse, asi que una ranura agotada nunca se
# veia sin medir a mano (asi se encontro el hueco de D-081 con
# MAXUSERFONTS en 16 y las cirilicas de body/label). Cualquier captura
# es tambien, gratis, un chequeo de que las 7+6+7 fuentes cargaron.
if grep -q "failed to load" "$RUN_LOG"; then
  echo "ERROR: una fuente no cargo (ver DEBUGF abajo) -- probable MAXUSERFONTS agotado (firmware/export/font.h)" >&2
  grep "failed to load" "$RUN_LOG" >&2
  exit 1
fi

DUMP_FILE="$(ls -t simdisk/dump*.bmp 2>/dev/null | head -1)"
if [[ -z "$DUMP_FILE" ]]; then
  echo "ERROR: no se genero ningun dump.bmp" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUT_PNG")"
sips -s format png "$DUMP_FILE" --out "$OUT_PNG" > /dev/null
rm -f "$DUMP_FILE"
echo "==> Screenshot guardado en $OUT_PNG"
