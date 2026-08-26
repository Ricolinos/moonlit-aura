# Movimiento

Fuente: `design-system/tokens.json:motion`. Regla dura, sin excepción:
**toda** animación corre bajo `lcd_active()` y respeta
`metro_settings.animations` (`METRO_ANIM_OFF/MINIMAL/ALL`) — ninguna
lectura de disco dentro de un bucle de animación (regla del repo).

| Token | Valor |
|---|---|
| `transition_ms` | 220 ms |
| `ease` | `out_expo` |

`metro_transitions.c` no puede incluir `moonlit_tokens.h` (D-035, único
includer dentro de `apps/metro/` es `moonlit_palette.c`) — la duración
se repite ahí como literal documentado (`METRO_TRANSITION_MS`). Con
`HZ=100` fijo en todo Rockbox y `frame_delay=3` ticks (30 ms/cuadro),
`animations=all` usa 7 cuadros (210 ms, D-037).

**Prohibido** (D-011): easing bezier en runtime, rasterización
vectorial en runtime. El easing se resuelve siempre por tabla
(`metro_ease()`, `metro_motion.c`).

## Paquete Waning (D-052)

Cuatro piezas que sustituyen al *turnstile* heredado de Metro
(eliminados `metro_fb_draw_turnstile_layer()`, `metro_turnstile_table.c/.h`
y `tools/gen_turnstile_table.py`). Todas viven en `apps/metro/` y ninguna
lee disco dentro del bucle.

| Pieza | Qué hace | Primitivas | Costo estimado / cuadro | Puerta | Dónde aplica |
|---|---|---|---|---|---|
| **C1 Luz de canto** | PUSH/POP: la página nueva entra deslizando **desde la izquierda** (la luz viene de ahí, D-012); al volver, la saliente se retira hacia la izquierda. 7 cuadros × 3 ticks bajo `all`, 4 bajo `minimal`, `METRO_EASE_OUT_QUAD`. CONTINUUM sigue montado sobre este bucle (`all`+`full`, solo push). | `metro_fb_compose_slide(from, to, dx, seam)` + `lcd_update()` (`metro_fb_present_slide`) | 76 800 px blit (320×240) | `lcd_active()` && `animations != off` | `metro_transitions_push()`, todo nivel de la pila |
| **C3 Filo de luna** | Una línea vertical de 1 px en el color de borde-luz de `surface_container_high` (`moonlit_surface(MSURFACE_HIGH, MEDGE_LIGHT)`) sobre la costura entre página saliente y entrante; ausente en el último cuadro (ya no hay costura). | `lcd_vline` dentro de `metro_fb_compose_slide()` | 240 px | la de C1 + `tokens.json motion.seam` | mismo que C1 |
| **C2 Menguante** | FADE de 7 cuadros × 3 ticks (210 ms) con `METRO_EASE_OUT_QUAD` (antes 6 lineales). | `metro_fb_present_fade(from, to, alpha)` | 76 800 blends por píxel | `all` **y** `graphics=full`; `minimal` o `lite` caen al deslizamiento C1 | entrar/salir de Ahora suena, Marea, visor de fotos, retorno de plugin |
| **C4 Marea que sube** | Al mover la selección **una** fila, la tarjeta nueva pasa de `surface` a `surface_container_high` en 4 cuadros × 2 ticks (80 ms, `motion.selection_ms`), `METRO_EASE_OUT_QUAD`; el marcador `primary` de 3 px crece desde arriba hasta la altura completa; los bordes luz/sombra (D-012) solo en el último cuadro. Solo se repintan las dos filas afectadas con `lcd_update_rect()`. | `moonlit_draw_selection_card(y, h, alpha, marker_h, edges)` sobre `metro_fb_blend_color()`; `metro_draw_row_slot()` (listas), `draw_hub_row()` (hub) | ~18 000 px (2 bandas de 320×28; 2 × 320×52 en el hub) | `lcd_active()` && `animations != off` (**también bajo `minimal`**); nunca con `steps > 1`, ni si la ventana desplazó, ni en rejillas, "Acerca de" o listas vacías, ni con la letra de índice flotando | `metro_screen_list.c run_selection_rise()`, `metro_screen_hub.c run_selection_rise()`. Marea no cambia. |

Tokens (`design-system/tokens.json:motion`): `selection_ms: 80`,
`ease_selection: "out_quad"`, `seam: true` → `MOONLIT_MOTION_SELECTION_MS`,
`MOONLIT_MOTION_EASE_SELECTION`, `MOONLIT_MOTION_SEAM` en
`moonlit_tokens.h`. Como con `METRO_TRANSITION_MS`, los módulos que no
pueden incluir ese header (D-035) citan el literal documentado:
`METRO_SELECTION_FRAMES 4` × `METRO_SELECTION_FRAME_TICKS 2` en
`metro_transitions.h` (patrón D-037).

### Degradación

| | `graphics=full` | `graphics=lite` |
|---|---|---|
| `animations=all` | C1 7 cuadros + C3 + CONTINUUM; C2 fade real; C4 | C1 7 cuadros + C3 (sin CONTINUUM); C2 → deslizamiento C1; C4 |
| `animations=minimal` | C1 4 cuadros + C3; C2 → deslizamiento C1; C4 | C1 4 cuadros + C3; C2 → deslizamiento C1; C4 |
| `animations=off` | nada: `redraw_current()` directo, selección salta | nada |

FEATHER (cascada de filas tras PUSH) sigue siendo solo `all`, como antes.

### Medición

**No existe medición real de ninguna pieza**: la checklist H de hardware
(32/32 puntos) sigue sin responder. Los costos de la tabla son
conteos de píxeles, no tiempos. M12 mide en el iPod `present_slide`,
`present_fade` y C4 con las trazas `METRO_TRACE` por cuadro
(`metro_transitions_trace()`: "`<nombre> frame i/n at +t ticks`");
criterio: ningún cuadro por encima de su `frame_delay`.

Capturas del simulador (`sim_shot.sh`, ticks de asentamiento 0 tras el
último botón): `docs/screenshots/v0.1.1-motion-push-mid.png`
(`WAIT,SCROLL_FWD,SCROLL_FWD,SCROLL_FWD,SELECT`: Ajustes entra desde la
izquierda, filo de luz en x=83), `v0.1.1-motion-fade-mid.png`
(`WAIT,SELECT,WAIT,SELECT`: hub → Música → Marea a medio fundido) y
`v0.1.1-motion-select-mid.png` (`WAIT,SCROLL_FWD` en el hub: cuadro 3/4,
marcador de 48/52 px, tono intermedio, sin bordes). El cuadro intermedio
de C4 no es determinista — 80 ms contra un sondeo de 2 ticks del hilo
del simulador — así que se verificó mecánicamente con PIL que la
captura guardada es intermedia (≈ 2 de cada 3 corridas lo son).
