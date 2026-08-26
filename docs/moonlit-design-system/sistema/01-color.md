# Color

Fuente: `design-system/tokens.json:color`. API en C: `moonlit_color(enum
moonlit_role)` (`apps/metro/moonlit_palette.h`) — nunca un literal RGB
fuera de `moonlit_palette.c`, el único includer de `moonlit_tokens.h`
dentro de `apps/metro/` (excepción documentada en `DECISIONS.md` D-035
para plugins de Rockbox, que no pueden enlazar `moonlit_palette.c`).

## Dos esquemas (D-027)

| Esquema | Ajuste (`metro_theme.h`) | Nombre en UI | Uso |
|---|---|---|---|
| `night` | `METRO_THEME_DARK` (predeterminado) | "noche" | Calma nocturna, identidad Waning Crescent |
| `dawn`  | `METRO_THEME_LIGHT` | "amanecer" | Alto contraste diurno |

## 16 roles MD3 (D-028)

`primary`, `on_primary`, `primary_container`, `on_primary_container` —
familia de acento, varían por **preset** (ver abajo) además de por
esquema — y los 12 roles de superficie, fijos por esquema:
`surface`, `surface_dim`, `surface_bright`,
`surface_container_lowest/low/(base)/high/highest`, `on_surface`,
`on_surface_variant`, `outline`, `outline_variant`.

Mapeo de los 4 tonos heredados de Metro (D-034, `metro_theme.c`):

| Función (compat) | Rol MD3 |
|---|---|
| `metro_color_bg()` | `surface` |
| `metro_color_fg()` | `on_surface` |
| `metro_color_secondary()` | `on_surface_variant` |
| `metro_color_tertiary()` | `outline` |
| `metro_color_accent()` (alias de `moonlit_color_accent()`) | `primary` |

## 4 presets de acento (D-028)

Sustituyen a los 10 acentos WP7 (`enum metro_accent`, `metro_theme.h`).
Cada uno trae sus 4 roles de la familia `primary` por esquema (MD3
"primary40 vs primary80", no el mismo hex reflejado — PC-5 cerrada en
(a)):

| Preset | Nombre ES | Nombre EN |
|---|---|---|
| `moonstone` (predeterminado) | piedra lunar | moonstone |
| `tide` | marea | tide |
| `ember` | ascua | ember |
| `moss` | musgo | moss |

## Contraste

`design-system/generate.py --contrast` verifica WCAG ≥ 4.5:1 para
`on_surface`/`surface` y `on_surface_variant`/`surface`, en los dos
esquemas, en cada regeneración de tokens.
