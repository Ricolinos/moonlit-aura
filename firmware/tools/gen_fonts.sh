#!/usr/bin/env bash
# Wrapper de design-system/generate.py --fonts (M2). La logica real
# (rango decimal 32-383 de convttf, D-007; verificacion de cabecera
# RB12; excepcion de tamano D-032) vive en ese script -- este archivo
# solo resuelve el venv y lo invoca, para que quien conocia
# `firmware/tools/gen_fonts.sh` de antes de M2 lo siga encontrando ahi.
#
# Uso:
#   firmware/tools/gen_fonts.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VENV_PY="$ROOT_DIR/design-system/.venv/bin/python3"

if [[ ! -x "$VENV_PY" ]]; then
  echo "==> Creando design-system/.venv (falta pillow para --icons/--logo mas adelante)"
  python3 -m venv "$ROOT_DIR/design-system/.venv"
  "$VENV_PY" -m pip install --quiet pillow
fi

exec "$VENV_PY" "$ROOT_DIR/design-system/generate.py" --fonts
