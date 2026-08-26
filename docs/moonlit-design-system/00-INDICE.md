# Sistema de diseño Waning Crescent — índice

Fuente viva del lenguaje visual de moonlit.aura. Fuente de verdad de los
*valores*: `design-system/tokens.json`. Fuente de verdad de las
*decisiones* de por qué esos valores son esos: `DECISIONS.md`. Ante
cualquier discrepancia entre esta carpeta y esos dos archivos, mandan
ellos.

## Sistema (los cinco pilares MD3 adaptados al hardware)

- [`docs/moonlit-design-system/sistema/01-color.md`](sistema/01-color.md) — los 16 roles MD3, dos
  esquemas (night/dawn), 4 presets de acento.
- [`docs/moonlit-design-system/sistema/02-tipografia.md`](sistema/02-tipografia.md) — 7 roles,
  Libre Baskerville + Montserrat.
- [`docs/moonlit-design-system/sistema/03-forma.md`](sistema/03-forma.md) — escala de radios de
  esquina.
- [`docs/moonlit-design-system/sistema/04-elevacion.md`](sistema/04-elevacion.md) — superficies
  tintadas, borde luz/sombra, sin sombra proyectada.
- [`docs/moonlit-design-system/sistema/05-movimiento.md`](sistema/05-movimiento.md) — duración y
  easing, siempre bajo `lcd_active()`.

## Componentes

Se documentan a medida que cada hito los construye — ver
`docs/plan/05-plan-correctivo.md`:

- Hub, lista, ajustes, barra de estado: **M4** (implementado; sin nota
  de componente propia todavía — los cinco pilares de arriba cubren su
  vocabulario completo).
- Ahora suena, candado, USB, splash: **M5** (implementado) —
  [`docs/moonlit-design-system/componentes/ahora-suena.md`](componentes/ahora-suena.md),
  [`docs/moonlit-design-system/componentes/candado.md`](componentes/candado.md),
  [`docs/moonlit-design-system/componentes/usb.md`](componentes/usb.md) (splash documentado ahí
  mismo, sin cambio de código propio).
- Marea (Cover Flow vertical): **M8** (implementado, experimental hasta
  M12; desde v0.1.1 es el primer pivote de Música, D-051, y precarga
  las tapas visibles antes de cada scroll, D-045 cerrada) —
  [`docs/moonlit-design-system/componentes/marea.md`](componentes/marea.md).
- "Preparando biblioteca" (**v0.1.1**, D-049): pantalla bloqueante e
  interrumpible entre el hub y Música — creciente 64 px, título
  `MFONT_HEADLINE`, fase en `MFONT_BODY`, barra 120×2 y contador
  `MFONT_LABEL`; sin nota propia, especificación en `DECISIONS.md`
  D-049 y `firmware/rockbox/apps/metro/moonlit_screen_library.c`.
- Logotipo Waning Crescent: **M9** (implementado) — sin nota de
  componente propia: la especificación vectorial vive en `DECISIONS.md`
  D-016/D-044 y el detalle de integración por pantalla en
  [`docs/moonlit-design-system/componentes/usb.md`](componentes/usb.md) (pantalla USB) y las notas
  de `firmware/rockbox/apps/metro/metro_screen_splash.c`,
  `firmware/rockbox/apps/metro/metro_screen_list.c` y
  `firmware/rockbox/apps/metro/metro_screen_hub.c` citadas ahí.

## Cómo se genera todo esto

```
design-system/.venv/bin/python3 design-system/generate.py --header    # apps/metro/moonlit_tokens.h
design-system/.venv/bin/python3 design-system/generate.py --contrast  # WCAG on_surface/on_surface_variant vs surface
design-system/.venv/bin/python3 design-system/generate.py --fonts     # firmware/assets/fonts/moonlit-*.fnt
design-system/.venv/bin/python3 design-system/generate.py --icons     # apps/metro/moonlit_icons_table.c
design-system/.venv/bin/python3 design-system/generate.py --logo      # apps/metro/moonlit_logo_table.c
```

Verificación mecánica (nunca "a ojo"): `firmware/tools/check_fonts.py`,
`firmware/tools/check_tones.py` (`--region` cuenta tonos, `--edge`
verifica el borde de elevación luz/sombra).
