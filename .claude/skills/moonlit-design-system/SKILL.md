---
name: moonlit-design-system
description: "Consulta esta skill antes de crear, modificar o revisar cualquier pantalla, animación, ícono, transición, fuente o componente visual del firmware moonlit.aura. Cubre el lenguaje visual Waning Crescent (color nocturno, luz desde la izquierda, elevación por tono, tipografía Libre Baskerville + Montserrat, subconjunto Material sin GPU), los tokens de design-system/tokens.json, el Cover Flow vertical Marea y el logotipo. Dispárala para cualquier tarea de UI/UX aunque el usuario no diga la palabra 'diseño' — por ejemplo al tocar apps/metro/metro_screen_*.c, metro_draw.c, metro_transitions.c, design-system/ o firmware/assets/fonts/."
---

# Sistema de diseño Waning Crescent — guía de consulta

Fuente viva y completa: [`docs/moonlit-design-system/00-INDICE.md`](/docs/moonlit-design-system/00-INDICE.md)
(sistema + componentes, con rutas y referencias `D-NNN` exactas). Fuente
de los *valores*: `design-system/tokens.json`. Fuente de las *decisiones*
(por qué esos valores): `DECISIONS.md`. Ante discrepancia entre esta
skill, el índice y esos dos archivos, mandan `DECISIONS.md` y
`design-system/tokens.json`, en ese orden.

Antes de escribir o modificar cualquier código que afecte a la interfaz:
1. Lee la nota de `docs/moonlit-design-system/` que cubra la pieza que
   vas a tocar (tabla de abajo).
2. Si necesitas un color, tamaño, radio o duración nuevo: se agrega a
   `design-system/tokens.json` y se regenera con `design-system/generate.py`
   — nunca un literal a mano.
3. Si el cambio no encaja en ninguna regla de abajo: para y cierra una
   decisión nueva en `DECISIONS.md` (D-NNN) antes de codificar.

## Los cinco pilares (MD3 adaptado a S5L8702, sin GPU/FPU)

| Pilar | Nota | Resumen |
|---|---|---|
| Color | [`docs/moonlit-design-system/sistema/01-color.md`](/docs/moonlit-design-system/sistema/01-color.md) | 16 roles MD3 × 2 esquemas (`night` predeterminado, `dawn`) × 4 presets de acento (`moonstone`, `tide`, `ember`, `moss`). API: `moonlit_color(rol)`. |
| Tipografía | [`docs/moonlit-design-system/sistema/02-tipografia.md`](/docs/moonlit-design-system/sistema/02-tipografia.md) | 7 roles, Libre Baskerville (títulos) + Montserrat estática (texto), ninguno < 18 px. API: `metro_font_id(rol)`. |
| Forma | [`docs/moonlit-design-system/sistema/03-forma.md`](/docs/moonlit-design-system/sistema/03-forma.md) | Escala de radios `corner_none/xs/s/m/full`. |
| Elevación | [`docs/moonlit-design-system/sistema/04-elevacion.md`](/docs/moonlit-design-system/sistema/04-elevacion.md) | Tono (`surface_container_*`), nunca sombra proyectada; borde de 1px luz arriba/izquierda, sombra abajo/derecha ("Waning Crescent"). |
| Movimiento | [`docs/moonlit-design-system/sistema/05-movimiento.md`](/docs/moonlit-design-system/sistema/05-movimiento.md) | 220 ms, `out_expo`, siempre bajo `lcd_active()` + `metro_settings.animations`. |

## Componentes documentados

- Ahora suena — [`docs/moonlit-design-system/componentes/ahora-suena.md`](/docs/moonlit-design-system/componentes/ahora-suena.md)
- Candado — [`docs/moonlit-design-system/componentes/candado.md`](/docs/moonlit-design-system/componentes/candado.md)
- USB (splash incluido) — [`docs/moonlit-design-system/componentes/usb.md`](/docs/moonlit-design-system/componentes/usb.md)
- Marea (Cover Flow vertical) — [`docs/moonlit-design-system/componentes/marea.md`](/docs/moonlit-design-system/componentes/marea.md)
- Hub/lista/ajustes/barra de estado: cubiertos por los cinco pilares de
  arriba, sin nota propia (así lo deja el índice).
- Logotipo Waning Crescent: sin nota propia — especificación en
  `DECISIONS.md` D-016/D-044.

## Iconos y logo

Material Symbols Rounded (Apache 2.0) e íconos propios, compilados en
tabla C (`firmware/rockbox/apps/metro/moonlit_icons_table.c`,
`firmware/rockbox/apps/metro/moonlit_logo_table.c`) — **nunca** leídos
de disco en runtime. Generados por `design-system/generate.py --icons`/`--logo`
(SVG → `rsvg-convert` → supersampleo 16× + filtro de caja → máscara de
cobertura de 8 bits), verificación mecánica `MIN_INK_TONES ≥ 4` que
rompe el build si falla (D-008, D-016). Nunca a ojo: usa
`firmware/tools/check_tones.py`.

## Reglas duras (detalle y cita exacta en `DECISIONS.md`)

- Cero literales RGB en `apps/metro/`; solo `firmware/rockbox/apps/metro/moonlit_palette.c`
  incluye `firmware/rockbox/apps/metro/moonlit_tokens.h` (D-010, D-035).
- Elevación = dos tonos por nivel, luz izquierda/superior, sombra
  derecha/inferior, precalculados en tokens (D-012). Sin gradientes por
  píxel fuera de `lcd_active()`.
- Prohibido: blur, ripple, sombras difusas, easing bezier en runtime,
  rasterización vectorial en runtime (D-011).
- Tipografía: títulos Libre Baskerville, texto Montserrat estática;
  ningún rol < 18 px; 7 roles (D-004, D-005, D-007).
- Iconos y logo: tabla C generada, ≥ 4 tonos verificados
  mecánicamente, jamás "a ojo" (D-008, D-016).
- Fondo del reproductor: plano tonal, nunca la portada (D-013).
- Marea: vertical, sin reflejo, sin morphs, monograma sin portada;
  experimental hasta medir en hardware real (M12) (D-014, D-043).
- Logotipo: sustracción de dos círculos, acento dinámico, wordmark solo
  ≥ 64 px (D-016).

## Verificación mecánica (nunca visual)

```
design-system/.venv/bin/python3 design-system/generate.py --header|--fonts|--icons|--logo|--contrast
firmware/tools/check_fonts.py <fnt|--capheight png>
firmware/tools/check_tones.py <png> [--region x,y,w,h] [--edge]
```
