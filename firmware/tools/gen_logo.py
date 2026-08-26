#!/usr/bin/env python3
"""Genera el logo de arranque (wordmark provisional "moonlit.aura") como BMP nativo.

Reemplaza apps/bitmaps/native/rockboxlogo.320x98x16.bmp -- mismo
nombre de archivo y dimensiones exactas que el original de Rockbox
(320x98), asi la regla de apps/bitmaps/bitmaps.make que lo compila a
bm_rockboxlogo via bmp2rb no necesita ningun cambio. Ver DECISIONS.md
M-020.

Uso:
    python3 firmware/tools/gen_logo.py  (requiere Pillow)
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT_DIR = Path(__file__).resolve().parent.parent.parent
# moonlit M2 (D-032): la fuente vendoreada previa (M-020) se elimino
# del repo; el provisional usa la cara de titulos que D-004 ya trae
# vendoreada. gen_logo.py se retira por completo en M9 junto con este
# wordmark provisional (D-016/D-026).
FONT_PATH = ROOT_DIR / "design-system/vendor/libre-baskerville/LibreBaskerville-Regular.ttf"
OUT_PATH = ROOT_DIR / "firmware/rockbox/apps/bitmaps/native/rockboxlogo.320x98x16.bmp"

WIDTH, HEIGHT = 320, 98
# moonlit (D-026): wordmark provisional de una sola linea "moonlit.aura"
# hasta que H5 traiga el logotipo Waning Crescent (D-016). SUBTEXT vacio
# = sin segunda linea (el par "metro"/"aura" de M-092 queda retirado).
TEXT = "moonlit.aura"
SUBTEXT = ""
BG = (0, 0, 0)
FG = (255, 255, 255)
SUBFG = (153, 153, 153)


def main():
    font = ImageFont.truetype(str(FONT_PATH), 56)
    subfont = ImageFont.truetype(str(FONT_PATH), 26)

    img = Image.new("RGB", (WIDTH, HEIGHT), BG)
    draw = ImageDraw.Draw(img)

    bbox = draw.textbbox((0, 0), TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    sbbox = draw.textbbox((0, 0), SUBTEXT, font=subfont)
    sub_w = sbbox[2] - sbbox[0]
    sub_h = (sbbox[3] - sbbox[1]) if SUBTEXT else 0

    # "metro" arriba, "aura" debajo con 8px de aire; el bloque completo
    # centrado en el lienzo de 98px de alto.
    text_h = bbox[3] - bbox[1]
    total_h = text_h + (8 + sub_h if SUBTEXT else 0)
    top = (HEIGHT - total_h) // 2
    draw.text(((WIDTH - text_w) // 2 - bbox[0], top - bbox[1]), TEXT, font=font, fill=FG)
    if SUBTEXT:
        draw.text(((WIDTH - sub_w) // 2 - sbbox[0], top + text_h + 8 - sbbox[1]),
                  SUBTEXT, font=subfont, fill=SUBFG)

    img.save(OUT_PATH, "BMP")
    print(f"==> Logo generado: {OUT_PATH} ({WIDTH}x{HEIGHT})")


if __name__ == "__main__":
    main()
