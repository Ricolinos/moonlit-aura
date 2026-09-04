# Tipografía

Fuente: `design-system/tokens.json:type_scale`. Dos familias vendoreadas
en `design-system/vendor/<familia>/` con su `OFL.txt` (D-004): **Libre
Baskerville** (títulos) y **Montserrat** estática (texto — nunca la
variable, D-004). API en C: `enum metro_font_role` +
`metro_font_id(rol)` (`apps/metro/moonlit_fonts.h`).

## 7 roles (D-005; ningún rol < 18px)

| Rol | Familia | px | Archivo `.fnt` (D-031) |
|---|---|---|---|
| `MFONT_DISPLAY` | Libre Baskerville Regular | 40 | `moonlit-display-40.fnt` |
| `MFONT_TITLE` | Libre Baskerville Regular | 28 | `moonlit-title-28.fnt` |
| `MFONT_HEADLINE` | Libre Baskerville Regular | 22 | `moonlit-headline-22.fnt` |
| `MFONT_LIST` | Montserrat Regular | 20 | `moonlit-list-20.fnt` |
| `MFONT_LIST_SEL` | Montserrat SemiBold | 20 | `moonlit-listsel-20.fnt` |
| `MFONT_BODY` | Montserrat Regular | 18 | `moonlit-body-18.fnt` |
| `MFONT_LABEL` | Montserrat Medium | 18 | `moonlit-label-18.fnt` |

`MFONT_LIST`/`MFONT_LIST_SEL` equivalen a "body-large" en el vocabulario
MD3; el nombre se conserva por los 19 sitios de llamada que ya existían
en Metro.

## Generación

`convttf` con rango **decimal** 32–383 (nunca `0x…`, D-007) — ver
`design-system/generate.py --fonts` y `firmware/tools/check_fonts.py`
(verifica `firstchar`/`size` de la cabecera RB12, y con `--capheight`
mide en píxeles la altura real de cada rol sobre una captura del
especímen, `metro_screen_specimen.c`).

## Retiro de `MFONT_CAPTION` (M4, D-038)

El alias temporal de M2 (`MFONT_CAPTION` → `MFONT_BODY`) se retiró: los
22 sitios de llamada se reanotaron con su rol MD3 real —
`MFONT_LABEL` para header/subtítulos/valores/tiempos, `MFONT_BODY` para
el único caso de una oración completa (el mensaje de lista vacía).

## Puntuación tipográfica y carácter por omisión (D-066)

Los siete `.fnt` cubren el rango decimal **32–383** (D-007). Ampliarlo
para que entraran las comillas curvas, los guiones largos y los puntos
suspensivos se **midió y se descartó**: RB12 es un rango denso (una
entrada de offset + una de ancho por código, exista o no el glifo), así
que llegar a 8482 cuesta +1 010 578 B en disco y +286 998 B de tablas en
RAM con los siete roles cargados, contra un presupuesto de 40 KB.

En su lugar, `apps/metro/moonlit_translit.c` transcribe a su equivalente
ASCII los 24 codepoints que sí lo tienen (espacio duro, guiones
U+2010–U+2015, comillas simples y dobles tipográficas, viñetas, puntos
suspensivos, primas, angulares, ™). Se aplica dentro de
`metro_draw_text()`/`metro_draw_text_clipped()`, el único sitio que hace
falta porque M-051 obliga a que todo el texto pase por ahí.

Lo que no tiene equivalente honesto (♪ ★ ♥, CJK, emoji) **no se
inventa**: cae en el `defaultchar`, que es **`·` (U+00B7)** y no `?` — un
punto medio no parece un error de lectura.

Para medir un ancho que decida geometría, usa
`metro_draw_text_width()`, que translitera antes de medir;
`lcd_getstringsize()` sobre la cadena original da otro número.

Verificación mecánica: `firmware/tools/check_fonts.py --coverage` compara
la tabla de glifos de cada `.fnt` con las cadenas de `metro_lang.c`, una
lista curada de codepoints de metadatos y la propia tabla de
transliteración. **Falla** si la UI tiene un hueco.

## Marquesina (D-067)

El texto que no cabe en su banda se desplaza: **2 000 ms quieto, 5 000 ms
de barrido lineal de derecha a izquierda, 24 px de hueco entre copias, en
bucle** (tokens `motion.marquee_*`). Se dibujan dos copias para que el
bucle no tenga costura. Solo desplaza el texto con foco —la fila
seleccionada, el rótulo del tile seleccionado, las tres líneas de "Ahora
suena", el título del panel de Marea— y solo bajo `lcd_active()` con
`animations != off`; apagadas, se corta a la derecha como siempre. API:
`moonlit_marquee_draw()` (`apps/metro/moonlit_marquee.h`); el reloj del
ciclo es puro y host-testable (`moonlit_marquee_cycle.c`).
