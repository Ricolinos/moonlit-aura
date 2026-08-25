---
name: moonlit-design-system
description: "Consulta esta skill antes de crear, modificar o revisar cualquier pantalla, animación, ícono, transición, fuente o componente visual del firmware moonlit.aura. Cubre el lenguaje visual Waning Crescent (color nocturno, luz desde la izquierda, elevación por tono, tipografía Libre Baskerville + Montserrat, subconjunto Material sin GPU), los tokens de design-system/tokens.json, el Cover Flow vertical Marea y el logotipo. Dispárala para cualquier tarea de UI/UX aunque el usuario no diga la palabra 'diseño' — por ejemplo al tocar apps/metro/metro_screen_*.c, metro_draw.c, metro_transitions.c, design-system/ o firmware/assets/fonts/."
---

# Sistema de diseño Waning Crescent — guía de consulta

**Esqueleto (hito H0).** El cuerpo de esta skill se redacta en H2–H3 a
partir de las secciones B–E de `docs/plan/03-plan-implementacion.md`.
Hasta entonces, la fuente de verdad es ese plan más `DECISIONS.md`.

Antes de escribir o modificar cualquier código que afecte a la interfaz:

1. **`docs/moonlit-design-system/00-INDICE.md` — fuente viva** (se crea
   en H2–H3): `sistema/{01-color,02-tipografia,03-geometria,04-movimiento,05-elevacion}.md`
   y `componentes/{lista,hub,ahora-suena,marea,acerca-de}.md`. Copias de
   referencia en `reference/` junto a este archivo.
2. **`design-system/tokens.json`** — único origen de color, escala
   tipográfica, espaciado, radios, elevación e iconos (D-010). Genera
   `apps/metro/moonlit_tokens.h` vía `design-system/generate.py`.
3. **`DECISIONS.md` D-004…D-016** — tipografía, iconos, lenguaje visual,
   Marea, logotipo. Ante conflicto con cualquier documento, manda
   `DECISIONS.md`.

## Reglas duras (resumen; detalle en el plan §B–§E)

- Paleta nocturna única ("night"); sin literal RGB en C (D-010).
- Elevación = dos tonos por nivel, luz izquierda/superior, sombra
  derecha/inferior, precalculados en tokens (D-012). Sin gradientes por
  píxel fuera de `lcd_active()`.
- Prohibido: blur, ripple, sombras difusas, easing bezier en runtime,
  rasterización vectorial en runtime (D-011).
- Tipografía: títulos Libre Baskerville, texto Montserrat estática;
  ningún rol < 18 px; ≤ 12 roles (D-004, D-005, D-007).
- Iconos Material Symbols compilados en tabla C con máscara de 8 bits;
  ≥ 4 tonos por ícono verificados mecánicamente (D-008).
- Fondo del reproductor: plano tonal, nunca la portada (D-013).
- Marea: vertical, sin reflejo, sin morphs, monograma sin portada;
  experimental hasta medir en hardware (D-014).
- Logotipo Waning Crescent: sustracción de dos círculos, acento
  dinámico; wordmark solo ≥ 64 px (D-016).
