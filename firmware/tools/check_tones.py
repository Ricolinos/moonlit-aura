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

El modo --edge para verificar el borde de elevacion luz/sombra
(D-012: izquierda mas clara, derecha mas oscura) llega en el hito M4
(05-plan-correctivo.md), cuando exista moonlit_elevation.c para
producirlo.
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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image")
    parser.add_argument("--region", metavar="x,y,w,h",
                         help="recorta la imagen antes de contar (default: imagen completa)")
    parser.add_argument("--min", type=int, default=4,
                         help="minimo de tonos distintos aceptado (default 4, D-008)")
    args = parser.parse_args()

    try:
        from PIL import Image
    except ImportError:
        die("falta Pillow -- design-system/.venv/bin/python3 (pip install pillow)")

    region = None
    if args.region:
        parts = args.region.split(",")
        if len(parts) != 4:
            die("--region espera x,y,w,h")
        region = tuple(int(p) for p in parts)

    img = Image.open(args.image).convert("RGB")
    tones = count_tones(img, region)
    ok = len(tones) >= args.min

    where = f" region={args.region}" if args.region else ""
    print(f"{args.image}{where}: {len(tones)} tono(s), minimo {args.min}  "
          f"{'OK' if ok else 'FAIL'}")

    if not ok:
        sys.exit(1)


if __name__ == "__main__":
    main()
