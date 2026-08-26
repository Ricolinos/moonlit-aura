# moonlit.aura — Auditoría de brecha (fase 4)

**Fecha:** 2026-08-25 · **Estado:** REGISTRO (solo lectura; no contiene trabajo pendiente). Compara `moonlit-aura/` contra el objetivo original, `00-decisiones-moonlit.md`, `02-investigacion.md` y `03-plan-implementacion.md`. El plan que corrige esta brecha es `05-plan-correctivo.md`.

**Ojo con el nombre:** el repo se llama `moonlit-aura` (guion); "moonlit.aura" es el nombre visible del producto (D-001).

Convención: rutas sin prefijo = `moonlit-aura/` (idénticas a Metro-Aura en todo lo no tocado desde el fork); `AF/` = `Aura-Firmware/`; `apps/metro/` abrevia `firmware/rockbox/apps/metro/`. CONCLUSIÓN = verificado con ruta:línea. HIPÓTESIS = no verificado.

---

## I.1 Qué hitos del plan 03 se ejecutaron

`git log --oneline moonlit-fork-base..HEAD` = 7 commits, todos del 2026-08-25:

| Hito plan 03 | Estado | Commits | Evidencia |
|---|---|---|---|
| H0 fork + identidad documental | **HECHO** | `3d050d1a` chore: fork from Metro-Aura 2f1bd28a · `37bef992` docs: DECISIONS D-001..D-022, CLAUDE.md, skill skeleton | tag `moonlit-fork-base` = `2f1bd28a`; `DECISIONS.md` D-001…D-022; `.claude/skills/moonlit-design-system/SKILL.md` (41 líneas, esqueleto) |
| H1 identidad runtime + contrato | **HECHO** | `c4130f2b` feat: firmware_family moonlit, moonlitcache, dormant tree (D-001) · `c8b18720` docs: CONTRATO-moonlit-studio v1 · `4511b868` feat: about credits + THIRD-PARTY-NOTICES (D-002) · `10f7dbc3` docs: evidencia H1 · `49973dc1` feat: wordmark provisional (D-026) | `apps/metro/metro_settings.c:131-132,219,250`; `metro_screen_about.c:29`; `firmware/tools/package_dist.sh:181-211`; `docs/screenshots/H1-*.png` |
| H2 design-system (tokens, fuentes, iconos) | **NO EJECUTADO** | — | no existe `design-system/` (find en todo el repo) |
| H3 paleta, elevación, pantallas base | **NO EJECUTADO** | — | `apps/metro/metro_palette.h` intacto (paleta WP7: `:33-56`) |
| H4 Ahora suena | **NO EJECUTADO** | — | `metro_screen_nowplaying.c` sin cambios desde la base |
| H5 logotipo Waning Crescent | **NO EJECUTADO** (sustituido por wordmark de texto provisional D-026) | `49973dc1` | `firmware/tools/gen_logo.py:25` sigue siendo el generador de Metro (M-092) con el string cambiado |
| H6 Marea | **NO EJECUTADO** | — | solo 4 strings sin consumidor: `metro_lang.h:181-182`, `metro_lang.c:179-180,336-337` |
| H7 hardware + empaquetado | **NO EJECUTADO** | — | `firmware/dist/` solo tiene `README.md`; `build-ipod6g-boot/` no existe |

Balance de código: `git diff --stat moonlit-fork-base..HEAD` = 35 archivos (+4660/−3344), de los cuales **9 `.c/.h`**,
todos cambios de strings/identidad (`metro_lang.c/.h`, `metro_settings.c/.h`, `metro_screen_about.c`, `metro_screen_splash.c`,
`metro_screen_usb.c`, `metro_thumbs.c` [solo comentario], `mpegplayer.c:113` [include relativo, D-025]). **Ningún archivo de código nuevo.**
**Ningún símbolo `moonlit_*`** en C (solo cadenas de datos: `"firmware_family: moonlit"`, `"%s/moonlitcache/%s"`, `"/.firmware-moonlit"`).

## I.2 Los tres componentes obligatorios — existen o no

| Componente | ¿Existe? | Evidencia |
|---|---|---|
| Archivo de tokens de diseño propio (`design-system/tokens.json`, `moonlit_tokens.h`, `moonlit_palette.h`) | **NO** | `find . -name 'tokens.json' -o -name 'moonlit_*'` vacío (excl. `.git`). Únicos `*tokens*` son ajenos: `apps/gui/skin_engine/skin_tokens.c` (Rockbox) y `flipit_tokens.h` en build dirs (plugin). Lo que hay: `apps/metro/metro_palette.h` (58 líneas, 18 `#define` WP7: `METRO_DARK_BG 000000`… `METRO_ACCENT_MAGENTA FF0097`) |
| Fuente `.fnt` nueva (Libre Baskerville / Montserrat) | **NO** | `git diff --stat moonlit-fork-base..HEAD -- '*.fnt' '*.ttf'` vacío. `firmware/assets/fonts/` = las 5 Selawik heredadas (`metro-display-48` 189 814 B, `metro-title-28` 81 982 B, `metro-list-20` 59 174 B, `metro-listsel-20` 59 692 B, `metro-caption-14` 40 116 B). `firmware/assets/fonts-src/` = Selawik-{Light,Regular,Semibold}.ttf + `LICENSE.txt` (OFL de Selawik). Ningún `OFL.txt` de Libre Baskerville/Montserrat |
| Módulo de flujo vertical de portadas | **NO** | `find firmware/rockbox/apps -name '*flow*' -o -name '*marea*'` → solo `plugins/pictureflow.c` (Rockbox stock). No hay `moonlit_flow.*`, `moonlit_art.*`, `moonlit_wheel.*`, `moonlit_screen_marea.c` |

Además: `docs/moonlit-design-system/` **no existe**; `.claude/` contiene solo el `SKILL.md` esqueleto (`SKILL.md:8-10`: "el cuerpo se redacta en H2–H3").

## I.3 Por qué el resultado fue un clon renombrado — diagnóstico

**Causa principal: (b) los hitos H2+ nunca se ejecutaron.** La evidencia es unívoca: los 7 commits cubren exactamente H0 y H1 tal
como el plan 03 §G los enumera (los mensajes de commit coinciden literalmente con los "Commits:" de H0 y H1, `03-plan-implementacion.md:457,462`). El trabajo se detuvo al terminar H1. No hay commit parcial, rama ni stash de H2 (`git status` limpio, `git branch` = `main`).

**No fue (a):** las secciones B–D del plan 03 **no son vagas** — B.1 especifica el esquema de `tokens.json` con valores, B.3 la primitiva `moonlit_draw_surface(x,y,w,h,level,radius)` con su costo, C.1 el comando exacto de `convttf`, C.3 la tabla de 8 roles con px y archivo, D.1–D.5 el modelo de datos de Marea con tamaños. Sonnet no tenía por qué "no saber qué hacer".

**Tres causas contribuyentes (c) que el plan correctivo sí debe corregir, porque explican por qué el resultado se *lee* como terminado sin estarlo:**

1. **Documentación escrita en presente sobre estado futuro.** `CLAUDE.md` lista como comando vigente `design-system/.venv/bin/python3 design-system/generate.py` (anotado "desde H2" solo al final de la línea); `README.md:12` afirma "ningún color fuera de `design-system/tokens.json`" y `README.md:67` remite a `design-system/vendor/<asset>/`; `SKILL.md:18-20` apunta a `docs/moonlit-design-system/00-INDICE.md`. Nada de eso existe. Un lector (humano o Sonnet en sesión nueva) que abra el repo ve un sistema de diseño "documentado" y concluye que solo faltan detalles. **Corrección:** ningún documento del repo cita rutas inexistentes; cada hito del plan correctivo añade a su definición de hecho un `test -e` de cada ruta que documenta.

2. **H2 era demasiado grande para una sesión.** H2 (`03-plan-implementacion.md:464-472`) agrupa en un solo hito: `tokens.json` + `generate.py` podado de AF (766 líneas, con dependencias de AppKit/Swift para SF Symbols que hay que extirpar) + vendoreo de 3 familias + 8 `.fnt` + 20×3 iconos con verificación de tonos + `moonlit_fonts.c` + `check_tones.py` + eliminación de 5 archivos Metro + skill. Es, con margen, 3–4 sesiones. Un hito que no cabe en una sesión no se empieza o se abandona a medias; aquí no se empezó. **Corrección:** hitos de ≤1 sesión, cada uno con un solo entregable verificable.

3. **Definiciones de hecho no ejecutables en H0/H1 dejaron pasar deuda que el propio plan mandaba cerrar.** D-007 (`DECISIONS.md:42`) y `CLAUDE.md` exigen `gen_fonts.sh` con rango decimal 32–383; `firmware/tools/gen_fonts.sh:47-48` sigue con `START=0x20`/`LIMIT=0x17F`. El contrato §A.8 promete a Studio el centinela `/.rockbox/fonts/moonlit-body-18.fnt` (`CONTRATO-moonlit-studio.md`, D-024) y no existe. Ninguna de las dos se detectó porque el cierre de H0/H1 no tenía un comando que fallara. **Corrección:** toda definición de hecho es un comando con salida esperada; el hito de revisión adversarial final los re-ejecuta todos.

**Hallazgo adicional que el plan 03 no conocía (afecta a los tres repos):** el bug del rango no es solo un desperdicio de tamaño. Verificado en la cabecera RB12 de los `.fnt` versionados (`firmware/rockbox/firmware/font.c:399-411` para el layout): los 5 archivos llevan `firstchar=13`, `defaultchar=13`, `size=8470` — el rango efectivo es U+000D…U+2122, `-D 0x3F` tampoco aplicó (`convttf.c:1163` `atol("0x3F")=0`, luego `:718-722` fuerza `defaultchar=firstchar`). Causa: `convttf.c:1101` `atoi(p)` y `:1113` `atol(p)` no entienden hex; `:674` convierte `limit_char==0` en "todo el font". ~42 KB de los 190 KB de `metro-display-48.fnt` son tabla de offsets vacía. Sin regresión funcional hoy (carga completa por `font.c:365-377`, no por el conteo de 352 que `metro_fonts.c:28-32` afirma).

## I.4 Flujo de portadas en Aura-Firmware — SÍ existe como módulo identificable

Renombrado "Music Flow" en `22f44275` (D-317). Archivos en `AF/firmware/rockbox/apps/aura/`:

| Archivo | Líneas | Rol | Dependencias |
|---|---|---|---|
| `aura_flow.c` / `.h` | 229 / 136 | Motor de proyección (punto fijo, Möbius de `pictureflow.c`), **1D horizontal por construcción** | **Cero** (`aura_flow.c:23` solo `aura_flow.h`; `__builtin_clz` en `:44-47`) |
| `aura_musicflow.c` / `.h` | 1471 / 86 | Pantalla: carrusel, flip, tracklist, input | 12 módulos Aura + `apple2026_tokens.h` (`aura_musicflow.c:23-51`) |
| `aura_art.c` / `.h` | 206 / — | Caché `.pfraw`, transposición, máscara de esquinas, reflejo | `aura_settings.theme` (`aura_art.c:104,149`), `a26_shell_isqrt256` (`:181`), `a26_shell_blend` (`:203`) |
| `aura_albumart.c` / `.h` | 583 / 134 | Decodificación desde tagcache/`find_albumart`/APIC + caché | 7 cabeceras Aura (`:39-46`) — **no se copia** (D-020 ya lo cerró) |
| `aura_wheel.c` / `.h` | 51 / 58 | `aura_wheel_step(velocity_deg_s)` → 1..3 pasos | Cero (`aura_wheel.c:23`) |
| `test/test_flow.c` | 211 | 7 pruebas del motor en host (`test/Makefile:46-47`) | — |

Cómo dibuja (`aura_musicflow.c:605-713`, `draw_slide_perspective`): por tapa, `aura_flow_begin_projection(&proj,&slide,130)` (`:679`); bucle `while (proj.screen_x < AURA_FLOW_SCREEN_W)` (`:681`) toma columna fuente `col` y escala vertical `dy`, lee `cover + col*MF_COVER_SIZE` (`:687`, **memoria transpuesta: columna contigua**), muestrea 162 filas y blitea **`lcd_bitmap(col_buf, proj.screen_x, y_col, 1, n_rows)`** (`:709`). Fade lateral por LUT de 256 entradas (`:513-567`, D-240). Sin `lcd_active()` dentro del módulo: la puerta única está en `aura_main.c:513` con cadencia `HZ/20` mientras `aura_musicflow_animating()` (`:606-614`).

Cómo consume la rueda: Aura **no usa `get_action()`**; lee `button_get()` crudo (`aura_main.c:167-239`) y captura la velocidad con `button_get_data() & 0xFFFFFF` (`:233-234`) → `aura_main_wheel_velocity()` (`:81-83`) → `aura_wheel_step()` (`aura_wheel.c:25-47`) → `scroll_step(dir)` (`aura_musicflow.c:1238-1259`) que mueve `s_target_index` acotado (sin loop) y arma la animación de 220 ms (`:185`, `anim_pos_x256()` `:316-321`, unidad = índice×256).

Memoria: `mf_slot_t` (`aura_musicflow.c:154-161`) = `cover_buf` 130×130×2 + `reflection_buf` 130×32×2 = 42 120 B × 39 slots = 1,60 MB **estático en BSS** (`:76-90`). Caché disco `/.rockbox/aura/cfcache/<seek>-<size>.pfraw` (`aura_albumart.c:79-85`), cabecera `{size,radius,theme,extra}` (`aura_art.c:91-96`), payload transpuesto. Precarga síncrona al arranque (D-224, `aura_music.c:221-300`).

Rendimiento: **no existe ninguna medición de ms/cuadro** de Music Flow en `AF/DECISIONS.md` ni `docs/` (solo el perfil por lectura de código del morph de NowPlaying, `DECISIONS.md:461`, y la nota `:262` "el simulador no sirve para medir el costo real por cuadro"). La cadencia objetivo es 20 fps (`aura_main.c:607`).

Consecuencia para el plan: **portar a vertical no es transponer parámetros**. Tres puntos cableados al eje X: `AURA_FLOW_SCREEN_W` (`aura_flow.h:72`), `AURA_FLOW_DISPLAY_LEFT_R`/`MAXSLIDE_LEFT_R` (`:77-78`), `proj->screen_x` (`:109`) y el corte en `aura_flow.c:192,207`. En vertical el barrido es por filas → el `.pfraw` conviene **fila-contigua** (no transpuesto) y el blit pasa a `lcd_bitmap(row_buf, x, y, n_cols, 1)`. Movie Flow (`aura_movieflow.c:70-77`, tapas 120×160) demuestra que el motor tolera aspectos no cuadrados sin tocarlo.

## I.5 Pipeline de fuentes en Metro-Aura (idéntico en moonlit-aura)

- **`.fnt`**: `firmware/assets/fonts/metro-{display-48,title-28,list-20,listsel-20,caption-14}.fnt` (versionados, `git ls-files`). TTF fuente en `firmware/assets/fonts-src/` + `LICENSE.txt` (`.gitignore:20-21` los exceptúa).
- **Generación**: `firmware/tools/gen_fonts.sh`. Compila `convttf` si falta (`:20-25`: `cc -lm -std=c99 -O2 -Wall -g convttf.c -o convttf $(pkg-config --cflags --libs freetype2)`). Tabla `rol:ttf:px:spacing` en `:30-36`. Comando `:64-65`: `"$CONVTTF" -p "$size" -s "$START" -l "$LIMIT" -D "$DEFAULT" -c "$spacing" -o "$out" "$in"`. Sin `-x` (M-028, `:42-46`). Bug de rango hex en `:47-49` (I.3). Nota: en `convttf` **`-r` no es rango** sino separación de filas (`convttf.c:1184-1193`); el rango es `-s`/`-l`.
- **Instalación**: simulador `firmware/tools/build_sim.sh:46-53` (`make install` + `cp firmware/assets/fonts/*.fnt simdisk/.rockbox/fonts/`); dispositivo `firmware/tools/package_dist.sh:128-136` (mismo `cp` sobre el stage) con centinelas `:145-161` (`metro-display-48.fnt`, `metro-list-20.fnt`). Las fuentes **no** pasan por el build de Rockbox (ninguna regla en `apps/SOURCES`/`*.make`).
- **Selección en runtime**: `apps/metro/metro_fonts.h:30-37` (`enum metro_font_role`: DISPLAY 48, TITLE 28, LIST 20, LIST_SEL 20, CAPTION 14); tabla rol→archivo `metro_fonts.c:40-46`; carga `metro_fonts_init()` `:50-76` con `font_load_ex(path, 0, METRO_FONT_GLYPH_BUDGET=400)` (`:62`); fallback por rol a `FONT_SYSFIXED` (`:62-74`); accessor `metro_font_id()` `:78-83`; init única en `metro_main.c:241`. `MAXUSERFONTS 12` (`firmware/rockbox/firmware/export/font.h:51`), Metro usa 5. Todo texto pasa por `metro_draw_text*()` (`metro_draw.c:48-106`, `DRMODE_FG`, M-051); 19 sitios `lcd_setfont(metro_font_id(...))` (`metro_draw.c:51,98,168,202,261,325,386`; `metro_main.c:151`; `metro_screen_lock.c:172`; `metro_screen_hub.c:862`; `metro_screen_nowplaying.c:619`; `metro_screen_splash.c:42,44,69,71`; `metro_screen_photo_viewer.c:353`; `metro_transitions.c:229`; `metro_widgets.c:99`). Pantalla de especímen: `metro_screen_specimen.c:21-30`.

## I.6 Simulador SDL en este Mac — CONFIRMADO

- `firmware/rockbox/tools/configure` existe en los tres repos (148 185 B); parche Darwin `configure:396-411` elige `gcc-16/15/14/13` de Homebrew.
- Este Mac: `/opt/homebrew/bin/sdl2-config` (sdl2 2.32.10), `gcc-16` (gcc 16.1.0), freetype 2.14.3, librsvg 2.62.3 (`rsvg-convert`), python3. **No hay `arm-elf-eabi-gcc` en PATH**; `moonlit-aura/firmware/toolchain/` **no existe** — el target se compiló con `RBDEV_TOOLCHAIN` apuntando a `Metro-Aura/firmware/toolchain/bin` (existe).
- Binarios ya producidos hoy: `moonlit-aura/firmware/build-sim/rockboxui` (1 873 480 B, 25-ago 15:50) y `firmware/build-ipod6g/rockbox.ipod` (1 216 864 B, 15:40). `build-ipod6g-boot/` no existe (bootloader sin compilar en moonlit).
- Captura headless: `firmware/tools/sim_shot.sh <out.png> [ticks] "BOTONES"` vía `METRO_SIM_AUTODUMP_*` (`sim_shot.sh:37-45`, `uisimulator/common/sim_tasks.c:313-324`) + `sips` a PNG (`:54`). Matriz: `sim_matrix.sh`. Tests host: `make -C apps/metro/test test` (9 binarios, `test/Makefile:11`).

## I.7 Otros hechos de Metro que el plan correctivo reutiliza

- Acento dinámico: `metro_theme.c:89-92` `metro_color_accent()`; tabla `:23-34`; tema dark/light ya existe (`metro_theme.c:67-87` resolvers bg/fg/secondary/tertiary).
- Primitivas fb: `metro_fb.c` — `blend_pixel` `:98-105` (lerp entero 8.8), `metro_fb_plot_alpha` `:116-127`, `metro_fb_blend_over_color` `:151`, `metro_fb_fill_rect` `:220`. **No existe rectángulo redondeado** en `apps/metro/`; sí anillo antialias `metro_widgets_draw_circle` (`metro_widgets.h:49`) y glifo 8-bit `metro_widgets_draw_glyph` (`:94`, M-089).
- Input: `metro_input.c:24-43` (`get_custom_action(ctx|CONTEXT_PLUGIN)` + `button_apply_acceleration(get_action_data())` bajo `HAVE_WHEEL_ACCELERATION`); contextos `metro_keymap.h:74-81`; tablas `metro_keymap.c` (HUB `:25-29`, LIST `:41-45`, PLAYER `:76-80` = volumen, VIEWER `:98-100`). Loop `metro_main.c:288-335`; despacho por pantalla `:453-461` (hub/nowplaying/photo_viewer/list); `redraw_current()` `:92-100`. Patrón de pantalla a pantalla completa = `metro_screen_photo_viewer.h:49-58` (`push/is_current/show/handle`).
- Álbumes: `metro_music_albums(out,max)` (`metro_music.h:114`), `metro_music_songs_of_album` (`:135`), `metro_music_track_path` (`:131`), `metro_music_play_songs_of_album` (`:144`); ítem `{label, subtitle(artista), seek}` (`metro_music.h:65-75`). Decodificación de carátula ya existente: `metro_albumart_decode_track_cover(path, fb_data*)` (`metro_albumart.h:94`) a `METRO_TILE_SIZE 80` (`metro_draw.h:143`), usada por `album_thumb_decode` (`metro_screen_hub.c:363-379`) con caché `.mth` de `metro_thumbs` (`metro_thumbs.h:38-62`).
- `lcd_active()` en Metro: `metro_screen_hub.c:818`, `metro_screen_list.c:280`, `metro_screen_nowplaying.c:438`, `metro_screen_usb.c:144`, `metro_transitions.c:170,366,402`.
- Registro de fuentes C en el build: `firmware/rockbox/apps/SOURCES:312-354` (43 líneas `metro/…`).
