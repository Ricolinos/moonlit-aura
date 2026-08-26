#!/usr/bin/env python3
"""Verificacion mecanica de tonos de tinta sobre una imagen ya
renderizada (D-008, D-033) -- el mismo criterio que
design-system/generate.py --icons aplica sobre las mascaras de
cobertura antes de compilarlas, pero corrido sobre una captura del
simulador (docs/screenshots/*.png), donde el icono ya esta mezclado
contra el color de fondo real de la pantalla.

Uso:
  check_tones.py <imagen.png> [--region x,y,w,h] [--min N]
      Cuenta colores RGB distintos dentro de la region (o de toda la
      imagen si se omite --region). Falla si hay menos de --min (4).

  check_tones.py --edge <imagen.png> --row Y --x0 X0 --x1 X1 [--interior-dx N]
      Verifica el borde de elevacion tonal (D-012, moonlit_elevation.c,
      hito M4): en la fila Y, el pixel en X0 (borde izquierdo/superior,
      EDGE_LIGHT) debe ser mas luminoso que el interior de la tarjeta, y
      el pixel en X1 (borde derecho/inferior, EDGE_SHADOW) debe ser mas
      oscuro que el interior -- "luz desde arriba-izquierda" (identidad
      Waning Crescent). El "interior" se muestrea a `--interior-dx` (12
      por defecto) hacia el centro desde cada borde, misma fila Y.
      Luminancia relativa WCAG (la misma formula que
      design-system/generate.py --contrast). Falla si cualquiera de las
      dos comparaciones no se cumple.
"""
import argparse
import sys


def die(msg):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def count_tones(image, region):
    if region is not None:
        x, y, w, h = region
        image = image.crop((x, y, x + w, y + h))
    w, h = image.size
    pixels = image.load()
    return {pixels[x, y] for x in range(w) for y in range(h)}


# Misma formula que design-system/generate.py (WCAG 2.x, sRGB -> luminancia
# lineal) -- duplicada a proposito: este script no importa generate.py
# (herramientas independientes, sin dependencia cruzada).
def _srgb_channel(c):
    c = c / 255.0
    return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4


def relative_luminance(rgb):
    r, g, b = (_srgb_channel(c) for c in rgb)
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def check_edge(image, row, x0, x1, interior_dx):
    pixels = image.load()
    w, h = image.size

    if not (0 <= row < h):
        die(f"--row {row} fuera de la imagen (alto {h})")
    if not (0 <= x0 < w and 0 <= x1 < w):
        die(f"--x0/--x1 fuera de la imagen (ancho {w})")

    interior_left = min(x0 + interior_dx, w - 1)
    interior_right = max(x1 - interior_dx, 0)

    light_edge = relative_luminance(pixels[x0, row])
    light_interior = relative_luminance(pixels[interior_left, row])
    shadow_edge = relative_luminance(pixels[x1, row])
    shadow_interior = relative_luminance(pixels[interior_right, row])

    left_ok = light_edge > light_interior
    right_ok = shadow_edge < shadow_interior

    print(f"row={row} x0={x0} (L={light_edge:.4f}) vs interior x={interior_left} (L={light_interior:.4f})  "
          f"{'OK' if left_ok else 'FAIL'}")
    print(f"row={row} x1={x1} (L={shadow_edge:.4f}) vs interior x={interior_right} (L={shadow_interior:.4f})  "
          f"{'OK' if right_ok else 'FAIL'}")

    if left_ok and right_ok:
        print("left lighter, right darker: OK")
        return True

    print("left lighter, right darker: FAIL")
    return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image")
    parser.add_argument("--region", metavar="x,y,w,h",
                         help="recorta la imagen antes de contar (default: imagen completa)")
    parser.add_argument("--min", type=int, default=4,
                         help="minimo de tonos distintos aceptado (default 4, D-008)")
    parser.add_argument("--edge", action="store_true",
                         help="verifica el borde de elevacion luz/sombra (D-012) en vez de contar tonos")
    parser.add_argument("--row", type=int, help="--edge: fila Y a muestrear")
    parser.add_argument("--x0", type=int, help="--edge: columna del borde izquierdo/superior (EDGE_LIGHT)")
    parser.add_argument("--x1", type=int, help="--edge: columna del borde derecho/inferior (EDGE_SHADOW)")
    parser.add_argument("--interior-dx", type=int, default=12,
                         help="--edge: distancia desde cada borde hacia el centro para el pixel 'interior' (default 12)")
    args = parser.parse_args()

    try:
        from PIL import Image
    except ImportError:
        die("falta Pillow -- design-system/.venv/bin/python3 (pip install pillow)")

    img = Image.open(args.image).convert("RGB")

    if args.edge:
        if args.row is None or args.x0 is None or args.x1 is None:
            die("--edge requiere --row, --x0 y --x1")
        ok = check_edge(img, args.row, args.x0, args.x1, args.interior_dx)
        if not ok:
            sys.exit(1)
        return

    region = None
    if args.region:
        parts = args.region.split(",")
        if len(parts) != 4:
            die("--region espera x,y,w,h")
        region = tuple(int(p) for p in parts)

    tones = count_tones(img, region)
    ok = len(tones) >= args.min

    where = f" region={args.region}" if args.region else ""
    print(f"{args.image}{where}: {len(tones)} tono(s), minimo {args.min}  "
          f"{'OK' if ok else 'FAIL'}")

    if not ok:
        sys.exit(1)


if __name__ == "__main__":
    main()
