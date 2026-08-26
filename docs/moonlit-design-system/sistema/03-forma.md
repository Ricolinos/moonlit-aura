# Forma

Fuente: `design-system/tokens.json:shape`. Escala de radios de esquina
(px), aplicada por `moonlit_draw_surface()` (`apps/metro/moonlit_elevation.h`)
al dibujar una tarjeta de elevación.

| Nombre | px | Uso |
|---|---|---|
| `corner_none` | 0 | Barra de estado, fila de lista de borde a borde (D-011) |
| `corner_xs` | 4 | — |
| `corner_s` | 8 | Tarjetas pequeñas |
| `corner_m` | 12 | Tarjetas medianas (p. ej. panel de Marea, M8) |
| `corner_full` | 999 | Círculos/píldoras completos |

`apps/metro/moonlit_palette.c/.h` es el único includer de
`moonlit_tokens.h` dentro de `apps/metro/` (D-035) — los llamadores de
`moonlit_draw_surface()` pasan el radio como literal `int` documentado
con el nombre del token que refleja (p. ej. `0` para la fila
seleccionada de una lista, borde a borde, sin esquina flotando en el
borde de pantalla).
