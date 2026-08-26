#!/usr/bin/env python3
"""Verificacion mecanica de fuentes bitmap RB12 (D-006, D-007, D-032).

Dos modos:

  check_fonts.py <archivo.fnt>
      Lee la cabecera RB12 (firmware/rockbox/firmware/font.c:378-411,
      36 bytes, little-endian) e imprime firstchar/defaultchar/size/
      height/maxwidth. No falla por si solo -- es un reporte; quien
      genera la fuente (design-system/generate.py --fonts) es quien
      decide si un valor es aceptable.

  check_fonts.py --capheight <captura.png> [--rows N] [--min-px PX]
      Mide, sobre una captura del simulador, la altura en pixeles del
      primer glifo de cada fila de texto (asume metro_screen_specimen:
      una fila por rol, alineadas a la izquierda). Cuenta como trazo
      un pixel con luminancia > 60/255 (D-006). Falla si detecta menos
      de N filas o si alguna cae bajo min-px (10.5 px, D-005/ISO
      9241-303).
"""
import argparse
import struct
import sys

RB12_HEADER = struct.Struct("<4sHHHHiiiiii")
INK_THRESHOLD = 60  # D-006: luminancia > 60/255 cuenta como trazo visible
MIN_CAPHEIGHT_PX = 10.5  # D-005


def die(msg):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def read_rb12_header(path):
    with open(path, "rb") as f:
        data = f.read(RB12_HEADER.size)
    if len(data) < RB12_HEADER.size:
        die(f"{path}: archivo mas chico que la cabecera RB12 ({len(data)} bytes)")
    (magic, maxwidth, height, ascent, depth, firstchar, defaultchar,
     size, bits_size, noffset, nwidth) = RB12_HEADER.unpack(data)
    if magic != b"RB12":
        die(f"{path}: cabecera invalida (magic={magic!r}, se esperaba RB12)")
    return {
        "maxwidth": maxwidth, "height": height, "ascent": ascent, "depth": depth,
        "firstchar": firstchar, "defaultchar": defaultchar, "size": size,
        "bits_size": bits_size, "noffset": noffset, "nwidth": nwidth,
    }


def cmd_header(path):
    h = read_rb12_header(path)
    print(f"{path}: firstchar={h['firstchar']} defaultchar={h['defaultchar']} "
          f"size={h['size']} height={h['height']} maxwidth={h['maxwidth']}")


MAX_DIACRITIC_GAP_PX = 2  # ver comentario en _detect_row_bands


def _detect_row_bands(px, w, hgt):
    """Bandas horizontales con tinta, separadas por fondo. Una tilde o
    acento suelto (la de 'Á', D-005/D-006: el especimen usa acentos y
    enie) puede quedar separada del cuerpo de la letra por 1-2px de
    fondo -- eso produce una banda fantasma de 2-3px que no es una
    fila real (las separaciones reales entre filas, con el layout de
    metro_screen_specimen.c, miden >=3px). Se fusiona con su vecina
    cualquier banda separada por <= MAX_DIACRITIC_GAP_PX px."""
    def row_has_ink(y):
        return any(px[x, y] > INK_THRESHOLD for x in range(w))

    bands = []
    in_band = False
    band_start = 0
    for y in range(hgt):
        active = row_has_ink(y)
        if active and not in_band:
            in_band, band_start = True, y
        elif not active and in_band:
            in_band = False
            bands.append((band_start, y - 1))
    if in_band:
        bands.append((band_start, hgt - 1))

    merged = []
    for band in bands:
        if merged and band[0] - merged[-1][1] - 1 <= MAX_DIACRITIC_GAP_PX:
            merged[-1] = (merged[-1][0], band[1])
        else:
            merged.append(band)
    return merged


def _first_glyph_capheight(px, w, y0, y1):
    """Altura en px del primer glifo (de izquierda a derecha) dentro
    de la banda [y0, y1] -- en metro_screen_specimen ese glifo es
    siempre la 'H' inicial de SPECIMEN_STRING."""
    x = 0
    while x < w and not any(px[x, y] > INK_THRESHOLD for y in range(y0, y1 + 1)):
        x += 1
    if x >= w:
        return 0.0
    x_start = x
    while x < w and any(px[x, y] > INK_THRESHOLD for y in range(y0, y1 + 1)):
        x += 1
    x_end = x - 1

    min_y = max_y = None
    for yy in range(y0, y1 + 1):
        for xx in range(x_start, x_end + 1):
            if px[xx, yy] > INK_THRESHOLD:
                min_y = yy if min_y is None else min_y
                max_y = yy
    if min_y is None:
        return 0.0
    return float(max_y - min_y + 1)


def cmd_capheight(png_path, expected_rows, min_px):
    try:
        from PIL import Image
    except ImportError:
        die("falta Pillow -- design-system/.venv/bin/python3 (pip install pillow)")

    img = Image.open(png_path).convert("L")
    w, hgt = img.size
    px = img.load()

    bands = _detect_row_bands(px, w, hgt)
    # La primera banda es la ceja de metro_draw_header() (hora +
    # bateria), no una fila de rol -- se descarta antes de contar.
    role_bands = bands[1:]
    if len(role_bands) < expected_rows:
        die(f"{png_path}: se detectaron {len(role_bands)} filas de texto "
            f"(sin contar la ceja), se esperaban >= {expected_rows}")

    all_ok = True
    for i, (y0, y1) in enumerate(role_bands):
        cap_h = _first_glyph_capheight(px, w, y0, y1)
        ok = cap_h >= min_px
        all_ok = all_ok and ok
        print(f"fila {i}: cap-height {cap_h:.1f} px  {'OK' if ok else 'FAIL'}")

    if not all_ok:
        die(f"alguna fila no alcanza {min_px} px de cap-height (D-005)")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="?", help=".fnt a inspeccionar")
    parser.add_argument("--capheight", metavar="PNG",
                         help="mide cap-height por fila en una captura del especimen")
    parser.add_argument("--rows", type=int, default=7,
                         help="numero minimo de filas esperadas (default 7)")
    parser.add_argument("--min-px", type=float, default=MIN_CAPHEIGHT_PX,
                         help=f"cap-height minimo aceptado (default {MIN_CAPHEIGHT_PX})")
    args = parser.parse_args()

    if args.capheight:
        cmd_capheight(args.capheight, args.rows, args.min_px)
    elif args.path:
        cmd_header(args.path)
    else:
        parser.error("pasa una ruta .fnt o --capheight PNG")


if __name__ == "__main__":
    main()
