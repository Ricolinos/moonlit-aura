# Sistema de diseño Waning Crescent — índice

Fuente viva del lenguaje visual de moonlit.aura. Fuente de verdad de los
*valores*: `design-system/tokens.json`. Fuente de verdad de las
*decisiones* de por qué esos valores son esos: `DECISIONS.md`. Ante
cualquier discrepancia entre esta carpeta y esos dos archivos, mandan
ellos.

## Sistema (los cinco pilares MD3 adaptados al hardware)

- [`sistema/01-color.md`](sistema/01-color.md) — los 16 roles MD3, dos
  esquemas (night/dawn), 4 presets de acento.
- [`sistema/02-tipografia.md`](sistema/02-tipografia.md) — 7 roles,
  Libre Baskerville + Montserrat.
- [`sistema/03-forma.md`](sistema/03-forma.md) — escala de radios de
  esquina.
- [`sistema/04-elevacion.md`](sistema/04-elevacion.md) — superficies
  tintadas, borde luz/sombra, sin sombra proyectada.
- [`sistema/05-movimiento.md`](sistema/05-movimiento.md) — duración y
  easing, siempre bajo `lcd_active()`.

## Componentes

Se documentan a medida que cada hito los construye — ver
`docs/plan/05-plan-correctivo.md`:

- Hub, lista, ajustes, barra de estado: **M4** (implementado; sin nota
  de componente propia todavía — los cinco pilares de arriba cubren su
  vocabulario completo).
- Ahora suena, candado, USB, splash: **M5**.
- Marea (Cover Flow vertical): **M8**.
- Logotipo Waning Crescent: **M9**.

## Cómo se genera todo esto

```
design-system/.venv/bin/python3 design-system/generate.py --header    # apps/metro/moonlit_tokens.h
design-system/.venv/bin/python3 design-system/generate.py --contrast  # WCAG on_surface/on_surface_variant vs surface
design-system/.venv/bin/python3 design-system/generate.py --fonts     # firmware/assets/fonts/moonlit-*.fnt
design-system/.venv/bin/python3 design-system/generate.py --icons     # apps/metro/moonlit_icons_table.c
```

Verificación mecánica (nunca "a ojo"): `firmware/tools/check_fonts.py`,
`firmware/tools/check_tones.py` (`--region` cuenta tonos, `--edge`
verifica el borde de elevación luz/sombra).
