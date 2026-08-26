# Candado

`apps/metro/metro_screen_lock.c`. Ya resolvía sus colores por rol MD3
desde M2 (fuentes) y M4 (paleta) a través de los mismos alias que el
resto de la app (`metro_color_fg()`/`_accent()`/`_secondary()`/
`_tertiary()`, `moonlit_fonts.h`) — M5 no le cambió una sola línea de
código, solo lo verificó (`docs/screenshots/M5-lock.png`).

Título en `MFONT_DISPLAY`, casillas del PIN en `MFONT_TITLE`, pista en
`MFONT_LABEL`. Casilla enfocada con borde en `primary`; ya confirmadas,
con un punto sólido en `on_surface`. Sin RGB fuera de tokens.
