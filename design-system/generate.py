#!/usr/bin/env python3
"""Pipeline determinista del sistema de diseno moonlit.aura (Waning Crescent).

Lee tokens.json (unica fuente de verdad, D-010) y produce:

  - apps/metro/moonlit_tokens.h  Header C con los 16 roles MD3 de color
                                  (D-028) por esquema (night/dawn, D-027),
                                  presets de acento, espaciado, forma,
                                  elevacion y movimiento.

Este hito (M1) solo implementa --header y --contrast. Fuentes (--fonts),
iconos (--icons) y logotipo (--logo) llegan en M2/M3/M9 como subcomandos
nuevos de este mismo archivo -- no se porta nada del generador de
Aura-Firmware mas alla de la forma de tokens.json y el patron
dict-plano -> #define de generate_header()/generate_aura_ds_defines()
(AF/design-system/generate.py:124,197-262).
"""
import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TOKENS_PATH = ROOT / "tokens.json"
HEADER_OUT = ROOT.parent / "firmware" / "rockbox" / "apps" / "metro" / "moonlit_tokens.h"

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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--header", action="store_true", help="genera apps/metro/moonlit_tokens.h")
    parser.add_argument("--contrast", action="store_true", help="verifica contraste WCAG on_surface/on_surface_variant vs surface")
    args = parser.parse_args()

    if not args.header and not args.contrast:
        parser.error("nada que hacer: pasa --header y/o --contrast")

    tokens = json.loads(TOKENS_PATH.read_text())

    if args.header:
        generate_header(tokens)
    if args.contrast:
        check_contrast(tokens)

    print("==> listo")


if __name__ == "__main__":
    main()
