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
índice comprometido) y **nunca toca disco dentro de `show()`/
`show_carousel()`** (D-053): un cache-miss reclama el slot en estado
`MAREA_ART_PENDING` y dibuja un relleno liso proyectado (tapas laterales)
o, exactamente en el centro, una tarjeta plana con la inicial del álbum
en `primary` (D.5). `moonlit_screen_marea_tick()` — una carga por vuelta
ociosa del bucle principal, **solo cuando no anima** — lee el `.pfraw`
horneado por M7 (clave estable D-055) o decodifica la carátula si falta;
sin pendientes, precarga el álbum más cercano sin slot dentro de
`MAREA_PREFETCH_RADIUS` (6). Un álbum sin carátula queda
`MAREA_ART_MISSING` (monograma definitivo, sin reintentos).

## Navegación

Rueda: `MACT_NEXT`/`MACT_PREV` fijan el destino (`scroll_step()`, paso
`moonlit_wheel_step()` ≤ 3) y regresan de inmediato; la posición visual
es una función del reloj — 220 ms `METRO_EASE_OUT_EXPO` desde la posición
animada **actual** (retarget) — y `metro_main.c` pide un cuadro cada
`HZ/20` mientras `moonlit_screen_marea_animating()` (D-053, modelo Music
Flow de Aura). Cada cuadro repinta solo la banda izquierda bajo
`cpu_boost()`; el panel derecho (título, artista, "N canciones" cacheado
por índice) y la cabecera se redibujan una vez, en el asentamiento. Bajo
`animations=off` o LCD dormido el destino se dibuja directo. D-045 (v0.1.1,
precarga síncrona) queda sustituida. `MACT_SELECT` empuja la subpágina de canciones del álbum
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
