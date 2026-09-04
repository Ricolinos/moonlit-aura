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

  check_tones.py --align <captura.png> [--bar-h 20] [--tol 1]
      moonlit (D-068, maestro SS H). Mide la caja de TINTA de cada
      elemento de la barra de estado sobre una captura del simulador y
      exige que sus centros verticales coincidan dentro de --tol
      pixeles. Un elemento = un grupo de columnas contiguas con tinta
      dentro de la barra, separado del siguiente por al menos
      --gap columnas en blanco (4 por defecto). Es la unica forma de
      comprobar la alineacion sin mirarla: el ojo no distingue 1.5 px
      de desfase, y era justo el error que tenia el texto (4 px).

      OJO al comparar con el codigo: el firmware centra el texto por su
      ALTURA DE MAYUSCULAS (D-068) y esta herramienta mide la caja de
      TINTA. En un titulo con acentos ("musica") la tilde sube por
      encima de la mayuscula, asi que la tinta queda medio pixel mas
      arriba que la caja de mayusculas. Es esperado: no "corrijas" el
      codigo para que la herramienta de 0.0 -- la tolerancia de 1 px
      existe justo para eso.
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


INK_LUMA = 60  # D-006: > 60/255 cuenta como trazo visible

# --align (D-068): "tinta" NO es "pixel claro" sino "pixel que se
# distingue del fondo de la barra". Un umbral absoluto de luminancia
# mide brillo, no marca: con el esquema `dawn` (fondo claro, tinta
# oscura) daria justo al reves, y con un icono en secundario sobre
# surface_container_lowest deja fuera filas que SI se ven. El fondo se
# deduce de la propia captura -- el color mas repetido dentro de la
# barra -- y cuenta como tinta lo que se aparta de el.
ALIGN_INK_DELTA = 24


def _luma(px):
    r, g, b = px[:3]
    return (r * 299 + g * 587 + b * 114) // 1000


def cmd_align(image, bar_h, tol, gap, min_w):
    """Caja de tinta de cada grupo de columnas con tinta dentro de la barra."""
    from collections import Counter

    w, h = image.size
    px = image.convert("RGB").load()
    bar_h = min(bar_h, h)

    counts = Counter(px[x, y] for x in range(w) for y in range(bar_h))
    bg = counts.most_common(1)[0][0]
    bg_luma = _luma(bg)
    print(f"  fondo de la barra {bg} (luma {bg_luma}), "
          f"tinta = |luma - fondo| > {ALIGN_INK_DELTA}")

    def is_ink(x, y):
        return abs(_luma(px[x, y]) - bg_luma) > ALIGN_INK_DELTA

    ink_col = []
    for x in range(w):
        ink_col.append(any(is_ink(x, y) for y in range(bar_h)))

    groups = []
    x = 0
    while x < w:
        if not ink_col[x]:
            x += 1
            continue
        x0 = x
        blanks = 0
        while x < w and blanks < gap:
            x += 1
            if x < w and ink_col[x]:
                blanks = 0
            else:
                blanks += 1
        x1 = x - blanks
        if x1 - x0 >= min_w:
            groups.append((x0, x1))

    if len(groups) < 2:
        die(f"solo se detectaron {len(groups)} elemento(s) en la barra -- "
            f"la captura no parece de una pantalla con barra de estado")

    rows_of = []
    print(f"== alineacion de la barra de estado (alto {bar_h} px, "
          f"tolerancia +-{tol} px) ==")
    for x0, x1 in groups:
        rows = [y for y in range(bar_h)
                if any(is_ink(x, y) for x in range(x0, x1))]
        top, bot = rows[0], rows[-1]
        center = (top + bot) / 2.0
        rows_of.append(center)
        print(f"  x {x0:>3}..{x1:<3}  tinta y {top:>2}..{bot:<2}  centro {center:>5.1f}")

    spread = max(rows_of) - min(rows_of)
    print(f"  dispersion de centros: {spread:.1f} px")
    if spread > tol:
        die(f"los centros de la barra difieren {spread:.1f} px (tope {tol})")
    print("check_tones: la barra esta alineada.")


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
    parser.add_argument("--align", action="store_true",
                         help="mide la alineacion vertical de la barra de estado (D-068)")
    parser.add_argument("--bar-h", type=int, default=20,
                         help="--align: alto de la barra en px (20)")
    parser.add_argument("--tol", type=float, default=1.0,
                         help="--align: dispersion maxima de centros en px (1)")
    parser.add_argument("--gap", type=int, default=4,
                         help="--align: columnas en blanco que separan dos elementos (4)")
    parser.add_argument("--min-w", type=int, default=3,
                         help="--align: ancho minimo de un elemento en px (3)")
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

    if args.align:
        cmd_align(img, args.bar_h, args.tol, args.gap, args.min_w)
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
