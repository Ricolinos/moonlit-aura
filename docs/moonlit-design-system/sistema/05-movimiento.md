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
