#!/usr/bin/env python3
"""Biblioteca sintetica para las capturas del README publico.

Por que existe aparte de gen_test_media.sh: aquel genera FIXTURES DE
QA -- nombres como "Flat Album Test", "Colon Artist Test" o "Metro QA"
que existen para ejercitar casos borde del parseo, y que en el README
publico se ven como lo que son, andamio de pruebas. Este arma una
biblioteca chica y presentable, con los MISMOS casos interesantes
(cirilico, diacriticos alemanes, puntuacion tipografica, titulo larguisimo
para la marquesina, album sin caratula para el monograma) pero con
nombres que se pueden mostrar.

Todo es ORIGINAL y sintetico, a proposito (Aura D-359: un simdisk
arrastraba portadas reales con derechos de rondas anteriores). Las
caratulas son composiciones geometricas generadas aqui con Pillow --
nunca imagenes de terceros -- y el audio son tonos senoidales. Ningun
nombre de artista, album o cancion corresponde a nadie real.

JPEG con Pillow, no con el codificador mjpeg de ffmpeg (D-303/D-030).

Uso:
    design-system/.venv/bin/python3 firmware/tools/gen_readme_media.py \\
        [--out firmware/build-sim/simdisk/Music] [--keep]

Sin --keep, borra el arbol de Music antes de escribir (que es lo que se
quiere para una captura limpia). Despues hay que borrar las caches
derivadas (/.aura/art, /.aura/thumbs, /.aura/tagcache) y dejar que el
simulador reconstruya, o las caratulas viejas siguen saliendo.
"""

import argparse
import math
import os
import shutil
import subprocess
import sys

from PIL import Image, ImageDraw

COVER_PX = 500


# --- caratulas: composiciones geometricas, ninguna imagen de terceros ---

def _base(bg):
    im = Image.new("RGB", (COVER_PX, COVER_PX), bg)
    return im, ImageDraw.Draw(im)


def wave(bg, fg, accent):
    im, d = _base(bg)
    for k in range(9):
        pts = []
        for x in range(0, COVER_PX + 8, 8):
            t = x / COVER_PX * math.pi * 2
            y = COVER_PX * 0.5 + math.sin(t + k * 0.42) * COVER_PX * 0.16 + (k - 4) * 26
            pts.append((x, y))
        d.line(pts, fill=(accent if k % 3 == 0 else fg), width=9, joint="curve")
    return im


def horizon(bg, fg, accent):
    im, d = _base(bg)
    d.ellipse([COVER_PX * .28, COVER_PX * .18, COVER_PX * .72, COVER_PX * .62], fill=accent)
    y, h = COVER_PX * 0.62, 16
    while y < COVER_PX:
        d.rectangle([0, y, COVER_PX, y + h], fill=fg)
        y += h * 2.1
        h *= 1.18
    return im


def grid(bg, fg, accent):
    im, d = _base(bg)
    step = COVER_PX // 7
    for i in range(1, 7):
        d.line([i * step, 0, i * step, COVER_PX], fill=fg, width=6)
        d.line([0, i * step, COVER_PX, i * step], fill=fg, width=6)
    d.polygon([(0, COVER_PX), (COVER_PX, 0), (COVER_PX, COVER_PX)], fill=accent)
    return im


def arcs(bg, fg, accent):
    im, d = _base(bg)
    cx, cy = COVER_PX * 0.5, COVER_PX * 0.72
    for i, r in enumerate(range(int(COVER_PX * 0.62), 20, -46)):
        d.ellipse([cx - r, cy - r, cx + r, cy + r],
                  outline=(accent if i % 2 else fg), width=14)
    return im


def blocks(bg, fg, accent):
    im, d = _base(bg)
    for i in range(4):
        for j in range(4):
            if (i + j) % 3 == 0:
                continue
            x, y = 30 + i * 112, 30 + j * 112
            d.rectangle([x, y, x + 88, y + 88],
                        fill=(accent if (i * j) % 4 == 0 else fg))
    return im


def crescent(bg, fg, accent):
    im, d = _base(bg)
    r = COVER_PX * 0.34
    cx, cy = COVER_PX * 0.52, COVER_PX * 0.5
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=fg)
    d.ellipse([cx - r * 1.32, cy - r * 1.06, cx + r * 0.62, cy + r * 1.06], fill=bg)
    d.ellipse([COVER_PX * .74, COVER_PX * .2, COVER_PX * .74 + 18, COVER_PX * .2 + 18],
              fill=accent)
    return im


def staves(bg, fg, accent):
    im, d = _base(bg)
    for k in range(6):
        y = 60 + k * 72
        for i in range(5):
            d.line([40, y + i * 9, COVER_PX - 40, y + i * 9], fill=fg, width=3)
        d.ellipse([70 + k * 54, y + 6, 70 + k * 54 + 26, y + 26], fill=accent)
    return im


# artista, album, generador (None = sin caratula), colores, canciones
ALBUMS = [
    ("Wheel & Click", "Analog Dreams", wave,
     ((18, 22, 34), (232, 236, 245), (150, 190, 255)),
     ["Tape Hiss", "Slow Rewind", "Analog Dreams", "Click Track", "Night Bus"]),

    ("Nocturne Atlas", "Night Drive", grid,
     ((12, 16, 28), (86, 104, 150), (120, 160, 255)),
     ["Ring Road", "Headlights", "Night Drive", "Toll Booth"]),

    ("Field Notes", "First Light", horizon,
     ((28, 20, 26), (250, 242, 232), (255, 168, 120)),
     ["Dawn Chorus", "First Light", "Long Shadow"]),

    # Puntuacion tipografica de verdad (D-074): comillas curvas, raya,
    # puntos suspensivos y apostrofo, todos en la fuente de puntuacion.
    ("Journey’s Edge", "Don’t Stop — “Live”…", arcs,
     ((24, 18, 20), (240, 226, 220), (226, 122, 96)),
     ["Don’t Stop — “Live”…", "Halfway There", "Encore"]),

    # Diacriticos alemanes en el rango primario (32-383, D-007).
    ("Käfer & Größe", "Weiße Straße", blocks,
     ((240, 238, 232), (34, 40, 56), (198, 88, 72)),
     ["Weiße Straße", "Grün", "Über den Fluß"]),

    # Cirilico (D-080/D-081): fuente aparte por rol.
    ("Лунный Свет", "Ночная Симфония", crescent,
     ((14, 16, 30), (226, 232, 248), (150, 190, 255)),
     ["Полночь", "Ночная Симфония", "Рассвет"]),

    # Titulo y artista largos: desborda y dispara la marquesina (D-067,
    # y las dos marquesinas desfasadas del panel de Marea, D-078).
    ("The Panoramic Foundation for Extended Symphonic Instrumentation",
     "Symphony No. 14 in F Sharp Minor, First Movement", staves,
     ((20, 24, 30), (228, 232, 240), (160, 200, 180)),
     ["I. Adagio sostenuto e molto tranquillo", "II. Allegro"]),

    # SIN caratula: el mosaico y Marea caen al monograma (D-065/D-070).
    ("Paper Lanterns", "Untitled Sessions", None,
     None,
     ["Sketch in Blue", "Untitled", "Last Take"]),
]


# --- fotos: paisajes geometricos, tambien originales ---

PHOTO_PX = (640, 480)


def _pbase(top, bottom):
    """Cielo con degradado vertical."""
    im = Image.new("RGB", PHOTO_PX)
    d = ImageDraw.Draw(im)
    w, h = PHOTO_PX
    for y in range(h):
        t = y / h
        d.line([0, y, w, y],
               fill=tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)))
    return im, d


def ph_dunes(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    d.ellipse([w * .68, h * .12, w * .68 + 90, h * .12 + 90], fill=ink)
    for k, base_y in enumerate((0.58, 0.68, 0.80)):
        pts = [(x, h * base_y + math.sin(x / w * math.pi * (1.5 + k)) * h * 0.06)
               for x in range(0, w + 10, 10)]
        d.polygon(pts + [(w, h), (0, h)],
                  fill=tuple(max(0, c - 26 * (3 - k)) for c in bottom))
    return im


def ph_peaks(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    d.polygon([(0, h), (w * .30, h * .28), (w * .56, h)],
              fill=tuple(max(0, c - 40) for c in bottom))
    d.polygon([(w * .34, h), (w * .66, h * .40), (w, h)],
              fill=tuple(max(0, c - 70) for c in bottom))
    d.ellipse([w * .12, h * .10, w * .12 + 64, h * .10 + 64], fill=ink)
    return im


def ph_shore(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    d.rectangle([0, h * .62, w, h], fill=tuple(max(0, c - 34) for c in bottom))
    for k in range(7):
        y = h * .66 + k * 22
        d.line([w * (.04 + .03 * k), y, w * (.96 - .03 * k), y], fill=ink, width=4)
    return im


def ph_city(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    x = 20
    heights = (.46, .60, .38, .68, .52, .72, .42, .58)
    for i, f in enumerate(heights):
        bw = 56 + (i % 3) * 14
        d.rectangle([x, h * f, x + bw, h], fill=tuple(max(0, c - 55) for c in bottom))
        for wy in range(int(h * f) + 14, int(h) - 14, 26):
            for wx in range(x + 10, x + bw - 10, 20):
                d.rectangle([wx, wy, wx + 8, wy + 12], fill=ink)
        x += bw + 12
    return im


def ph_forest(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    for i in range(9):
        x = 30 + i * 68
        th = h * (0.34 + (i % 3) * 0.08)
        d.polygon([(x, h * .92), (x + 34, th), (x + 68, h * .92)],
                  fill=tuple(max(0, c - 30 - (i % 3) * 18) for c in bottom))
    d.rectangle([0, h * .92, w, h], fill=ink)
    return im


def ph_bloom(top, bottom, ink):
    im, d = _pbase(top, bottom)
    w, h = PHOTO_PX
    cx, cy = w * .5, h * .52
    for k in range(12):
        a = k / 12 * math.pi * 2
        d.ellipse([cx + math.cos(a) * 110 - 46, cy + math.sin(a) * 110 - 46,
                   cx + math.cos(a) * 110 + 46, cy + math.sin(a) * 110 + 46],
                  outline=ink, width=5)
    return im


PHOTOS = [
    ("dunes.jpg",   ph_dunes,  ((252, 214, 170), (206, 138, 96),  (255, 246, 232))),
    ("peaks.jpg",   ph_peaks,  ((176, 206, 236), (86, 110, 140),  (250, 250, 245))),
    ("shore.jpg",   ph_shore,  ((196, 226, 238), (54, 108, 138),  (226, 244, 250))),
    ("skyline.jpg", ph_city,   ((38, 44, 78),    (22, 26, 44),    (255, 214, 140))),
    ("pines.jpg",   ph_forest, ((214, 226, 216), (46, 78, 62),    (34, 44, 38))),
    ("bloom.jpg",   ph_bloom,  ((28, 26, 44),    (58, 40, 70),    (240, 178, 210))),
]


def write_photos(root):
    os.makedirs(root, exist_ok=True)
    for name, fn, (top, bottom, ink) in PHOTOS:
        print("==> foto %s" % name)
        fn(top, bottom, ink).save(os.path.join(root, name),
                                  "JPEG", quality=90, subsampling=0)


def gen_track(path, title, artist, album, track_no, freq):
    subprocess.run(
        ["ffmpeg", "-y", "-loglevel", "error",
         "-f", "lavfi", "-i", "sine=frequency=%d:duration=3" % freq,
         "-metadata", "title=%s" % title,
         "-metadata", "artist=%s" % artist,
         "-metadata", "album=%s" % album,
         "-metadata", "track=%d" % track_no,
         "-c:a", "libmp3lame", "-b:a", "128k", path],
        check=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="firmware/build-sim/simdisk/Music")
    ap.add_argument("--keep", action="store_true",
                    help="no borrar el arbol existente antes de escribir")
    ap.add_argument("--photos", default="firmware/build-sim/simdisk/Photos",
                    help="donde escribir las fotos sinteticas")
    args = ap.parse_args()

    root = os.path.abspath(args.out)
    if os.path.isdir(root) and not args.keep:
        print("==> Borrando %s" % root)
        shutil.rmtree(root)
    os.makedirs(root, exist_ok=True)

    for artist, album, cover_fn, colors, tracks in ALBUMS:
        d = os.path.join(root, artist, album)
        os.makedirs(d, exist_ok=True)
        print("==> %s / %s (%d pistas%s)"
              % (artist, album, len(tracks),
                 "" if cover_fn else ", SIN caratula"))
        for i, title in enumerate(tracks, start=1):
            gen_track(os.path.join(d, "%02d %s.mp3" % (i, title.replace("/", "-"))),
                      title, artist, album, i, 300 + i * 40)
        if cover_fn:
            bg, fg, accent = colors
            cover_fn(bg, fg, accent).save(os.path.join(d, "cover.jpg"),
                                          "JPEG", quality=92, subsampling=0)

    photos_root = os.path.abspath(args.photos)
    if os.path.isdir(photos_root) and not args.keep:
        print("==> Borrando %s" % photos_root)
        shutil.rmtree(photos_root)
    write_photos(photos_root)

    print("\nListo. Ahora borra las caches derivadas y deja que el simulador "
          "las reconstruya:")
    print("  rm -rf firmware/build-sim/simdisk/.aura/{art,thumbs} \\")
    print("         firmware/build-sim/simdisk/.rockbox/aura/moonlitcache \\")
    print("         firmware/build-sim/simdisk/.aura/tagcache/*.tcd")
    return 0


if __name__ == "__main__":
    sys.exit(main())
