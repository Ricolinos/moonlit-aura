# moonlit.aura — Reporte de la corrida nocturna (M10→M12)

**Fecha:** 2026-08-26 · **Estado:** DETENIDO en M12 — M10 y M11 hechos y
verificados; M12 exige hardware físico (ver "Estado al despertar" al final).

**Nota de reanudación (2026-08-26):** el usuario confirmó por chat seguir sin
`docs/plan/prompt-hito.md` — opción (b) del cierre de este reporte: cada
subagente de hito recibe `05-plan-correctivo.md` §II.0 "Reglas del plan"
directamente como sus instrucciones de método, sin un prompt B aparte.

## Verificación de prerrequisitos (M1–M9)

`git log --oneline moonlit-fork-base..HEAD` confirma M1–M9 completos, terminando en:

```
c992cbb6 fix: hub no repite el creciente en la barra de estado (D-044)
ef718d2f feat: splash/about/hub/statusbar/usb use logo; remove provisional wordmark (D-044)
bff717f8 feat: Waning Crescent logo spec + generated masks (D-016)
114b932d feat: pantalla Marea -- Cover Flow vertical, portadas izq/info der (D-029, D-030)
```

Esto sí está en orden. El problema está en el método pedido para M10 en adelante.

## Motivo de la parada

La instrucción dice: *"Cada hito lo ejecuta un subagente nuevo con el prompt de
`docs/plan/prompt-hito.md` (el prompt B) y el número de hito"*.

**`docs/plan/prompt-hito.md` no existe.** Verificado:

```
$ test -f docs/plan/prompt-hito.md && echo existe || echo "NO EXISTE"
NO EXISTE

$ find . -iname "*prompt*hito*" -not -path "*/.git/*"
(sin resultados)

$ git log --all --oneline -- docs/plan/prompt-hito.md
(sin resultados — nunca existió en ningún commit de este repo)
```

No aparece bajo otro nombre en `docs/plans/` (que sí tiene `PROMPT-aura-studio.md` y
`PROMPT-hermanos-gen-fonts.md`, pero ninguno de los dos es un prompt de ejecución de
hitos de este plan — son prompts aparte para Aura-Studio y para los repos hermanos,
ya referenciados por `05-plan-correctivo.md`).

Sin ese archivo no puedo ejecutar el método pedido: no sé qué instrucciones exactas
debe llevar cada subagente de hito más allá de lo que yo mismo podría inferir de
`05-plan-correctivo.md` §II.0 — y las condiciones de parada de esta misma tarea dicen
explícitamente **"no improvises una solución"**. Redactar yo un prompt B a partir de
mi propia lectura del plan sería precisamente eso: una decisión de método tomada en
tu ausencia que no pediste, sobre un archivo que aparentemente ya existía en tu
cabeza (o en otra sesión) y no llegó a este repositorio.

En esta primera pasada no ejecuté M10. No toqué `apps/metro/` ni ningún otro
archivo de código — el único cambio fue este mismo reporte. La reanudación
(M10 en adelante) queda documentada abajo.

## M10 — DECISIONS/README/CLAUDE/skill/design-system al día

**Commit:** `4b6476fb` — `docs: M10 -- DECISIONS/README/CLAUDE/skill/design-system en sincronía con M1..M9`

Ejecutado por subagente con contexto limpio, instrucciones = `05-plan-correctivo.md`
§II.0 + sección M10. Definición de hecho pasó en el primer intento (sin reintentos).
Verificado independientemente después por la sesión coordinadora (comandos
re-ejecutados, mismo resultado) y con `make -C firmware/rockbox/apps/metro/test test`
(12 suites, 0 fallas).

**Salida real de la definición de hecho (re-verificada por la sesión coordinadora):**

```
$ for p in $(grep -oh '`[a-zA-Z0-9_./-]*\.\(md\|json\|py\|sh\|h\|c\|fnt\|txt\)`' CLAUDE.md README.md .claude/skills/moonlit-design-system/SKILL.md docs/moonlit-design-system/*.md | tr -d '`' | sort -u); do test -e "$p" || echo "MISSING $p"; done
(sin salida — cero MISSING)

$ grep -c 'Implementada en M' DECISIONS.md
21

$ head -1 docs/plans/archivo/03-plan-implementacion.md | grep -c 'ESTADO:'
1
$ ! test -e docs/plan/03-plan-implementacion.md
(no existe -- OK)

$ wc -l CLAUDE.md
37 CLAUDE.md

$ git diff --stat moonlit-fork-base..HEAD -- 'firmware/rockbox/*' ':!firmware/rockbox/apps/metro' | tail -1
5 files changed, 96 insertions(+), 54 deletions(-)
```

**Qué se hizo:**
- `DECISIONS.md`: D-004…D-016 con línea "Implementada en M`<n>`, commit `<sha>`"
  (D-009 y D-015 documentadas como sin código propio en M1–M9, citando D-018/H0/
  `CONTRATO-moonlit-studio.md`).
- `CLAUDE.md` reescrito con la versión II.3 del plan (37 líneas), rutas corregidas
  hacia `Aura-Firmware/` como repo hermano (`../`), no subcarpeta.
- `README.md`: sección "Estado" real (M1–M9 hechos, M10–M12 pendientes).
- `CONTRATO-moonlit-studio.md` §A.8/§B: centinela de fuente ya real (no aspiracional),
  cita M12 en vez del H7 del plan viejo.
- `docs/plan/03-plan-implementacion.md` → `docs/plans/archivo/` con
  `ESTADO: SUPERADO por 05-plan-correctivo.md (H0–H1 ejecutados)`; se corrigió la
  única referencia a la ruta vieja fuera de `docs/plan` (`design-system/generate.py`).
- `.claude/skills/moonlit-design-system/SKILL.md`: cuerpo real (81 líneas, antes
  esqueleto de H0).
- `docs/moonlit-design-system/00-INDICE.md`: enlaces a Marea (M8) y logo (M9) que
  faltaban.
- `MODIFICATIONS.md`: no necesitó cambios — los 5 archivos de `firmware/rockbox/`
  fuera de `apps/metro/` tocados en M1–M9 ya estaban documentados ahí.

**Desviación (no es una decisión de diseño nueva):** el propio comando de
verificación de rutas reveló referencias preexistentes (en `CLAUDE.md`, `SKILL.md`,
`00-INDICE.md`) con nombres de archivo sueltos o enlaces relativos que no resolvían
desde la raíz del repo — nunca se habían comprobado mecánicamente antes de M10. Se
normalizaron a rutas completas sin cambiar ningún comportamiento.

Sin hipótesis abiertas. `git status --short` tras el commit: solo
`docs/plan/04-auditoria-brecha.md` y `docs/plan/05-plan-correctivo.md` sin trackear
(preexistentes, fuera del alcance de M10).

## M11 — Revisión adversarial global

**Commit:** `79131c0e` — `docs: M11 -- revision adversarial global, re-verificacion M1..M10 (D-045)`

Ejecutado por subagente con contexto limpio (instrucciones = §II.0 + sección M11),
que a su vez lanzó 4 subagentes independientes (uno por refutación, sin verse entre
sí), como exige el propio hito. Sin reintentos — completó en la primera pasada.
Verificado independientemente después por la sesión coordinadora: `M11-verificacion.txt`
releído directamente (10 líneas `OK M<n>`, el único match de "FAIL" es texto
descriptivo — "tonos... sin FAIL" —, no una línea de fallo), el hallazgo D-045
confirmado leyendo el código citado línea por línea, `make test` (13 suites, 0
fallas), `build_sim.sh` y `build_target.sh --firmware` (ambos exit 0).

**`docs/screenshots/M11-verificacion.txt` — re-verificación de M1–M10, una línea por hito:**

```
OK M1  (con nota de falsos positivos del grep RGB -- button_get_data() & 0xFFFFFF
        es máscara de bits, no color; sin violación real)
OK M2  (fvar ausente confirmado por parseo directo de tabla TTF)
OK M3  (verificación reinterpretada sobre Ahora Suena -- el hub raíz es lista WP7
        sin iconos por D-034, consistente con el commit real f8516c50)
OK M4  (24/24 PNG de la matriz; check_tones --edge OK en night y dawn)
OK M5  (nowplaying night/dawn, lock, usb; píxel de fondo == night.surface ±8)
OK M6  (diff moonlit_flow.c vs aura_flow.c = 58 líneas, < 60)
OK M7  (1ª corrida: 8 decode; 2ª corrida: 0 decode / 8 hit; 8×28816B)
OK M8  (.bss=8569948B, coincide con D-043)
OK M9  (5 máscaras del creciente + wordmark, tonos ≥4, luz desde la izquierda)
OK M10 (sin rutas MISSING, 21 "Implementada en M", CLAUDE.md 37 líneas)
```

**Las 4 refutaciones adversariales (subagentes independientes):**

1. *"moonlit no tiene sistema de diseño propio"* — **no prospera**. Cero
   `#define METRO_*` de color, cero `MFONT_CAPTION`, cero fuga de "metro" en UI,
   cero RGB fuera de `moonlit_tokens.h`.
2. *"Marea no parte de Aura-Firmware"* — **no prospera**. Diff mecánico de
   `moonlit_flow.c` contra `aura_flow.c` = 58 líneas (< 60), trazabilidad de sha
   presente, adaptaciones documentadas (D-014/019/020/041/042/043) sí están en el
   código.
3. *"La frontera GPL del bootloader no se sostiene"* — **no prospera**. Cero
   commits desde `moonlit-fork-base` en `bootloader/`/`mks5lboot/`.
4. *"Las restricciones vinculantes no se cumplen"* — **prospera parcialmente.**
   Hallazgo real, verificado línea por línea por la sesión coordinadora:

### ⚠️ D-045 (pendiente) — lectura de disco dentro de un bucle de animación, en Marea

`run_scroll_animation()` (`apps/metro/moonlit_screen_marea.c:511-538`) está bien
gateada por `lcd_active() && animations != OFF` **antes** de su `for` — pero el
cuerpo de ese `for` llama, cuadro a cuadro, a `moonlit_screen_marea_show()` →
`draw_slide()` → `get_slot_for()` (`:192-227`), y en un cache-miss de tapa
(álbum que entra a la ventana visible sin estar en los 37 slots LRU),
`get_slot_for()` llama a `moonlit_art_read_pfraw()`, que hace `open()`+`read()`
reales (`moonlit_art.c:51,72`) — **dentro del bucle**, no antes. Es una lectura de
un `.pfraw` ya horneado (nunca un decode de JPEG, eso sí sigue fuera del bucle,
en `moonlit_screen_marea_tick()`), pero `CLAUDE.md` no distingue tipos de lectura:
la regla dice "ninguna lectura de disco dentro de un bucle de animación", punto.

No se corrigió inline (regla de M11: solo si son ≤ 20 líneas) porque la corrección
real exige decidir una ventana de precarga — precalcular antes del `for` todos los
álbumes que puedan entrar a la vista durante el scroll (no solo el destino), o
partir `get_slot_for()` en una variante solo-caché (para el bucle, cae a monograma
en miss) y una con lectura real (para precarga y `_tick()`) — contra el presupuesto
fijo de `MAREA_CACHE_SLOTS` (37) y verificado contra scroll rápido (`step > 1`).
Eso es una decisión de diseño que ni `00-decisiones-moonlit.md` ni `DECISIONS.md`
tenían cerrada, así que quedó registrada como **D-045 pendiente**
(`DECISIONS.md:919-968`), no inventada sobre la marcha. Severidad acotada en la
práctica (una sola pantalla, ya D-043 "experimental hasta M12"; archivo de tamaño
fijo 28 816 B, no un JPEG), pero es una violación real de una regla vinculante y
sigue sin corregirse.

Sin más hipótesis abiertas de M11. `git status --short` tras el commit: solo los
dos archivos de plan preexistentes, sin trackear desde antes de esta corrida.

## M12 — Hardware, empaquetado y cierre: NO EJECUTADO

M12 es, por diseño del propio plan, `[hardware, usuario]`: exige flashear un iPod
Classic 6G real y medir `DEBUGF("marea frame %ld ms")` en el dispositivo (criterio
máx ≤ 33 ms/cuadro), además de decidir sobre esa medición si `MOONLIT_FLOW_CAM_DIST`
(D-041, sin retunear) necesita ajuste. Ningún subagente puede ejecutar esto —no hay
hardware ni manos para flashear— así que la corrida se detiene aquí de forma
esperada, no por un error.

## Estado al despertar

- **Hechos y verificados esta corrida:** M10 (`4b6476fb`) y M11 (`79131c0e`).
  Ambos con definición de hecho re-ejecutada de forma independiente por la sesión
  coordinadora (no solo por el reporte del subagente) y en verde.
- **M1–M11: todos hechos.** Solo queda M12.
- **Detenido en:** M12, por diseño (requiere tu iPod real). No es una falla ni una
  ambigüedad del plan — es la única parada que el propio plan marca como
  `[hardware, usuario]`.
- **Hallazgo abierto que necesita tu decisión, no solo tu iPod:** `D-045`
  (arriba) — Marea lee disco dentro de su bucle de animación de scroll en un
  cache-miss. Es real, acotado, pero viola una regla vinculante de `CLAUDE.md`.
  Antes o durante M12 hay que decidir el enfoque de corrección (ventana de
  precarga vs. `get_slot_for()` con variante solo-caché) — no lo puedo cerrar yo
  solo porque es una decisión de diseño nueva, no una que ya estuviera cerrada.
- **Qué necesito de ti:**
  1. Flashear M12 en un iPod real y correr su medición (o decirme si prefieres que
     alguien más lo haga).
  2. Decidir el enfoque de D-045 (o decirme que lo decida yo con una recomendación,
     si prefieres delegarlo).
- `git status --short` limpio salvo los dos archivos de plan que ya estaban sin
  trackear al empezar toda esta corrida (`04-auditoria-brecha.md`,
  `05-plan-correctivo.md`) — nada de esta sesión quedó sin commitear. Ningún
  `git push` en toda la corrida.
