# moonlit.aura — Reporte de la corrida nocturna (M10→M12)

**Fecha:** 2026-08-26 · **Estado:** EN CURSO — M10 hecho, M11 en marcha.

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
