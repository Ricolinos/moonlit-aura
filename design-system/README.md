# design-system

`tokens.json` es el único origen de color, escala tipográfica, espaciado,
forma, elevación y movimiento del firmware (D-010). `generate.py` lo lee
y produce `firmware/rockbox/apps/metro/moonlit_tokens.h`.

```sh
python3 -m venv design-system/.venv
design-system/.venv/bin/pip install pillow
design-system/.venv/bin/python3 design-system/generate.py --header      # regenera moonlit_tokens.h
design-system/.venv/bin/python3 design-system/generate.py --contrast    # verifica WCAG on_surface/surface
design-system/.venv/bin/python3 design-system/generate.py --fonts       # regenera firmware/assets/fonts/*.fnt
design-system/.venv/bin/python3 design-system/generate.py --icons       # regenera moonlit_icons_table.c
design-system/.venv/bin/python3 design-system/generate.py --logo        # regenera moonlit_logo_table.c
```
