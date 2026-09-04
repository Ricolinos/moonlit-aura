#!/usr/bin/env python3
"""Verificacion mecanica de fuentes bitmap RB12 (D-006, D-007, D-032).

Dos modos:

  check_fonts.py <archivo.fnt>
      Lee la cabecera RB12 (firmware/rockbox/firmware/font.c:378-411,
      36 bytes, little-endian) e imprime firstchar/defaultchar/size/
      height/maxwidth. No falla por si solo -- es un reporte; quien
      genera la fuente (design-system/generate.py --fonts) es quien
      decide si un valor es aceptable.

  check_fonts.py --coverage [--fonts DIR] [--lang RUTA] [--tags ARCHIVO]
      moonlit (D-066). Lee la tabla de glifos de cada .fnt y la compara
      con lo que la UI y los metadatos van a pedirle: (a) todas las
      cadenas de metro_lang.c, (b) una lista curada de codepoints
      frecuentes en metadatos de musica, y (c) opcionalmente un volcado
      de tags (un archivo de texto cualquiera; se leen sus codepoints).
      Reporta los faltantes por rol. Falla si falta algo de (a) -- la
      UI propia no puede tener huecos; lo de (b)/(c) se reporta pero no
      rompe, porque la respuesta ahi puede ser la transliteracion.

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


# --- coverage (D-066) -----------------------------------------------------

# font.c:50. Por debajo de este bits_size la tabla de offsets es de 16
# bits; por encima, de 32. Hace falta para saltar hasta la tabla de
# anchos, que es la que dice si un codigo tiene glifo de verdad.
MAX_FONTSIZE_FOR_16_BIT_OFFSETS = 0xFFDB

# Codepoints frecuentes en metadatos de musica reales que NO estan en el
# rango 32-383 de D-007. Lista curada (plan de la ronda, Fase 3.1): lo
# que de verdad aparece en titulos de disco, no todo Unicode.
METADATA_CODEPOINTS = (
    list(range(0x2010, 0x2028)) +   # guiones, comillas tipograficas, puntos suspensivos
    list(range(0x2030, 0x203B)) +   # por mil, primas, angulares, dagas, vinetas
    [0x2122,                        # (TM)
     0x266A, 0x266B,                # corcheas
     0x2605, 0x2606,                # estrellas
     0x2665,                        # corazon
     0x2022,                        # vineta
     0x00A0]                        # espacio duro
)


def read_glyph_table(path):
    """-> (firstchar, size, set de codepoints con glifo real).

    Un codigo dentro de [firstchar, firstchar+size) puede seguir sin
    glifo: convttf emite una entrada de ancho 0 para el hueco (D-032 ya
    documenta un caso, U+017F en Libre Baskerville). Por eso no basta
    con el rango de la cabecera -- hay que leer la tabla de anchos."""
    h = read_rb12_header(path)
    with open(path, "rb") as f:
        data = f.read()

    # font.c:224-238: tras el bitmap se ALINEA (16 o 32 bits segun
    # bits_size) antes de la tabla de offsets. Sin ese relleno la tabla
    # de anchos sale corrida un byte y todo el reporte miente -- se
    # detecto asi: el ancho del espacio salia 0 en dos roles y el final
    # calculado del archivo no coincidia con su tamano real.
    off = RB12_HEADER.size + h["bits_size"]
    if h["bits_size"] < MAX_FONTSIZE_FOR_16_BIT_OFFSETS:
        off = (off + 1) & ~1
        off += h["noffset"] * 2
    else:
        off = (off + 3) & ~3
        off += h["noffset"] * 4

    # Cruce que impide leer la tabla corrida en silencio: con el
    # relleno bien puesto, la tabla de anchos termina EXACTAMENTE donde
    # termina el archivo.
    if off + h["nwidth"] != len(data):
        die(f"{path}: la tabla de anchos termina en {off + h['nwidth']} "
            f"pero el archivo mide {len(data)} -- el calculo de "
            f"desplazamiento no coincide con font.c")

    widths = data[off:off + h["nwidth"]]
    covered = set()
    if h["nwidth"] == 0:
        # sin tabla de anchos: fuente de ancho fijo, todo el rango cuenta
        covered = set(range(h["firstchar"], h["firstchar"] + h["size"]))
    else:
        for i, w in enumerate(widths):
            if w > 0:
                covered.add(h["firstchar"] + i)
    return h, covered


C_STRING_RE = None


def lang_codepoints(lang_path):
    """Codepoints de todas las cadenas literales de metro_lang.c."""
    import re
    text = open(lang_path, encoding="utf-8").read()
    # Quita comentarios de bloque y de linea antes de buscar literales,
    # para no recoger acentos que solo viven en la prosa del comentario.
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    cps = set()
    for lit in re.findall(r'"((?:[^"\\]|\\.)*)"', text):
        lit = lit.replace("\\n", "\n").replace("\\t", "\t").replace('\\"', '"')
        for ch in lit:
            cps.add(ord(ch))
    return cps


def file_codepoints(path):
    cps = set()
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            for ch in line.rstrip("\n"):
                cps.add(ord(ch))
    return cps


def translit_codepoints(translit_path):
    """Codepoints que apps/metro/moonlit_translit.c resuelve sin fuente.

    Se leen de la tabla misma (formato `{ 0xNNNN, "..." },`, una por
    linea) para que no haya dos listas que mantener sincronizadas: si
    alguien agrega una entrada alla, este reporte la ve sola."""
    import re
    import os

    if not os.path.exists(translit_path):
        return set()
    text = open(translit_path, encoding="utf-8").read()
    return {int(m, 16) for m in re.findall(r"\{\s*0x([0-9A-Fa-f]{2,6})\s*,\s*\"",
                                           text)}


def fmt_cp(cp):
    ch = chr(cp)
    shown = ch if ch.isprintable() and not ch.isspace() else " "
    return f"U+{cp:04X} '{shown}'"


def cmd_coverage(fonts_dir, lang_path, tags_path, translit_path):
    import glob
    import os

    fonts = sorted(glob.glob(os.path.join(fonts_dir, "*.fnt")))
    if not fonts:
        die(f"no hay .fnt en {fonts_dir}")

    ui = lang_codepoints(lang_path)
    meta = set(METADATA_CODEPOINTS)
    tags = file_codepoints(tags_path) if tags_path else set()
    translit = translit_codepoints(translit_path)

    print(f"== cobertura de glifos ==  UI {len(ui)} codepoints, "
          f"metadatos {len(meta)}, tags {len(tags)}, "
          f"transliterados {len(translit)} (D-066)")
    ui_failures = 0

    for path in fonts:
        h, covered = read_glyph_table(path)
        # Un codepoint transliterado no necesita glifo: se dibuja con su
        # equivalente ASCII, que si esta en el rango.
        covered = covered | translit
        name = os.path.basename(path)
        miss_ui = sorted(c for c in ui if c not in covered and c >= 32)
        miss_meta = sorted(c for c in meta if c not in covered)
        miss_tags = sorted(c for c in tags if c not in covered and c >= 32)

        print(f"\n{name}: firstchar={h['firstchar']} size={h['size']} "
              f"defaultchar={h['defaultchar']} glifos_reales={len(covered)}")
        if miss_ui:
            ui_failures += 1
            print(f"   FALTA de la UI ({len(miss_ui)}): "
                  + ", ".join(fmt_cp(c) for c in miss_ui[:12])
                  + (" ..." if len(miss_ui) > 12 else ""))
        else:
            print("   UI: completa")
        print(f"   metadatos: faltan {len(miss_meta)}/{len(meta)}"
              + (("  " + ", ".join(fmt_cp(c) for c in miss_meta[:8])
                  + (" ..." if len(miss_meta) > 8 else "")) if miss_meta else ""))
        if tags:
            print(f"   tags: faltan {len(miss_tags)}/{len(tags)}"
                  + (("  " + ", ".join(fmt_cp(c) for c in miss_tags[:8])
                      + (" ..." if len(miss_tags) > 8 else "")) if miss_tags else ""))

    if ui_failures:
        die(f"{ui_failures} fuente(s) sin cubrir la UI de metro_lang.c")
    print("\ncheck_fonts: la UI esta cubierta en todos los roles.")


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
    parser.add_argument("--coverage", action="store_true",
                         help="compara la tabla de glifos de cada .fnt con la UI y los metadatos")
    parser.add_argument("--fonts", default="firmware/assets/fonts",
                         help="directorio de .fnt para --coverage")
    parser.add_argument("--lang", default="firmware/rockbox/apps/metro/metro_lang.c",
                         help="fuente de las cadenas de UI para --coverage")
    parser.add_argument("--tags", help="volcado de tags (texto) para --coverage")
    parser.add_argument("--translit",
                         default="firmware/rockbox/apps/metro/moonlit_translit.c",
                         help="tabla de transliteracion (D-066) para --coverage")
    args = parser.parse_args()

    if args.coverage:
        cmd_coverage(args.fonts, args.lang, args.tags, args.translit)
    elif args.capheight:
        cmd_capheight(args.capheight, args.rows, args.min_px)
    elif args.path:
        cmd_header(args.path)
    else:
        parser.error("pasa una ruta .fnt o --capheight PNG")


if __name__ == "__main__":
    main()
