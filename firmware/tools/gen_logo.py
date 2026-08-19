#!/usr/bin/env python3
"""Genera el logo de arranque (wordmark "metro") como BMP nativo.

Reemplaza apps/bitmaps/native/rockboxlogo.320x98x16.bmp -- mismo
nombre de archivo y dimensiones exactas que el original de Rockbox
(320x98), asi la regla de apps/bitmaps/bitmaps.make que lo compila a
bm_rockboxlogo via bmp2rb no necesita ningun cambio. Ver DECISIONS.md
M-020.

Uso:
    firmware/tools/.venv/bin/python3 firmware/tools/gen_logo.py
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT_DIR = Path(__file__).resolve().parent.parent.parent
FONT_PATH = ROOT_DIR / "firmware/assets/fonts-src/Selawik-Light.ttf"
OUT_PATH = ROOT_DIR / "firmware/rockbox/apps/bitmaps/native/rockboxlogo.320x98x16.bmp"

WIDTH, HEIGHT = 320, 98
TEXT = "metro"
BG = (0, 0, 0)
FG = (255, 255, 255)


def main():
    font_size = 64
    font = ImageFont.truetype(str(FONT_PATH), font_size)

    img = Image.new("RGB", (WIDTH, HEIGHT), BG)
    draw = ImageDraw.Draw(img)

    bbox = draw.textbbox((0, 0), TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    x = (WIDTH - text_w) // 2 - bbox[0]
    y = (HEIGHT - text_h) // 2 - bbox[1]
    draw.text((x, y), TEXT, font=font, fill=FG)

    img.save(OUT_PATH, "BMP")
    print(f"==> Logo generado: {OUT_PATH} ({WIDTH}x{HEIGHT})")


if __name__ == "__main__":
    main()
