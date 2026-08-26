# Elevación

MD3 sin GPU: elevación = **tono**, no sombra proyectada — es justamente
lo que hace viable MD3 en un LCD sin acelerador 2D (D-011, D-012).

## Niveles (`enum moonlit_surface_level`, D-028)

`surface_container_lowest` (0, el más hundido) → `low` (1) → `(base)`
(2) → `high` (3) → `highest` (4, el más alto). Color por nivel:
`moonlit_surface(nivel, MEDGE_NONE)`.

## Borde luz/sombra (D-012)

"Waning Crescent: luz desde la izquierda". Cada nivel lleva dos tonos
de borde precalculados en `design-system/generate.py` (nunca por
cuadro): `MEDGE_LIGHT` (arriba/izquierda, un paso más claro) y
`MEDGE_SHADOW` (abajo/derecha, un paso más oscuro) —
`elevation.light_edge_delta`/`shadow_edge_delta` en `tokens.json`,
sumados por canal al color base del nivel. **Prohibido**: blur, ripple,
sombra difusa, gradiente por píxel fuera de `lcd_active()`.

Verificación mecánica: `firmware/tools/check_tones.py --edge <png>
--row Y --x0 X0 --x1 X1` — compara la luminancia relativa WCAG del
borde contra el interior de la tarjeta; falla si el borde izquierdo no
es más claro o el derecho no es más oscuro.

## `moonlit_draw_surface(x, y, w, h, nivel, radio)`

(`apps/metro/moonlit_elevation.c`) Rellena el rectángulo con el tono
base del nivel, redondea las esquinas por cobertura (antialias, misma
técnica que el anillo de `metro_widgets_draw_circle()`) hacia
`moonlit_color(MROLE_SURFACE)` — el fondo plano de pantalla que hay
detrás — y traza el borde de 1px (`MOONLIT_ELEVATION_EDGE_PX`) luz
arriba/izquierda, sombra abajo/derecha.

## Capa de estado (foco/selección)

La fila elegida de una lista o del hub (M4) se dibuja sobre una tarjeta
`surface_container_high` con un marcador de 3px en `primary` a la
izquierda — arranca en x=1, no x=0, para no tapar el propio borde de
luz de la tarjeta. Texto encima en `on_surface`.
