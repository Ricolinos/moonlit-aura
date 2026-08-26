# Marea

Implementado en M8 (D-029, D-030, D-043). `apps/metro/moonlit_screen_marea.c`.
**Experimental** hasta la medición en hardware real de M12 (D-041, D-043).

## Layout (D-030)

Columna de portadas a la izquierda, `x ∈ [0,152)`, eje de barrido vertical
centrado en `x=76`. Tapa central 120 px en `y ∈ [70,190)` (barra de estado
20 px + centrado de la franja de 120 px dentro de los 220 px útiles). Panel
de información a la derecha, `x ∈ [160,312)` (152 px de ancho, mismo margen
de 8 px que separa la columna izquierda del panel, espejado del lado
derecho de la pantalla): título en `MFONT_HEADLINE`, artista en `MFONT_BODY`
(`on_surface_variant`), "N canciones" en `MFONT_LABEL`.

## Motor de proyección

`moonlit_flow.h` (D-041, M6): barrido por filas de pantalla, no columnas —
`moonlit_flow_source_row()` indexa directo el `.pfraw` fila-contigua de
`moonlit_art` (D-042, M7), sin transponer. `moonlit_flow_cross_scale()` da
la escala horizontal por fila; el ancho proyectado de cada fila
(`cover_disp`) ancla el blit centrado en `x=76`. Ángulo/separación lateral
(`MAREA_ITILT`/`MAREA_OFFSETX_R`/`MAREA_SLIDE_SPACING_R`) re-derivados de
`aura_musicflow.c` para 120 px — HIPÓTESIS sin retunear (D-043).

## Carátulas y monograma

`get_slot_for()` cachea hasta `MAREA_CACHE_SLOTS` (37, LRU por distancia al
índice comprometido) y **nunca decodifica un JPEG dentro de `show()`** —
solo lee el `.pfraw` ya horneado por M7. Un cache-miss cae a un relleno
liso proyectado (tapas laterales) o, exactamente en el centro, a una
tarjeta plana con la inicial del álbum en `primary` (D.5) — y encola el
álbum para que `moonlit_screen_marea_tick()` lo decodifique en la próxima
vuelta ociosa del bucle principal (mismo presupuesto de un decode por
tick que el motor de miniaturas, DD-9).

## Navegación

Rueda: `MACT_NEXT`/`MACT_PREV` mueven el índice comprometido, animado 220 ms
`METRO_EASE_OUT_EXPO` solo bajo `lcd_active()` y `animations=all` (si no,
salto directo). Antes de entrar al bucle de cuadros, `preload_range()`
lee de disco los `.pfraw` de todos los álbumes que cualquier cuadro del
scroll puede mostrar; dentro del bucle no se abre ningún archivo (un
miss cae a monograma y se repinta al salir) — D-045, cerrada en v0.1.1. `MACT_SELECT` empuja la subpágina de canciones del álbum
enfocado (la misma que Álbumes/Quickplay/un artista, `metro_screen_hub.c`).
`MACT_PLAYPAUSE` reproduce el álbum enfocado desde la pista 0. `MACT_BACK`
saca a Marea de la pila. Contexto reusado: `MCTX_LIST` (D-030), sin tocar
`metro_keymap.c`.

## Entrada

Pivote de una sola fila ("marea"), el **primero** de `music_pivots[]`
(`metro_screen_hub.c`, D-029; D-051 lo movió del final al principio en
v0.1.1: entrar a Música y pulsar SELECT abre Marea) — seleccionarlo
empuja Marea como pantalla completa, un cuarto centinela junto a Ahora suena y el visor de fotos
(`metro_main.c`).
