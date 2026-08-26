#!/usr/bin/env python3
"""Pipeline determinista del sistema de diseno moonlit.aura (Waning Crescent).

Lee tokens.json (unica fuente de verdad, D-010) y produce:

  - apps/metro/moonlit_tokens.h    Header C con los 16 roles MD3 de color
                                    (D-028) por esquema (night/dawn, D-027),
                                    presets de acento, espaciado, forma,
                                    elevacion y movimiento (--header).
  - firmware/assets/fonts/*.fnt    Las 7 fuentes bitmap MD3 (Libre
                                    Baskerville + Montserrat estatica,
                                    D-004), via tools/convttf con rango
                                    decimal 32-383 (D-007) (--fonts).

  - apps/metro/moonlit_icons_table.c  Mascaras de cobertura de 8 bits
                                    (Material Symbols Rounded, D-008,
                                    D-033) para los iconos y tamanos de
                                    tokens.json:icon, via rsvg-convert +
                                    supersampleo 16x (--icons).

  - apps/metro/moonlit_logo_table.c   Mascaras de cobertura de 8 bits del
                                    logotipo Waning Crescent (D-016,
                                    D-044): creciente en tokens.json:
                                    logo.crescent_sizes y wordmark
                                    "moonlit" en Libre Baskerville a
                                    contornos (design-system/logo/*.svg),
                                    mismo pipeline de rasterizacion y
                                    verificacion de tonos que --icons,
                                    mas cobertura/cuspides a 16px
                                    (--logo).

No se porta nada del generador de Aura-Firmware mas alla de la forma de
tokens.json, el patron dict-plano -> #define de
generate_header()/generate_aura_ds_defines()
(AF/design-system/generate.py:124,197-262) y, para --icons/--logo, la
tecnica de supersampleo 16x + filtro de caja + MIN_INK_TONES de
AF/design-system/generate.py:372-391,475,575-583,626-632 (sin AppKit:
la rasterizacion es rsvg-convert sobre SVG vendoreados, como ya hacia
firmware/tools/gen_icons.py para los iconos Fluent que --icons
sustituye).
"""
import argparse
import json
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    Image = None

ROOT = Path(__file__).resolve().parent
TOKENS_PATH = ROOT / "tokens.json"
HEADER_OUT = ROOT.parent / "firmware" / "rockbox" / "apps" / "metro" / "moonlit_tokens.h"
ROCKBOX_TOOLS = ROOT.parent / "firmware" / "rockbox" / "tools"
CONVTTF = ROCKBOX_TOOLS / "convttf"
FONTS_OUT = ROOT.parent / "firmware" / "assets" / "fonts"
ICONS_VENDOR_DIR = ROOT / "vendor" / "material-symbols"
ICONS_OUT = ROOT.parent / "firmware" / "rockbox" / "apps" / "metro" / "moonlit_icons_table.c"
LOGO_VENDOR_DIR = ROOT / "logo"
LOGO_OUT = ROOT.parent / "firmware" / "rockbox" / "apps" / "metro" / "moonlit_logo_table.c"
# D-050: bitmap de arranque de Rockbox (apps/main.c show_logo_boot()) --
# mismo nombre/dimensiones que el original para que la regla de
# apps/bitmaps/bitmaps.make (bmp2rb -> bm_rockboxlogo) no cambie. El
# "x16" del nombre es la profundidad NATIVA que bmp2rb produce para el
# LCD; el archivo en disco es un BMP Windows 3.x de 24 bpp sin
# compresion, igual que el original de Rockbox (`file`: "320 x 98 x 24").
BOOTLOGO_OUT = ROOT.parent / "firmware" / "rockbox" / "apps" / "bitmaps" / "native" / "rockboxlogo.320x98x16.bmp"
BOOTLOGO_TONES_OUT = ROOT.parent / "docs" / "screenshots" / "v0.1.1-bootlogo-tones.txt"
BOOTLOGO_W, BOOTLOGO_H = 320, 98
BOOTLOGO_CRESCENT_PX = 72

SCHEMES = ("night", "dawn")

# D-028: exactamente estos 16 roles en color.night / color.dawn.
COLOR_ROLES = [
    "primary", "on_primary", "primary_container", "on_primary_container",
    "surface", "surface_dim", "surface_bright",
    "surface_container_lowest", "surface_container_low", "surface_container",
    "surface_container_high", "surface_container_highest",
    "on_surface", "on_surface_variant", "outline", "outline_variant",
]

# Niveles de superficie con borde luz/sombra (D-012).
ELEVATION_LEVELS = [
    "surface_container_lowest", "surface_container_low", "surface_container",
    "surface_container_high", "surface_container_highest",
]

PRESET_ROLES = ["primary", "on_primary", "primary_container", "on_primary_container"]


def die(msg):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def hex_to_rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i : i + 2], 16) for i in (0, 2, 4))


def clamp(v):
    return max(0, min(255, v))


def rgb_defines(name, rgb):
    r, g, b = rgb
    rgb24 = (r << 16) | (g << 8) | b
    return [
        f"#define {name} LCD_RGBPACK({r}, {g}, {b})",
        f"#define {name}_RGB24 0x{rgb24:06X}",
    ]


def edge_rgb(base_rgb, delta):
    return tuple(clamp(c + delta) for c in base_rgb)


def generate_header(tokens):
    print("==> Generando apps/metro/moonlit_tokens.h")
    lines = []
    lines.append("/* Generado por design-system/generate.py a partir de tokens.json. */")
    lines.append("/* NO editar a mano: los cambios se perderian al regenerar. */")
    lines.append("#ifndef MOONLIT_TOKENS_H")
    lines.append("#define MOONLIT_TOKENS_H")
    lines.append("")
    lines.append("/* LCD_RGBPACK depende del formato de pixel del target -- lo trae")
    lines.append(" * \"lcd.h\" de Rockbox. Un test host que no enlaza Rockbox puede")
    lines.append(" * definirlo antes de este include (solo necesita los *_RGB24). */")
    lines.append("#ifndef LCD_RGBPACK")
    lines.append("#define LCD_RGBPACK(r, g, b) (((r) << 16) | ((g) << 8) | (b))")
    lines.append("#endif")
    lines.append("")

    screen = tokens["screen"]
    lines.append(f"#define MOONLIT_SCREEN_WIDTH  {screen['width']}")
    lines.append(f"#define MOONLIT_SCREEN_HEIGHT {screen['height']}")
    lines.append(f"#define MOONLIT_SCREEN_DPI    {screen['dpi']}")
    lines.append("")

    lines.append("/* Espaciados (px, grilla MD3) */")
    for name, value in tokens["spacing"].items():
        lines.append(f"#define MOONLIT_SPACING_{name.upper()} {value}")
    lines.append("")

    lines.append("/* Escala de forma: radios de esquina (px) */")
    for name, value in tokens["shape"].items():
        lines.append(f"#define MOONLIT_{name.upper()} {value}")
    lines.append("")

    elevation = tokens["elevation"]
    lines.append("/* Elevacion tonal (D-012): deltas por canal aplicados a cada nivel")
    lines.append(" * surface_container_<nivel> mas abajo -- ver EDGE_LIGHT/EDGE_SHADOW. */")
    lines.append(f"#define MOONLIT_ELEVATION_LIGHT_EDGE_DELTA {elevation['light_edge_delta']}")
    lines.append(f"#define MOONLIT_ELEVATION_SHADOW_EDGE_DELTA {elevation['shadow_edge_delta']}")
    lines.append(f"#define MOONLIT_ELEVATION_EDGE_PX {elevation['edge_px']}")
    lines.append("")

    motion = tokens["motion"]
    lines.append("/* Movimiento -- solo bajo lcd_active() (regla del repo) */")
    lines.append(f"#define MOONLIT_MOTION_TRANSITION_MS {motion['transition_ms']}")
    lines.append(f'#define MOONLIT_MOTION_EASE "{motion["ease"]}"')
    lines.append("")

    color = tokens["color"]
    light_delta = elevation["light_edge_delta"]
    shadow_delta = elevation["shadow_edge_delta"]

    for scheme in SCHEMES:
        roles = color[scheme]
        missing = [r for r in COLOR_ROLES if r not in roles]
        if missing:
            die(f"color.{scheme} no tiene los roles MD3 {missing} (D-028)")

        lines.append(f"/* Esquema {scheme} -- 16 roles MD3 (D-028) */")
        for role in COLOR_ROLES:
            name = f"MOONLIT_{scheme.upper()}_{role.upper()}"
            lines.extend(rgb_defines(name, hex_to_rgb(roles[role])))
        lines.append("")

        lines.append(f"/* Esquema {scheme} -- bordes de elevacion (D-012): luz arriba-izquierda,")
        lines.append(" * sombra abajo-derecha, precalculados por canal, cero costo por cuadro. */")
        for level in ELEVATION_LEVELS:
            base_rgb = hex_to_rgb(roles[level])
            light_name = f"MOONLIT_{scheme.upper()}_{level.upper()}_EDGE_LIGHT"
            shadow_name = f"MOONLIT_{scheme.upper()}_{level.upper()}_EDGE_SHADOW"
            lines.extend(rgb_defines(light_name, edge_rgb(base_rgb, light_delta)))
            lines.extend(rgb_defines(shadow_name, edge_rgb(base_rgb, shadow_delta)))
        lines.append("")

    presets = color["primary_presets"]
    lines.append("/* Presets de acento (sustituyen enum metro_accent, D-028) */")
    for preset_name, per_scheme in presets.items():
        if preset_name == "comment":
            continue
        for scheme in SCHEMES:
            roles = per_scheme[scheme]
            missing = [r for r in PRESET_ROLES if r not in roles]
            if missing:
                die(f"color.primary_presets.{preset_name}.{scheme} no tiene {missing}")
            for role in PRESET_ROLES:
                name = f"MOONLIT_{scheme.upper()}_{preset_name.upper()}_{role.upper()}"
                lines.extend(rgb_defines(name, hex_to_rgb(roles[role])))
    lines.append("")

    lines.append("#endif /* MOONLIT_TOKENS_H */")
    lines.append("")

    HEADER_OUT.parent.mkdir(parents=True, exist_ok=True)
    HEADER_OUT.write_text("\n".join(lines))


# D-007: rango decimal 32-383 (charmap 0x20-0x17F expresado en decimal,
# nunca hex -- convttf.c:1101,1113 usa atoi()/atol(), que no entienden
# "0x..." y truncan a 0). '?' = 63 decimal como defaultchar.
FONT_CHARSET_START = 32
FONT_CHARSET_LIMIT = 383
FONT_CHARSET_DEFAULT = 63
FONT_EXPECTED_SIZE = FONT_CHARSET_LIMIT - FONT_CHARSET_START + 1  # 352

# D-032: Libre Baskerville-Regular no trae el glifo U+017F ("long s",
# codigo 383 del rango 32-383) -- convttf.c:693 salta cualquier codigo
# sin glifo al calcular lastchar, asi que topa en U+017E (382) y
# size = 382-32+1 = 351 en vez de 352. Montserrat si lo trae completo.
# Caracter historico sin uso en espanol/UI (metro_lang.c) -- sin
# impacto funcional. Excepcion documentada, no un umbral relajado.
FONT_SIZE_EXCEPTIONS = {
    "libre-baskerville-regular": FONT_EXPECTED_SIZE - 1,  # 351
}

RB12_HEADER = struct.Struct("<4sHHHHiiiiii")


def ensure_convttf():
    if CONVTTF.exists():
        return
    print("==> Compilando convttf (herramienta de fuentes de Rockbox)...")
    freetype_flags = subprocess.run(
        ["pkg-config", "--cflags", "--libs", "freetype2"],
        capture_output=True, text=True,
    )
    if freetype_flags.returncode != 0:
        die("pkg-config freetype2 no disponible (brew install freetype pkg-config)")
    cmd = (
        ["cc", "-lm", "-std=c99", "-O2", "-Wall", "-g", "convttf.c", "-o", "convttf"]
        + freetype_flags.stdout.split()
    )
    result = subprocess.run(cmd, cwd=ROCKBOX_TOOLS)
    if result.returncode != 0 or not CONVTTF.exists():
        die("no se pudo compilar tools/convttf")


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


def font_filename(role_name, role):
    slug = role.get("file_slug", role_name)
    return f"moonlit-{slug}-{role['px']}.fnt"


def generate_fonts(tokens):
    print("==> Generando fuentes bitmap (firmware/assets/fonts/*.fnt)")
    ensure_convttf()
    FONTS_OUT.mkdir(parents=True, exist_ok=True)

    faces = tokens["font"]["faces"]
    type_scale = tokens["type_scale"]

    for role_name, role in type_scale.items():
        if role_name == "comment":
            continue
        ttf_path = ROOT / faces[role["face"]]
        if not ttf_path.exists():
            die(f"falta {ttf_path} (rol '{role_name}', ver design-system/vendor/)")
        out_fnt = FONTS_OUT / font_filename(role_name, role)
        cmd = [
            str(CONVTTF), "-p", str(role["px"]),
            "-s", str(FONT_CHARSET_START), "-l", str(FONT_CHARSET_LIMIT),
            "-D", str(FONT_CHARSET_DEFAULT), "-c", str(role["spacing"]),
            "-o", str(out_fnt), str(ttf_path),
        ]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0 or not out_fnt.exists():
            die(f"convttf fallo para {role_name}@{role['px']}px:\n{result.stdout}\n{result.stderr}")

        header = read_rb12_header(out_fnt)
        expected_size = FONT_SIZE_EXCEPTIONS.get(role["face"], FONT_EXPECTED_SIZE)
        if header["firstchar"] != FONT_CHARSET_START:
            die(f"{out_fnt}: firstchar={header['firstchar']}, se esperaba {FONT_CHARSET_START} (D-007)")
        if header["size"] != expected_size:
            die(f"{out_fnt}: size={header['size']}, se esperaba {expected_size} (D-007/D-032)")

        print(f"   {role_name} ({role['face']} @ {role['px']}px) -> {out_fnt.name} "
              f"(firstchar={header['firstchar']}, size={header['size']}, height={header['height']})")


# WCAG 2.x contraste relativo (formula estandar, sRGB -> luminancia lineal).
def _srgb_channel(c):
    c = c / 255.0
    return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4


def relative_luminance(rgb):
    r, g, b = (_srgb_channel(c) for c in rgb)
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def contrast_ratio(rgb_a, rgb_b):
    la = relative_luminance(rgb_a)
    lb = relative_luminance(rgb_b)
    lighter, darker = max(la, lb), min(la, lb)
    return (lighter + 0.05) / (darker + 0.05)


MIN_CONTRAST = 4.5


def check_contrast(tokens):
    color = tokens["color"]
    pairs = [("on_surface", "surface"), ("on_surface_variant", "surface")]
    rows = []
    failed = []
    for scheme in SCHEMES:
        roles = color[scheme]
        for fg_role, bg_role in pairs:
            fg = hex_to_rgb(roles[fg_role])
            bg = hex_to_rgb(roles[bg_role])
            ratio = contrast_ratio(fg, bg)
            ok = ratio >= MIN_CONTRAST
            rows.append((scheme, fg_role, bg_role, ratio, ok))
            if not ok:
                failed.append((scheme, fg_role, bg_role, ratio))

    print(f"{'esquema':<6} {'texto':<18} {'fondo':<8} {'razon':>7}  ")
    for scheme, fg_role, bg_role, ratio, ok in rows:
        mark = "OK" if ok else "FAIL"
        print(f"{scheme:<6} {fg_role:<18} {bg_role:<8} {ratio:7.2f}  {mark}")

    if failed:
        detail = "\n".join(
            f"  {s}: {fg}/{bg} = {r:.2f} < {MIN_CONTRAST}" for s, fg, bg, r in failed
        )
        die(f"contraste insuficiente:\n{detail}")


# D-033: supersampleo 16x + filtro de caja (AF/design-system/generate.py:372-391)
# -- rsvg-convert ya antialiasa, pero pedirle el simbolo a 16x el tamano
# final y reducirlo con Image.BOX (promedio de cobertura de subpixeles,
# sin el ringing que LANCZOS puede meter en curvas muy chicas a 16px) da
# un canal alfa mas fiel a la forma real que rasterizar directo al
# tamano final.
ICON_SUPERSAMPLE = 16

# Mismo umbral y motivo que AF/design-system/generate.py:475 (D-008):
# una rampa antialiasada sana tiene decenas de tonos intermedios; 3 o
# menos significa que el icono se binarizo en algun paso del pipeline.
MIN_INK_TONES = 4


def _rasterize_icon_alpha(svg_path, size_px):
    hi = size_px * ICON_SUPERSAMPLE
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = Path(tmp.name)
    try:
        result = subprocess.run(
            ["rsvg-convert", "-w", str(hi), "-h", str(hi), "-o", str(tmp_path), str(svg_path)],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            die(f"rsvg-convert fallo para {svg_path} @ {size_px}px:\n{result.stderr}")
        img = Image.open(tmp_path).convert("RGBA")
    finally:
        tmp_path.unlink(missing_ok=True)
    img = img.resize((size_px, size_px), Image.BOX)
    return img.getchannel("A")


def generate_icons(tokens):
    print("==> Generando apps/metro/moonlit_icons_table.c")
    if Image is None:
        die(
            "falta el modulo Pillow -- crea el venv del design system:\n"
            "  python3 -m venv design-system/.venv && "
            "design-system/.venv/bin/pip install pillow"
        )

    icon_cfg = tokens["icon"]
    names = icon_cfg["names"]
    sizes = icon_cfg["sizes"]

    rows = []
    fail = []
    entries = []  # (name, [(size, w, h, data), ...])
    for name in names:
        svg_path = ICONS_VENDOR_DIR / f"{name}.svg"
        if not svg_path.exists():
            die(f"falta {svg_path} (ver design-system/vendor/material-symbols/)")
        per_size = []
        for size_px in sizes:
            alpha = _rasterize_icon_alpha(svg_path, size_px)
            data = alpha.tobytes()
            tones = {v for v in data if v > 0}
            ok = len(tones) >= MIN_INK_TONES
            rows.append((name, size_px, len(tones), ok))
            if not ok:
                fail.append((name, size_px, len(tones)))
            per_size.append((size_px, size_px, size_px, data))
        entries.append((name, per_size))

    print(f"{'icono':<24} {'px':>4} {'tonos':>6}  ")
    for name, size_px, n_tones, ok in rows:
        print(f"{name:<24} {size_px:>4} {n_tones:>6}  {'OK' if ok else 'FAIL'}")

    if fail:
        detail = "\n".join(f"  {n}@{s}px: {t} tono(s)" for n, s, t in fail)
        die(f"verificacion de tonos fallo -- < {MIN_INK_TONES} tonos de tinta:\n{detail}")

    lines = [
        "/* GENERATED by design-system/generate.py --icons -- do not edit by hand.",
        " *",
        " * Material Symbols Rounded (Google), Apache License 2.0 -- ver",
        " * design-system/vendor/material-symbols/LICENSE y DECISIONS.md D-008.",
        " *",
        " * 8-bit coverage masks (one byte per pixel, row-major), rasterised",
        " * with anti-aliasing (16x supersample + box filter) at each of the",
        " * three sizes. Drawn by blending a colour over whatever is behind",
        " * (metro_fb_plot_alpha via moonlit_icon_draw), so they stay",
        " * theme-neutral and never hard-code an RGB (D-033).",
        " */",
        '#include "moonlit_icons.h"',
        "",
    ]
    for name, per_size in entries:
        for size_px, w, h, data in per_size:
            arr_name = f"moonlit_icon_{name}_{size_px}_cov"
            lines.append(f"static const uint8_t {arr_name}[{w * h}] = {{")
            for y in range(h):
                row = data[y * w:(y + 1) * w]
                lines.append("    " + ", ".join(f"{v:3d}" for v in row) + ",")
            lines.append("};")
        lines.append("")

    lines.append(f"const struct moonlit_icon_mask moonlit_icons[MOONLIT_ICON_COUNT][MOONLIT_ICON_SIZE_COUNT] = {{")
    for name, per_size in entries:
        parts = []
        for size_px, w, h, _data in per_size:
            arr_name = f"moonlit_icon_{name}_{size_px}_cov"
            parts.append(f"{{ {w}, {h}, {arr_name} }}")
        lines.append(f"    /* {name} */")
        lines.append(f"    {{ {', '.join(parts)} }},")
    lines.append("};")
    lines.append("")

    ICONS_OUT.parent.mkdir(parents=True, exist_ok=True)
    ICONS_OUT.write_text("\n".join(lines))
    print(f"==> {ICONS_OUT.relative_to(ROOT.parent)} ({len(entries)} iconos x {len(sizes)} tamanos)")


# D-016/D-044: verificacion mecanica E.3 del logotipo (docs/plans/archivo/03-plan-implementacion.md:392-398),
# solo sobre el creciente a 16px -- el tamano mas chico, donde las
# cuspides del creciente son ~1px y el cuerpo corre riesgo de
# adelgazarse hasta desaparecer.
MIN_CRESCENT_COVERAGE_16 = 3
CRESCENT_COVERAGE_THRESHOLD = 200
CUSP_DUST_THRESHOLD = 60


def _rasterize_alpha(svg_path, w, h):
    hi_w, hi_h = w * ICON_SUPERSAMPLE, h * ICON_SUPERSAMPLE
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = Path(tmp.name)
    try:
        result = subprocess.run(
            ["rsvg-convert", "-w", str(hi_w), "-h", str(hi_h), "-o", str(tmp_path), str(svg_path)],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            die(f"rsvg-convert fallo para {svg_path} @ {w}x{h}:\n{result.stderr}")
        img = Image.open(tmp_path).convert("RGBA")
    finally:
        tmp_path.unlink(missing_ok=True)
    img = img.resize((w, h), Image.BOX)
    return list(img.getchannel("A").tobytes())


def _check_crescent_16(data, w, h):
    best_col, best_count = 0, -1
    for x in range(w):
        count = sum(1 for y in range(h) if data[y * w + x] > 0)
        if count > best_count:
            best_col, best_count = x, count

    strong = sum(1 for y in range(h) if data[y * w + best_col] >= CRESCENT_COVERAGE_THRESHOLD)
    coverage_ok = strong >= MIN_CRESCENT_COVERAGE_16

    dust = []
    for y in range(h):
        for x in range(w):
            v = data[y * w + x]
            if 0 < v < CUSP_DUST_THRESHOLD:
                neighbour_ok = any(
                    0 <= x + dx < w and 0 <= y + dy < h and data[(y + dy) * w + (x + dx)] >= CUSP_DUST_THRESHOLD
                    for dy in (-1, 0, 1) for dx in (-1, 0, 1) if not (dx == 0 and dy == 0)
                )
                if not neighbour_ok:
                    dust.append((x, y, v))

    return coverage_ok, strong, dust


def generate_logo(tokens):
    print("==> Generando apps/metro/moonlit_logo_table.c")
    if Image is None:
        die(
            "falta el modulo Pillow -- crea el venv del design system:\n"
            "  python3 -m venv design-system/.venv && "
            "design-system/.venv/bin/pip install pillow"
        )

    logo_cfg = tokens["logo"]
    sizes = logo_cfg["crescent_sizes"]
    ww, wh = logo_cfg["wordmark_size"]

    crescent_svg = LOGO_VENDOR_DIR / "moonlit-crescent.svg"
    wordmark_svg = LOGO_VENDOR_DIR / "moonlit-wordmark.svg"
    if not crescent_svg.exists():
        die(f"falta {crescent_svg}")
    if not wordmark_svg.exists():
        die(f"falta {wordmark_svg}")

    rows = []
    fail = []
    crescent_entries = []
    for size_px in sizes:
        data = _rasterize_alpha(crescent_svg, size_px, size_px)
        tones = {v for v in data if v > 0}
        ok = len(tones) >= MIN_INK_TONES
        rows.append(("crescent", str(size_px), len(tones), ok))
        if not ok:
            fail.append(("crescent", size_px, len(tones)))
        crescent_entries.append((size_px, data))

    wordmark_data = _rasterize_alpha(wordmark_svg, ww, wh)
    wordmark_tones = {v for v in wordmark_data if v > 0}
    wordmark_ok = len(wordmark_tones) >= MIN_INK_TONES
    rows.append(("wordmark", f"{ww}x{wh}", len(wordmark_tones), wordmark_ok))
    if not wordmark_ok:
        fail.append(("wordmark", f"{ww}x{wh}", len(wordmark_tones)))

    print(f"{'mascara':<12} {'tam':>8} {'tonos':>6}  ")
    for name, size, n_tones, ok in rows:
        print(f"{name:<12} {size:>8} {n_tones:>6}  {'tones>=4 OK' if ok else 'FAIL'}")

    if fail:
        detail = "\n".join(f"  {n}@{s}: {t} tono(s)" for n, s, t in fail)
        die(f"verificacion de tonos fallo -- menos de {MIN_INK_TONES} tonos de tinta:\n{detail}")

    data16 = dict(crescent_entries)[16]
    coverage_ok, strong, dust = _check_crescent_16(data16, 16, 16)
    print(f"cobertura 16px: {strong} px >= {CRESCENT_COVERAGE_THRESHOLD}/255 en la columna mas ancha "
          f"(minimo {MIN_CRESCENT_COVERAGE_16}) -- {'cobertura 16px OK' if coverage_ok else 'FAIL'}")
    if not coverage_ok:
        die(f"cobertura del creciente a 16px insuficiente: {strong} < {MIN_CRESCENT_COVERAGE_16}")

    cusps_ok = len(dust) == 0
    print(f"cuspides 16px: {len(dust)} pixel(es) aislado(s) < {CUSP_DUST_THRESHOLD}/255 "
          f"-- {'cuspides OK' if cusps_ok else 'FAIL'}")
    if not cusps_ok:
        die(f"cuspides con polvo aislado a 16px (x,y,valor): {dust}")

    lines = [
        "/* GENERATED by design-system/generate.py --logo -- do not edit by hand.",
        " *",
        " * Waning Crescent (D-016, D-044) -- creciente por sustraccion de dos",
        " * circulos (design-system/logo/moonlit-crescent.svg) y wordmark",
        " * \"moonlit\" en Libre Baskerville a contornos",
        " * (design-system/logo/moonlit-wordmark.svg, OFL 1.1, D-004).",
        " *",
        " * 8-bit coverage masks (one byte per pixel, row-major), rasterised",
        " * with anti-aliasing (16x supersample + box filter), verified for ink",
        " * tone count and (16px) coverage/cusp isolation before being",
        " * committed. Drawn by blending a colour over whatever is behind",
        " * (metro_fb_plot_alpha via moonlit_logo_draw_*), so they stay",
        " * theme-neutral and never hard-code an RGB.",
        " */",
        '#include "moonlit_logo.h"',
        "",
    ]
    for size_px, data in crescent_entries:
        arr_name = f"moonlit_logo_crescent_{size_px}_cov"
        lines.append(f"static const uint8_t {arr_name}[{size_px * size_px}] = {{")
        for y in range(size_px):
            row = data[y * size_px:(y + 1) * size_px]
            lines.append("    " + ", ".join(f"{v:3d}" for v in row) + ",")
        lines.append("};")
        lines.append("")

    lines.append(f"static const uint8_t moonlit_logo_wordmark_cov[{ww * wh}] = {{")
    for y in range(wh):
        row = wordmark_data[y * ww:(y + 1) * ww]
        lines.append("    " + ", ".join(f"{v:3d}" for v in row) + ",")
    lines.append("};")
    lines.append("")

    lines.append("const struct moonlit_logo_mask moonlit_logo_crescent[MOONLIT_LOGO_CRESCENT_SIZE_COUNT] = {")
    for size_px, _data in crescent_entries:
        arr_name = f"moonlit_logo_crescent_{size_px}_cov"
        lines.append(f"    {{ {size_px}, {size_px}, {arr_name} }},")
    lines.append("};")
    lines.append("")
    lines.append(f"const struct moonlit_logo_mask moonlit_logo_wordmark = {{ {ww}, {wh}, moonlit_logo_wordmark_cov }};")
    lines.append("")

    LOGO_OUT.parent.mkdir(parents=True, exist_ok=True)
    LOGO_OUT.write_text("\n".join(lines))
    print(f"==> {LOGO_OUT.relative_to(ROOT.parent)} ({len(crescent_entries)} tamanos de creciente + wordmark)")


def generate_bootlogo(tokens):
    """D-050: creciente Waning Crescent sobre `surface` del esquema night,
    tinta `on_surface` (no `primary`: el preset de acento del usuario
    aun no se ha leido cuando apps/main.c dibuja esto, y el splash de
    metro_screen_splash.c que sigue dibuja el mismo creciente con
    metro_color_fg() == on_surface -- asi el primer cuadro y el splash
    son la misma figura del mismo color, sin salto). Sin texto."""
    print("==> Generando apps/bitmaps/native/rockboxlogo.320x98x16.bmp")
    if Image is None:
        die("falta el modulo Pillow -- crea el venv del design system")

    crescent_svg = LOGO_VENDOR_DIR / "moonlit-crescent.svg"
    if not crescent_svg.exists():
        die(f"falta {crescent_svg}")

    bg = hex_to_rgb(tokens["color"]["night"]["surface"])
    ink = hex_to_rgb(tokens["color"]["night"]["on_surface"])
    size = BOOTLOGO_CRESCENT_PX
    alpha = _rasterize_alpha(crescent_svg, size, size)

    tones = {v for v in alpha if v > 0}
    ok = len(tones) >= MIN_INK_TONES
    report = [
        f"design-system/generate.py --bootlogo (D-050)",
        f"fuente: {crescent_svg.relative_to(ROOT.parent)} @ {size}px, supersampleo {ICON_SUPERSAMPLE}x + filtro de caja",
        f"salida: {BOOTLOGO_OUT.relative_to(ROOT.parent)} ({BOOTLOGO_W}x{BOOTLOGO_H}, BMP 24 bpp)",
        f"fondo: night.surface {tokens['color']['night']['surface']}  tinta: night.on_surface {tokens['color']['night']['on_surface']}",
        f"tonos de tinta: {len(tones)} (minimo {MIN_INK_TONES})  {'tones>=4 OK' if ok else 'FAIL'}",
    ]
    for line in report:
        print(line)
    if not ok:
        die(f"verificacion de tonos fallo -- {len(tones)} < {MIN_INK_TONES}")

    img = Image.new("RGB", (BOOTLOGO_W, BOOTLOGO_H), bg)
    px = img.load()
    x0 = (BOOTLOGO_W - size) // 2
    y0 = (BOOTLOGO_H - size) // 2
    for y in range(size):
        for x in range(size):
            a = alpha[y * size + x]
            if a == 0:
                continue
            px[x0 + x, y0 + y] = tuple(bg[c] + ((ink[c] - bg[c]) * a) // 255 for c in range(3))

    BOOTLOGO_OUT.parent.mkdir(parents=True, exist_ok=True)
    img.save(BOOTLOGO_OUT, format="BMP")
    BOOTLOGO_TONES_OUT.parent.mkdir(parents=True, exist_ok=True)
    BOOTLOGO_TONES_OUT.write_text("\n".join(report) + "\n")
    print(f"==> {BOOTLOGO_OUT.relative_to(ROOT.parent)}; reporte en {BOOTLOGO_TONES_OUT.relative_to(ROOT.parent)}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--header", action="store_true", help="genera apps/metro/moonlit_tokens.h")
    parser.add_argument("--contrast", action="store_true", help="verifica contraste WCAG on_surface/on_surface_variant vs surface")
    parser.add_argument("--fonts", action="store_true", help="genera firmware/assets/fonts/moonlit-*.fnt")
    parser.add_argument("--icons", action="store_true", help="genera apps/metro/moonlit_icons_table.c")
    parser.add_argument("--logo", action="store_true", help="genera apps/metro/moonlit_logo_table.c")
    parser.add_argument("--bootlogo", action="store_true", help="genera apps/bitmaps/native/rockboxlogo.320x98x16.bmp (D-050)")
    args = parser.parse_args()

    if not (args.header or args.contrast or args.fonts or args.icons or args.logo or args.bootlogo):
        parser.error("nada que hacer: pasa --header, --contrast, --fonts, --icons, --logo y/o --bootlogo")

    tokens = json.loads(TOKENS_PATH.read_text())

    if args.header:
        generate_header(tokens)
    if args.contrast:
        check_contrast(tokens)
    if args.fonts:
        generate_fonts(tokens)
    if args.icons:
        generate_icons(tokens)
    if args.logo:
        generate_logo(tokens)
    if args.bootlogo:
        generate_bootlogo(tokens)

    print("==> listo")


if __name__ == "__main__":
    main()
