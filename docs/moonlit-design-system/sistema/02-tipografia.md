# Tipografía

Fuente: `design-system/tokens.json:type_scale`. Dos familias vendoreadas
en `design-system/vendor/<familia>/` con su `OFL.txt` (D-004): **Libre
Baskerville** (títulos) y **Montserrat** estática (texto — nunca la
variable, D-004). API en C: `enum metro_font_role` +
`metro_font_id(rol)` (`apps/metro/moonlit_fonts.h`).

## 7 roles (D-005; ningún rol < 18px)

| Rol | Familia | px | Archivo `.fnt` (D-031) |
|---|---|---|---|
| `MFONT_DISPLAY` | Libre Baskerville Regular | 40 | `moonlit-display-40.fnt` |
| `MFONT_TITLE` | Libre Baskerville Regular | 28 | `moonlit-title-28.fnt` |
| `MFONT_HEADLINE` | Libre Baskerville Regular | 22 | `moonlit-headline-22.fnt` |
| `MFONT_LIST` | Montserrat Regular | 20 | `moonlit-list-20.fnt` |
| `MFONT_LIST_SEL` | Montserrat SemiBold | 20 | `moonlit-listsel-20.fnt` |
| `MFONT_BODY` | Montserrat Regular | 18 | `moonlit-body-18.fnt` |
| `MFONT_LABEL` | Montserrat Medium | 18 | `moonlit-label-18.fnt` |

`MFONT_LIST`/`MFONT_LIST_SEL` equivalen a "body-large" en el vocabulario
MD3; el nombre se conserva por los 19 sitios de llamada que ya existían
en Metro.

## Generación

`convttf` con rango **decimal** 32–383 (nunca `0x…`, D-007) — ver
`design-system/generate.py --fonts` y `firmware/tools/check_fonts.py`
(verifica `firstchar`/`size` de la cabecera RB12, y con `--capheight`
mide en píxeles la altura real de cada rol sobre una captura del
especímen, `metro_screen_specimen.c`).

## Retiro de `MFONT_CAPTION` (M4, D-038)

El alias temporal de M2 (`MFONT_CAPTION` → `MFONT_BODY`) se retiró: los
22 sitios de llamada se reanotaron con su rol MD3 real —
`MFONT_LABEL` para header/subtítulos/valores/tiempos, `MFONT_BODY` para
el único caso de una oración completa (el mensaje de lista vacía).
