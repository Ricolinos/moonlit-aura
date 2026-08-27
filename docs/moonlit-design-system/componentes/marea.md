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
`moonlit_flow_source_row()` indexa directo la tapa de 120 px fila-contigua
que `moonlit_art_cache.c` deriva de la caché maestra compartida
(`/.aura/art/albums/`, D-059) al cargar el slot, sin transponer.
`moonlit_flow_cross_scale()` da
la escala horizontal por fila; el ancho proyectado de cada fila
(`cover_disp`) ancla el blit centrado en `x=76`. Ángulo/separación lateral
(`MAREA_ITILT`/`MAREA_OFFSETX_R`/`MAREA_SLIDE_SPACING_R`) re-derivados de
`aura_musicflow.c` para 120 px — HIPÓTESIS sin retunear (D-043).

## Carátulas y monograma

`get_slot_for()` cachea hasta `MAREA_CACHE_SLOTS` (37, LRU por distancia al
índice comprometido, nunca desaloja un slot visible en el destino actual,
D-057 item 5) y **nunca decodifica JPEG ni toca tagcache dentro de
`show()`/`show_carousel()`** (D-053/D-057): un cache-miss reclama el slot
en estado `MAREA_ART_PENDING` y dibuja un relleno liso proyectado (tapas
laterales) o, exactamente en el centro, una tarjeta plana con la inicial
del álbum en `primary` (D.5).

D-057 (reporte del dueño en hardware real, iPod 6G/1083 álbumes: "las
carátulas tardan en aparecer" pese a estar ya todas cacheadas) relaja
D-053 en un solo punto acotado: mientras anima, `show_carousel()` se
permite **a lo sumo una** lectura PLANA + remuestreo de la caché
maestra por cuadro (`try_frame_bounded_read()` →
`moonlit_art_derive_from_master()`), solo si la clave del álbum ya se
conocía de antes (`moonlit_art_master_path_peek()`, nunca tagcache);
una maestra ausente o un `.none` ya conocido no gastan ese cupo. El
resto de la mejora vive fuera del cuadro: `moonlit_screen_marea_tick()`
— antes una carga por vuelta ociosa del bucle principal, **solo cuando
no anima** — ahora carga varias por vuelta con presupuesto (~15 ms o 4
lecturas) y `moonlit_screen_marea_wants_ticks()` hace que
`metro_main.c` sondee a `HZ/20` mientras falten tapas en la ventana
visible, no solo mientras anima. La precarga ociosa (antes
`MAREA_PREFETCH_RADIUS` = 6 parejo) ahora sigue
`moonlit_marea_prefetch_order()` (módulo puro,
`apps/metro/test/test_marea_prefetch.c`): 10 álbumes en la dirección
del último scroll, 4 en la contraria — sigue cabiendo en los 37 slots,
sin subir `MAREA_CACHE_SLOTS`.

**D-059 — caché maestra compartida, sin decode propio de Marea salvo
cuando el constructor está ocioso.** `moonlit_art_load_for_album()`
(`moonlit_art_cache.c`) prueba, en orden: la maestra compartida
`/.aura/art/albums/<clave>.art` (leída y derivada 130→120 + esquinas
horneadas contra `surface` del tema vigente) → `LOADED`; el `.none`
compartido → `NONE` (monograma definitivo); si nada de eso existe Y el
constructor en segundo plano (`moonlit_master_art_builder.c`) está a
mitad de una pasada, el slot pasa a `MAREA_ART_WAITING` (se le avisa
con `moonlit_master_art_builder_hint_album()` para que lo priorice) en
vez de decodificar el JPEG en el hilo de UI; solo si el constructor
está ocioso decodifica aquí mismo y ESCRIBE la maestra para las tres
familias (contrato v16, nunca solo un `.pfraw` privado). Un slot
`WAITING` se ve igual que uno `PENDING` (monograma/relleno) y se
reintenta en cuanto `moonlit_master_art_builder_generation()` avanza
— nunca en cada vuelta, para no confundirlo con un slot recién
reclamado.

Un álbum sin carátula queda `MAREA_ART_MISSING` (monograma definitivo,
sin reintentos) y deja en `/.aura/art/albums/` un marcador
`<clave>.none` de 0 bytes, **compartido por las tres familias** desde
D-059 (antes, D-056, era privado bajo `moonlitcache/art/`): la
siguiente vez, `tick()` cae al monograma sin abrir la pista ni
decodificar. Ya no hay pre-pase de "preparando biblioteca" que lo
cuente — el constructor en segundo plano lo resuelve sin pantalla
(D-059 retira esa fase por completo, ver `docs/moonlit-design-system/sistema/05-movimiento.md`
y `DECISIONS.md` D-059). Capturas:
`docs/screenshots/v0.1.3-marea-none-monogram.png` (D-056),
`v0.1.3-marea-settle-3ticks.png`/`-30ticks.png` (D-057),
`v0.1.5-marea-from-master.png` (D-059, carátulas reales servidas desde
la maestra sin decode en el hilo de UI).

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
