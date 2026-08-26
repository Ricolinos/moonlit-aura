# USB

`apps/metro/metro_screen_usb.c`. Implementado en M5 (D-040).

Pantalla dibujada solo con lo embebido en el binario — sin fuente ni
disco, porque `gui_usb_screen_run()` llama `font_disable_all()` en
cuanto el host toma el disco. El glifo "sync" de Fluent System Icons
(M-089, `metro_glyphs_table.c`, 8-bit anti-aliased pero fuera del
pipeline de `moonlit_icons.h`) se sustituye por el ícono `usb` de
Material Symbols a 40 px (`moonlit_icon_draw(MOONLIT_ICON_USB,
MOONLIT_ICON_SIZE_40, ...)`) — mismas máscaras compiladas que el resto
de la app, cero lecturas de disco. `metro_glyphs_table.c`,
`metro_glyphs.h` y `metro_widgets_draw_glyph()` se retiran por
completo: ningún consumidor les sobrevive.

El wordmark de texto (D-026) y el barrido de puntos WP7 no cambian —
el logotipo Waning Crescent llega en M9.

Splash (`metro_screen_splash.c`) no cambió de código en M5: ya usaba
`MFONT_DISPLAY` y los roles MD3 desde M2/M4. Se beneficia del cambio
de pista de `metro_draw_progress()` (D-039, ver `componentes/ahora-suena.md`).
