# Ahora suena

Implementado en M5 (D-013, D-039). `apps/metro/metro_screen_nowplaying.c`.

## Fondo (D-013)

Superficie plana (`surface`), sin decodificar ni promediar la
carátula ni la foto del artista — costo por cuadro cero. Sustituye al
30% de la carátula mezclada sobre el fondo (F12/R4-FA-7, retirado).
`metro_albumart_load_background()`/`_file()` (`metro_albumart.c`)
quedan sin llamador aquí; M11 decide si se retiran del todo.

## Carátula

136×136 px (`METRO_ALBUMART_SIZE`), con una tarjeta de elevación tonal
detrás (`moonlit_draw_surface(..., MSURFACE_BASE, corner_s)`, D-012).
Con carátula real, el bitmap cuadrado tapa la tarjeta por completo — no
existe una primitiva de blit con esquinas redondeadas en `metro_fb.c`,
así que el redondeo solo se ve en el respaldo sin carátula: inicial del
álbum/título en `primary` sobre la tarjeta, mismo lenguaje que el
monograma de Marea (M8).

## Jerarquía tipográfica (D-039)

El título de la pista es la línea fuerte (`MFONT_TITLE`, `on_surface`);
el artista pasa a texto de cuerpo (`MFONT_BODY`, `on_surface_variant`).
Invierte a propósito el orden WP7/Zune original de M-083 ("la línea
fuerte es quién"): MD3 encabeza con QUÉ suena. El álbum no cambia
(`MFONT_LIST`/secundario).

## Controles e íconos

Fila de estado (favorito/aleatorio/repetir) y fila de transporte
(anterior/reproducir/siguiente) sobre `moonlit_icon_draw()` vía
`metro_widgets_draw_icon()`/`_draw_icon_in_circle()` — sin cambio de M3.

## Barra de progreso (D-039)

`metro_draw_progress()` (compartida con el splash): pista en
`surface_container_highest`, relleno en `primary`. Antes usaba
`outline` (tono de contorno, no de superficie).
